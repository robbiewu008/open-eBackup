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
import re
import sys

import pexpect
from bs4 import BeautifulSoup

from common import cleaner
from common.cleaner import clear
from common.common import exter_attack, execute_cmd, check_command_injection
from common.common_models import ActionResult, QueryClusterResponse, ClusterNodeInfo, ClusterNodeExtendInfo
from common.const import ParamConstant, ExecuteResultEnum, AuthType, SysData, CMDResult, RoleType
from common.logger import Logger
from common.util import check_utils
from common.util.check_utils import is_valid_id
from common.util.cmd_utils import cmd_format
from common.util.exec_utils import exec_overwrite_file
from postgresql.common.const import PexpectResult, InstallDeployType, CmdRetCode, PgConst
from postgresql.common.error_code import ErrorCode
from postgresql.common.models import ClupDb, ClupCluster, QueryClupClusterResponse, ClupClusterNodeInfo
from postgresql.common.pg_exec_sql import ExecPgSql
from postgresql.common.util.domain_2_ip_util import domain_2_ip
from postgresql.common.util.get_sensitive_utils import get_env_variable
from postgresql.common.util.get_version_util import get_version
from postgresql.common.util.pg_common_utils import PostgreCommonUtils
from postgresql.common.util.pg_param import JsonParam

LOGGER = Logger().get_logger("postgresql.log")


