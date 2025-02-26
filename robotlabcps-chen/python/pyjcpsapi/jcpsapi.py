import json
import time
import threading
import queue
from collections import defaultdict
from functools import wraps
from .core import pycpsapi
from .log import JLOGGER as LOG


class JCPSMsgThread(threading.Thread):
    """
    CPS内部消息处理线程
    """

    def __init__(self, api):
        super(JCPSMsgThread, self).__init__()
        self._msg_cache = queue.Queue()
        self._queue = queue.Queue()
        self._stop_flag = False
        self._jcpsapi = api

    def add_new_msg(self, msg_type, msg):
        """
        将收到的CPS消息放入处理队列
        :param msg_type: 消息类型，int
        :param msg: 消息内容，tuple of (from_id, msg_type, data, msg_len)
        :return: None
        """
        self._queue.put((msg_type, msg))

    def run(self):
        while not self._stop_flag:
            if self._queue.empty():
                time.sleep(0.1)
                continue
            msg_type, msg = self._queue.get()
            # check if msg is sync send response
            tid = self._jcpsapi._get_tid_from_wait_msg_type(msg_type)
            if tid:
                self._jcpsapi._wait_msg_arrived(tid, msg)
            else:
                # cache to queue
                self._msg_cache.put(msg)
            self._queue.task_done()

    def recv_msg(self):
        """
        从缓存取出新消息
        :return: 消息内容，tuple of (from_id, msg_type, data, msg_len); 如果没有，则返回None
        """
        if self._msg_cache.empty():
            return None
        msg = self._msg_cache.get()
        self._msg_cache.task_done()
        return msg

    def stop(self):
        self._stop_flag = True
        self.join()


class JCPSAPIHandler(pycpsapi.CCPSEventHandler):
    """
    CPS事件响应类
    """

    def __init__(self, api):
        super(JCPSAPIHandler, self).__init__()
        self._is_connected = False
        self._api = api

    def OnConnected(self) -> "void":
        self._is_connected = True
        self._api.register_device()
        LOG.info('Connected to Server!')

    def OnDisconnected(self) -> "void":
        self._is_connected = False
        LOG.info('Diconnected from Server!')

    def OnMsg(self, from_id: "uint32_t", msg_type: "uint32_t", data: "char const *", msg_len: "uint32_t") -> "void":
        LOG.info('received msg from {}, msg_type={}, len={}'.format(from_id, msg_type, msg_len))
        self._api._on_msg(from_id, msg_type, data, msg_len)

    @property
    def connected(self):
        return self._is_connected


JCPS_API_TYPE_DEVICE = pycpsapi.E_CPS_TYPE_DEVICE
JCPS_API_TYPE_APP = pycpsapi.E_CPS_TYPE_APP
JCPS_API_TYPE_MIX = pycpsapi.E_CPS_TYPE_MIX


def _check_connection(func):
    @wraps(func)
    def inner(*args, **kwargs):
        cls = args[0]
        if not cls._msg_handler.connected:
            LOG.error('function {} failed, api not connected!'.format(func.__name__))
            return False
        return func(*args, **kwargs)

    return inner


