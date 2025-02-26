# -*- coding: utf-8 -*-
"""
Created by sunddy at 2019/4/17 21:51
"""
import time
import logging

LOG_FORMAT = '%(asctime)s - %(thread)d - %(levelname)s - %(name)s - %(message)s'
DATE_FORMAT = '%Y-%m-%d %H:%M:%S'

logging.basicConfig(level=logging.INFO, format=LOG_FORMAT, datefmt=DATE_FORMAT)


def create_logger(name):
    import os
    os.makedirs('./log', exist_ok=True)
    LOG_FILE = "./log/{}_{}.log".format(name, time.strftime("%Y%m%d%H%M%S"))
    # 日志对象
    handler = logging.FileHandler(LOG_FILE)
    handler.setLevel(logging.INFO)
    formatter = logging.Formatter(LOG_FORMAT)
    handler.setFormatter(formatter)
    logger = logging.getLogger(name)
    logger.addHandler(handler)
    return logger


JLOGGER = create_logger('JCPS')
