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
import re
import threading
import time
import stat
import configparser
from logging.handlers import RotatingFileHandler
from common.security.anonym_utils.anonymity import Anonymity
from common.env_common import get_install_head_path

PATH = os.getenv("GENERALDB_LOG_PATH", "/opt/DataBackup/ProtectClient/ProtectClient-E/slog/GeneralDBPlugin/log")
MAX_LOG_FILE_NUM = 20
os.umask(0o027)
LOG_LEVEL_PATH = f"{get_install_head_path()}/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/conf/hcpconf.ini"
LOG_LEVEL = ["DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL"]


"""
脱敏基本规则：
1.关键字不区分大小写
2.以下关键字作包含匹配: pass, pwd, key, crypto, session, token, fingerprint, auth, enc, dec, tgt, iqn, initiator,
                secret, cert, salt, private, user_info, verfiycode, rand, safe, PKCS1, base64, AES128, AES256,
                RSA, SHA1, SHA256, SHA384, SHA512, algorithm
3.以下关键字作全词匹配: sk, mk, iv
4.业务中有2、3场景不能覆盖的其他敏感字段, 自行加入匹配关键字中。
"""
SENSITIVE_WORDS = ["%pass%", "%pwd%", "%key%", "%crypto%", "%session%", "%token%", "%fingerprint%", "%auth%",
                    "%enc%", "%dec%", "%tgt%", "%iqn%", "%initiator%", "%secret%", "%cert%",  "%salt%", "%private%",
                    "%user_info%", "%verfiycode%", "%rand%", "%safe%", "%PKCS1%", "%base64%", "%AES128%", "%AES256%",
                    "%RSA%", "%SHA1%", "%SHA256%", "%SHA384%", "%SHA521%", "%algorithm%", "@mk", "@sk", "@iv"]


def find_first_sens_word_index(key: str, sensitive_word_list: list):
    """
    判断key是否为敏感词
    :param key: 要检查的字符串
    :param sensitive_word_list: 敏感信息关键字列表
    :return: 返回字符串中第一个的敏感字符串的索引
    """
    first_index = len(key)
    for sens_word in sensitive_word_list:
        # 全字匹配字符串等于敏感词本身，不需要脱敏,只需匹配关键字
        if sens_word.find("%") == -1:
            continue
        result = re.search(sens_word.replace("%", ""), key, re.I)
        if not result:
            continue
        if result.end() < first_index:
            first_index = result.end()
    if first_index == len(key):
        return False, ''
    return True, first_index


def process_string(input_str: str, sensitive_word_list: list):
    if input_str is None:
        return ""
    result, sens_word_index = find_first_sens_word_index(input_str, sensitive_word_list)
    if not result:
        return input_str
    return input_str[:sens_word_index] + ':******'


def get_log_level():
    if not os.path.isfile(LOG_LEVEL_PATH):
        return False, "INFO"
    read_ini = configparser.ConfigParser()
    read_ini.read(LOG_LEVEL_PATH)
    value = read_ini.get("General", "LogLevel")
    if int(value) < len(LOG_LEVEL):
        return True, LOG_LEVEL[int(value)]
    return False, "INFO"


def update_log_level():
    status, log_level = get_log_level()
    if not status:
        Logger.logger.warning('Failed to get log level.')
    update_log_level_from_conf(log_level)


def update_log_level_from_conf(log_level):
    if logging.getLevelName(log_level) != Logger.logger.level:
        Logger.set_level(log_level)
        Logger.logger.info(f'info : update log level to {log_level}')


def get_all_path(dir_path):
    """
    获取指定目录及其子目录下所有文件名列表
    :param dir_path:目录名
    :return: file_list 绝对路径的文件名列表
    """
    file_list = []
    for main_dir, subdir, file_name_list in os.walk(dir_path):
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
    os.lchown(dest, 0, 0)
    os.chmod(dest, 0o440)
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
        os.lchown(self.baseFilename, 0, 0)


class CrlfFormatter(logging.Formatter):
    def formatMessage(self, record):
        message = super(CrlfFormatter, self).formatMessage(record)
        message = re.sub('[\r\n\b\t\f\\x7f\\v]', ' ', message)
        msg_end = Anonymity.process(record.msg)
        if msg_end != record.msg:
            message = message.replace(record.msg, msg_end)
        return message


class SingletonType(type):
    _instance_lock = threading.Lock()

    def __call__(cls, *args, **kwargs):
        if not hasattr(cls, "_instance"):
            with SingletonType._instance_lock:
                if not hasattr(cls, "_instance"):
                    cls._instance = super(SingletonType, cls).__call__(*args, **kwargs)
        return cls._instance


class Logger(metaclass=SingletonType):
    logger = logging.getLogger()
    filename = None

    def __init__(self):
        self.formatter_str = "[%(asctime)s][%(levelname)s][%(message)s][%(funcName)s][0x%(process)x][0x%(thread)x]" \
                             "[%(name)s][%(filename)s][%(lineno)s]"
        if not os.path.isdir(PATH):
            os.makedirs(PATH)
        self.formatter = CrlfFormatter(self.formatter_str)
        self.logger.setLevel(logging.INFO)

    @classmethod
    def set_level(cls, log_level):
        cls.logger.setLevel(log_level)

    def get_logger(self, filename=None):
        if self.filename:
            return self.logger
        if not self.filename and filename:
            self.filename = filename
        else:
            self.filename = "database_plugin.log"
        # if not use default log name, only can set once other name
        update_log_level()
        self.set_filename()
        return self.logger

    def set_filename(self):
        filename = os.path.join(PATH, self.filename)
        flags = os.O_WRONLY | os.O_CREAT
        modes = stat.S_IWUSR | stat.S_IRUSR
        if not os.path.exists(filename):
            os.fdopen(os.open(filename, flags, modes), 'w')
        os.lchown(filename, 0, 0)
        log_filename = os.path.realpath(filename)
        file_handler = ChangeOwnerRotatingFileHandler(filename=log_filename, maxBytes=10 * 1024 * 1024,
                                                       backupCount=10)
        file_handler.rotate = rotator
        file_handler.namer = namer
        file_handler.setFormatter(self.formatter)
        self.logger.addHandler(file_handler)