class ClusterNodesChecker:
    def __init__(self, check_type, request_pid):
        self.check_type = check_type
        self.pid = request_pid
        self.context = JsonParam.parse_param_with_jsonschema(self.pid)
        self.application = self.context.get("application", {})
        self.env = self.context.get("appEnv", {})
        self.extend_info = self.application.get("extendInfo", {})
        self.env_extend = self.env.get("extendInfo", {})
        self.os_username = self.extend_info.get("osUsername")
        self.pgpool_client = self.extend_info.get("pgpoolClientPath", "/usr/local/pgpool")
        if self.extend_info.get(PgConst.ACTION_TYPE) != PgConst.QUERY_CLUP_SERVER:
            self.rep_user = self.application.get("auth", {}).get("extendInfo", {}).get("dbStreamRepUser", "repl")
        else:
            self.rep_user = ""
        self.service_ip = self.extend_info.get("serviceIp")
        self.inst_port = self.extend_info.get("instancePort")
        self.result_file = os.path.join(ParamConstant.RESULT_PATH, f"result{self.pid}")
        self.auth_type = get_env_variable(f"application_auth_authType_{self.pid}")
        self.nodes = self.env_extend.get("allNodes")
        self.virtual_ip = self.env_extend.get("virtualIp")
        self.port = self.env_extend.get("instancePort")
        self.deploy_type = self.env_extend.get("installDeployType", InstallDeployType.PGPOOL)
        self.client_path = os.path.realpath(os.path.join(self.extend_info.get("clientPath"), "bin",
                                                         "psql")) if self.deploy_type != InstallDeployType.CLUP else ""
        self.version = None
        self.enable_root = PostgreCommonUtils.get_root_switch()

    @staticmethod
    def get_nodes(patroni_config):
        if check_command_injection(patroni_config):
            LOGGER.error(f"patroni config:{patroni_config} check error!")
            return []
        cmd = cmd_format("patronictl -c {} list", patroni_config)
        ret_code, std_out, std_err = execute_cmd(cmd)
        if ret_code != CMDResult.SUCCESS.value:
            LOGGER.info(f"get_nodes out: {std_out},error: {std_err}")
            return []
        nodes = []
        if len(std_out) > 0:
            rows = std_out.splitlines()
            rows_len = len(rows)
            for i in range(3, rows_len - 1):
                row = rows[i]
                columns = row.split()
                role = columns[5]
                hostname = columns[3]
                status = columns[7]
                node = {
                    'hostname': hostname,
                    'role': str(RoleType.PRIMARY.value) if role in ['Leader'] else str(RoleType.STANDBY.value),
                    'status': status
                }
                nodes.append(node)
        return nodes

    @staticmethod
    def query_clup_cluster():
        sql_cmd = "select cluster_id,cluster_data,state from clup_cluster;"
        return_code, std_out, st_err = execute_cmd(cmd_format("su - {} -c {}", 'csumdb', f'psql -c \"{sql_cmd}\"'))
        if return_code != CMDResult.SUCCESS:
            LOGGER.error(f"Query clupserver error. error out: {st_err}")
            raise Exception("Query CLup server failed")
        clusters = []
        if len(std_out) > 0:
            split_lines = std_out.splitlines()
            for index, row in enumerate(split_lines):
                if index < 2 or index > len(split_lines) - 3:
                    continue
                clup_cluster = ClupCluster()
                columns = row.split("|")
                clup_cluster.cluster_id = columns[0].strip()
                clup_cluster.cluster_data = json.loads(columns[1].strip())
                clup_cluster.cluster_state = columns[2].strip()
                clusters.append(clup_cluster)
        return clusters

    @exter_attack
    def check_cluster_nodes(self):
        LOGGER.info(f"Begin to check cluster by os param: {self.context}, pid: {self.pid}")
        param = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, bodyErr=ErrorCode.CHECK_CLUSTER_FAILED,
                             message="check cluster failed!")
        try:
            if self.deploy_type == InstallDeployType.CLUP and self.check_type == "QueryCluster":
                if self.extend_info.get(PgConst.ACTION_TYPE) == PgConst.QUERY_CLUP_SERVER:
                    param = self.query_clup_server_cluster()
                else:
                    param = self.clup_query_cluster()
            elif self.deploy_type == InstallDeployType.CLUP and self.check_type == "CheckApplication":
                if self.extend_info.get(PgConst.ACTION_TYPE) == PgConst.QUERY_CLUP_SERVER:
                    param = self.clup_query_cluster_status()
                else:
                    param = self.clup_check_application()
            else:
                param = self._check_cluster_nodes()
        except Exception as e:
            param = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, bodyErr=ErrorCode.CHECK_CLUSTER_FAILED,
                                 message="An exception occur when check cluster failed.")
            LOGGER.error(f"Failed to check cluster!pid: {self.pid}", e)
        finally:
            cleaner.clear(SysData.SYS_STDIN)
            LOGGER.info('Clearing data successfully')
            LOGGER.info(f"Finally to check cluster, pid: {self.pid}, param:{param}")
            exec_overwrite_file(self.result_file, param.dict(by_alias=True))

    def get_pg_pool_info(self, pg_pool_info):
        soup = BeautifulSoup(pg_pool_info, "html.parser")
        trs = soup.find_all(name="tr")
        nodes = []
        soup = BeautifulSoup(str(trs[0]), "html.parser")
        ths = soup.find_all(name='th', attrs={"align": "center"})
        headers = [header.get_text() for header in ths]
        LOGGER.info(f"Get headers : {headers}, pid: {self.pid}")
        for i in range(1, len(trs)):
            tr = trs[i]
            _soup = BeautifulSoup(str(tr), "html.parser")
            tds = _soup.find_all(name='td', attrs={"align": "left"})
            node = [td.get_text() for td in tds]
            LOGGER.info(f"Get node : {node}, pid: {self.pid}")
            nodes.append(dict(zip(headers, node)))
        return nodes

    def clup_query_cluster(self):
        clusters = self.query_clup_cluster()
        for cluster in clusters:
            cluster_data = cluster.cluster_data
            if self.virtual_ip == cluster_data.get('vip'):
                if self.check_username_pwd(cluster_data):
                    result = QueryClupClusterResponse(name=self.env.get('name'), subType='PostgreClusterInstance')
                    result.extend_info = {
                        'code': ExecuteResultEnum.INTERNAL_ERROR,
                        'bodyErr': ErrorCode.LOGIN_FAILED,
                        'message': f'Login denied'
                    }
                    return result
                cluster_id = cluster.cluster_id
                cluster_state = cluster.cluster_state
                break
        return self.query_clup_cluster_db(cluster_id, cluster_state)

    def clup_query_cluster_status(self):
        self.rep_user = self.application.get("auth", {}).get("extendInfo", {}).get("dbStreamRepUser", "repl")
        clusters = self.query_clup_cluster()
        for cluster in clusters:
            cluster_data = cluster.cluster_data
            if self.virtual_ip == cluster_data.get('vip'):
                if self.check_username_pwd(cluster_data) or cluster.cluster_state == PgConst.CLUP_SERVER_OFFLINE:
                    return ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR.value,
                                        bodyErr=PgConst.CLUP_SERVER_OFFLINE, message='query success.')
                return ActionResult(code=ExecuteResultEnum.SUCCESS.value, bodyErr=cluster.cluster_state,
                                    message='query success.')
        return ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR.value, bodyErr=ErrorCode.LOGIN_FAILED,
                            message="Login denied.")

    def check_username_pwd(self, cluster_data):
        db_user_name = get_env_variable(f"application_auth_authKey_{self.pid}")
        db_pwd = get_env_variable(f"application_auth_authPwd_{self.pid}")
        db_stream_rep_pwd = get_env_variable(f"application_auth_extendInfo_dbStreamRepPwd_{self.pid}")
        return (db_user_name != cluster_data.get('db_user') or self.rep_user != cluster_data.get('repl_user') or
                PostgreCommonUtils.to_db_text(db_pwd) != cluster_data.get('db_pass') or
                PostgreCommonUtils.to_db_text(db_stream_rep_pwd) != cluster_data.get('repl_pass'))

    def query_clup_cluster_db(self, cluster_id, cluster_state):
        sql_cmd = f"select pgdata,is_primary,host,port,db_detail from clup_db where cluster_id = '{cluster_id}';"
        return_code, std_out, st_err = execute_cmd(cmd_format("su - {} -c {}", 'csumdb', f'psql -c \"{sql_cmd}\"'))
        if return_code != CMDResult.SUCCESS:
            LOGGER.error(f"Query clup db error. error out: {st_err}")
            raise Exception("Query clup db failed")
        clup_dbs = []
        result = QueryClupClusterResponse(name=self.env.get('name'), subType='PostgreClusterInstance')
        result.extend_info["clupClusterState"] = cluster_state
        if len(std_out) > 0:
            split_lines = std_out.splitlines()
            for index, row in enumerate(split_lines):
                if index < 2 or index > len(split_lines) - 3:
                    continue
                clup_db = ClupDb()
                columns = row.split("|")
                clup_db.pgdata = columns[0].strip()
                clup_db.is_primary = columns[1].strip()
                clup_db.host = columns[2].strip()
                clup_db.port = columns[3].strip()
                db_detail = json.loads(columns[4].strip())
                clup_db.pg_bin_path = db_detail.get('pg_bin_path')
                clup_db.version = db_detail.get('version')
                clup_db.os_user = db_detail.get('os_user')
                clup_dbs.append(clup_db)
                clup_cluster_node_info = ClupClusterNodeInfo(endpoint=columns[2].strip(),
                                                             subType=self.application.get('subType'),
                                                             extendInfo=json.loads(json.dumps(clup_db.__dict__)))
                result.nodes.append(clup_cluster_node_info)
        return result

    def query_clup_server_cluster(self):
        LOGGER.info('Begin to query clup server cluster by clup.conf file.')
        if not os.path.exists(PgConst.CLUP_SERVER_PATH):
            LOGGER.error(f"The path: {PgConst.CLUP_SERVER_PATH} doest not exist, Query clup server cluster failed")
            raise Exception("Query clup server cluster failed")
        clup_servers = []
        with open(PgConst.CLUP_SERVER_PATH, 'r', encoding='utf-8') as f:
            for file_line in f.readlines():
                if '#' not in file_line and 'clup_host_list' in file_line:
                    # 用正则表达式匹配CLup Server集群中所有节点IP地址
                    pattern = r'\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}'
                    clup_servers = re.findall(pattern, file_line)
        result = QueryClusterResponse(name=self.env.get('name'), subType='PostgreCluster')
        for clup_server in clup_servers:
            clup_server_node_info = ClusterNodeInfo(endpoint=clup_server)
            result.nodes.append(clup_server_node_info)
        return result

    def clup_check_application(self):
        LOGGER.info(f"Begin to check the application of clup cluster by clup server, pid: {self.pid}")
        clup_db = self.clup_query_cluster()
        if clup_db.extend_info.get('code', ExecuteResultEnum.SUCCESS) == ExecuteResultEnum.INTERNAL_ERROR:
            LOGGER.error(f"Login denied! pid: {self.pid}")
            param = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, bodyErr=ErrorCode.LOGIN_FAILED,
                                 message="Login denied.")
            return param
        if clup_db.nodes[0].extend_info.get('os_user') != self.os_username:
            LOGGER.error(f"Checkout user: {self.os_username} failed!pid: {self.pid}")
            param = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, bodyErr=ErrorCode.LOGIN_FAILED,
                                 message="Login denied.")
            return param
        result = [
            f"{domain_2_ip(i.extend_info.get('host'))}:{i.extend_info.get('port')}"
            for i in clup_db.nodes
            if i.extend_info.get('host') and i.extend_info.get('port')
        ]
        node_list = self.nodes.split(",")
        if len(result) != len(node_list):
            LOGGER.error("Missing or redundant cluster nodes.")
            return ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, bodyErr=ErrorCode.CHECK_CLUSTER_FAILED,
                                message="Missing or redundant cluster nodes.")
        res = set(result)
        node_list = set(node_list)
        if not res.difference(node_list):
            LOGGER.info(f"Success to check cluster pid: {self.pid}")
            self.version = clup_db.nodes[0].extend_info.get('version')
            return ActionResult(code=ExecuteResultEnum.SUCCESS, bodyErr=ExecuteResultEnum.SUCCESS,
                                message=json.dumps({"version": self.version}))
        LOGGER.error("Nodes not belong to a cluster.")
        param = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, bodyErr=ErrorCode.CHECK_CLUSTER_FAILED,
                             message="Nodes not belong to a cluster.")
        return param

    def check_application(self, nodes):
        LOGGER.info(f"Already get nodes: {nodes}, pid: {self.pid}")
        result = \
            [f"{domain_2_ip(i.get('hostname'))}:{i.get('port')}" for i in nodes if i.get('hostname') and i.get('port')]
        LOGGER.info(f"Already get nodes: {result}, pid: {self.pid}")
        node_list = self.nodes.split(",")
        if len(result) != len(node_list):
            return ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, bodyErr=ErrorCode.CHECK_CLUSTER_FAILED,
                                message="Missing or redundant cluster nodes.")
        res = set(result)
        node_list = set(node_list)
        LOGGER.info(f"Already get result: {res}, node_list: {node_list}, pid: {self.pid}")
        if not res.difference(node_list):
            LOGGER.info(f"Success to check cluster pid: {self.pid}")
            return ActionResult(code=ExecuteResultEnum.SUCCESS, bodyErr=ExecuteResultEnum.SUCCESS,
                                message=json.dumps({"version": self.version}))
        param = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, bodyErr=ErrorCode.CHECK_CLUSTER_FAILED,
                             message="Nodes not belong to a cluster.")
        return param

    def query_app_cluster(self, nodes):
        result = QueryClusterResponse(name=self.env.get('name'), subType='PostgreClusterInstance')
        for i in nodes:
            tmp_extend = ClusterNodeExtendInfo()
            tmp_extend.role = 1 if i.get('role') in ['primary', 'master', 'Leader'] else 2
            tmp_node = ClusterNodeInfo(endpoint=domain_2_ip(i.get('hostname')),
                                       subType=self.application.get('subType'), extendInfo=tmp_extend)
            result.nodes.append(tmp_node)
        LOGGER.info(f"Success to check cluster all_info: {result.dict(by_alias=True)}")
        return result

    def check_cluster_by_request_type(self, pg_pool_info):
        if self.deploy_type == InstallDeployType.PATRONI:
            nodes = pg_pool_info
        else:
            nodes = self.get_pg_pool_info(pg_pool_info)
        if self.check_type == "CheckApplication":
            return self.check_application(nodes)
        if self.check_type == "QueryCluster":
            return self.query_app_cluster(nodes)
        LOGGER.error(f"Unknown command: {self.check_type} from command line!")
        return ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, bodyErr=ErrorCode.GET_CLUSTER_NODE_FAILED,
                            message=f"check_type is not exist!")

    def login_database_and_execute_cmd(self, cmd, pwd=None):
        result, child = self._checkout_os_user()
        if not result:
            return False, child

        LOGGER.info(f"Ready to execute cmd:{cmd} virtual_ip:{self.virtual_ip}, pid: {self.pid}")
        child.sendline(cmd)
        index = child.expect(PexpectResult.DB_LOGIN_PASSWORD)
        if index in (0, 1):
            LOGGER.error(f"Login denied: {self.os_username}, pid: {self.pid}")
            result = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, bodyErr=ErrorCode.LOGIN_FAILED,
                                  message=f"Login denied")
            child.close()
            return False, result

        if not pwd:
            pwd = get_env_variable(f"application_auth_authPwd_{self.pid}")
        child.sendline(pwd)
        clear(pwd)
        LOGGER.info(f"Input password, pid: {self.pid}")
        index = child.expect(PexpectResult.HTML_RESULT)
        LOGGER.info(f"Get Node Info,pool:{child.before} index:{index},pid: {self.pid}")
        if index in (0, 1):
            LOGGER.error(f"Get pg pool info failed: {self.os_username}, pid: {self.pid}")
            result = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, bodyErr=ErrorCode.GET_CLUSTER_NODE_FAILED,
                                  message=f"Get pg pool info failed!")
            child.close()
            return False, result
        return True, child

    def check_pgpool_client(self):
        LOGGER.info("Begin to check pgpool client!")
        cmd = (f"{self.client_path} -h {self.virtual_ip} -p {self.port} -H -W -d postgres -c "
               f"'pgpool show pid_file_name;'")
        child = None
        try:
            result, child = self.login_database_and_execute_cmd(cmd)
            if not result:
                LOGGER.error("Check pgpool client failed, login failed!")
                return False
            exec_sql = ExecPgSql(self.pid, self.client_path, self.service_ip, self.inst_port)
            pgpool_pid = exec_sql.parse_sql_result(child.before, "pid_file_name")
            check_pgpool_pid = os.path.join(self.pgpool_client, "pgpool.pid")
            if pgpool_pid == check_pgpool_pid:
                LOGGER.info(
                    f"Check pgpool client success!")
                return True
            if os.path.isfile(check_pgpool_pid):
                LOGGER.info(f"Check pgpool client success!")
                return True
            LOGGER.error(f"Check pgpool client failed!pgpool_pid:{pgpool_pid}, check_pgpool_pid：{check_pgpool_pid}")
            return False
        finally:
            if child and not isinstance(child, ActionResult):
                child.close()

    def check_repl_user(self):
        LOGGER.info("Begin to check repl user!")
        cmd = f"{self.client_path} \"dbname=postgres replication=true\" -c \"IDENTIFY_SYSTEM;\" " \
              f"-U {self.rep_user} -h {self.service_ip} -H -W"
        repl_pwd = None
        child = None
        try:
            repl_pwd = get_env_variable(f"application_auth_extendInfo_dbStreamRepPwd_{self.pid}")
            result, child = self.login_database_and_execute_cmd(cmd, repl_pwd)
            if not result:
                LOGGER.error("Check repl user failed!")
                return False
            LOGGER.info("Check repl user success!")
            return True
        finally:
            if child and not isinstance(child, ActionResult):
                child.close()
            if repl_pwd:
                clear(repl_pwd)

    def get_nodes_domain_and_port_by_database_patroni(self):
        LOGGER.info("Begin to check patroni client!")
        cmd = cmd_format("patronictl -c {} list", self.pgpool_client)
        ret_code, std_out, std_err = execute_cmd(cmd)
        param = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, bodyErr=ErrorCode.GET_CLUSTER_NODE_FAILED,
                             message=f"check_type is not exist!")
        if ret_code != CMDResult.SUCCESS.value:
            LOGGER.info(f"get_nodes_domain_and_port_by_database_patroni, out: {std_out}, error: {std_err}")
            return param
        if len(std_out) <= 0:
            return param
        rows = std_out.splitlines()
        nodes = []
        rows_len = len(rows)
        node_list = self.nodes.split(",")
        node_list = set(node_list)
        for i in range(3, rows_len - 1):
            row = rows[i]
            columns = row.split()
            role = columns[5]
            hostname = columns[3]
            port = ''
            for temp_node in node_list:
                node_fields = temp_node.split(":")
                if node_fields[0] == hostname:
                    port = node_fields[1]
                    break
            node = {'hostname': hostname, 'role': role, 'port': port}
            nodes.append(node)
        return self.check_cluster_by_request_type(nodes)

    def _check_cluster_nodes_first(self):
        for check in (self.os_username, self.client_path):
            if not PostgreCommonUtils.check_special_characters(check):
                return ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, bodyErr=ErrorCode.CHECK_CONNECTIVITY_FAILED,
                                    message=f"String contains special characters!")
        if not PostgreCommonUtils.check_db_user_valid(self.rep_user):
            return ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, bodyErr=ErrorCode.CHECK_CONNECTIVITY_FAILED,
                                message=f"Repl user name is invalid!")
        if not PostgreCommonUtils.check_path(self.client_path):
            LOGGER.info(f"{self.client_path} is invalid.")
            return ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR,
                                bodyErr=ErrorCode.DATABASE_INSTALLATION_PATH_IS_INVALID,
                                message=f"Database installation path is invalid!")
        if self.deploy_type == InstallDeployType.PGPOOL:
            if not PostgreCommonUtils.check_path(self.pgpool_client):
                return ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR,
                                    bodyErr=ErrorCode.PGPOOL_INSTALLATION_PATH_IS_INVALID,
                                    message=f"The pgpool installation path is invalid!")
            if not check_utils.is_port(self.port) or not PostgreCommonUtils.check_port_is_listen(self.port):
                return ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, bodyErr=ErrorCode.PGPOOL_PORT_IS_INVALID,
                                    message=f"The pgpool port is invalid!")
        else:
            if not PostgreCommonUtils.check_path(self.pgpool_client) or not os.path.isfile(self.pgpool_client):
                LOGGER.info(f"{self.pgpool_client} is not a file.")
                return ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR,
                                    bodyErr=ErrorCode.FULL_PATH_OF_PATRONI_CONFIGURATION_FILE_IS_INVALID,
                                    message=f"The full path of Patroni configuration file is invalid!")

        if not check_utils.is_ip_address(self.service_ip) or self.service_ip not in PostgreCommonUtils.get_local_ips():
            LOGGER.info(f"{self.service_ip} is invalid.")
            return ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, bodyErr=ErrorCode.SERVICE_IP_IS_INVALID,
                                message=f"Service ip is invalid or can't be localhost!")
        if not check_utils.is_port(self.inst_port) or not PostgreCommonUtils.check_port_is_listen(self.inst_port):
            LOGGER.info(f"{self.inst_port} is invalid.")
            return ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, bodyErr=ErrorCode.DATABASE_PORT_IS_INVALID,
                                message=f"The inst_port is invalid!")
        if not check_utils.is_ip_address(self.virtual_ip):
            return ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, bodyErr=ErrorCode.CHECK_CONNECTIVITY_FAILED,
                                message=f"The virtual ip invalid!")
        if not PostgreCommonUtils.check_os_user(self.os_username, self.client_path, self.enable_root)[0]:
            return ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, bodyErr=ErrorCode.USER_IS_NOT_EXIST,
                                message=f"Os username is not exist!")
        return ActionResult(code=ExecuteResultEnum.SUCCESS, bodyErr=ExecuteResultEnum.SUCCESS,
                            message='check success')

    def _check_cluster_nodes(self):
        check_first_result = self._check_cluster_nodes_first()
        if check_first_result.code != ExecuteResultEnum.SUCCESS:
            return check_first_result
        get_version_res, version = get_version(self.pid, self.client_path, self.os_username, self.enable_root)
        if not get_version_res:
            return ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, bodyErr=version,
                                message="Get version failed!")
        self.version = version
        if int(self.auth_type) == AuthType.NO_AUTO.value:
            LOGGER.info(f"Begin to check cluster by os param: {self.context}, pid: {self.pid}")
            return self._get_nodes_domain_and_port_by_os()
        if int(self.auth_type) == AuthType.APP_PASSWORD.value:
            if self.deploy_type == InstallDeployType.PATRONI:
                # 检查用户名和密码,通过用户名和密码查询归档开关是否开启
                db_user_name = get_env_variable(f"application_auth_authKey_{self.pid}")
                db_pwd = get_env_variable(f"application_auth_authPwd_{self.pid}")
                db_stream_rep_pwd = get_env_variable(f"application_auth_extendInfo_dbStreamRepPwd_{self.pid}")
                if self._query_archive_mode(db_user_name,
                                            db_pwd) != CmdRetCode.EXEC_SUCCESS.value or self._query_archive_mode(
                    self.rep_user, db_stream_rep_pwd) != CmdRetCode.EXEC_SUCCESS.value:
                    return ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, bodyErr=ErrorCode.LOGIN_FAILED,
                                        message=f"Login denied")
                return self.get_nodes_domain_and_port_by_database_patroni()
            else:
                if not os.path.isfile(os.path.join(self.pgpool_client, "pgpool.pid")):
                    return ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR,
                                        bodyErr=ErrorCode.PGPOOL_INSTALLATION_PATH_IS_INVALID,
                                        message=f"The pgpool installation path is invalid!")
                if self.check_pgpool_client() and self.check_repl_user():
                    LOGGER.info(f"Begin to check cluster by database param: {self.context}, pid: {self.pid}")
                    return self._get_nodes_domain_and_port_by_database()
        return ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, bodyErr=ErrorCode.LOGIN_FAILED,
                            message=f"Login denied")

    def _query_archive_mode(self, db_user_name, db_pwd):
        sql_cmd = "show archive_mode;"
        pg_sql = ExecPgSql(self.pid, self.extend_info.get("clientPath"), self.service_ip, self.port)
        return_code, _, _ = pg_sql.exec_sql_cmd_resource(self.os_username, sql_cmd, db_user_name, db_pwd)
        return return_code

    def _get_nodes_domain_and_port_by_os(self):
        child = None
        cmd = f"{self.client_path} -h {self.virtual_ip} -p {self.port} -H -d postgres -c 'show pool_nodes'"
        try:
            result, child = self._checkout_os_user()
            if not result:
                return child
            # 以虚拟ip登录数据库
            LOGGER.info(f"Success to execute cmd:{cmd} virtual_ip:{self.virtual_ip}, pid: {self.pid}")
            child.sendline(cmd)
            index = child.expect(PexpectResult.HTML_RESULT)
            if index in (0, 1):
                LOGGER.error(f"Fail to execute cmd:{cmd} virtual_ip:{self.virtual_ip}, pid: {self.pid}")
                return ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, bodyErr=ErrorCode.GET_CLUSTER_NODE_FAILED,
                                    message=f"Fail to execute cmd:{cmd}")
            param = self.check_cluster_by_request_type(child.before)
            LOGGER.info(f"Already to get pg pool info virtual_ip:{self.virtual_ip}, pid: {self.pid}")
            return param
        finally:
            if child:
                child.close()

    def _get_nodes_domain_and_port_by_database(self):
        child = None
        cmd = f"{self.client_path} -h {self.virtual_ip} -p {self.port} -H -W -d postgres -c 'show pool_nodes'"
        try:
            result, child = self.login_database_and_execute_cmd(cmd)
            if not result:
                return child
            param = self.check_cluster_by_request_type(child.before)
            LOGGER.info(f"Check os user: {self.os_username}, pid: {self.pid}")
            child.close()
            return param
        finally:
            if child and not isinstance(child, ActionResult):
                child.close()

    def _checkout_os_user(self):
        # 登录
        child = pexpect.spawn(f"su - {self.os_username}", encoding="utf-8", timeout=20)
        LOGGER.info(f"Success to execute su - {self.os_username}, pid: {self.pid}")
        index = child.expect(PexpectResult.OS_LOGIN_RESULT)

        if index in (0, 1):
            LOGGER.error(
                f"Checkout user: {self.os_username} failed!pid: {self.pid}")
            child.close()
            param = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, bodyErr=ErrorCode.LOGIN_FAILED,
                                 message="Login failed!")
            return False, param
        LOGGER.info(f"Success check os user: {self.os_username}, pid: {self.pid}")
        return True, child


if __name__ == '__main__':
    function = sys.argv[1]
    pid = sys.argv[2]  # argv
    for line in sys.stdin:
        SysData.SYS_STDIN = line
        break
    if not is_valid_id(pid):
        LOGGER.warn(f'pid is invalid!')
        sys.exit(1)
    LOGGER.info("Begin to check Cluster!")
    checker = ClusterNodesChecker(check_type=function, request_pid=pid)
    checker.check_cluster_nodes()
