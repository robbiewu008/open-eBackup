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
import shutil
import time

from common.cleaner import clear
from common.common import output_result_file, execute_cmd, check_command_injection
from common.const import SysData, CMDResult, PathConstant
from tdsql.common.const import BackupPath, MountType
from tdsql.common.tdsql_common import check_ip, check_port, execute_cmd_list
from tdsql.common.util import check_priv_is_ok, get_version_path, get_tdsql_config
from tdsql.handle.common.const import ClusterNodeCheckType, ErrorCode, TDSQLResourceInterface, TDSQLNodeService, \
    TDSQLResourceType, TDSQLSubType, TDSQLResourceKeyName, TDSQLDataNodeStatus, TDSQLGroupSetStatus
from tdsql.handle.common.exec_sql_cmd import get_mysql_db_session
from tdsql.handle.common.tdsql_exception import ErrCodeException
from tdsql.handle.common.tdsql_restful import request_post
from tdsql.handle.resource.parse_params import ResourceParam
from tdsql.logger import log

SINGLE_GRP_FILTER: bool = True


def get_env_variable(str_env_variable: str):
    env_variable = ''
    input_str = json.loads(SysData.SYS_STDIN)

    if input_str.get(str_env_variable):
        env_variable = input_str.get(str_env_variable)
    return env_variable


def build_instance_info(instance_list):
    groups = []
    for instance in instance_list:
        group = dict()
        set_id = instance.get("id")
        data_nodes = []
        db_version = instance.get("db_version")
        for db in instance.get("db"):
            port = str(db.get("port"))
            data_node = dict()
            data_node["setId"] = set_id
            data_node["ip"] = db.get("ip")
            data_node["port"] = port
            data_node["isMaster"] = db.get("master")
            conf_file = "my_" + port + ".cnf"
            log.info(f"port:{port}")
            version_path = get_version_path(db_version)
            defaults_file = os.path.join(BackupPath.BACKUP_PRE, port, f"{version_path}/etc", conf_file)
            data_node["defaultsFile"] = defaults_file
            # 改成从my.conf文件读取
            exec_cmd_list = [f"cat {defaults_file}", "grep mysql.sock", "head -n 1", "awk -F '=' '{print $2}'"]
            return_code, out_info, err_info = execute_cmd_list(exec_cmd_list)
            data_node["socket"] = out_info.strip()
            data_node["linkStatus"] = TDSQLDataNodeStatus.ONLINE if db.get(
                "alive") == 0 else TDSQLDataNodeStatus.OFFLINE
            data_nodes.append(data_node)
        group["setId"] = set_id
        group["dataNodes"] = data_nodes
        groups.append(group)

    return groups


def build_group_info(instance_list):
    group = dict()
    set_ids = []
    data_nodes = []
    ip_list = []
    for instance in instance_list:
        set_ids.append(instance.get("id"))
        for db in instance.get("db"):
            data_node_ip = db.get("ip")
            if data_node_ip not in ip_list:
                data_node = dict()
                ip_list.append(data_node_ip)
                data_node["ip"] = data_node_ip
                data_nodes.append(data_node)
    group["setIds"] = set_ids
    group["dataNodes"] = data_nodes
    return group


