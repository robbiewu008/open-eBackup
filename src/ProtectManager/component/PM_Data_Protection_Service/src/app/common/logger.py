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
import glob
import gzip
import logging
import logging.config
import os
import threading
import time
import re
from logging.handlers import RotatingFileHandler

from uvicorn.logging import ColourizedFormatter
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler
from watchdog.utils.dirsnapshot import DirectorySnapshot, DirectorySnapshotDiff

from app.common.sensitive.log_filter import LogFilter
from app.conf.sensitive_rule_config import config

log = logging.getLogger(__name__)
LOG_LEVEL = {10: "DEBUG", 20: "INFO", 30: "WARN", 40: "ERROR"}
LOG_FUNC = {"DEBUG": log.debug, "INFO": log.info,
            "WARN": log.warning, "ERROR": log.error}
MONITOR_FILE_PATH = os.path.abspath(os.getenv("MONITOR_LOG_PATH", "/opt/config/loglevel"))
CRLF_PATTERN = r"[\x00-\x1F\x7f\r\n\b\t\f\v]"  # 现有的控制字符
# 启动服务时，设置log level超时时间，20*10s为pod就绪探针超时时间
READINESS_PROBE = 20 * 10
# 10份
LOG_FILES_COUNT = 10
# 30MB
LOG_FILES_MAX_BYTES = 1024 * 1024 * 30


class CrlfFormatter(ColourizedFormatter):
    def formatMessage(self, record):
        message = super(CrlfFormatter, self).formatMessage(record)

        if re.search(CRLF_PATTERN, message):
            message = re.sub(CRLF_PATTERN, ' ', message)
        return message


class CompressRotatingFileHandler(RotatingFileHandler):
    def doRollover(self):
        """
        Do a rollover, as described in __init__().
        """
        if self.stream:
            self.stream.close()
            self.stream = None
        if self.backupCount > 0:
            for i in range(self.backupCount - 1, 0, -1):
                self.rotate(i)
            dfn = self.rotation_filename(
                self.baseFilename + ".1" + '-' + time.strftime("%Y%m%d.%H%M%S", time.localtime(time.time())) + ".gz")
            if os.path.exists(dfn):
                os.remove(dfn)
            with open(self.baseFilename, "rb") as sf:
                with gzip.open(dfn, "wb") as df:
                    df.writelines(sf)
            os.remove(self.baseFilename)
        if not self.delay:
            self.stream = self._open()

    def rotate(self, i):
        sfn = self.rotation_filename("%s.%d" % (self.baseFilename, i))
        dfn = self.rotation_filename("%s.%d" % (self.baseFilename, i + 1))
        sfn_list = glob.glob(f'{sfn}-*.gz')
        if sfn_list:
            sfn_exist = sfn_list[0]
        dfn_list = glob.glob(f'{dfn}-*.gz')
        if dfn_list:
            dfn_exist = dfn_list[0]
        if sfn_list:
            if dfn_list:
                os.remove(dfn_exist)
            os.rename(sfn_exist, dfn + "-" + sfn_exist.rsplit("-", 1)[1])


