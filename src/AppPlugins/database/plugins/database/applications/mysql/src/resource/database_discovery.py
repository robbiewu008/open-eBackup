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
import sys
import uuid

from mysql import log
from common.common import exter_attack, execute_cmd
from common.const import ParamConstant
from common.util.check_utils import is_valid_id
from common.util.exec_utils import exec_write_new_file
from mysql.src.common.constant import MySQLType, IPConstant, MySQLJsonConstant, SystemServiceType, ExecCmdResult, \
    MySQLStrConstant
from mysql.src.common.error_code import MySQLErrorCode, MySQLCode
from mysql.src.common.execute_cmd import get_cmd_result, get_operating_system, safe_get_environ, \
    get_charset_from_instance, match_greatsql
from mysql.src.common.parse_parafile import ParseParaFile
from mysql.src.common.parse_parafile import BaseConnectParam
from mysql.src.common.response_param import get_body_error_param
from mysql.src.protect_mysql_base import MysqlBase
from mysql.src.utils.mysql_service_info_utils import MysqlServiceInfoUtil

# mysql系统库，不允许备份，所以这查询时就直接屏蔽掉
SYSTEM_DATABASE = ["information_schema", "mysql", "performance_schema", "sys"]


class DatabaseDiscovery:
    """查询实例下的数据库"""
    @exter_attack
    def __init__(self):
        self.pid = sys.argv[1]
        if not is_valid_id(self.pid):
            log.warn(f'req_id is invalid')
            sys.exit(1)
        self.context = ParseParaFile.parse_para_file(self.pid)
        self.instance = self.context.get("application", {})
        self.env = self.context.get("appEnv", {})
        self.port = self.instance.get("auth", {}).get("extendInfo", {}).get("instancePort")
        self.charset = get_charset_from_instance(self.instance)
        self.instance_ip = self.instance.get("extendInfo", {}).get("instanceIp", "")
        self.mysql_ip = self.instance_ip if self.instance_ip else IPConstant.LOCALHOST
        self.result_file = os.path.join(ParamConstant.RESULT_PATH, f"result{self.pid}")

    @exter_attack
    def get_databases(self):
        """获取database列表"""
        log.info(f"Begin to get database from {self.instance_ip}:{self.port}, pid :{self.pid}")
        try:
            self._get_databases()
        except Exception as e:
            log.error(f"An exception occurs when get database pid :{self.pid}!", e)

    def _get_databases(self):
        databases = []
        host_ip = self.instance_ip if self.instance_ip else IPConstant.LOCALHOST
        connect_param = BaseConnectParam(host_ip, self.port, self.charset, "show databases")
        database_list = get_cmd_result(connect_param, self.result_file, self.pid)
        if len(database_list) == 0:
            error_message = f"Get database failed or database is not exist in {self.port}"
            params = get_body_error_param(MySQLCode.FAILED.value, MySQLErrorCode.GET_DATABASES_FAILED, error_message)
            exec_write_new_file(self.result_file, params)
            log.error(error_message)
            raise Exception(error_message)
        # 查询版本信息
        version = get_mysql_version(self.port, self.mysql_ip, self.charset, self.pid)
        if not version:
            error_message = "Mysql is not running"
            params = get_body_error_param(MySQLCode.FAILED.value, MySQLErrorCode.ERROR_INSTANCE_IS_NOT_RUNNING,
                                          error_message)
            exec_write_new_file(self.result_file, params)
            log.error(error_message)
            raise Exception(error_message)
        code, out, err = execute_cmd("mysql --version")
        if code == ExecCmdResult.SUCCESS:
            if match_greatsql(out):
                version = version + "-" + MySQLStrConstant.GREATSQLAPPLICTATION
        # 获取操作系统
        deploy_operating_system = get_operating_system()
        # 获取数据库服务信息，包含是否running、服务名称、服务注册的系统服务名称
        is_running, system_service_type, mysql_service_type = MysqlServiceInfoUtil.get_mysql_service_info()
        if not is_running:
            system_service_type = "manual"
            mysql_service_type = "mysqld"
        for database in database_list:
            # 忽略系统数据库
            if database[0] in SYSTEM_DATABASE:
                continue
            # 忽略乱码的数据库
            if database[0].__contains__('#mysql50#'):
                continue
            database_uuid = str(uuid.uuid5(uuid.NAMESPACE_X500, self.instance.get("id") + MySQLType.SUBTYPE
                                           + self.instance.get("name") + database[0]))
            log.info(
                f"Ready to get database, pid: {self.pid}, uuid: {database_uuid},"
                f"instance_name: {self.instance.get('name')},database_name: {database[0]}")
            resource = dict()
            resource["id"] = database_uuid
            resource["type"] = MySQLType.TYPE
            resource["subType"] = MySQLType.SUBTYPE
            resource["name"] = database[0]
            resource["parentId"] = self.instance.get("id")
            resource["parentName"] = self.instance.get("name")
            extend_info = {
                "version": version, "deployOperatingSystem": deploy_operating_system,
                MySQLJsonConstant.SERVICE_NAME: mysql_service_type,
                MySQLJsonConstant.SYSTEM_SERVICE_TYPE_KEY: system_service_type
            }
            resource["extendInfo"] = extend_info
            databases.append(resource)
        params = {"resourceList": databases}
        exec_write_new_file(self.result_file, params)
        log.info(f"Already to get database from {self.port},pid: {self.pid}, length: {len(databases)}")


def get_mysql_version(port, mysql_ip, charset, pid):
    mysql_base = MysqlBase("", "", "", {})
    mysql_base.set_mysql_port(port)
    mysql_base.set_mysql_ip(mysql_ip)
    mysql_base.set_mysql_charset(charset)
    mysql_base.set_mysql_user(safe_get_environ(f"application_auth_authKey_{pid}"))
    mysql_base.set_mysql_pwd(f"application_auth_authPwd_{pid}")
    return mysql_base.get_mysql_version()


if __name__ == '__main__':
    log.info("Begin to get database!")
    database_discovery = DatabaseDiscovery()
    database_discovery.get_databases()
