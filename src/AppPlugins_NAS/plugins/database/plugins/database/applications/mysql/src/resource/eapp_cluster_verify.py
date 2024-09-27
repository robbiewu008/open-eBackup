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

from mysql import log
from mysql.src.common.constant import IPConstant, MysqlExecSqlError
from mysql.src.common.error_code import MySQLErrorCode


class EAppClusterVerify:
    def __init__(self, mysql_base):
        self.mysql_base = mysql_base
        self.check_code = 0

    def check(self):
        if not self._check_ip(self.mysql_base.get_mysql_ip()):
            log.error("Invalid ip")
            return self.check_code
        if not self._check_port():
            log.error("Invalid port")
            return self.check_code
        if not self._check_auth():
            log.error("Invalid auth")
            return self.check_code
        if not self._check_sync_status():
            log.error("Data sync error.")
            return self.check_code
        return self.check_code

    def _check_ip(self, ip):
        if ip == IPConstant.LOCALHOST:
            return True
        local_ips = self.mysql_base.get_local_ips()
        if ip not in local_ips:
            self.check_code = MySQLErrorCode.ERR_IP
            return False
        return True

    def _check_port(self):
        ret, cnf_path = self.mysql_base.find_mycnf_path(self.mysql_base.my_cnf_path)
        if not ret:
            self.check_code = MySQLErrorCode.CHECK_MYSQL_CONF_FAILED
            return False
        port = self.mysql_base.get_mysql_port()
        with open(cnf_path) as cnf_file:
            lines = cnf_file.readlines()
        for line in lines:
            line = line.strip()
            if line.startswith("port") and not line.endswith(str(port)):
                self.check_code = MySQLErrorCode.ERR_PORT
                return False
        return True

    def _check_auth(self):
        ret, output = self.mysql_base.check_auth_info()
        if ret:
            return True
        if MysqlExecSqlError.ERROR_ACCESS_DENINED in output:
            self.check_code = MySQLErrorCode.ERROR_LOGIN_INFO
        return ret

    def _check_sync_status(self):
        ret, _ = self.mysql_base.check_cluster_sync_status()
        if not ret:
            self.check_code = MySQLErrorCode.ERROR_CLUSTER_SYNC_STATUS
            return False
        return True
