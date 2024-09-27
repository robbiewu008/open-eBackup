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

from common.common import exter_attack, output_execution_result_ex
from common.const import ParamConstant

from tidb.handle.resource.parse_params import ResourceParam
from tidb.handle.resource.resource_info import TiDBResourceInfo
from tidb.common.const import TiDBSubType, TiDBRegisterActionType, TiDBCode, ErrorCode
from tidb.common.tidb_common import ErrCodeException
from tidb.logger import log


class ResourceAbility:
    """
    资源接入相关接口
    """

    @staticmethod
    @exter_attack
    def check_application(req_id, job_id, sub_id, data):
        # 检查单个cluster、单个数据库、单个表或表集
        # 健康性检查
        log.info("Start to check_application.")
        params_from_pm = ResourceParam(req_id)
        action_type = params_from_pm.get_app_env_info().get("action_type")
        resource_info = TiDBResourceInfo(req_id, params_from_pm)
        log.info(f"action_type:{action_type}")
        check_ret = False
        if action_type == TiDBRegisterActionType.CHECK_CLUSTER:
            check_ret = resource_info.check_cluster()
        if action_type == TiDBRegisterActionType.CHECK_DB:
            check_ret = resource_info.check_database()
        if action_type == TiDBRegisterActionType.CHECK_TABLE:
            check_ret = resource_info.check_table()
        log.info(f"Action {action_type} result: {check_ret}!")
        log.info("End to check_application.")
        return check_ret

    @staticmethod
    @exter_attack
    def list_application_v2(req_id, job_id, sub_id, data):
        # 注册集群、数据库、表
        log.info("start to list_application_v2")
        params_from_pm = ResourceParam(req_id)
        action_type, conditions = params_from_pm.get_register_info()
        resource_info = TiDBResourceInfo(req_id, params_from_pm)
        log.info(f"action_type:{action_type}")
        if action_type == TiDBRegisterActionType.CHECK_TIUP_LIST_CLUSTER:
            resource_info.handle_tiup()
        if action_type == TiDBRegisterActionType.LIST_HOSTS:
            resource_info.list_cluster_hosts()
        if action_type == TiDBRegisterActionType.CHECK_USER:
            resource_info.handle_user()
        if action_type == TiDBRegisterActionType.LIST_DB:
            resource_info.handle_db()
        if action_type == TiDBRegisterActionType.LIST_TABLE:
            resource_info.handle_table()
        log.info("End to list_application_v2")

    @staticmethod
    @exter_attack
    def query_cluster(req_id, job_id, sub_id, data):
        log.info("Start to query_cluster.")
        params_from_pm = ResourceParam(req_id)
        resource_info = TiDBResourceInfo(req_id, params_from_pm)
        check_ret = resource_info.check_cluster()
        log.info("End to query_cluster.")
        return check_ret

    @staticmethod
    def write_error_param_to_result_file(path: str, code: int, error_code: int, message: str):
        output_execution_result_ex(path, {"code": code, "bodyErr": error_code, "message": message})