class JCPSApi(object):
    """
    CPS python接口类
    """

    def __init__(self):
        super(JCPSApi, self).__init__()
        self._api_type = None
        self._dev_id = None
        self._app_id = None
        self._server_ip = None
        self._server_port = None
        self._log_ip = None
        self._log_port = None
        self._msg_handler = JCPSAPIHandler(self)
        self._api = pycpsapi.CCPSAPI_CreateAPI()

        self._current_wait_msg_type = dict()
        self._msg = dict()
        self._msg_cond = defaultdict(threading.Condition)

        self._msg_thread = JCPSMsgThread(self)
        self._msg_thread.start()

    def api_version(self):
        """
        获取API版本信息
        :return: 版本信息，str类型
        """
        return self._api.GetApiVersion().decode()

    def initialize(self, api_type, dev_id, app_id, server_ip, server_port,
                   log_ip='127.0.0.1', log_port=4888, timeout_in_sec=3.0):
        """
        初始化CPSAPI
        :param api_type:
        :param dev_id:
        :param app_id:
        :param server_ip:
        :param server_port:
        :param log_ip:
        :param log_port:
        :param timeout_in_sec:
        :return: 成功返回True，失败返回False
        """
        self._api_type = api_type
        self._dev_id = dev_id
        self._app_id = app_id
        self._server_ip = server_ip.encode()
        self._server_port = server_port
        self._log_ip = log_ip.encode()
        self._log_port = log_port

        if self._api.Init(self._api_type, self._dev_id, self._app_id, self._server_ip, self._server_port,
                          self._log_ip, self._log_port, self._msg_handler) != 0:
            LOG.error('failed to initialize jcpsapi!')
            return False
        # wait for connected
        delta = 0.1
        while timeout_in_sec > 0:
            if self._msg_handler.connected:
                break
            time.sleep(delta)
            timeout_in_sec -= delta
        else:
            LOG.error('connect cps server timeout!')
            self._api.Release()
            return False
        return True

    def release(self):
        """
        释放API资源
        :return: None
        """
        self._api.Release()
        self._msg_thread.stop()

    def _get_tid_from_wait_msg_type(self, msg_type):
        for tid, _msg_type in self._current_wait_msg_type.items():
            if _msg_type == msg_type:
                return tid
        return None

    def _on_msg(self, from_id, msg_type, data, msg_len):
        self._msg_thread.add_new_msg(msg_type, (from_id, msg_type, data, msg_len))

    @_check_connection
    def register_device(self):
        # check api type
        if self._api_type != JCPS_API_TYPE_DEVICE and self._api_type != JCPS_API_TYPE_MIX:
            return False
        self._api.RegisterDevice()

    @_check_connection
    def register_app(self, dev_ids):
        # check api type
        if self._api_type != JCPS_API_TYPE_APP and self._api_type != JCPS_API_TYPE_MIX:
            LOG.error('you can not register app!')
            return False
        for dev_id in dev_ids:
            self._api.SubscribeDevice(dev_id)

    def _wait_msg_arrived(self, tid, msg):
        self._msg_cond[tid].acquire()
        self._msg[tid] = msg
        self._msg_cond[tid].notify()
        self._msg_cond[tid].release()

    @_check_connection
    def async_send_json_msg_to_bus(self, msg_type, json_data):
        """
        异步发送消息给总线
        :param msg_type: 消息类型
        :param json_data: json格式数据
        :return: 成功返回True，失败返回False
        """
        if not isinstance(msg_type, int):
            LOG.error('msg_type should be int type!')
            return False

        if not isinstance(json_data, dict):
            LOG.error('json_data should be dict type!')
            return False

        json_str = json.dumps(json_data, ensure_ascii=False).encode('gbk')
        json_str_len = len(json_str) + 1
        LOG.info('send msg to bus, msg_type={}, msg_len={}'.format(msg_type, json_str_len))

        if self._api.SendMsg(msg_type, json_str, json_str_len) != 0:
            return False
        return True

    @_check_connection
    def async_send_json_msg_to_device(self, dev_id, msg_type, json_data):
        """
        异步发送消息给指定设备
        :param dev_id: 设备ID
        :param msg_type: 消息类型
        :param json_data: json格式数据
        :return: 成功返回True，失败返回False
        """
        # check api type
        if self._api_type != JCPS_API_TYPE_APP and self._api_type != JCPS_API_TYPE_MIX:
            LOG.error('you can not send msg to device!')
            return False

        # check params
        if not isinstance(dev_id, int):
            LOG.error('dev_id should be int type!')
            return False

        if not isinstance(msg_type, int):
            LOG.error('msg_type should be int type!')
            return False

        if not isinstance(json_data, dict):
            LOG.error('json_data should be dict type!')
            return False

        json_str = json.dumps(json_data, ensure_ascii=False).encode('gbk')
        json_str_len = len(json_str) + 1
        LOG.info('send msg to device {}, msg_type={}, msg_len={}'.format(dev_id, msg_type, json_str_len))

        if self._api.SendAPPMsg(dev_id, msg_type, json_str, json_str_len) != 0:
            return False
        return True

    @_check_connection
    def async_send_json_msg_to_app(self, app_id, msg_type, json_data):
        """
        异步发送消息给指定APP
        :param app_id: APP ID
        :param msg_type: 消息类型
        :param json_data: json格式数据
        :return: 成功返回True，失败返回False
        """
        # check api type
        if self._api_type != JCPS_API_TYPE_DEVICE and self._api_type != JCPS_API_TYPE_MIX:
            LOG.error('you can not send msg to app!')
            return False

        # check params
        if not isinstance(app_id, int):
            LOG.error('app_id should be int type!')
            return False

        if not isinstance(msg_type, int):
            LOG.error('msg_type should be int type!')
            return False

        if not isinstance(json_data, dict):
            LOG.error('json_data should be dict type!')
            return False

        json_str = json.dumps(json_data, ensure_ascii=False).encode('gbk')
        json_str_len = len(json_str) + 1
        LOG.info('send msg to app {}, msg_type={}, msg_len={}'.format(app_id, msg_type, json_str_len))

        if self._api.SendDeviceMsg(app_id, msg_type, json_str, json_str_len) != 0:
            return False
        return True

    def sync_send_json_msg_to_device(self, dev_id, msg_type, json_data, timeout_in_sec=3.0):
        """
        同步发送消息给指定设备
        :param dev_id: 设备ID
        :param msg_type: 消息类型
        :param json_data: json格式数据
        :param timeout_in_sec: 超时时间
        :return: 返回from_id, msg_type, msg，超时时返回值为None
        """
        tid = threading.current_thread().ident
        self._msg_cond[tid].acquire()
        self._current_wait_msg_type[tid] = msg_type
        self.async_send_json_msg_to_device(dev_id, msg_type, json_data)
        if not self._msg_cond[tid].wait(timeout_in_sec):
            # timeout
            self._msg_cond[tid].release()
            return None, None, None
        # get result
        from_id, msg_type, msg, _ = self._msg[tid]
        self._msg_cond[tid].release()
        msg = json.loads(msg.decode('gbk'))
        return from_id, msg_type, msg

    def sync_send_json_msg_to_app(self, app_id, msg_type, json_data, timeout_in_sec=3.0):
        """
        同步发送消息给指定APP
        :param app_id: APP ID
        :param msg_type: 消息类型
        :param json_data: json格式数据
        :param timeout_in_sec: 超时时间
        :return: 返回from_id, msg_type, msg，超时时返回值为None
        """
        tid = threading.current_thread().ident
        self._msg_cond[tid].acquire()
        self._current_wait_msg_type[tid] = msg_type
        self.async_send_json_msg_to_app(app_id, msg_type, json_data)
        if not self._msg_cond[tid].wait(timeout_in_sec):
            # timeout
            self._msg_cond[tid].release()
            return None, None, None
        # get result
        from_id, msg_type, msg, _ = self._msg[tid]
        self._msg_cond[tid].release()
        msg = json.loads(msg.decode('gbk'))
        return from_id, msg_type, msg

    def sync_send_json_msg_to_bus(self, msg_type, json_data, timeout_in_sec=3.0):
        """
        同步发送消息给总线
        :param msg_type: 消息类型
        :param json_data: json格式数据
        :param timeout_in_sec: 超时时间
        :return: 返回from_id, msg_type, msg，超时时返回值为None
        """
        tid = threading.current_thread().ident
        self._msg_cond[tid].acquire()
        self._current_wait_msg_type[tid] = msg_type
        self.async_send_json_msg_to_bus(msg_type, json_data)
        if not self._msg_cond[tid].wait(timeout_in_sec):
            # timeout
            self._msg_cond[tid].release()
            return None, None, None
        # get result
        from_id, msg_type, msg, _ = self._msg[tid]
        self._msg_cond[tid].release()
        msg = json.loads(msg.decode('gbk'))
        return from_id, msg_type, msg

    def broadcast_device_msg(self, msg_type, json_data):
        """
        广播设备数据，所有订阅该设备的APP均可收到此数据
        :param msg_type: 消息类型
        :param json_data: json格式数据
        :return: 成功返回True，失败返回False
        """
        return self.async_send_json_msg_to_app(-1, msg_type, json_data)

    def recv_msg(self):
        """
        从缓存取出新消息
        :return: 消息内容，tuple of (from_id, msg_type, json_data, msg_len); 如果没有，则返回None
        """
        msg = self._msg_thread.recv_msg()
        if msg:
            from_id, msg_type, msg, msg_len = msg
            try:
                json_data = json.loads(msg[:msg_len].decode('gbk'))
            except:
                json_data = {}
            return from_id, msg_type, json_data, msg_len
        else:
            return None
