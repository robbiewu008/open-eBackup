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

import pexpect
from bs4 import BeautifulSoup

from common import cleaner
from common.logger import Logger
from common.common import execute_cmd, check_command_injection
from common.const import AuthType
from common.util import check_utils
from common.util.cmd_utils import cmd_format
from postgresql.common.const import CmdRetCode, PexpectResult
from postgresql.common.util.domain_2_ip_util import domain_2_ip
from postgresql.common.util.get_sensitive_utils import get_env_variable
from postgresql.common.util.pg_common_utils import PostgreCommonUtils

LOGGER = Logger().get_logger("postgresql.log")


class ExecPgSql(object):
    pg_15_process = None

    def __init__(self, pid, client_path, service_ip, port):
        self._pid = pid
        self._client_path = client_path
        self._service_ip = service_ip
        self._port = port
        self.enable_root = PostgreCommonUtils.get_root_switch()

    @staticmethod
    def parse_html_result(cmd_result):
        soup = BeautifulSoup(cmd_result, "html.parser")
        trs = soup.find_all(name="tr")
        params = []
        soup = BeautifulSoup(str(trs[0]), "html.parser")
        ths = soup.find_all(name='th', attrs={"align": "center"})
        headers = [header.get_text() for header in ths]
        for i in range(1, len(trs)):
            tr = trs[i]
            _soup = BeautifulSoup(str(tr), "html.parser")
            tds = _soup.find_all(name='td', attrs={"align": "left"})
            param = [td.get_text() for td in tds]
            params.append(dict(zip(headers, param)))
        LOGGER.info(f"Get parse html result: {params}.")
        return params

    @staticmethod
    def close_session():
        try:
            if ExecPgSql.pg_15_process is not None:
                ExecPgSql.pg_15_process.close()
                ExecPgSql.pg_15_process = None
        except Exception as e:
            LOGGER.error(e, exc_info=True)

    def parse_sql_result(self, cmd_result, key):
        nodes = self.parse_html_result(cmd_result)
        if not nodes or not nodes[0].get(key):
            return ""
        return nodes[0].get(key)

    def exec_sql_cmd(self, os_user_name, sql_cmd, timeout=-1, pager_off=False):
        if not os.path.isdir(self._client_path):
            LOGGER.error(f"Param client path {self._client_path} error, pid: {self._pid}.")
            return CmdRetCode.EXEC_ERROR.value, "", "Param client path not exist."

        p_sql_path = os.path.join(self._client_path, "bin", "psql")
        if not os.path.exists(p_sql_path):
            LOGGER.error("Param psql path not exist!")
            return CmdRetCode.EXEC_ERROR.value, "", "Param psql path not exist."
        if not PostgreCommonUtils.check_os_name(os_user_name, p_sql_path, self.enable_root)[0]:
            return CmdRetCode.EXEC_ERROR.value, "", "Os username is not exist."
        if not check_utils.is_ip_address(self._service_ip):
            LOGGER.error("Service ip is incorrect!")
            return CmdRetCode.EXEC_ERROR.value, "", "Service ip is incorrect."
        if not check_utils.is_port(self._port):
            LOGGER.error("Port is incorrect!")
            return CmdRetCode.EXEC_ERROR.value, "", "Port is incorrect."
        # 数据库认证“-W”命令强制输入密码
        conn_cmd = cmd_format("{} -H -h {} -p {} -W", p_sql_path, self._service_ip, self._port)
        auth_type = get_env_variable(f"job_protectObject_auth_authType_{self._pid}")
        if int(auth_type) == AuthType.NO_AUTO.value:
            exec_cmd = f"su - {os_user_name} -c '{conn_cmd} -c \"{sql_cmd}\"'"
            code, out, err = execute_cmd(exec_cmd)
            return code, out, err
        db_user_name = get_env_variable(f"job_protectObject_auth_authKey_{self._pid}")
        if not PostgreCommonUtils.check_special_characters(db_user_name):
            LOGGER.error("Db username is incorrect!")
            return CmdRetCode.EXEC_ERROR.value, "", "Db username is incorrect."
        conn_cmd = f"{conn_cmd} -U {db_user_name} -d postgres"
        if pager_off:
            exec_cmd = f"su - {os_user_name} -c '{conn_cmd} -c \"\\pset pager off\" -c \"{sql_cmd}\"'"
        else:
            exec_cmd = f"su - {os_user_name} -c '{conn_cmd} -c \"{sql_cmd}\"'"
        code, out, err = self.exec_sql_cmd_pass(exec_cmd, timeout=timeout)
        return code, out, err

    def exec_sql_cmd_resource(self, os_user_name, sql_cmd, db_user_name, db_pwd):
        if not os.path.isdir(self._client_path):
            LOGGER.error(f"Param client path {self._client_path} error, pid: {self._pid}.")
            return CmdRetCode.EXEC_ERROR.value, "", "Param client path not exist."

        p_sql_path = os.path.join(self._client_path, "bin", "psql")
        if not os.path.exists(p_sql_path):
            LOGGER.error("Param psql path not exist!")
            return CmdRetCode.EXEC_ERROR.value, "", "Param psql path not exist."
        if not PostgreCommonUtils.check_os_name(os_user_name, p_sql_path, self.enable_root)[0]:
            return CmdRetCode.EXEC_ERROR.value, "", "Os username is not exist."
        if not check_utils.is_ip_address(self._service_ip):
            LOGGER.error("Service ip is incorrect!")
            return CmdRetCode.EXEC_ERROR.value, "", "Service ip is incorrect."
        if not check_utils.is_port(self._port):
            LOGGER.error("Port is incorrect!")
            return CmdRetCode.EXEC_ERROR.value, "", "Port is incorrect."
        # 数据库认证“-W”命令强制输入密码
        conn_cmd = cmd_format("{} -H -h {} -p {} -W", p_sql_path, self._service_ip, self._port)
        if not PostgreCommonUtils.check_db_user_valid(db_user_name):
            LOGGER.error("Db username is invalid!")
            return CmdRetCode.EXEC_ERROR.value, "", "Db username is invalid."
        conn_cmd = f"{conn_cmd} -U {db_user_name} -d postgres"
        exec_cmd = f"su - {os_user_name} -c '{conn_cmd} -c \"{sql_cmd}\"'"
        code, out, err = self.exec_sql_cmd_pass_resoource(exec_cmd, db_pwd)
        return code, out, err

    def exec_backup_cmd(self, os_user_name, sql_cmd, timeout=-1, pager_off=False):
        db_pwd = get_env_variable(f"job_protectObject_auth_authPwd_{self._pid}")
        while True:
            if ExecPgSql.pg_15_process is not None:
                process = ExecPgSql.pg_15_process
            else:
                p_sql_path = os.path.join(self._client_path, "bin", "psql")
                # 数据库认证“-W”命令强制输入密码
                conn_cmd = cmd_format("{} -H -h {} -p {} -W", p_sql_path, self._service_ip, self._port)
                db_user_name = get_env_variable(f"job_protectObject_auth_authKey_{self._pid}")
                if check_command_injection(db_user_name):
                    raise Exception(f"Param of db_user_name invalid.{db_user_name}")
                conn_cmd = f"{conn_cmd} -U {db_user_name} -d postgres"
                if pager_off:
                    exec_cmd = f"su - {os_user_name} -c '{conn_cmd} -c \"\\pset pager off\"'"
                else:
                    exec_cmd = f"su - {os_user_name} -c '{conn_cmd}'"
                process = pexpect.spawn(exec_cmd, encoding='utf-8')
                index = process.expect(PexpectResult.DB_LOGIN_PASSWORD, timeout=timeout)
                if index in (0, 1):
                    return CmdRetCode.EXEC_ERROR.value, "", "Connect database failed."
                process.sendline(db_pwd)
                index = process.expect(
                    [pexpect.EOF, pexpect.TIMEOUT, "recovery is in progress", "<p>", "archive_mode enabled",
                     "Check that your archive_command is executing properly", "请检查您的归档命令是否正确执行",
                     "psql"],
                    timeout=timeout)
                if index in (0, 1):
                    return CmdRetCode.EXEC_ERROR.value, "", "Exec sql cmd failed."
                if index == 2:
                    LOGGER.info("Database recovery is in progress, retry in 3 seconds.")
                    time.sleep(3)
                    continue
                if index in (4, 5, 6):
                    LOGGER.error("The archive mode is incorrectly.")
                    return CmdRetCode.CONFIG_ERROR.value, "", "Base backup done, but archive mode error."
                if ExecPgSql.pg_15_process is None:
                    ExecPgSql.pg_15_process = process
            process.sendline(sql_cmd)
            process.expect(r"<table.*</table>", timeout=timeout)
            cmd_result = process.after
            break
        return CmdRetCode.EXEC_SUCCESS.value, cmd_result, ""

    def exec_sql_cmd_pass(self, exec_cmd, timeout=-1):
        db_pwd = get_env_variable(f"job_protectObject_auth_authPwd_{self._pid}")
        try:
            while True:
                process = pexpect.spawn(exec_cmd, encoding='utf-8')
                index = process.expect(PexpectResult.DB_LOGIN_PASSWORD)
                if index in (0, 1):
                    return CmdRetCode.EXEC_ERROR.value, "", "Connect database failed."
                process.sendline(db_pwd)
                index = process.expect(
                    [pexpect.EOF, pexpect.TIMEOUT, "recovery is in progress", "<p>", "archive_mode enabled",
                     "Check that your archive_command is executing properly", "请检查您的归档命令是否正确执行"],
                    timeout=timeout)
                if index in (0, 1):
                    return CmdRetCode.EXEC_ERROR.value, "", "Exec sql cmd failed."
                if index == 2:
                    LOGGER.info("Database recovery is in progress, retry in 3 seconds.")
                    time.sleep(3)
                    continue
                if index in (4, 5, 6):
                    LOGGER.error("The archive mode is incorrectly.")
                    return CmdRetCode.CONFIG_ERROR.value, "", "Base backup done, but archive mode error."
                cmd_result = process.before
                break
        finally:
            cleaner.clear(db_pwd)
        return CmdRetCode.EXEC_SUCCESS.value, cmd_result, ""

    def exec_sql_cmd_pass_resoource(self, exec_cmd, db_pwd, timeout=-1):
        try:
            while True:
                process = pexpect.spawn(exec_cmd, encoding='utf-8')
                index = process.expect(PexpectResult.DB_LOGIN_PASSWORD)
                if index in (0, 1):
                    return CmdRetCode.EXEC_ERROR.value, "", "Connect database failed."
                process.sendline(db_pwd)
                index = process.expect(
                    [pexpect.EOF, pexpect.TIMEOUT, "recovery is in progress", "<p>", "archive_mode enabled",
                     "Check that your archive_command is executing properly", "请检查您的归档命令是否正确执行"],
                    timeout=timeout)
                if index in (0, 1):
                    return CmdRetCode.EXEC_ERROR.value, "", "Exec sql cmd failed."
                if index == 2:
                    LOGGER.info("Database recovery is in progress, retry in 3 seconds.")
                    time.sleep(3)
                    continue
                if index in (4, 5, 6):
                    LOGGER.error("The archive mode is incorrectly.")
                    return CmdRetCode.CONFIG_ERROR.value, "", "Base backup done, but archive mode error."
                cmd_result = process.before
                break
        finally:
            cleaner.clear(db_pwd)
        return CmdRetCode.EXEC_SUCCESS.value, cmd_result, ""

    def get_pg_cluster_info(self, cmd_result):
        nodes = self.parse_html_result(cmd_result)
        for node_info in nodes:
            for key, value in node_info.items():
                if key == "hostname":
                    node_info[key] = domain_2_ip(value)
        return nodes
