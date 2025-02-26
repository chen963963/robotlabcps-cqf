import pyrealsense2 as rs
import numpy as np
import cv2
import time
import os
import threading

import sys
sys.path.append(r'D:\ZhangChen\robotlabcps\python')
from pyjcpsapi import jcpsapi
import pyshare


IMAGE_NUM = 5  # save %d color images and %d depth images at a time

######################################    接收CPSUI的采集指令    #######################################
#CAMERA_SERVER_DEV_ID = 774
#VISION_SERVER_APP_ID = 4098

#MSG_CAPTURE_IMAGE = 0x1100
#MSG_CAPTURE_IMAGE_RSP = 0x1101


class CameraHandler(threading.Thread):
    def __init__(self, api):
        super(CameraHandler, self).__init__()
        self._api = api
        self._stop_flag = False
        # self._path = ?

    def run(self):
        """
        接收到信息后，触发process_request
        """
        while not self._stop_flag:
            msg = self._api.recv_msg()
            if not msg:
                time.sleep(0.1)
                continue
            self.process_request(msg)


    def process_request(self, msg): # process_request(self, msg):
        """
        处理请求：保存图片、把图片路径返回给总线
        """
        path = "TEST_PATH: CameraServer"
        from_id, msg_type, json_data, msg_len = msg
        print('new messge from {}, data: {}'.format(from_id, json_data['value']))
        if msg_type == pyshare.MSG_CAPTURE_IMAGE:
            self.save_image()
            self.send_path_msg(from_id, path)
        #self._api.async_send_json_msg_to_app(from_id, msg_type, json_data)
 
    def send_path_msg(self, from_id, path): 
        """
        把保存图像的途径发送给总线 （先broadcast，之后再改）
        :path: 图片的保存路径
        """
        json_data = {'name': 'image path', 'value': path}
        msg_type_test = pyshare.MSG_CAPTURE_IMAGE_RSP
        self._api.async_send_json_msg_to_app(from_id, msg_type_test, json_data)
        # self._api.async_send_json_msg_to_bus(msg_type, json_data)
    
    def save_image(self):
        """
        创建文件夹、实时显示图像、存储图像
        """
        pipeline = rs.pipeline()

        #Create a config并配置要流​​式传输的管道
        config = rs.config()
        config.enable_stream(rs.stream.depth, 640, 480, rs.format.z16, 30)
        config.enable_stream(rs.stream.color, 640, 480, rs.format.bgr8, 30)

        profile = pipeline.start(config)

        depth_sensor = profile.get_device().first_depth_sensor()
        depth_scale = depth_sensor.get_depth_scale()
        print("Depth Scale is: " , depth_scale)

        align_to = rs.stream.color
        align = rs.align(align_to)

        # 按照日期创建文件夹
        save_path = os.path.join(os.getcwd(), "out", time.strftime("%Y_%m_%d_%H_%M_%S", time.localtime()))  #D:\ZhangChen\robotlabcps\msvc\CameraServer\out
        os.makedirs(save_path)
        os.mkdir(os.path.join(save_path, "color"))
        os.mkdir(os.path.join(save_path, "depth"))

        # 保存的图片和实时的图片界面
        cv2.namedWindow("live", cv2.WINDOW_AUTOSIZE)
        cv2.namedWindow("save", cv2.WINDOW_AUTOSIZE)
        saved_color_image = None # 保存的临时图片
        saved_depth_mapped_image = None
        saved_count = 0

        # 主循环
        try:
            while True:
                frames = pipeline.wait_for_frames()

                aligned_frames = align.process(frames)

                aligned_depth_frame = aligned_frames.get_depth_frame()
                color_frame = aligned_frames.get_color_frame()

                if not aligned_depth_frame or not color_frame:
                    continue

                depth_data = np.asanyarray(aligned_depth_frame.get_data(), dtype="float16")
                depth_image = np.asanyarray(aligned_depth_frame.get_data())
                color_image = np.asanyarray(color_frame.get_data())
                depth_mapped_image = cv2.applyColorMap(cv2.convertScaleAbs(depth_image, alpha=0.03), cv2.COLORMAP_JET)
                cv2.imshow("live", np.hstack((color_image, depth_mapped_image)))
                cv2.waitKey(1000) #30

                # 保存图片 (之后添加keyboard交互，例如按s保存)
                saved_color_image = color_image
                saved_depth_mapped_image = depth_mapped_image

                # 彩色图片保存为png格式
                cv2.imwrite(os.path.join((save_path), "color", "{}.png".format(saved_count)), saved_color_image)
                # 深度信息由采集到的float16直接保存为npy格式
                np.save(os.path.join((save_path), "depth", "{}".format(saved_count)), depth_data)
                saved_count+=1
                cv2.imshow("save", np.hstack((saved_color_image, saved_depth_mapped_image)))

                # q 退出 (之后添加keyboard交互，例如按q退出)
                if saved_count >= IMAGE_NUM:#key & 0xFF == ord('q') or key == 27:
                    cv2.destroyAllWindows()
                    break    
        finally:
            pipeline.stop()



def main():
    # create and start handler
    api = jcpsapi.JCPSApi()
    print('API Version: {}'.format(api.api_version()))
    t = CameraHandler(api)
    t.start()

    # initialize CPSAPI
    if not api.initialize(jcpsapi.JCPS_API_TYPE_DEVICE, pyshare.CAMERA_SERVER_DEV_ID, 0, '127.0.0.1', 4000, '127.0.0.1', 4888):
        return

    #t.process_request()


    #value = 0
    #msg_type_test = 1
    while True: # 改
        #json_data = {'name': 'x', 'value': value}
         #api.broadcast_device_msg(msg_type_test, json_data)
         #value += 1
         time.sleep(1)
         #if value == 10:
         #    break

    api.release()
    
if __name__ == '__main__':
    main()

######################################################################################################

    




######################################    发送采样结果给CPSUI    #######################################

# 发送“采样成功”消息
# bool IMAGE_ACQUISITION_SUCCESS

# 发送 图片存储路径
# save_path

