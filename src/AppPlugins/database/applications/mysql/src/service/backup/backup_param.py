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
import os.path

from common.cleaner import clear
from common.common import get_host_sn
from common.const import RepositoryDataTypeEnum, BackupTypeEnum, IPConstant
from common.parse_parafile import get_env_variable
from mysql import log
from mysql.src.common.constant import MySQLType, MySQLClusterType, SystemConstant, MySQLParamType, RoleType
from mysql.src.common.parse_parafile import ParseParaFile
from mysql.src.protect_mysql_base_utils import MysqlBaseUtils
from mysql.src.utils.common_func import get_value, SQLParam


class BackupParam:
    def __init__(self, pid):
        self.pid = pid
        try:
            json_param = ParseParaFile.parse_para_file(self.pid)
            self.param = json_param
            self.json_param = json_param.get("job", {})
            self.backup_type = self.json_param.get("jobParam", {}).get("backupType")
            self.protect_object = self.json_param.get("protectObject", {})
            self.cluster_type = self.protect_object.get("extendInfo", {}).get("clusterType")
            self.app_type = self.protect_object.get("subType")
            self.job_type = self.json_param.get("application", {}).get("extendInfo", {}).get("jobType")
            if self.app_type == MySQLParamType.CLUSTER:
                self.set_cluster_sql_param()
            else:
                self.set_single_sql_param()
            self.cache_path = self.get_mount_path(RepositoryDataTypeEnum.CACHE_REPOSITORY.value)
            self.log_path = self.get_mount_path(RepositoryDataTypeEnum.LOG_REPOSITORY.value)
            self.data_path = self.get_mount_path(RepositoryDataTypeEnum.DATA_REPOSITORY.value)
            self.meta_path = self.get_mount_path(RepositoryDataTypeEnum.META_REPOSITORY.value)
        except Exception as err:
            log.error(f"error:{err}")
            raise Exception(f"Failed to parse job param file for {err}") from err

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        clear(self.pass_wd)

    @property
    def is_full_backup(self):
        return self.backup_type == BackupTypeEnum.FULL_BACKUP.value

    @property
    def is_incr_backup(self):
        return self.backup_type in [BackupTypeEnum.DIFF_BACKUP.value, BackupTypeEnum.INCRE_BACKUP.value]

    @property
    def is_log_backup(self):
        return self.backup_type == BackupTypeEnum.LOG_BACKUP.value

    @property
    def is_active_node(self):
        return self.role_type == RoleType.ACTIVE_NODE

    @property
    def force_optimize(self):
        job_extend = self.json_param.get("extendInfo", {})
        force_opt = get_value(job_extend, "force_optimize", default_value="false")
        return force_opt == "true"

    def get_mount_path(self, repo_type):
        repositories_json = self.json_param.get("repositories", [])
        for repository in repositories_json:
            repository_type = repository.get("repositoryType", "")
            if str(repository_type) == str(repo_type):
                path = repository.get("path", [""])[0]
                return path
        return ""

    def set_cluster_sql_param(self):
        current_ips = MysqlBaseUtils.get_local_ips()
        cluster_nodes = self.json_param.get("protectEnv", {}).get("nodes", [])
        for index, node in enumerate(cluster_nodes):
            if node.get("endpoint") not in current_ips:
                continue
            node_extend = node.get("extendInfo")
            auth_extend = node.get("auth", {}).get("extendInfo", {})
            self.mysql_ip = get_value(node_extend, "instanceIp", IPConstant.LOCAL_HOST)
            self.port = get_value(auth_extend, "instancePort", SystemConstant.DEFAULT_PORT)
            self.charset = get_value(node_extend, "charset", SystemConstant.DEFAULT_CHARSET)
            self.my_cnf_path = node_extend.get("myCnfPath", "")
            self.user = get_env_variable(f"job_protectEnv_nodes_{index}_auth_authKey_{self.pid}")
            self.pass_wd = get_env_variable(f"job_protectEnv_nodes_{index}_auth_authPwd_{self.pid}")
            self.sql_param = SQLParam(self.mysql_ip, self.port, self.user, self.pass_wd, charset=self.charset)
            self.role_type = get_value(node_extend, "role", "1")
            break

    def set_single_sql_param(self):
        protect_object_extend = self.json_param.get("protectObject", {}).get("extendInfo", {})
        self.mysql_ip = get_value(protect_object_extend, "instanceIp", IPConstant.LOCAL_HOST)
        self.port = get_value(protect_object_extend, "instancePort", SystemConstant.DEFAULT_PORT)
        self.charset = get_value(protect_object_extend, "charset", SystemConstant.DEFAULT_CHARSET)
        self.my_cnf_path = protect_object_extend.get("myCnfPath", "")
        self.user = get_env_variable(f"job_protectObject_auth_authKey_{self.pid}")
        self.pass_wd = get_env_variable(f"job_protectObject_auth_authPwd_{self.pid}")
        self.sql_param = SQLParam(self.mysql_ip, self.port, self.user, self.pass_wd, charset=self.charset)
        self.role_type = "1"

    def get_backup_job_result(self):
        return self.json_param.get("backupJobResult", 0)

    def get_sub_job_name(self):
        return self.param.get("subJob", {}).get("jobName")

    def check_param_can_incr(self):
        if self.app_type != MySQLParamType.CLUSTER:
            return True
        if self.cluster_type == MySQLClusterType.EAPP:
            return True
        sub_job_name = self.get_sub_job_name()
        if not sub_job_name or sub_job_name != "backup":
            return True
        return False

    def get_nodes(self):
        return self.json_param.get("appEnv", {}).get("nodes", [])

    def get_current_ip(self):
        nodes = self.json_param.get("protectEnv", {}).get("nodes", [])
        current_ips = MysqlBaseUtils.get_local_ips()
        for node in nodes:
            if node.get("endpoint") in current_ips:
                return node.get("endpoint")
        return ""

    def get_version(self):
        return self.protect_object.get("extendInfo", {}).get("version")

    def get_channel_number(self):
        try:
            backup_task_sla = json.loads(self.json_param.get("extendInfo", {}).get("backupTask_sla", {}))
            policy_list = backup_task_sla.get("policy_list", [])
            return int(policy_list[0].get("ext_parameters", {}).get("channel_number"))
        except Exception as err:
            log.error(f"get_channel_number error,{err}")
            return 1

    def get_backup_database(self):
        if self.app_type == MySQLType.SUBTYPE:
            return self.protect_object.get("name")
        return ""

    def get_copy_path(self, job_id):
        if self.is_log_backup:
            return os.path.join(self.log_path, get_host_sn())
        else:
            return self.get_full_copy_path(job_id)

    def get_full_copy_path(self, job_id):
        return os.path.join(self.data_path, f"mysql_{job_id}_1")

    def get_copy(self):
        return self.json_param.get("copy", [])[0]

    def get_meta_host_sn_file_path(self):
        return os.path.join(self.meta_path, get_host_sn())

    def get_last_bin_log_file(self):
        return os.path.join(self.cache_path, get_host_sn() + '_bin_log')
