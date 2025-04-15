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

import json
import os
import sys
import platform

import pexpect
from bs4 import BeautifulSoup

from common import cleaner
from common.common import exter_attack
from common.common_models import ActionResult
from common.const import IPConstant
from common.const import ParamConstant, ExecuteResultEnum, AuthType, SysData
from common.logger import Logger
from common.util import check_utils
from common.util.check_utils import is_valid_id
from common.util.cmd_utils import cmd_format
from common.util.exec_utils import exec_overwrite_file
from postgresql.common.const import PexpectResult, CmdRetCode
from postgresql.common.error_code import ErrorCode
from postgresql.common.pg_exec_sql import ExecPgSql
from postgresql.common.util.get_sensitive_utils import get_env_variable
from postgresql.common.util.get_version_util import get_version
from postgresql.common.util.pg_common_utils import PostgreCommonUtils
from postgresql.common.util.pg_param import JsonParam

if platform.system().lower() == "linux":
    import pwd

LOGGER = Logger().get_logger("postgresql.log")


def parse_html_result(data_directory):
    LOGGER.info(f"Begin to parse data dir :{data_directory}")
    soup = BeautifulSoup(data_directory, "html.parser")
    trs = soup.find_all(name="tr")
    _soup = BeautifulSoup(str(trs[1]), "html.parser")
    td = _soup.find_all(name='td', attrs={"align": "left"})
    data_dir = td[0].get_text()
    LOGGER.info(f"Success to parse data dir :{data_dir}")
    return data_dir


