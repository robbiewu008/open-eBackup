#
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
#

import gzip
import logging
import os
import threading
import time
import re
from logging.handlers import RotatingFileHandler
from common.security.anonym_utils.anonymity import Anonymity

LOG_HOME = os.getenv("DATA_BACKUP_AGENT_HOME", "").replace("\\", "/")
PATH = f"{LOG_HOME}/DataBackup/ProtectClient/ProtectClient-E/log/Plugins/GeneralDBPlugin/"
MAX_LOG_FILE_NUM = 20
os.umask(0o027)


def get_all_path(dir_path):
    """
    获取指定目录及其子目录下所有文件名列表
    :param dir_path:目录名
    :return: file_list 绝对路径的文件名列表
    """
    file_list = []
    for main_dir, file_name_list in os.walk(dir_path):
        for filename in file_name_list:
            if filename.startswith('.'):
                continue
            file_path = os.path.join(main_dir, filename)
            file_list.append(file_path)
    return file_list


def namer(name):
    return name + '-' + time.strftime("%Y%m%d.%H%M%S", time.localtime(time.time())) + ".gz"


def rotator(source, dest):
    with open(source, "rb") as sf:
        with gzip.open(dest, "wb") as df:
            df.writelines(sf)
    os.remove(source)
    # 转储文件只保留20份，超过的按时间顺序删除
    file_dir = os.path.split(PATH)[0]
    file_list = get_all_path(file_dir)
    if len(file_list) > MAX_LOG_FILE_NUM:
        files = sorted(file_list, key=lambda x: os.path.getmtime(x))
        os.remove(files[0])
    return dest


class ChangeOwnerRotatingFileHandler(RotatingFileHandler):
    def doRollover(self):
        """
        Override base class method to chown the new log file.
        """
        # Rotate the file first.
        logging.handlers.RotatingFileHandler.doRollover(self)

        # Change the owner of the current file.
        os.lchown(self.baseFilename, 99, 99)


class SingletonType(type):
    _instance_lock = threading.Lock()

    def __call__(cls, *args, **kwargs):
        if not hasattr(cls, "_instance"):
            with SingletonType._instance_lock:
                if not hasattr(cls, "_instance"):
                    cls._instance = super(SingletonType, cls).__call__(*args, **kwargs)
        return cls._instance


class CrlfFormatter(logging.Formatter):
    def formatMessage(self, record):
        message = super(CrlfFormatter, self).formatMessage(record)
        message = re.sub('[\r\n\b]', ' ', message)
        security_message = Anonymity.process(message)
        if security_message != message:
            message = message.replace(record.msg, "******")
        return message


class Logger(metaclass=SingletonType):

    def __init__(self, application="general_db_wins.log"):
        self.formatter_str = "[%(asctime)s][%(levelname)s][%(message)s][%(funcName)s][%(process)d][%(thread)d]" \
                             "[%(name)s][%(threadName)s][%(filename)s][%(lineno)s]"
        if not os.path.isdir(PATH):
            os.makedirs(PATH)
        filename = os.path.join(PATH, application)
        self.filename = os.path.realpath(filename)
        self.file_handler = ChangeOwnerRotatingFileHandler(filename=self.filename, maxBytes=10 * 1024 * 1024,
                                                           backupCount=10)
        self.stream_handler = logging.StreamHandler()
        self.file_handler.rotate = rotator
        self.file_handler.namer = namer
        _formatter = CrlfFormatter(self.formatter_str)
        self.stream_handler.setFormatter(_formatter)
        self.file_handler.setFormatter(_formatter)

    def get_logger(self):
        logger = logging.getLogger(self.filename)
        logger.setLevel(logging.INFO)
        logger.addHandler(self.stream_handler)
        logger.addHandler(self.file_handler)
        return logger
