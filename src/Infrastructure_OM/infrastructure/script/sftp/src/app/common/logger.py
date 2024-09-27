import gzip
import logging
import os
import re
import time
from logging.handlers import RotatingFileHandler

NODE_NAME = os.getenv("NODE_NAME")

PATH = f"/opt/OceanProtect/logs/{NODE_NAME}/infrastructure/sftp/"
MAX_LOG_FILE_NUM = 20
os.umask(0o027)


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
    os.chown(dest, 15004, 99)
    os.chmod(dest, 0o440)
    os.remove(source)
    # 转储文件只保留20份，超过的按时间顺序删除
    file_dir = os.path.split(PATH)[0]
    file_list = get_all_path(file_dir)
    if len(file_list) > MAX_LOG_FILE_NUM:
        files = sorted(file_list, key=lambda x: os.path.getmtime(x))
        os.remove(files[0])
    return dest


class ChangeOwnerRotatingFileHandler(logging.handlers.RotatingFileHandler):
    def doRollover(self):
        """
        Override base class method to chown the new log file.
        """
        # Rotate the file first.
        logging.handlers.RotatingFileHandler.doRollover(self)

        # Change the owner of the current file.
        os.chown(self.baseFilename, 15004, 99)


class CrlfFormatter(logging.Formatter):
    def formatMessage(self, record):
        message = super(CrlfFormatter, self).formatMessage(record)
        message = re.sub('[\r\n\b]', ' ', message)
        return message


class Logger:

    def __init__(self):
        self.formatter_str = "[%(asctime)s][%(levelname)s][%(message)s][%(funcName)s][%(process)d][%(thread)d]" \
                             "[%(name)s][%(threadName)s][%(filename)s][%(lineno)s]"
        if not os.path.isdir(PATH):
            os.makedirs(PATH)
        filename = os.path.join(PATH, "sftp.log")
        if not os.path.exists(filename):
            os.mknod(filename, mode=0o640)
        os.chown(filename, 15004, 99)
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


log = Logger().get_logger()
