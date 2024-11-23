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

from common.logger import Logger
from common.const import ExecuteResultEnum, SubJobStatusEnum, BackupTypeEnum, DeployType, RepositoryDataTypeEnum, \
    DBLogLevel, ReportDBLabel, CopyDataTypeEnum, ParamConstant, RoleType
from common.common import output_result_file, execute_cmd, convert_time_to_timestamp, exter_attack, \
    write_content_to_file, read_tmp_json_file, output_execution_result_ex, clean_dir_not_walk_link
from common.number_const import NumberConst
from common.util import check_utils, check_user_utils
from common.util.backup import backup, backup_files, query_progress
from common.common_models import ActionResult, SubJobDetails, LogDetail
from common.util.cmd_utils import cmd_format
from common.util.exec_utils import check_path_valid, exec_cp_cmd
from common.util.scanner_utils import scan_dir_size
from postgresql.common.const import CmdRetCode, ErrorCode, PgConst, BackupStatus, DirAndFileNameConst, InstallDeployType
from postgresql.common.pg_exec_sql import ExecPgSql
from postgresql.common.models import BackupJobPermission, BackupProgressInfo
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
        self._os_user_name = self._job_dict.get("protectObject", {}).get("extendInfo", {}).get("osUsername", "")
        self._port = self._job_dict.get("protectObject", {}).get("extendInfo", {}).get("instancePort",
                                                                                       PgConst.DB_DEFAULT_PORT)
        deploy_type = self._job_dict.get("protectEnv", {}).get("extendInfo", {}).get("deployType", 0)
        if int(deploy_type) == DeployType.SINGLE_TYPE.value:
            self._client_path = self._job_dict.get("protectObject", {}).get("extendInfo", {}).get("clientPath", "")
            self._data_path = self._job_dict.get("protectObject", {}).get("extendInfo", {}).get("dataDirectory", "")
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
            self._port = self._job_dict.get("protectObject", {}).get("extendInfo", {}).get("instancePort",
                                                                                           PgConst.DB_DEFAULT_PORT)
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
        if not check_path_valid(self._data_path, False):
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

    def query_archive_mode(self):
        sql_cmd = "show archive_mode;"
        pg_sql = self.get_pg_sql()
        return_code, std_out, st_err = pg_sql.exec_sql_cmd(self._os_user_name, sql_cmd)
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error(f"Failed to exec cmd: {sql_cmd}, job_id: {self._job_id}.")
            return False, ErrorCode.PLUGIN_CANNOT_BACKUP_ERR.value
        if "archive_mode" not in std_out:
            return False, ErrorCode.PLUGIN_CANNOT_BACKUP_ERR.value
        archive_mode = pg_sql.parse_sql_result(std_out, "archive_mode")
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
            if install_deploy_type == InstallDeployType.PATRONI and not self.check_node_is_primary_patroni():
                body_err = ErrorCode.PLUGIN_CANNOT_BACKUP_ERR.value
                message = "Cluster node is standby."
                self.set_action_result(ExecuteResultEnum.INTERNAL_ERROR.value, body_err, message)
                LOGGER.error(f"Failed to check cluster status, job_id: {self._job_id}.")
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

    def query_archive_dir(self):
        sql_cmd = "show archive_command;"
        pg_sql = self.get_pg_sql()
        return_code, std_out, std_err = pg_sql.exec_sql_cmd(self._os_user_name, sql_cmd)
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error(f"Failed to exec cmd: {sql_cmd}, job id: {self._job_id}.")
            return ""
        archive_info = pg_sql.parse_sql_result(std_out, "archive_command")
        archive_dir = archive_info.split()[archive_info.split().index("cp") + 2]
        return archive_dir.strip('"%f')

    @exter_attack
    def check_backup_job_type(self):
        return True

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
        result_file = os.path.join(self._cache_area, "BackupPrerequisiteProgress")
        pathlib.Path(result_file).touch()
        return True

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
            return_code, start_file, std_err = pg_sql.exec_backup_cmd(self._os_user_name, sql_cmd,
                                                                      timeout=PgConst.CHECK_POINT_TIME_OUT)
        else:
            return_code, start_file, std_err = pg_sql.exec_sql_cmd(self._os_user_name, sql_cmd,
                                                                   timeout=PgConst.CHECK_POINT_TIME_OUT)
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error(f"Failed to exec start backup cmd, job id: {self._job_id}.")
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

    def backup_file_list(self, files, target):
        if not files or not target:
            LOGGER.error(f"Param error, job id: {self._job_id}.")
            return False
        res = backup_files(self._job_id, files, target, write_meta=True, thread_num=self._thread_number)
        if not res:
            LOGGER.error(f"Failed to start backup, jobId: {self._job_id}.")
            return False
        return self.get_backup_status(self._job_id)

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
                result = self.backup_file_list(files[i:i + PgConst.MAX_FILE_NUMBER_OF_LOG_BACKUP], self._log_area)
                if not result:
                    LOGGER.error(f"Failed to backup wal file: {len(files)}, job id: {self._job_id}.")
                    return False, ErrCode.BACKUP_TOOL_FAILED
            LOGGER.info(f"Succeed to backup wal's files, job id: {self._job_id}.")
        else:
            LOGGER.error(f"Unsupported backup type: {backup_type}, job id: {self._job_id}.")
            return False, ErrCode.BACKUP_FAILED
        return True, NumberConst.ZERO

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
            return_code, out, std_err = pg_sql.exec_backup_cmd(self._os_user_name, sql_cmd,
                                                               timeout=PgConst.STOP_PG_BACKUP_TIME_OUT)
            pg_sql.close_session()
        else:
            return_code, _, std_err = pg_sql.exec_sql_cmd(self._os_user_name, sql_cmd,
                                                          timeout=PgConst.STOP_PG_BACKUP_TIME_OUT)
        if return_code == CmdRetCode.CONFIG_ERROR.value:
            return False, ErrCode.ARCHIVE_MODE_CONFIG_ERROR
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error(f"Failed to exec stop backup cmd, job id: {self._job_id}.")
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
        if int(version_info.split('.')[0]) < PgConst.DATABASE_V10:
            pg_wal_dir = "pg_xlog"
        else:
            pg_wal_dir = "pg_wal"
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
            if not check_utils.check_path_in_white_list(self._log_area):
                LOGGER.error(f"Data area is incorrect :{self._log_area}.")
                return False
            target = self._log_area
        # 备份wal日志
        files = [os.path.join(archive_dir, file) for file in file_list]
        result = self.backup_file_list(files, target)
        if not result:
            LOGGER.error(f"Failed to wal file: {len(files)}, job id: {self._job_id}.")
            return False
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
        backup_file_path = os.path.join(archive_dir, backup_file)
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
            copy_dict = self.write_full_backup_time(copy_dict, stop_time)
            copy_dict["extendInfo"]["timeline"] = copy_timeline
            copy_dict["extendInfo"]["stopWalFile"] = wal_file
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
        ret, pre_copy_bak_time, pre_copy_timeline = self.get_pre_copy_bak_time_for_log_backup_from_lastest_copy()
        if not ret or not pre_copy_bak_time:
            return False, extend_info

        # 查询到日志副本检查timeline是否连续
        if pre_copy_timeline and copy_timeline and pre_copy_timeline != copy_timeline:
            return False, extend_info

        extend_info["beginTime"] = pre_copy_bak_time
        stop_timestamp = convert_time_to_timestamp(stop_time)
        extend_info["endTime"] = stop_timestamp
        extend_info["backupTime"] = stop_timestamp
        extend_info["timeline"] = copy_timeline
        extend_info["stopWalFile"] = wal_file
        return True, extend_info

    def get_pre_copy_bak_time_for_log_backup_from_lastest_copy(self):
        LOGGER.info(
            f"Getting previous log copy backup time for log backup, pid: {self._pid}, job id: {self._job_id}.")
        ret, pre_log_copy = self.get_last_copy_info([CopyDataTypeEnum.LOG_COPY.value])
        if not ret or not pre_log_copy:
            ret, pre_log_copy = self.get_last_copy_info([CopyDataTypeEnum.FULL_COPY.value])
        if ret and pre_log_copy:
            pre_copy_bak_time = pre_log_copy.get("extendInfo", {}).get("backupTime")
            pre_copy_stop_wal = pre_log_copy.get("", {}).get("stopWalFile")
            pre_copy_timeline = pre_copy_stop_wal[:8] if PostgreCommonUtils.is_wal_file(pre_copy_stop_wal) else ""
            LOGGER.info(f"Succeed to get previous log copy info, backup time: {pre_copy_bak_time}, stop wal: "
                        f"{pre_copy_stop_wal}, pid: {self._pid}, job id: {self._job_id}.")
            return True, pre_copy_bak_time, pre_copy_timeline
        return False, "", ""

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

    def write_full_backup_time(self, copy_dict, stop_time):
        """
        记录全量备份的结束时间和恢复后首次全量备份的结束时间
        :return:
        """
        param = dict()
        param["backupTime"] = convert_time_to_timestamp(stop_time)
        copy_dict["extendInfo"] = param
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
        if self._backup_type == BackupTypeEnum.LOG_BACKUP:
            last_log_backup_stop_wal_info_path = os.path.join(self._meta_area,
                                                              DirAndFileNameConst.LAST_LOG_BACKUP_STOP_WAL_INFO)
            last_log_backup_stop_wal_info = {
                "last_stop_wal": self._last_stop_wal
            }
            output_execution_result_ex(last_log_backup_stop_wal_info_path, last_log_backup_stop_wal_info)
        LOGGER.info(f"Succeed to exec backup sub task, job id: {self._job_id}")
        return True

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