class InstanceConnectivityVerify:
    def __init__(self, request_pid):
        self.pid = request_pid
        self.context = JsonParam.parse_param_with_jsonschema(self.pid)
        self.application = self.context.get("application", {})
        self.result_file = os.path.join(ParamConstant.RESULT_PATH, f"result{self.pid}")
        self.extend_info = self.application.get("extendInfo", {})
        self.os_username = self.extend_info.get("osUsername")
        self.client_path = os.path.realpath(os.path.join(self.extend_info.get("clientPath"), "bin", "psql"))
        self.archive_dir = self.extend_info.get("archiveDir", "")
        self.port = self.extend_info.get("instancePort")
        self.service_ip = self.extend_info.get("serviceIp")
        self.enable_root = PostgreCommonUtils.get_root_switch()

    @exter_attack
    def check_connectivity(self):
        param = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, bodyErr=ErrorCode.CHECK_CONNECTIVITY_FAILED,
                             message=f"Check connectivity failed!")
        try:
            param = self._check_connectivity()
        except Exception as e:
            LOGGER.info(f"Check connectivity by os verify failed!pid:{self.pid}", e)
            param = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, bodyErr=ErrorCode.CHECK_CONNECTIVITY_FAILED,
                                 message="Check connectivity failed!")
        finally:
            cleaner.clear(SysData.SYS_STDIN)
            LOGGER.info('Clearing data successfully')
            LOGGER.info(f"Output param : {param.dict(by_alias=True)}")
            exec_overwrite_file(self.result_file, param.dict(by_alias=True))

    def _check_connectivity(self):
        for check in (self.os_username, self.client_path):
            if not PostgreCommonUtils.check_special_characters(check):
                return ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, bodyErr=ErrorCode.CHECK_CONNECTIVITY_FAILED,
                                    message=f"String contains special characters!")
        if not check_utils.is_port(self.port) or not PostgreCommonUtils.check_port_is_listen(self.port):
            return ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, bodyErr=ErrorCode.DATABASE_PORT_IS_INVALID,
                                message=f"The port is invalid!")
        if not self._check_service_ip() or self.service_ip not in PostgreCommonUtils.get_local_ips():
            return ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, bodyErr=ErrorCode.SERVICE_IP_IS_INVALID,
                                message=f"Service ip is invalid or can't be localhost!")
        if not os.path.exists(self.client_path):
            return ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, bodyErr=ErrorCode.CLIENT_PATH_IS_NOT_EXIST,
                                message="Cilent path is not exist!")
        if not PostgreCommonUtils.check_black_list(self.client_path):
            return ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, bodyErr=ErrorCode.CLIENT_PATH_IS_NOT_EXIST,
                                message="Cilent path in black list!")
        if not PostgreCommonUtils.check_os_user(self.os_username, self.client_path, self.enable_root)[0]:
            return ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, bodyErr=ErrorCode.USER_IS_NOT_EXIST,
                                message="Os username is not exist!")
        LOGGER.info(
            f"Start to check connectivity os_username: {self.os_username}, client_path:{self.client_path}, "
            f"port: {self.port}, service_ip: {self.service_ip}, pid: {self.pid}")
        # 获取版本信息
        get_version_res, version = get_version(self.pid, self.client_path, self.os_username, self.enable_root)
        if not get_version_res:
            return ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, bodyErr=version,
                                message="Get version failed!")
        # 校验连通性并查询data目录
        res = False
        data = ErrorCode.CHECK_CONNECTIVITY_FAILED
        auth_type = get_env_variable(f"application_auth_authType_{self.pid}")
        LOGGER.info(
            f"Begin to check connectivity auth_type: {auth_type}, param: {self.context}, pid: {self.pid}")
        if int(auth_type) == AuthType.NO_AUTO.value:
            res, data = self._pg_login_by_os()
            LOGGER.info(f"Already to login by os:{data}")
        if int(auth_type) == AuthType.APP_PASSWORD.value:
            res, data = self._check_connectivity_by_database_verify()
            LOGGER.info(f"Already to login by database:{data}")
        if not res:
            return ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, bodyErr=data,
                                message="Check connectivity failed!")
        message_dict = {"version": version, "dataDirectory": data}
        if self.archive_dir and not self.check_archive_dir():
            return ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, bodyErr=ErrorCode.ARCHIVE_MODE_CONFIG_ERROR,
                                message="Input archive directory error.")
        self.set_config_file(message_dict)
        return ActionResult(code=ExecuteResultEnum.SUCCESS, bodyErr=ExecuteResultEnum.SUCCESS,
                            message=json.dumps(message_dict))

    def set_config_file(self, message_dict):
        res, data = self._query_system_param("config_file")
        if res:
            message_dict['configFile'] = data
        res, data = self._query_system_param("hba_file")
        if res:
            message_dict['hbaFile'] = data
        res, data = self._query_system_param("ident_file")
        if res:
            message_dict['identFile'] = data

    def check_archive_dir(self):
        archive_dir = self.query_archive_dir()
        if archive_dir:
            if os.path.isdir(archive_dir) and os.path.realpath(self.archive_dir) == os.path.realpath(archive_dir):
                return True
            return False
        if os.path.isdir(self.archive_dir):
            wal_files = os.listdir(self.archive_dir)
            file_list = []
            for wal_file in wal_files:
                if PostgreCommonUtils.is_wal_file(wal_file):
                    file_list.append(os.path.join(self.archive_dir, wal_file))
            # 用户填写归档目录存在WAL日志文件则认为是正确目录
            if file_list:
                return True
        return False

    def query_archive_dir(self):
        try:
            res, archive_info = self._query_system_param("archive_command")
            if res:
                return str(archive_info)
            return ""
        except Exception as err:
            LOGGER.error(f"show archive_command err: {err}.")
            return ""

    def _query_system_param(self, param_name):
        child = None
        db_user = get_env_variable(f'application_auth_authKey_{self.pid}')
        if not PostgreCommonUtils.check_db_user_valid(db_user):
            LOGGER.error(f"Db user name is invalid, check db_user:{db_user}!")
            return False, ErrorCode.CHECK_CONNECTIVITY_FAILED
        cmd = cmd_format("su - {} -c \"{} -U {} -h {} -p {} -d postgres -W -H -c \'show {}\'\"",
                         self.os_username, self.client_path, db_user, self.service_ip, self.port, param_name)
        try:
            child = pexpect.spawn(cmd, timeout=10, encoding="utf-8")
            index = child.expect(PexpectResult.DB_LOGIN_PASSWORD)
            if index in (0, 1):
                LOGGER.error(
                    f"Login database error! Check client path:{self.client_path}, port: {self.port}, "
                    f"ip: {self.service_ip}, pid: {self.pid}")
                child.close()
                return False, ErrorCode.CHECK_CONNECTIVITY_FAILED

            child.sendline(get_env_variable(f"application_auth_authPwd_{self.pid}"))
            db_result = child.expect(PexpectResult.HTML_RESULT)
            if index in (0, 1):
                child.close()
                LOGGER.error(
                    f"Password incorrect! client path:{self.client_path}, db_result: {db_result}, "
                    f"port: {self.port}, ip: {self.service_ip}")
                return False, ErrorCode.CHECK_CONNECTIVITY_FAILED
            data_dir = child.before
            LOGGER.info(f"Success to login pgsql by db verify!pid:{self.pid}")
            child.close()
            data_file_directory = parse_html_result(data_dir)
            if param_name == "archive_command":
                data_file_directory = data_file_directory.split()[data_file_directory.split().index("cp") + 2].strip(
                    '"%f')
            code, res = PostgreCommonUtils.check_os_user(self.os_username, data_file_directory, self.enable_root)
            return code, res
        finally:
            if child:
                child.close()

    def _check_connectivity_by_database_verify(self):
        child = None
        db_user = get_env_variable(f'application_auth_authKey_{self.pid}')
        if not PostgreCommonUtils.check_db_user_valid(db_user):
            LOGGER.error(f"Db user name is invalid, check db_user:{db_user}!")
            return False, ErrorCode.CHECK_CONNECTIVITY_FAILED
        cmd = cmd_format("su - {} -c \"{} -U {} -h {} -p {} -d postgres -W -H -c \'show data_directory\'\"",
                         self.os_username, self.client_path, db_user, self.service_ip, self.port)
        try:
            child = pexpect.spawn(cmd, timeout=10, encoding="utf-8")
            index = child.expect(PexpectResult.DB_LOGIN_PASSWORD)
            if index in (0, 1):
                LOGGER.error(
                    f"Login database error! Check client path:{self.client_path}, port: {self.port}, "
                    f"ip: {self.service_ip}, pid: {self.pid}")
                child.close()
                return False, ErrorCode.CHECK_CONNECTIVITY_FAILED

            child.sendline(get_env_variable(f"application_auth_authPwd_{self.pid}"))
            db_result = child.expect(PexpectResult.HTML_RESULT)
            if index in (0, 1):
                child.close()
                LOGGER.error(
                    f"Password incorrect! client path:{self.client_path}, db_result: {db_result}, "
                    f"port: {self.port}, ip: {self.service_ip}")
                return False, ErrorCode.CHECK_CONNECTIVITY_FAILED
            data_dir = child.before
            LOGGER.info(f"Success to login pgsql by db verify!pid:{self.pid}")
            child.close()
            data_file_directory = parse_html_result(data_dir)
            code, res = PostgreCommonUtils.check_os_user(self.os_username, data_file_directory, self.enable_root)
            return code, res
        finally:
            if child:
                child.close()

    def _pg_login_by_os(self):
        child = None
        cmd = cmd_format("su -{} -c \"{} -U {} -h {} -p {} -d postgres -H -c 'show data_directory'\"", self.os_username,
                         self.client_path, self.os_username, self.service_ip, self.port)
        try:
            child = pexpect.spawn(cmd, timeout=10, encoding="utf-8")
            LOGGER.info(f"Exec cmd :{cmd}, pid : {self.pid}")
            index = child.expect(PexpectResult.HTML_RESULT)
            if index in (0, 1):
                LOGGER.error(
                    f"Login database error! Check client path:{self.client_path}, port: {self.port}, "
                    f"ip: {self.service_ip}, pid :{self.pid}")
                child.close()
                return False, ErrorCode.LOGIN_FAILED
            LOGGER.info(f"Success to login pgsql by os verify! pid :{self.pid}")
            data_dir = child.before
            child.close()
            data_file_directory = parse_html_result(data_dir)
            code, res = PostgreCommonUtils.check_os_user(self.os_username, data_file_directory, self.enable_root)
            return code, res
        finally:
            if child:
                child.close()

    def _check_service_ip(self):
        if not check_utils.is_ip_address(self.service_ip):
            LOGGER.error(f"The service ip is invalid.")
            return False
        return self.service_ip != IPConstant.LOCAL_HOST


if __name__ == '__main__':
    pid = sys.argv[1]
    for line in sys.stdin:
        SysData.SYS_STDIN = line
        break
    # 校验pid
    if not is_valid_id(pid):
        LOGGER.warn(f'pid is invalid!')
        sys.exit(1)
    instance_check_connectivity = InstanceConnectivityVerify(pid)
    instance_check_connectivity.check_connectivity()
