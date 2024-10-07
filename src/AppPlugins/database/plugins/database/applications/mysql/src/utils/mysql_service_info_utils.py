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

import os
import time

from mysql import log
from common.common import execute_cmd_list, execute_cmd, execute_cmd_list_communicate
from mysql.src.common.constant import MYSQL_SERVICE_LIST, ExecCmdResult, SystemServiceType, MySQLClusterType, \
    MySQLStrConstant
from mysql.src.common.execute_cmd import is_port_in_use


class MysqlServiceInfoUtil:
    @staticmethod
    def get_mysql_service_info(cluster_type=None):
        """
        获取数据库服务信息，包含是否running、服务名称、服务注册的系统服务名称。
        注意：service可以是重定向到systemctl场景，所以执行service可以获取到的systemctl的回显信息
        service场景：
        SUCCESS! MariaDB running (9336)
        ERROR! MariaDB is not running (9336)
        systemctl场景：
        mysqld.service - MySQL Server
            #    Loaded: loaded (/usr/lib/systemd/system/mysqld.service; enabled; vendor preset: disabled)
            #    Active: active (running) since Wed 2023-08-23 14:07:04 CST; 1h 40min ago
            #   Process: 14644 ExecStart=/usr/sbin/mysqld --daemonize --pid-file=
            #   /var/run/mysqld/mysqld.pid $MYSQLD_OPTS (code=exited, status=0/SUCCESS)
        mysqld.service - MySQL Server
            #    Loaded: loaded (/usr/lib/systemd/system/mysqld.service; enabled; vendor preset: disabled)
            #    Active: active (exited) since Wed 2023-08-23 14:07:04 CST; 1h 40min ago
            #   Process: 14644 ExecStart=/usr/sbin/mysqld --daemonize --pid-file=
            #   /var/run/mysqld/mysqld.pid $MYSQLD_OPTS (code=exited, status=13/FAILED)
        :return:
        """
        if cluster_type == MySQLClusterType.EAPP:
            return True, SystemServiceType.SYSTEMCTL, MySQLStrConstant.EAPPMYSQLSERVICES
        for mysql_service in MYSQL_SERVICE_LIST:
            try:
                ret, _, _ = execute_cmd_list([f"systemctl status {mysql_service}", "grep \"Active: active (running)\""],
                                             True)
                if ret == ExecCmdResult.SUCCESS:
                    log.info(f"Mysql systemctl {mysql_service} running.")
                    return True, SystemServiceType.SYSTEMCTL, mysql_service,
            except Exception as exception_str:
                log.info(f"get_mysql_service_info exception_str:{exception_str}")
            service_ret, success_out, error_out = execute_cmd(f"service {mysql_service} status")
            log.info(success_out)
            if service_ret != ExecCmdResult.SUCCESS:
                log.error(f"Get service {mysql_service} status error. error out: {error_out}")
                continue
            # 命令执行成功，根据回显的字符串，判断是否有这个服务，如果有这个服务则判断这个服务是否处于running状态
            if success_out.find("could not be found") != -1:
                log.error(f"Get service {mysql_service} status error.service is not be found. out: {success_out}")
                continue
            if success_out.find("SUCCESS!") != -1 or success_out.find("running") != -1:
                log.info(f"Get service {mysql_service} status is running. out: {success_out}")
                return True, SystemServiceType.SERVICE, mysql_service
            if success_out.find("0/SUCCESS") != -1 and success_out.find("active (running)") != -1:
                log.info(f"Get service {mysql_service} status is running. out: {success_out}")
                return True, SystemServiceType.SERVICE, mysql_service
        return False, "", ""

    @staticmethod
    def wait_mysql_running(port):
        for _ in range(30):
            time.sleep(10)
            ret = MysqlServiceInfoUtil.check_mysql_is_running()
            if ret:
                return True
            if is_port_in_use(port):
                return True
        return False

    @staticmethod
    def check_mysql_is_running():
        """
        查询mysql状态，实例是否运行
        :return:
        """
        is_running, system_service_type, mysql_service_type = MysqlServiceInfoUtil.get_mysql_service_info()
        return is_running

    @staticmethod
    def get_undo_size_in_point_dir(undo_dir: str):
        """
        获取目标路径下undo文件数量，如果没有则返回0
        :param undo_dir: 目标路径
        :return: undo文件梳理
        """
        prefix_files = [file for file in os.listdir(undo_dir) if file.startswith('undo')]
        return len(prefix_files)