class TDSQLResourceInfo:

    def __init__(self, pid, resource_param: ResourceParam):
        self.pid = pid
        self.param = resource_param
        self.app_extend_info = self.param.get_extend_info()
        self._mount_type = self.app_extend_info.get('agentMountType')

    @staticmethod
    def get_cluster_version(os_user):
        """
        获取tdsql版本
        """
        return ''

    @staticmethod
    def check_data_node_is_match_agent(data_node):
        """
        检查数据节点的ip是否属于所关联的代理主机
        @:param: data_node
        :return: True 正常 False不正常
        """
        # 根据节点业务IP和当前agent是否在同一台机器上
        log.info(f"check_data_node_is_match_agent, data_node is {data_node}")
        host_ip = data_node.get("ip")
        if not check_ip(host_ip):
            log.error(f"check_data_node_is_match_agent host_ip is not expected, host_ip is {data_node}")
            raise ErrCodeException(ErrorCode.ERROR_PARAM, f"ip {host_ip} is not expected")

        cmd_list = ["ifconfig", f"grep {host_ip}", "wc -l"]
        return_code, out_info, err_info = execute_cmd_list(cmd_list)
        if int(out_info) <= 0:
            log.error(f"host_ip not match agent, host_ip is {host_ip}")
            raise ErrCodeException(ErrorCode.ERROR_IP_NOT_MATCH_AGENT, f"host_ip {host_ip} not match agent", host_ip)

    def check_node_info(self):
        """
        检测集群节点信息
        """
        resource_param = self.param
        extend_info = resource_param.get_extend_info()
        check_type = extend_info.get("checkType")
        log.info(f"check_node_info, check_type is {check_type}")
        if check_type == ClusterNodeCheckType.CHECK_NODE:
            single_node = json.loads(extend_info.get("singleNode"))
            self.check_node_status(single_node)

        if check_type == ClusterNodeCheckType.CHECK_MANAGE_NODE_NUM:
            if not self.check_manage_nodes_num(extend_info):
                raise ErrCodeException(ErrorCode.ERROR_DIFFERENT_TOPO, f"input manage node num not match")

        if check_type == ClusterNodeCheckType.CHECK_GROUP_INFO:
            self.check_group_info(extend_info)

        if check_type == ClusterNodeCheckType.CHECK_GROUP_DATA_NODE:
            data_node = json.loads(extend_info.get("dataNode"))
            self.check_data_node_is_match_agent(data_node)

    def check_group_info(self, extend_info):
        """
        检查分布式实例信息
        @:param: extend_info
        :return: True 正常 False不正常
        """
        user = get_env_variable(TDSQLResourceKeyName.APPLICATION_AUTH_AUTHKEY + self.pid)
        pwd = get_env_variable(TDSQLResourceKeyName.APPLICATION_AUTH_AUTHPWD + self.pid)
        # 调用oss接口查询实例信息
        request_url = extend_info.get("requestUrl")
        timestamp = int(time.time())
        request_body = {
            "callee": "TDSQL", "caller": user, "eventId": 101,
            "interface": {
                "interfaceName": "TDSQL.GetGroup",
                "para": {
                    "groups": [{"id": extend_info.get("id")}]
                }
            },
            "password": pwd, "timestamp": timestamp, "version": "1.0"
        }
        request_header = {'Content-type': 'application/json'}
        ret, ret_body, ret_header = request_post(request_url, request_body, request_header)
        if not ret:
            log.error(f'Failed query tdsql group nodes info, request_body is : {request_body}')
            raise ErrCodeException(ErrorCode.ERROR_PARAM, f"Failed query tdsql group nodes info")
        if ret_body.get("returnMsg") != "ok":
            log.error(f'query tdsql module info error with return: {ret_body.get("returnMsg")}')
            raise ErrCodeException(ErrorCode.ERROR_PARAM, f"query tdsql module info error")

        input_group_info = json.loads(extend_info.get("group"))
        log.info(f"check_group_info, input_group_info is {input_group_info}")
        instance_list = ret_body.get("returnData").get("groups")[0].get("instances")
        oss_set_ids = []
        oss_data_nodes = []
        for instance in instance_list:
            if instance.get("status") != TDSQLGroupSetStatus.ONLINE:
                log.error(f'set_status is not online, instance is : {instance}')
                raise ErrCodeException(ErrorCode.ERROR_SET_IS_NOT_ONLINE, f"set_status is not online")
            oss_set_ids.append(instance.get("id"))
            for db in instance.get("db"):
                data_node_ip = db.get("ip")
                if data_node_ip not in oss_data_nodes:
                    oss_data_nodes.append(data_node_ip)
        input_data_nodes = []
        for data_node in input_group_info.get("dataNodes"):
            node_ip = data_node.get("ip")
            if node_ip not in input_data_nodes:
                input_data_nodes.append(node_ip)
        if set(oss_set_ids) != set(input_group_info["setIds"]):
            log.error(f'group_info is not match input setIds is : {input_group_info["setIds"]}')
            raise ErrCodeException(ErrorCode.ERROR_SETS_NOT_MATCH, f"input setIds is not match")
        if set(oss_data_nodes) != set(input_data_nodes):
            log.error(f'group_info is not match input dataNodes is : {input_data_nodes}')
            raise ErrCodeException(ErrorCode.ERROR_DATANODES_NOT_MATCH, f"input dataNodes is not match")

    def check_node_status(self, single_node):
        """
        检查节点状态
        @:param: single_node注册的节点信息, auth_info用户信息
        :return: True 正常 False不正常
        """
        # 校验服务是否存在
        node_type = single_node.get("nodeType", "")
        log.info(f"node_type is {node_type}")
        port = single_node.get("port", "")

        server_type = TDSQLNodeService.SERVICE_DICT.get(node_type)
        if not server_type:
            raise ErrCodeException(ErrorCode.ERROR_PARAM, f"service_type {server_type} is not expected")

        cmd_list = ["ps -ef", f"grep {server_type}", "grep -v tail", "grep -v gdb", "grep -v grep", "wc -l"]
        if node_type == "dataNode":
            check_port(port)
            cmd_list = [
                "ps -ef", f"grep {server_type}", f"grep {port}", "grep -v tail", "grep -v gdb", "grep -v grep",
                "wc -l"
            ]
        return_code, out_info, err_info = execute_cmd_list(cmd_list)
        if int(out_info) <= 0:
            log.error(f"Check node service failed, {server_type} not exists")
            raise ErrCodeException(ErrorCode.ERROR_SERVICE, f"service {server_type} not exists", server_type)

        # 根据节点业务IP和当前agent是否在同一台机器上
        host_ip = single_node.get("ip")
        if not check_ip(host_ip):
            raise ErrCodeException(ErrorCode.ERROR_PARAM, f"ip {host_ip} is not expected")

        cmd_list = ["ifconfig", f"grep {host_ip}", "wc -l"]
        return_code, out_info, err_info = execute_cmd_list(cmd_list)
        if int(out_info) <= 0:
            raise ErrCodeException(ErrorCode.ERROR_IP_NOT_MATCH_AGENT, f"host_ip {host_ip} not match agent", host_ip)

        # 校验oss节点相关信息
        if node_type == "ossNode":
            self.check_oss_connection(single_node)

        # 校验数据节点信息
        if node_type == "dataNode":
            self.check_data_node_status(single_node)

    def check_oss_connection(self, single_node):
        # 校验oss节点端口信息是否正确
        port = single_node.get("port")
        if port != "8080":
            raise ErrCodeException(ErrorCode.ERROR_OSS_PORT, f"ossNode port {port} incorrect")

        host_ip = single_node.get("ip")
        if not check_ip(host_ip):
            log.error(f"check_oss_connection host_ip is not expected, host_ip is {host_ip}")
            raise ErrCodeException(ErrorCode.ERROR_PARAM, f"ip {host_ip} is not expected")
        request_url = f'http://{host_ip}:{port}/tdsql'
        timestamp = int(time.time())
        user = get_env_variable(TDSQLResourceKeyName.APPLICATION_AUTH_AUTHKEY + self.pid)
        pwd = get_env_variable(TDSQLResourceKeyName.APPLICATION_AUTH_AUTHPWD + self.pid)
        request_body = {
            "callee": "TDSQL", "caller": user, "eventId": 101,
            "interface": {
                "interfaceName": "TDSQL.TdsqlModuleVersion", "para": {}
            },
            "password": pwd, "timestamp": timestamp, "version": "1.0"
        }
        request_header = {'Content-type': 'application/json'}
        try:
            ret, ret_body, ret_header = request_post(request_url, request_body, request_header)
        except Exception as ex:
            log.error(f"check oss ip error, ex is {ex}")
            raise ErrCodeException(ErrorCode.ERROR_OSS_IP_ADDRESS, f"ossNode ip {host_ip} incorrect", host_ip) from ex

        if not ret:
            raise ErrCodeException(ErrorCode.ERROR_OSS_IP_ADDRESS, f"ossNode ip {host_ip} incorrect", host_ip)

    def check_data_node_status(self, single_node):
        """
        检查数据节点状态
        @:param: single_node注册的节点信息, auth_info用户信息
        :return: True 正常 False不正常
        """
        # 校验文件路径
        defaults_file_path = single_node.get("defaultsFile")
        if not os.path.exists(defaults_file_path):
            raise ErrCodeException(ErrorCode.ERROR_CONF_PATH,
                                   f"Check node params: defaults file {defaults_file_path} not exist!")

        socket_path = single_node.get("socket")
        if not os.path.exists(socket_path):
            raise ErrCodeException(ErrorCode.ERROR_SOCKET_PATH,
                                   f"Check node params: socket file {socket_path} not exist!")

        # 校验每一个数据节点dataNode的MySQL认证信息
        host_ip = single_node.get("ip")
        port = single_node.get("port")
        user = get_env_variable(TDSQLResourceKeyName.APPLICATION_AUTH_AUTHKEY + self.pid)
        pwd = get_env_variable(TDSQLResourceKeyName.APPLICATION_AUTH_AUTHPWD + self.pid)
        if get_mysql_db_session(host_ip, port, user, pwd) is None:
            raise ErrCodeException(ErrorCode.ERROR_AUTH, "Check connectivity: auth info error!")

        # 校验数据库用户是否有权限
        version = defaults_file_path.split("/")[4]
        if not check_priv_is_ok(user, pwd, socket_path, version):
            raise ErrCodeException(ErrorCode.CHECK_PRIVILEGE_FAILED,
                                   "Check connectivity: user have no privilege for any ip")

    def check_manage_nodes_num(self, extend_info):
        """
        检查注册的oss节点数和scheduler节点数是否和集群上的一致
        @:param: node_info注册的节点信息, auth_info用户信息
        :return: True 正常 False不正常
        """
        # 获取集群的组件节点信息
        user = get_env_variable(TDSQLResourceKeyName.APPLICATION_AUTH_AUTHKEY + self.pid)
        pwd = get_env_variable(TDSQLResourceKeyName.APPLICATION_AUTH_AUTHPWD + self.pid)
        request_url = extend_info.get("requestUrl")
        timestamp = int(time.time())
        request_body = {
            "callee": "TDSQL", "caller": user, "eventId": 101,
            "interface": {
                "interfaceName": "TDSQL.TdsqlAllModuleInformation", "para": {}
            },
            "password": pwd, "timestamp": timestamp, "version": "1.0"
        }
        request_header = {'Content-type': 'application/json'}
        ret, ret_body, ret_header = request_post(request_url, request_body, request_header)

        if not ret:
            log.error("Failed query modules info!")
            return False
        if ret_body.get("returnMsg"):
            log.error(f'query tdsql module info error with return: {ret_body.get("returnMsg")}')
            return False

        # 获取集群的oss列表和scheduler列表
        oss_list = []
        scheduler_list = []
        for module in ret_body.get("returnData").get("modules"):
            if module.get("name") == "oss":
                for oss_node in module.get("info"):
                    oss_list.append(oss_node.get("ip"))
            elif module.get("name") == "scheduler":
                for scheduler_node in module.get("info"):
                    scheduler_list.append(scheduler_node.get("ip"))

        manage_nodes = json.loads(extend_info.get("manageNodes"))
        input_oss = set(manage_nodes.get("oss"))
        input_scheduler = set(manage_nodes.get("scheduler"))
        # 检查oss和scheduler节点数是否一致
        if len(input_oss) != len(oss_list) or len(input_scheduler) != len(scheduler_list):
            log.error(f'oss node sizes or scheduler node size not match')
            return False
        # 检查注册的oss和scheduler是否是所属集群
        for oss in input_oss:
            if oss not in oss_list:
                log.error(f"the input oss node {oss} not belong to tdsql cluster")
                return False
        for scheduler in input_scheduler:
            if scheduler not in scheduler_list:
                log.error(f"the input scheduler node {scheduler} not belong to tdsql cluster")
                return False
        return True

    def get_data_nodes(self):
        """
        查询实例节点信息
        """
        log.info(f"start get_data_nodes")
        resource_param = self.param
        extend_info = resource_param.get_list_application_extend_info()
        user = get_env_variable(TDSQLResourceKeyName.LIST_APPLICATION_AUTH_AUTHKEY + self.pid)
        pwd = get_env_variable(TDSQLResourceKeyName.LIST_APPLICATION_AUTH_AUTHPWD + self.pid)
        # 发送post请求
        request_url = extend_info.get("requestUrl")
        timestamp = int(time.time())
        instance_type = extend_info.get("instanceType")
        interface_name = TDSQLResourceInterface.INTERFACE_DICT.get(instance_type).get("interface")
        para_name = TDSQLResourceInterface.INTERFACE_DICT.get(instance_type).get("paraName")
        request_body = {
            "callee": "TDSQL", "caller": user, "eventId": 101,
            "interface": {
                "interfaceName": interface_name,
                "para": {
                    para_name: [{"id": extend_info.get("id")}]
                }
            },
            "password": pwd, "timestamp": timestamp, "version": "1.0"
        }
        request_header = {'Content-type': 'application/json'}
        ret, ret_body, ret_header = request_post(request_url, request_body, request_header)
        if not ret:
            log.error("Failed query instance nodes info!")
            return
        if ret_body.get("returnMsg") != "ok":
            log.error(f'query tdsql module info error with return: {ret_body.get("returnMsg")}')
            return
        self.out_resource_list(extend_info, ret_body.get("returnData"))

    def out_resource_list(self, extend_info, return_data):
        resource_list = []
        resource = dict()
        cluster_instance_info = dict()
        cluster_instance_info["id"] = extend_info.get("id")
        instance_type = extend_info.get("instanceType")
        cluster_instance_info["type"] = instance_type
        if instance_type == TDSQLResourceType.INSTANCE:
            instance_list = return_data.get("instance")
            groups = build_instance_info(instance_list)
            cluster_instance_info["groups"] = groups
            resource["subType"] = TDSQLSubType.SUBTYPE_CLUSTER_INSTANCE
            db_version = return_data.get("instance")[0].get("db_version")
            version_path = get_version_path(db_version)
            resource["extendInfo"] = {
                "clusterInstanceInfo": json.dumps(cluster_instance_info),
                "mysql_version": version_path,
            }
        else:
            instance_list = return_data.get("groups")[0].get("instances")
            group = build_group_info(instance_list)
            cluster_instance_info["group"] = group
            resource["subType"] = TDSQLSubType.SUBTYPE_CLUSTER_GROUP
            mysql_version = return_data.get("groups")[0].get("db_version")
            tdsql_config = get_tdsql_config()
            version_path = tdsql_config.get('versionPath').get(mysql_version, '')
            resource["extendInfo"] = {
                "clusterGroupInfo": json.dumps(cluster_instance_info),
                "mysql_version": version_path
            }
        resource["type"] = TDSQLSubType.TYPE
        log.info(f"get_data_nodes resource: {resource}")
        resource_list.append(resource)
        params = {"resourceList": resource_list}
        output_result_file(self.pid, params)

    def get_machine_info(self):
        """
        查询实例机型信息
        """
        log.info(f"start get_machine_info")
        resource_param = self.param
        extend_info = resource_param.get_list_application_extend_info()
        log.info(f"get_machine_info extend_info")
        user = get_env_variable(TDSQLResourceKeyName.LIST_APPLICATION_AUTH_AUTHKEY + self.pid)
        pwd = get_env_variable(TDSQLResourceKeyName.LIST_APPLICATION_AUTH_AUTHPWD + self.pid)
        # 发送post请求
        request_url = extend_info.get("requestUrl")
        timestamp = int(time.time())
        request_body = {
            "callee": "TDSQL", "caller": user, "eventId": 101,
            "interface": {
                "interfaceName": "TDSQL.QuerySpec",
                "para": {
                }
            },
            "password": pwd, "timestamp": timestamp, "version": "1.0"
        }
        request_header = {'Content-type': 'application/json'}
        ret, ret_body, ret_header = request_post(request_url, request_body, request_header)
        if not ret:
            log.error("Failed query instance nodes info!")
            return ""
        if ret_body.get("returnMsg") != "ok":
            log.error(f'query tdsql module info error with return: {ret_body.get("returnMsg")}')
            return ""
        spec_list = ret_body.get("returnData").get("spec")
        machines = []
        for spec in spec_list:
            if spec.get("machine") == "PROXY":
                continue
            machine = dict()
            machine["machine"] = spec.get("machine")
            machine["memory"] = spec.get("mem")
            machine["cpu"] = spec.get("cpu") / 100
            machine["dataDisk"] = spec.get("data_disk")
            machine["logDisk"] = spec.get("log_disk")
            machines.append(machine)
        return json.dumps(machines)

    def get_cluster_host_info(self):
        """
        查询集群所有数据节点信息信息
        """
        log.info(f"start get_cluster_host_info")
        resource_param = self.param
        extend_info = resource_param.get_list_application_extend_info()
        log.info(f"get_machine_info extend_info")
        user = get_env_variable(TDSQLResourceKeyName.LIST_APPLICATION_AUTH_AUTHKEY + self.pid)
        pwd = get_env_variable(TDSQLResourceKeyName.LIST_APPLICATION_AUTH_AUTHPWD + self.pid)
        # 发送post请求
        request_url = extend_info.get("requestUrl")
        timestamp = int(time.time())
        request_body = {
            "callee": "TDSQL", "caller": user, "eventId": 101,
            "interface": {
                "interfaceName": "TDSQL.GetResource",
                "para": {
                }
            },
            "password": pwd, "timestamp": timestamp, "version": "1.0"
        }
        request_header = {'Content-type': 'application/json'}
        ret, ret_body, ret_header = request_post(request_url, request_body, request_header)
        if not ret:
            log.error("Failed query cluster host info!")
            return
        if ret_body.get("returnMsg") != "ok":
            log.error(f'query tdsql query cluster host error with return: {ret_body.get("returnMsg")}')
            return
        resource_list = []
        resource = dict()
        host_list = ret_body.get("returnData").get("hosts")
        hosts = []
        for host in host_list:
            if host.get("type") == "set":
                ip = host.get("ip")
                hosts.append(ip)
        resource["extendInfo"] = {"hosts": json.dumps(hosts)}
        machine_info = self.get_machine_info()
        log.info(f"get_machine_info is {machine_info}")
        if machine_info:
            resource["extendInfo"]["machineSpec"] = machine_info
        resource_list.append(resource)
        params = {"resourceList": resource_list}
        output_result_file(self.pid, params)

    def query_cluster(self):
        """
        查询集群信息
        """
        resource = dict()
        resource["id"] = ""
        resource["type"] = TDSQLSubType.TYPE
        resource["subType"] = TDSQLSubType.SUBTYPE_CLUSTER
        resource["name"] = ""
        resource["endpoint"] = ""
        resource["nodes"] = []
        resource["extendInfo"] = {"version": "10.3.20.4.0"}
        output_result_file(self.pid, resource)

    def clear_auth(self):
        """
        清理敏感信息
        :return:
        """
        clear(get_env_variable(TDSQLResourceKeyName.APPLICATION_AUTH_AUTHKEY + self.pid))
        clear(get_env_variable(TDSQLResourceKeyName.APPLICATION_AUTH_AUTHPWD + self.pid))
        clear(get_env_variable(TDSQLResourceKeyName.LIST_APPLICATION_AUTH_AUTHKEY + self.pid))
        clear(get_env_variable(TDSQLResourceKeyName.LIST_APPLICATION_AUTH_AUTHPWD + self.pid))

    def remove_project(self):
        log.info("start to remove project")
        resource_param = self.param
        extend_info = resource_param.get_extend_info()
        if not extend_info:
            log.warn("extend info not exist")
            return False
        cluster_group_str = extend_info.get("clusterGroupInfo")
        cluster_group = json.loads(cluster_group_str)
        group_id = cluster_group.get("group").get("groupId")
        log.info(f"umount group {group_id} path")
        # 解挂载
        mount_path = os.path.join("/tdsqlbackup/tdsqlzk/", group_id)
        if not os.path.exists(mount_path) or not os.path.isdir(mount_path) or check_command_injection(mount_path):
            log.warn(f"target_mount_path {mount_path} can not umount")
            return False
        if self._mount_type == MountType.FUSE:
            unmount_str = f"{PathConstant.FILE_CLIENT_PATH} --remove --mount_point={mount_path}"
        else:
            unmount_str = f"umount -l {mount_path}"
        ret, _, err = execute_cmd(unmount_str)
        if ret != CMDResult.SUCCESS:
            log.error(f"Fail to umount {err}")
            return False
        try:
            shutil.rmtree(mount_path)
        except Exception as err:
            log.error(f"Fail to remove mount path for resource, {err}")
            return False
        log.info("Remove protect success.")
        return True
