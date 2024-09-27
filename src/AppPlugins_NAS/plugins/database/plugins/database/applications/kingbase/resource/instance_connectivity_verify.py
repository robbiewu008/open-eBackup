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
import json

from common.cleaner import clear
from common.common import output_execution_result, exter_attack
from common.common_models import ActionResult
from common.const import ParamConstant, ExecuteResultEnum, AuthType, SysData
from common.logger import Logger
from common.parse_parafile import ParamFileUtil, get_env_variable
from common.util import check_utils
from kingbase.common.util.get_html_result_utils import execute_cmd_and_parse_res
from kingbase.common.util.resource_util import get_version, check_special_character, \
    check_black_list, check_service_ip, check_os_name
from kingbase.common.error_code import ErrorCode

LOGGER = Logger().get_logger("kingbase.log")


class InstanceConnectivityVerify:
    def __init__(self, request_pid):
        self.pid = request_pid
        self.context = ParamFileUtil.parse_param_file(self.pid)
        self.application = self.context.get("application", {})
        self.result_file = os.path.join(ParamConstant.RESULT_PATH, f"result{self.pid}")
        self.extend_info = self.application.get("extendInfo", {})
        self.os_username = self.extend_info.get("osUsername")
        self.client_path = os.path.join(self.extend_info.get("clientPath"), "bin", "ksql")
        self.port = self.extend_info.get("instancePort")
        self.service_ip = self.extend_info.get("serviceIp")
        self.database_mode = self.extend_info.get("databaseMode")

    @exter_attack
    def check_connectivity(self):
        param = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR.value, bodyErr=ErrorCode.CHECK_CONNECTIVITY_FAILED,
                             message=f"Check connectivity failed!")
        try:
            param = self._check_connectivity()
        except Exception:
            LOGGER.info(f"Check connectivity by os verify failed!pid:{self.pid}")
            param = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR.value,
                                 bodyErr=ErrorCode.CHECK_CONNECTIVITY_FAILED,
                                 message="Check connectivity failed!")
        finally:
            clear(SysData.SYS_STDIN)
            LOGGER.info(f"Check connectivity result of Output param: {param.dict(by_alias=True)}")
            output_execution_result(self.result_file, param.dict(by_alias=True))

    def _check_connectivity(self):
        check_special_character([self.client_path, self.os_username])
        # 检查客户端输入是否正确
        if not os.path.exists(self.client_path):
            LOGGER.error('Client path is not exist.')
            return ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR.value, bodyErr=ErrorCode.CLIENT_PATH_IS_NOT_EXIST,
                                message="Client path is not exist!")

        check_black_list([self.client_path])

        if not check_os_name(self.os_username, self.client_path):
            return ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR.value, bodyErr=ErrorCode.OS_USERNAME_ERROR,
                                message="Check connectivity failed!")

        if not check_service_ip(self.service_ip):
            return ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, bodyErr=ErrorCode.AUTH_INFO_INCORRECT,
                                message=f"Service ip is invalid or can't be localhost!")

        if not check_utils.is_port(self.port):
            return ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, bodyErr=ErrorCode.AUTH_INFO_INCORRECT,
                                message=f"The port is invalid!")

        # 获取版本信息
        LOGGER.info(f"Start getting kingbase version! pid: {self.pid}")
        err_code, version = get_version(self.pid, self.client_path, self.os_username)
        if err_code != ErrorCode.SUCCESS:
            return ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR.value,
                                bodyErr=err_code, message="Get version failed!")

        # 校验连通性并查询data目录
        ret_code = ErrorCode.AUTH_INFO_INCORRECT
        data_path = None
        auth_type = get_env_variable(f"application_auth_authType_{self.pid}")
        LOGGER.info(f"Begin to check connectivity, auth_type: {auth_type}, param: {self.context}, pid: {self.pid}")
        if int(auth_type) == AuthType.APP_PASSWORD.value:
            ret_code, data_path = self._check_connectivity_by_database_verify()
            LOGGER.info(f"Already to login by database: {data_path}")
        if ret_code != ErrorCode.SUCCESS:
            return ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR.value, bodyErr=ret_code,
                                message="Check connectivity failed!")
        LOGGER.info(f"Success to check connectivity! pid: {self.pid}")
        if not self.database_mode:
            ret_code, self.database_mode = self._get_database_mode()
            if ret_code != ErrorCode.SUCCESS or not self.database_mode:
                return ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR.value, bodyErr=ret_code,
                                    message="Check connectivity failed!")
        return ActionResult(code=ExecuteResultEnum.SUCCESS.value, bodyErr=ret_code,
                            message=json.dumps(
                                {"version": version, "dataDirectory": data_path, "databaseMode": self.database_mode}))

    def _check_connectivity_by_database_verify(self):
        db_user_name = get_env_variable(f'application_auth_authKey_{self.pid}')
        check_special_character([db_user_name, self.client_path])
        cmd = f"su - {self.os_username} -c '{self.client_path} -U {db_user_name} " \
              f"-h {self.service_ip} -p {self.port} -d test -W -H -c \"show data_directory;\"'"
        return execute_cmd_and_parse_res(self.pid, cmd)

    def _get_database_mode(self):
        db_user_name = get_env_variable(f'application_auth_authKey_{self.pid}')
        check_special_character([db_user_name, self.client_path])
        cmd = f"su - {self.os_username} -c '{self.client_path} -U {db_user_name} " \
              f"-h {self.service_ip} -p {self.port} -d test -W -H -c \"show database_mode;\"'"
        return execute_cmd_and_parse_res(self.pid, cmd)


if __name__ == '__main__':
    pid = sys.argv[1]
    SysData.SYS_STDIN = sys.stdin.readline()
    instance_check_connectivity = InstanceConnectivityVerify(pid)
    check_special_character([instance_check_connectivity.os_username, instance_check_connectivity.client_path])
    instance_check_connectivity.check_connectivity()
