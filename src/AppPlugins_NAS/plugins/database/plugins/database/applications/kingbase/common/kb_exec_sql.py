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
import re
import pwd
import pexpect
from bs4 import BeautifulSoup

from common.logger import Logger
from common.cleaner import clear
from common.const import AuthType, DeployType, RoleType
from common.common import execute_cmd
from common.parse_parafile import get_env_variable
from common.util import check_utils
from kingbase.common.const import CmdRetCode, TableSpaceKey
from kingbase.common.util import resource_util
from kingbase.common.util.resource_util import check_is_path_exists


LOGGER = Logger().get_logger("kingbase.log")


class ExecKbSql(object):
    """
    KingBase类
    """
    def __init__(self, kb_sql_params):
        self._pid = kb_sql_params[0]
        self._install_path = kb_sql_params[1]
        self._service_ip = kb_sql_params[2]
        self._host_port = kb_sql_params[3]
        self._deploy_type = kb_sql_params[4]

    @staticmethod
    def get_query_job_permission_params(param_dict):
        job_permission_params = dict()
        nodes = param_dict.get("appEnv", {}).get("nodes", [{}])
        # 单实例执行QueryJobPermission获取参数
        if len(nodes) == 1:
            job_permission_params["os_user_name"] = nodes[0].get("extendInfo", {}).get("osUsername")
            job_permission_params["install_path"] = nodes[0].get("extendInfo", {}).get("clientPath")
            job_permission_params["data_path"] = nodes[0].get("extendInfo", {}).get("dataDirectory")
            job_permission_params["service_ip"] = nodes[0].get("extendInfo", {}).get("serviceIp")
            job_permission_params["host_port"] = nodes[0].get("extendInfo", {}).get("instancePort")
            return job_permission_params

        sub_objs = param_dict.get("appEnv", {}).get("nodes", [{}])
        for obj in sub_objs:
            node_role = obj.get("extendInfo", {}).get("role", "")
            if str(node_role) == str(RoleType.PRIMARY.value):
                job_permission_params["os_user_name"] = obj.get("extendInfo", {}).get("osUsername")
                job_permission_params["install_path"] = obj.get("extendInfo", {}).get("clientPath")
                job_permission_params["data_path"] = obj.get("extendInfo", {}).get("dataDirectory")
                job_permission_params["service_ip"] = obj.get("extendInfo", {}).get("serviceIp")
                job_permission_params["host_port"] = obj.get("extendInfo", {}).get("instancePort")
                break
        return job_permission_params

    @staticmethod
    def get_cluster_params(param_dict):
        cluster_params = dict()
        sub_objs = param_dict.get("job", {}).get("protectSubObject", [{}])
        for obj in sub_objs:
            node_role = obj.get("extendInfo", {}).get("role", "")
            if str(node_role) == str(RoleType.PRIMARY.value):
                cluster_params["os_user_name"] = obj.get("extendInfo", {}).get("osUsername")
                cluster_params["install_path"] = obj.get("extendInfo", {}).get("clientPath")
                cluster_params["data_path"] = obj.get("extendInfo", {}).get("dataDirectory", )
                cluster_params["service_ip"] = obj.get("extendInfo", {}).get("serviceIp")
                cluster_params["host_port"] = obj.get("extendInfo", {}).get("instancePort")
                break
        return cluster_params

    @staticmethod
    def parse_sql_result(cmd_result, key):
        """
        解析sql语句执行结果
        :param cmd_result: str，SQL语句执行结果
        :param key: str，关键词
        :return: str，关键词对应的结果
        """
        value = ""
        result_list = cmd_result.strip().split('\n')
        for info in result_list:
            if key in info:
                value = info.split()[-1]
                break
        return value

    @staticmethod
    def parse_db_sql_result(cmd_result):
        """
        解析查询表空间sql语句执行结果
        :param cmd_result: str，SQL语句执行结果
        :return: dict，不在默认路径下的表空间名称和地址
        """
        result_list = cmd_result.split('\n')
        name_list = []
        location_list = []
        for info in result_list:
            if (TableSpaceKey.NAME_CN in info) or (TableSpaceKey.NAME_EN in info):
                name_list.append(info.split()[-1])
                continue
            if (TableSpaceKey.LOCATION_CN in info) or (TableSpaceKey.LOCATION_EN in info):
                location_list.append(info.split()[-1])
        table_space = dict()
        tb_info = zip(name_list, location_list)
        for name, location in tb_info:
            if not re.match(location, "|"):
                table_space[name] = location
        return table_space

    def exec_sql_cmd(self, os_user_name, sql_cmd, timeout=-1, query_job_permission=False, pager_off=False):
        """
        执行sql语句
        :param os_user_name: str，操作系统用户名
        :param sql_cmd: str，SQL语句
        :return: tuple，返回码、标准输出、标准错误
        """
        if not os.path.exists(self._install_path):
            LOGGER.error(f"Param install path {self._install_path} error, pid: {self._pid}.")
            return CmdRetCode.EXEC_ERROR.value, "", f"Param install path {self._install_path} error."
        ksql_path = os.path.join(self._install_path, "bin", "ksql")
        check_is_path_exists(ksql_path)
        if not resource_util.check_os_name(os_user_name, ksql_path):
            return CmdRetCode.EXEC_ERROR.value, "", "Os username is not exist."
        if not check_utils.is_ip_address(self._service_ip):
            LOGGER.error("Service ip is incorrect.")
            return CmdRetCode.EXEC_ERROR.value, "", "Service ip is incorrect."
        if not check_utils.is_port(self._host_port):
            LOGGER.error("Port is incorrect.")
            return CmdRetCode.EXEC_ERROR.value, "", "Port is incorrect."
        if query_job_permission:
            auth_type = get_env_variable(f"appEnv_nodes_0_auth_authType_{self._pid}")
            db_user_name = get_env_variable(f"appEnv_nodes_0_auth_authKey_{self._pid}")
        else:
            if self._deploy_type == str(DeployType.SINGLE_TYPE.value):
                auth_type = get_env_variable(f"job_protectObject_auth_authType_{self._pid}")
                db_user_name = get_env_variable(f"job_protectObject_auth_authKey_{self._pid}")
            elif self._deploy_type == str(DeployType.CLUSTER_TYPE.value):
                auth_type = get_env_variable(f"job_protectEnv_nodes_0_auth_authType_{self._pid}")
                db_user_name = get_env_variable(f"job_protectEnv_nodes_0_auth_authKey_{self._pid}")
            else:
                LOGGER.error(f"Invalid deploy type ({self._deploy_type}), pid: {self._pid}.")
                return CmdRetCode.EXEC_ERROR.value, "", f"Invalid deploy type ({self._deploy_type})."
        if auth_type == str(AuthType.NO_AUTO.value):
            conn_cmd = f"{ksql_path} -d test -x -h {self._service_ip} -p {self._host_port}"
            exec_cmd = f"su - {os_user_name} -c '{conn_cmd} -c \"{sql_cmd}\"'"
            LOGGER.debug(f"Exec sql cmd: {exec_cmd}, pid: {self._pid}.")
            code, out, err = execute_cmd(exec_cmd)
            LOGGER.debug(f"Exec sql cmd result: {code, out, err}, pid: {self._pid}.")
            return code, out, err
        elif auth_type == str(AuthType.APP_PASSWORD.value):
            resource_util.check_special_character([db_user_name])
            conn_cmd = f"{ksql_path} -U {db_user_name} -d test -x -h {self._service_ip} -p {self._host_port} -W"
            if pager_off:
                exec_cmd = f"su - {os_user_name} -c '{conn_cmd}  -c \"\\pset pager off\" -c \"{sql_cmd}\"'"
            else:
                exec_cmd = f"su - {os_user_name} -c '{conn_cmd} -c \"{sql_cmd}\"'"
            LOGGER.debug(f"Exec sql cmd: {exec_cmd}, pid: {self._pid}.")
            code, out, err = self.exec_sql_cmd_pass(exec_cmd, query_job_permission, timeout=timeout)
            LOGGER.debug(f"Exec sql cmd result: {code, out, err}, pid: {self._pid}.")
            return code, out, err
        else:
            LOGGER.error(f"Invalid auth type ({auth_type}), pid: {self._pid}.")
            return CmdRetCode.EXEC_ERROR.value, "", f"Invalid auth type ({auth_type})."

    def exec_sql_cmd_pass(self, exec_cmd, query_job_permission, timeout=-1):
        """
        执行sql语句
        :param exec_cmd: str，命令行
        :return: tuple，返回码、标准输出、标准错误
        """
        db_pwd = None
        try:
            if query_job_permission:
                db_pwd = get_env_variable(f"appEnv_nodes_0_auth_authPwd_{self._pid}")
            else:
                if self._deploy_type == str(DeployType.SINGLE_TYPE.value):
                    db_pwd = get_env_variable(f"job_protectObject_auth_authPwd_{self._pid}")
                elif self._deploy_type == str(DeployType.CLUSTER_TYPE.value):
                    db_pwd = get_env_variable(f"job_protectEnv_nodes_0_auth_authPwd_{self._pid}")
                else:
                    LOGGER.error(f"Invalid deploy type ({self._deploy_type}), pid: {self._pid}.")
                    return CmdRetCode.EXEC_ERROR.value, "", f"Invalid deploy type ({self._deploy_type})."
            process = pexpect.spawn(exec_cmd, encoding='utf-8')
            index = process.expect([pexpect.EOF, pexpect.TIMEOUT, "Password for", "Password:", "口令"])
            if index in (0, 1):
                return CmdRetCode.EXEC_ERROR.value, "", "Connect database failed."
            process.sendline(db_pwd)
            exec_ret_index = process.expect([pexpect.EOF, pexpect.TIMEOUT, "请检查您的归档命令是否正确执行",
                                             "Check that your archive_command is executing properly"], timeout=timeout)
            LOGGER.info(f"exec_ret_index: {exec_ret_index}")
            # 归档命令或wal日志等级配置错误
            if exec_ret_index in (2, 3):
                LOGGER.error("Failed to execute the archive_command.")
                return CmdRetCode.CONFIG_ERROR.value, "", "Base backup done, but archive_command execute error."
            # 执行命令超时
            if exec_ret_index in (1,):
                return CmdRetCode.EXEC_ERROR.value, "", "Exec sql cmd failed."
            cmd_result = process.before
            return CmdRetCode.EXEC_SUCCESS.value, cmd_result, ""
        finally:
            clear(db_pwd)
