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
from common.const import ExecuteResultEnum
from mongodb import LOGGER
from mongodb.comm.const import MongoDBCode, ErrorCode, CLUSTER_ROLE_MAP, MongoRoles
from mongodb.comm.exception_err import DBAuthorizedError
from mongodb.service.base_service import MetaServiceWorker
from mongodb.service.resource.resource_ability import Resource


class MongoDBResource(MetaServiceWorker):

    def __init__(self, job_manager, param):
        super().__init__(job_manager, param)
        self._check_app_out = {
            "code": ExecuteResultEnum.INTERNAL_ERROR,
            "bodyErr": 0,
            "message": "Check application failed.",
            "bodyErrParams": []
        }
        self._query_cluster_out = {
            "id": "",
            "name": "",
            "type": "DataBase",
            "subType": "MongoDB-cluster",
            "endpoint": "",
            "role": 3,
            "nodes": [],
            "extendInfo": ""
        }

    @classmethod
    def update_extend_info(cls, extend_info, resource, shard_cluster_type):
        agent_list = resource.get_nodes_agent_list(shard_cluster_type)
        if agent_list:
            extend_info.update({
                "agent_nodes": agent_list
            })
        nodes = []
        if shard_cluster_type != "mongos" and shard_cluster_type != "single":
            nodes = resource.get_env_nodes()
        else:
            extend_info.update({
                "exist_nodes": "0"
            })
        if shard_cluster_type == "mongos":
            agent_shard_list = resource.get_agent_shard_list()
            extend_info.update({
                "agent_shard_list": agent_shard_list
            })
        for node in nodes:
            if node.get("extendInfo").get("exist_nodes") == "0":
                extend_info.update({
                    "exist_nodes": "0"
                })
                nodes = []
        return nodes

    @classmethod
    def auth_user(cls, resource):
        try:
            auth_type = resource.get_auth_type()
            if not auth_type:
                LOGGER.error("Get authType fail.")
                return False, ErrorCode.ERROR_PARAM.value, ""
            ret, result, error_param = resource.connect_cluster()
            if not ret:
                LOGGER.error("Connect cluster fail.")
                return ret, result, error_param
            ret, result, error_param = resource.auth_password_user()
            if not ret:
                LOGGER.error("Auth password user fail.")
                return ret, result, error_param
            ret, result, error_param = resource.auth_user_role()
            if not ret:
                LOGGER.error("Auth user role fail.")
                return ret, result, error_param
            ret, result, error_param = resource.check_bin_path(resource.url_arbiter)
            if not ret:
                LOGGER.error(f"Resource check bin path fail, url: {resource.url_arbiter}.")
                return ret, result, error_param
            return True, MongoDBCode.SUCCESS.value, ""
        except DBAuthorizedError as e:
            LOGGER.error(f"Not authorized on admin to connect resource! Error: {e}")
            return False, ErrorCode.USER_ROLE_PERMISSION_ERROR.value, "root clusterAdmin"
        except FileNotFoundError as ex:
            LOGGER.error(f"Resource check bin path fail, Error: {ex}.")
            return False, ErrorCode.INVALID_BIN_PATH.value, \
                resource.url_arbiter + ",mongo/mongos/mongod/mongodump/mongorestre"
        except Exception as ex:
            LOGGER.error(f"Query auth user failed, Error: {ex}, error type: {type(ex)}")
            return False, ErrorCode.ERROR_PARAM.value, "Check application error."

    @exter_attack
    def check_application(self):
        """
        检查连通性
        """
        resource = Resource(self.job_manager, self.param)
        ret, result, error_param = self.auth_user(resource)
        if not ret:
            return self.update_action_result(MongoDBCode.FAILED.value, result, "Check application failed.",
                                             err_param=error_param)
        return self.update_action_result(MongoDBCode.SUCCESS.value, MongoDBCode.SUCCESS.value,
                                         "Check application success.")

    @exter_attack
    def query_node_all_cluster(self):
        """
        获取节点上所有集群信息
        """
        self._query_cluster_out["name"] = "0"
        self.return_result = self._query_cluster_out
        self.update_result()
        resource = Resource(self.job_manager, self.param)
        try:
            ret, result, error_param = self.auth_user(resource)
            if not ret:
                LOGGER.error(f"Check node environment failed, PID: {self.job_manager.pid}.")
                self._query_cluster_out["extendInfo"] = {"errorCode": result, "errorParam": error_param}
                self.return_result = self._query_cluster_out
                return self.update_result()
            # 构造集群参数
            self.get_all_nodes_list(resource)
        finally:
            resource.clean()
            resource.close()
        return self.update_result()

    def get_all_nodes_list(self, resource):
        """
        获取节点上所有集群信息
        """
        shard_cluster_type = resource.get_node_server_type()
        cluster_type = resource.get_cluster_type()
        role = CLUSTER_ROLE_MAP.get(shard_cluster_type)
        if role != cluster_type:
            self._query_cluster_out["extendInfo"] = {
                "errorCode": ErrorCode.ENV_CLUSTER_TYPE_ERROR.value,
                "errorParam": resource.get_agent_host().split(":")[0],
            }
            self.return_result = self._query_cluster_out
            LOGGER.error(f"The host register type does not match the actual type. Actual type is {shard_cluster_type}.")
            return self.update_result()
        if not self.valid_single_inst(shard_cluster_type, resource):
            return self.update_result()
        version = resource.get_version()
        config_path = resource.get_config_path()
        argv = resource.get_start_cmd()
        parsed = resource.get_config_info()
        ret_instance_name = resource.get_ret_instance_name(shard_cluster_type)
        data_path = resource.get_data_path()
        local_host = resource.get_local_host(shard_cluster_type)
        agent_host = resource.get_agent_host()
        single_node_repl = shard_cluster_type if shard_cluster_type in (
            MongoRoles.SINGLE, MongoRoles.SINGLE_NODE_REPL) else ""
        extend_info = {
            "version": version,
            "shard_cluster_type": shard_cluster_type,
            "config_path": config_path,
            "data_path": data_path,
            "local_host": local_host,
            "exist_nodes": "1",
            "argv": argv,
            "parsed": parsed,
            "agent_host": agent_host,
            "ret_instance_name": ret_instance_name,
            "agent_shard_list": "",
            "single_node_repl": single_node_repl
        }
        nodes = self.update_extend_info(extend_info, resource, shard_cluster_type)
        self.build_query_cluster_out(extend_info, nodes)
        self.return_result = self._query_cluster_out
        LOGGER.info(f"Success get all nodes info, instance name is {ret_instance_name}.")
        return self.update_result()

    def build_query_cluster_out(self, extend_info, nodes):
        self._query_cluster_out["extendInfo"] = extend_info
        self._query_cluster_out["nodes"] = nodes
        self._query_cluster_out["subType"] = self.param.get("application").get("subType")
        self._query_cluster_out["name"] = "1"

    def valid_single_inst(self, shard_cluster_type, resource):
        # 校验单实例类型是否对应（单机/单节点副本集）
        if shard_cluster_type in (MongoRoles.SINGLE, MongoRoles.SINGLE_NODE_REPL):
            single_type = self.param.param_dict.get("application", "").get("extendInfo", "").get("singleType", "")
            if shard_cluster_type != single_type:
                self._query_cluster_out["extendInfo"] = {
                    "errorCode": ErrorCode.CHECK_OPEN_OPLOG_ERROR.value,
                    "errorParam": resource.get_agent_host().split(":")[0],
                }
                self.return_result = self._query_cluster_out
                return False
        LOGGER.info(f"Single instance is valid, single instance type is {shard_cluster_type}.")
        return True
