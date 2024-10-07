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

from common.common import exter_attack
from common.const import JobData, AuthType
from common.logger import Logger
from dameng.commons.common import get_env_value
from dameng.commons.const import ErrCode, ResourcesSubType
from dameng.commons.dameng_tool import DmSqlTool
from dameng.commons.dm_param_parse import verifying_special_characters
from dameng.commons.query_information import query_db_status, get_db_name, query_auth_permisson, get_version
from dameng.resource.damengsource import DamengSource

LOGGER = Logger().get_logger("dameng.log")


class DamengApplication(DamengSource):

    def get_resource(self, param_msg):
        """
        检查达梦软件是否存在
        :param
        :return: 达梦软件信息
        """
        LOGGER.info("Start get app resource.")
        output = {"code": ErrCode.AUTH_INFO_ERR, "bodyErr": ErrCode.AUTH_INFO_ERR, "message": {}}
        app_result, _, _ = self.discover_application()
        if not app_result:
            LOGGER.error("Get application fail.")
            return output
        user_info = self.prepare_db_info_param(param_msg)
        version = get_version(user_info)
        if version == '':
            LOGGER.error("Get version fail.")
            return output
        # 查询实例信息
        param_info = param_msg.get("application", '')
        if param_info == '':
            LOGGER.info("Param file error get application fail.")
            return output
        else:
            subtype = param_info.get("subType", '')
            if subtype == ResourcesSubType.CLUSTER:
                output["code"] = 0
                output["bodyErr"] = 0
                output.get("message", {})["version"] = version
                return output
            elif subtype == ResourcesSubType.SINGLE_NODE:
                func_instance = DamengSingleNode()
                instance_info = func_instance.get_resource(param_msg, user_info)
                if instance_info.get("result"):
                    output["code"] = 0
                    output["bodyErr"] = 0
                    result_info = instance_info.get("info", {})
                    result_info["uuid"] = param_info.get("uuid")
                    result_info["version"] = version
                    result_info["bigVersion"] = instance_info.get("big_version")
                    output["message"] = str(result_info)
                    LOGGER.info("DamengApplication succ.")
                    return output
                else:
                    LOGGER.error(f"Get instance fail.")
                    output['code'] = instance_info.get("err_code", 200)
                    output['bodyErr'] = instance_info.get("err_code", 200)
                    return output
            else:
                LOGGER.error(f"DamengSubType error {subtype}.")
                return output

    def prepare_db_info_param(self, param_msg):
        inst_port = param_msg.get("application", {}).get("extendInfo", {}).get("port", "")
        if not verifying_special_characters(inst_port):
            LOGGER.error("Failed to check port.")
            return {}
        if not inst_port:
            LOGGER.error("Get port error.")
            return {}
        authtype = get_env_value(f"application_auth_authType_{JobData.PID}")
        if not authtype:
            LOGGER.error("Get authType fail.")
            return {}
        elif authtype == str(AuthType.OS_PASSWORD.value):
            user_info = {
                "port": inst_port,
                "auth_type": AuthType.OS_PASSWORD,
                "single_or_cluser": 'single',
                "is_connect_other_version": False
            }
        elif authtype == str(AuthType.APP_PASSWORD.value):
            env_user = f"application_auth_authKey_{JobData.PID}"
            env_pwd = f"application_auth_authPwd_{JobData.PID}"
            user_info = {
                "port": inst_port,
                "userkey": env_user,
                "pwdkey": env_pwd,
                "auth_type": AuthType.APP_PASSWORD,
                "single_or_cluser": 'single',
                "is_connect_other_version": False
            }
        else:
            LOGGER.error("Auth type param error.")
            return {}
        dmsql_tool = DmSqlTool(user_info)
        big_version = dmsql_tool.get_db_real_version()
        user_info["big_version"] = big_version
        return user_info


class DamengSingleNode(DamengSource):

    @exter_attack
    def get_resource(self, param_info_, user_info_):

        LOGGER.info("Start get dameng single node resource.")
        return self.pre_single_output(param_info_, user_info_)

    def pre_single_output(self, param_info_, user_info):
        single_output = {
            "result": False,
            "err_code": ErrCode.INSTANCE_REGISTER_ERROR,
            "info":
                {
                    "port": "",
                    "dbName": '',
                    "dbPath": '',
                    "dminiPath": '',
                }
        }
        dmini_path_list = self.get_all_dmini_path()
        for path in dmini_path_list:
            result = self.get_instancename_port(path)
            #2：结果中包含实例名和端口
            if len(result) < 2:
                continue
            instance_name, port = result[0], result[1]
            if param_info_.get("application", {}).get("extendInfo", {}).get('port', "") != port:
                continue
            errcode = query_auth_permisson(user_info)
            if errcode:
                single_output["err_code"] = errcode
                return single_output
            status_info, mode_info = query_db_status(user_info)
            if not status_info and not mode_info:
                single_output["err_code"] = ErrCode.AUTH_INFO_ERR
                return single_output
            is_online = 0
            if status_info == 'OPEN':
                is_online = 1
            if single_output.get("result", "") == "" or (not is_online):
                single_output["result"] = False
                single_output["err_code"] = ErrCode.DB_NOT_RUNNING
                return single_output
            return_type, db_name = get_db_name(user_info)
            if not return_type:
                return single_output
            single_output["result"] = True
            single_output["err_code"] = 0
            single_output["big_version"] = user_info.get("big_version")
            single_optpu_info = single_output.get("info", {})
            single_optpu_info["dbName"] = db_name
            single_optpu_info["port"] = port
            single_optpu_info["dbPath"] = "/".join(path.strip('\"').split('/')[0:-2])
            single_optpu_info["dminiPath"] = path.strip('\"')
            return single_output
        single_output["err_code"] = ErrCode.AUTH_INFO_ERR
        return single_output
