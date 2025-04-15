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

import copy
import sys

from common.common import output_result_file, exter_attack
from common.const import ParamConstant, DeployType
from common.parse_parafile import ParamFileUtil
from common.util.check_utils import is_valid_id
from openGauss.common.common import set_uuid, path_check
from openGauss.common.const import SOURCE_RESULT, BASE_RET, OpenGaussType, logger, SUCCESS_RET, \
    Status, OpenGaussDeployType
from openGauss.common.error_code import BodyErr, OpenGaussErrorCode
from openGauss.common.opengauss_param import JsonParam
from openGauss.common.param_struct import ParamStruct
from openGauss.resource.cluster_instance import GaussCluster


class Resource:
    def __init__(self, pid, param_obj: ParamStruct):

        self.pid = pid
        self.auth = param_obj.env_auth
        self.env_path = param_obj.app_env_path
        self.deploy_type = param_obj.app_deploy_type
        self.param = param_obj
        self.cluster_version = param_obj.app_version
        self.dcs_address = param_obj.app_dcs_address
        self.dcs_port = param_obj.app_dcs_port
        self.dcs_user = param_obj.app_dcs_user
        self.dcs_pass = param_obj.dcs_pass
        self.cluster = GaussCluster(self.auth, self.env_path, self.cluster_version)

    def get_cluster_nodes(self):
        if self.cluster.nodes:
            return self.cluster.nodes
        if not self.cluster.cmd_obj:
            logger.info("Create cmd obj failed, check params.")
            return []
        self.cluster.get_cluster_nodes()
        return self.cluster.nodes

    @staticmethod
    def check_deploy_type(deploy_type, output):
        output['message'] = "not camper with the deploy type ,check it"
        # 当集群本身为单机时
        if str(deploy_type) == OpenGaussDeployType.SINGLE:
            output['bodyErr'] = OpenGaussErrorCode.DB_TYPE_AND_AGENTS_NOT_MATCH
        else:
            output['bodyErr'] = OpenGaussErrorCode.CHECK_CLUSTER_FAILED
        return output

    @exter_attack
    def check_application(self):
        """
        check the user name(authKey) is the same as database user name
        """
        output = copy.deepcopy(BASE_RET)
        if not self.cluster.check_env_path():
            output['message'] = "Check env path failed."
            output['bodyErr'] = BodyErr.ERROR_ENVPATH_NOT_EXIST
            logger.error("Check env path failed.")
            return output
        if not self.cluster.check_db_user():
            output['message'] = "Check user failed"
            output['bodyErr'] = BodyErr.ERROR_USER_NOT_EXIST_CLUSTER
            logger.error("Check db user failed.")
            return output
        ret = self.cluster.check_db_version()
        if not ret:
            output['message'] = "Failed execute db command  with db user and env path,check it."
            output['bodyErr'] = BodyErr.ERROR_DB_REGISTER_INFO
            logger.error("Failed execute db command  with db user and env path,check it")
            return output
        ret = self.cluster.check_endpoint(self.param.gui_nodes)
        if not ret:
            output['message'] = "Check endpoint error."
            output['bodyErr'] = BodyErr.ERROR_CLUSTER_NODES_INCONSISTENT
            logger.error("Check endpoint failed.")
            return output
        deploy_type = self.cluster.deploy_type
        if str(deploy_type) != self.deploy_type:
            logger.error("Check deploy type failed.")
            return self.check_deploy_type(deploy_type, output)
        if self.cluster.cluster_state not in (Status.NORMAL, Status.DEGRADED):
            output['message'] = "Check cluster status"
            output['bodyErr'] = OpenGaussErrorCode.ERR_DATABASE_STATUS
            logger.error("Cluster state is not normal!")
            return output
        if self.deploy_type == OpenGaussDeployType.DISTRIBUTED:
            monitor_ret, cont = self.cluster.cmd_obj.check_cmdb_user(self.dcs_user, self.dcs_pass,
                                                                     self.cluster.get_instance_port())
            if not monitor_ret:
                output['message'] = "Check cmdb user and password"
                output['bodyErr'] = OpenGaussErrorCode.ERR_CMDB_WRONG_DATABASE_PWD
                logger.error(f"Check cmdb user and password failed! Reason is {cont}")
                return output
            dcs_ret = self.cluster.check_dcs(self.dcs_address, self.dcs_port, self.param.gui_nodes)
            if not dcs_ret:
                output['message'] = "Check cmdb dcs"
                output['bodyErr'] = OpenGaussErrorCode.ERROR_DCS_NOT_SUITABLE
                logger.error("Check cmdb dcs failed!")
                return output
        output["code"] = SUCCESS_RET
        return output

    @exter_attack
    def get_cluster_info(self):
        """
        get all cluster information
        """
        all_nodes = self.get_cluster_nodes()
        output = copy.deepcopy(SOURCE_RESULT)
        nodes = []
        if not all_nodes:
            return output
        for this_node in all_nodes:
            extend_info = {
                'status': this_node.instance_state,
                'role': this_node.role_type
            }
            node = {
                'endpoint': this_node.node_ip,
                'extendInfo': extend_info,
                'name': this_node.node_name,
                'subType': OpenGaussType.SUBTYPE,
                'type': OpenGaussType.TYPE,
                'uuid': "",
                "systemId": self.cluster.get_system_identifier()
            }
            nodes.append(node)
        output['nodes'] = nodes
        output['name'] = self.cluster.host_name
        output['endpoint'] = self.cluster.get_endpoint_by_hostname()
        output['extendInfo'] = {
            "clusterState": self.cluster.cluster_state,
            "clusterVersion": self.cluster.db_version,
            "deployType": self.cluster.deploy_type,
            "syncMode": self.cluster.sync_state,
            "systemId": self.cluster.get_system_identifier()
        }
        return output

    @exter_attack
    def get_applictaion_resources(self):
        """
        获取资源列表
        :return:
        """
        resources = []
        nodes = self.get_cluster_nodes()
        if not nodes:
            return {"resourceList": []}

        uid = self.param.application.id
        name = self.param.application.name
        node = self.cluster.prim_node
        if not node:
            return {"resourceList": []}
        cluster_name = self.cluster.cluster_name if self.cluster.cluster_name else name
        instance_id = set_uuid(self.cluster.get_instance_data_path(), self.cluster.get_instance_port(), uid)
        # 取环境id后4位为当前instance标识
        instance_tag = uid[-4:]
        instance_name = f"{cluster_name}_{instance_tag}_{node.instance_id}"
        new_instance = {
            "type": OpenGaussType.TYPE,
            "subType": "OpenGauss-instance",
            "name": instance_name,
            "parentId": uid,
            "parentName": name,
            "extendInfo": {"instanceState": node.instance_state,
                           "clusterState": self.cluster.cluster_state,
                           "clusterVersion": self.cluster.db_version,
                           "deployType": self.cluster.deploy_type,
                           "syncMode": self.cluster.sync_state,
                           },
            "id": instance_id

        }
        # 如果是CMDB分布式，再添加上archiveMode字段
        if new_instance.get("extendInfo").get("deployType") == DeployType.SHARDING_TYPE.value:
            new_instance["extendInfo"]["archiveMode"] = node.get_archive_mode(self.param.env_auth,
                                                                              self.param.app_env_path)
        resources.append(new_instance)
        databases = node.get_databases(self.param.env_auth, self.param.app_env_path)
        for db in databases:
            new_db = {
                "type": OpenGaussType.TYPE,
                "subType": "OpenGauss-database",
                "name": db,
                "parentId": instance_id,
                "parentName": instance_name,
                "extendInfo": {"deployType":self.cluster.deploy_type},
                "id": set_uuid(db, instance_name, instance_id)
            }
            resources.append(new_db)
        return {"resourceList": resources}


