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

from common.cleaner import clear
from common.common import execute_cmd_list, execute_cmd
from common.const import IPConstant
from common.exception.common_exception import ErrCodeException
from common.parse_parafile import get_env_variable
from mysql import log
from mysql.src.common.constant import MySQLJsonConstant, MySQLStrConstant, ExecCmdResult, MysqlPrivilege, MySQLType
from mysql.src.common.error_code import MySQLErrorCode
from mysql.src.common.execute_cmd import get_operating_system, match_greatsql
from mysql.src.common.parse_parafile import ParseParaFile
from mysql.src.protect_mysql_base_utils import MysqlBaseUtils
from mysql.src.utils.common_func import validate_my_cnf, get_version_from_sql, SQLParam, exec_mysql_sql_cmd, \
    find_log_bin_path_dir, get_data_dir_from_sql, get_conf_by_key, check_cluster_sync_status, \
    get_master_ips, get_master_info, get_log_bin_from_sql, contains_all_elements, get_privilege_from_sql, \
    contains_any_elements, get_pxc_info, get_databases_from_sql, get_value
from mysql.src.utils.mysql_service_info_utils import MysqlServiceInfoUtil
from mysql.src.utils.mysql_utils import MysqlUtils


class ResourceParam:
    def __init__(self, pid):
        self.pid = pid
        try:
            self.body_param = ParseParaFile.parse_para_file(self.pid)
            self.application = self.body_param.get("application", {})
            app_extend = self.application.get("extendInfo", {})
            self.mysql_ip = get_value(app_extend, "instanceIp", "127.0.0.1")
            self.port = get_value(app_extend, "instancePort", "3306")
            self.charset = get_value(app_extend, "charset", "utf8mb4")
            self.cluster_type = app_extend.get("clusterType", "")
            self.my_cnf_path = app_extend.get("myCnfPath", "")
            self.user = get_env_variable(f"application_auth_authKey_{self.pid}")
            self.pass_wd = get_env_variable(f"application_auth_authPwd_{self.pid}")
            self.sql_param = SQLParam(self.mysql_ip, self.port, self.user, self.pass_wd, charset=self.charset)
            self.ip_list = self.body_param.get("appEnv", {}).get("endpoint", "").split(",")
        except Exception as err:
            log.info(f"error:{err}")
            raise Exception(f"Failed to parse job param file for {err}") from err

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        clear(self.pass_wd)


