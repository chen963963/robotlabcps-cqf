import time
import threading
import sys
sys.path.append(r'D:\ZhangChen\robotlabcps\python')

from pyjcpsapi import jcpsapi

PYTHON_TEST_DEVICE_ID = 257


class MsgHandler(threading.Thread):
    def __init__(self, api):
        super(MsgHandler, self).__init__()
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
        self._api.async_send_json_msg_to_app(from_id, msg_type, json_data)


def main():
    api = jcpsapi.JCPSApi()
    print('API Version: {}'.format(api.api_version()))

    if not api.initialize(jcpsapi.JCPS_API_TYPE_APP, 0, 5000, '127.0.0.1', 4000, '127.0.0.1', 4888):
        return
    api.register_app([PYTHON_TEST_DEVICE_ID])

    while True:
        json_data = {'value': 3.14}
        response = api.sync_send_json_msg_to_device(PYTHON_TEST_DEVICE_ID, 1, json_data)
        print(response)
        time.sleep(1)

    api.release()


if __name__ == '__main__':
    main()
