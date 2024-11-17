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
# coding: utf-8
import grp
import json
import os
import pathlib
import pwd
import re
import shutil
import stat
import time
import uuid
from pathlib import Path

import psutil

from common.common import execute_cmd_oversize_return_value, convert_time_to_timestamp, convert_timestamp_to_time, \
    exter_attack, output_result_file, execute_cmd
from common.common_models import ActionResult, SubJobDetails, LogDetail
from common.const import CopyDataTypeEnum, DeployType, ReportDBLabel, DBLogLevel, IPConstant, \
    BackupJobResult, BackupTypeEnum, ExecuteResultEnum, RepositoryDataTypeEnum, SubJobStatusEnum, ParamConstant, \
    CMDResult
from common.logger import Logger
from common.number_const import NumberConst
from common.util import check_user_utils
from common.util.backup import backup, backup_files, query_progress
from common.util.exec_utils import exec_overwrite_file
from common.util.exec_utils import su_exec_cmd_list, ExecFuncParam
from common.util.scanner_utils import scan_dir_size
from common.util.validators import ValidatorEnum
from kingbase.common.const import CmdRetCode, BodyErrCode, KbConst, BackupStatus, DirAndFileNameConst, \
    BackupProgressEnum, DatabaseMode, BackupType
from kingbase.common.error_code import ErrorCode as ErrCode
from kingbase.common.kb_exec_sql import ExecKbSql
from kingbase.common.models import BackupJobPermission, BackupProgressInfo
from kingbase.common.util import resource_util
from kingbase.common.util.resource_util import check_special_character, check_black_list, \
    check_is_path_exists, check_white_list, convert_path_to_realpath, is_wal_file, \
    create_soft_link, get_parallel_process, get_db_version_id_and_system_id, \
    delete_soft_link, get_sys_rman_configuration_item, extract_ip, get_current_repo_host, get_database_timeline

LOGGER = Logger().get_logger("kingbase.log")


