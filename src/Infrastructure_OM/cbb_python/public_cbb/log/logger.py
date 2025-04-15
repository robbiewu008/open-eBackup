# This file is a part of the open-eBackup project.
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file, You can obtain one at
# http://mozilla.org/MPL/2.0/.
#
# Copyright (c) [2024] Huawei Technologies Co.,Ltd.
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
#!/usr/bin/env python
# _*_ coding:utf-8 _*_

import gzip
import os
import logging
import threading
import time
import re
from logging.handlers import RotatingFileHandler

# no use public_cbb.config to avoid cyclic importing
LOG_LEVEL_PATH = "/opt/common-conf/loglevel"
MAX_LOG_FILE_NUM = 10  # 转储数量修改 10 个
MAX_LOG_BYTES = 52428800  # 50MB, 50 * 1024 * 1024


def get_logger():
    return Logger().logger


def get_log_level():
    if not os.path.isfile(LOG_LEVEL_PATH):
        return False, 'INFO'
    with open(LOG_LEVEL_PATH, 'r') as f:
        level = f.read().strip('\n\"')
        return True, level


def update_log_level():
    status, log_level = get_log_level()
    if not status:
        Logger.logger.warning('Failed to get log level.')
    else:
        update_log_level_from_conf(log_level)
    timer = threading.Timer(60, update_log_level)
    timer.start()


def update_log_level_from_conf(log_level):
    if logging.getLevelName(log_level) != Logger.logger.level:
        Logger.set_level(log_level)
        Logger.logger.info('info : update log level to %s', log_level)


threading.Timer(60, update_log_level).start()


class SingletonType(type):
    _instance_lock = threading.Lock()

    def __call__(cls, *args, **kwargs):
        if not hasattr(cls, "_instance"):
            with SingletonType._instance_lock:
                if not hasattr(cls, "_instance"):
                    cls._instance = super(SingletonType, cls).__call__(*args, **kwargs)
        return cls._instance


def get_all_path(dir_path):
    """
    获取指定目录及其子目录下所有文件名列表
    :param dir_path:目录名
    :return: file_list 绝对路径的文件名列表
    """
    file_list = []
    for main_dir, _, file_name_list in os.walk(dir_path):
        for filename in file_name_list:
            if filename.startswith('.'):
                continue
            file_path = os.path.join(main_dir, filename)
            file_list.append(file_path)
    return file_list


def namer(name):
    return name + '-' + time.strftime("%Y%m%d.%H%M%S", time.localtime(time.time())) + ".gz"


def get_log_folder_path():
    service_name = os.getenv('MICRO_SERVICE_NAME', 'dme')
    node_name = os.getenv('NODE_NAME', 'node-0')
    subsystem_name = os.getenv('SUB_SYSTEM_NAME', 'protectengine-e')
    log_folder_path = f"/opt/OceanProtect/logs/{node_name}/{subsystem_name}/{service_name}"
    return log_folder_path


def rotator(source, dest):
    with open(source, "rb") as sf:
        with gzip.open(dest, "wb") as df:
            df.writelines(sf)
    os.remove(source)
    # 转储文件只保留20份，超过的按时间顺序删除
    file_dir = get_log_folder_path()
    file_list = get_all_path(file_dir)
    if len(file_list) > MAX_LOG_FILE_NUM:
        files = sorted(file_list, key=lambda x: os.path.getmtime(x))
        os.remove(files[0])
    return dest


def get_file_handler():
    service_name = os.getenv('MICRO_SERVICE_NAME', 'dme')
    node_name = os.getenv('NODE_NAME', 'node-0')
    subsystem_name = os.getenv('SUB_SYSTEM_NAME', 'protectengine-e')
    log_folder_path = f"/opt/OceanProtect/logs/{node_name}/{subsystem_name}/{service_name}"
    if not os.path.exists(log_folder_path):
        os.makedirs(log_folder_path)
    file_path = f'{log_folder_path}/{service_name}.log'
    file_handler = RotatingFileHandler(filename=file_path, maxBytes=MAX_LOG_BYTES,
                                       backupCount=MAX_LOG_FILE_NUM)
    file_handler.rotate = rotator
    file_handler.namer = namer
    return file_handler


class CrlfFormatter(logging.Formatter):
    def formatMessage(self, record):
        message = super(CrlfFormatter, self).formatMessage(record)
        message = re.sub('[\b\n\r\v\f\x7f]', ' ', message)
        return message


class Logger(metaclass=SingletonType):
    logger = logging.getLogger()

    def __init__(self):
        self.formatter_str = "[%(asctime)s][%(levelname)s][%(message)s][%(funcName)s][0x%(process)x][0x%(thread)x]" \
                             "[%(name)s][%(filename)s][%(lineno)s]"
        self.logger.setLevel(logging.INFO)
        stream_handler = logging.StreamHandler()
        _formatter = CrlfFormatter(self.formatter_str)
        stream_handler.setFormatter(_formatter)
        file_handler = get_file_handler()
        file_handler.setFormatter(_formatter)
        self.logger.addHandler(stream_handler)
        self.logger.addHandler(file_handler)
        stream_handler.close()
        file_handler.close()

    @classmethod
    def set_level(cls, log_level):
        cls.logger.setLevel(log_level)