class MySQLResource:
    def __init__(self, param: ResourceParam):
        self.param = param
        self.sql_param: SQLParam = param.sql_param
        self.version = ""
        self.master_ips = []
        self.master_info = []
        self.node_role = 2

    def query_cluster(self):
        self.check_my_cnf_path()
        self.check_version()

    def check_version(self):
        version = get_version_from_sql(self.sql_param)
        if not version:
            raise ErrCodeException(MySQLErrorCode.ERROR_INSTANCE_IS_NOT_RUNNING)
        code, out, err = execute_cmd("mysql --version")
        if code == ExecCmdResult.SUCCESS:
            if match_greatsql(out):
                version = version + "-" + MySQLStrConstant.GREATSQLAPPLICTATION
        self.version = version

    def check_my_cnf_path(self):
        if self.param.my_cnf_path and not validate_my_cnf(self.param.my_cnf_path):
            raise ErrCodeException(MySQLErrorCode.INPUT_MY_CNF_NOT_EXIST)

    def check_application(self):
        self.check_my_cnf_path()
        self.check_version()
        self.check_log_bin_on()
        self.check_privileges()

    def check_log_bin_on(self):
        log_bin_result = get_log_bin_from_sql(self.sql_param)
        log.info(f"log_bin_result:{log_bin_result}")
        if log_bin_result and log_bin_result.lower() != "on":
            raise ErrCodeException(MySQLErrorCode.LOG_BIN_ID_OFF_ERROR)

    def check_privileges(self):
        for host in MysqlPrivilege.PRIVILEGE_IPS:
            privilege_lst = get_privilege_from_sql(self.sql_param, host)
            if not privilege_lst:
                continue
            if MysqlPrivilege.ALL_PRIVILEGES in privilege_lst:
                log.info("all privileges check success")
                return
            # 校验必要权限
            necessary_privilege = MysqlPrivilege.get_necessary_privilege(self.version)
            log.info(f"necessary_privilege:{necessary_privilege}")
            if not contains_all_elements(privilege_lst, necessary_privilege):
                log.warn(f"check necessary_privilege miss, privilege:{necessary_privilege},version:{self.version}")
                continue
            # 校验可选权限，只要包含一个即可
            optional_privilege = MysqlPrivilege.get_optional_privilege(self.version)
            if optional_privilege and not contains_any_elements(privilege_lst, optional_privilege):
                log.warn(f"check optional_privilege miss, privilege:{optional_privilege},version:{self.version}")
                continue
            log.info(f"check privileges success,version:{self.version}")
            return
        log.error(f"check privileges error,version:{self.version}")
        raise ErrCodeException(MySQLErrorCode.CHECK_PRIVILEGE_FAILED, "permission deny")

    def check_relay_log(self):
        relay_log = get_conf_by_key(self.param.my_cnf_path, "relay_log")
        if not relay_log:
            log.info("relay log config not exist")
            return
        relay_log_recovery = get_conf_by_key(self.param.my_cnf_path, "relay_log_recovery")
        if not relay_log_recovery:
            log.info("relay_log_recovery not exist")
            return
        log.info(f"relay_log:{relay_log},relay_log_recovery:{relay_log_recovery}")
        if relay_log_recovery.lower() != "on":
            log.error(f"Check relay log status field!")
            raise ErrCodeException(MySQLErrorCode.RELAY_LOG_DIRECTORY_OFF_ERROR)

    def build_check_result(self):
        cluster_type = self.param.cluster_type
        system_service_type, mysql_service_type = get_service_type(cluster_type)
        return {
            "id": self.param.application.get("id"),
            "type": self.param.application.get("type"),
            "subType": self.param.application.get("subType"),
            "extendInfo": {
                "is_master": self.node_role, "status": "0", "dataDir": get_data_dir_from_sql(self.sql_param),
                "version": self.version, "deployOperatingSystem": get_operating_system(),
                MySQLJsonConstant.SERVICE_NAME: mysql_service_type,
                MySQLJsonConstant.SYSTEM_SERVICE_TYPE_KEY: system_service_type,
                MySQLJsonConstant.LOG_BIN_INDEX_PATH: find_log_bin_path_dir(self.sql_param, self.param.my_cnf_path),
                "master_list": ",".join(self.master_ips), "master_info": json.dumps(self.master_info),
                "current_ip_list": ",".join(MysqlBaseUtils.get_local_ips())
            }
        }

    def verify_cluster_node(self):
        pass

    def list_application(self):
        self.check_version()
        cluster_type = self.param.cluster_type
        system_service_type, mysql_service_type = get_service_type(cluster_type)
        deploy_operating_system = get_operating_system()
        instance_id, instance_name = self.param.application.get("id"), self.param.application.get("name")
        database_list = get_databases_from_sql(self.sql_param, instance_id, instance_name)
        log.info(f"database_list:{database_list}")
        if not database_list:
            error_message = f"Get database failed or database is not exist"
            raise ErrCodeException(MySQLErrorCode.GET_DATABASES_FAILED, message=error_message)
        extend_info = {
            "version": self.version, "deployOperatingSystem": deploy_operating_system,
            MySQLJsonConstant.SERVICE_NAME: mysql_service_type,
            MySQLJsonConstant.SYSTEM_SERVICE_TYPE_KEY: system_service_type
        }
        log.info(f"extend_info:{extend_info}")
        for database in database_list:
            database.update({"extendInfo": extend_info, "type": MySQLType.TYPE, "subType": MySQLType.SUBTYPE})
        log.info(f"Already to get database from length: {len(database_list)}")
        return database_list


