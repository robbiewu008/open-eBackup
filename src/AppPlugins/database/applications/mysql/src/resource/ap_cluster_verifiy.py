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

from mysql import log
from common.const import ParamConstant
from common.util.exec_utils import exec_overwrite_file
from mysql.src.common.error_code import MySQLCode, MySQLErrorCode
from mysql.src.common.execute_cmd import get_cmd_result, get_charset_from_instance
from mysql.src.common.parse_parafile import BaseConnectParam
from mysql.src.common.response_param import get_body_error_param


class ApClusterVerify:
    def __init__(self, pid, context):
        self.pid = pid
        self.context = context
        self.env = self.context.get("appEnv", {})
        self.db_instance = self.context.get("application", {})
        self.port = self.db_instance.get("auth", {}).get("extendInfo", {}).get("instancePort", 0)
        self.charset = get_charset_from_instance(self.db_instance)
        self.result_path = os.path.join(ParamConstant.RESULT_PATH, f"result{self.pid}")
        self.ip_list = self.env.get("endpoint").split(",")
        self.slave_status = None
        self.master_host = set()
        self.count = 0

    def get_slave_status(self, host_ip):
        connect_param = BaseConnectParam(host_ip, self.port, self.charset, "show slave status")
        slave_status = get_cmd_result(connect_param, self.result_path, self.pid)
        if not slave_status:
            self.master_host.add(host_ip)
            self.count += 1
            return False
        self.slave_status = slave_status
        self.master_host.add(slave_status[0][2])
        return True

    def check_is_cluster(self):
        try:
            if not self.check_is_ap_cluster():
                self.check_is_aa_cluster()
        except Exception as e:
            log.error(f"An Exception occur when check is cluster,{e}")

    def check_is_ap_cluster(self):
        for ip in self.ip_list:
            self.get_slave_status(ip)
            if len(self.master_host) > 1:
                self.count = 0
                return False
            if self.master_host not in self.ip_list:
                self.check_cluster_failed(ip)
                return True
        params = get_body_error_param(MySQLCode.SUCCESS.value, MySQLCode.SUCCESS.value, f"check ap cluster success!")
        exec_overwrite_file(self.result_path, params)
        return True

    def check_is_aa_cluster(self):
        for ip in self.ip_list:
            if ip is None or ip == "":
                continue
            if self.slave_status[0][1] == ip or self.slave_status[0][1] not in self.ip_list:
                self.check_cluster_failed(ip)
                return False
        params = get_body_error_param(MySQLCode.SUCCESS.value, MySQLCode.SUCCESS.value, f"Check aa cluster success!")
        exec_overwrite_file(self.result_path, params)
        return True

    def check_cluster_failed(self, ip):
        params = get_body_error_param(MySQLCode.FAILED.value, MySQLErrorCode.CHECK_CLUSTER_FAILED,
                                      f"Check ap cluster failed!pid = {self.pid}, ip = {ip}")
        log.info(
            f"Failed to execute the connectivity command, params = {params}, pid = {self.pid}")
        exec_overwrite_file(self.result_path, params)
