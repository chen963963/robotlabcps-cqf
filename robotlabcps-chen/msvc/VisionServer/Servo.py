import numpy as np
import torch
import torchvision.transforms as T
import warnings
import time
import os
import threading
import itertools
import logging
from cv_bridge import CvBridge
from rtde_control import RTDEControlInterface as RTDEControl
from rtde_receive import RTDEReceiveInterface as RTDEReceive
import sys

#from msvc.VisionServer.Seg.florence_sam.seg_utils.tmp.create_training_data import goal_path

sys.path.append('./dino-vit-features')
from correspondences import find_correspondences, draw_correspondences
from my_utils.MODEL_v2_2 import QuaternionAlignmentTransformer, Alpha
from my_utils.helper_functions import pose_euler_to_quaternion, quaternion_to_euler, compute_error
import my_utils.quaternion_calc as qc
from my_utils.parameters import *
IP = "192.168.12.252"

warnings.filterwarnings("ignore")
model_trans="./my_utils/trans_0213_3.pth"
model_rot="./my_utils/rot_model_0222-1.pth"
log_dir="/home/chenqifan/robotlabcps-cqf/robotlabcps-chen/msvc/VisionServer/servo_log"
class Servo:
    def __init__(self, final_pose,goal_pose, weight, logger,index):
        self.bridge = CvBridge()
        self.fps = 30
        self.process_flag = False
        self.rgb_goal = None
        self.rgb_live = None
        self.R_cam2gripper = np.eye(3)
        self.new_pose = None  # 新增属性，用于存储 new_pose
        self.state = None
        self.image_path_1 = None
        self.image_path_2 = None
        # 加载模型
        self.model_trans = Alpha()
        self.model_trans.load_state_dict(torch.load(model_trans))
        self.model_trans.eval()
        self.model_rot = QuaternionAlignmentTransformer(hidden_dim, hidden_depth_dim, num_heads, num_layers, input_dim)
        self.model_rot.load_state_dict(torch.load(model_rot))
        self.model_rot.eval()
        self.final_pose = final_pose
        self.goal_pose = goal_pose
        self.weight = weight
        self.logger = logger
        self.index = index

        self.lock = threading.Lock()
        # servoing thread
        self.servo_thread = threading.Thread(target=self.servoing)

    @staticmethod
    def _delta_euler_from_quat(curr_quat, goal_quat):
        """
        计算两个四元数之间的欧拉角差值，并解决相位跳变问题。

        参数:
        curr_quat -- 当前四元数 [x, y, z, w]
        goal_quat -- 目标四元数 [x, y, z, w]

        返回:
        delta -- 欧拉角差值 [roll, pitch, yaw] (弧度)，仅保留最大差值维度，其余为0
        """
        # 将四元数转换为欧拉角
        curr_euler = quaternion_to_euler(np.array(curr_quat))
        goal_euler = quaternion_to_euler(np.array(goal_quat))

        # 计算差值
        delta = goal_euler - curr_euler

        # 解决相位跳变问题（将差值限制在 [-pi, pi] 范围内）
        for i in range(3):
            if delta[i] > np.pi:
                delta[i] -= 2 * np.pi
            elif delta[i] < -np.pi:
                delta[i] += 2 * np.pi

        # 找到最大差值的维度
        max_index = np.argmax(np.abs(delta))
        max_delta = delta[max_index]

        # 仅保留最大差值维度，其余维度设为0
        delta = np.zeros_like(delta)
        delta[max_index] = max_delta

        return delta

    def pred_action(self, points1, points2, live_pose):
        '''预测eef位姿变换'''
        points1 = torch.tensor(list(itertools.chain(*points1)), dtype=torch.int)
        points2 = torch.tensor(list(itertools.chain(*points2)), dtype=torch.int)

        x = torch.cat((points1, points2), dim=0).float()

        # output of tanslation :
        delta_trans = self.model_trans(x)[:3]

        depth = torch.norm((torch.tensor(live_pose[:3]) - torch.tensor(self.final_pose[:3])),
                           dim=-1).unsqueeze(-1)

        # output of rotation
        live_pose_quat = pose_euler_to_quaternion(live_pose)
        output_quaternion = self.model_rot(live_pose_quat.float(),
                                           x,
                                           delta_trans,
                                           depth.float()).detach().numpy()

        predicted_quaternion = qc.batch_concatenate_quaternions(live_pose_quat[3:],
                                                                torch.tensor(output_quaternion,
                                                                             dtype=float))

        pred_delta_pose = np.concatenate((delta_trans.detach().numpy(),
                                          self._delta_euler_from_quat(live_pose_quat[3:],
                                                                      predicted_quaternion.detach().numpy())),
                                         axis=0)

        return pred_delta_pose


    def servoing(self):
        # visual servoing 对齐阶段
        print("已拍摄目标图像, 等待3秒...")
        time.sleep(3)
        print("开始执行视觉伺服...")
        #index = 0
        while True:
            # Compute pixel correspondences between new observation and bottleneck observation.
            with torch.no_grad():
                with self.lock:
                    points1, points2, image1_pil, image2_pil = find_correspondences(PROMPT, self.rgb_live,
                                                                                    self.rgb_goal, num_pairs, load_size,
                                                                                    layer,
                                                                                    facet, bin, thresh, model_type,
                                                                                    stride)

                fig1, fig2 = draw_correspondences(points1, points2, image1_pil, image2_pil)
                #/home/chenqifan /robotlabcps-cqf/robotlabcps-chen/msvc/VisionServer/serve_log
                image_path_1 = os.path.join(log_dir, f'./keypoints_live_{self.index}.png')
                self.image_path_1 = image_path_1
                fig1.savefig(image_path_1)
                image_path_2 = os.path.join(log_dir, f'./keypoints_goal_{self.index}.png')
                self.image_path_2 = image_path_2
                fig2.savefig(image_path_2)
            print("finish corresp")
            points1 = np.array(points1).reshape(1, 2 * num_pairs)
            points2 = np.array(points2).reshape(1, 2 * num_pairs)

            error = compute_error(points1, points2)
            rtde_r = RTDEReceive(IP)
            ActualTCPPose = rtde_r.getActualTCPPose()
            ActualQ = rtde_r.getActualQ()
            live_pose = torch.tensor(ActualTCPPose)
            rtde_r.disconnect()
            #live_pose = torch.tensor([-318.34845, 476.89444, 395.504242, 1.614076, -0.791094, -1.838659])
            print("当前误差是:", error)
            self.logger.info("当前误差是: %s", error)
            if error < ERR_THRESHOLD:
                self.logger.info("Error small enough, servoing ends. \n")
                self.state="catch"
                self.new_pose = self.final_pose-self.goal_pose+live_pose
                break

            ## 调用模型  ##
            prim_command = self.pred_action(points1, points2, live_pose)
            self.logger.info("predicted command: %s", prim_command)
            print("predicted command: ", prim_command)
            command = [cmd * weight for cmd, weight in zip(prim_command, self.weight)]
            self.logger.info("weighted command %s", command)


            new_pose = [live_pose[0] + command[0], live_pose[1] + command[1], live_pose[2] + command[2],
                        live_pose[3] + command[5], live_pose[4] + command[3], live_pose[5] + command[4]]
            self.new_pose = new_pose  # 将 new_pose 存储到属性中
            self.logger.info("live_new %s", new_pose)
            if new_pose[2] < 0.02:
                self.logger.info("Error: gripper sill touch the desk \n")
                self.state = "error"
                self.new_pose = None
                break
            self.logger.info("----- UR MOVED -----")
            self.state = "move"
            break

