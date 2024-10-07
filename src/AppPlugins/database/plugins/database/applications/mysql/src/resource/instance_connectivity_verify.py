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

from mysql import log
from common.common import exter_attack
from common.const import ParamConstant
from common.util.check_utils import is_valid_id
from common.util.exec_utils import exec_overwrite_file
from mysql.src.common.constant import MySQLClusterType, IPConstant
from mysql.src.common.error_code import MySQLErrorCode, MySQLCode
from mysql.src.common.execute_cmd import get_cmd_result, check_privilege, get_charset_from_instance, \
    get_config_value_from_instance, validate_my_cnf
from mysql.src.common.parse_parafile import ParseParaFile
from mysql.src.common.parse_parafile import BaseConnectParam
from mysql.src.common.response_param import get_body_error_param
from mysql.src.protect_mysql_base import MysqlBase
from mysql.src.utils.mysql_utils import MysqlUtils


def check_mysql_is_ok(self, my_cnf_path):
    # 检查bin-log,
    binlog_error_code = check_binlog_status(self)
    # 检查MySQL或MariaDB权限
    check_privilege(self.instance_ip if self.instance_ip else IPConstant.LOCALHOST, self.port, self.result_path,
                    self.pid, self.charset)
    if binlog_error_code != 0:
        return binlog_error_code
    # 节点检验
    if self.mysql_cluster_type:
        if self.mysql_cluster_type == MySQLClusterType.AP:
            # 检查relay-log  当有relay-log，需要打开relay_log_recovery=on 否则恢复至本机会失败
            relay_log_error_code = check_relay_log_status(my_cnf_path)
            if relay_log_error_code != 0:
                return relay_log_error_code
        # 集群节点实例校验
        return check_cluster_type(self)
    # 单实例节点校验
    return check_single_node(self)


def check_relay_log_status(my_cnf_path):
    relay_log = MysqlUtils.get_relay_log_by_config_file(my_cnf_path)
    if relay_log:
        relay_log_recovery = MysqlUtils.get_relay_log_recovery_by_config_file(my_cnf_path)
        if not relay_log_recovery or relay_log_recovery != 'on':
            log.error(f"Check relay log status field!")
            return MySQLErrorCode.RELAY_LOG_DIRECTORY_OFF_ERROR
    return MySQLCode.SUCCESS.value


def get_pxc_info(self):
    """根据当前节点执行命令获取PXC节点信息"""
    get_cluster_status = "show status like 'wsrep_incoming_addresses'"
    host_ip = self.instance_ip if self.instance_ip else IPConstant.LOCALHOST
    connect_param = BaseConnectParam(host_ip, self.port, self.charset, get_cluster_status)
    cluster_status_res = get_cmd_result(connect_param, self.result_path, self.pid)
    return cluster_status_res


def check_single_node(self):
    """检测单节点的时候，排除集群节点"""
    if get_pxc_info(self):
        return MySQLErrorCode.CHECK_MYSQL_REGISTER_TYPE_ERROR
    return MySQLCode.SUCCESS.value


def check_cluster_type(self):
    """根据集群类型检查节点，执行命令show status like '%wsrep_incoming_addresses%'根据返回结果判断PXC集群和主备集群"""
    if self.mysql_cluster_type == MySQLClusterType.EAPP:
        return check_eapp()
    if self.mysql_cluster_type not in (MySQLClusterType.AP, MySQLClusterType.PXC):
        return MySQLErrorCode.CHECK_CLUSTER_TYPE_FAILED
    cluster_status_res = get_pxc_info(self)
    # 初始值为AP，
    mysql_cluster_type = MySQLClusterType.AP
    # 如果为AP集群则cluster_status_res为空，如果cluster_status_res不为空，则执行以下判断逻辑，验证到底是否为PXC集群
    if cluster_status_res:
        # 拿到当前主机ip判断其是否在查出的PXC集群ip集合中 例如：pxc_ips "xxx.xxx.100.120:330x,xxx.xxx.100.121:330x"
        pxc_ips = cluster_status_res[0][1]
        for node_ip in MysqlBase("", "", "", {}).get_local_ips():
            if pxc_ips.__contains__(node_ip):
                # 如果本机ip能与pxc集群节点匹配，则说明该节点PXC节点
                mysql_cluster_type = MySQLClusterType.PXC
                break
    if mysql_cluster_type == self.mysql_cluster_type:
        return MySQLCode.SUCCESS.value
    log.error(f"Check MySQL cluster type PXC or AP field!")
    return MySQLErrorCode.CHECK_CLUSTER_TYPE_FAILED


