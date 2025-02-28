import numpy as np
import cv2
import time
import os
import threading

import sys
sys.path.append(r'D:\ZhangChen\robotlabcps\python')
from pyjcpsapi import jcpsapi



CAMERA_SERVER_DEV_ID = 774
VISION_SERVER_DEV_ID = 775
MOVE_SERVER_DEV_ID   = 776
VISION_SERVER_APP_ID = 4098  # APP的id需要在[0x1001,0x2000]范围内


class VisionHandler(threading.Thread):
    def __init__(self, api):
        super(VisionHandler, self).__init__()
        self._api = api
        self._stop_flag = False

    def run(self):
        while not self._stop_flag:
            msg = self._api.recv_msg()
            if not msg:
                time.sleep(0.1)
                continue
            self.process_request(msg)

    def process_request(self, msg):
        from_id, msg_type, json_data, msg_len = msg
        print(json_data['value'])
        #构造抓取结果
        target_pose = [1.309,-1.633, 1.269, -1.207, -1.591, -0.288]
        move_command = {'name': 'grasp command', 'value': target_pose} 
        self._api.async_send_json_msg_to_app(MOVE_SERVER_DEV_ID, msg_type, move_command)
        # self._api.async_send_json_msg_to_bus(msg_type, grasp_command)


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



