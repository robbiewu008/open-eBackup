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

import multiprocessing
import os
import threading
import uuid

import grp
import pwd
import pathlib
import json
import stat
import shutil
import time
import socket
import psutil

from common.file_common import copy_user_file_to_dest_path
from common.logger import Logger
from common.const import ExecuteResultEnum, SubJobStatusEnum, BackupTypeEnum, DeployType, RepositoryDataTypeEnum, \
    DBLogLevel, ReportDBLabel, CopyDataTypeEnum, ParamConstant, RoleType, SubJobPolicyEnum, SubJobPriorityEnum, \
    SubJobTypeEnum, RepositoryNameEnum
from common.common import output_result_file, execute_cmd, convert_time_to_timestamp, exter_attack, \
    write_content_to_file, read_tmp_json_file, output_execution_result_ex, clean_dir_not_walk_link
from common.number_const import NumberConst
from common.parse_parafile import get_env_variable
from common.util import check_utils, check_user_utils
from common.util.backup import backup, backup_files, query_progress
from common.common_models import ActionResult, SubJobDetails, LogDetail, SubJobModel, RepositoryPath, ScanRepositories
from common.util.cmd_utils import cmd_format
from common.util.exec_utils import check_path_valid, exec_cp_cmd, exec_mkdir_cmd
from common.util.kmc_utils import Kmc
from common.util.scanner_utils import scan_dir_size
from postgresql.common.const import CmdRetCode, ErrorCode, PgConst, BackupStatus, DirAndFileNameConst, \
    InstallDeployType, BackupSubJob, PgsqlBackupStatus, ReportPgsqlLabel
from postgresql.common.pg_exec_sql import ExecPgSql
from postgresql.common.models import BackupJobPermission, BackupProgressInfo, NodeInfo
from postgresql.common.util.get_version_util import get_version
from postgresql.common.util.pg_common_utils import PostgreCommonUtils
from postgresql.common.error_code import ErrorCode as ErrCode
from postgresql.resource.cluster_node_checker import ClusterNodesChecker

LOGGER = Logger().get_logger("postgresql.log")


