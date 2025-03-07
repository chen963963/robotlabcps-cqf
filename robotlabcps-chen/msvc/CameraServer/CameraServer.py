import pyrealsense2 as rs
import numpy as np
import cv2
import time
import os
import threading

import sys

sys.path.append("/home/chenqifan/robotlabcps-cqf/robotlabcps-chen/python")
from pyjcpsapi import jcpsapi
import pyshare

IMAGE_NUM = 5  # 每次保存的彩色图像数量

######################################    接收CPSUI的采集指令    #######################################
CAMERA_SERVER_DEV_ID = 774
VISION_SERVER_APP_ID = 4098

MSG_CAPTURE_IMAGE = 0x1100
MSG_CAPTURE_IMAGE_RSP = 0x1101


class CameraHandler(threading.Thread):
    def __init__(self, api, save_path=None):
        super(CameraHandler, self).__init__()
        self._api = api
        self._stop_event = threading.Event()
        self._save_path = save_path if save_path else os.path.join(os.getcwd(), "out")

    def run(self):
        """
        接收到信息后，触发process_request
        """
        while not self._stop_event.is_set():
            msg = self._api.recv_msg()
            if not msg:
                time.sleep(0.1)
                continue
            self.process_request(msg)

    def process_request(self, msg):
        """
        处理请求：保存图片、把图片路径返回给总线
        """
        from_id, msg_type, json_data, msg_len = msg
        print('new message from {}, data: {}'.format(from_id, json_data['value']))
        if msg_type == pyshare.MSG_CAPTURE_IMAGE:
            path = self.save_image()
            self.send_path_msg(from_id, path)

    def send_path_msg(self, from_id, path):
        """
        把保存图像的路径发送给总线
        :param from_id: 消息来源的ID
        :param path: 图片的保存路径
        """
        # 构造包含路径的JSON数据
        json_data = {"path":path}

        # 使用MSG_CAPTURE_IMAGE_RSP作为消息类型
        msg_type = pyshare.MSG_CAPTURE_IMAGE_RSP

        # 发送消息
        self._api.async_send_json_msg_to_app(from_id, msg_type, json_data)
        print(f"Sent image path to app {from_id}: {path}")

    def save_image(self):
        """
        创建文件夹、实时显示图像、自动保存图像
        """
        pipeline = rs.pipeline()

        # 配置管道以仅流式传输彩色图像
        config = rs.config()
        config.enable_stream(rs.stream.color, 640, 480, rs.format.bgr8, 30)

        # 启动管道
        pipeline.start(config)

        # 按照日期创建文件夹
        save_path = os.path.join(self._save_path, time.strftime("%Y_%m_%d_%H_%M_%S", time.localtime()))
        os.makedirs(save_path, exist_ok=True)
        os.makedirs(os.path.join(save_path, "color"), exist_ok=True)

        # 实时显示图像的窗口
        cv2.namedWindow("live", cv2.WINDOW_AUTOSIZE)
        saved_count = 0

        # 主循环
        try:
            last_save_time = time.time()  # 记录上次保存的时间
            while True:
                frames = pipeline.wait_for_frames()
                color_frame = frames.get_color_frame()

                if not color_frame:
                    continue

                # 获取彩色图像
                color_image = np.asanyarray(color_frame.get_data())
                cv2.imshow("live", color_image)

                # 每隔 0.2 秒保存一张图像
                current_time = time.time()
                if current_time - last_save_time >= 0.2:
                    cv2.imwrite(os.path.join(save_path, "color", "{}.png".format(saved_count)), color_image)
                    saved_count += 1
                    last_save_time = current_time  # 更新上次保存的时间

                # 按 q 退出
                if cv2.waitKey(1) & 0xFF == ord('q') or saved_count >= IMAGE_NUM:
                    cv2.destroyAllWindows()
                    break
        finally:
            pipeline.stop()
        return save_path

    def stop(self):
        self._stop_event.set()


def main():
    # 创建并启动handler
    api = jcpsapi.JCPSApi()
    print('API Version: {}'.format(api.api_version()))
    t = CameraHandler(api)
    t.start()

    # 初始化CPSAPI
    if not api.initialize(jcpsapi.JCPS_API_TYPE_DEVICE, pyshare.CAMERA_SERVER_DEV_ID, 0, '127.0.0.1', 4000, '127.0.0.1',
                          4888):
        return

    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        t.stop()
        t.join()

    api.release()


if __name__ == '__main__':
    main()