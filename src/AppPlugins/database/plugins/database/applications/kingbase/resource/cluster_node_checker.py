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
from common.common import output_execution_result, execute_cmd, exter_attack
from common.common_models import (ActionResult, QueryClusterResponse, QueryClusterResponseExtendInfo,
                                  ClusterNodeExtendInfo, ClusterNodeInfo)
from common.const import ParamConstant, ExecuteResultEnum, RoleType, SysData
from common.logger import Logger
from common.parse_parafile import ParamFileUtil, get_env_variable
from common.util import check_user_utils
from kingbase.common.error_code import ErrorCode, BodyErr
from kingbase.common.util.get_html_result_utils import execute_cmd_and_parse_res
from kingbase.common.util.resource_util import get_version, domain_2_ip, check_special_character, \
    check_is_path_exists, check_black_list, check_os_name

LOGGER = Logger().get_logger("kingbase.log")


class ClusterNodesChecker:

    def __init__(self, func_type, request_pid):
        self.func_type = func_type
        self.pid = request_pid
        self.context = ParamFileUtil.parse_param_file(self.pid)
        self.application = self.context.get("application", {})
        self.env = self.context.get("appEnv", {})
        self.extend_info = self.application.get("extendInfo", {})
        self.env_extend = self.env.get("extendInfo", {})
        self.os_username = self.extend_info.get("osUsername")
        self.client_path = os.path.join(self.extend_info.get("clientPath"), "bin", "ksql")
        self.repmgr_path = os.path.join(self.extend_info.get("clientPath"), "bin", "repmgr")
        self.result_file = os.path.join(ParamConstant.RESULT_PATH, f"result{self.pid}")
        self.nodes = self.env_extend.get("allNodes")
        self.port = self.extend_info.get("instancePort")
        self.service_ip = self.extend_info.get("serviceIp")
        self.install_path = self.extend_info.get("clientPath")
        self.database_mode = self.extend_info.get("databaseMode")
        self.version = None
        self.node_info = None

    @staticmethod
    def parse_cluster_info(cluster_info):
        info = []
        for line in cluster_info.strip().split(os.linesep)[2:]:
            tmp_info = line.split('|')
            dict_info = {
                "ID": tmp_info[0].strip(),
                "Name": tmp_info[1].strip(),
                "Role": tmp_info[2].strip(),
                "Status": tmp_info[3].strip(),
                "Upstream": tmp_info[4].strip(),
                "Location": tmp_info[5].strip(),
                "Priority": tmp_info[6].strip(),
                "Timeline": tmp_info[7].strip(),
                "ConnectionString": {i.split('=')[0]: i.split('=')[1] for i in tmp_info[-1].strip().split()}
            }
            info.append(dict_info)
        LOGGER.info(f"Get cluster node info: {info} successful.")
        return info

    @exter_attack
    def query_cluster(self):
        param = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR.value, bodyErr=ErrorCode.CHECK_CLUSTER_FAILED,
                             message="check cluster failed!")
        try:
            param = self.run()
        except Exception as ex:
            param = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR.value, bodyErr=ErrorCode.CHECK_CLUSTER_FAILED,
                                 message="An exception occur when check cluster failed.")
            LOGGER.error(f"Failed to check cluster! pid: {self.pid}.", ex)
        finally:
            clear(SysData.SYS_STDIN)
            LOGGER.info(f"Finally to output check cluster result, pid: {self.pid}, param: {param.dict(by_alias=True)}")
            output_execution_result(self.result_file, param.dict(by_alias=True))

    def run(self):
        check_special_character([self.client_path, self.repmgr_path, self.os_username])
        err_code, msg = self.get_all_info()
        if err_code != ErrorCode.SUCCESS:
            return ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR.value, bodyErr=err_code, message=msg)

        if self.func_type in ['CheckApplication']:
            LOGGER.info(f"Begin to check cluster by os param: {self.context}, pid: {self.pid}")
            return self.check_cluster_nodes()
        else:
            LOGGER.info(f"Begin to query cluster nodes info by os params: {self.context}, pid: {self.pid}")
            return self.query_nodes_info()

    def check_cluster_nodes(self):
        """检查输入节点信息是否正确"""
        check_code = self.check_node_info(self.node_info)
        if check_code != ErrorCode.SUCCESS:
            return ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR.value, bodyErr=check_code,
                                message="Nodes not belong to a cluster.")
        if not self.database_mode:
            LOGGER.info("Begin to get database mode!")
            res, self.database_mode = self.get_database_mode()
            if res != ErrorCode.SUCCESS or not self.database_mode:
                return ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR.value, bodyErr=check_code,
                                    message="Get database mode failed.")
        return ActionResult(code=ExecuteResultEnum.SUCCESS.value, bodyErr=check_code,
                            message=json.dumps({"version": self.version, "databaseMode": self.database_mode}))

    def get_database_mode(self):
        db_user_name = get_env_variable(f'application_auth_authKey_{self.pid}')
        check_special_character([db_user_name, self.client_path])
        cmd = f"su - {self.os_username} -c '{self.client_path} -U {db_user_name} " \
              f"-h {self.service_ip} -p {self.port} -d test -W -H -c \"show database_mode;\"'"
        return execute_cmd_and_parse_res(self.pid, cmd)

    def query_nodes_info(self):
        resp = QueryClusterResponse(name=self.env.get('name'), subType='KingBaseClusterInstance')
        resp.extend_info = QueryClusterResponseExtendInfo(clusterVersion=self.version)
        for node in self.node_info:
            tmp_node_extend = ClusterNodeExtendInfo()
            tmp_node_extend.role = RoleType.PRIMARY.value if node.get('Role') == 'primary' else RoleType.STANDBY.value
            tmp_node_extend.status = node.get("Status")
            tmp_node = ClusterNodeInfo(name=node.get('Name'), endpoint=node.get('ConnectionString', {}).get('host'),
                                       subType=self.application.get('subType'), extendInfo=tmp_node_extend)
            resp.nodes.append(tmp_node)

        LOGGER.info(f"Get the cluster info: {resp.dict(by_alias=True)}")
        return resp

    def get_all_info(self):
        if not os.path.exists(self.client_path) or not os.path.exists(self.repmgr_path):
            return ErrorCode.CLIENT_PATH_IS_NOT_EXIST, "Client path is not exist!"

        if not check_os_name(self.os_username, self.client_path):
            return ErrorCode.OS_USERNAME_ERROR, "Check connectivity failed!"

        # 获取数据库版本信息
        err_code, self.version = get_version(self.pid, self.client_path, self.os_username)
        if err_code != ErrorCode.SUCCESS:
            return err_code, "Get version failed!"

        # 获取集群节点信息
        ret_code, self.node_info = self.get_cluster_info()
        if ret_code != ErrorCode.SUCCESS:
            return ret_code, "Get cluster nodes info failed!"
        return ErrorCode.SUCCESS, ''

    def get_cluster_info(self):
        node_info = None
        check_is_path_exists(self.repmgr_path)
        if not check_user_utils.check_path_owner(self.repmgr_path, [self.os_username]):
            LOGGER.error("Repmgr path and os username is not matching!")
            return ErrorCode.GET_CLUSTER_NODE_FAILED, node_info
        get_cluster_info_cmd = f"su - {self.os_username} -c '{self.repmgr_path} cluster show'"
        err_code, stdout, stderr = execute_cmd(get_cluster_info_cmd)
        if err_code != '0':
            LOGGER.error(f"Failed to get cluster node information, err_code: {err_code}, err_msg: {stderr}")
            return ErrorCode.GET_CLUSTER_NODE_FAILED, node_info
        node_info = self.parse_cluster_info(stdout)
        return ErrorCode.SUCCESS, node_info

    def check_node_info(self, node_info):
        actual_all_nodes = []
        for node in node_info:
            host = node.get("ConnectionString", {}).get('host')
            port = node.get("ConnectionString", {}).get('port')
            if host and port:
                actual_all_nodes.append(f"{domain_2_ip(host)}:{port}")
        input_nodes = set(self.nodes.split(','))
        actual_nodes = set(actual_all_nodes)
        if actual_nodes != input_nodes:
            LOGGER.error(f"The input nodes: {input_nodes} info is not a cluster, pls check")
            return ErrorCode.CHECK_CLUSTER_FAILED
        LOGGER.info(f"Success to check cluster pid: {self.pid}")
        return ErrorCode.SUCCESS


@exter_attack
def entrance():
    if len(sys.argv) != 3:
        LOGGER.error("Number of argv wrong. ")
        return BodyErr.ERROR_COMMON_INVALID_PARAMETER

    LOGGER.debug(f"args: {sys.argv}")
    func_type = sys.argv[1]
    pid = sys.argv[2]
    SysData.SYS_STDIN = sys.stdin.readline()

    if func_type not in ("CheckApplication", "QueryCluster", "QueryHostCluster", "QueryAppCluster"):
        LOGGER.error(f"Unknown command: {func_type} from command line!")
        return BodyErr.ERROR_COMMON_INVALID_PARAMETER

    checker = ClusterNodesChecker(func_type, pid)
    check_special_character([checker.repmgr_path, checker.client_path])
    check_black_list([checker.repmgr_path, checker.client_path])
    checker.query_cluster()
    return BodyErr.SUCCESS


if __name__ == '__main__':
    sys.exit(entrance())