class PgBackup(object):
    def __init__(self, pid, job_id, sub_job_id, param_dict):
        self._pid = pid
        self._job_id = job_id
        self._sub_job_id = sub_job_id
        self._param_dict = param_dict
        self._job_dict = param_dict.get("job", {})
        self._extend_info = self._job_dict.get("protectObject", {}).get("extendInfo", {})
        self._os_user_name = self._job_dict.get("protectObject", {}).get("extendInfo", {}).get("osUsername", "")
        self._instance_id = self._job_dict.get("protectObject", {}).get("id", "")
        self._sub_job_name = ""
        self._job_status = SubJobStatusEnum.RUNNING
        self._backup_status = BackupStatus.RUNNING
        self._err_code = 0
        self._query_progress_interval = 15
        self._logdetail = None
        self._loop_time = 0
        self._port = self._extend_info.get("instancePort", PgConst.DB_DEFAULT_PORT)
        deploy_type = self._job_dict.get("protectEnv", {}).get("extendInfo", {}).get("deployType", 0)
        if int(deploy_type) == DeployType.SINGLE_TYPE.value:
            self._client_path = self._job_dict.get("protectObject", {}).get("extendInfo", {}).get("clientPath", "")
            self._data_path = self._job_dict.get("protectObject", {}).get("extendInfo", {}).get("dataDirectory", "")
            self._archive_dir = self._job_dict.get("protectObject", {}).get("extendInfo", {}).get("archiveDir", "")
            self._service_ip = self._job_dict.get("protectObject", {}).get("extendInfo", {}).get("serviceIp", "")
        elif int(deploy_type) == DeployType.CLUSTER_TYPE.value:
            host_ips = PostgreCommonUtils.get_local_ips()
            nodes = self._job_dict.get("protectSubObject", [{}])
            for node in nodes:
                node_extend_info = node.get("extendInfo", {})
                service_ip = node_extend_info.get('serviceIp', "")
                if service_ip in host_ips:
                    self._client_path = node_extend_info.get('clientPath', "")
                    self._data_path = node_extend_info.get('dataDirectory', "")
                    self._archive_dir = node_extend_info.get("archiveDir", "")
                    self._port = node_extend_info.get("instancePort", PgConst.DB_DEFAULT_PORT)
                    break
            self._service_ip = self._job_dict.get("protectEnv", {}).get("endpoint", "")
        repositories_info = self.parse_repositories_path(self._job_dict.get("repositories", []))
        self._data_area = repositories_info.get("data_repository", [""])[0]
        self._log_area = repositories_info.get("log_repository", [""])[0]
        self._meta_area = repositories_info.get("meta_repository", [""])[0]
        self._cache_area = repositories_info.get("cache_repository", [""])[0]
        self._output_code = ActionResult(code=ExecuteResultEnum.SUCCESS.value)
        self._calc_progress_thread_flag = True
        self.install_deploy_type = (self._job_dict.get("protectEnv", {}).get("extendInfo", {}).
                                    get("installDeployType", InstallDeployType.PGPOOL))
        if self.install_deploy_type == InstallDeployType.PGPOOL:
            self._port = self._extend_info.get("instancePort", PgConst.DB_DEFAULT_PORT)
        self.enable_root = PostgreCommonUtils.get_root_switch()
        self._backup_type = self._job_dict.get("jobParam", {}).get("backupType", "")
        self._last_stop_wal = None
        self._thread_number = int(self._job_dict.get("extendInfo", {}).get("thread_number", 1))

    @staticmethod
    def parse_repositories_path(repositories: list):
        backup_dirs = dict()
        for repository in repositories:
            repository_type = repository.get("repositoryType")
            if repository_type == RepositoryDataTypeEnum.DATA_REPOSITORY:
                backup_dirs["data_repository"] = repository.get("path", [""])
            elif repository_type == RepositoryDataTypeEnum.CACHE_REPOSITORY:
                backup_dirs["cache_repository"] = repository.get("path", [""])
            elif repository_type == RepositoryDataTypeEnum.LOG_REPOSITORY:
                backup_dirs["log_repository"] = repository.get("path", [""])
            elif repository_type == RepositoryDataTypeEnum.META_REPOSITORY \
                    and "meta_repository" not in backup_dirs:
                backup_dirs["meta_repository"] = repository.get("path", [""])
        return backup_dirs

    @staticmethod
    def get_host_ip(service_ip, port):
        s = None
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            s.connect((service_ip, int(port)))
            ip = s.getsockname()[0]
        finally:
            if s:
                s.close()
        return ip

    @staticmethod
    def clear_repository_dir(dir_path):
        """
        清空数据仓目录
        """
        if os.path.islink(dir_path):
            return
        ret, realpath = PostgreCommonUtils.check_path_in_white_list(dir_path)
        if not ret:
            return
        for path in os.listdir(realpath):
            new_path = os.path.join(realpath, path)
            if '.snapshot' in new_path:
                continue
            if os.path.isfile(new_path) or os.path.islink(new_path):
                os.remove(new_path)
            elif os.path.isdir(new_path):
                shutil.rmtree(new_path, ignore_errors=True)

    @staticmethod
    def path_check(path):
        return os.path.exists(path) and os.path.islink(path)

    @staticmethod
    def get_last_backup_stop_time(backup_file):
        with open(backup_file, "r") as file:
            lines = file.readlines()
        stop_time = None
        for info in lines:
            if "STOP TIME:" in info:
                stop_time = f"{info.split()[2]} {info.split()[3]}"
        return stop_time

    @staticmethod
    def exec_rc_tool_cmd(cmd, in_path, out_path):
        cmd = f"{os.path.join(ParamConstant.BIN_PATH, 'rpctool.sh')} {cmd} {in_path} {out_path}"
        ret, out, err = execute_cmd(cmd)
        if ret != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error(f"An error occur in execute cmd. ret:{ret} err:{err}")
            return False
        return True

    @staticmethod
    def set_start(expect_start_wal, file_list):
        start = file_list.index(expect_start_wal) + 1
        if file_list[start].endswith("backup"):
            start = start + 1
        return start

    @staticmethod
    def get_port(nodes, service_ip):
        for node in nodes:
            node_extend_info = node.get("extendInfo", {})
            if service_ip == node_extend_info.get('serviceIp', ""):
                port = node_extend_info.get('instancePort', "")
                break
        return port

    @staticmethod
    def get_ip_and_port(cluster_nodes, nodes):
        for cluster_node in cluster_nodes:
            if cluster_node.get('role', "") == str(RoleType.PRIMARY.value):
                service_ip = cluster_node.get('hostname', "")
                port = PgBackup.get_port(nodes, service_ip)
                break
        return port, service_ip

    def set_action_result(self, code, body_err, message):
        self._output_code.code = code
        self._output_code.body_err = body_err
        self._output_code.message = message

    def get_action_result(self):
        return self._output_code

    def check_params(self):
        for check in (self._data_path, self._client_path):
            if not PostgreCommonUtils.check_black_list(check):
                LOGGER.error("The path is invalid!")
                return False
        if not check_utils.is_ip_address(self._service_ip):
            LOGGER.error(f"The ip: {self._service_ip} is invalid!")
            return False
        if not check_utils.is_port(self._port):
            LOGGER.error(f"The port: {self._port} is invalid!")
            return False
        if not PostgreCommonUtils.check_os_user(self._os_user_name, self._client_path, self.enable_root)[0]:
            return False
        return True

    def check_database_status(self):
        pg_ctl_path = os.path.join(self._client_path, "bin", "pg_ctl")
        if not os.path.exists(pg_ctl_path):
            LOGGER.error(f"pg_ctl_path is not exist!")
            return False
        if not PostgreCommonUtils.check_os_user(self._os_user_name, pg_ctl_path, self.enable_root)[0]:
            LOGGER.error("Check database status failed!Because os username is not exist!")
            return False
        if not self.enable_root:
            if not check_user_utils.check_path_owner(self._data_path, [self._os_user_name]):
                LOGGER.error("Check data path failed!Because data path owner is not valid!")
                return False
        if not check_path_valid(self._data_path, False, False):
            LOGGER.error(f"data_path[{self._data_path}] is invalid")
            return False

        cmd = cmd_format("su - {} -c '{} status -D {}'", self._os_user_name, pg_ctl_path, self._data_path)
        return_code, std_out, std_err = execute_cmd(cmd)
        if return_code == CmdRetCode.NO_SUCH_PROCESS.value:
            LOGGER.info(f"Database server is not running, job id: {self._job_id}.")
            return False
        elif return_code == CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.info(f"Database server is running, job id: {self._job_id}.")
            return True
        LOGGER.error(f"Failed to exec cmd: {cmd}, job id: {self._job_id}.")
        return False

    def query_system_param(self, param_name, pg_sql=None):
        sql_cmd = f"show {param_name};"
        if not pg_sql:
            pg_sql = self.get_pg_sql()
        return_code, std_out, st_err = pg_sql.exec_sql_cmd(self._os_user_name, sql_cmd)
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error(f"Failed to exec cmd: {sql_cmd}, job_id: {self._job_id}.")
            return False, ErrorCode.PLUGIN_CANNOT_BACKUP_ERR.value
        if param_name not in std_out:
            return False, ErrorCode.PLUGIN_CANNOT_BACKUP_ERR.value
        value = pg_sql.parse_sql_result(std_out, param_name)
        LOGGER.info(f"{param_name} is {value}, job_id: {self._job_id}.")
        return True, value

    def query_archive_mode(self):
        result, archive_mode = self.query_system_param("archive_mode")
        if not result:
            return result, archive_mode
        LOGGER.info(f"Archive mode is {archive_mode}, job_id: {self._job_id}.")
        return archive_mode == "on", ErrorCode.ARCHIVE_MODE_ENABLED.value

    def check_node_is_primary_patroni(self):
        host_ips = PostgreCommonUtils.get_local_ips()
        nodes = self._job_dict.get("protectSubObject", [{}])
        patroni_config, _, _ = PostgreCommonUtils.get_patroni_config(host_ips, nodes)
        # 先通过ip找到对应的对应的patroni_config文件
        cluster_nodes = ClusterNodesChecker.get_nodes(patroni_config)
        for cluster_node in cluster_nodes:
            host = cluster_node.get('hostname', "")
            if host in host_ips:
                if cluster_node.get('role', "") == str(RoleType.PRIMARY.value):
                    return True
                else:
                    return False
        return False

    def check_all_patroni_node_are_online(self):
        """
        针对Patroni集群，为了实现备份时Patroni集群触发切主操作仍可以备份成功，执行AllowBackupInLocalNode时不去寻找主节点，
        只判断是否所有patroni节点都在线，而在后续备份时通过查询数据库动态更新节点情况完成备份
        """
        LOGGER.info(
            f'step 1: execute check_all_patroni_node_are_online, job_id: {self._job_id}')
        agent_infos = self._job_dict.get('extendInfo', {}).get('agents', [])
        agent_uuids = set()
        for agent_info in agent_infos:
            agent_uuids.add(agent_info['id'])
        LOGGER.info(f"patroni cluster agent_uuids {agent_uuids}")
        nodes = self._job_dict.get("protectSubObject", [])
        LOGGER.info(f"patroni cluster instance protectSubObject nodes {nodes}")
        for node in nodes:
            node_id = node.get("extendInfo", {}).get('hostId', "")
            LOGGER.info(f"node_id {node_id}")
            if node_id not in agent_uuids:
                log_detail = LogDetail(logInfo="plugin_generate_subjob_fail_label", logLevel=DBLogLevel.ERROR)
                PostgreCommonUtils.report_job_details(self._pid, SubJobDetails(taskId=self._job_id, progress=100,
                                                      logDetail=[log_detail],
                                                      taskStatus=SubJobStatusEnum.FAILED.value).dict(by_alias=True))
                LOGGER.error(f"patroni cluster instance {node_id} is offline")
                return False
        LOGGER.info(f"step 1: execute check_all_patroni_node_are_online success, job_id: {self._job_id}")
        return True

    def check_node_is_primary_clup(self):
        local_service_ip = ''
        host_ips = PostgreCommonUtils.get_local_ips()
        nodes = self._job_dict.get("protectSubObject", [])
        for node in nodes:
            node_extend_info = node.get("extendInfo", {})
            service_ip = node_extend_info.get('serviceIp', "")
            if service_ip in host_ips:
                local_service_ip = service_ip
                break
        if local_service_ip == '':
            LOGGER.error(f"Unable to find the IP of the local database host, job_id: {self._job_id}.")
            return False
        sql_cmd = "select pg_is_in_recovery();"
        pg_sql = ExecPgSql(self._pid, self._client_path, local_service_ip, self._port)
        return_code, std_out, st_err = pg_sql.exec_sql_cmd(self._os_user_name, sql_cmd)
        if not return_code:
            LOGGER.error(
                f"Failed to exec cmd: {sql_cmd}, job_id: {self._job_id}, return_code: {return_code}, st_err: {st_err}.")
            return False
        recovery_state = pg_sql.parse_html_result(std_out)
        if not recovery_state:
            LOGGER.error(f"The command execution did not yield any results, cmd: {sql_cmd}, job_id: {self._job_id}.")
            return False
        if recovery_state[0].get('pg_is_in_recovery') == 't':
            LOGGER.error(f"The node is standy and status is normal, job_id: {self._job_id}.")
            return False
        LOGGER.info(f"The node is primary and status is normal, job_id: {self._job_id}.")
        return True

    def check_node_is_primary(self):
        sql_cmd = "show pool_nodes;"
        pg_sql = self.get_pg_sql()
        return_code, std_out, err_info = pg_sql.exec_sql_cmd(self._os_user_name, sql_cmd)
        if not return_code:
            LOGGER.error(f"Failed to exec cmd: {sql_cmd}, job_id: {self._job_id}.")
            return False
        nodes_list = pg_sql.get_pg_cluster_info(std_out)
        host_ips = PostgreCommonUtils.get_local_ips()
        node_dict = [node for node in nodes_list if node.get("hostname") in host_ips]
        if not node_dict or node_dict[0].get("role") in ["standby", "slave"]:
            LOGGER.error(f"The pgpool node is standby, job_id: {self._job_id}.")
            return False
        LOGGER.info(f"The node is primary and status is normal, job_id: {self._job_id}.")
        return True

    @exter_attack
    def allow_backup_in_local_node(self):
        if not self.check_params():
            LOGGER.error(f"Check the path, IP address, and port!")
            return False
        # 查询是否开启数据库
        if not self.check_database_status():
            body_err = ErrorCode.DATABASE_OFFLINE_ERR.value
            message = "Database system is shut down."
            self.set_action_result(ExecuteResultEnum.INTERNAL_ERROR.value, body_err, message)
            LOGGER.error(f"Database system is shut down, job_id: {self._job_id}.")
            return False

        # 查询是否主节点
        deploy_type = self._job_dict.get("protectEnv", {}).get("extendInfo", {}).get("deployType", 0)
        if int(deploy_type) == DeployType.CLUSTER_TYPE.value:
            install_deploy_type = (self._job_dict.get("protectEnv", {}).get("extendInfo", {}).
                                   get("installDeployType", InstallDeployType.PGPOOL))
            if install_deploy_type == InstallDeployType.PATRONI and not self.check_all_patroni_node_are_online():
                body_err = ErrorCode.PLUGIN_CANNOT_BACKUP_ERR.value
                message = "Current patroni cluster has offline nodes."
                self.set_action_result(ExecuteResultEnum.INTERNAL_ERROR.value, body_err, message)
                LOGGER.error(f"Failed to check patroni cluster status, job_id: {self._job_id}.")
                return False
            elif install_deploy_type == InstallDeployType.PGPOOL and not self.check_node_is_primary():
                body_err = ErrorCode.PLUGIN_CANNOT_BACKUP_ERR.value
                message = "Cluster node is standby."
                self.set_action_result(ExecuteResultEnum.INTERNAL_ERROR.value, body_err, message)
                LOGGER.error(f"Failed to check cluster status, job_id: {self._job_id}.")
                return False
            elif install_deploy_type == InstallDeployType.CLUP and not self.check_node_is_primary_clup():
                body_err = ErrorCode.PLUGIN_CANNOT_BACKUP_ERR.value
                message = "Cluster node is standby."
                self.set_action_result(ExecuteResultEnum.INTERNAL_ERROR.value, body_err, message)
                LOGGER.error(f"Failed to check cluster status, job_id: {self._job_id}.")
                return False
        LOGGER.info(f"The node can backup, job_id: {self._job_id}.")
        return True

    def query_data_directory_mode(self):
        sql_cmd = "show data_directory_mode;"
        pg_sql = self.get_pg_sql()
        return_code, std_out, std_err = pg_sql.exec_sql_cmd(self._os_user_name, sql_cmd)
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error(f"Failed to query directory mode, err: {std_err}, job id: {self._job_id}.")
            return ""
        file_mode = pg_sql.parse_sql_result(std_out, "data_directory_mode")
        if not file_mode:
            LOGGER.error(f"Failed to parse app permission, job id: {self._job_id}.")
            return ""
        return file_mode

    @exter_attack
    def query_job_permission(self):
        os_user_name = self._param_dict.get("application", {}).get("extendInfo", {}).get("osUsername", "")
        if not os_user_name:
            LOGGER.error(f"Failed to get os username:{os_user_name}, job id: {self._job_id}.")
            return False
        group_id = pwd.getpwnam(os_user_name).pw_gid
        user_group = grp.getgrgid(group_id).gr_name
        # pgsql的data目录权限必须是700，否则会报错
        output = BackupJobPermission(user=os_user_name, group=user_group, fileMode="0700")
        output_result_file(self._pid, output.dict(by_alias=True))
        return True

    def query_archive_dir(self, pg_sql=None):
        try:
            sql_cmd = "show archive_command;"
            if not pg_sql:
                pg_sql = self.get_pg_sql()
            return_code, std_out, std_err = pg_sql.exec_sql_cmd(self._os_user_name, sql_cmd)
            if return_code != CmdRetCode.EXEC_SUCCESS.value:
                LOGGER.error(f"Failed to exec cmd: {sql_cmd}, job id: {self._job_id}.")
                return self._archive_dir
            archive_info = pg_sql.parse_sql_result(std_out, "archive_command")
            archive_dir = archive_info.split()[archive_info.split().index("cp") + 2]
            return archive_dir.strip('"%f')
        except Exception as err:
            LOGGER.error(f"show archive_command err: {err}.")
            return self._archive_dir

    @exter_attack
    def check_backup_job_type(self):
        LOGGER.info(f"Begin to check backup job type, self.job_id: {self._job_id}")
        if self._backup_type == BackupTypeEnum.FULL_BACKUP.value:
            return True
        deploy_type = self._job_dict.get("protectEnv", {}).get("extendInfo", {}).get("deployType", 0)
        if int(deploy_type) == DeployType.SINGLE_TYPE.value:
            return True
        ret, pre_log_or_full_copy = self.get_last_copy_info(
            [CopyDataTypeEnum.LOG_COPY.value, CopyDataTypeEnum.FULL_COPY.value])
        if ret and pre_log_or_full_copy:
            pre_copy_backup_node = pre_log_or_full_copy.get("extendInfo", {}).get("backup_node")
            LOGGER.info(f"pre log or full copy backup node: {pre_copy_backup_node}")
            if self.install_deploy_type == InstallDeployType.PATRONI:
                # 如果是patroni要去找主节点的ip和端口，因为是动态变化的，所以得实时去查，不能根据PM传的入参
                host_ips = PostgreCommonUtils.get_local_ips()
                nodes = self._job_dict.get("protectSubObject", [{}])
                patroni_config, _, _ = PostgreCommonUtils.get_patroni_config(host_ips, nodes)
                # 先通过ip找到对应的对应的patroni_config文件
                cluster_nodes = ClusterNodesChecker.get_nodes(patroni_config)
                port, service_ip = PgBackup.get_ip_and_port(cluster_nodes, nodes)
                if service_ip == pre_copy_backup_node:
                    return True
            else:
                host_ips = PostgreCommonUtils.get_local_ips()
                if pre_copy_backup_node in host_ips:
                    return True

        body_err = ErrorCode.ERR_LOG_TO_FULL.value
        message = "After the primary-secondary switch in a cluster, the first backup needs to be a full backup."
        self.set_action_result(ExecuteResultEnum.INTERNAL_ERROR.value, body_err, message)
        return False

    @exter_attack
    def backup_prerequisite(self):
        if not self._os_user_name:
            LOGGER.error(f"Os user name is invalid, job id: {self._job_id}.")
            return False
        backup_type = self._job_dict.get("jobParam", {}).get("backupType", 0)
        dir_path = self._data_area if backup_type == BackupTypeEnum.FULL_BACKUP.value else self._log_area
        if not os.path.exists(dir_path) and not check_utils.check_path_in_white_list(dir_path):
            LOGGER.error(f"The path[{dir_path}] is incorrect, job id: {self._job_id}.")
            return False
        # get database user id
        user_id = pwd.getpwnam(self._os_user_name).pw_uid
        stat_info = os.stat(dir_path)
        if stat_info.st_uid != user_id:
            LOGGER.error(f"Dir permission is incorrect, job id: {self._job_id}.")
            return False
        if self.install_deploy_type == InstallDeployType.PATRONI:
            # 如果是patroni要去找主节点的ip和端口，因为是动态变化的，所以得实时去查，不能根据PM传的入参
            host_ips = PostgreCommonUtils.get_local_ips()
            nodes = self._job_dict.get("protectSubObject", [{}])
            patroni_config, _, _ = PostgreCommonUtils.get_patroni_config(host_ips, nodes)
            # 先通过ip找到对应的对应的patroni_config文件
            cluster_nodes = ClusterNodesChecker.get_nodes(patroni_config)
            port, service_ip = PgBackup.get_ip_and_port(cluster_nodes, nodes)
            pg_sql = ExecPgSql(self._pid, self._client_path, service_ip, port)
            # 查询是否开启归档模式
            result, archive_mode = self.query_system_param("archive_mode", pg_sql=pg_sql)
            if not result or archive_mode != "on":
                log_detail = LogDetail(logInfo=ReportPgsqlLabel.PREREQUISITE_CHECK_FAILED,
                                       logDetail=ErrorCode.ARCHIVE_MODE_ENABLED.value, logLevel=DBLogLevel.ERROR)
                sub_dict = SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, progress=100,
                                         logDetail=[log_detail], taskStatus=SubJobStatusEnum.FAILED.value)
                PostgreCommonUtils.report_job_details(self._pid, sub_dict.dict(by_alias=True))
                LOGGER.error(f"Archive mode is {archive_mode}, job_id: {self._job_id}.")
                return False
            archive_dir = self.query_archive_dir(pg_sql=pg_sql)
            if not archive_dir or not os.path.isdir(archive_dir):
                log_detail = LogDetail(logInfo=ReportPgsqlLabel.PREREQUISITE_CHECK_FAILED,
                                       logDetail=ErrCode.ARCHIVE_MODE_CONFIG_ERROR, logLevel=DBLogLevel.ERROR)
                sub_dict = SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, progress=100,
                                         logDetail=[log_detail], taskStatus=SubJobStatusEnum.FAILED.value)
                PostgreCommonUtils.report_job_details(self._pid, sub_dict.dict(by_alias=True))
                LOGGER.error(f"Archive command is useless, archive_dir: {archive_dir}, job_id: {self._job_id}.")
                return False
            LOGGER.info(f'step 2: execute backup_pre_job_patroni, job_id:{self._job_id}')
            self.backup_pre_job_patroni()
        LOGGER.info(f'Succeed to execute backup_prerequisite, job_id:{self._job_id}')
        log_detail = LogDetail(logInfo="plugin_execute_prerequisit_task_success_label", logLevel=DBLogLevel.INFO)
        sub_dict = SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, progress=100, logDetail=[log_detail],
                                 taskStatus=SubJobStatusEnum.COMPLETED.value)
        PostgreCommonUtils.report_job_details(self._pid, sub_dict.dict(by_alias=True))
        return True

    def backup_pre_job_patroni(self):
        # 共享文档逻辑如下：
        # 1、node_info文件创建处，以job id唯一标记
        # 2、建立一个线程在备份，循环请求node info，进行更新，循环时间15S
        # 3、主从节点切换选择新节点：读取文件后进行选择，并更新共享文档
        # 4、主从节点切换或备份发生错误时，更新共享文档
        LOGGER.info("step 2: start to backup_pre_job_patroni")
        nodes_info_file = os.path.join(self._meta_area, "nodesInfo", f"job_id_{self._job_id}")
        nodes_info_path = os.path.join(self._meta_area, "nodesInfo")
        if not os.path.exists(nodes_info_path):
            try:
                os.makedirs(nodes_info_path)
            except Exception as err:
                LOGGER.error(f"Make dir for {nodes_info_path} err: {err}.")

        nodes = PostgreCommonUtils.get_online_data_nodes(self._job_dict.get("protectSubObject", []))
        nodes_info = {}
        for node in nodes:
            node_extend_info = node.get("extendInfo", {})
            node_host = node_extend_info.get('serviceIp', "")
            host_id = node_extend_info.get('hostId', "")
            node_info = NodeInfo(nodeHost=node_host,
                                 setId=self._instance_id,
                                 agentUuid=host_id,
                                 )
            nodes_info[node_host] = node_info.dict(by_alias=True)
        LOGGER.info(f"step 2: Obtained information of all online Patroni cluster nodes: {nodes_info}")
        PostgreCommonUtils.init_node_data(nodes_info_file, nodes_info, 100)
        LOGGER.info("step 2: Successfully initialized the patroni cluster node information database.")
        LOGGER.info(f"step 2: finish to execute backup_pre_job_patroni, job_id:{self._job_id}")
        return

    @exter_attack
    def backup_gen_sub_job(self):
        """
        目前Patroni集群备份流程与其余两种集群存在差异，Patroni集群需要额外生成子任务
        """
        LOGGER.info(f"start to execute backup_gen_sub_job, job_id: {self._job_id}")
        deploy_type = self._job_dict.get("protectEnv", {}).get("extendInfo", {}).get("deployType", 0)
        if int(deploy_type) == DeployType.CLUSTER_TYPE.value:
            install_deploy_type = (self._job_dict.get("protectEnv", {}).get("extendInfo", {}).
                                   get("installDeployType", InstallDeployType.PGPOOL))
            if install_deploy_type == InstallDeployType.PATRONI:
                self.backup_gen_sub_job_patroni()
            else:
                self.backup_gen_sub_job_other()
        LOGGER.info(f"finish to execute backup_gen_sub_job, job_id:{self._job_id}")
        return True

    def backup_gen_sub_job_patroni(self):
        LOGGER.info(f"step 3: start to gen_sub_job for patroni cluster data backup, job_id:{self._job_id}")
        nodes = PostgreCommonUtils.get_online_data_nodes(self._job_dict.get("protectSubObject", []))
        host_ips = PostgreCommonUtils.get_local_ips()
        file_path = os.path.join(ParamConstant.RESULT_PATH, f"result{self._pid}")
        sub_job_array = []

        # 子任务：BACKUP
        # 在每个节点执行，具体执行权限在执行子任务中实现判断
        LOGGER.info("start to gen backup_sub_job: backup")
        job_policy = SubJobPolicyEnum.FIXED_NODE.value
        job_name = "backup"
        job_priority = SubJobPriorityEnum.JOB_PRIORITY_1
        for node in nodes:
            node_extend_info = node.get("extendInfo", {})
            host = node_extend_info.get('serviceIp', "")
            node_id = node_extend_info.get('hostId', "")
            job_info = f"{host}"
            sub_job = SubJobModel(jobId=self._job_id, jobType=SubJobTypeEnum.BUSINESS_SUB_JOB.value,
                                  execNodeId=node_id, jobPriority=job_priority, jobName=job_name,
                                  policy=job_policy, jobInfo=job_info, ignoreFailed=False).dict(by_alias=True)
            sub_job_array.append(sub_job)

        # 子任务：QUERYCOPY
        # 在任意节点均可执行
        LOGGER.info("start to gen backup_sub_job: queryCopy")
        job_policy = SubJobPolicyEnum.ANY_NODE.value
        job_name = "queryCopy"
        job_priority = SubJobPriorityEnum.JOB_PRIORITY_2
        for node in nodes:
            node_extend_info = node.get("extendInfo", {})
            host = node_extend_info.get('serviceIp', "")
            if host in host_ips:
                node_id = node_extend_info.get('hostId', "")
                job_info = f"{host}"
                sub_job = SubJobModel(jobId=self._job_id, jobType=SubJobTypeEnum.BUSINESS_SUB_JOB.value,
                                      execNodeId=node_id, jobPriority=job_priority, jobName=job_name,
                                      policy=job_policy, jobInfo=job_info, ignoreFailed=False).dict(by_alias=True)
                sub_job_array.append(sub_job)
                break

        LOGGER.info(f"succeed to gen_sub_job for patroni cluster data backup, get sub_job_array: {sub_job_array}")
        LOGGER.info(f"step 3: Sub-task splitting succeeded.sub-task num:{len(sub_job_array)}")
        output_execution_result_ex(file_path, sub_job_array)
        LOGGER.info(f"step 3: end to gen_sub_job for patroni cluster data backup, job_id:{self._job_id}")

    def backup_gen_sub_job_other(self):
        LOGGER.info(f"start to gen_sub_job for other cluster data backup, job_id:{self._job_id}")
        nodes = self._job_dict.get("protectSubObject", [])
        file_path = os.path.join(ParamConstant.RESULT_PATH, f"result{self._pid}")
        host_ips = PostgreCommonUtils.get_local_ips()
        sub_job_array = []
        for node in nodes:
            node_extend_info = node.get("extendInfo", {})
            host = node_extend_info.get('serviceIp', "")
            if host in host_ips:
                node_id = node_extend_info.get('hostId', "")
                job_info = f"{host}"
                sub_job = SubJobModel(jobId=self._job_id, jobType=SubJobTypeEnum.BUSINESS_SUB_JOB.value,
                                      execNodeId=node_id,
                                      jobPriority=SubJobPriorityEnum.JOB_PRIORITY_1, jobName="backup",
                                      policy=SubJobPolicyEnum.LOCAL_NODE.value,
                                      jobInfo=job_info,
                                      ignoreFailed=False).dict(by_alias=True)
                sub_job_array.append(sub_job)
                sub_job = SubJobModel(jobId=self._job_id, jobType=SubJobTypeEnum.BUSINESS_SUB_JOB.value,
                                      execNodeId=node_id,
                                      jobPriority=SubJobPriorityEnum.JOB_PRIORITY_2, jobName="queryCopy",
                                      policy=SubJobPolicyEnum.ANY_NODE.value,
                                      jobInfo=job_info,
                                      ignoreFailed=False).dict(by_alias=True)
                sub_job_array.append(sub_job)
                break
        LOGGER.info(f"succeed to gen_sub_job for other cluster data backup, get sub_job_array: {sub_job_array}")
        output_execution_result_ex(file_path, sub_job_array)
        LOGGER.info(f"end to gen_sub_job for other cluster data backup, job_id:{self._job_id}")

    def exec_start_backup(self):
        pg_ctl_path = os.path.join(self._client_path, "bin", "pg_ctl")
        if not os.path.exists(pg_ctl_path):
            LOGGER.error(f"pg_ctl_path is not exist!")
            return False, ""
        res, version_info = get_version(self._pid, pg_ctl_path, self._os_user_name, self.enable_root)
        if not res:
            LOGGER.error(f"Failed to get version, job id: {self._job_id}.")
            return False, ""
        if not check_utils.is_valid_uuid(self._job_id):
            LOGGER.error(f"Failed to check job id: {self._job_id}.")
            return False, ""
        # 9.6及以上版本和9.6以下版本pg_start_backup传参不同
        is_ge_15 = int(version_info.split('.')[0]) >= PgConst.DATABASE_V15
        if int(version_info.split('.')[0]) < PgConst.DATABASE_V10 \
                and int(version_info.split('.')[1]) < PgConst.DATABASE_MINOR_VERSION_SIX:
            sql_cmd = f"select pg_start_backup(\'\\\'{self._job_id}\\\'\', false);"
        elif is_ge_15:
            sql_cmd = f"select pg_backup_start('{self._job_id}', false);"
        else:
            sql_cmd = f"select pg_start_backup(\'\\\'{self._job_id}\\\'\', false, true);"
        pg_sql = self.get_pg_sql()
        if is_ge_15:
            return_code, start_file, std_err = pg_sql.exec_backup_cmd_catch_error(self._os_user_name, sql_cmd,
                                                                                  timeout=PgConst.CHECK_POINT_TIME_OUT)
        else:
            return_code, start_file, std_err = pg_sql.exec_sql_cmd(self._os_user_name, sql_cmd,
                                                                   timeout=PgConst.CHECK_POINT_TIME_OUT)
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error(f"Failed to exec start backup cmd, job id: {self._job_id}, error : {std_err}.")
            return False, ""
        try:
            if is_ge_15:
                pg_start_file = pg_sql.parse_sql_result(start_file, "pg_backup_start")
            else:
                pg_start_file = pg_sql.parse_sql_result(start_file, "pg_start_backup")
        except Exception as ex:
            LOGGER.exception(f"Can not get the first bakup file, start_file: {start_file}.")
            raise Exception("The first backup file not exist.") from ex
        LOGGER.info(f"Succeed to exec start backup cmd, job id: {self._job_id}, pg_start_file:{pg_start_file}.")
        return True, pg_start_file

    def get_pg_sql(self):
        if self.install_deploy_type == InstallDeployType.PATRONI:
            # 如果是patroni要去找主节点的ip和端口，因为是动态变化的，所以得实时去查，不能根据PM传的入参
            host_ips = PostgreCommonUtils.get_local_ips()
            nodes = self._job_dict.get("protectSubObject", [{}])
            patroni_config, port, _ = PostgreCommonUtils.get_patroni_config(host_ips, nodes)
            # 先通过ip找到对应的对应的patroni_config文件
            cluster_nodes = ClusterNodesChecker.get_nodes(patroni_config)
            for cluster_node in cluster_nodes:
                host = cluster_node.get('hostname', "")
                if host in host_ips and cluster_node.get('role', "") == str(RoleType.PRIMARY.value):
                    service_ip = cluster_node.get('hostname', "")
                    break
            pg_sql = ExecPgSql(self._pid, self._client_path, service_ip, port)
        else:
            pg_sql = ExecPgSql(self._pid, self._client_path, self._service_ip, self._port)
        return pg_sql

    def backup_files(self, source, target):
        if not source or not target:
            LOGGER.error(f"Param error, job id: {self._job_id}.")
            return False
        # 此处source参数请勿使用os.path.realpath标准化处理
        ret = exec_cp_cmd(source, target, self._os_user_name, "-r", is_check_white_list=False)
        if not ret:
            LOGGER.error(f"Failed to exec cp cmd, job id: {self._job_id}.")
            return False
        return True

    def backup_directory(self, source, target, job_id):
        if not source or not target:
            LOGGER.error(f"Param error, job id: {self._job_id}.")
            return False
        if not backup(job_id, source, target, thread_num=self._thread_number):
            LOGGER.error(f"Failed to start backup, jobId: {self._job_id}.")
            return False
        return self.get_backup_status(job_id=job_id)

    def backup_file_list(self, files, target, number):
        temp_job_id = f"{self._job_id}_{number}"
        LOGGER.info(f"Start backup, temp_job_id: {temp_job_id}.")
        if not files or not target:
            LOGGER.error(f"Param error, temp_job_id: {temp_job_id}.")
            return False
        res = backup_files(temp_job_id, files, target, write_meta=True, thread_num=self._thread_number)
        if not res:
            LOGGER.error(f"Failed to start backup, temp_job_id: {temp_job_id}.")
            return False
        return self.get_backup_status(temp_job_id)

    def get_backup_status(self, job_id):
        backup_status = False
        while True:
            time.sleep(10)
            status, progress, data_size = query_progress(job_id)
            LOGGER.info(f"Get backup progress: status:{status}, progress:{progress}, "
                        f"data_size:{data_size}")
            if status == BackupStatus.COMPLETED:
                LOGGER.info(f"Backup completed, jobId: {self._job_id}.")
                backup_status = True
                break
            elif status == BackupStatus.RUNNING:
                continue
            elif status == BackupStatus.FAILED:
                LOGGER.error(f"Backup failed, jobId: {self._job_id}.")
                backup_status = False
                break
            else:
                LOGGER.error(f"Backup failed, status error jobId: {self._job_id}.")
                backup_status = False
                break
        return backup_status

    def query_log_file_list(self, wal_dir, pg_start_file):
        # 解析副本元数据信息
        if not check_utils.check_path_in_white_list(self._meta_area):
            LOGGER.error(f"The meta area：{self._meta_area} is incorrect.")
            return []
        copy_meta_file = os.path.join(self._meta_area, "LastBackupCopy.info")
        wal_file = self.get_start_file_name(pg_start_file)
        if wal_file:
            wal_file_time_id = wal_file[:8]
            # 前8位是TimeLineID，取值范围是0x00000000 -> 0xFFFFFFFF，备份和当前数据timeline一致的wal
            file_list = list(filter(lambda x: PostgreCommonUtils.is_wal_file(x) and x[:8] == wal_file_time_id,
                                    os.listdir(wal_dir)))
            #  只备份在首次开始时间生成之后的日志，转换为16进制比较时间大小，越大的说明生成时间越靠后
            wal_file_int = int(wal_file, 16)
            files = [file for file in file_list if int(file, 16) <= wal_file_int]
        else:
            files = [file for file in os.listdir(wal_dir) if PostgreCommonUtils.is_wal_file(file)]
            LOGGER.info("The first backup file is not exist, ready to backup all wal files.")
            return files
        if len(files) == 0:
            return files
        if os.path.exists(copy_meta_file):
            copy_meta_info = self.read_tmp_json_file(copy_meta_file)
            last_wal_file = copy_meta_info.get("wal_file")
            if last_wal_file not in files:
                return files
            # 查找备份文件
            last_wal_file_time_id = int(last_wal_file, 16)
            wal_files = [file for file in files if int(file, 16) > last_wal_file_time_id]
            LOGGER.info(f"Filter file list number :{len(wal_files)}")
            return wal_files
        LOGGER.info(f"Filter file list number :{len(files)}")
        return files

    def get_start_file_name(self, pg_start_file):
        pg_ctl_path = os.path.join(self._client_path, "bin", "pg_ctl")
        if not os.path.exists(pg_ctl_path):
            LOGGER.error(f"pg_ctl_path is not exist!")
            return ""
        res, version_info = get_version(self._pid, pg_ctl_path, self._os_user_name, self.enable_root)
        if not res:
            LOGGER.error(f"Failed to get version, job id: {self._job_id}.")
            return ""
        if int(version_info.split('.')[0]) < PgConst.DATABASE_V10:
            key = "pg_xlogfile_name"
            sql_cmd = f"select pg_xlogfile_name(\'\\\'{pg_start_file}\\\'\');"
        else:
            key = "pg_walfile_name"
            sql_cmd = f"select pg_walfile_name(\'\\\'{pg_start_file}\\\'\');"
        # 解析pg_wal文件
        pg_sql = self.get_pg_sql()
        return_code, std_out, std_err = pg_sql.exec_sql_cmd(self._os_user_name, sql_cmd)
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error(f"Failed to exec cmd: {sql_cmd}, job id: {self._job_id}.")
            return ""
        wal_file = pg_sql.parse_sql_result(std_out, key)
        LOGGER.info(f"Success to get wal file, file name :{wal_file}")
        return wal_file

    def backup_data(self, pg_start_file):
        backup_type = self._job_dict.get("jobParam", {}).get("backupType", 0)
        if backup_type == BackupTypeEnum.FULL_BACKUP.value:
            if not check_utils.check_path_in_white_list(self._data_area):
                LOGGER.error(f"Data area is incorrect :{self._data_area}.")
                return False, ErrCode.BACKUP_FAILED
            if self._data_path.endswith("/"):
                src_path = self._data_path
            else:
                src_path = self._data_path + "/"
            if not self.backup_directory(src_path, self._data_area, self._job_id):
                LOGGER.error(f"Failed to backup database's files, job id: {self._job_id}.")
                return False, ErrCode.BACKUP_TOOL_FAILED
            copy_pg_start_log_path = os.path.join(self._data_area, "pg_start.log")
            if os.path.exists(copy_pg_start_log_path) and os.path.isfile(copy_pg_start_log_path):
                os.remove(copy_pg_start_log_path)
            if not self.backup_table_space():
                LOGGER.error(f"Failed to backup table space, job id: {self._job_id}.")
                return False, ErrCode.BACKUP_TABLE_SPACE_FAILED
            self.backup_config_file()
            LOGGER.info(f"Succeed to backup database's all files, job id: {self._job_id}.")
        elif backup_type == BackupTypeEnum.LOG_BACKUP.value:
            if not check_utils.check_path_in_white_list(self._log_area):
                LOGGER.error(f"log area is incorrect :{self._log_area}.")
                return False, ErrCode.BACKUP_FAILED
            archive_dir = self.query_archive_dir()
            if not archive_dir:
                LOGGER.error(f"Failed to query archive dir, job id: {self._job_id}")
                return False, ErrCode.BACKUP_FAILED
            file_list = self.query_log_file_list(archive_dir, pg_start_file)
            if len(file_list) == 0:
                LOGGER.info("No new logs need to be backed up.")
                return True, NumberConst.ZERO
            files = [os.path.realpath(os.path.join(archive_dir, file)) for file in file_list]
            for i in range(0, len(files), PgConst.MAX_FILE_NUMBER_OF_LOG_BACKUP):
                result = self.backup_file_list(files[i:i + PgConst.MAX_FILE_NUMBER_OF_LOG_BACKUP], self._log_area,
                                               str(self._loop_time))
                if not result:
                    LOGGER.error(f"Failed to backup wal file: {len(files)}, job id: {self._job_id}.")
                    return False, ErrCode.BACKUP_TOOL_FAILED
                self._loop_time += 1
            LOGGER.info(f"Succeed to backup wal's files, job id: {self._job_id}.")
        else:
            LOGGER.error(f"Unsupported backup type: {backup_type}, job id: {self._job_id}.")
            return False, ErrCode.BACKUP_FAILED
        return True, NumberConst.ZERO

    def backup_config_file(self):
        result, data_directory = self.query_system_param("data_directory")
        if result:
            result, config_file = self.query_system_param("config_file")
            if result and not os.path.abspath(os.path.dirname(config_file)) == os.path.abspath(
                    data_directory) and os.path.exists(config_file):
                copy_user_file_to_dest_path(config_file, self._data_area)
            result, hba_file = self.query_system_param("hba_file")
            if result and not os.path.abspath(os.path.dirname(hba_file)) == os.path.abspath(
                    data_directory) and os.path.exists(hba_file):
                copy_user_file_to_dest_path(hba_file, self._data_area)
            result, ident_file = self.query_system_param("ident_file")
            if result and not os.path.abspath(os.path.dirname(ident_file)) == os.path.abspath(
                    data_directory) and os.path.exists(ident_file):
                copy_user_file_to_dest_path(ident_file, self._data_area)

    def backup_table_space(self):
        table_space = self.get_table_space()
        if len(table_space) == 0:
            LOGGER.info("There is no need to backup table space.")
            return True
        tb_info = dict()
        for name, path in table_space.items():
            if not os.path.exists(path):
                LOGGER.error(f"Table space path is not exist!path :{path}")
                return False
            sub_path = path.split("/")
            sub_path.pop()
            if len(sub_path) == 1:
                # 适配表空间目录在根目录的情况
                table_space_path = os.path.join(self._data_area, DirAndFileNameConst.TABLE_SPACE_INFO_DIR)
            else:
                table_space_path = os.path.join(self._data_area, DirAndFileNameConst.TABLE_SPACE_INFO_DIR, *sub_path)
            LOGGER.info(f"Table_space_path:{table_space_path}")
            job_id = \
                str(uuid.uuid5(uuid.NAMESPACE_X500, self._job_id + DirAndFileNameConst.TABLE_SPACE_INFO_DIR + name))
            LOGGER.info(f"Start backup table space self._job_id: {self._job_id}, backup table space job_id: {job_id}")
            result = self.backup_directory(path, table_space_path, job_id)
            if not result:
                LOGGER.error(f"Failed to backup table space, job id: {self._job_id}, space:{name}.")
                return False
            LOGGER.info(f"Succeed to to backup table space, job id: {self._job_id}, space:{name}.")
            tb_info[name] = (path, self._os_user_name, *self.get_table_space_dir_permission(path))
        # 记录表空间信息
        table_space_path = os.path.join(self._data_area, DirAndFileNameConst.TABLE_SPACE_INFO_DIR,
                                        DirAndFileNameConst.TABLE_SPACE_INFO_FILE)
        self.write_tmp_json_file(tb_info, table_space_path)
        LOGGER.info(f"Write table space info success, tb_info:{tb_info}, job id: {self._job_id}.")
        return True

    def get_table_space(self):
        sql_cmd = "\\db"
        pg_sql = self.get_pg_sql()
        return_code, std_out, std_err = pg_sql.exec_sql_cmd(self._os_user_name, sql_cmd, pager_off=True)
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error(f"Failed to get table space, job id: {self._job_id}.")
            return {}
        all_table_space = pg_sql.parse_html_result(std_out)
        table_space = dict()
        for tb_sp in all_table_space:
            tb_sp_name = tb_sp.get("Name")
            if tb_sp_name and not tb_sp_name.startswith("pg_"):
                table_space[tb_sp_name] = tb_sp.get("Location")
        LOGGER.info(f"Get table space:{table_space}")
        return table_space

    def get_table_space_dir_permission(self, dir_path):
        # 获取table_space目录的权限
        stat_info = os.stat(dir_path)
        uid = stat_info.st_uid
        gid = stat_info.st_gid
        LOGGER.info(f"Get table space permission success!table_space:{dir_path}, job_id:{self._job_id}")
        return uid, gid

    def exec_stop_backup(self):
        pg_ctl_path = os.path.join(self._client_path, "bin", "pg_ctl")
        if not os.path.exists(pg_ctl_path):
            LOGGER.error(f"pg_ctl_path is not exist!")
            return False, ""
        res, version_info = get_version(self._pid, pg_ctl_path, self._os_user_name, self.enable_root)
        if not res:
            LOGGER.error(f"Failed to get version, job id: {self._job_id}.")
            return False, ""
        if not check_utils.is_valid_uuid(self._job_id):
            LOGGER.error(f"Failed to check job id: {self._job_id}.")
            return False, ""
        is_ge_15 = int(version_info.split('.')[0]) >= PgConst.DATABASE_V15
        if is_ge_15:
            sql_cmd = "select pg_backup_stop(true);"
        else:
            sql_cmd = "select pg_stop_backup();"
        pg_sql = self.get_pg_sql()
        if is_ge_15:
            return_code, out, std_err = pg_sql.exec_backup_cmd_catch_error(self._os_user_name, sql_cmd,
                                                                           timeout=PgConst.STOP_PG_BACKUP_TIME_OUT)
            pg_sql.close_session()
        else:
            return_code, _, std_err = pg_sql.exec_sql_cmd(self._os_user_name, sql_cmd,
                                                          timeout=PgConst.STOP_PG_BACKUP_TIME_OUT)
        if return_code == CmdRetCode.CONFIG_ERROR.value:
            return False, ErrCode.ARCHIVE_MODE_CONFIG_ERROR
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error(f"Failed to exec stop backup cmd, job id: {self._job_id}, error : {std_err}.")
            return False, ErrCode.BACKUP_FAILED
        LOGGER.info(f"Succeed to exec stop backup cmd, job id: {self._job_id}.")
        return True, CmdRetCode.EXEC_SUCCESS.value

    def get_backup_info_file(self, wal_dir, index=-1):
        wal_files = os.listdir(wal_dir)
        file_list = []
        for wal_file in wal_files:
            if PostgreCommonUtils.is_backup_wal_file(wal_file):
                file_list.append(os.path.join(wal_dir, wal_file))
        file_list = sorted(file_list, key=lambda x: os.path.getmtime(x))
        backup_file = file_list[index]
        LOGGER.info(f"Get backup info file:{backup_file}")
        return backup_file

    def query_wal_file_list(self, wal_dir):
        # 查找备份文件
        file_list = []
        backup_file = self.get_backup_info_file(wal_dir)
        if not backup_file:
            LOGGER.error(f"Failed to get backup info file, job id: {self._job_id}.")
            return file_list
        # 解析备份文件
        with open(backup_file, "r") as f:
            lines = f.readlines()
        start_wal = None
        stop_wal = None
        for info in lines:
            if "START WAL" in info:
                # START WAL LOCATION: 0/8000028 (file 000000010000000000000008)
                start_wal_info = info.split()[-1]
                # 000000010000000000000008)
                start_wal = start_wal_info[:-1]
            if "STOP WAL" in info:
                # STOP WAL LOCATION: 0/B0000F0 (file 00000001000000000000000B)
                stop_wal_info = info.split()[-1]
                # 000000010000000000000008)
                stop_wal = stop_wal_info[:-1]
        file_list = os.listdir(wal_dir)
        file_list = sorted(file_list, key=lambda x: os.path.getmtime(os.path.join(wal_dir, x)))
        if not (start_wal in file_list and stop_wal in file_list):
            LOGGER.info(f"Start wal:{start_wal} or stop wal:{stop_wal} not in archive dir!")
            return []
        start = file_list.index(start_wal)
        # 日志备份的时候，先去找上次记录下来的上次的备份结束位置，如果能找到，就用上次的结束位置+1。
        # 如果找不到，再用当前备份任务产生的开始位置。最后新增或者更新这个结束位置。
        if self._backup_type == BackupTypeEnum.LOG_BACKUP:
            last_log_backup_stop_wal_info_path = os.path.join(self._meta_area,
                                                              DirAndFileNameConst.LAST_LOG_BACKUP_STOP_WAL_INFO)
            if os.path.exists(last_log_backup_stop_wal_info_path):
                last_log_backup_stop_wal_info = read_tmp_json_file(last_log_backup_stop_wal_info_path)
                expect_start_wal = last_log_backup_stop_wal_info.get("last_stop_wal", start_wal)
                if expect_start_wal in file_list:
                    # expect_start_wal+1 是 00000019000000000000002F.00000028.backup
                    start = self.set_start(expect_start_wal, file_list)
            self._last_stop_wal = stop_wal
        end = file_list.index(stop_wal) + 1
        backup_wal_list = file_list[start:end]
        # 若归档目录存在与本次需备份的日志文件TimeLineID相同的".history"文件，需备份
        time_line_id = start_wal[:8]
        file_list = os.listdir(wal_dir)
        for idx, obj in enumerate(file_list):
            if obj.endswith(".history") and obj[:8] == time_line_id:
                backup_wal_list.append(file_list[idx])
        return backup_wal_list

    def backup_wal_files(self):
        # 查找本次备份wal日志
        archive_dir = self.query_archive_dir()
        if not archive_dir:
            LOGGER.error(f"Failed to query archive dir, job id: {self._job_id}")
            return False
        LOGGER.info(f"archive_dir: {archive_dir}, job id: {self._job_id}")
        file_list = self.query_wal_file_list(archive_dir)
        if not file_list:
            LOGGER.error(f"Failed to wal file list, job id: {self._job_id}")
            return False
        # 获取版本信息
        pg_ctl_path = os.path.join(self._client_path, "bin", "pg_ctl")
        if not os.path.exists(pg_ctl_path):
            LOGGER.error(f"pg_ctl_path is not exist!")
            return False
        res, version_info = get_version(self._pid, pg_ctl_path, self._os_user_name, self.enable_root)
        if not res:
            LOGGER.error(f"Failed to get version, job id: {self._job_id}.")
            return False
        pg_wal_dir = "pg_xlog" if int(version_info.split('.')[0]) < PgConst.DATABASE_V10 else "pg_wal"
        backup_type = self._job_dict.get("jobParam", {}).get("backupType", 0)
        if backup_type == BackupTypeEnum.FULL_BACKUP.value:
            if not check_utils.check_path_in_white_list(self._data_area):
                LOGGER.error(f"Data area is incorrect :{self._data_area}.")
                return False
            # 清空wal目录
            target = os.path.join(self._data_area, pg_wal_dir)
            if not os.path.islink(target):
                clean_dir_not_walk_link(target)
            else:
                # 备份的pg_wal或pg_xlog目录为软链接时，删除软链接直接创一个同名空目录
                real_wal_path = os.path.realpath(target)
                file_stat = os.stat(real_wal_path)
                uid, gid, permissions = file_stat.st_uid, file_stat.st_gid, file_stat.st_mode
                os.remove(target)
                os.makedirs(target, permissions)
                os.chown(target, uid, gid)
        else:
            if not check_utils.check_path_in_white_list(self._log_area):
                LOGGER.error(f"Data area is incorrect :{self._log_area}.")
                return False
            target = self._log_area
        # 备份wal日志
        files = [os.path.realpath(os.path.join(archive_dir, file)) for file in file_list]
        for i in range(0, len(files), PgConst.MAX_FILE_NUMBER_OF_LOG_BACKUP):
            result = self.backup_file_list(files[i:i + PgConst.MAX_FILE_NUMBER_OF_LOG_BACKUP], target,
                                           str(self._loop_time))
            if not result:
                LOGGER.error(f"Failed to backup wal file: {len(files)}, job id: {self._job_id}.")
                return False
            self._loop_time += 1
        LOGGER.info(f"Succeed to backup wal file: {len(files)}, job id: {self._job_id}.")
        return True

    def write_backup_info_file(self):
        content = BackupProgressInfo(last_objects=0, backup_objects=0)
        content.s_time = int(time.time())
        content.c_time = int(time.time())
        content.status = SubJobStatusEnum.RUNNING.value
        self.write_tmp_json_file(content.dict(by_alias=True), "BackupProgress")

    def set_backup_result(self, status, body_err, message):
        progress_file = os.path.realpath(os.path.join(self._cache_area, "BackupProgress"))
        task_info = self.read_tmp_json_file(progress_file)
        if "status" in task_info and "error_code" in task_info and "message" in task_info:
            task_info["status"] = status
            task_info["error_code"] = body_err
            task_info["message"] = message
        LOGGER.info(f"Set backup result, status: {status}, message: {message}, job id: {self._job_id}.")
        self.write_tmp_json_file(task_info, progress_file)

    def save_backup_info(self):
        # 查询归档目录
        archive_dir = self.query_archive_dir()
        if not archive_dir:
            LOGGER.error(f"Failed to query archive dir, job id: {self._job_id}.")
            return False, ErrCode.BACKUP_FAILED
        # 查询备份信息
        backup_file = self.get_backup_info_file(archive_dir)
        if not backup_file:
            LOGGER.error(f"Failed to get backup info file, job id: {self._job_id}.")
            return False, ErrCode.BACKUP_FAILED
        backup_file_path = os.path.realpath(os.path.join(archive_dir, backup_file))
        if not PostgreCommonUtils.check_path_in_white_list(self._cache_area)[0]:
            LOGGER.error(f"Save backup copy failed!cache repo :{self._cache_area} is not in white list!")
            return False, ErrCode.BACKUP_FAILED
        if not self.backup_files(backup_file_path, self._cache_area):
            LOGGER.error(f"Failed to get backup info file, job id: {self._job_id}.")
            return False, ErrCode.BACKUP_TOOL_FAILED
        res, code = self.save_copy_info(backup_file, backup_file_path)
        if not res:
            return False, code
        backup_type = self._job_dict.get("jobParam", {}).get("backupType", 0)
        meta_file_name = os.path.realpath(os.path.join(self._meta_area, "LastBackupCopy.info"))
        if backup_type == BackupTypeEnum.LOG_BACKUP.value and not os.path.exists(meta_file_name):
            # 首次日志备份读取全量备份的结束时间
            res, code = self.save_last_backup_meta(archive_dir)
            return res, code
        LOGGER.info(f"Succeed to save backup info, job id: {self._job_id}.")
        return True, NumberConst.ZERO

    def save_copy_info(self, backup_file, backup_file_path):
        with open(backup_file_path, "r") as file:
            lines = file.readlines()
        stop_time, wal_file = None, None
        for info in lines:
            if "STOP TIME:" in info:
                stop_time = f"{info.split()[2]} {info.split()[3]}"
            if "STOP WAL" in info:
                stop_wal_info = info.split()[-1]
                wal_file = stop_wal_info[:-1]
        # WAL文件名前8位为timeline
        copy_timeline = wal_file[:8] if PostgreCommonUtils.is_wal_file(wal_file) else ""
        LOGGER.info(f"Report backup copy, stop time: {stop_time}, stop wal: {wal_file}, timeline: {copy_timeline}, "
                    f"pid: {self._pid}, job id: {self._job_id}.")
        copy_dict = self._job_dict.get("copy", [{}])[0]
        copy_dict["timestamp"] = convert_time_to_timestamp(stop_time)
        backup_type = self._job_dict.get("jobParam", {}).get("backupType", 0)
        if backup_type == BackupTypeEnum.LOG_BACKUP.value:
            build_ret, extend_info = self.build_log_copy_ext_info(stop_time, wal_file)
            if not build_ret:
                return False, ErrCode.LOG_INCONSISTENT
            copy_dict["extendInfo"] = extend_info
        elif backup_type == BackupTypeEnum.FULL_BACKUP.value:
            copy_dict = self.write_full_backup_time(copy_dict, stop_time, copy_timeline, wal_file)
            result, config_file = self.query_system_param("config_file")
            if result:
                copy_dict["extendInfo"]["configFile"] = config_file
            result, hba_file = self.query_system_param("hba_file")
            if result:
                copy_dict["extendInfo"]["hbaFile"] = hba_file
            result, ident_file = self.query_system_param("ident_file")
            if result:
                copy_dict["extendInfo"]["identFile"] = ident_file
        else:
            return False, ErrCode.BACKUP_FAILED
        LOGGER.info(f"Backup copy info, job id: {self._job_id}.")
        # 记录副本信息文件
        copy_info = {
            "stop_time": stop_time, "wal_file": wal_file, "backup_file": backup_file,
            "copy_dict": copy_dict
        }
        LOGGER.info(f"Write copy info :{copy_info}")
        try:
            self.write_tmp_json_file(copy_info, DirAndFileNameConst.COPY_FILE_INFO)
        except Exception as ex:
            LOGGER.error(f"Failed to write tmp json file, job id: {self._job_id}. Exception info: {ex}.")
            return False, ErrCode.BACKUP_FAILED
        return True, NumberConst.ZERO

    def build_log_copy_ext_info(self, stop_time, wal_file):
        extend_info = dict()
        copy_timeline = wal_file[:8] if PostgreCommonUtils.is_wal_file(wal_file) else ""
        ret, pre_copy_bak_time, pre_copy_stop_wal, pre_copy_timeline = (
            self.get_pre_copy_bak_time_for_log_backup_from_lastest_copy())
        if not ret or not pre_copy_bak_time:
            return False, extend_info
        archive_dir = self.query_archive_dir()
        files = [file for file in os.listdir(archive_dir) if PostgreCommonUtils.is_wal_file(file)]
        is_disconnected_timeline = (pre_copy_timeline and copy_timeline and pre_copy_timeline != copy_timeline)

        # 查询到日志副本检查timeline是否连续
        if is_disconnected_timeline or pre_copy_stop_wal not in files:
            ret, pre_log_copy = self.get_last_copy_info([CopyDataTypeEnum.FULL_COPY.value])
            if ret and pre_log_copy:
                pre_copy_bak_time = pre_log_copy.get("extendInfo", {}).get("backupTime")
                pre_copy_stop_wal = pre_log_copy.get("extendInfo", {}).get("stopWalFile")
                pre_copy_timeline = pre_copy_stop_wal[:8] if PostgreCommonUtils.is_wal_file(
                    pre_copy_stop_wal) else ""
                LOGGER.info(
                    f"Succeed to get previous full copy info, backup time: {pre_copy_bak_time}, stop wal: "
                    f"{pre_copy_stop_wal}, pid: {self._pid}, job id: {self._job_id}.")
                is_disconnected_timeline = (pre_copy_timeline and copy_timeline and pre_copy_timeline != copy_timeline)
                if (is_disconnected_timeline or pre_copy_stop_wal not in files):
                    return False, extend_info

        extend_info["beginTime"] = pre_copy_bak_time
        stop_timestamp = convert_time_to_timestamp(stop_time)
        extend_info["endTime"] = stop_timestamp
        extend_info["backupTime"] = stop_timestamp
        extend_info["timeline"] = copy_timeline
        extend_info["stopWalFile"] = wal_file
        extend_info["backup_node"] = self.get_local_node_service_ip()
        return True, extend_info

    def get_pre_copy_bak_time_for_log_backup_from_lastest_copy(self):
        LOGGER.info(
            f"Getting previous log copy backup time for log backup, pid: {self._pid}, job id: {self._job_id}.")
        ret, pre_log_copy = self.get_last_copy_info([CopyDataTypeEnum.LOG_COPY.value])
        if not ret or not pre_log_copy:
            ret, pre_log_copy = self.get_last_copy_info([CopyDataTypeEnum.FULL_COPY.value])
        if ret and pre_log_copy:
            pre_copy_bak_time = pre_log_copy.get("extendInfo", {}).get("backupTime")
            pre_copy_stop_wal = pre_log_copy.get("extendInfo", {}).get("stopWalFile")
            pre_copy_timeline = pre_copy_stop_wal[:8] if PostgreCommonUtils.is_wal_file(pre_copy_stop_wal) else ""
            LOGGER.info(f"Succeed to get previous log copy info, backup time: {pre_copy_bak_time}, stop wal: "
                        f"{pre_copy_stop_wal}, pid: {self._pid}, job id: {self._job_id}.")
            return True, pre_copy_bak_time, pre_copy_stop_wal, pre_copy_timeline
        return False, "", "", ""

    def get_local_node_service_ip(self):
        host_ips = PostgreCommonUtils.get_local_ips()
        nodes = self._job_dict.get("protectSubObject", [{}])
        for node in nodes:
            node_extend_info = node.get("extendInfo", {})
            service_ip = node_extend_info.get('serviceIp', "")
            if service_ip in host_ips:
                return service_ip
        return ""

    def get_last_copy_info(self, copy_type_array):
        LOGGER.info("Start to get data copy host_sn")
        param = dict()
        param["application"] = self._job_dict.get("protectObject")
        param["types"] = copy_type_array
        param["copyId"] = self._job_id
        param["jobId"] = self._job_id
        copy_info_in_file_name = f"copy_info_hostsn_in_{self._job_id}"
        copy_info_out_file_name = f"copy_info_hostsn_out_{self._job_id}"
        self.write_tmp_json_file(param, copy_info_in_file_name)
        param_file = os.path.join(self._cache_area, copy_info_in_file_name)
        out_file = os.path.join(self._cache_area, copy_info_out_file_name)
        ret = self.exec_rc_tool_cmd("QueryPreviousCopy", param_file, out_file)
        if not ret:
            LOGGER.error(f"Failed to QueryPreviousCopy. pid:{self._pid} jobId{self._job_id}")
            return False, ""
        out_info = self.read_tmp_json_file(out_file)
        if not out_info:
            LOGGER.error(f"Get copy info failed. pid:{self._pid} jobId{self._job_id}")
            return False, ""
        return True, out_info

    def write_full_backup_time(self, copy_dict, stop_time, copy_timeline, wal_file):
        """
        记录全量备份的结束时间和恢复后首次全量备份的结束时间
        :return:
        """
        param = dict()
        param["backupTime"] = convert_time_to_timestamp(stop_time)
        copy_dict["extendInfo"] = param
        copy_dict["extendInfo"]["timeline"] = copy_timeline
        copy_dict["extendInfo"]["stopWalFile"] = wal_file
        copy_dict["extendInfo"]["backup_node"] = self.get_local_node_service_ip()
        LOGGER.info(f"Success get first full backup time, pid:{self._pid} jobId:{self._job_id}")
        return copy_dict

    def save_last_backup_meta(self, archive_dir):
        LOGGER.info("First log backup get last backup stop time.")
        last_backup_file = self.get_backup_info_file(archive_dir, index=0)
        if not last_backup_file:
            LOGGER.error(f"First log backup ,failed to get last backup file, job id: {self._job_id}.")
            return False, ErrCode.BACKUP_FAILED
        last_backup_file_path = os.path.join(archive_dir, last_backup_file)
        stop_time = PgBackup.get_last_backup_stop_time(last_backup_file_path)
        copy_meta_info = {
            "copy_id": self._job_id, "timestamp": stop_time,
            "backup_file": last_backup_file_path
        }
        self.save_copy_meta_info_to_file(copy_meta_info)
        LOGGER.info(f"Success to get and save last backup stop time, stop_time:{stop_time}.")
        return True, NumberConst.ZERO

    @exter_attack
    def backup(self):
        """
        目前Patroni集群备份流程与其余两种集群以及单实例数据库存在差异
        """
        deploy_type = self._job_dict.get("protectEnv", {}).get("extendInfo", {}).get("deployType", 0)
        if int(deploy_type) == DeployType.CLUSTER_TYPE.value:
            install_deploy_type = (self._job_dict.get("protectEnv", {}).get("extendInfo", {}).
                                   get("installDeployType", InstallDeployType.PGPOOL))
            if install_deploy_type == InstallDeployType.PATRONI:
                LOGGER.info(f"step 4: execute backup_sub_job_patroni, job_id: {self._job_id}")
                return self.backup_sub_job_patroni()
        return self.backup_action()

    def backup_task_subjob_dict(self):
        sub_job_dict = {
            BackupSubJob.BACKUP: self.backup_patroni,
        }
        return sub_job_dict

    def backup_sub_job_patroni(self):
        LOGGER.info(f"step 4: start to execute backup_sub_job_patroni, job_id: {self._job_id}")
        self.write_progress_file(SubJobStatusEnum.RUNNING, 0)
        # 启动一个线程查询备份进度
        sub_job_dict = self.backup_task_subjob_dict()
        progress_thread = threading.Thread(name='pre_progress', target=self.upload_backup_progress)
        progress_thread.daemon = True
        progress_thread.start()
        # 执行子任务
        sub_job_name = self._param_dict.get("subJob", {}).get("jobName", "")
        if not sub_job_name:
            return False
        self._sub_job_name = sub_job_name

        try:
            ret = sub_job_dict.get(sub_job_name)()
        except Exception as err:
            LOGGER.error(f"do {sub_job_name} fail: {err}, job_id: {self._job_id}")
            log_detail_param = []
            if sub_job_name == BackupSubJob.BACKUP:
                log_detail_param.append(self._instance_id)
            log_detail = LogDetail(logInfo="plugin_task_subjob_fail_label", logInfoParam=[self._sub_job_id],
                                   logLevel=DBLogLevel.ERROR.value, logDetailParam=log_detail_param)

            PostgreCommonUtils.report_job_details(self._pid, SubJobDetails(taskId=self._job_id,
                                                  subTaskId=self._sub_job_id, progress=100, logDetail=[log_detail],
                                                  taskStatus=SubJobStatusEnum.FAILED.value).dict(by_alias=True))
            return False
        if not ret:
            LOGGER.error(f"Exec sub job {sub_job_name} failed.{self.get_log_comm()}.")
            log_detail_param = []
            if sub_job_name == BackupSubJob.BACKUP:
                log_detail_param.append(self._instance_id)
            log_detail = LogDetail(logInfo="plugin_task_subjob_fail_label", logInfoParam=[self._sub_job_id],
                                   logLevel=DBLogLevel.ERROR.value, logDetailParam=log_detail_param)

            PostgreCommonUtils.report_job_details(self._pid, SubJobDetails(taskId=self._job_id,
                                                  subTaskId=self._sub_job_id, progress=100, logDetail=[log_detail],
                                                  taskStatus=SubJobStatusEnum.FAILED.value).dict(by_alias=True))
            return False

        progress_thread.join()
        return True

    def fetch_current_host(self):
        host = str(self._param_dict.get("subJob", {}).get("jobInfo", ""))
        LOGGER.info(f"the local host is {host}")
        return host

    def backup_patroni(self):
        # 执行数据备份子任务
        LOGGER.info(f"step 5: start to exec_back_up, job_id: {self._job_id}")
        # 发送备份请求
        host = self.fetch_current_host()
        # NodesInfo共享文档
        nodes_info_file = os.path.join(self._meta_area, "nodesInfo", f"job_id_{self._job_id}")
        backup_thread = ""
        timeout = 10

        while True:
            nodes = self._job_dict.get("protectSubObject", [])
            cluster_nodes = PostgreCommonUtils.get_nodes(nodes)
            sql_lists = []
            for cluster_node in cluster_nodes:
                curr_node = cluster_node.get('hostname')
                is_master = 1 if cluster_node.get('role') in [str(RoleType.PRIMARY.value)] else 0
                is_alive = 0 if cluster_node.get('status') in ['running', 'streaming'] else 1
                sql = f"update nodeinfo set is_master={is_master},is_alive={is_alive} " \
                      f"where node_host='{curr_node}'"
                sql_lists.append(sql)
            PostgreCommonUtils.exec_sqlite_sql(nodes_info_file, timeout, sql_lists)
            LOGGER.info("step 5: before can_exec_backup")
            if PostgreCommonUtils.can_exec_backup(host, nodes_info_file, nodes, self._job_id):
                LOGGER.info(f"step 5: can_exec_backup")
                backup_thread = multiprocessing.Process(target=self.backup_patroni_action,
                                                        args=(nodes_info_file, host))
                LOGGER.info(f"step 5: after multiprocessing.Process")
                try:
                    backup_thread.start()
                except Exception as ex:
                    LOGGER.error(f"when start backup thread, exception {ex} occurs")
                LOGGER.info(f"step 5: after backup_thread.start")
                if backup_thread:
                    monitor_thread = multiprocessing.Process(target=PostgreCommonUtils.monitor,
                                                             args=(backup_thread, nodes_info_file, host, nodes))
                    monitor_thread.daemon = True
                    monitor_thread.start()
                backup_thread.join()
            if PostgreCommonUtils.is_job_finished(nodes_info_file):
                break
            time.sleep(15)
        LOGGER.info("step 5: end to exec_back_up")
        return True

    def backup_patroni_action(self, file_name, node_info):
        try:
            res = self.backup_action()
            if not res:
                PostgreCommonUtils.set_failed_and_choose_node(file_name, node_info)
                return False
            PostgreCommonUtils.after_backup(file_name, node_info)
            return True
        except Exception as ex:
            LOGGER.exception(f"Back up data Failed!, exception: {ex}")
            PostgreCommonUtils.set_failed_and_choose_node(file_name, node_info)
            return False

    def write_progress_file(self, task_status, progress):
        LOGGER.info("start write_progress_file")
        if task_status == SubJobStatusEnum.FAILED.value:
            log_detail_param = []
            if self._sub_job_name == BackupSubJob.BACKUP:
                self._err_code = None
                log_detail_param.append(self._instance_id)
                LOGGER.info(f"start self._sub_job_name: {self._sub_job_name}")
            self.set_log_detail_with_params("plugin_task_subjob_fail_label", self._sub_job_id, self._err_code,
                                            log_detail_param,
                                            DBLogLevel.ERROR.value)
        if task_status == SubJobStatusEnum.COMPLETED.value:
            self.set_log_detail_with_params("plugin_task_subjob_success_label", self._sub_job_id, 0, [],
                                            DBLogLevel.INFO.value)
            LOGGER.info("task_status == SubJobStatusEnum.COMPLETED.value")

        progress_str = SubJobDetails(taskId=self._job_id,
                                     subTaskId=self._sub_job_id,
                                     taskStatus=task_status,
                                     progress=progress,
                                     logDetail=self._logdetail)
        json_str = progress_str.dict(by_alias=True)
        progress_file = os.path.join(self._cache_area, f"progress_{self._job_id}_{self._sub_job_id}")
        LOGGER.debug(f"Write file.{progress_str}{self.get_log_comm()}.")
        output_execution_result_ex(progress_file, json_str)

    def set_log_detail_with_params(self, log_label, sub_job_id, err_code=None, log_detail_param=None,
                                   log_level=DBLogLevel.INFO.value):
        err_dict = LogDetail(logInfo=log_label,
                             logInfoParam=[sub_job_id],
                             logTimestamp=int(time.time()),
                             logDetail=err_code,
                             logDetailParam=log_detail_param,
                             logLevel=log_level)
        self._logdetail = []
        self._logdetail.append(err_dict)
        return True

    def get_log_comm(self):
        return f"pid:{self._pid} jobId:{self._job_id} subjobId:{self._sub_job_id}"

    def upload_backup_progress(self):

        # 定时上报备份进度
        while self._job_status == SubJobStatusEnum.RUNNING:
            if self._sub_job_name == BackupSubJob.BACKUP:
                self._backup_status = self.get_progress()
                if self._backup_status == PgsqlBackupStatus.RUNNING:
                    progress = 20
                    status = SubJobStatusEnum.RUNNING
                elif self._backup_status == PgsqlBackupStatus.SUCCEED:
                    status = SubJobStatusEnum.COMPLETED
                    progress = 100
                else:
                    status = SubJobStatusEnum.FAILED
                    progress = 0
                LOGGER.info(f"progress_info.job(status): {self._backup_status}")
                LOGGER.info(f"status：{status}   progress: {progress}")
                self.write_progress_file(status, progress)

            LOGGER.info("Start to report progress.")
            progress_file = os.path.join(self._cache_area, f"progress_{self._job_id}_{self._sub_job_id}")
            # 没有进度文件可能是还没有生成,不返回失败
            comm_progress_dict = SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id,
                                               taskStatus=SubJobStatusEnum.RUNNING,
                                               progress=0, logDetail=self._logdetail)
            if not os.path.exists(progress_file):
                PostgreCommonUtils.report_job_details(self._job_id, comm_progress_dict.dict(by_alias=True))
                time.sleep(self._query_progress_interval)
                continue
            with open(progress_file, "r") as f_object:
                progress_dict = json.loads(f_object.read())

            self._job_status = progress_dict.get("taskStatus")
            LOGGER.info(f"Get progress_dict in upload_backup_progress.{self._job_status}")
            LOGGER.info(f"upload_backup_progress{self.get_log_comm()}")
            if not self._job_status:
                LOGGER.error(f"Failed to obtain the task status.{self.get_log_comm()}")
                self._job_status = SubJobStatusEnum.FAILED
                fail_dict = SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id,
                                          taskStatus=SubJobStatusEnum.FAILED, progress=100,
                                          logDetail=self._logdetail)
                progress_dict = fail_dict.dict(by_alias=True)
            LOGGER.info(f"progress_dict{progress_dict}")

            time.sleep(self._query_progress_interval)
            PostgreCommonUtils.report_job_details(self._job_id, progress_dict)

    def get_progress(self):
        nodes_info_file = os.path.join(self._meta_area, "nodesInfo", f"job_id_{self._job_id}")
        nodes_info = PostgreCommonUtils.get_nodelist(nodes_info_file, 10)
        num = len(nodes_info)
        LOGGER.info("start get_progress")
        is_failed = 0
        for value in nodes_info:
            if value[3] == 1:
                LOGGER.info("job succeed")
                return PgsqlBackupStatus.SUCCEED
            elif value[3] == 2:
                is_failed += 1
        if is_failed == num and num != 0:
            LOGGER.error("job failed")
            return PgsqlBackupStatus.FAILED
        LOGGER.info("job running")
        return PgsqlBackupStatus.RUNNING

    def check_node_status_patroni(self):
        host_ips = PostgreCommonUtils.get_local_ips()
        nodes = self._job_dict.get("protectSubObject", [{}])
        patroni_config, _, _ = PostgreCommonUtils.get_patroni_config(host_ips, nodes)
        # 先通过ip找到对应的对应的patroni_config文件
        cluster_nodes = ClusterNodesChecker.get_nodes(patroni_config)
        for cluster_node in cluster_nodes:
            host = cluster_node.get('hostname', "")
            if host in host_ips:
                if cluster_node.get('role', "") == str(RoleType.PRIMARY.value):
                    return True
                else:
                    return False
        return False

    def backup_action(self):
        if not self.check_params():
            LOGGER.error(f"Check the path, IP address, and port!")
            return False
        # 备份状态设置为备份中
        self.write_backup_info_file()
        # 查询是否开启归档模式
        res, code = self.query_archive_mode()
        if not res:
            message = "Archive mode is off."
            self.set_backup_result(SubJobStatusEnum.FAILED.value, code, message)
            LOGGER.error(f"Archive mode is off, job_id: {self._job_id}.")
            return False
        archive_dir = self.query_archive_dir()
        if not archive_dir or not os.path.isdir(archive_dir):
            self.set_backup_result(SubJobStatusEnum.FAILED.value, ErrCode.ARCHIVE_MODE_CONFIG_ERROR, "Archive command "
                                                                                                     "is error.")
            LOGGER.error(f"Archive command is useless, archive_dir: {archive_dir}, job_id: {self._job_id}.")
            return False
        # 开始备份
        result, pg_start_file = self.exec_start_backup()
        if not result:
            self.set_backup_result(SubJobStatusEnum.FAILED.value, ErrCode.BACKUP_FAILED,
                                   "Failed to exec start backup cmd.")
            return False
        # 备份数据文件
        try:
            result, error_code = self.backup_data(pg_start_file)
        except Exception as ex:
            LOGGER.exception("Back up data Failed!")
            self.exec_stop_backup()
            raise ex
        if not result:
            self.set_backup_result(SubJobStatusEnum.FAILED.value, error_code, "Failed to backup database's files.")
            self.exec_stop_backup()
            return False
        # 停止备份
        result, error_code = self.exec_stop_backup()
        if not result:
            self.set_backup_result(SubJobStatusEnum.FAILED.value, error_code, "Failed to exec stop backup cmd.")
            return False
        # 备份wal日志
        result = self.backup_wal_files()
        if not result:
            self.set_backup_result(SubJobStatusEnum.FAILED.value, ErrCode.BACKUP_FAILED, "Failed to backup wal files.")
            return False
        result, error_code = self.save_backup_info()
        if not result:
            self.set_backup_result(SubJobStatusEnum.FAILED.value, error_code, "Failed to save backup info.")
            return False
        # 备份完成
        self.set_backup_result(SubJobStatusEnum.COMPLETED.value, "", "")
        # 如果是日志备份，备份完成后，要更新stop_wal
        self.update_last_log_backup_stop_wal_info()
        LOGGER.info(f"Succeed to exec backup sub task, job id: {self._job_id}")
        return True

    def update_last_log_backup_stop_wal_info(self):
        if self._backup_type == BackupTypeEnum.LOG_BACKUP:
            last_log_backup_stop_wal_info_path = os.path.join(self._meta_area,
                                                              DirAndFileNameConst.LAST_LOG_BACKUP_STOP_WAL_INFO)
            last_log_backup_stop_wal_info = {
                "last_stop_wal": self._last_stop_wal
            }
            output_execution_result_ex(last_log_backup_stop_wal_info_path, last_log_backup_stop_wal_info)

    @exter_attack
    def backup_post_job(self):
        backup_file = os.path.join(self._cache_area, "BackupProgress")
        if os.path.exists(backup_file):
            backup_result = self.read_tmp_json_file(backup_file)
            backup_path = None
            backup_type = self._job_dict.get("jobParam", {}).get("backupType", 0)
            if backup_type == BackupTypeEnum.FULL_BACKUP.value:
                backup_path = self._data_area
            elif backup_type == BackupTypeEnum.LOG_BACKUP.value:
                backup_path = self._cache_area
            if backup_result.get("status", 0) != SubJobStatusEnum.COMPLETED.value \
                    or os.path.exists(os.path.join(self._cache_area, "abort.done")):
                # 任务失败或终止清理备份数据
                if not check_utils.check_path_in_white_list(backup_path):
                    LOGGER.error("The backup repo is incorrect.")
                    return False
                self.clear_repository_dir(backup_path)
                LOGGER.info(f"Succeed to clean backup storage: {backup_path}, job id: {self._job_id}.")
                # 任务失败执行一次停止备份命令
                self.exec_stop_backup()
        if os.path.exists(self._cache_area) and check_utils.check_path_in_white_list(self._cache_area):
            # 清理cache仓
            self.clear_repository_dir(self._cache_area)
            LOGGER.info(f"Succeed to remove tmp dir: {self._cache_area}, job id: {self._job_id}.")
        result_file = os.path.join(self._cache_area, "BackupPostJobProgress")
        pathlib.Path(result_file).touch()
        return True

    def save_copy_meta_info_to_file(self, context):
        file_name = os.path.join(self._meta_area, "LastBackupCopy.info")
        if self.path_check(file_name):
            LOGGER.error(f"The path[{file_name}] is invalid, job id: {self._job_id}.")
            return
        if os.path.exists(file_name):
            ret, realpath = PostgreCommonUtils.check_path_in_white_list(file_name)
            if ret:
                os.remove(realpath)
                LOGGER.info(f"Remove file: {realpath}, job id: {self._job_id}.")
        write_content_to_file(file_name, json.dumps(context))
        repl_user = self._job_dict.get("protectObject", {}).get("auth", {}).get("extendInfo", {}).get("dbStreamRepUser")
        if repl_user:
            # 集群才需要将生产端的用户名密码保存到副本里
            repl_info_name = os.path.join(self._data_area, f"Repl_{self._job_id}.info")
            if not check_path_valid(repl_info_name):
                LOGGER.error(f"The path[{repl_info_name}] is invalid, job id: {self._job_id}.")
                return
            repl_pwd = get_env_variable(f"job_protectObject_auth_extendInfo_dbStreamRepPwd_{self._pid}")
            repl_user = Kmc().encrypt(repl_user)
            repl_pwd = Kmc().encrypt(repl_pwd)
            write_content_to_file(repl_info_name, json.dumps({'repl_user': repl_user, 'repl_pwd': repl_pwd}))
        LOGGER.info(f"Save copy meta info to file: {file_name}, job id: {self._job_id}.")

    @exter_attack
    def query_backup_copy(self):
        # 查询备份信息
        if not PostgreCommonUtils.check_path_in_white_list(self._meta_area)[0]:
            LOGGER.error(f"Query backup copy failed!meta repo :{self._meta_area} is not in white list!")
            return False
        if not PostgreCommonUtils.check_path_in_white_list(self._cache_area)[0]:
            LOGGER.error(f"Query backup copy failed!cache repo :{self._cache_area} is not in white list!")
            return False
        copy_info_path = os.path.realpath(os.path.join(self._cache_area, DirAndFileNameConst.COPY_FILE_INFO))
        try:
            copy_info = self.read_tmp_json_file(copy_info_path)
        except Exception as ex:
            LOGGER.error(f"Failed to read tmp json file, job id: {self._job_id}. Exception info: {ex}.")
            return False
        backup_file = copy_info.get("backup_file")
        stop_time = copy_info.get("stop_time")
        wal_file = copy_info.get("wal_file")
        copy_dict = copy_info.get("copy_dict")
        backup_file_path = os.path.join(self._cache_area, backup_file)
        copy_meta_info = {
            "copy_id": self._job_id, "timestamp": stop_time, "wal_file": wal_file,
            "backup_file": backup_file_path
        }
        self.save_copy_meta_info_to_file(copy_meta_info)
        LOGGER.info(f"Backup copy info, job id: {self._job_id}, copy dict :{copy_dict}.")
        output_result_file(self._pid, copy_dict)
        return True

    def query_progress_common(self, progress_type):
        file_path = os.path.join(self._cache_area, progress_type)
        progress = PgConst.PROGRESS_ONE_HUNDRED
        if not os.path.exists(file_path):
            LOGGER.error(f"Failed to query progress, job id: {self._job_id}.")
            job_status = SubJobStatusEnum.FAILED.value
        else:
            LOGGER.info(f"Succeed to query progress, job id: {self._job_id}.")
            job_status = SubJobStatusEnum.COMPLETED.value
        return progress, job_status

    @exter_attack
    def query_prerequisite_progress(self):
        progress, job_status = self.query_progress_common("BackupPrerequisiteProgress")
        output = SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, taskStatus=job_status,
                               progress=progress)
        output_result_file(self._pid, output.dict(by_alias=True))
        LOGGER.info(f"SubJobDetails: {output.dict(by_alias=True)}, job id: {self._job_id}.")
        return True

    @exter_attack
    def query_backup_progress(self):
        """
        查询备份进度
        :return: boolean，True代表备份成功，False代表备份失败
        """
        file_path = os.path.join(self._cache_area, "BackupProgress")
        try:
            progress_info = self.read_tmp_json_file(file_path)
        except Exception as ex:
            LOGGER.error(f"Failed to read tmp json file, job id: {self._job_id}. Exception info: {ex}.")
            return False
        LOGGER.info(f"Progress info: {progress_info}, job id: {self._job_id}.")
        status = progress_info.get("status")
        data_size = 0
        if status == SubJobStatusEnum.COMPLETED.value:
            progress = PgConst.PROGRESS_ONE_HUNDRED
            backup_type = self._job_dict.get("jobParam", {}).get("backupType", 0)
            data_path = self._data_area if backup_type == BackupTypeEnum.FULL_BACKUP.value else self._log_area
            if not check_utils.check_path_in_white_list(data_path):
                LOGGER.error("The backup repo is incorrect.")
                return False
            ret, size = scan_dir_size(str(uuid.uuid4()), data_path)
            LOGGER.info(f"Success get copy size: {size}")
            if ret:
                data_size = int(size)
        else:
            progress = PgConst.PROGRESS_FIFTY
        progress_record = os.path.realpath(os.path.join(self._cache_area, "progress.done"))

        if status == SubJobStatusEnum.FAILED.value:
            error_code = progress_info.get("error_code", ErrCode.BACKUP_FAILED)
            log_detail = LogDetail(logInfo=ReportDBLabel.BACKUP_SUB_FAILED, logInfoParam=[self._sub_job_id],
                                   logLevel=DBLogLevel.ERROR.value, logDetail=error_code)
            output = SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, taskStatus=status,
                                   progress=progress, logDetail=[log_detail])
        # 如果子任务job log已上报，则不需要再次上报
        elif not os.path.exists(progress_record):
            pathlib.Path(progress_record).touch()
            log_detail = LogDetail(logInfo=ReportDBLabel.BACKUP_SUB_START_COPY, logInfoParam=[self._sub_job_id],
                                   logLevel=DBLogLevel.INFO.value)
            output = SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, taskStatus=status,
                                   progress=progress, logDetail=[log_detail], dataSize=data_size)
        else:
            output = SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, taskStatus=status,
                                   progress=progress, dataSize=data_size)
        output_result_file(self._pid, output.dict(by_alias=True))
        LOGGER.info(f"SubJobDetails: {output.dict(by_alias=True)}, job id: {self._job_id}.")
        return True

    @exter_attack
    def query_post_job_progress(self):
        progress, job_status = self.query_progress_common("BackupPostJobProgress")
        output = SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, progress=progress,
                               taskStatus=job_status)
        LOGGER.info(f"SubJobDetails: {output.dict(by_alias=True)}, job id: {self._job_id}.")
        output_result_file(self._pid, output.dict(by_alias=True))
        return True

    def write_tmp_json_file(self, context, file_name):
        file_path = os.path.join(self._cache_area, file_name)
        if self.path_check(file_path):
            LOGGER.error(f"The path[{file_path}] is invalid, job id: {self._job_id}.")
            return
        if os.path.exists(file_path):
            ret, realpath = PostgreCommonUtils.check_path_in_white_list(file_path)
            if ret:
                os.remove(realpath)
                LOGGER.info(f"Remove file: {realpath}, job id: {self._job_id}.")
        flags = os.O_WRONLY | os.O_CREAT | os.O_EXCL
        modes = stat.S_IWUSR | stat.S_IRUSR | stat.S_IXUSR
        with os.fdopen(os.open(file_path, flags, modes), 'w+') as out_file:
            out_file.write(json.dumps(context))

    def read_tmp_json_file(self, file_path):
        if not os.path.isfile(file_path):
            LOGGER.error(f"File:{file_path} not exist, job id: {self._job_id}.")
            raise Exception(f"File:{file_path} not exist, job id: {self._job_id}.")
        try:
            with open(file_path, "r", encoding='UTF-8') as f:
                json_dict = json.loads(f.read())
        except Exception as e:
            LOGGER.error(f"Failed to parse param file, job id: {self._job_id}.")
            raise Exception("parse param file failed") from e
        return json_dict

    @exter_attack
    def abort_job(self):
        abort_file = os.path.join(self._cache_area, "abort.ing")
        pathlib.Path(abort_file).touch()
        pid_list = psutil.pids()
        for pid in pid_list:
            process = psutil.Process(pid)
            try:
                cmd = process.cmdline()
            except Exception as ex:
                LOGGER.warn(f"The pid {pid} of process not exist! err:{ex}")
                continue
            if 'python3' in cmd and (self._job_id in cmd and self._pid not in cmd):
                try:
                    process.kill()
                except Exception as ex:
                    LOGGER.warn(f"Kill process kill error!err:{ex}")
                    break
                LOGGER.info(f"The backup task has been terminated, job id: {self._job_id}.")
                break
        os.rename(abort_file, os.path.join(self._cache_area, "abort.done"))
        LOGGER.info(f"Succeed to abort backup job, job id: {self._job_id}.")
        return True

    @exter_attack
    def query_scan_repositories(self):
        # E6000适配
        LOGGER.info(f"Query scan repositories, job_id: {self._job_id}.")
        backup_type = self._job_dict.get("jobParam", {}).get("backupType", 0)
        if backup_type == BackupTypeEnum.LOG_BACKUP.value:
            # log仓的meta区 /Database_{resource_id}_LogRepository_su{num}/{ip}/meta/{job_id}
            meta_copy_path = os.path.join(os.path.dirname(self._log_area), RepositoryNameEnum.META, self._job_id)
            # log仓的data区 /Database_{resource_id}_LogRepository_su{num}/{ip}/{job_id}
            data_path = self._log_area
            # /Database_{resource_id}_LogRepository_su{num}/{ip}
            save_path = os.path.dirname(self._log_area)
        else:
            # meta/Database_{resource_id}_InnerDirectory_su{num}/source_policy_{job_id}/Context_Global_MD/{ip}
            meta_copy_path = self._meta_area
            # data/Database_{resource_id}_InnerDirectory_su{num}/source_policy_{job_id}/Context/{ip}
            data_path = self._data_area
            # meta/Database_{resource_id}_InnerDirectory_su{num}/source_policy_{job_id}/Context_Global_MD/{ip}
            save_path = self._meta_area
        if not os.path.exists(meta_copy_path):
            exec_mkdir_cmd(meta_copy_path, mode=0x777)
        log_meta_copy_repo = RepositoryPath(repositoryType=RepositoryDataTypeEnum.META_REPOSITORY.value,
                                            scanPath=meta_copy_path)
        log_data_repo = RepositoryPath(repositoryType=RepositoryDataTypeEnum.LOG_REPOSITORY.value,
                                       scanPath=data_path)
        scan_repos = ScanRepositories(scanRepoList=[log_data_repo, log_meta_copy_repo], savePath=save_path)
        output_result_file(self._pid, scan_repos.dict(by_alias=True))
        LOGGER.info(f"Query scan repos success, return result {scan_repos}, job id: {self._job_id}")
        return True

    @exter_attack
    def query_abort_job_progress(self):
        if os.path.exists(os.path.join(self._cache_area, "abort.ing")):
            status = SubJobStatusEnum.ABORTING.value
        elif os.path.exists(os.path.join(self._cache_area, "abort.done")):
            status = SubJobStatusEnum.ABORTED.value
        else:
            status = SubJobStatusEnum.ABORTED_FAILED.value
        output = SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, progress=PgConst.PROGRESS_ONE_HUNDRED,
                               taskStatus=status)
        LOGGER.info(f"SubJobDetails: {output.dict(by_alias=True)}, job id: {self._job_id}.")
        output_result_file(self._pid, output.dict(by_alias=True))
        return True