class SingleMySQLResource(MySQLResource):

    def check_application(self):
        super().check_application()
        info = get_pxc_info(self.sql_param)
        if info:
            log.error(f"pxc node can't register as single node,info:{info}")
            raise ErrCodeException(MySQLErrorCode.CHECK_MYSQL_REGISTER_TYPE_ERROR)


class APMySQLResource(MySQLResource):

    def query_cluster(self):
        super().query_cluster()
        is_standby = self.check_local_is_standby()
        log.info(f"is_standby:{is_standby}")
        if not is_standby:
            self.node_role = 1

    def check_local_is_standby(self):
        self.sql_param.sql = "show slave status"
        ret, result = exec_mysql_sql_cmd(self.sql_param)
        log.info(f"ret:{ret}, result:{result}")
        if not ret:
            # 连不上认为是备机
            return True
        # 低版本的数据库，主机查不到这个信息
        if not result:
            return False
        master_ip = result[0][1]
        local_ips = MysqlBaseUtils.get_local_ips()
        if master_ip in local_ips:
            return False
        return True

    def check_application(self):
        super().check_application()
        self.check_relay_log()

    def verify_cluster_node(self):
        master_ips = get_master_ips(self.sql_param)
        if len(master_ips) > 1:
            raise ErrCodeException(MySQLErrorCode.CHECK_CLUSTER_FAILED)
        master_host = master_ips.pop()
        if master_host not in self.param.ip_list:
            raise ErrCodeException(MySQLErrorCode.CHECK_CLUSTER_FAILED)


def check_local_is_bootstrap():
    ret, _, _ = execute_cmd_list(
        [f"systemctl status {MySQLStrConstant.MYSQLPXCSERVICES}", "grep \"Active: active\""])
    return ret == ExecCmdResult.SUCCESS


def get_service_type(cluster_type: str):
    is_running, system_service_type, mysql_service_type = MysqlServiceInfoUtil.get_mysql_service_info(cluster_type)
    if not is_running:
        system_service_type = "manual"
        mysql_service_type = "mysqld"
    return system_service_type, mysql_service_type


class PXCMySQLResource(MySQLResource):
    def query_cluster(self):
        super().query_cluster()
        if check_local_is_bootstrap():
            self.node_role = 1

    def check_application(self):
        super().check_application()
        self.check_pxc_node()

    def check_pxc_node(self):
        pxc_info_list = get_pxc_info(self.sql_param)
        log.info(f"pxc_info_list:{pxc_info_list}")
        pxc_ips = [pxc_info.get("ip") for pxc_info in pxc_info_list]
        log.info(f"pxc_ips:{pxc_ips}")
        for node_ip in MysqlBaseUtils.get_local_ips():
            if node_ip in pxc_ips:
                return
        raise ErrCodeException(MySQLErrorCode.CHECK_CLUSTER_TYPE_FAILED)

    def verify_cluster_node(self):
        pxc_info_list = get_pxc_info(self.sql_param)
        if not pxc_info_list:
            raise ErrCodeException(MySQLErrorCode.CHECK_CLUSTER_FAILED)
        all_nodes = self.param.application.get("extendInfo", {}).get("allNodes").split(",")
        if len(all_nodes) != len(pxc_info_list):
            log.error(f"Failed to check the number of cluster instances. pid: {self.param.pid}")
            raise ErrCodeException(MySQLErrorCode.CHECK_CLUSTER_NUM_FAILED)
        host_ips = MysqlBaseUtils.get_local_ips()
        current_pm_nodes = list(filter(lambda node: node.split(":")[0] in host_ips, all_nodes))
        if not current_pm_nodes:
            log.error(f"check host_ips:{host_ips} not in all_nodes:{all_nodes} error")
            raise ErrCodeException(MySQLErrorCode.CHECK_CLUSTER_NUM_FAILED)
        current_pxc_nodes = list(filter(lambda node: node.get("ip") in host_ips, pxc_info_list))
        if not current_pxc_nodes:
            log.error(f"check host_ips:{host_ips} not in pxc_ips:{pxc_info_list} error")
            raise ErrCodeException(MySQLErrorCode.CHECK_CLUSTER_NUM_FAILED)
        log.info(f"current_pm_nodes:{current_pm_nodes}, current_pxc_nodes:{current_pxc_nodes}")
        pm_node_port = current_pm_nodes[0].split(":")[1]
        pxc_node_port = current_pxc_nodes[0].get("port")
        if pm_node_port != pxc_node_port:
            log.error(f"check pm port:{pm_node_port},pxc port:{pxc_node_port} not equals")
            raise ErrCodeException(MySQLErrorCode.CHECK_AUTHENTICATION_INFO_FAILED)