class LoggerConfig:

    def __init__(self, application, log_path):
        self._application = application
        self._log_path = log_path

    def get_config(self):
        dirs = f'{self._log_path}/{self._application}'

        if not os.path.exists(dirs):
            os.makedirs(dirs)

        return {
            "version": 1,
            "disable_existing_loggers": False,
            "formatters": {
                "default": {
                    "()": "app.common.logger.CrlfFormatter",
                    "fmt": "[%(asctime)s][%(levelname)s][%(message)s]"
                           "[%(filename)s, %(funcName)s:%(lineno)d][%(threadName)s]",
                    "use_colors": True,
                },
                "access": {
                    "()": "app.common.logger.CrlfFormatter",
                    "fmt": '[%(asctime)s][%(levelprefix)s][%(client_addr)s]["%(request_line)s"][%(status_code)s]',
                },
                "detail": {
                    "()": "app.common.logger.CrlfFormatter",
                    "fmt": "[%(asctime)s][%(levelname)s][%(message)s]"
                           "[%(filename)s, %(funcName)s:%(lineno)d][%(threadName)s]",
                    "use_colors": None,
                },
            },
            "handlers": {
                "default": {
                    "formatter": "default",
                    "class": "logging.StreamHandler",
                    "stream": "ext://sys.stderr",
                },
                "access": {
                    "formatter": "detail",
                    "class": "logging.StreamHandler",
                    "stream": "ext://sys.stdout",
                },
                "info_file": {
                    "formatter": "detail",
                    "class": "app.common.logger.CompressRotatingFileHandler",
                    "filename": f"{dirs}/all.log",
                    "level": "INFO",
                    "encoding": "utf-8",
                    "maxBytes": LOG_FILES_MAX_BYTES,
                    "backupCount": LOG_FILES_COUNT
                },
            },
            "loggers": {
                "uvicorn": {"handlers": ["default", "info_file"], "level": "WARN"},
                "uvicorn.error": {"handlers": ["info_file"], "level": "WARN"},
                "uvicorn.access": {"handlers": ["access", "info_file"], "level": "WARN", "propagate": False},
                "root": {"handlers": ["access", "info_file"], "level": "ERROR", "propagate": False},
                "common": {"handlers": ["access", "info_file"], "level": "DEBUG", "propagate": False},
                "app": {"handlers": ["access", "info_file"], "level": "DEBUG", "propagate": False},
            },
        }


def get_logger(name):
    logger = logging.getLogger(name)
    if config.filter_enable():
        logger.addFilter(LogFilter())
    return logger


class FileEventHandler(FileSystemEventHandler):
    handlers = ("default", "access", "info_file")

    def __init__(self):
        super().__init__()
        self.snapshot = DirectorySnapshot(MONITOR_FILE_PATH)

    def on_created(self, event):
        if event.is_directory:
            snapshot = DirectorySnapshot(MONITOR_FILE_PATH)
            diff = DirectorySnapshotDiff(self.snapshot, snapshot)
            if diff.files_created:
                self.snapshot = snapshot
                self.set_log_level()

    @classmethod
    def set_log_level(cls):
        if os.path.isfile(MONITOR_FILE_PATH):
            with open(MONITOR_FILE_PATH, 'r', encoding="utf-8") as fi:
                file_log_level = str(fi.read())

            info_file_handler = logging._handlers.get(cls.handlers[-1])
            if not info_file_handler:
                log.error("Can't get handler")
                return None

            if file_log_level not in LOG_LEVEL.values():
                log.error(f"The target log level({file_log_level}) is invalid.")
                return
            if file_log_level != LOG_LEVEL.get(info_file_handler.level, "NOTSET"):
                log.info(f"Starting to set log level to {file_log_level}.")
                for handler in cls.handlers:
                    logging._handlers.get(handler, log).setLevel(file_log_level)
                log_func = LOG_FUNC.get(file_log_level, logging.info)
                log_func(f"Set log level to {file_log_level} success.")
            else:
                log.debug(f"The target log level({file_log_level}) is same as the current log level.")
        else:
            log.error(f"The log level configuration file({MONITOR_FILE_PATH}) does not exist.")


def monitor_file():
    times = 0
    while times < READINESS_PROBE:
        if logging._handlers.get("info_file", None):
            FileEventHandler.set_log_level()
            break
        else:
            times += 1
            time.sleep(1)

    while True:
        path = os.path.dirname(MONITOR_FILE_PATH)
        event_handler = FileEventHandler()
        observer = Observer()
        observer.schedule(event_handler, path, recursive=True)
        observer.start()
        observer.join()


def create_daemon_thread():
    try:
        thread = threading.Thread(target=monitor_file)
        thread.setDaemon(True)
        thread.start()
    except threading.ThreadError as th_err:
        log.error(f"Start monitor file thread failed: {th_err}")


create_daemon_thread()