def check_eapp():
    if not MysqlUtils.eapp_is_running():
        return MySQLErrorCode.CHECK_MYSQL_NOT_RUNNING
    return MySQLCode.SUCCESS.value


def check_binlog_status(self):
    """判断binlog是否开启"""
    get_binlog_info = "show variables like '%log_bin%'"
    host_ip = self.instance_ip if self.instance_ip else IPConstant.LOCALHOST
    connect_param = BaseConnectParam(host_ip, self.port, self.charset, get_binlog_info)
    binlog_res = get_cmd_result(connect_param, self.result_path, self.pid)
    error_code = MySQLCode.SUCCESS.value
    for res in binlog_res:
        if "log_bin" in res and res[1] != "ON":
            log.error("Mysql log-bin is off!")
            error_code = MySQLErrorCode.LOG_BIN_ID_OFF_ERROR
    return error_code


class InstanceConnectivityVerity:
    @exter_attack
    def __init__(self):
        self.pid = sys.argv[1]
        if not is_valid_id(self.pid):
            log.warn(f'req_id is invalid')
            sys.exit(1)
        self.context = ParseParaFile.parse_para_file(self.pid)
        self.env = self.context.get("appEnv", {})
        self.db_instance = self.context.get("application", {})
        self.port = self.db_instance.get("auth", {}).get("extendInfo", {}).get("instancePort")
        self.charset = get_charset_from_instance(self.db_instance)
        self.my_cnf_path = get_config_value_from_instance(self.db_instance, "myCnfPath", "")
        self.instance_ip = self.db_instance.get("extendInfo", {}).get("instanceIp", "")
        self.result_path = os.path.join(ParamConstant.RESULT_PATH, f"result{self.pid}")
        if self.context.get("application", {}).get("extendInfo", {}).get("clusterType"):
            self.mysql_cluster_type = self.context.get("application", {}).get("extendInfo", {}).get("clusterType")
        else:
            self.mysql_cluster_type = None

    @exter_attack
    def check_connectivity(self):
        log.info(f"Begin to check connectivity from {self.port}, pid: {self.pid}")
        try:
            self._check_connectivity()
        except Exception as e:
            log.error(f"An exception occurs when check connectivity pid: {self.pid}!", e)

    def _check_connectivity(self):
        """校验实例的鉴权信息是否正确,包括logbin是否开启，集群区分，单实例注册"""
        error_code = check_mysql_is_ok(self, self.my_cnf_path)
        if error_code == 0:
            code = MySQLCode.SUCCESS.value
        else:
            code = MySQLCode.FAILED.value
        connectivity_params = get_body_error_param(code, error_code, f"Check connectivity!")
        log.info(
            f"Success to execute the connectivity command, params: {connectivity_params}, pid: {self.pid}")
        if self.my_cnf_path and not validate_my_cnf(self.my_cnf_path):
            connectivity_params = {
                "code": MySQLCode.FAILED.value,
                "bodyErr": MySQLErrorCode.INPUT_MY_CNF_NOT_EXIST,
                "message": "my.cnf path not exists"
            }
        exec_overwrite_file(self.result_path, connectivity_params)


if __name__ == '__main__':
    log.info(
        f"Begin to execute the connectivity command")
    instance_connectivity_verify = InstanceConnectivityVerity()
    instance_connectivity_verify.check_connectivity()
