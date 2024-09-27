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

from mysql import log
from common.const import ParamConstant
from common.util.exec_utils import exec_write_new_file
from mysql.src.common.constant import IPConstant
from mysql.src.common.error_code import MySQLErrorCode, MySQLCode
from mysql.src.common.execute_cmd import get_cmd_result, get_charset_from_instance
from mysql.src.common.parse_parafile import BaseConnectParam
from mysql.src.common.response_param import get_body_error_param
from mysql.src.protect_mysql_base import MysqlBase


def _get_ip_index(ips, ip):
    index = -1
    for ip_single in ips:
        index = index + 1
        if ip_single.__contains__(ip):
            return index
    return -1


class PxcClusterVerify:
    def __init__(self, pid, context):
        self.pid = pid
        self.context = context
        self.env = self.context.get("appEnv", {})
        self.db_instance = self.context.get("application", {})
        self.port = self.db_instance.get("auth", {}).get("extendInfo", {}).get("instancePort")
        self.charset = get_charset_from_instance(self.db_instance)
        self.result_path = os.path.join(ParamConstant.RESULT_PATH, f"result{self.pid}")
        self.host_ip = self.env.get("endpoint")
        self.mysql_ip = self.db_instance.get("extendInfo", {}).get("instanceIp", "")

    def check_is_pxc_cluster(self):
        log.info(f"Begin to check is pxc cluster, pid: {self.pid}")
        try:
            self._check_is_pxc_cluster()
        except Exception as e:
            log.error(f"An Exception occur in check is oxc cluster pid: {self.pid}", e)

    def _check_is_pxc_cluster(self):
        """校验节点是否为pxc集群，查询pxc所有节点，如果pm的节点都在pxc节点的集群中，则校验通过，否则校验失败"""
        cmd = "show status like 'wsrep_incoming_addresses'"
        host_ip = self.mysql_ip if self.mysql_ip else IPConstant.LOCALHOST
        connect_param = BaseConnectParam(host_ip, self.port, self.charset, cmd)
        pxc_info = get_cmd_result(connect_param, self.result_path, self.pid)
        pxc_ips = pxc_info[0][1]
        nodes = pxc_ips.split(",")
        cluster_node = self._get_cluster_node()
        if len(pxc_info) == 0:
            # 节点self.host_ip不是pxc集群
            log.error(f"The node :{self.host_ip} is not in pxc cluster. pid: {self.pid}")
            code = MySQLErrorCode.CHECK_CLUSTER_FAILED
            self._output_check_result(MySQLCode.FAILED.value, self.host_ip, code)
            return False
        else:
            # 判断pxc集群节点数量
            if len(nodes) != len(self._get_cluster_node()):
                log.error(f"Failed to check the number of cluster instances. pid: {self.pid}")
                code = MySQLErrorCode.CHECK_CLUSTER_NUM_FAILED
                self._output_check_result(MySQLCode.FAILED.value, self.host_ip, code)
                return False
        return self._check_pxc_ips(nodes, cluster_node)

    def _get_cluster_node(self):
        all_nodes = self.db_instance.get("extendInfo", {}).get("allNodes")
        nodes = all_nodes.split(",")
        return nodes

    def _output_check_result(self, code, node, error_code=0):
        message = f"Check pxc cluster is success!node_ip: {node}, pid: {self.pid}"
        if code != MySQLCode.SUCCESS.value:
            message = f"Check pxc cluster failed, because {node} is not in pxc, pid: {self.pid}"
        params = get_body_error_param(code, error_code, message)
        log.info(
            f"Success to execute the connectivity command, params: {params}, pid: {self.pid}")
        exec_write_new_file(self.result_path, params)

    def _check_pxc_ips(self, pxc_node_ips_from_mysql, all_nodes_ips_from_pm):
        """检查ip以及端口号"""
        # 拿到当前主机的所有ip列表
        host_ips = MysqlBase("", "", "", {}).get_local_ips()
        # mysql_pxc_index为当前主机ip在mysql集群ip列表中的索引，pm_nodes_index为当前主机ip在pm返回的集群ip列表中的位置
        mysql_pxc_index = -1
        pm_nodes_index = -1
        for ip in host_ips:
            pm_nodes_index = _get_ip_index(all_nodes_ips_from_pm, ip)
            if pm_nodes_index != -1:
                break
        for ip in host_ips:
            mysql_pxc_index = _get_ip_index(pxc_node_ips_from_mysql, ip)
            if mysql_pxc_index != -1:
                break
        # 检验ip失败
        if mysql_pxc_index == -1 or pm_nodes_index == -1:
            log.error(f"Check pxc ip failed cluster.pid: {self.pid}")
            code = MySQLErrorCode.CHECK_AUTHENTICATION_INFO_FAILED
            self._output_check_result(MySQLCode.FAILED.value, all_nodes_ips_from_pm[pm_nodes_index], code)
            return False
        # 检验端口号失败
        pxc_mysql_port = pxc_node_ips_from_mysql[mysql_pxc_index][pxc_node_ips_from_mysql[mysql_pxc_index].index(":"):]
        pxc_pm_port = all_nodes_ips_from_pm[pm_nodes_index][all_nodes_ips_from_pm[pm_nodes_index].index(":"):]
        if pxc_mysql_port != pxc_pm_port:
            log.error(f"Check pxc Port failed cluster.pid: {self.pid}")
            code = MySQLErrorCode.CHECK_AUTHENTICATION_INFO_FAILED
            self._output_check_result(MySQLCode.FAILED.value, all_nodes_ips_from_pm[pm_nodes_index], code)
            return False
        # 校验成功
        self._output_check_result(MySQLCode.SUCCESS.value, self.host_ip)
        return True
