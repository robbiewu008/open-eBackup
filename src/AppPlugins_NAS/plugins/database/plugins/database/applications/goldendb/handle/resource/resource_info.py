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

from goldendb.logger import log
from common.cleaner import clear
from common.common import check_command_injection_exclude_quote, execute_cmd, execute_cmd_list
from common.const import SysData
from common.util.check_user_utils import check_os_user
from goldendb.handle.common.const import CMDResult, ClusterInfoStr, ErrorCode, GoldenDBSupportVersion, \
    GoldenDBNodeStatus, GoldenDBNodeType, GoldenDBNodeService
from goldendb.handle.common.exec_sql_cmd import get_mysql_db_session
from goldendb.handle.common.goldendb_exception import ErrCodeException
from goldendb.handle.resource.parse_params import ResourceParam

SINGLE_GRP_FILTER: bool = True


def get_env_variable(str_env_variable: str):
    env_variable = ''
    input_str = json.loads(SysData.SYS_STDIN)
    if input_str.get(str_env_variable):
        env_variable = input_str.get(str_env_variable)
    return env_variable


class GoldenDBResourceInfo:

    def __init__(self, pid, resource_param: ResourceParam):
        self.pid = pid
        self.param = resource_param

    @staticmethod
    def get_cluster_ids(os_user):
        """
        执行命令：su - zxmanager -c 'dbtool -cm -qcg'查看goldenDB集群的所有id
        """
        if check_command_injection_exclude_quote(os_user) or not check_os_user(os_user):
            log.error("command injection in os user! The os user(%s) is invalid.", os_user)
            return ''
        cmd = f'su - {os_user} -c "dbtool -cm -qcg"'
        return_code, out_info, err_info = execute_cmd(cmd)
        if return_code != CMDResult.SUCCESS:
            log.error("Get cluster error!")
            return ''
        return out_info

    @staticmethod
    def get_cluster_version(os_user):
        """
        执行命令：su - zxmanager -c 'dbtool -version'查看goldenDB集群版本
        """
        if check_command_injection_exclude_quote(os_user) or not check_os_user(os_user):
            log.error("command injection in os user! The os user(%s) is invalid.", os_user)
            return ''
        cmd = f'su - {os_user} -c "dbtool -version"'
        return_code, out_info, err_info = execute_cmd(cmd)
        if return_code != CMDResult.SUCCESS:
            log.error("Get cluster version error!")
            return ''
        for version in GoldenDBSupportVersion:
            if version.value in out_info:
                log.info(f"Get cluster version {version.value} success!")
                return version.value
        return ''

    @staticmethod
    def parse_output_to_cluster_ids(os_user):
        """
        解析出cluster_id的set集合
        """
        output_info = GoldenDBResourceInfo.get_cluster_ids(os_user)
        if not output_info:
            return set()
        # ClusterID:3,GroupID:2 ClusterID:3,GroupID:1 ClusterID:4,GroupID:1 ~success~
        cluster_ids_and_groups = output_info[output_info.find("ClusterID:"):output_info.rfind(":") + 2]
        # ClusterID:3,GroupID:2\nClusterID:3,GroupID:1\nClusterID:4,GroupID:1
        id_and_group_list = cluster_ids_and_groups.split("\n")
        ids_set = set()
        for id_and_group in id_and_group_list:
            if not id_and_group:
                continue
            # 拿到ClusterID:3,GroupID:2 拿到: 后面的ClusterID
            ids_set.add(id_and_group.split(",")[0][id_and_group.find(":") + 1:])
        log.info(f"cluster ids set: {ids_set}")
        return ids_set

    @staticmethod
    def parse_cluster_info(group_info):
        """
        解析出一个cluster的详细信息
        """
        # 1、先根据----------------------------将cluster信息和group信息分割开
        cluster_info_list = group_info.split("----------------------------")
        cluster_info_lines = cluster_info_list[0].split("\n")
        cluster_info = dict()
        single_grp_flag = ''
        for line_info in cluster_info_lines:
            if ClusterInfoStr.CLUSTER_ID in line_info:
                cluster_info["id"] = line_info[line_info.rfind(":") + 1:]
            if ClusterInfoStr.CLUSTER_NAME in line_info:
                cluster_info["name"] = line_info[line_info.rfind(":") + 1:]
            if ClusterInfoStr.SINGLE_GRP in line_info:
                single_grp_flag = line_info[line_info.rfind(":") + 1:]

        group_list = GoldenDBResourceInfo.parse_group_info(cluster_info_list)
        cluster_info["group"] = group_list
        return cluster_info, single_grp_flag

    @staticmethod
    def parse_group_info(cluster_info_list):
        # 2、根据Intermediate response:将每个group分片的信息分割开
        group_list = []
        group_infos = cluster_info_list[1].split("Intermediate response:")

        # 3、使用------------------------将group信息和nodes信息分割开
        for index_i, group_node_info in enumerate(group_infos):
            # 舍弃截取的第一部分类容：[02-07 15:54:41:410]
            if index_i == 0:
                continue
            group_info_lines = group_node_info.split("------------------------")
            group_info = dict()
            mysql_node_list = list()
            for index_j, item in enumerate(group_info_lines):
                mysql_node = dict()
                # 4、封装group信息和node信息
                GoldenDBResourceInfo.parse_node_info(index_j, item, group_info, mysql_node)
                # j = 0 时，查询的是group信息，j != 0 时查询的mysql node信息
                if index_j != 0:
                    mysql_node["linkStatus"] = GoldenDBNodeStatus.ONLINE
                    mysql_node["nodeType"] = GoldenDBNodeType.DATA_NODE
                    mysql_node_list.append(mysql_node)
            group_info["mysqlNodes"] = mysql_node_list
            group_list.append(group_info)
        return group_list

    @staticmethod
    def parse_node_info(index_j, item, group_info, mysql_node):
        """
        封装group信息和node信息
        """
        for group_info_line in item.split("\n"):
            if index_j == 0:
                # 封装group信息 每一行的形式都是以group id:2此形式
                if ClusterInfoStr.GROUP_ID in group_info_line:
                    group_info["groupId"] = group_info_line[group_info_line.rfind(":") + 1:]
                if ClusterInfoStr.DATABASE_NUM in group_info_line:
                    group_info["databaseNum"] = group_info_line[group_info_line.rfind(":") + 1:]
            else:
                # 封装MySQLNode信息
                if ClusterInfoStr.NODE_ID in group_info_line:
                    mysql_node["id"] = group_info_line[group_info_line.rfind(":") + 1:]
                if ClusterInfoStr.NODE_NAME in group_info_line:
                    mysql_node["name"] = group_info_line[group_info_line.rfind(":") + 1:]
                if ClusterInfoStr.NODE_ROLE in group_info_line:
                    mysql_node["role"] = group_info_line[group_info_line.rfind(":") + 1:]
                if ClusterInfoStr.NODE_IP in group_info_line:
                    ip_info = group_info_line.split(":")
                    mysql_node["ip"] = ip_info[1]
                    mysql_node["port"] = ip_info[2]

    @staticmethod
    def check_os_user_exist(os_user):
        """
        使用命令 id -u root检查用户是否存在
        :param os_user: 用户输入的操作系统用户
        :return:True 存在 False不存在
        """
        if check_command_injection_exclude_quote(os_user):
            log.error("command injection in os_user!")
            return False
        cmd = f"id -u {os_user}"
        return_code, out_info, err_info = execute_cmd(cmd)
        if return_code != CMDResult.SUCCESS:
            log.error("The os user is not exist!")
            return False
        return True

    @staticmethod
    def check_os_user_legal(os_user, port):
        """
        检查用户输入的操作系统的是否正确
        :param port: 端口号
        :param os_user: 用户输入的操作系统用户
        :return: True 正确 False不正确
        """
        if check_command_injection_exclude_quote(os_user):
            log.error("command injection in os_user!")
            return False
        # 查询含有my.cnf 和 os_user的进程
        cmd_get_mysql_cnf = ["ps -ef", f"grep {os_user}", "grep defaults-file"]
        return_code, out_info, std_err = execute_cmd_list(cmd_get_mysql_cnf)
        if return_code != CMDResult.SUCCESS:
            log.error("Get mysql cnf by os user failed!")
            return False
        # 拿到cnf文件路径
        cnf_path = ""
        for out_info_line in out_info.split(" "):
            if "--defaults-file=" in out_info_line and f"/{os_user}/" in out_info_line:
                cnf_path = out_info_line.split("=")[1]
        if check_command_injection_exclude_quote(cnf_path):
            log.error("command injection detected in command!")
            return False
        if not cnf_path:
            log.error("Get mysql cnf path by os user failed!")
            return False
        # 查看端口号是否在配置文件中
        cat_cnf_cmd = f"cat {cnf_path}"
        return_code, out_info, std_err = execute_cmd_list([cat_cnf_cmd])
        if return_code != CMDResult.SUCCESS:
            log.error("The get mysql cnf by os user failed!")
            return False
        if port not in out_info:
            log.error("The OS user is not legal!")
            return False
        return True

    @staticmethod
    def check_service_status(os_user, node_type):
        """
        检查管理节点，gtm节点，dn zxmanager节点的服务是否运行
        @:param: os_user zxmanager用户 GTM节点用户 DN节点用户
        :return: True 正常 False不正常
        """
        if check_command_injection_exclude_quote(os_user) or not check_os_user(os_user):
            log.error("command injection in os user! The os user(%s) is invalid.", os_user)
            return []
        cmd = f"su - {os_user} -c 'dbstate'"
        return_code, out_info, err_info = execute_cmd(cmd)
        # 获取当前节点必须存在的服务列表
        service_list = GoldenDBNodeService.SERVICE_DICT.get(node_type)
        if return_code != CMDResult.SUCCESS:
            log.error(f"The execute get {node_type} service cmd failed!")
            return service_list
        service_error_list = []
        for service_status in out_info.split("\n"):
            for service in service_list:
                if service in service_status and "not running" in service_status:
                    log.error(f"The {node_type} service {service_status} failed!")
                    service_error_list.append(service)
        return service_error_list

    @staticmethod
    def check_node_type(os_user, node_type):
        """
        检查管理节点，gtm节点，dn zxmanager节点类型
        检某个节点某个服务命令：dbtool -{某个服务名称} -state
        @:param: os_user zxmanager用户 GTM节点用户 DN节点用户
        :return: True 正常 False不正常
        """
        # 获取当前节点必须存在的服务
        service_list = GoldenDBNodeService.SERVICE_DICT.get(node_type)
        if check_command_injection_exclude_quote(os_user) or not check_os_user(os_user):
            log.error("command injection in os user! The os user(%s) is invalid.", os_user)
            return False
        cmd = f"su - {os_user} -c 'dbstate'"
        return_code, out_info, err_info = execute_cmd(cmd)
        if service_list[0] not in out_info:
            log.error(f"Check node type {node_type} failed")
            return False
        log.info(f"Check node type {node_type} success")
        return True

    @staticmethod
    def parse_gtm_info(gtm_info):
        def parse_gtm_cluster_info(idx, gtm_infos, gtm_cluster):
            # 获取gtm_cluster id和Godlendb-cluster id的映射关系
            while not gtm_infos[idx].startswith("*") and gtm_infos[idx].strip() and gtm_infos[idx][0].isalnum():
                gtm_infos_temp = gtm_infos[idx].split()
                if len(gtm_infos_temp) == 2:
                    gtm_cluster_id = gtm_infos_temp[0]
                    cluster_ids = gtm_infos_temp[1]
                    cluster_list = cluster_ids.split(",")
                    for cluster_id in cluster_list:
                        gtm_cluster[cluster_id] = gtm_cluster_id
                idx += 1
            return idx, gtm_cluster

        def parse_gtm_grp_conf_info(idx, gtm_infos):
            # 处理第二部分GTM信息数据
            while idx < len(gtm_infos) and not gtm_infos[idx].startswith("*") and gtm_infos[idx].strip():
                idx += 1
            return idx

        def parse_gtm_basic_info(idx, gtm_infos, gtm_cluster):
            # 获取Godlendb-cluster id和Gtm节点信息
            cluster_gtm_dic = dict()
            temp_dic = dict()
            while idx < len(gtm_infos) and not gtm_infos[idx].startswith("*") and gtm_infos[idx].strip() and \
                    gtm_infos[idx][0].isalnum():
                gtm_cluster_id, _, _, gtm_id, gtm_ip, listen_port, _, _, _, master_flag, status = gtm_infos[idx].split()
                cluster_gtm_info = {"gtmId": gtm_id, "gtmIp": gtm_ip, "port": listen_port, "masterFlag": master_flag}
                if gtm_cluster_id not in temp_dic.keys():
                    temp_dic[gtm_cluster_id] = [cluster_gtm_info]
                else:
                    temp_dic[gtm_cluster_id].append(cluster_gtm_info)
                idx += 1

            for cluster_id in gtm_cluster.keys():
                temp_dic_info = temp_dic.get(gtm_cluster[cluster_id], list())
                temp_dic_info = sorted(temp_dic_info, key=lambda x: int(x['gtmId']))
                cluster_gtm_dic[cluster_id] = temp_dic_info
            return cluster_gtm_dic

        gtm_infos = gtm_info.split("\n")
        gtm_info_idx = 1
        gtm_cluster = dict()
        cluster_gtm_dic = dict()
        # 防止不同版本gtm信息顺序不同， 这里循环3次，获取不同的gtm信息
        while gtm_info_idx < len(gtm_infos):
            if gtm_infos[gtm_info_idx].__contains__("GTM Cluster Information"):
                gtm_info_idx, gtm_cluster = parse_gtm_cluster_info(gtm_info_idx + 2, gtm_infos, gtm_cluster)
            gtm_info_idx += 1

        gtm_info_idx = 1
        while gtm_info_idx < len(gtm_infos):
            if gtm_infos[gtm_info_idx].__contains__("GTM Group Config Information"):
                gtm_info_idx = parse_gtm_grp_conf_info(gtm_info_idx + 1, gtm_infos)
            gtm_info_idx += 1

        gtm_info_idx = 1
        while gtm_info_idx < len(gtm_infos):
            if gtm_infos[gtm_info_idx].__contains__("GTM Basic Information"):
                cluster_gtm_dic = parse_gtm_basic_info(gtm_info_idx + 2, gtm_infos, gtm_cluster)
            gtm_info_idx += 1

        return cluster_gtm_dic

    def get_clusters(self):
        """
        执行命令su - zxmanager -c 'dbtool -cm -qc 3'查看集群的详细信息并返回结果
        """
        os_user = self.param.get_os_user_when_brows()
        # 获取操作用户失败
        if not os_user:
            log.error("Get os user from PM failed.")
            return list()
        if check_command_injection_exclude_quote(os_user) or not check_os_user(os_user):
            log.error("command injection in os user! The os user(%s) is invalid.", os_user)
            return list()
        cluster_ids_set = GoldenDBResourceInfo.parse_output_to_cluster_ids(os_user)
        # 执行命令dbtool -mds -showgtminfos 查看集群GTM使用信息
        cmd = f"su - {os_user} -c 'dbtool -mds -showgtminfos'"
        return_code, gtm_info, err_info = execute_cmd(cmd)
        if return_code != CMDResult.SUCCESS:
            log.error(f"Get gtm info by exec cmd error!")
            return list()
        cluster_gtm_info = GoldenDBResourceInfo.parse_gtm_info(gtm_info)
        log.info("query cluster gmt info success")
        cluster_list = list()
        for cluster_id in cluster_ids_set:
            cmd = f"su - {os_user} -c 'dbtool -cm -qc {cluster_id}'"
            return_code, out_info, err_info = execute_cmd(cmd)
            # 执行命令失败
            if return_code != CMDResult.SUCCESS:
                log.error(f"Get cluster{cluster_id} by exec cmd error!")
                return list()
            cluster_info, single_grp_flag = GoldenDBResourceInfo.parse_cluster_info(out_info)
            # 解析某个集群实例失败
            if not cluster_info:
                log.error(f"Parse cluster{cluster_id} error!")
                return list()
            cluster_info["gtm"] = cluster_gtm_info.get(cluster_id, list())
            if SINGLE_GRP_FILTER:
                if single_grp_flag == '0':
                    cluster_list.append(cluster_info)
            else:
                cluster_list.append(cluster_info)
        log.info(f"cluster_list size: {cluster_list.__len__()}")
        return cluster_list

    def check_node_status(self):
        """
        检测每个节点的MySQL连通性
        """
        resource_param = self.param
        user = get_env_variable(f"application_auth_authKey_{self.pid}")
        pwd = get_env_variable(f"application_auth_authPwd_{self.pid}")
        node_info = resource_param.get_node_info()
        node_type = node_info.get("nodeType")
        host_ip = node_info.get("ip")
        os_user = node_info.get("osUser")
        port = node_info.get("port")
        agent_uuid = node_info.get("parentUuid")
        log.info("start to check node status")
        # 检查当前输入的用户是否存在 检查当前用户下的服务是否正常
        if not GoldenDBResourceInfo.check_os_user_exist(os_user):
            log.error("os user is not exist")
            raise ErrCodeException(ErrorCode.LOGIN_FAILED,
                                   "Check connectivity: os user is not exist!")

        # 节点类型校验
        if not GoldenDBResourceInfo.check_node_type(os_user, node_type):
            log.error("Check connectivity: node type is not matched!")
            raise ErrCodeException(ErrorCode.ERROR_NODE_TYPE, f"{os_user},{node_type},{agent_uuid}")

        # 根据节点类型检查当前节点必须存在的服务是否存在
        err_service_list = GoldenDBResourceInfo.check_service_status(os_user, node_type)
        if err_service_list:
            log.error("err_service_list is not empty")
            # 返回：dbagent,loadserver
            raise ErrCodeException(ErrorCode.ERR_DB_SERVICES, ','.join(err_service_list))

        # 校验每一个数据节点dataNode的MySQL认证信息
        if node_type != "dataNode":
            return
        if get_mysql_db_session(host_ip, port, user, pwd) is None:
            log.error("Check connectivity: auth info error!")
            raise ErrCodeException(ErrorCode.ERROR_AUTH, "Check connectivity: auth info error!")

        # 校验DN节点输入的用户是否正确
        if not GoldenDBResourceInfo.check_os_user_legal(os_user, port):
            log.error("Check connectivity: os user is error!")
            raise ErrCodeException(ErrorCode.LOGIN_FAILED, "Check connectivity: os user is error!")
        log.info("Check node status success!")

    def clear_auth(self):
        """
        清理敏感信息
        :return:
        """
        clear(get_env_variable(f"application_auth_authKey_{self.pid}"))
        clear(get_env_variable(f"application_auth_authPwd_{self.pid}"))
