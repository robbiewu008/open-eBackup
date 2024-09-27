import logging
import re
import os
from server.common.consts import LOG_PATH


class CrlfFormatter(logging.Formatter):
    def formatMessage(self, record):
        message = super(CrlfFormatter, self).formatMessage(record)
        message = re.sub('[\r\n\b]', ' ', message)
        return message


def get_logger(log_file_path: str, loglevel=logging.INFO):
    """get logger
    The log file path should be created by the caller.
    Support both stream handler and file handler.
    Do not support rotate.
    logging.getLogger() is Singleton.

    :param log_file_path:
    :param loglevel: 日志级别
    :return:
    """
    logger = logging.getLogger(log_file_path)
    logger.setLevel(loglevel)

    formatter_str = "[%(asctime)s][%(levelname)s][%(message)s][%(funcName)s][%(process)d][%(thread)d]" \
                    "[%(name)s][%(threadName)s][%(filename)s][%(lineno)s]"
    log_formatter = CrlfFormatter(formatter_str)

    # stream_handler
    stream_handler = logging.StreamHandler()
    stream_handler.setFormatter(log_formatter)

    # file_handler
    file_handler = logging.FileHandler(log_file_path, mode='a')
    file_handler.setFormatter(log_formatter)
    logger.addHandler(file_handler)

    logger.addHandler(stream_handler)

    return logger


log_file = os.path.join(LOG_PATH, 'dp_deploy', 'package_process.log')
os.makedirs(os.path.dirname(log_file), exist_ok=True)
logger = get_logger(log_file)
