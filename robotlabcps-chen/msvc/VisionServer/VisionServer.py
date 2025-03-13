import numpy as np
import cv2
import time
import os
import threading
import logging
import sys

import torch

sys.path.append("/home/chenqifan/robotlabcps-cqf/robotlabcps-chen/python")
from pyjcpsapi import jcpsapi
from Servo import Servo


CAMERA_SERVER_DEV_ID = 774
VISION_SERVER_DEV_ID = 775
MOVE_SERVER_DEV_ID   = 776
VISION_SERVER_APP_ID = 4098  # APP的id需要在[0x1001,0x2000]范围内

MSG_PREDICT = 0x1102
MSG_PREDICT_RSP = 0x1103
def configure_logger(log_dir: str) -> logging.Logger:
    """Configure and return training logger.

    Args:
        log_dir: Directory to store log files

    Returns:
        Configured logger instance
    """
    os.makedirs(log_dir, exist_ok=True)
    logging.basicConfig(
        level=logging.INFO,
        format='[%(asctime)s] %(message)s',
        datefmt='%Y-%m-%d %H:%M:%S',
        handlers=[
            logging.FileHandler(os.path.join(log_dir, 'servo.log')),
            logging.StreamHandler()
        ]
    )
    return logging.getLogger(__name__)


class VisionHandler(threading.Thread):
    def __init__(self, api):
        super(VisionHandler, self).__init__()
        self._api = api
        self._stop_flag = False
        self.path = None
        self.index = 0

    def run(self):
        while not self._stop_flag:
            msg = self._api.recv_msg()
            if not msg:
                time.sleep(0.1)
                continue
            self.process_request(msg)

    def process_request(self, msg):
        from_id, msg_type, json_data, msg_len = msg
        path = json_data['path']
        self.path = path
        print(path)
        #构造抓取结果
        if msg_type==MSG_PREDICT:#条件记得改！！！！
            state,new_pose,image_path1,image_path2= self.run_servo_logic()
            target_pose =np.array(new_pose).tolist()
            print(state)
            print("New pose:", target_pose)
        else:
            target_pose = [1.309,-1.633, 1.269, -1.207, -1.591, -0.288]
        move_command = {'name': state, 'value': target_pose , 'image_path':image_path1,'image_path0':image_path2}
        msg_type = MSG_PREDICT_RSP
        self._api.async_send_json_msg_to_app(from_id, msg_type, move_command)
        # self._api.async_send_json_msg_to_bus(msg_type, grasp_command)

    def run_servo_logic(self):
        # 初始化 Servo 类并运行视觉伺服逻辑
        logger = configure_logger('./servo_log')  # 配置日志
        goal_pose = torch.tensor([0.0356, -0.3057,  0.4143, -0.7668, -2.9868, -0.0674])
        final_pose = torch.tensor([0.0357, -0.4589,  0.3059, -0.7669, -2.9868, -0.0675])  # 目标位姿
        weight = [0, -0.5, 0.6, 0, 0, 0]  # 运动权重

        # 创建 Servo 实例
        node = Servo(final_pose,goal_pose, weight, logger,self.index)
        self.index+=1
        # 设置目标图像和实时图像
        #node.rgb_goal = '/home/chenqifan/IBVS_keypoint_based/light.png'
        node.rgb_goal = '/home/chenqifan/robotlabcps-cqf/robotlabcps-chen/msvc/goal_image.png'  # 替换为你的目标图像路径
        #node.rgb_live = '/home/chenqifan/IBVS_keypoint_based/light.png'
        node.rgb_live = os.path.join(self.path,"0.png")  # 替换为你的实时图像路径

        # 运行视觉伺服逻辑
        node.servo_thread.start()
        node.servo_thread.join()  # 等待线程完成

        # 返回 new_pose
        return node.state,node.new_pose,node.image_path_1,node.image_path_2

def main():
    # create and start handler
    api = jcpsapi.JCPSApi()
    print('API Version: {}'.format(api.api_version()))
    t = VisionHandler(api)
    t.start()

    # initialize CPSAPI
    if not api.initialize(jcpsapi.JCPS_API_TYPE_MIX, 
                          VISION_SERVER_DEV_ID, VISION_SERVER_APP_ID, 
                          '127.0.0.1', 4000, '127.0.0.1', 4888):
        return

    # app向总线订阅device_id
    api.register_app([CAMERA_SERVER_DEV_ID])  # 作为APP需要收到指定的DEVICE的id
    
    # device向总线注册自己的device_id
    # api.register_device() ## already registered a device:似乎在initialize那一步就已经register device了

    t.run()

    api.release()


if __name__ == '__main__':
    main()






    ###### 视频笔记

### 定义ProcessThread 类*

#def run  #while calc_queue里非空时、处理请求
##queue的用法 get 、处理process_calc_request、 task down

#def process_calc_request(self, calc_req:PPC.ST_ReqReversalCalc):    # ST_ReqReversalCalc ???
#    from_id, req = calc_req

#    # 计算

#    result = PPC.ST_ReversalCalcResult()     # ST_ReversalCalcResult 在Myhandler.h
#    result.id = req.id
#    result.L1 = 2.3
#    result.L2 = 2.4
#    result.L3 = 2.5
#    result.L4 = 2.6

#    result.info = error_code = 0
#    result,info.error_msg = "success"

#    self._handler.SendReversalCalcResult(from_id, result)     # handler里面封装了CPSAPI的sendMsg


### 定义calc handler 类*
#class CalcHandler(PPC.CParallelCalcHandler):
#def __init__

#def OnReqReversalCalc(self, from_id, req):
#    print("received calc reques from id ...")
### 收到请求后扔到queue里、
#    self._calc_queue.put((from_id,req))
##构造应答信息
#    rsp

## 发送应答信息
#    self.SendReversalCalcRsp(from_id, rsp)                  # handler里面封装了CPSAPI的sendMsg


#def main():
#    api = PPC.CPSAPI.CreateAPI.GetApiVersion()
#    print("AsPI version:{}".format(api_version))

#    # 创建API
#    api = PPC.CPSAPI.CreateAPI()      
#    # 创建消息队列
#    calc_queue = queue.Queue()          
#    # 创建计算请求Handler 在总线
#    handler = CalcHandler(api, calc_queue)      #CalcHandler ???

#    # API初始化
#    if api.Init(PPC.E_CPS_TYPE_DEVICE, PPC.CALC_SERVER_DEV_ID, 0, "127.0.0.1", 5000, "127.0.0.1",
#                8888, handler)!= 0 :
#        print("API init failed.\n")
#        return
#    # 创建计算处理线程 几个核就可以创建几个线程、同时处理几个计算请求
#    process_thread = ProcessThread(calcqueue, handler)
#    process_thread.start()




###############################  收到来自CPSUI的图片路径  ########################
## 收到字符串






###############################  发送目标位姿给CPSUI  ########################
## 1.948, -1.316, 0.830, -0.925, -1.921, 0.393 



