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

from common.cleaner import clear
from common.const import RepositoryDataTypeEnum
from common.parse_parafile import get_env_variable
from common.util.kmc_utils import Kmc
from mysql import log
from mysql.src.common.constant import MariaDBNeedExcludeDir, SystemServiceType, MySQLParamType, RoleType, \
    MySQLClusterType
from mysql.src.common.execute_cmd import safe_get_environ
from mysql.src.common.parse_parafile import ParseParaFile, ReadFile
from mysql.src.protect_mysql_base_utils import MysqlBaseUtils
from mysql.src.utils.common_func import get_value, SQLParam


class RestoreParam:
    def __init__(self, pid, json_param):
        self.pid = pid
        try:
            if json_param:
                self.old_param = True
            else:
                self.old_param = False
                json_param = ParseParaFile.parse_para_file(self.pid)
            self.json_param = json_param.get("job", {})
            job_extend = self.json_param.get("extendInfo", {})
            self.restore_time_stamp = job_extend.get("restoreTimestamp", "")
            self.target_object = self.json_param.get("targetObject", {})
            self.cluster_type = self.target_object.get("extendInfo", {}).get("clusterType")
            self.endpoint = self.json_param.get("targetEnv", {}).get("endpoint")
            self.sub_type = self.target_object.get("subType")
            if self.cluster_type:
                self.set_cluster_sql_param()
            else:
                self.set_single_sql_param()
            self.set_other_param()
            self.cache_path = self.get_mount_path(RepositoryDataTypeEnum.CACHE_REPOSITORY.value)
            self.log_path = self.get_mount_path(RepositoryDataTypeEnum.LOG_REPOSITORY.value)
            self.data_path = self.get_mount_path(RepositoryDataTypeEnum.DATA_REPOSITORY.value)
            self.meta_path = self.get_mount_path(RepositoryDataTypeEnum.META_REPOSITORY.value)
        except Exception as err:
            log.info(f"error:{err}")
            raise Exception(f"Failed to parse job param file for {err}") from err

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        clear(self.pass_wd)

    @property
    def get_copy_database(self):
        copy_path = self.restore_copy_path
        for path in os.listdir(copy_path):
            if path == MariaDBNeedExcludeDir.ROCKSDB or \
                    path.startswith(MariaDBNeedExcludeDir.HASGTAG):
                log.info(f"exclude dir {path}.")
                continue
            new_path = os.path.join(copy_path, path)
            if os.path.isdir(new_path):
                return path
        return ""

    @property
    def target_database(self):
        return self.target_object.get("name", "")

    @property
    def newname_database(self):
        job_extend = self.json_param.get("extendInfo", {})
        return get_value(job_extend, "newDatabaseName", "")

    @property
    def restore_copy_path(self):
        for path in os.listdir(self.data_path):
            new_path = os.path.join(self.data_path, path)
            if os.path.isdir(new_path) and "mysql" in new_path and new_path.endswith(f"_1"):
                return new_path
        return ""

    @property
    def restore_copy_id(self):
        job_extend = self.json_param.get("extendInfo", {})
        return job_extend.get("restoreCopyId")

    @property
    def version(self):
        return self.target_object.get("extendInfo", {}).get("version")

    @property
    def origin_log_path(self):
        return os.path.dirname(self.get_mount_path(RepositoryDataTypeEnum.LOG_REPOSITORY.value))

    @property
    def get_force_recovery(self):
        job_extend = self.json_param.get("extendInfo", {})
        force_recovery = int(job_extend.get("forceRecovery", 0))
        if force_recovery == 1:
            return "--innodb-force-recovery=1"
        return ""

    @property
    def is_pxc_node(self):
        if not self.cluster_type:
            return False
        return self.cluster_type == MySQLClusterType.PXC

    @property
    def is_ap_node(self):
        if not self.cluster_type:
            return False
        return self.cluster_type == MySQLClusterType.AP

    @property
    def forbidden_strict_mode(self):
        job_extend = self.json_param.get("extendInfo", {})
        return int(get_value(job_extend, "forbiddenStrictMode", "0"))

    @property
    def channel_number(self):
        try:
            backup_task_sla = json.loads(self.json_param.get("extendInfo", {}).get("backupTask_sla", {}))
            policy_list = backup_task_sla.get("policy_list", [])
            return int(policy_list[0].get("ext_parameters", {}).get("channel_number"))
        except Exception as err:
            log.error(f"get_channel_number error,{err}")
            return 1

    @property
    def is_restore_table_space(self):
        if self.sub_type != MySQLParamType.DATABASE:
            return False
        if not self.cluster_type:
            return True
        if self.cluster_type != MySQLClusterType.PXC:
            return True
        return self.role_type == RoleType.ACTIVE_NODE

    @property
    def is_active_node(self):
        if not self.cluster_type:
            return True
        return self.role_type == RoleType.ACTIVE_NODE

    @property
    def is_pxc_master(self):
        return self.is_pxc_node and self.is_active_node

    def get_mount_path(self, repo_type):
        copies_json = self.json_param.get("copies", [])
        for copy_json in copies_json:
            repositories_json = copy_json.get("repositories", [])
            for repo in repositories_json:
                repository_type = repo.get("repositoryType", "")
                if str(repository_type) == str(repo_type):
                    path = repo.get("path", [""])[0]
                    return path
        return ""

    def set_cluster_sql_param(self):
        current_ips = MysqlBaseUtils.get_local_ips()
        cluster_nodes = self.json_param.get("targetEnv", {}).get("nodes", [])
        for index, node in enumerate(cluster_nodes):
            if node.get("endpoint") not in current_ips:
                continue
            node_extend = node.get("extendInfo")
            auth_extend = node.get("auth")
            self.mysql_ip = get_value(node_extend, "instanceIp", "127.0.0.1")
            self.port = get_value(auth_extend, "instancePort", "3306")
            self.charset = get_value(node_extend, "charset", "utf8mb4")
            self.my_cnf_path = node_extend.get("myCnfPath", "")
            self.log_bin_path = get_value(node_extend, "logBinIndexPath", "")
            if self.old_param:
                self.user = safe_get_environ(f"job_targetEnv_nodes_{index}_auth_authKey_{self.pid}")
                self.pass_wd = safe_get_environ(f"job_targetEnv_nodes_{index}_auth_authPwd_{self.pid}")
            else:
                self.user = get_env_variable(f"job_targetEnv_nodes_{index}_auth_authKey_{self.pid}")
                self.pass_wd = get_env_variable(f"job_targetEnv_nodes_{index}_auth_authPwd_{self.pid}")
            self.sql_param = SQLParam(self.mysql_ip, self.port, self.user, self.pass_wd, charset=self.charset)
            break

    def set_single_sql_param(self):
        target_extend = self.json_param.get("targetObject", {}).get("extendInfo", {})
        self.mysql_ip = get_value(target_extend, "instanceIp", "127.0.0.1")
        self.port = get_value(target_extend, "instancePort", "3306")
        self.charset = get_value(target_extend, "charset", "utf8mb4")
        self.my_cnf_path = target_extend.get("myCnfPath", "")
        self.log_bin_path = get_value(target_extend, "logBinIndexPath", "")
        if self.old_param:
            self.user = safe_get_environ(f"job_targetObject_auth_authKey_{self.pid}")
            self.pass_wd = safe_get_environ(f"job_targetObject_auth_authPwd_{self.pid}")
        else:
            self.user = get_env_variable(f"job_targetObject_auth_authKey_{self.pid}")
            self.pass_wd = get_env_variable(f"job_targetObject_auth_authPwd_{self.pid}")
        self.sql_param = SQLParam(self.mysql_ip, self.port, self.user, self.pass_wd, charset=self.charset)
        self.role_type = "1"

    def set_other_param(self):
        current_ips = MysqlBaseUtils.get_local_ips()
        cluster_nodes = self.json_param.get("targetEnv", {}).get("nodes", [])
        for node in cluster_nodes:
            if node.get("endpoint") not in current_ips:
                continue
            node_extend = node.get("extendInfo")
            self.role_type = get_value(node_extend, "role", "1")
            self.service_name = get_value(node_extend, "serviceName", "mysqld")
            self.service_type = get_value(node_extend, "systemServiceType", SystemServiceType.SYSTEMCTL)
            self.data_dir = get_value(node_extend, "dataDir", "")
            self.log_index_path = get_value(node_extend, "logBinIndexPath", "")

    def reset_sql_param(self):
        if self.sub_type == MySQLParamType.DATABASE:
            return
        try:
            connect_param_path = os.path.join(self.data_path, "connect_param.json")
            connect_param = ReadFile.read_param_file(connect_param_path)
        except Exception as exception_str:
            log.error(f"read connect param err:{exception_str}")
            connect_param = {}
        if connect_param:
            mysql_user = connect_param.get("user")
            encrypt_pwd = connect_param.get("passwd")
            try:
                mysql_passwd = Kmc().decrypt(encrypt_pwd)
            except Exception as err:
                log.warning(f"decrypt error,{err}")
                mysql_passwd = encrypt_pwd
            self.user = mysql_user
            self.pass_wd = mysql_passwd
            self.sql_param.user = self.user
            self.sql_param.passwd = self.pass_wd