@exter_attack
def run():
    if len(sys.argv) < 3:
        logger.error("Number of argv wrong. ")
        return BodyErr.ERROR_COMMON_INVALID_PARAMETER
    func_type = sys.argv[1]
    pid = sys.argv[2]
    # 添加id校验
    if not is_valid_id(pid):
        logger.warn(f'pid is invalid!')
        return BodyErr.ERROR_COMMON_INVALID_PARAMETER
    try:
        parm_cont_dict = JsonParam.parse_param_with_jsonschema(pid)
    except Exception as e:
        logger.error("Parse param file failed as %s.", str(e))
        return BodyErr.ERROR_COMMON_INVALID_PARAMETER
    if not parm_cont_dict:
        logger.error("Param is none.")
        return BodyErr.ERROR_COMMON_INVALID_PARAMETER

    if func_type not in ("CheckApplication", "QueryHostCluster", "ListApplicationResource"):
        logger.error("Unknown command from command line!")
        return BodyErr.ERROR_COMMON_INVALID_PARAMETER

    ret = path_check(ParamConstant.RESULT_PATH)
    if not ret:
        logger.error("Result dir not exists.")
        return BodyErr.ERROR_FILE_NOT_EXIST

    param = ParamStruct(parm_cont_dict, pid)
    resource = Resource(pid, param)
    execute_func = {
        "CheckApplication": resource.check_application,
        "ListApplicationResource": resource.get_applictaion_resources,
        "QueryHostCluster": resource.get_cluster_info
    }
    res_exec = execute_func.get(func_type)
    try:
        output = res_exec()
        logger.info(f"Excute cmd {func_type} finish.")
    except Exception as e:
        logger.error(f"Execute cmd {func_type} with exception. Exception: {e}")
        return OpenGaussErrorCode.OPERATION_FAILED
    try:
        output_result_file(pid, output)
    except Exception as e:
        logger.error("Write result failed with error")
        return OpenGaussErrorCode.OPERATION_FAILED
    return SUCCESS_RET


if __name__ == '__main__':
    sys.exit(run())