class AAMySQLResource(MySQLResource):

    def query_cluster(self):
        super().query_cluster()
        self.sql_param.sql = "show slave status"
        ret, result = exec_mysql_sql_cmd(self.sql_param)
        if not ret or not result:
            log.error(f"check slave status error")
            raise ErrCodeException(MySQLErrorCode.CHECK_CLUSTER_NUM_FAILED)
        master_ip = result[0][1]
        local_ips = MysqlBaseUtils.get_local_ips()
        if master_ip in local_ips:
            log.error(f"check AA cluster error, master_ip {master_ip} in local_ips:{local_ips}")
            raise ErrCodeException(MySQLErrorCode.CHECK_CLUSTER_NUM_FAILED)
        self.node_role = 1
        master_ips = set()
        master_ips.add(master_ip)
        self.master_ips = master_ips

    def verify_cluster_node(self):
        all_nodes = self.param.application.get("extendInfo", {}).get("allNodes").split(",")
        host_ips = MysqlBaseUtils.get_local_ips()
        current_pm_nodes = filter(lambda node: node.split(":")[0] in host_ips, all_nodes)
        if not current_pm_nodes:
            log.error(f"check host_ips:{host_ips} not in all_nodes:{all_nodes} error")
            raise ErrCodeException(MySQLErrorCode.CHECK_CLUSTER_NUM_FAILED)

    def check_application(self):
        super().check_application()
        self.check_relay_log()


class EAPPMySQLResource(MySQLResource):
    def query_cluster(self):
        super().query_cluster()
        self.check_ip()
        self.check_port()
        self.check_sync_status()
        self.master_ips = get_master_ips(self.sql_param)
        self.master_info = get_master_info(self.sql_param)

    def check_ip(self):
        ip = self.sql_param.host
        if ip == IPConstant.LOCAL_HOST:
            return
        local_ips = MysqlBaseUtils.get_local_ips()
        if ip not in local_ips:
            raise ErrCodeException(MySQLErrorCode.ERR_IP, "")

    def check_port(self):
        conf_port = get_conf_by_key(self.param.my_cnf_path, "port")
        if not conf_port:
            raise ErrCodeException(MySQLErrorCode.CHECK_MYSQL_CONF_FAILED, "")
        if conf_port != self.sql_param.port:
            raise ErrCodeException(MySQLErrorCode.ERR_PORT, "")

    def check_sync_status(self):
        ret, _ = check_cluster_sync_status(self.sql_param)
        if not ret:
            raise ErrCodeException(MySQLErrorCode.ERROR_CLUSTER_SYNC_STATUS, "")

    def check_application(self):
        super().check_application()
        if not MysqlUtils.eapp_is_running():
            raise ErrCodeException(MySQLErrorCode.CHECK_MYSQL_NOT_RUNNING)
