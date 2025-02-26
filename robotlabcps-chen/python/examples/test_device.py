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
    # create CPSAPI
    api = jcpsapi.JCPSApi()
    print('API Version: {}'.format(api.api_version()))
    t = MsgHandler(api)
    t.start()

    # initialize CPSAPI
    api.initialize(jcpsapi.JCPS_API_TYPE_DEVICE, PYTHON_TEST_DEVICE_ID, 0, '127.0.0.1', 4000, '127.0.0.1', 4888)

    value = 0
    msg_type_test = 1
    while True:
        # json_data = {'name': 'x', 'value': value}
        # api.broadcast_device_msg(msg_type_test, json_data)
        # value += 1
        time.sleep(1)
        # if value == 10:
        #     break

    api.release()


if __name__ == '__main__':
    main()