class KbBackup(object):
    """
    KingBase备份操作类
    """

    def __init__(self, params):
        self._command = params.get("command")
        self._pid, self._job_id, self._sub_job_id = params.get("pid"), params.get("job_id"), params.get("sub_job_id")
        self._param_dict = params.get("param_dict")
        self._app_env_dict, self._job_dict = self._param_dict.get("appEnv", {}), self._param_dict.get("job", {})
        self._protect_env = self._job_dict.get("protectEnv", {})
        self._deploy_type = self._job_dict.get("protectEnv", {}).get("extendInfo", {}).get("deployType", "-1")
        self._protect_object = self._job_dict.get("protectObject", {})
        self._cluster_name = self._protect_object.get("name", "")
        self.archive_dir, self._db_system_id, self._db_version_id = "", None, "12-1"
        if self._command == "QueryJobPermission":
            job_permission_params = ExecKbSql.get_query_job_permission_params(self._param_dict)
            self._os_user_name = job_permission_params.get("os_user_name", "")
            self._install_path = convert_path_to_realpath(job_permission_params.get("install_path", ""))
            self._data_path = job_permission_params.get("data_path", "")
            self._service_ip = job_permission_params.get("service_ip", "")
            self._host_port = job_permission_params.get("host_port", "54321")
        else:
            if self._deploy_type == str(DeployType.SINGLE_TYPE.value):
                protect_object_extend_info = self._job_dict.get("protectObject", {}).get("extendInfo", {})
                self._os_user_name = protect_object_extend_info.get("osUsername", "")
                self._install_path = convert_path_to_realpath(protect_object_extend_info.get("clientPath", ""))
                self._data_path = protect_object_extend_info.get("dataDirectory", "")
                self._service_ip = protect_object_extend_info.get("serviceIp", "")
                self._host_port = protect_object_extend_info.get("instancePort", "54321")
            elif self._deploy_type == str(DeployType.CLUSTER_TYPE.value):
                cluster_params = ExecKbSql.get_cluster_params(self._param_dict)
                self._os_user_name = cluster_params.get("os_user_name", "")
                self._install_path = convert_path_to_realpath(cluster_params.get("install_path", ""))
                self._data_path = cluster_params.get("data_path", "")
                self._service_ip = cluster_params.get("service_ip", "")
                self._host_port = cluster_params.get("host_port", "54321")
            else:
                self._os_user_name, self._install_path, self._data_path, self._service_ip = ("", "", "", "")
                self._host_port = "54321"
        kb_sql_params = (self._pid, self._install_path, self._service_ip, self._host_port, self._deploy_type)
        self._archive_info_file_path = None
        self._repo_path = None
        self._kb_sql = ExecKbSql(kb_sql_params)
        self._backup_type = self._job_dict.get("jobParam", {}).get("backupType", -1)
        repositories_info = self.parse_repositories_path(self._job_dict.get("repositories", []))
        self._repositories = self._job_dict.get("repositories", [])
        self._data_area = convert_path_to_realpath(repositories_info.get("data_repository", [""])[0])
        self._log_area = convert_path_to_realpath(repositories_info.get("log_repository", [""])[0])
        self._meta_area = convert_path_to_realpath(repositories_info.get("meta_repository", [""])[0])
        self._cache_area = convert_path_to_realpath(repositories_info.get("cache_repository", [""])[0])
        self._calc_progress_thread_flag = True
        self._output = ActionResult(code=ExecuteResultEnum.SUCCESS.value)
        self._wal_file_target_path = None

    @staticmethod
    def parse_repositories_path(repositories):
        """
        解析仓库路径
        :param repositories: list，仓库信息列表
        :return: dict，仓库路径
        """
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
    def is_host_address(endpoint):
        """
        检查是否为ip格式
        :return: boolean，True代表是，False代表不是
        """
        pat = re.compile('^(1\d{2}|2[0-4]\d|25[0-5]|[1-9]\d|[1-9])\.(1\d{2}|2[0-4]\d|25[0-5]|[1-9]\d|\d)\.'
                         '(1\d{2}|2[0-4]\d|25[0-5]|[1-9]\d|\d)\.(1\d{2}|2[0-4]\d|25[0-5]|[1-9]\d|\d)$')
        if pat.match(endpoint):
            return True
        else:
            return False

    @staticmethod
    def clear_mount_path(mount_path):
        """
        清理挂载目录
        :param mount_path:
        """
        check_white_list([mount_path])
        for path in os.listdir(mount_path):
            new_path = os.path.join(mount_path, path)
            if '.snapshot' in new_path or re.match(r"^[0-9]{19}$", path):
                continue
            if os.path.isfile(new_path):
                LOGGER.info(f"Remove file:{new_path}")
                os.remove(new_path)
            elif os.path.isdir(new_path):
                LOGGER.info(f"Remove dir:{new_path}")
                shutil.rmtree(new_path, ignore_errors=True)

    @staticmethod
    def check_path_is_link(path):
        """
        路径是否软连接
        :return: boolean，True代表是软连接，False代表不是软连接
        """
        check_white_list([path])
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
    def get_local_ips():
        """
        获取当前主机所有ip
        :return: list，主机ip
        """
        LOGGER.info(f"Start getting all local ips ...")
        local_ips = []
        ip_dict = None
        try:
            ip_dict = psutil.net_if_addrs()
        except Exception as err:
            LOGGER.error(f"Get ip address err: {err}.")
        for _, value in ip_dict.items():
            for ips in value:
                if ips[0] == KbConst.ADDRESS_FAMILY_AF_INET and ips[1] != IPConstant.LOCAL_HOST:
                    local_ips.append(ips[1])
        LOGGER.info(f"Get all local ips: {local_ips} success.")
        return local_ips

    @staticmethod
    def get_table_space_dir_permission(dir_path):
        # 获取table_space目录的所属用户
        stat_info = os.stat(dir_path)
        uid = stat_info.st_uid
        user = pwd.getpwuid(uid)[0]
        LOGGER.info(f"Get table space permission success!table_space:{dir_path}, user:{user}")
        return user

    @staticmethod
    def exec_rc_tool_cmd(cmd, in_path, out_path):
        cmd = f"{os.path.join(ParamConstant.BIN_PATH, 'rpctool.sh')} {cmd} {in_path} {out_path}"
        ret, out, err = execute_cmd(cmd)
        if ret != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error(f"An error occur in execute cmd. ret:{ret} err:{err}")
            return False
        return True

    def check_database_status(self):
        """
        数据库状态是否开启
        :return: boolean，True代表开启，False代表未开启
        """
        kb_ctl_path = os.path.join(self._install_path, "bin", "sys_ctl")
        check_is_path_exists(kb_ctl_path)
        if not resource_util.check_os_name(self._os_user_name, kb_ctl_path):
            return False
        if not check_user_utils.check_path_owner(self._data_path, [self._os_user_name]):
            LOGGER.error(f"Os user name and data dir is not matching, data dir :{self._data_path}!")
            return False
        cmd = f"su - {self._os_user_name} -c '{kb_ctl_path} status -D {self._data_path}'"
        return_code, std_out, std_err = execute_cmd(cmd)
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error(f"Failed to exec cmd: {cmd}, return code: {return_code}, "
                         f"standard error: {std_err}, job id: {self._job_id}.")
            return False
        for info in std_out.split('\n'):
            if "server is running" in info or "正在运行服务器进程" in info:
                LOGGER.info(f"Database server is running, job id: {self._job_id}.")
                return True
        LOGGER.error(f"Succeed to exec cmd, but database server is not running, job id: {self._job_id}.")
        return False

    def query_archive_mode(self):
        """
        归档模式是否开启
        :return: boolean，True代表开启，False代表未开启
        """
        sql_cmd = "show archive_mode;"
        return_code, std_out, std_err = self._kb_sql.exec_sql_cmd(self._os_user_name, sql_cmd)
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error(f"Failed to exec sql cmd: {sql_cmd}, return code: {return_code}, "
                         f"standard error: {std_err}, job id: {self._job_id}.")
            return False, BodyErrCode.PLUGIN_CANNOT_BACKUP_ERR.value
        if "archive_mode" not in std_out:
            return False, BodyErrCode.PLUGIN_CANNOT_BACKUP_ERR.value
        archive_mode = self._kb_sql.parse_sql_result(std_out, "archive_mode")
        LOGGER.info(f"Archive mode is {archive_mode}, job id: {self._job_id}.")
        return archive_mode == "on", BodyErrCode.ARCHIVE_MODE_ENABLED.value

    def hostname_to_ipaddress(self, host_name):
        """
        主机名转ip，如果已经是ip则直接返回
        :param host_name: str，主机名
        :return: str，节点ip
        """
        if self.is_host_address(host_name):
            return host_name
        cmd = "cat /etc/hosts"
        return_code, std_out, std_err = execute_cmd(cmd)
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error(f"Failed to exec cmd: {cmd}, return code: {return_code}, "
                         f"standard error: {std_err}, job id: {self._job_id}.")
            return ""
        for info in std_out.split('\n'):
            if host_name == info.split()[1]:
                return info.split()[0]
        return ""

    def get_cluster_nodes_info(self):
        """
        获取集群节点信息
        :return: dict，集群节点信息
        """
        nodes_dict = dict()
        kb_repmgr_path = os.path.join(self._install_path, "bin", "repmgr")
        check_is_path_exists(kb_repmgr_path)
        if not check_user_utils.check_path_owner(kb_repmgr_path, [self._os_user_name]):
            LOGGER.error("Repmgr path and os username is not matching.")
            return nodes_dict
        cmd = f"su - {self._os_user_name} -c '{kb_repmgr_path} cluster show'"
        return_code, std_out, std_err = execute_cmd(cmd)
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error(f"Failed to exec cmd: {cmd}, return code: {return_code}, "
                         f"standard error: {std_err}, job id: {self._job_id}.")
            return dict()
        lines = std_out.strip().split('\n')
        for line in lines[2:]:
            cells = line.split("|")
            role = cells[2].strip()
            status = cells[3].strip()
            conn_str = cells[-1]
            if "host" not in conn_str:
                LOGGER.error(f"No connection string for finding host info, job id: {self._job_id}.")
                return dict()
            host_name = conn_str.strip().split()[0][5:]
            node_ip = self.hostname_to_ipaddress(host_name)
            if not node_ip:
                LOGGER.error(f"Invalid hostname, job id: {self._job_id}.")
                return dict()
            nodes_dict[node_ip] = (role, status)
        return nodes_dict

    def check_node_is_primary(self):
        """
        查询是否主节点
        :return: boolean，True代表主节点(单机默认是主节点)，False代表非主节点
        """
        if int(self._deploy_type) == DeployType.CLUSTER_TYPE.value:
            # 集群
            nodes_dict = self.get_cluster_nodes_info()
            if not nodes_dict:
                LOGGER.error(f"Failed to get cluster nodes info, job id: {self._job_id}.")
                return False
            local_ips = self.get_local_ips()
            node_list = [node for node in nodes_dict if node in local_ips]
            if nodes_dict.get(node_list[0])[0] != "primary":
                LOGGER.error(f"Current node is not primary, job id: {self._job_id}.")
                return False
            if nodes_dict.get(node_list[0])[1] != "* running":
                LOGGER.error(f"Current node status is abnormal, job id: {self._job_id}.")
                return False
            LOGGER.info(f"Current node is primary and running, job id: {self._job_id}.")
            return True
        elif int(self._deploy_type) == DeployType.SINGLE_TYPE:
            # 单机
            LOGGER.info(f"Current node is deployed by single, job id: {self._job_id}.")
            return True
        else:
            LOGGER.error(f"Invalid deploy type, job id: {self._job_id}.")
            return False

    @exter_attack
    def allow_backup_in_local_node(self):
        """
        当前节点是否允许备份
        :return: boolean，True代表允许备份，False代表不允许备份
        """
        # 查询是否开启数据库
        check_special_character([self._job_id, self._os_user_name, self._install_path, self._data_path,
                                 self._service_ip, self._host_port])
        check_black_list([self._install_path, self._data_path])
        check_is_path_exists(self._data_path)
        if not self.check_database_status():
            LOGGER.error(f"Current node can not backup, database system is shut down, job id: {self._job_id}.")
            self.set_action_result(ExecuteResultEnum.INTERNAL_ERROR.value,
                                   BodyErrCode.PLUGIN_CANNOT_BACKUP_ERR.value,
                                   "Database system is shut down.")
            return False
        # 查询是否开启归档模式
        res, code = self.query_archive_mode()
        if not res:
            LOGGER.error(f"Current node can not backup, archive mode is off, job id: {self._job_id}.")
            self.set_action_result(ExecuteResultEnum.INTERNAL_ERROR.value, code, "Archive mode is off.")
            return False

        # 查询是否主节点
        if not self.check_node_is_primary():
            LOGGER.error(f"Current node can not backup, current node is not primary, job id: {self._job_id}.")
            self.set_action_result(ExecuteResultEnum.INTERNAL_ERROR.value,
                                   BodyErrCode.PLUGIN_CANNOT_BACKUP_ERR.value,
                                   "Current node is not primary.")
            return False

        # repo节点才能备份
        try:
            repo_ip = get_sys_rman_configuration_item(self._install_path, self._job_id, "_repo_ip")
            host_ips = extract_ip()
            if repo_ip not in host_ips:
                LOGGER.error(f"Current node can not backup, current node is not repo node, job id: {self._job_id}.")
                return False
        except Exception as ex:
            LOGGER.error(f"Current node can not backup, current node is not repo node, job id: {self._job_id}.ex:{ex}")
            self.set_action_result(ExecuteResultEnum.INTERNAL_ERROR.value,
                                   BodyErrCode.PLUGIN_CANNOT_BACKUP_ERR.value,
                                   "Current node is not primary.")
            return False
        return True

    @exter_attack
    def query_job_permission(self):
        """
        查询job权限
        :return: boolean，True代表查询成功，False代表查询失败
        """
        if not self._os_user_name:
            LOGGER.error(f"Failed to get user name, job id: {self._job_id}.")
            return False
        group_id = pwd.getpwnam(str(self._os_user_name)).pw_gid
        user_group = grp.getgrgid(group_id).gr_name
        # kingbase的data目录权限必须是700，否则会报错
        output = BackupJobPermission(user=self._os_user_name, group=user_group, fileMode="0700")
        output_result_file(self._pid, output.dict(by_alias=True))
        return True

    @exter_attack
    def check_backup_job_type(self):
        """
        检查job类型
        """

        # 当此次任务是增量量备份，且之前没做过全量备份，需要增量转全量
        def check_last_copy_is_null(backup_type):
            # 读取last_copy_info
            last_copy_type = [
                CopyDataTypeEnum.FULL_COPY.value,
                CopyDataTypeEnum.INCREMENT_COPY.value
            ]
            last_copy_id_path = 'last_increment_copy_id'
            if backup_type == BackupTypeEnum.LOG_BACKUP:
                last_copy_type = [
                    CopyDataTypeEnum.FULL_COPY.value,
                    CopyDataTypeEnum.INCREMENT_COPY.value,
                    CopyDataTypeEnum.LOG_COPY.value
                ]
                last_copy_id_path = 'last_copy_id'
            ret, pre_copy_info = self.get_last_copy_info(last_copy_type)
            if ret and pre_copy_info:
                # 读取cache仓中记录的上一次副本ID,
                # 如果此次是增量备份，检测上次增量备份副本是否存在；
                # 如果此次是日志备份，检测上次任意任意副本是否存在:
                cache_path_parent = Path(self._cache_area).parent
                last_copy_id_file = os.path.join(cache_path_parent, last_copy_id_path)
                with open(last_copy_id_file, "r", encoding='utf-8') as copy_info:
                    last_copy_id = copy_info.read().strip()

                # 从rpc接口中查询上一次副本ID
                last_copy_info_id = pre_copy_info.get("id", "")
                LOGGER.info(f"get last_copy_info_id {last_copy_info_id}, last_copy_id {last_copy_id}")

                # 判断上一次副本是否被删除, 如果被删除，转全量备份
                if last_copy_info_id == last_copy_id:
                    return False
                else:
                    LOGGER.warn("last_copy_info_id is different from last_copy_id")
                    return True
            else:
                LOGGER.warn("last_copy_info is empty")
                return True

        LOGGER.info(f"Start to check backup job type:{self._backup_type}")
        if not self._backup_type:
            return False
        elif self._backup_type == BackupTypeEnum.FULL_BACKUP:
            return True
        if self.check_timeline_change() or check_last_copy_is_null(self._backup_type):
            self.set_action_result(ExecuteResultEnum.INTERNAL_ERROR.value,
                                   BodyErrCode.ERROR_INCREMENT_TO_FULL.value,
                                   f"Can not apply this type backup job")
            LOGGER.info(f"Change backup_type to full")
            return False
        # 成功没写记录
        LOGGER.info(f'Finish execute check_backup_job_type, pid: {self._pid}, job_id:{self._job_id}')
        return True

    def check_timeline_change(self):
        """
        pitr恢复后执行了promote操作（select pg_wal_replay_resume()），或者备节点提升为主节点场景下，会产生新时间线，日志不连续需要转全备
        """
        # 获取上次备份时间线
        cache_path_parent = Path(self._cache_area).parent
        pre_timeline_file = os.path.join(cache_path_parent, 'timeline')
        if not os.path.exists(pre_timeline_file):
            LOGGER.info(f"Not find timeline file, job id: {self._job_id}.")
            return False
        with open(pre_timeline_file, "r", encoding='utf-8') as copy_info:
            pre_timeline = copy_info.read().strip()
        # 获取本次备份时间线
        current_timeline = get_database_timeline(self._install_path, self._data_path, self._os_user_name, self._job_id)
        LOGGER.info(f"get current_timeline {current_timeline}, pre_timeline {pre_timeline}, job id: {self._job_id}.")

        if current_timeline and current_timeline != pre_timeline:
            LOGGER.warn("The current_timeline is different from pre_timeline, job id: {self._job_id}.")
            return True
        return False

    def save_last_id(self):
        # 记录上次备份时间线
        cache_path_parent = Path(self._cache_area).parent
        timeline_path = 'timeline'
        last_timeline_file = os.path.join(cache_path_parent, timeline_path)
        current_timeline = get_database_timeline(self._install_path, self._data_path, self._os_user_name, self._job_id)
        exec_overwrite_file(last_timeline_file, current_timeline, json_flag=False)
        LOGGER.info(f"Save timeline:{current_timeline}")

        # 记录上一次copy_id
        # 记录全备、增备、日志备份 副本ID
        last_copy_id_path = 'last_copy_id'
        last_copy_id_file = os.path.join(cache_path_parent, last_copy_id_path)
        exec_overwrite_file(last_copy_id_file, self._job_id, json_flag=False)
        LOGGER.info(f"Save copy id:{self._job_id}")
        if self._backup_type == BackupTypeEnum.LOG_BACKUP.value:
            return

        # 记录全备、增备副本ID
        last_increment_copy_id_path = 'last_increment_copy_id'
        last_increment_copy_id_file = os.path.join(cache_path_parent, last_increment_copy_id_path)
        exec_overwrite_file(last_increment_copy_id_file, self._job_id, json_flag=False)
        LOGGER.info(f"Save increment copy id:{self._job_id}")

    @exter_attack
    def backup_prerequisite(self):
        """
        备份前置任务
        :return: boolean，True代表备份前置任务成功，False代表备份前置任务失败
        """
        self.write_backup_info_file(BackupProgressEnum.PRE_TASK_PROGRESS.value)
        if not self._os_user_name:
            LOGGER.error(f"Failed to get user name, job id: {self._job_id}.")
            return False
        if not resource_util.check_os_name(self._os_user_name, self._install_path):
            LOGGER.error(f"Backup prerequisite failed.")
            return False
        # get database user id
        user_id = pwd.getpwnam(self._os_user_name).pw_uid
        dir_path = self._data_area if (self._backup_type == BackupTypeEnum.FULL_BACKUP.value
                                       or self._backup_type == BackupTypeEnum.INCRE_BACKUP.value) else self._log_area
        if not os.path.exists(dir_path):
            LOGGER.error(f"The path area is not exist, job id: {self._job_id}.")
            self.set_backup_result(SubJobStatusEnum.FAILED.value, ErrCode.BACKUP_FAILED, "Data dir is not exist.",
                                   BackupProgressEnum.PRE_TASK_PROGRESS.value)
            return False
        check_white_list([dir_path, self._cache_area])
        stat_info = os.stat(dir_path)
        if stat_info.st_uid != user_id:
            self.set_backup_result(SubJobStatusEnum.FAILED.value, ErrCode.BACKUP_FAILED, "Dir permission is incorrect.",
                                   BackupProgressEnum.PRE_TASK_PROGRESS.value)
            LOGGER.error(f"Dir permission is incorrect, job id: {self._job_id}.")
            return False
        self.set_backup_result(SubJobStatusEnum.COMPLETED.value, 0, "",
                               BackupProgressEnum.PRE_TASK_PROGRESS.value)
        return True

    def check_cluster_role_is_not_switch(self):
        ret, pre_full_copy_info = self.get_last_copy_info([CopyDataTypeEnum.FULL_COPY.value])
        if not ret or not pre_full_copy_info:
            LOGGER.error("Find pre full copy failed!")
            self.set_backup_result(SubJobStatusEnum.FAILED.value,
                                   BodyErrCode.NOT_EXIT_WAL_BACKUP_FILE_AND_SNAPSHOT_BACKUP.value,
                                   "Failed to get the lastest full copy.", BackupProgressEnum.PRE_TASK_PROGRESS.value)
            return False
        role_in_copy = pre_full_copy_info.get("extendInfo", {}).get("role")
        role_in_instance = self.get_cluster_role_from_instance()
        if role_in_copy != role_in_instance:
            LOGGER.error(
                f"Current cluster role info has changed!Role in copy :{role_in_copy}, "
                f"role in instance :{role_in_instance}")
            self.set_backup_result(SubJobStatusEnum.FAILED.value,
                                   ErrCode.CLUSTER_ROLE_INFO_HAS_CHANGED,
                                   "Cluster role info has changed.", BackupProgressEnum.PRE_TASK_PROGRESS.value)
            return False
        return True

    def query_progress_common(self, progress_type):
        """
        查询进度100%
        :param progress_type: str，进度文件名
        :return: tuple，（进度值100，job状态）
        """
        file_path = os.path.join(self._cache_area, progress_type)
        progress = KbConst.PROGRESS_ONE_HUNDRED
        if not os.path.exists(file_path):
            LOGGER.error(f"Failed to query progress, job id: {self._job_id}.")
            job_status = SubJobStatusEnum.FAILED.value
        else:
            LOGGER.info(f"Succeed to query progress, job id: {self._job_id}.")
            job_status = SubJobStatusEnum.COMPLETED.value
        return progress, job_status

    @exter_attack
    def query_prerequisite_progress(self):
        """
        查询备份前置任务进度
        :return: boolean，True代表查询备份前置任务进度成功，False代表查询备份前置任务进度失败
        """
        file_path = os.path.join(self._cache_area, "BackupPrerequisiteProgress")
        try:
            progress_info = self.read_tmp_json_file(file_path)
        except Exception as ex:
            LOGGER.error(f"Failed to read tmp json file, job id: {self._job_id}.Excepetion for: {ex}")
            return False
        LOGGER.info(f"Progress info: {progress_info}, job id: {self._job_id}")
        status = progress_info.get("status")
        progress = 50
        if status == SubJobStatusEnum.COMPLETED.value:
            progress = KbConst.PROGRESS_ONE_HUNDRED
        elif status == SubJobStatusEnum.FAILED.value:
            error_code = progress_info.get("error_code", ErrCode.BACKUP_FAILED)
            log_detail = LogDetail(logInfo="", LogInfoParam=[self._job_id],
                                   logLevel=DBLogLevel.ERROR, logDetail=error_code)
            output = SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, progress=progress,
                                   taskStatus=status, logDetail=[log_detail])
            output_result_file(self._pid, output.dict(by_alias=True))
            LOGGER.info(f"Prerequisite is failed: {output.dict(by_alias=True)}, job id: {self._job_id}.")
            return True
        output = SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, progress=progress,
                               taskStatus=status)
        output_result_file(self._pid, output.dict(by_alias=True))
        LOGGER.info(f"SubJobDetails: {output.dict(by_alias=True)}, job id: {self._job_id}.")
        return True

    def get_cluster_role_from_instance(self):
        role = {"primary": "", "standby": []}
        nodes_dict = self.get_cluster_nodes_info()
        if not nodes_dict:
            LOGGER.error(f"Failed to get cluster nodes info, job id: {self._job_id}.")
            return {}
        for host_ip, node_info in nodes_dict.items():
            if node_info[0] == "primary":
                role["primary"] = host_ip
            if node_info[0] == "standby":
                role["standby"].append(host_ip)
        LOGGER.info(f"Success to get cluster role info： role:{role}, job id: {self._job_id}.")
        return role

    def get_cluster_primary_ip(self):
        """
        获取集群主节点ip
        :return: str，集群主节点ip
        """
        nodes_dict = self.get_cluster_nodes_info()
        if not nodes_dict:
            LOGGER.error(f"Failed to get cluster nodes info, job id: {self._job_id}.")
            return False, ""
        for host_ip, node_info in nodes_dict.items():
            if node_info[0] == "primary":
                return True, host_ip
        LOGGER.error(f"Failed to get cluster primary ip, job id: {self._job_id}.")
        return False, ""

    def exec_start_backup(self):
        """
        开始备份
        :return: boolean，True代表开始备份成功，False代表开始备份失败
        """
        database_mode = self.get_database_mode()
        sql_cmd = f"select sys_start_backup('\\'{self._job_id}\\'', false, true);"
        sql_parse = "sys_start_backup"
        if database_mode == DatabaseMode.PG:
            sql_parse = "pg_start_backup"
            sql_cmd = f"select pg_start_backup('\\'{self._job_id}\\'', false, true);"
        return_code, std_out, std_err = self._kb_sql.exec_sql_cmd(self._os_user_name, sql_cmd,
                                                                  timeout=KbConst.CHECK_POINT_TIME_OUT)
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error(f"Failed to exec start backup cmd: {sql_cmd}, return code: {return_code}, "
                         f"standard error: {std_err}, job id: {self._job_id}.")
            return False, ""
        try:
            kb_start_file = self._kb_sql.parse_sql_result(std_out, sql_parse)
        except Exception as ex:
            LOGGER.exception(f"Can not get the first backup file, std_out: {std_out}.")
            raise Exception("The first backup file not exist.") from ex
        LOGGER.info(f"Succeed to exec start backup cmd, job id: {self._job_id}.")
        # 命令执行结果拿到lsn(类似0 / D000028)，用于筛选备份本次备份时间范围内日志
        return True, kb_start_file

    def get_database_mode(self):
        sql_cmd = f"show database_mode;"
        return_code, std_out, std_err = self._kb_sql.exec_sql_cmd(self._os_user_name, sql_cmd)
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error(f"Failed to exec database mode cmd: {sql_cmd}, return code: {return_code}, "
                         f"standard error: {std_err}, job id: {self._job_id}.")
            return False
        LOGGER.info(f"Succeed to exec show database mode cmd, job id: {self._job_id}.")
        database_mode = self._kb_sql.parse_sql_result(std_out, "database_mode")
        LOGGER.info(f"DB mode is {database_mode}, job id: {self._job_id}.")
        return database_mode

    def switch_wal(self):
        """
        手动切换一次归档
        :return: boolean，True代表切换成功，False代表切换失败
        """
        sql_cmd = f"select sys_switch_wal();"
        return_code, std_out, std_err = self._kb_sql.exec_sql_cmd(self._os_user_name, sql_cmd)
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error(f"Failed to exec switch wal cmd: {sql_cmd}, return code: {return_code}, "
                         f"standard error: {std_err}, job id: {self._job_id}.")
            return False
        LOGGER.info(f"Succeed to exec switch wal cmd, job id: {self._job_id}.")
        return True

    def backup_files(self, source, target):
        """
        cp执行文件备份
        :return: boolean，True代表备份成功，False代表备份失败
        """
        if not source or not target:
            LOGGER.error(f"Param error, job id: {self._job_id}.")
            return False
        check_special_character([target])
        # 此处source参数请勿使用os.path.realpath标准化处理
        cmd = f"su - {self._os_user_name} -c \"cp -rp {source} {target}\""
        return_code, std_out, std_err = execute_cmd(cmd)
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error(f"Failed to exec save information to cache cmd, return code: {return_code}, "
                         f"standard error: {std_err}, job id: {self._job_id}.")
            return False
        return True

    def get_last_backup_time_by_sysrman(self, sys_rman_path, sys_rman_conf_path):
        sql_cmd = f'{sys_rman_path} --config={sys_rman_conf_path} --stanza=kingbase info'
        return_code, out, std_err = execute_cmd_oversize_return_value(sql_cmd)
        if return_code != CmdRetCode.EXEC_SUCCESS.value or "error" in out:
            LOGGER.error(f"Get latest backup info failed, return code: {return_code}, "
                         f"out: {out}, err: {std_err}.")
            return 0
        stop_timestamp = 0
        for line in out.splitlines():
            if re.match(r".*timestamp start.*", str(line)):
                stop_timestamp = convert_time_to_timestamp(str(line).split("/")[2].strip())
        return stop_timestamp

    def full_or_increase_backup_by_sys_rman(self):
        """
        使用sys_rman工具全量或增量备份
        :return: boolean，True代表成功，False代表失败
        """
        # 检查是否是第一次用sys_rman备份
        sys_rman_conf_path = self.check_whether_the_backup_is_the_first_backup_by_sys_rman()
        if not sys_rman_conf_path:
            return False

        # 如果上次备份时间大于当前时间，只能修改主机时间
        sys_rman_path = os.path.join(self._install_path, KbConst.BIN_DIR_NAME, KbConst.SYS_RMAN_NAME)
        last_backup_timestamp = self.get_last_backup_time_by_sysrman(sys_rman_path, sys_rman_conf_path)
        LOGGER.info(f"Last backup timestamp:{last_backup_timestamp}, job id: {self._job_id}.")
        if int(time.time()) < last_backup_timestamp:
            LOGGER.info(f"Current timestamp is earlier than last backup timestamp, job id: {self._job_id}.")
            return False

        # 组装备份命令：全量备份参数full,增量备份参数incr
        if self._backup_type == BackupTypeEnum.FULL_BACKUP.value:
            backup_type = BackupType.FULL
        elif self._backup_type == BackupTypeEnum.INCRE_BACKUP.value:
            backup_type = BackupType.INCR
        else:
            LOGGER.error(f"Backup type:{self._backup_type} is error, job id: {self._job_id}.")
            return False

        # 执行备份
        parallel_process = get_parallel_process(self._job_dict)
        LOGGER.info(f"Begin to to execute the backup_command, job id: {self._job_id}.")
        param = ExecFuncParam(
            os_user=f"{self._os_user_name}",
            cmd_list=[
                "{sys_rman_path} --config={sys_rman_conf_path} --stanza=kingbase --archive-copy "
                f"--process-max={parallel_process} --type={backup_type} backup"],
            fmt_params_list=[[
                ("sys_rman_path", sys_rman_path, ValidatorEnum.PATH_CHK_FILE),
                ("sys_rman_conf_path", sys_rman_conf_path, ValidatorEnum.PATH_CHK_FILE)
            ]],
            shell_file="/bin/sh", chk_exe_owner=False)
        try:
            result, out = su_exec_cmd_list(param)
        except Exception as err:
            LOGGER.error(f"exec cmd with kingbase env new failed, error: {err}")
            return False
        if result != CMDResult.SUCCESS:
            LOGGER.error(f"Failed to execute the backup_command, out:{out}, jobId: {self._job_id}.")
            return False
        LOGGER.info(f"Succeed to exec backup cmd, job id: {self._job_id}.")
        return delete_soft_link(os.path.join(self._repo_path, KbConst.BACKUP_DIR_NAME), self._job_id)

    def log_backup_by_sys_rman(self):
        try:
            result, error_code = self.backup_process()
            if not result:
                LOGGER.error(f"Failed to execute backup task. jobId: {self._job_id}")
                self.set_backup_result(SubJobStatusEnum.FAILED.value, error_code,
                                       "Failed to execute backup task.")
                return False
        finally:
            # 停止备份
            result, error_code = self.exec_stop_backup()
        if not result:
            LOGGER.error(f"Failed to exec stop backup cmd. jobId: {self._job_id}")
            self.set_backup_result(SubJobStatusEnum.FAILED.value, error_code,
                                   "Failed to exec stop backup cmd.")
            return False
        # 备份本次备份时间范围内的wal日志
        result = self.backup_wal_files()
        if not result:
            LOGGER.error(f"Failed to backup wal files. jobId: {self._job_id}")
            self.set_backup_result(SubJobStatusEnum.FAILED.value, ErrCode.BACKUP_FAILED,
                                   "Failed to backup wal files.")
            return False
        return True

    def not_exist_backup_config_files(self, sys_rman_conf_path):
        if not os.path.exists(self._repo_path):
            LOGGER.warn(f"The repo directory path:{self._repo_path} not exist, jobId: {self._job_id}.")
            return True
        if not os.path.exists(self._archive_info_file_path):
            LOGGER.warn(f"The archive.info path:{self._archive_info_file_path} not exist, jobId: {self._job_id}.")
            return True
        if not os.path.exists(sys_rman_conf_path):
            LOGGER.warn(f"The sys_rman.conf path:{sys_rman_conf_path} not exist, jobId: {self._job_id}.")
            return True
        LOGGER.info(f"Repo directory, archive.info and sys_rman.conf exist, jobId: {self._job_id}.")
        return False

    def init_sys_rman_before_backup(self, backup_info_file_path):
        sys_rman_conf_path = os.path.join(self._repo_path, KbConst.SYSRMAN_CONF_FILE_NAME)
        host_ips = extract_ip()
        repo_host = get_current_repo_host(sys_rman_conf_path, self._job_id)
        LOGGER.info(f"Current repo1-host is {repo_host}, all local ips are {host_ips}")
        if self.not_exist_backup_config_files(sys_rman_conf_path) or (repo_host and repo_host not in host_ips):
            # 1-repo目录或archive.info或者sys_rman.conf不存在的场景下，需要重新初始化
            # 2-主节点不会标记repo1-host，主备切换场景下主节点repo1-host标记的是备节点（非当前ips），需要重新初始化
            self.init_sys_rman_tool()
            self._db_version_id, self._db_system_id = get_db_version_id_and_system_id(self._archive_info_file_path,
                                                                                      self._job_id)
        else:
            self._db_version_id, self._db_system_id = get_db_version_id_and_system_id(self._archive_info_file_path,
                                                                                      self._job_id)
            remote_backup_info_file_path = os.path.join(self._data_area, self._db_system_id, KbConst.BACKUP_DIR_NAME,
                                                        KbConst.KINGBASE_DIR_NAME, KbConst.BACKUP_INFO_FILE_NAME)
            if not os.path.exists(backup_info_file_path) and not os.path.exists(remote_backup_info_file_path):
                # 3-重新接入资源场景下，本地和远端backup.info文件可能都不存在，需要重新初始化
                LOGGER.warn("The backup.info does not exist both locally and remotely, "
                            f"local backup.info path:{backup_info_file_path}, "
                            f"remote backup.info path:{remote_backup_info_file_path}, jobId: {self._job_id}.")
                self.init_sys_rman_tool()

    def init_sys_rman_tool(self):
        """
        初始化sys_rman工具之后才能做备份恢复
        """
        LOGGER.info(f"Begin to init sys_rman tool, job_id:{self._job_id}.")
        if os.path.exists(self._repo_path):
            shutil.rmtree(self._repo_path)
        sys_backup_sh_path = os.path.join(self._install_path, "bin", "sys_backup.sh")
        param = ExecFuncParam(
            os_user=self._os_user_name,
            cmd_list=["{sys_backup_sh_path} init"],
            fmt_params_list=[[("sys_backup_sh_path", sys_backup_sh_path, ValidatorEnum.PATH_CHK_FILE)]],
            shell_file="/bin/sh", chk_exe_owner=False)
        try:
            result, out = su_exec_cmd_list(param)
        except Exception as err:
            LOGGER.error(f"exec init cmd with kingbase env new failed, error: {err}")
            return False
        if result != CMDResult.SUCCESS:
            LOGGER.error(f"Failed to init sys_rman tool, out:{out}, jobId: {self._job_id}.")
            return False
        LOGGER.info(f"Succeed to exec init sys_rman cmd, job id: {self._job_id}.")
        return True

    def check_whether_the_backup_is_the_first_backup_by_sys_rman(self):
        """
        检查是否是第一次用sys_rman备份：第一次需拷贝数据库主机初始化的全量副本和sys_rman.conf文件，且修改权限777
        """
        LOGGER.info("Begin to check whether the backup is the first backup by sys_rman...")
        backup_info_file_path = os.path.join(self._repo_path, KbConst.BACKUP_DIR_NAME, KbConst.KINGBASE_DIR_NAME,
                                             KbConst.BACKUP_INFO_FILE_NAME)
        # 1 是否执行了初始化，初始化才能使用
        self.init_sys_rman_before_backup(backup_info_file_path)

        # 2 本地存在backup.conf文件，第一次备份，先直接移动到远端
        backup_path = os.path.join(self._repo_path, KbConst.BACKUP_DIR_NAME)
        system_id_dir = os.path.join(self._data_area, self._db_system_id)
        if os.path.exists(backup_info_file_path):
            LOGGER.info(f"First backup by sys_rman, self._job_id: {self._job_id}.")
            # 不存在system_id目录就创建
            if not os.path.exists(system_id_dir):
                os.makedirs(system_id_dir)
                LOGGER.info(f"Make {system_id_dir} directory.")
            ret, output, err = execute_cmd(f"cp -rf {backup_path} {system_id_dir}")
            if ret != CmdRetCode.EXEC_SUCCESS.value or "errors" in err:
                LOGGER.error(
                    f"Failed to backup the old copy: {backup_path} generated by sys_rman,err：{err}")
                return ""
            LOGGER.info(
                f"Success to backup the old copy: {self._repo_path} generated by sys_rman, jobId: {self._job_id}.")
        else:
            LOGGER.info(f"Not the first time.")

        # 3 创建backup软链接，指向挂载路径
        real_backup_path = os.path.join(self._data_area, self._db_system_id, KbConst.BACKUP_DIR_NAME)
        if not create_soft_link(real_backup_path, backup_path, self._job_id):
            return ""
        # 4 修改data仓system_id_dir，和本地backup目录属主
        ret, output, err = execute_cmd(
            f"chown -R {self._os_user_name}:{self._os_user_name} {system_id_dir} {backup_path}")
        if ret != CmdRetCode.EXEC_SUCCESS.value or "errors" in err:
            LOGGER.error(f"Failed to change owner: {system_id_dir},err：{err}, jobId: {self._job_id}.")
            return ""
        return os.path.join(self._repo_path, KbConst.SYSRMAN_CONF_FILE_NAME)

    def backup_directory(self, source, target, job_id):
        if not source or not target:
            LOGGER.error(f"Param error, job id: {job_id}.")
            return False
        res = backup(job_id, source, target)
        if not res:
            LOGGER.error(f"Failed to start backup, jobId: {job_id}.")
            return False
        return self.get_backup_status(job_id=job_id)

    def backup_file_list(self, files, target):
        if not files or not target:
            LOGGER.error(f"Param error, job id: {self._job_id}.")
            return False
        res = backup_files(self._job_id, files, target, write_meta=True)
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
                        f"data_size:{data_size}.")
            if status == BackupStatus.COMPLETED:
                LOGGER.info(f"Backup completed, jobId: {job_id}.")
                backup_status = True
                break
            elif status == BackupStatus.RUNNING:
                continue
            elif status == BackupStatus.FAILED:
                LOGGER.error(f"Backup failed, jobId: {job_id}.")
                backup_status = False
                break
            else:
                LOGGER.error(f"Backup failed, status error jobId: {job_id}.")
                backup_status = False
                break
        return backup_status

    def query_log_file_list(self, wal_dir, kb_start_file):
        """
        查询需要备份的日志文件
        :param wal_dir: str，归档日志目录路径
        :return: list，需要备份的日志文件列表
        """
        # 解析副本元数据信息
        copy_meta_file = os.path.realpath(os.path.join(self._meta_area, "LastBackupCopy.info"))
        wal_file = self.get_start_file_name(kb_start_file)
        if wal_file:
            wal_file_time_id = wal_file[:16]
            # 前16位是TimeLineID，备份和当前数据timeline一致的wal
            file_list = list(filter(lambda x: is_wal_file(x) and x[:16] == wal_file_time_id,
                                    os.listdir(wal_dir)))
            #  备份上一次备份开始到本次日志备份开始之间的日志，转换为16进制比较时间大小，越大的说明生成时间越靠后
            wal_file_int = int(wal_file[:16], 16)
            files = [file for file in file_list if int(file[:16], 16) <= wal_file_int]
        else:
            files = [file for file in os.listdir(wal_dir) if is_wal_file(file)]
            LOGGER.info("The first backup file is not exist, ready to backup all wall files.")
        # 获取需要备份的文件
        if not files:
            return files
        if os.path.exists(copy_meta_file):
            check_white_list([copy_meta_file])
            copy_meta_info = self.read_tmp_json_file(copy_meta_file)
            last_wal_file = copy_meta_info.get("wal_file")
            # 查找备份文件
            # 前24位是TimeLineID+日志编号
            last_wal_file_time_id = int(last_wal_file[:24], 16)
            wal_files = [file for file in files if int(file[:24], 16) > last_wal_file_time_id]
            LOGGER.info(f"Filter file list number :{len(wal_files)}")
            return wal_files
        LOGGER.info(f"Filter file list number:{len(files)}")
        return files

    def get_start_file_name(self, kb_start_file):
        key = "pg_walfile_name"
        sql_cmd = f"select pg_walfile_name(\'\\\'{kb_start_file}\\\'\');"
        # 解析pg_wal文件
        return_code, std_out, std_err = self._kb_sql.exec_sql_cmd(self._os_user_name, sql_cmd)
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error(f"Failed to exec start backup cmd: {sql_cmd}, return code: {return_code}, "
                         f"standard error: {std_err}, job id: {self._job_id}.")
            return ""
        wal_file = self._kb_sql.parse_sql_result(std_out, key)
        LOGGER.info(f"Success to get wal file, file name :{wal_file}.")
        return wal_file

    def backup_data(self, kb_start_file):
        """
        备份数据文件
        :return: boolean，True代表备份成功，False代表备份失败
        """
        self.archive_dir = self.query_archive_dir_by_sys_rman()
        check_white_list([self._log_area])
        timeline = self.get_start_file_name(kb_start_file)[:16]
        self._wal_file_target_path = os.path.join(self._log_area, self._db_system_id, KbConst.ARCHIVE_DIR_NAME,
                                                  KbConst.KINGBASE_DIR_NAME, self._db_version_id, timeline)
        LOGGER.info(f"_wal_file_target_path: {self._wal_file_target_path}")
        if not os.path.exists(self._wal_file_target_path):
            os.makedirs(self._wal_file_target_path)
        # 备份自上次全量备份到本次备份之间数据库产生的日志
        if not self.archive_dir:
            LOGGER.error(f"Failed to query archive dir, job id: {self._job_id}.")
            return False, ErrCode.BACKUP_FAILED
        file_list = self.query_log_file_list(self.archive_dir, kb_start_file)
        files = [os.path.join(self.archive_dir, file) for file in file_list]
        for i in range(0, len(files), KbConst.MAX_FILE_NUMBER_OF_LOG_BACKUP):
            result = self.backup_file_list(files[i:i + KbConst.MAX_FILE_NUMBER_OF_LOG_BACKUP],
                                           self._wal_file_target_path)
            if not result:
                LOGGER.error(f"Failed to backup wal file: {len(files)}, job id: {self._job_id}.")
                return False, ErrCode.BACKUP_TOOL_FAILED
        LOGGER.info(f"Succeed to backup wal files, job id: {self._job_id}.")
        return True, 0

    def backup_table_space(self):
        table_space, result = self.get_table_space()
        if not result:
            LOGGER.error(f"Failed to get table space.")
            return False
        if not table_space:
            LOGGER.info("There is no need to backup table space.")
            return True
        tb_info = dict()
        for name, path in table_space.items():
            if not os.path.exists(path):
                LOGGER.error(f"Table space path is not exist!path :{path}")
                return False
            sub_paths = path.split("/")
            sub_paths.pop()
            if len(sub_paths) == 1:
                # 适配表空间目录在根目录的情况
                table_space_path = os.path.join(self._data_area, DirAndFileNameConst.TABLE_SPACE_INFO_DIR)
            else:
                table_space_path = os.path.join(self._data_area, DirAndFileNameConst.TABLE_SPACE_INFO_DIR, *sub_paths)
            LOGGER.info(f"Record the source table path: %s, target table space path: %s.", path, table_space_path)
            job_id = \
                str(uuid.uuid5(uuid.NAMESPACE_X500, self._job_id + DirAndFileNameConst.TABLE_SPACE_INFO_DIR + name))
            LOGGER.info(f"Start backup table space self._job_id: {self._job_id}, backup table space job_id: {job_id}")
            result = self.backup_directory(path, table_space_path, job_id)
            if not result:
                LOGGER.error(f"Failed to backup table space, job id: {self._job_id}, space:{name}.")
                return False
            LOGGER.info(f"Succeed to to backup table space, job id: {self._job_id}, space:{name}.")
            tb_info[name] = (path, self.get_table_space_dir_permission(path))
        # 记录表空间信息
        table_space_path = os.path.join(self._data_area, DirAndFileNameConst.TABLE_SPACE_INFO_DIR,
                                        DirAndFileNameConst.TABLE_SPACE_INFO_FILE)
        self.write_tmp_json_file(tb_info, table_space_path)
        LOGGER.info(f"Write table space info success, tb_info:{tb_info}, job id: {self._job_id}.")
        return True

    def get_table_space(self):
        sql_cmd = "\\db"
        return_code, std_out, std_err = self._kb_sql.exec_sql_cmd(self._os_user_name, sql_cmd, pager_off=True)
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error(f"Failed to get table space, job id: {self._job_id}.")
            return {}, False
        LOGGER.info(f"Get table space!")
        return self._kb_sql.parse_db_sql_result(std_out), True

    def exec_stop_backup(self):
        """
        结束备份
        :return: boolean，True代表结束备份成功，False代表结束备份失败
        """
        database_mode = self.get_database_mode()
        sql_cmd = "select sys_stop_backup();"
        if database_mode == DatabaseMode.PG:
            sql_cmd = f"select pg_stop_backup();"
        return_code, std_out, std_err = self._kb_sql.exec_sql_cmd(self._os_user_name, sql_cmd,
                                                                  timeout=KbConst.STOP_PG_BACKUP_TIME_OUT)
        if return_code == CmdRetCode.CONFIG_ERROR.value:
            return False, ErrCode.ARCHIVE_MODE_CONFIG_ERROR
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error(f"Failed to exec stop backup cmd: {sql_cmd}, return code: {return_code}, "
                         f"standard error: {std_err}, job id: {self._job_id}.")
            return False, ErrCode.BACKUP_FAILED
        LOGGER.info(f"Succeed to exec stop backup cmd, job id: {self._job_id}.")
        return True, CmdRetCode.EXEC_SUCCESS.value

    def query_archive_dir(self):
        """
        查询归档日志目录路径
        :return: str，归档日志目录路径
        """
        sql_cmd = "show archive_command;"
        return_code, std_out, std_err = self._kb_sql.exec_sql_cmd(self._os_user_name, sql_cmd)
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error(f"Failed to exec sql cmd: {sql_cmd}, return code: {return_code}, "
                         f"standard error: {std_err}, job id: {self._job_id}.")
            return ""
        lines = std_out.strip().split('\n')
        for line in lines:
            if "archive_command" in line:
                archive_dir = line.split()[-1][:-3:]
                return archive_dir
        return ""

    def query_archive_dir_by_sys_rman(self):
        """
        查询归档日志目录路径
        :return: str，归档日志目录路径
        """
        version_id_dir = os.path.join(self._repo_path, KbConst.ARCHIVE_DIR_NAME, KbConst.KINGBASE_DIR_NAME,
                                      self._db_version_id)
        timeline_list = []
        for timeline_dir in os.listdir(version_id_dir):
            if os.path.isdir(os.path.join(version_id_dir, timeline_dir)):
                timeline_list.append(os.path.join(version_id_dir, timeline_dir))
        file_list = sorted(timeline_list, key=lambda x: os.path.getmtime(x))
        if file_list:
            return os.path.join(version_id_dir, file_list[-1])
        LOGGER.info(f"Archive dir:{self.archive_dir}")
        return ""

    def get_backup_info_file(self, wal_dir, index=-1):
        """
        获取日志备份信息文件
        :return: str，日志备份信息文件
        """
        check_is_path_exists(wal_dir)
        wal_files = os.listdir(wal_dir)
        file_list = []
        for wal_file in wal_files:
            if resource_util.is_backup_wal_file(wal_file):
                file_list.append(os.path.join(wal_dir, wal_file))

        file_list = sorted(file_list, key=lambda x: os.path.getmtime(x))
        backup_file = file_list[index]
        return backup_file

    def query_wal_file_list(self, wal_dir):
        """
        获取日志备份文件列表
        :param wal_dir: str，数据库归档日志目录路径
        :return: list，日志备份文件列表
        """
        # 查找备份文件
        file_list = []
        backup_file = self.get_backup_info_file(wal_dir)
        if not backup_file:
            LOGGER.error(f"Failed to get backup info file, job id: {self._job_id}.")
            return file_list
        # 解析备份文件
        with open(backup_file, "r") as file:
            lines = file.readlines()
        start_wal = None
        stop_wal = None
        for info in lines:
            if "START WAL" in info:
                start_wal_info = info.split()[-1]
                start_wal = start_wal_info[:-1]
            if "STOP WAL" in info:
                stop_wal_info = info.split()[-1]
                stop_wal = stop_wal_info[:-1]
        file_list = os.listdir(wal_dir)
        file_list = [file for file in file_list if is_wal_file(file)]
        file_list = sorted(file_list, key=lambda x: os.path.getmtime(os.path.join(wal_dir, x)))
        LOGGER.info(f"file_list size: {len(file_list)}")
        if not file_list:
            LOGGER.error(f"Wal file list is empty, job id: {self._job_id}.")
            return file_list
        start = None
        end = None
        for idx, file in enumerate(file_list):
            if file[:24] == start_wal:
                start = idx
            if file[:24] == stop_wal:
                end = idx
        backup_wal_list = file_list[start:end + 1]

        # 若归档目录存在与本次需备份的日志文件TimeLineID相同的".history"文件，需备份
        time_line_id = start_wal[:8]
        for idx, obj in enumerate(file_list):
            if obj.endswith(".history") and obj[:8] == time_line_id:
                backup_wal_list.append(file_list[idx])
        return backup_wal_list

    def backup_wal_files(self):
        """
        备份本次日志
        :return: boolean，True代表备份本次日志成功，False代表备份本次日志失败
        """
        # 查找本次备份wal日志
        file_list = self.query_wal_file_list(self.archive_dir)
        if not file_list:
            LOGGER.error(f"Failed to wal file list, job id: {self._job_id}.")
            return False
        # 备份wal日志，可能存在sys_wal目录下有.history文件，但归档目录没有的情况，对sys_wal不做清理操作
        files = [os.path.join(self.archive_dir, file) for file in file_list]
        result = self.backup_file_list(files, self._wal_file_target_path)
        if not result:
            LOGGER.error(f"Failed to wal file: {len(files)}, job id: {self._job_id}.")
            return False
        LOGGER.info(f"Succeed to backup wal files, job id: {self._job_id}.")
        return True

    def save_backup_info(self):
        """
        保存备份信息文件
        :return: boolean，True代表保存成功，False代表保存失败
        """
        # 查询备份信息
        backup_file = self.get_backup_info_file(self.archive_dir)
        if not backup_file:
            LOGGER.error(f"Failed to get backup info file, job id: {self._job_id}.")
            return False, ErrCode.BACKUP_FAILED
        # 保存备份信息文件到cache仓
        backup_file_path = os.path.join(self.archive_dir, backup_file)
        check_is_path_exists(backup_file_path)
        # 检查生成的日志副本或全量副本是否可用
        res, error_code = self.check_copy_is_available(backup_file, backup_file_path)
        if not res:
            return False, error_code
        meta_file_name = os.path.realpath(os.path.join(self._meta_area, "LastBackupCopy.info"))
        if self._backup_type == BackupTypeEnum.LOG_BACKUP.value and not os.path.exists(meta_file_name):
            # 首次日志备份读取全量备份的结束时间
            res, code = self.save_last_backup_meta(self.archive_dir)
            return res, code
        # 存储上一次copy_id
        self.save_last_id()
        LOGGER.info(f"Succeed to save backup info file, job id: {self._job_id}.")
        return True, NumberConst.ZERO

    def check_copy_is_available(self, backup_file, backup_file_path):
        """
        获取日志备份文件列表
        :param backup_file: str，备份完成后生成的.backup文件
        :param backup_file_path: str，备份完成后生成的.backup文件的路径
        :return: bool, int，副本是否可用的结果，不可用时需要返回错误码
        """
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
        copy_timeline = wal_file[:16] if is_wal_file(wal_file) else ""
        LOGGER.info(f"Report backup copy, stop time: {stop_time}, stop wal: {wal_file}, timeline: {copy_timeline}, "
                    f"pid: {self._pid}, job id: {self._job_id}.")
        copy_dict = self._job_dict.get("copy", [{}])[0]
        copy_dict["timestamp"] = convert_time_to_timestamp(stop_time)
        if self._backup_type == BackupTypeEnum.LOG_BACKUP.value:
            build_ret, extend_info = self.build_log_copy_ext_info(stop_time, wal_file)
            if not build_ret:
                return False, ErrCode.LOG_INCONSISTENT
            copy_dict["extendInfo"] = extend_info
        elif (self._backup_type == BackupTypeEnum.FULL_BACKUP.value
              or self._backup_type == BackupTypeEnum.INCRE_BACKUP.value):
            copy_dict = self.write_full_backup_time(copy_dict, stop_time)
            copy_dict["extendInfo"]["timeline"] = copy_timeline
            copy_dict["extendInfo"]["stopWalFile"] = wal_file
            # 保存当前主备信息
            if self._deploy_type == str(DeployType.CLUSTER_TYPE.value):
                copy_dict["extendInfo"]["role"] = self.get_cluster_role_from_instance()
        else:
            return False, ErrCode.BACKUP_FAILED

        # 标记sys_rman生成新副本
        copy_dict["extendInfo"]["db_system_id"] = self._db_system_id
        copy_dict["extendInfo"]["new_copy"] = KbConst.IS_NEW_COPY
        copy_dict["extendInfo"]["max_connections"] = self.get_max_connection()
        copy_dict["repositories"] = self.get_out_repositories()
        LOGGER.info(f"Backup copy info, job id: {self._job_id}.")
        # 记录副本信息文件
        copy_info = {
            "stop_time": stop_time, "wal_file": wal_file, "backup_file": backup_file, "copy_dict": copy_dict
        }
        LOGGER.info(f"Write copy info :{copy_info}")
        try:
            self.write_tmp_json_file(copy_info, DirAndFileNameConst.COPY_FILE_INFO)
        except Exception as ex:
            LOGGER.error(f"Failed to write tmp json file, job id: {self._job_id}. Exception info: {ex}.")
            return False, ErrCode.BACKUP_FAILED
        return True, NumberConst.ZERO

    def get_out_repositories(self):
        # 目录格式需要填充remotePath，复制从这里获取路径
        repositories = self._job_dict.get("repositories", [])
        out_repositories = []
        for repository in repositories:
            repository_type = repository.get("repositoryType", '')
            if repository_type == RepositoryDataTypeEnum.DATA_REPOSITORY:
                remote_kingbase_path = os.path.join(self._db_system_id,
                                                    KbConst.BACKUP_DIR_NAME, KbConst.KINGBASE_DIR_NAME)
                latest_copy_name = os.path.basename(
                    os.path.realpath(os.path.join(self._data_area, remote_kingbase_path, "latest")))
                LOGGER.info(f"Current backup set: {latest_copy_name}")
                repository["remotePath"] = os.path.join(repository["remotePath"], remote_kingbase_path,
                                                        latest_copy_name)
                out_repositories.append(repository)
                break
        return out_repositories

    def get_max_connection(self):
        if self._deploy_type == str(DeployType.CLUSTER_TYPE.value):
            tgt_config_file = os.path.realpath(os.path.join(self._data_path, KbConst.CLUSTER_CONF_FILE_NAME))
        else:
            tgt_config_file = os.path.realpath(os.path.join(self._data_path, KbConst.KINGBASE_CONF_FILE_NAME))
        if not os.path.exists(tgt_config_file):
            LOGGER.warning(f"File {tgt_config_file} is not exist!")
            return NumberConst.ZERO
        with open(tgt_config_file, 'r') as file:
            lines = file.readlines()
        for line in lines:
            line_content = line.strip()
            if re.match(r"\Amax_connections", line_content):
                max_connections = line_content.split("=")[1].strip()
                if "#" in max_connections:
                    max_connections = max_connections.split("#")[0].strip()
                LOGGER.debug(f"Get max connections from :{tgt_config_file} is {max_connections}")
                return max_connections
        LOGGER.warning(f"Failed to get max connection ,return 0.")
        return NumberConst.ZERO

    def save_last_backup_meta(self, archive_dir):
        LOGGER.info("First LOGGER backup get last backup stop time.")
        last_backup_file = self.get_backup_info_file(archive_dir, index=0)
        if not last_backup_file:
            LOGGER.error(f"First LOGGER backup ,failed to get last backup file, job id: {self._job_id}.")
            return False, ErrCode.BACKUP_FAILED
        last_backup_file_path = os.path.join(archive_dir, last_backup_file)
        stop_time = self.get_last_backup_stop_time(last_backup_file_path)
        copy_meta_info = {
            "copy_id": self._job_id, "timestamp": stop_time, "backup_file": last_backup_file_path
        }
        self.save_copy_meta_info_to_file(copy_meta_info)
        LOGGER.info(f"Success to get and save last backup stop time, stop_time:{stop_time}.")
        return True, NumberConst.ZERO

    def backup_process(self):
        result, kb_start_file = self.exec_start_backup()
        if not result:
            LOGGER.error("Failed to exec start backup cmd.")
            return False, ErrCode.BACKUP_FAILED
        # 备份数据文件
        result, error_code = self.backup_data(kb_start_file)
        if not result:
            LOGGER.error("Failed to backup database files.")
            return False, error_code
        return True, 0

    @exter_attack
    def backup(self):
        """
        备份子任务
        :return: boolean，True代表备份成功，False代表备份失败
        """
        self.write_backup_info_file()
        check_special_character([self._os_user_name, self._install_path])
        check_white_list([self._cache_area, self._meta_area])
        # 备份只能在主节点上执行，集群部署时以主节点ip登录
        if self._deploy_type == str(DeployType.CLUSTER_TYPE.value):
            flag, primary_ip = self.get_cluster_primary_ip()
            if not flag:
                self.set_backup_result(SubJobStatusEnum.FAILED.value, ErrCode.BACKUP_FAILED,
                                       "Failed to get cluster primary ip.")
                return False
            self._service_ip = primary_ip
            kb_sql_params = (self._pid, self._install_path, self._service_ip, self._host_port, self._deploy_type)
            self._kb_sql = ExecKbSql(kb_sql_params)
        self._repo_path = get_sys_rman_configuration_item(self._install_path, self._job_id)
        self._archive_info_file_path = os.path.join(self._repo_path, KbConst.ARCHIVE_DIR_NAME,
                                                    KbConst.KINGBASE_DIR_NAME, KbConst.ARCHIVE_INFO_FILE_NAME)
        if (self._backup_type == BackupTypeEnum.FULL_BACKUP.value
                or self._backup_type == BackupTypeEnum.INCRE_BACKUP.value):
            # 全量/增量备份
            result = self.full_or_increase_backup_by_sys_rman()
            if not result or not self.remove_archive_wal_files_and_copy_archive_info_file():
                self.set_backup_result(SubJobStatusEnum.FAILED.value, ErrCode.BACKUP_FAILED,
                                       "Failed to backup database files by sys_rman.")
                LOGGER.error(f"Failed to backup database files by sys_rman, job id: {self._job_id}.")
                return False
            self.archive_dir = self.query_archive_dir_by_sys_rman()
        elif self._backup_type == BackupTypeEnum.LOG_BACKUP.value:
            self._db_version_id, self._db_system_id = get_db_version_id_and_system_id(self._archive_info_file_path,
                                                                                      self._job_id)
            result = self.log_backup_by_sys_rman()
            if not result or not self.copy_archive_info_file(self._log_area):
                self.set_backup_result(SubJobStatusEnum.FAILED.value, ErrCode.BACKUP_FAILED,
                                       "Failed to copy archive.info.")
                LOGGER.error(f"Failed to copy archive.info, job id: {self._job_id}.")
                return False
        else:
            LOGGER.error(f"Invalid backup type, job id: {self._job_id}.")
            return False

        # 保存备份信息文件
        result, error_code = self.save_backup_info()
        if not result:
            self.set_backup_result(SubJobStatusEnum.FAILED.value, error_code, "Failed to save backup info.")
            return False
        # 备份完成
        self.set_backup_result(SubJobStatusEnum.COMPLETED.value, "", "")
        LOGGER.info(f"Succeed to exec backup sub task, job id: {self._job_id}.")
        return True

    @exter_attack
    def query_backup_progress(self):
        """
        查询备份进度
        :return: boolean，True代表备份成功，False代表备份失败
        """
        check_white_list([self._cache_area])
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
            progress = KbConst.PROGRESS_ONE_HUNDRED
            backup_type = self._job_dict.get("jobParam", {}).get("backupType", 0)
            if (backup_type == BackupTypeEnum.FULL_BACKUP.value
                    or self._backup_type == BackupTypeEnum.INCRE_BACKUP.value):
                data_path = self._data_area
            else:
                data_path = self._log_area
            check_white_list([data_path])
            ret, size = scan_dir_size(self._job_id, data_path)
            LOGGER.info(f"Success get copy size: {size}")
            if ret:
                data_size = int(size)
        else:
            progress = KbConst.PROGRESS_FIFTY
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
    def backup_post_job(self):
        """
        备份后置任务
        :return: boolean，True代表备份后置任务成功，False代表备份后置任务失败
        """
        backup_file = os.path.join(self._cache_area, "BackupProgress")
        if os.path.exists(backup_file):
            backup_result = self.read_tmp_json_file(backup_file)
            if (self._backup_type == BackupTypeEnum.FULL_BACKUP.value
                    or self._backup_type == BackupTypeEnum.INCRE_BACKUP.value):
                backup_path = self._data_area
            elif self._backup_type == BackupTypeEnum.LOG_BACKUP.value:
                backup_path = self._log_area
            else:
                LOGGER.error(f"Invalid backup type, job id: {self._job_id}.")
                self.set_action_result(ExecuteResultEnum.INTERNAL_ERROR.value,
                                       BodyErrCode.PLUGIN_CANNOT_BACKUP_ERR.value,
                                       f"Invalid backup type.")
                return False
            if self._param_dict.get("backupJobResult", BackupJobResult.FAIL) != BackupJobResult.SUCCESS:
                # 备份任务不成功，清理备份数据
                check_white_list([backup_path])
                try:
                    self.clear_mount_path(backup_path)
                except Exception as ex:
                    LOGGER.error(f"Failed to remove data dir, job id: {self._job_id}. "
                                 f"Exception info: {ex}.")
                    self.set_action_result(ExecuteResultEnum.INTERNAL_ERROR.value,
                                           BodyErrCode.PLUGIN_CANNOT_BACKUP_ERR.value,
                                           f"Failed to remove data dir: {self._data_area}.")
                    return False
                LOGGER.info(f"Succeed to clean backup storage, job id: {self._job_id}.")
            if backup_result.get("status") in (SubJobStatusEnum.ABORTED.value, SubJobStatusEnum.ABORTED_FAILED.value):
                # 备份任务终止执行stop命令
                result, error_code = self.exec_stop_backup()
                if not result:
                    self.set_backup_result(SubJobStatusEnum.FAILED.value, error_code, "Failed to exec stop backup cmd.")
                    return False
        if os.path.exists(self._cache_area):
            # 清空cache仓
            check_white_list([self._cache_area])
            try:
                self.clear_mount_path(self._cache_area)
            except Exception as ex:
                LOGGER.error(f"Failed to remove cache dir, job id: {self._job_id}. "
                             f"Exception info: {ex}.")
                self.set_action_result(ExecuteResultEnum.INTERNAL_ERROR.value,
                                       BodyErrCode.PLUGIN_CANNOT_BACKUP_ERR.value,
                                       f"Failed to cache data dir: {self._cache_area}.")
                return False
            LOGGER.info(f"Succeed to remove tmp dir, job id: {self._job_id}.")
        result_file = os.path.join(self._cache_area, "BackupPostJobProgress")
        pathlib.Path(result_file).touch()
        return True

    @exter_attack
    def query_post_job_progress(self):
        """
        查询备份后置任务进度
        :return: boolean，True代表查询备份后置任务进度成功，False代表查询备份后置任务进度失败
        """
        progress, job_status = self.query_progress_common("BackupPostJobProgress")
        output = SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, progress=progress,
                               taskStatus=job_status)
        output_result_file(self._pid, output.dict(by_alias=True))
        LOGGER.info(f"SubJobDetails: {output.dict(by_alias=True)}, job id: {self._job_id}.")
        return True

    def save_copy_meta_info_to_file(self, context):
        """
        保存副本信息
        :param context: dict，副本信息
        """
        file_path = os.path.realpath(os.path.join(self._meta_area, "LastBackupCopy.info"))
        if self.check_path_is_link(file_path):
            LOGGER.error(f"The path meta area is invalid, job id: {self._job_id}.")
            return
        if os.path.exists(file_path):
            try:
                os.remove(file_path)
            except Exception as ex:
                LOGGER.error(f"Failed to remove file file_path, job id: {self._job_id}.")
                raise Exception("remove file failed") from ex
            LOGGER.info(f"Removed copy meta info file_path, job id: {self._job_id}.")
        flags = os.O_WRONLY | os.O_CREAT | os.O_EXCL
        modes = stat.S_IWUSR | stat.S_IRUSR | stat.S_IXUSR
        with os.fdopen(os.open(file_path, flags, modes), 'w') as file:
            file.write(json.dumps(context))
        LOGGER.info(f"Save copy meta info to file, job id: {self._job_id}.")

    @exter_attack
    def query_backup_copy(self):
        """
        查询备份副本
        :return: bool 查询是否成功
        """
        # 查询备份信息
        check_white_list([self._meta_area, self._cache_area])
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
        new_copy = copy_info.get("new_copy")
        db_system_id = copy_info.get("db_system_id")
        max_connections = copy_info.get("max_connections")
        backup_file_path = os.path.join(self._cache_area, backup_file)
        copy_meta_info = {
            "copy_id": self._job_id, "timestamp": stop_time, "wal_file": wal_file,
            "backup_file": backup_file_path, "new_copy": new_copy, "max_connections": max_connections,
            "db_system_id": db_system_id
        }
        self.save_copy_meta_info_to_file(copy_meta_info)
        LOGGER.info(f"Backup copy info, job id: {self._job_id}, copy dict :{copy_dict}.")
        output_result_file(self._pid, copy_dict)
        return True

    def build_log_copy_ext_info(self, stop_time, wal_file):
        extend_info = dict()
        copy_timeline = wal_file[:8] if is_wal_file(wal_file) else ""
        pre_copy_type = CopyDataTypeEnum.LOG_COPY.value
        ret, pre_copy_bak_time, pre_copy_timeline = self.get_pre_copy_bak_time_for_log_backup_from_log_copy()
        # 没有查询到日志副本，继续查询全量副本
        if not ret or not pre_copy_bak_time:
            ret, pre_copy_bak_time, pre_copy_timeline = self.get_pre_copy_bak_time_for_log_backup_from_full_copy()
            pre_copy_type = CopyDataTypeEnum.FULL_COPY.value
            if not ret or not pre_copy_bak_time:
                LOGGER.error(f"Get previous copy backup time for log backup failed, result: {ret}, backup time: "
                             f"{pre_copy_bak_time}, job id: {self._job_id}.")
                return False, extend_info

        # 查询到日志副本检查timeline是否连续
        if pre_copy_timeline and copy_timeline and pre_copy_timeline != copy_timeline:
            # 日志副本timeline不连续，查询全量副本是否连续
            if pre_copy_type == CopyDataTypeEnum.LOG_COPY.value:
                ret, pre_copy_bak_time, pre_copy_timeline = self.get_pre_copy_bak_time_for_log_backup_from_full_copy()
            if not self.check_log_copy_timeline(pre_copy_timeline, copy_timeline):
                return False, extend_info

        extend_info["beginTime"] = pre_copy_bak_time
        stop_timestamp = convert_time_to_timestamp(stop_time)
        extend_info["endTime"] = stop_timestamp
        extend_info["backupTime"] = stop_timestamp + 1
        extend_info["timeline"] = copy_timeline
        extend_info["stopWalFile"] = wal_file
        return True, extend_info

    def get_pre_copy_bak_time_for_log_backup_from_log_copy(self):
        LOGGER.info(
            f"Getting previous log copy backup time for log backup, pid: {self._pid}, job id: {self._job_id}.")
        ret, pre_log_copy_info = self.get_last_copy_info([CopyDataTypeEnum.LOG_COPY.value])
        if ret and pre_log_copy_info:
            pre_copy_bak_time = pre_log_copy_info.get("extendInfo", {}).get("backupTime")
            pre_copy_stop_wal = pre_log_copy_info.get("extendInfo", {}).get("stopWalFile")
            pre_copy_timeline = pre_copy_stop_wal[:8] if is_wal_file(pre_copy_stop_wal) else ""
            LOGGER.info(f"Succeed to get previous log copy info, backup time: {pre_copy_bak_time}, stop wal: "
                        f"{pre_copy_stop_wal}, pid: {self._pid}, job id: {self._job_id}.")
            return True, pre_copy_bak_time, pre_copy_timeline
        # 查询是否进行过主备切换，进行过主备切换后，当前日志副本依赖于上一个全量副本
        return False, "", ""

    def get_pre_copy_bak_time_for_log_backup_from_full_copy(self):
        LOGGER.info(
            f"Getting previous full copy backup time for log backup, pid: {self._pid}, job id: {self._job_id}.")
        ret, pre_full_copy_info = self.get_last_copy_info([CopyDataTypeEnum.FULL_COPY.value])
        if not ret or not pre_full_copy_info:
            LOGGER.error(f"Failed to get previous full copy info, result: {ret}, pid: {self._pid}, "
                         f"job id: {self._job_id}.")
            return False, "", ""
        # 检查日志备份有无依赖的全量副本
        pre_copy_stop_wal = pre_full_copy_info.get("extendInfo", {}).get("stopWalFile")
        pre_copy_timeline = pre_copy_stop_wal[:8] if is_wal_file(pre_copy_stop_wal) else ""
        pre_copy_bak_time = pre_full_copy_info.get("extendInfo", {}).get("backupTime")
        if not pre_copy_bak_time:
            LOGGER.error(f"Failed to get previous full copy backup time: {pre_copy_bak_time}, stop wal: "
                         f"{pre_copy_stop_wal}, pid: {self._pid}, job id: {self._job_id}.")
            return False, "", ""
        LOGGER.info(f"Succeed to get previous full copy info, backup time: {pre_copy_bak_time}, stop wal: "
                    f"{pre_copy_stop_wal}, pid: {self._pid}, job id: {self._job_id}.")
        return True, pre_copy_bak_time, pre_copy_timeline

    def check_log_copy_timeline(self, pre_copy_timeline, copy_timeline):
        if pre_copy_timeline and copy_timeline and pre_copy_timeline != copy_timeline:
            LOGGER.error(f"The timeline: {copy_timeline} of the log backup is inconsistent with the timeline: "
                         f"{pre_copy_timeline} of the previous copy, pid: {self._pid}, job id: {self._job_id}.")
            return False
        return True

    def write_full_backup_time(self, copy_dict, stop_time):
        """
        记录全量备份的结束时间和恢复后首次全量备份的结束时间
        :return:
        """
        param = dict()
        # 全备/增备时间加1s sys_rman工具才能自动选择到对应备份集
        param["backupTime"] = convert_time_to_timestamp(stop_time) + 1
        if not self._job_dict.get("extendInfo", {}).get("next_cause_param", ""):
            LOGGER.warning(f"Parse param failed. pid:{self._pid} jobId:{self._job_id}")
            copy_dict["extendInfo"] = param
            return copy_dict
        ret = True
        next_cause_key = self._job_dict.get("extendInfo", {}).get("next_cause_param")
        if next_cause_key != KbConst.NOT_THE_FIRST_BACKUP_AFTER_RESTORE:
            param["firstFullBackupTime"] = convert_time_to_timestamp(stop_time)
        if not ret:
            # 获取失败不影响当次备份任务结果
            LOGGER.warning(f"Get first full backup time failed. pid:{self._pid} jobId:{self._job_id}")

        copy_dict["extendInfo"] = param
        LOGGER.info(f"Success get first full backup time, pid:{self._pid} jobId:{self._job_id}")
        return copy_dict

    def get_pre_backup_time_of_incr(self):
        pre_copy_timeline = ""
        pre_copy_bak_time = ""
        ret_full, pre_full_copy_info = self.get_last_copy_info([CopyDataTypeEnum.FULL_COPY.value])
        ret_incr, pre_incr_copy_info = self.get_last_copy_info([CopyDataTypeEnum.INCREMENT_COPY.value])
        if ret_full:
            pre_copy_stop_wal = pre_full_copy_info.get("extendInfo", {}).get("stopWalFile")
            pre_copy_timeline = pre_copy_stop_wal[:8] if is_wal_file(pre_copy_stop_wal) else ""
            pre_copy_bak_time = pre_full_copy_info.get("extendInfo", {}).get("backupTime") + 1
            LOGGER.info(
                f"Last full backup time:{convert_timestamp_to_time(pre_copy_bak_time)}, timestamp:{pre_copy_bak_time}.")
            if ret_incr and pre_copy_bak_time < pre_incr_copy_info.get("extendInfo", {}).get("backupTime") + 1:
                pre_copy_stop_wal = pre_incr_copy_info.get("extendInfo", {}).get("stopWalFile")
                pre_copy_timeline = pre_copy_stop_wal[:8] if is_wal_file(pre_copy_stop_wal) else ""
                pre_copy_bak_time = pre_incr_copy_info.get("extendInfo", {}).get("backupTime") + 1
                LOGGER.info(
                    f"Last incr backup time:{convert_timestamp_to_time(pre_copy_bak_time)},'\
                    f'timestamp:{pre_copy_bak_time}.")
        return pre_copy_timeline, pre_copy_bak_time

    def get_copy_info_last_full_backup_time(self):
        ret, out_info = self.get_last_copy_info(["full"])
        if not ret or not out_info:
            LOGGER.error(f"Failed to get last copy info. pid:{self._pid} jobId{self._job_id}")
            return False, ""
        if not out_info.get("extendInfo", {}).get("firstFullBackupTime", ""):
            LOGGER.error("Failed to get data copy last backup time")
            return False, ""
        last_backup_time = out_info.get("extendInfo", {}).get("firstFullBackupTime")
        LOGGER.info(f"Succeed to get data copy info last backup time({last_backup_time}). \
            pid:{self._pid} jobId{self._job_id}")
        return True, last_backup_time

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

    def get_log_backup_begin_time(self, begin_time):
        # 判断如果起始时间超过首次全量备份时间，就截断
        ret, first_backup_time = self.get_copy_info_last_full_backup_time()
        if ret and begin_time < first_backup_time:
            begin_time = first_backup_time
        return begin_time

    @exter_attack
    def abort_job(self):
        """
        终止备份任务
        :return: boolean，True代表终止成功，False代表终止失败
        """
        check_white_list([self._cache_area])
        aborting_file = os.path.join(self._cache_area, "abort.ing")
        try:
            pathlib.Path(aborting_file).touch()
        except Exception as ex:
            LOGGER.error(f"Failed to create aborting file, job id: {self._job_id}. Exception info: {ex}.")
            self.set_backup_result(SubJobStatusEnum.ABORTED_FAILED.value, BodyErrCode.PLUGIN_CANNOT_BACKUP_ERR.value,
                                   f"Failed to create file {aborting_file}.")
            return False
        pid_list = psutil.pids()
        for pid in pid_list:
            try:
                process = psutil.Process(pid)
            except Exception as err:
                LOGGER.error(f"Get process err: {err}.")
                continue
            cmd = process.cmdline()
            if 'python3' in cmd and (self._job_id in cmd and self._pid not in cmd):
                process.kill()
                LOGGER.info(f"The backup task has been terminated, job id: {self._job_id}.")
                break
        aborted_file = os.path.join(self._cache_area, "abort.done")
        try:
            os.rename(aborting_file, aborted_file)
        except Exception as ex:
            LOGGER.error(f"Failed to create aborted file, job id: {self._job_id}. Exception info: {ex}.")
            self.set_backup_result(SubJobStatusEnum.ABORTED_FAILED.value,
                                   BodyErrCode.PLUGIN_CANNOT_BACKUP_ERR.value,
                                   f"Failed to create file {aborted_file}.")
            return False
        self.set_backup_result(SubJobStatusEnum.ABORTED.value, "", "")
        LOGGER.info(f"Succeed to abort backup job, job id: {self._job_id}.")
        return True

    @exter_attack
    def query_abort_job_progress(self):
        """
        查询终止备份任务进度
        """
        check_white_list([self._cache_area])
        if os.path.exists(os.path.join(self._cache_area, "abort.ing")):
            status = SubJobStatusEnum.ABORTING.value
        elif os.path.exists(os.path.join(self._cache_area, "abort.done")):
            status = SubJobStatusEnum.ABORTED.value
        else:
            status = SubJobStatusEnum.ABORTED_FAILED.value
        output = SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, progress=KbConst.PROGRESS_ONE_HUNDRED,
                               taskStatus=status)
        LOGGER.info(f"SubJobDetails: {output.dict(by_alias=True)}, job id: {self._job_id}.")
        output_result_file(self._pid, output.dict(by_alias=True))
        return True

    def read_tmp_json_file(self, file_path):
        """
        读取备份子任务结果
        :param file_path: str，文件路径
        :return: dict，备份子任务结果信息
        """
        check_is_path_exists(file_path)
        try:
            with open(file_path, "r", encoding='UTF-8') as file:
                json_dict = json.loads(file.read())
        except Exception as ex:
            LOGGER.error(f"Failed to parse param file, job id: {self._job_id}.")
            raise Exception("parse param file failed") from ex
        return json_dict

    def write_tmp_json_file(self, context, file_name):
        """
        写入备份子任务结果
        :param file_name: str，文件名
        :param context: dict，备份子任务结果信息
        """
        file_path = os.path.join(self._cache_area, file_name)
        if self.check_path_is_link(file_path):
            LOGGER.error(f"The cache area path is invalid, job id: {self._job_id}.")
            return
        if os.path.exists(file_path):
            try:
                os.remove(file_path)
            except Exception as ex:
                LOGGER.error(f"Failed to remove cache area path file, job id: {self._job_id}.")
                raise Exception("remove file failed") from ex
            LOGGER.info(f"Remove cache area path file, job id: {self._job_id}.")
        flags = os.O_WRONLY | os.O_CREAT | os.O_EXCL
        modes = stat.S_IWUSR | stat.S_IRUSR | stat.S_IXUSR
        with os.fdopen(os.open(file_path, flags, modes), 'w+') as out_file:
            out_file.write(json.dumps(context))

    def copy_backup_info_file(self, remote_backup_info_file_path):
        """
        拷贝backup.info文件和backup.info.copy到data仓
        """
        backup_info_file = os.path.join(remote_backup_info_file_path, KbConst.BACKUP_INFO_FILE_NAME)
        backup_info_copy_file = os.path.join(remote_backup_info_file_path, KbConst.BACKUP_INFO_COPY_FILE_NAME)
        remote_last_path = os.path.join(remote_backup_info_file_path, "latest")
        try:
            shutil.copy2(backup_info_file, remote_last_path)
            shutil.copy2(backup_info_copy_file, remote_last_path)
        except Exception as exception_info:
            LOGGER.error(f"Copy backup.info and backup.info.copy err: {exception_info}, job id: {self._job_id}")
            return False
        LOGGER.info(f"Copy backup.info and backup.info.copy successfully, job id: {self._job_id}.")
        return True

    def remove_archive_wal_files_and_copy_archive_info_file(self):
        # 移走log日志到data仓
        if not self.remove_archive_wal_files():
            self.set_backup_result(SubJobStatusEnum.FAILED.value, ErrCode.BACKUP_FAILED,
                                   "Failed to remove archive log.")
            LOGGER.error(f"Failed to remove archive log, job id: {self._job_id}.")
            return False
        # 拷贝archive.info文件和archive.info.copy到data仓
        if not self.copy_archive_info_file(self._data_area):
            self.set_backup_result(SubJobStatusEnum.FAILED.value, ErrCode.BACKUP_FAILED,
                                   "Failed to copy archive.info.")
            LOGGER.error(f"Failed to copy archive.info, job id: {self._job_id}.")
            return False
        # 拷贝backup.info和backup.info.copy到data仓latest副本中
        if not self.copy_backup_info_file(os.path.join(self._data_area, self._db_system_id, KbConst.BACKUP_DIR_NAME,
                                                       KbConst.KINGBASE_DIR_NAME)):
            self.set_backup_result(SubJobStatusEnum.FAILED.value, ErrCode.BACKUP_FAILED,
                                   "Failed to copy backup.info.")
            LOGGER.error(f"Failed to copy backup.info, job id: {self._job_id}.")
            return False
        return True

    def copy_archive_info_file(self, target_path):
        """
        移走log日志和拷贝archive.info文件和archive.info.copy到data仓
        """
        # 拷贝archive.info文件和archive.info.copy到data仓
        archive_info_file = os.path.join(self._repo_path, KbConst.ARCHIVE_DIR_NAME, KbConst.KINGBASE_DIR_NAME,
                                         KbConst.ARCHIVE_INFO_FILE_NAME)
        archive_info_copy_file = os.path.join(self._repo_path, KbConst.ARCHIVE_DIR_NAME, KbConst.KINGBASE_DIR_NAME,
                                              KbConst.ARCHIVE_INFO_COPY_FILE_NAME)
        remote_kingbase_path = os.path.join(target_path, self._db_system_id, KbConst.ARCHIVE_DIR_NAME,
                                            KbConst.KINGBASE_DIR_NAME)
        return_code, out, std_err = execute_cmd(
            f'cp {archive_info_file} {archive_info_copy_file} {remote_kingbase_path}')
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error(f"Copy archive.info and archive.info.copy failed, return code: {return_code}, "
                         f"out: {out}, err: {std_err}.")
            return False
        LOGGER.info(f"Copy archive.info and archive.info.copy successfully, job id: {self._job_id}.")
        return True

    def remove_archive_wal_files(self):
        """
        移走log日志
        """
        # 移走归档路径下所有日志文件到data仓
        source_path = os.path.join(self._repo_path, KbConst.ARCHIVE_DIR_NAME, KbConst.KINGBASE_DIR_NAME,
                                   self._db_version_id)

        history_file_list = []
        for base_source_timeline_path in os.listdir(source_path):
            target_timeline_path = os.path.join(self._data_area, self._db_system_id, KbConst.ARCHIVE_DIR_NAME,
                                                KbConst.KINGBASE_DIR_NAME, self._db_version_id,
                                                os.path.basename(base_source_timeline_path))
            source_timeline_path = os.path.join(source_path, base_source_timeline_path)
            # 过滤.history文件
            if os.path.isfile(source_timeline_path):
                history_file_list.append(source_timeline_path)
                continue
            if not os.path.exists(target_timeline_path):
                os.makedirs(target_timeline_path)
            log_file_list = []

            for log_file in os.listdir(source_timeline_path):
                if "-" in log_file:
                    log_file_list.append(os.path.join(source_timeline_path, log_file))
            if log_file_list:
                log_files = ' '.join(log_file_list)
                LOGGER.info(f"Move archive log begin:{log_files}.")
                return_code, out, std_err = execute_cmd(f'mv {log_files} {target_timeline_path}')
                if return_code != CmdRetCode.EXEC_SUCCESS.value:
                    LOGGER.error(f"Move archive log failed, return code: {return_code}, "
                                 f"out: {out}, err: {std_err}.")
                    return False
        LOGGER.info(f"Copy archive log successfully, job id: {self._job_id}.")

        # 拷贝.history文件到data仓
        if history_file_list:
            history_files = ' '.join(history_file_list)
            LOGGER.info(f"Move .history begin:{history_files}.")
            target_history_path = os.path.join(self._data_area, self._db_system_id, KbConst.ARCHIVE_DIR_NAME,
                                               KbConst.KINGBASE_DIR_NAME, self._db_version_id)
            return_code, out, std_err = execute_cmd(f'mv {history_files} {target_history_path}')
            if return_code != CmdRetCode.EXEC_SUCCESS.value:
                LOGGER.error(f"Move .history failed, return code: {return_code}, "
                             f"out: {out}, err: {std_err}.")
                return False
        return True

    def set_backup_result(self, status, body_err, message, progress_file_name=BackupProgressEnum.BACKUP_PROGRESS.value):
        """
        设置备份子任务结果
        :param status: int，子任务状态码
        :param body_err: int，子任务错误码
        :param message: str，子任务错误信息
        :param progress_file_name:str, 进度文件名称
        :return: None
        """
        progress_file = os.path.join(self._cache_area, progress_file_name)
        task_info = self.read_tmp_json_file(progress_file)
        if "status" in task_info and "error_code" in task_info and "message" in task_info:
            task_info["status"] = status
            task_info["error_code"] = body_err
            task_info["message"] = message
        LOGGER.info(f"Set backup result, status: {status}, message: {message}, job id: {self._job_id}.")
        self.write_tmp_json_file(task_info, progress_file)

    def write_backup_info_file(self, progress_file_name=BackupProgressEnum.BACKUP_PROGRESS.value):
        """
        写入备份信息文件
        """
        content = BackupProgressInfo(last_objects=0, backup_objects=0)
        content.s_time = int(time.time())
        content.c_time = int(time.time())
        content.status = SubJobStatusEnum.RUNNING.value
        self.write_tmp_json_file(content.dict(by_alias=True), progress_file_name)

    def set_action_result(self, code=ExecuteResultEnum.SUCCESS.value, body_err=None, message=None):
        """
        设置步骤执行结果
        :param code: int，返回码
        :param body_err: int，错误码
        :param message: str，错误消息
        """
        self._output.code = code
        self._output.body_err = body_err
        self._output.message = message

    def get_action_result(self):
        """
        获取步骤执行结果
        :return: dict，步骤执行结果信息字典
        """
        return self._output
