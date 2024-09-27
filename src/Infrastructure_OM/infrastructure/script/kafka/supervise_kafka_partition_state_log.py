#!/usr/bin/env python
# _*_ coding:utf-8 _*_

import select
import os
import subprocess
from pathlib import Path

from common.logger.logger import get_logger
from kafka_consts import KAFKA_STATE_TXT_PATH, PARTITION_STATE_CHANGE_FAILED_MSG, CLIENT_SHUTDOWN_MSG, \
    INVALID_REPLICATION_FACTOR_MSG, INVALID_REPLICATION_FACTOR_VAL_MSG

node_name = os.getenv('NODE_NAME')
server_log_file = os.path.join(os.sep, 'opt', 'OceanProtect', 'logs', node_name,
                               'infrastructure', 'kafka', 'server.log')
controller_log_file = os.path.join(os.sep, 'opt', 'OceanProtect', 'logs', node_name,
                                   'infrastructure', 'kafka', 'controller.log')
install_log_file = os.path.join(os.sep, 'opt', 'OceanProtect', 'logs', node_name,
                                'infrastructure', 'kafka', 'install_kafka.log')
logger = get_logger(install_log_file)

log_dict = {
    'server': {
        'in_dir': server_log_file,
        'msg': [PARTITION_STATE_CHANGE_FAILED_MSG, INVALID_REPLICATION_FACTOR_MSG, INVALID_REPLICATION_FACTOR_VAL_MSG],
    },
    'controller': {
        'in_dir': controller_log_file,
        'msg': [CLIENT_SHUTDOWN_MSG],
    },
}


def check_kafka_log_contains_text(log_txt):
    """
    检查给定字符串是否存在日志关键字
    :param log_type: 待检查的日志类型
    :param log_txt: 待检查的字符串
    :return: 检查到特殊关键字，生成txt文件，待容器重启
    """
    for log_type in log_dict.keys():
        for msg in log_dict.get(log_type).get('msg'):
            if msg in log_txt:
                logger.error(f"{log_type} log file :{log_dict.get(log_type).get('in_dir')} "
                             f"has err msg: {msg}. log_txt:{log_txt}")
                Path(KAFKA_STATE_TXT_PATH).touch()
                os._exit(1)


def watch_kafka_state():
    """
    使用tail -F 获取日志文件最新输出， -F 表示跟踪文件名，文件不存在不报错
    :return:
    """
    cmd_server = ['tail', '-F', '-n', '0', log_dict.get('server').get('in_dir')]
    cmd_controller = ['tail', '-F', '-n', '0', log_dict.get('controller').get('in_dir')]
    pro_server = subprocess.Popen(cmd_server, shell=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                  start_new_session=True, encoding="utf-8")
    pro_controller = subprocess.Popen(cmd_controller, shell=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                      start_new_session=True, encoding="utf-8")
    while True:
        stdouts, _, _ = select.select([pro_server.stdout, pro_controller.stdout], [], [])

        for stdout in stdouts:
            log_txt = stdout.readline()
            if log_txt:
                check_kafka_log_contains_text(log_txt)

        if pro_server.poll() is not None and pro_controller.poll() is not None:
            break


if __name__ == "__main__":
    try:
        watch_kafka_state()
    except Exception as e:
        logger.exception(f'supervise kafka log failed! Exception:{e}')
        os._exit(1)
