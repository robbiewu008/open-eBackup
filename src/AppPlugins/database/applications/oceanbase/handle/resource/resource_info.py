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

from common.cleaner import clear
from common.common import execute_cmd, execute_cmd_list, check_command_injection, ismount_with_timeout
from common.parse_parafile import ParamFileUtil
from common.util.check_utils import is_ip_address, is_port
from oceanbase.common.const import OceanBaseSupportVersion, ErrorCode, CmdRetCode, CMDResult, ClusterCheckType, \
    OceanBaseResourceKeyName
from oceanbase.common.oceanbase_client import OceanBaseClient
from oceanbase.common.oceanbase_common import exec_sql_cmd, get_env_variable, check_special_characters, check_mount
from oceanbase.common.oceanbase_exception import ErrCodeException
from oceanbase.logger import log


class OceanBaseResourceInfo:

    def __init__(self, pid):
        self.pid = pid
        try:
            self.param = ParamFileUtil.parse_param_file(self.pid)
        except Exception as err:
            raise Exception(f"Failed to parse job param file for {err}") from err
        if not self.param:
            raise Exception(f"Failed to parse job param file is none")
        log.info(f"OceanBaseResourceInfo param:{self.param}")
        if self.param.get('application', ''):
            self.app_info = self.param.get('application')
        else:
            self.app_info = self.param.get("applications", {})[0]
        self.sub_type = self.app_info.get("subType", "")
        self.app_extend_info = self.app_info.get('extendInfo', '')
        self.check_type = self.app_extend_info.get("checkType")
        self.cluster_info_str = self.app_extend_info.get("clusterInfo", "")
        self.cluster_info = json.loads(self.cluster_info_str)
        self.cluster_id = self.cluster_info.get("cluster_id", "")
        self.cluster_name = self.cluster_info.get("cluster_name", "")
        if not check_special_characters(self.cluster_name):
            log.error(f'the cluster_name is Illegal {self.cluster_name}')
            raise Exception(f'the cluster_name is Illegal {self.cluster_name}')
        self.observers = self.cluster_info.get('obServerAgents', '')
        self.observer_ip = self.observers[0].get('ip', '')
        log.info(f'{self.cluster_id} {self.observers} {self.observer_ip}')
        if not is_ip_address(self.observer_ip):
            log.error(f'the ip_address is Illegal {self.observer_ip}')
            raise Exception(f'the ip_address is Illegal {self.observer_ip}')
        self.observer_port = self.observers[0].get('port', '')
        if not is_port(self.observer_port):
            log.error(f'the port is Illegal {self.observer_port}')
            raise Exception(f'the port is Illegal {self.observer_port}')
        self.tenants = self.cluster_info.get('tenantNames', '')
        self.timeout = 10
        self.conditions = self.param.get("condition", {}).get("conditions", {})
        self.query_type = ""
        if self.conditions and not isinstance(self.conditions, int):
            self.query_type = self.conditions.get("queryType", "")
        log.info(f"conditions {self.conditions} query_type {self.query_type}")
        try:
            self.client = OceanBaseClient(self.pid, self.param)
        except Exception as exception_str:
            log.error(f"OceanBaseClient init failed.{exception_str}")

    def check_cluster_version(self):
        query_version = ""
        try:
            query_version = self.client.query_cluster_version()
        except Exception as exception_str:
            log.error(f"query_cluster_version failed.{exception_str}")
        for version in OceanBaseSupportVersion:
            if query_version == version:
                log.info(f"Get cluster version {query_version} success!")
                return True, query_version
        log.error("OceanBase version not supported")
        return False, query_version

    def check_connectivity(self):
        log.info("check_connectivity")
        client = OceanBaseClient(self.pid, self.param)
        # 校验OBClient节点连通性
        client_ip = self.app_extend_info.get("obAgentIp")
        if not is_ip_address(client_ip):
            log.error(f'the obclient ip_address is Illegal {client_ip}')
            raise Exception(f'the obclient ip_address is Illegal {client_ip}')
        cmd_check_obclient = "rpm -q obclient"
        ret_code, std_out, std_err = execute_cmd(cmd_check_obclient)
        log.info(f"OBClient check std_out: {std_out}")
        if ret_code == CMDResult.FAILED.value:
            log.error(f"Check OBClient {client_ip} service failed: {std_err},")
            raise ErrCodeException(ErrorCode.ERROR_OBCLIENT_SERVICES, "OBClient service error", client_ip)
        log.info("Check OBClient service succeed.")

        # 校验observer节点连通性
        for observer in self.observers:
            observer_ip = observer.get('ip')
            observer_port = observer.get('port')
            ret, out, err = client.exec_oceanbase_cmd(observer_ip, observer_port, "", timeout=self.timeout)
            if ret == ErrorCode.ERROR_AUTH:
                log.error("check_connectivity fail")
                raise ErrCodeException(ErrorCode.ERROR_AUTH, f"Observer {observer_ip} auth failed!", observer_ip)
            if ret == CmdRetCode.EXEC_ERROR.value or ret == ErrorCode.ERR_DB_SERVICES:
                log.error("check_connectivity fail")
                raise ErrCodeException(ErrorCode.ERR_DB_SERVICES, f"Observer {observer_ip} check connectivity failed!",
                                       observer_ip)
        log.info("check_connectivity success")
        return True

    def get_cluster_info(self):
        """
        查看集群的详细信息并返回结果
        """
        clusters = ""
        try:
            clusters = self.client.query_cluster_info(self.observer_ip, self.observer_port)
        except Exception as exception_str:
            log.error(f"query_cluster_info failed.{exception_str}")
        return clusters

    def check_cluster_status(self):
        log.info("start to check ob cluster status")
        client = OceanBaseClient(self.pid, self.param)
        clusters = client.query_cluster_info(self.observer_ip, self.observer_port)
        for cluster in clusters:
            if cluster.get("cluster_status", "") != 'VALID':
                raise ErrCodeException(ErrorCode.ERR_CLUSTER_STATUS, "cluster status error")
        return True

    def check_observer_status(self):
        server_ip = self.app_extend_info.get("obAgentIp")
        if not is_ip_address(server_ip):
            log.error(f'the server ip_address is Illegal {server_ip}')
            raise Exception(f'the server ip_address is Illegal {server_ip}')
        # 校验服务是否存在
        cmd_list = ["ps -ef", f"grep observer", "grep -v tail", "grep -v gdb", "grep -v grep", "wc -l"]
        return_code, out_info, err_info = execute_cmd_list(cmd_list)
        if int(out_info) <= 0:
            log.error(f"Check node service failed, observer not exists")
            raise ErrCodeException(ErrorCode.ERR_DB_SERVICES, f"service observer not exists", server_ip)

        # 根据节点业务IP和当前agent是否在同一台机器上
        cmd_list = ["ifconfig", f"grep {server_ip}", "wc -l"]
        return_code, out_info, err_info = execute_cmd_list(cmd_list)
        if int(out_info) <= 0:
            raise ErrCodeException(ErrorCode.ERROR_IP_NOT_MATCH_AGENT, f"observer ip not match agent", server_ip)

    def get_observer(self):
        observers = ""
        try:
            observers = self.client.query_observer(self.observer_ip, self.observer_port)
        except Exception as exception_str:
            log.error(f"query_observer failed.{exception_str}")
        return observers

    def check_observer(self):
        observers = self.get_observer()
        for observer in observers:
            if observer.get('status', '') != "active" or observer.get('stop_time', '') != '0':
                return False
        return True

    def check_topo(self):
        log.info("start to check observer topo")
        client = OceanBaseClient(self.pid, self.param)
        # 1.遍历每个observer查询cluster_id是否一致
        cluster_ids = []
        for observer in self.observers:
            observer_ip = observer.get('ip')
            observer_port = observer.get('port')
            clusters = client.query_cluster_info(observer_ip, observer_port)
            cluster_ids.append(clusters[0].get('cluster_id'))
        cluster_ids_set = set(cluster_ids)
        if len(cluster_ids_set) != 1:
            log.error("observers not belong to the same cluster")
            raise ErrCodeException(ErrorCode.ERROR_OBSERVERS_NOT_MATCH, "observers not belong to the same cluster")

        # 2.选择某个server查询集群的observer总数, 和入参做对比
        observers = client.query_observer(self.observer_ip, self.observer_port)
        ip = []
        for server in observers:
            ip.append(server.get("svr_ip", ""))
        param_ip, param_port = self.get_server_ip_from_param()
        observer_ip_from_param = set(param_ip)
        observer_ip_from_query = set(ip)
        if len(observer_ip_from_param) != len(observer_ip_from_query):
            log.error("observers count error")
            raise ErrCodeException(ErrorCode.ERROR_DIFFERENT_TOPO, "observers count error")
        log.info("check observer topo succeed")
        return True

    def check_tenant(self):
        tenants_from_cluster = self.get_tenant()
        for tenant in self.tenants:
            if tenant not in tenants_from_cluster:
                log.error(f"the tenant {tenant} not in cluster")
                raise ErrCodeException(ErrorCode.ERROR_CLUSTER_ABNORMAL, f"the tenant {tenant} not in cluster")
        return True

    def get_tenant(self):
        tenants = ""
        try:
            tenants = self.client.query_tenant(self.observer_ip, self.observer_port)
        except Exception as exception_str:
            log.error(f"query_tenant failed.{exception_str}")
        return tenants

    def get_server_ip_from_param(self):
        ip = []
        port = []
        for server in self.observers:
            ip.append(server.get("ip", ""))
            port.append(server.get("port", ""))
        return ip, port

    def check(self):
        self.check_ip_port()
        if self.check_type == ClusterCheckType.CHECK_OBCLINET:
            self.check_connectivity()
            self.check_topo()
            self.check_cluster_status()
        if self.check_type == ClusterCheckType.CHECK_OBSERVER:
            self.check_observer_status()

    def get_resource_pool(self):
        resource_pool = ""
        try:
            resource_pool = self.client.query_resource_pool(self.observer_ip, self.observer_port)
        except Exception as exception_str:
            log.error(f"query_resource_pool failed.{exception_str}")
        return resource_pool

    def remove_project(self):
        log.info("start to remove project")
        need_stop_backup = self.app_extend_info.get("needStopBackup")
        if need_stop_backup == "1":
            # 停止备份
            log.info("stop db log archive")
            user = get_env_variable(f'{OceanBaseResourceKeyName.APPLICATION_AUTH_AUTHKEY}{self.pid}')
            db_pwd = get_env_variable(f'{OceanBaseResourceKeyName.APPLICATION_AUTH_AUTHPWD}{self.pid}')
            sql_str = "ALTER SYSTEM CANCEL all BACKUP force;"
            try:
                exec_sql_cmd(self.observer_ip, int(self.observer_port), user, db_pwd, sql_str)
            finally:
                clear(db_pwd)

        # 解挂载
        target_mount_path = os.path.join("/", self.cluster_name, self.cluster_id)
        if not check_mount(target_mount_path) or check_command_injection(target_mount_path):
            log.warn(f"target_mount_path can not umount")
            return False
        ret, _, err = execute_cmd(f"umount -l {target_mount_path}")
        if ret != CMDResult.SUCCESS:
            log.error(f"Fail to umount {err}")
            return False
        try:
            shutil.rmtree(os.path.join("/", self.cluster_name))
        except Exception as err:
            log.error(f"Fail to remove mount path for resource, {err}")
            return False
        log.info("Remove protect success.")
        return True

    def check_ip_port(self):
        illegal_ip_list = []
        illegal_port_list = []
        for server in self.observers:
            server_ip = server.get('ip', '')
            server_port = server.get('port', '')
            if not is_ip_address(server_ip):
                illegal_ip_list.append(server_ip)
            if not is_port(server_port):
                illegal_port_list.append(server_port)

        illegal_ip_set = set(illegal_ip_list)
        illegal_port_set = set(illegal_port_list)
        if len(illegal_ip_set) > 0 or len(illegal_port_set) > 0:
            log.error(f'the ip_address {illegal_ip_set} or port {illegal_port_set} Illegal')
            raise Exception(f'the ip_address {illegal_ip_set} or port {illegal_port_set} Illegal')
