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

from common.common import execute_cmd, output_execution_result_ex
from common.const import ParamConstant, ExecuteResultEnum, CMDResult
from common.exception.common_exception import ErrCodeException
from common.util.checkout_user_utils import get_path_owner
from tidb.common.const import TiDBSubType, ErrorCode, ClusterRequiredHost, TiDBResourceKeyName, TiDBConst, \
    TiDBDataBaseFilter, TiDBTaskType
from tidb.common.tidb_common import get_env_variable, output_result_file, write_error_to_result_file, exec_mysql_sql, \
    get_status_up_role_one_host, check_roles_up, check_paths_valid, check_params_valid
from tidb.handle.resource.parse_params import ResourceParam
from tidb.logger import log


class TiDBResourceInfo:
    def __init__(self, pid, resource_param: ResourceParam):
        self.pid = pid
        self.param = resource_param

    @staticmethod
    def write_error_param_to_result_file(path: str, code: int, error_code: int, message: str):
        output_execution_result_ex(path, {"code": error_code, "bodyErr": code, "message": message})

    def handle_tiup(self):
        # 注册集群Step1：检查tiup节点
        log.info("Start to register cluster step 1")
        body_err_code = ExecuteResultEnum.SUCCESS.value
        err_code = ExecuteResultEnum.SUCCESS.value
        err_msg = "Check success!"
        log.info("Start to check tiup node")
        try:
            self.check_tiup()
        except ErrCodeException as err_code_ex:
            log.error(f"Check tiup node failed, err_code_ex: {err_code_ex}")
            err_code = ExecuteResultEnum.INTERNAL_ERROR.value
            body_err_code = err_code_ex.error_code
            err_msg = err_code_ex.error_message
        except Exception as ex:
            log.error(f"Check tiup node failed,  ex: {ex}")
            err_code = ExecuteResultEnum.INTERNAL_ERROR.value
            body_err_code = ErrorCode.ERR_INPUT_STRING
            err_msg = "exception occurs."
        finally:
            log.info(f"Check tiup node: {err_msg}.")
            sub_type = TiDBSubType.SUBTYPE_CLUSTER
            report_dic = {"cluster_list": json.dumps([])}
            error_info = {"code": body_err_code, "bodyErr": err_code, "message": err_msg}
            write_error_to_result_file(self.pid, error_info, sub_type, report_dic)
        if err_code == ExecuteResultEnum.INTERNAL_ERROR.value:
            return False

        # 获取集群列表
        clusters = self.list_cluster()
        # 构建返回JSON
        cluster_list = {
            "type": TiDBSubType.TYPE, "subType": TiDBSubType.SUBTYPE_CLUSTER,
            "extendInfo": {"cluster_list": json.dumps(clusters)}
        }
        items = [cluster_list]
        log.info(f"Success to execute query cluster command. pid:{self.pid}, cluster:{clusters}")
        params = {"resourceList": items}
        output_result_file(self.pid, params)
        return True

    def handle_user(self):
        # 注册集群step3：校验tidb用户
        log.info("Start to register cluster step 3")
        body_err_code = ExecuteResultEnum.SUCCESS.value
        err_code = ExecuteResultEnum.SUCCESS.value
        err_msg = "Check success!"
        log.info("Start to check tidb user")
        try:
            self.check_tidb_user()
        except ErrCodeException as err_code_ex:
            log.error(f"Check user and password failed, err_code_ex: {err_code_ex}")
            err_code = ExecuteResultEnum.INTERNAL_ERROR.value
            body_err_code = err_code_ex.error_code
            err_msg = err_code_ex.error_message
        except Exception as ex:
            log.error(f"Check user and password failed,  ex: {ex}")
            err_code = ExecuteResultEnum.INTERNAL_ERROR.value
            body_err_code = ErrorCode.ERR_INPUT_STRING
            err_msg = "exception occurs."
        finally:
            log.info(f"Check user and password: {err_msg}.")
            sub_type = TiDBSubType.SUBTYPE_CLUSTER
            report_dic = {"cluster_list": json.dumps([])}
            error_info = {"code": body_err_code, "bodyErr": err_code, "message": err_msg}
            write_error_to_result_file(self.pid, error_info, sub_type, report_dic)
        if err_code == ExecuteResultEnum.INTERNAL_ERROR.value:
            return False
        log.info("Start to check log path")
        return True

    def get_tiup_path(self):
        resource_param = self.param
        action_type, condition = resource_param.get_register_info()
        tiup_path = condition.get("tiupPath", "")
        if tiup_path:
            log.info(f"tiup_path1: {tiup_path}")
        else:
            application_extend = resource_param.get_app_env_info()
            tiup_path = application_extend.get("tiupPath")
            log.info(f"tiup_path2: {tiup_path}")
            if not tiup_path:
                applications_extend = resource_param.get_list_application_extend_info()
                tiup_path = applications_extend.get("tiupPath")
                log.info(f"tiup_path3: {tiup_path}")
        if not check_paths_valid(tiup_path):
            log.error(f"tiup_path {tiup_path} is invalid")
            return ""
        return tiup_path

    def check_tiup(self):
        tiup_path = self.get_tiup_path()
        tiup_node = self.get_tiup_node()
        if not tiup_path:
            log.error("Failed get tiup path!")
        cmd_check_tiup = f"which {tiup_path}"
        ret_code, std_out, std_err = execute_cmd(cmd_check_tiup)
        if ret_code == CMDResult.FAILED.value:
            log.error(f"Check tiup node {tiup_node} failed: {std_err},")
            raise ErrCodeException(ErrorCode.CHECK_TIUP_FAILED, f"input tiup node does not exist")
        log.info(f"Check tiup node {tiup_node} succeed.")

    def get_br_version(self):
        log.info(f"Start to get br version {self.pid}")
        tiup_path = self.get_tiup_path()
        cmd_get_br_version = f"su - {get_path_owner(tiup_path)} -c '{tiup_path} br --version'"
        br_version = ""
        ret_code, std_out, std_err = execute_cmd(cmd_get_br_version)
        if ret_code == CMDResult.FAILED.value:
            log.error(f"Failed get br version: {std_err}.")
            return ""
        br_info = std_out.split("\n")
        for item in br_info:
            if TiDBConst.BR_VERSION in item:
                br_version = item.split(":")[1]
                break
        if not br_version:
            log.error("Failed get br version!")
        return br_version

    def list_cluster(self):
        # 请求集群列表
        tiup_path = self.get_tiup_path()
        cmd_list_cluster = f"su - {get_path_owner(tiup_path)} -c '{tiup_path} cluster list'"
        ret_code, std_out, std_err = execute_cmd(cmd_list_cluster)
        if ret_code == CMDResult.FAILED.value:
            log.error(f"List cluster failed: {std_err}.")
            return []
        clusters_info = std_out.split("\n")[2:]
        clusters = []
        for item in clusters_info:
            if item:
                cluster = item.split()
                cluster_info = {"cluster_name": cluster[0], "owner": cluster[1], "version": cluster[2]}
                clusters.append(cluster_info)
        return clusters

    def get_tiup_node(self):
        resource_param = self.param
        app_env = resource_param.get_app_env_info()
        node = app_env.get("tiupNode", "")
        log.info(f"Tiup node:{node}, pid:{self.pid}.")
        return node

    def list_cluster_hosts(self):
        # 注册集群Step2：列出主机信息
        log.info("Start to register cluster step 2")
        resource_param = self.param
        action_type, conditions = resource_param.get_register_info()
        cluster_name = conditions.get("clusterName", '')
        try:
            cluster_hosts = self.get_cluster_hosts(cluster_name)
        except ErrCodeException as err_code_ex:
            log.error(f"Get BR version failed, err_code_ex: {err_code_ex}")
            err_code = ExecuteResultEnum.INTERNAL_ERROR.value
            body_err_code = err_code_ex.error_code
            err_msg = err_code_ex.error_message
            sub_type = TiDBSubType.SUBTYPE_CLUSTER
            report_dic = {"cluster_list": json.dumps([]), "br_version": json.dumps("")}
            error_info = {"code": body_err_code, "bodyErr": err_code, "message": err_msg}
            write_error_to_result_file(self.pid, error_info, sub_type, report_dic)
            return False
        try:
            self.hosts_check(cluster_hosts)
        except ErrCodeException as err_code_ex:
            log.error(f"Host not up, err_code_ex: {err_code_ex}")
            err_code = ExecuteResultEnum.INTERNAL_ERROR.value
            body_err_code = err_code_ex.error_code
            err_msg = err_code_ex.error_message
            sub_type = TiDBSubType.SUBTYPE_CLUSTER
            report_dic = {"cluster_list": json.dumps([]), "br_version": json.dumps("")}
            error_info = {"code": body_err_code, "bodyErr": err_code, "message": err_msg}
            write_error_to_result_file(self.pid, error_info, sub_type, report_dic)
        return True

    def get_cluster_hosts(self, cluster_name):
        log.info("start to get_cluster_hosts")
        if not cluster_name:
            log.error("No cluster name")
            return []
        log.info(f"cluster_name {cluster_name}")
        # 获取br版本
        br_version = self.get_br_version()
        if not br_version:
            raise ErrCodeException(ErrorCode.ERR_GET_BR_VERSION, f"get br version failed")

        # 请求集群主机信息
        tiup_path = self.get_tiup_path()
        if not check_params_valid(cluster_name):
            return []
        cmd_list_cluster = f"su - {get_path_owner(tiup_path)} -c '{tiup_path} cluster display {cluster_name}'"
        ret_code, std_out, std_err = execute_cmd(cmd_list_cluster)
        if ret_code == CMDResult.FAILED.value:
            log.error(f"List hosts info for cluster {cluster_name}  failed. Error: {std_err}")
            return []
        # 检查ctl
        if not self.check_ctl(tiup_path, br_version):
            raise ErrCodeException(ErrorCode.ERR_CHECK_CTL_VERSION, f"Check ctl component failed")

        hosts_info = std_out.split("\n")
        strip_line = 0
        if not hosts_info[-1]:
            strip_line += 1
        hosts_count_str = hosts_info[- strip_line - 1]
        if not hosts_count_str:
            return []
        hosts_count = int(hosts_count_str.split()[-1])
        hosts = hosts_info[-hosts_count - strip_line - 1: -strip_line - 1]
        hosts_list = []
        required_host_type = {
            ClusterRequiredHost.PD, ClusterRequiredHost.TIKV, ClusterRequiredHost.TIFLASH, ClusterRequiredHost.TIDB
        }
        for item in hosts:
            host = item.split()
            if host and host[1] in required_host_type:
                host_status = host[5].split("|")
                if "Up" in host_status:
                    status = "up"
                else:
                    status = "down"
                host_info = {"id": host[0], "role": host[1], "host": host[2], "status": status}
                hosts_list.append(host_info)

        cluster_info_list = {
            "type": TiDBSubType.TYPE, "subType": TiDBSubType.SUBTYPE_CLUSTER,
            "extendInfo": {"cluster_info_list": json.dumps(hosts_list), "br_version": json.dumps(br_version)}
        }
        items = [cluster_info_list]
        result_param = {"resourceList": items}
        log.info(f"End to execute query cluster hosts command. cluster_info_list.")
        output_result_file(self.pid, result_param)
        return hosts_list

    def get_tidb_node(self, tidb_role):
        resource_param = self.param
        extend_info = resource_param.get_list_application_extend_info()
        if not extend_info:
            extend_info = resource_param.get_extend_info()
        cluster_info_list = extend_info.get("clusterInfoList", '[]')
        host = ''
        port = 0
        tidb_id = ''
        for cluster_node in json.loads(cluster_info_list):
            role = cluster_node.get("role", '')
            if role == tidb_role:
                tidb_id = cluster_node.get("id", '')
                break
        if tidb_id:
            tidb_host_port = tidb_id.split(":")
            host = tidb_host_port[0]
            port = int(tidb_host_port[1])
        return host, port

    def check_tidb_user(self):
        log.info("Start to check tidb user.")
        user = get_env_variable(TiDBResourceKeyName.APPLICATION_AUTH_AUTHKEY + self.pid)
        if not check_params_valid(user):
            log.error(f"user: {user} is invalid")
            raise ErrCodeException(ErrorCode.TIDB_USER_PRIVILEGE_FAILED,
                                   f"input tidb user does not have backup privilege")
        priv_ips = [TiDBConst.ALL_HOSTS]
        need_privs = [TiDBConst.DROP, TiDBConst.SELECT]
        cluster_name = self.get_cluster_name()
        log.info(f"cluster_name: {cluster_name}")
        tiup_path = self.get_tiup_path()
        tidb_host = get_status_up_role_one_host(cluster_name, tiup_path, ClusterRequiredHost.TIDB)
        if not tidb_host:
            raise ErrCodeException(ErrorCode.CHECK_PD_TIDB_FAILED, f"all tidb hosts down")
        tidb_id = tidb_host.split(":")

        host = tidb_id[0]
        port = int(tidb_id[1])
        log.info(f"tidb_id: {tidb_id}")

        log.debug(f"check_tidb_user: {host}', {port}")
        for priv_ip in priv_ips:
            cnt = 0
            priv_cmd = [f"show grants for '{user}'@'{priv_ip}';"]
            ret, output = exec_mysql_sql(TiDBTaskType.RESOURCE, self.pid, priv_cmd, host, port)
            if TiDBConst.ALL_PRIVILEGE in str(output):
                return True
            if not ret:
                continue
            for iter_priv in need_privs:
                if iter_priv in str(output):
                    cnt = cnt + 1
            if cnt == len(need_privs):
                return True
        raise ErrCodeException(ErrorCode.TIDB_USER_PRIVILEGE_FAILED, f"input tidb user does not have backup privilege")

    def get_log_path(self):
        resource_param = self.param
        log_path = resource_param.get_list_application_extend_info().get("logBackupPath", "")
        log.info(f"Log path: {log_path}")
        if not log_path:
            log_path = resource_param.get_app_env_info().get("logBackupPath", "")
            log.info(f"Log_path: {log_path}")
            if not log_path:
                log.error("Failed get Log path")
        return log_path

    def hosts_check(self, hosts_list):
        log.info(f"Start to check hosts {self.pid}")
        for host in hosts_list:
            if host.get("status", "") == "down":
                raise ErrCodeException(ErrorCode.HOST_NOT_UP, f"host not up")

    def handle_cluster_hosts(self, sub_type):
        resource_param = self.param
        action_type, conditions = resource_param.get_register_info()
        cluster_name = conditions.get("clusterName", '')
        hosts_list = self.get_cluster_hosts(cluster_name)
        body_err_code = ExecuteResultEnum.SUCCESS.value
        err_code = ExecuteResultEnum.SUCCESS.value
        err_msg = "Check success!"
        log.info(f"Start to check hosts status for cluster {cluster_name}.")
        try:
            self.hosts_check(hosts_list)
        except ErrCodeException as err_code_ex:
            log.error(f"Check hosts status, err_code_ex: {err_code_ex}")
            err_code = ExecuteResultEnum.INTERNAL_ERROR.value
            body_err_code = err_code_ex.error_code
            err_msg = err_code_ex.error_message
        finally:
            log.info(f"Check hosts status: {err_msg}.")
            report_dic = {"info_list": json.dumps([])}
            error_info = {"code": body_err_code, "bodyErr": err_code, "message": err_msg}
            write_error_to_result_file(self.pid, error_info, sub_type, report_dic)
        if err_code == ExecuteResultEnum.INTERNAL_ERROR.value:
            return False
        return True

    def get_db(self):
        tiup_path = self.get_tiup_path()
        cluster_name = self.get_cluster_name()
        tidb_host = get_status_up_role_one_host(cluster_name, tiup_path, ClusterRequiredHost.TIDB)
        if not tidb_host:
            raise ErrCodeException(ErrorCode.CHECK_PD_TIDB_FAILED, f"all tidb hosts down")

        tidb_id = tidb_host.split(":")
        host = tidb_id[0]
        port = int(tidb_id[1])

        list_cmd = [f"show databases;"]
        ret, output = exec_mysql_sql(TiDBTaskType.RESOURCE, self.pid, list_cmd, host, port)
        if not ret:
            log.error("Get database list failed!")
            return []
        db_list = []
        db_filter = {
            TiDBDataBaseFilter.MYSQL, TiDBDataBaseFilter.METRICS_SCHEMA, TiDBDataBaseFilter.INFORMATION_SCHEMA,
            TiDBDataBaseFilter.PERFORMANCE_SCHEMA, TiDBDataBaseFilter.TIDB_BR_TEMP
        }
        for item in output:
            if not str(item[0]) in db_filter:
                db_list.append(str(item[0]))
        result_param = {
            "type": TiDBSubType.TYPE, "subType": TiDBSubType.SUBTYPE_DATABASE,
            "extendInfo": {"database": json.dumps(db_list)}
        }
        log.info(f"Success to execute get_db command, database:{db_list}")
        items = [result_param]
        param = {"resourceList": items}
        output_result_file(self.pid, param)
        return db_list

    def register_check_hosts(self, sub_type):
        # 注册库表，校验主机状态
        if sub_type == TiDBSubType.SUBTYPE_DATABASE:
            report_key = "database"
        elif sub_type == TiDBSubType.SUBTYPE_TABLE:
            report_key = "table_list"
        cluster_name = self.get_cluster_name()
        tiup_path = self.get_tiup_path()
        ret_host = check_roles_up(cluster_name, tiup_path,
                                  [ClusterRequiredHost.PD, ClusterRequiredHost.TIDB, ClusterRequiredHost.TIKV,
                                   ClusterRequiredHost.TIFLASH])
        down_role = ret_host.get("down")
        if down_role:
            if down_role in [ClusterRequiredHost.PD, ClusterRequiredHost.TIDB]:
                err_code = ExecuteResultEnum.INTERNAL_ERROR.value
                body_err_code = ErrorCode.CHECK_PD_TIDB_FAILED
            elif down_role in [ClusterRequiredHost.TIKV, ClusterRequiredHost.TIFLASH]:
                err_code = ExecuteResultEnum.INTERNAL_ERROR.value
                body_err_code = ErrorCode.TIKV_TIFLASH_DIFFERENT
            err_msg = f"{down_role} host down!"
            log.error(f"{down_role} host down!")
            report_dic = {report_key: json.dumps([])}
            error_info = {"code": body_err_code, "bodyErr": err_code, "message": err_msg}
            write_error_to_result_file(self.pid, error_info, sub_type, report_dic)
            return False
        return True

    def handle_db(self):
        # 注册数据库
        log.info("Start to register database.")
        ret_tiup = self.handle_tiup()
        if not ret_tiup:
            log.error("Check tiup node failed!")
            return False
        sub_type = TiDBSubType.SUBTYPE_DATABASE
        # 校验主机状态
        ret_host = self.register_check_hosts(sub_type)
        if not ret_host:
            return False

        ret_user = self.handle_user()
        if not ret_user:
            log.error("Check user failed!")
            return False
        try:
            self.get_db()
        except ErrCodeException as err_code_ex:
            log.error(f"All tidb hosts down, get db failed, err_code_ex: {err_code_ex}")
            err_code = ExecuteResultEnum.INTERNAL_ERROR.value
            body_err_code = err_code_ex.error_code
            err_msg = err_code_ex.error_message
            report_dic = {"database": json.dumps([])}
            error_info = {"code": body_err_code, "bodyErr": err_code, "message": err_msg}
            write_error_to_result_file(self.pid, error_info, sub_type, report_dic)
        return True

    def get_db_name(self):
        resource_param = self.param
        db_name = resource_param.get_app_env_info().get("databaseName")
        return db_name

    def get_db_name_register(self):
        resource_param = self.param
        action_type, conditions = resource_param.get_register_info()
        db_name = conditions.get("databaseName")
        return db_name

    def get_table_name(self):
        resource_param = self.param
        table_name_list = []
        table_name_str = resource_param.get_app_env_info().get("tableNameList", '')
        if table_name_str:
            table_name_list = json.loads(table_name_str)
        return table_name_list

    def get_table(self, db_name):
        if not check_params_valid(db_name):
            log.error(f"The db_name {db_name} verification fails")
            raise ErrCodeException(ErrorCode.TIDB_DB_NOT_EXIST, f"input db name is invalid")
        tiup_path = self.get_tiup_path()
        cluster_name = self.get_cluster_name()
        tidb_host = get_status_up_role_one_host(cluster_name, tiup_path, ClusterRequiredHost.TIDB)
        if not tidb_host:
            raise ErrCodeException(ErrorCode.CHECK_PD_TIDB_FAILED, f"all tidb hosts down")

        tidb_id = tidb_host.split(":")
        host = tidb_id[0]
        port = int(tidb_id[1])
        # 分页获取表
        resource_param = self.param
        condition = resource_param.get_param().get("condition", {})

        limit = condition.get("pageSize", 20)
        offset = condition.get("pageNo", 0) * limit
        tables_limit_cmd = [
            f"SELECT TABLE_NAME FROM INFORMATION_SCHEMA.TABLES WHERE TABLE_SCHEMA=\"{db_name}\" "
            f"ORDER BY TABLE_NAME;"
        ]
        ret, output = exec_mysql_sql(TiDBTaskType.RESOURCE, self.pid, tables_limit_cmd, host, port)
        table_list = []
        if not ret:
            log.error(f"Get tables in database {db_name} failed!")
        else:
            for item in output:
                tb_name = item[0]
                table_list.append(tb_name)

        # 获取表的总数
        table_num = 0
        table_number_cmd = [f"SELECT COUNT(*) FROM INFORMATION_SCHEMA.TABLES WHERE TABLE_SCHEMA=\"{db_name}\";"]
        ret, output = exec_mysql_sql(TiDBTaskType.RESOURCE, self.pid, table_number_cmd, host, port)
        if not ret:
            log.error(f"Get table num in database {db_name} failed!")
        else:
            table_num = output[0][0]

        result_param = {
            "type": TiDBSubType.TYPE, "subType": TiDBSubType.SUBTYPE_TABLE,
            "extendInfo": {"table_list": json.dumps(table_list), "table_num": json.dumps(table_num)}
        }
        items = [result_param]
        param = {"resourceList": items}
        output_result_file(self.pid, param)
        return table_list

    def handle_table(self):
        # 注册表
        log.info("Start to register table.")
        ret_tiup = self.handle_tiup()
        if not ret_tiup:
            log.error("Check tiup node failed!")
            return False
        sub_type = TiDBSubType.SUBTYPE_TABLE
        # 校验主机状态
        ret_host = self.register_check_hosts(sub_type)
        if not ret_host:
            return False
        ret_user = self.handle_user()
        if not ret_user:
            log.error("Check user failed!")
            return False
        db_name = self.get_db_name_register()
        try:
            table_list = self.get_table(db_name)
        except ErrCodeException as err_code_ex:
            log.error(f"All tidb hosts down, get tables failed, err_code_ex: {err_code_ex}")
            err_code = ExecuteResultEnum.INTERNAL_ERROR.value
            body_err_code = err_code_ex.error_code
            err_msg = err_code_ex.error_message
            sub_type = TiDBSubType.SUBTYPE_TABLE
            report_dic = {"tables": json.dumps([])}
            error_info = {"code": body_err_code, "bodyErr": err_code, "message": err_msg}
            write_error_to_result_file(self.pid, error_info, sub_type, report_dic)
        log.info(f"End to list table {table_list}.")
        return True

    def get_cluster_hosts_pm(self):
        resource_param = self.param
        extend_info = resource_param.get_extend_info()
        cluster_info_list = extend_info.get("clusterInfoList", '[]')
        cluster_hosts_pm = json.loads(cluster_info_list)
        return cluster_hosts_pm

    def compare_cluster_hosts(self, hosts_list_from_cluster, hosts_list_from_pm):
        log.info(f"Start to compare cluster tikv、tiflash hosts {self.pid}.")
        pm_hosts_list = []
        for host in hosts_list_from_pm:
            host_id = host.get("id", "")
            pm_hosts_list.append(host_id)
        for item in hosts_list_from_cluster:
            role = item.get("role")
            host_id = item.get("id")
            if role == "tikv" or role == "tiflash":
                if item.get("status") == "down":
                    return False
                if host_id not in pm_hosts_list:
                    log.error(f"Host {item} not registered")
                    return False
        return True

    def handle_compare_cluter_hosts(self):
        # 检查集群主机tikv、tiflash是否扩缩容
        # 检查集群上tikv、tiflash全都在线
        resource_param = self.param
        extend_info = resource_param.get_app_env_info()
        cluster_name = extend_info.get("clusterName", '')

        hosts_list_from_cluster = self.get_cluster_hosts(cluster_name)
        hosts_list_from_pm = self.get_cluster_hosts_pm()
        ret_compare = self.compare_cluster_hosts(hosts_list_from_cluster, hosts_list_from_pm)
        if not ret_compare:
            log.error("Tidb cluster hosts info have changed.")
            return False
        log.info("Handle_compare_cluter_hosts success!")
        return True

    def check_pd_tidb_status(self):
        # 至少一个pd、tidb在线
        resource_param = self.param
        extend_info = resource_param.get_app_env_info()
        cluster_name = extend_info.get("clusterName", '')
        hosts_list_from_cluster = self.get_cluster_hosts(cluster_name)
        hosts_list_from_pm = self.get_cluster_hosts_pm()
        host_pm = {"tikv": [], "tiflash": [], "pd": [], "tidb": []}
        host_check = {"tikv": 0, "tiflash": 0, "pd": 0, "tidb": 0}
        for host in hosts_list_from_pm:
            role = host.get("role")
            host_id = host.get("id")
            try:
                host_pm[role].append(host_id)
            except KeyError as err:
                log.error("err")
        for host in hosts_list_from_cluster:
            role = host.get("role")
            host_id = host.get("id")
            status = host.get("status")
            if host_id in host_pm.get(role) and status == "up":
                try:
                    host_check[role] += 1
                except Exception as err:
                    log.error("err")
        for key, value in host_check.items():
            if key == "pd" or key == "tidb":
                if value == 0:
                    return False
        log.info("Check_pd_tidb_status for pd、tidb success!")
        return True

    def get_cluster_name(self):
        resource_param = self.param
        extend_info = resource_param.get_extend_info()
        cluster_name = extend_info.get("clusterName", '')
        if not cluster_name:
            resource_param = self.param
            extend_info = resource_param.get_app_env_info()
            cluster_name = extend_info.get("clusterName", '')
            if not cluster_name:
                action_type, conditions = resource_param.get_register_info()
                cluster_name = conditions.get("clusterName", '')
                if not cluster_name:
                    log.error("No Cluster Name!")
        return cluster_name

    def check_cluster_exist(self):
        clusters = self.list_cluster()
        cluster_name = self.get_cluster_name()
        for cluster in clusters:
            if cluster.get("cluster_name") == cluster_name:
                return True
        return False

    def check_cluster(self):
        log.info("Start to check cluster")
        result_path = os.path.join(ParamConstant.RESULT_PATH, f"result{self.pid}")
        body_err_code = ExecuteResultEnum.SUCCESS.value
        err_code = ExecuteResultEnum.SUCCESS.value
        err_msg = "Check success!"
        # 检查tiup
        ret_tiup = self.handle_tiup()
        if not ret_tiup:
            TiDBResourceInfo.write_error_param_to_result_file(result_path, ErrorCode.CHECK_TIUP_FAILED.value,
                                                              ExecuteResultEnum.INTERNAL_ERROR.value,
                                                              "Check Tiup Failed!")
            return False
        # 检查集群是否存在
        ret_cluster = self.check_cluster_exist()
        if not ret_cluster:
            TiDBResourceInfo.write_error_param_to_result_file(result_path, ErrorCode.TIDB_CLUSTER_NOT_EXIST.value,
                                                              body_err_code, "Check cluster exist Failed!")
            return False
        # 检查集群pd、tidb在线状态
        ret_cluster_hosts = self.check_pd_tidb_status()
        if not ret_cluster_hosts:
            TiDBResourceInfo.write_error_param_to_result_file(result_path, ErrorCode.CHECK_PD_TIDB_FAILED.value,
                                                              ExecuteResultEnum.INTERNAL_ERROR.value,
                                                              "Check pd、tidb Failed!")
            return False
        # 检查集群tikv、tiflash扩缩容和在线状态
        ret_compare_hosts = self.handle_compare_cluter_hosts()
        if not ret_compare_hosts:
            TiDBResourceInfo.write_error_param_to_result_file(result_path, ErrorCode.TIKV_TIFLASH_DIFFERENT.value,
                                                              ExecuteResultEnum.INTERNAL_ERROR.value,
                                                              "Check tikv、tiflash Failed!")
            return False
        # 检查用户是否有备份权限
        try:
            self.check_tidb_user()
        except ErrCodeException as err_code_ex:
            TiDBResourceInfo.write_error_param_to_result_file(result_path, ErrorCode.TIDB_USER_PRIVILEGE_FAILED.value,
                                                              ExecuteResultEnum.INTERNAL_ERROR.value,
                                                              "Check user Failed!")
            return False
        TiDBResourceInfo.write_error_param_to_result_file(result_path, err_code, body_err_code, err_msg)
        return True

    def check_database(self):
        log.info("Start to check database")
        result_path = os.path.join(ParamConstant.RESULT_PATH, f"result{self.pid}")
        body_err_code = ExecuteResultEnum.SUCCESS.value
        err_code = ExecuteResultEnum.SUCCESS.value
        err_msg = "Check success!"
        db_name = self.get_db_name()
        ret_cluster = self.check_cluster()
        if not ret_cluster:
            return False
        db_list = self.get_db()
        if not db_list:
            log.error("get db list failed!")
            return False
        if db_name not in db_list:
            log.error("db not exist!")
            body_err_code = ExecuteResultEnum.INTERNAL_ERROR.value
            err_code = ErrorCode.TIDB_DB_NOT_EXIST.value
            err_msg = "Database not exist!"
            TiDBResourceInfo.write_error_param_to_result_file(result_path, err_code, body_err_code, err_msg)
            return False
        TiDBResourceInfo.write_error_param_to_result_file(result_path, err_code, body_err_code, err_msg)
        log.info(f"Check db {db_name} success!")
        return True

    def check_table_list(self, host, port, db_name, table_name_list):
        if not check_params_valid(db_name):
            log.error(f"The db_name {db_name} verification fails")
            return False
        if not check_params_valid(*table_name_list):
            log.error(f"The table_name_list {table_name_list} verification fails")
            return False
        if len(table_name_list) == 1:
            tb_name_tuple = f"(\"{table_name_list[0]}\")"
        else:
            tb_name_tuple = tuple(table_name_list)
        log.info(f"tb_name_tuple {tb_name_tuple}")
        tables_exist_cmd = [
            f"SELECT TABLE_NAME FROM INFORMATION_SCHEMA.TABLES WHERE TABLE_SCHEMA=\"{db_name}\" "
            f"AND TABLE_NAME IN {tb_name_tuple};"
        ]
        ret, output = exec_mysql_sql(TiDBTaskType.RESOURCE, self.pid, tables_exist_cmd, host, port)
        if not ret:
            log.error(f"execute tables_exist_cmd {tables_exist_cmd} failed!")
            return False
        exist_num = len(output)
        log.info(f"exist tables: {output}")
        log.info(f"exist table num: {exist_num}")
        if exist_num == len(table_name_list):
            return True
        else:
            return False

    def check_table(self):
        log.info("Start to check table")
        result_path = os.path.join(ParamConstant.RESULT_PATH, f"result{self.pid}")
        body_err_code = ExecuteResultEnum.SUCCESS.value
        err_code = ExecuteResultEnum.SUCCESS.value
        err_msg = "Check success!"
        table_name_list = self.get_table_name()
        log.info(table_name_list)
        ret_database = self.check_database()
        if not ret_database:
            return False
        db_name = self.get_db_name()
        tiup_path = self.get_tiup_path()
        cluster_name = self.get_cluster_name()
        tidb_host = get_status_up_role_one_host(cluster_name, tiup_path, ClusterRequiredHost.TIDB)
        if not tidb_host:
            raise ErrCodeException(ErrorCode.CHECK_PD_TIDB_FAILED, f"all tidb hosts down")

        tidb_id = tidb_host.split(":")
        host = tidb_id[0]
        port = int(tidb_id[1])
        ret_check_table_list = self.check_table_list(host, port, db_name, table_name_list)
        if not ret_check_table_list:
            log.error("table not exist!")
            body_err_code = ExecuteResultEnum.INTERNAL_ERROR.value
            err_code = ErrorCode.TIDB_TABLE_NOT_EXIST.value
            err_msg = "Table not exist!"
            TiDBResourceInfo.write_error_param_to_result_file(result_path, err_code, body_err_code, err_msg)
            return False
        TiDBResourceInfo.write_error_param_to_result_file(result_path, err_code, body_err_code, err_msg)
        log.info(f"Check table {table_name_list} success!")
        return True

    def check_ctl(self, tiup_path, br_version):
        ctl_cmd = f"su - {get_path_owner(tiup_path)} -c '{tiup_path} ctl:{br_version.strip()} tikv -V'"
        ret_code, std_out, std_err = execute_cmd(ctl_cmd)
        if ret_code == CMDResult.FAILED.value:
            log.error(f"Check ctl component {ctl_cmd} failed, pid: {self.pid}. Error: {std_err}")
            return False
        log.info(f"Check ctl component {ctl_cmd} success, pid: {self.pid}.")
        return True
