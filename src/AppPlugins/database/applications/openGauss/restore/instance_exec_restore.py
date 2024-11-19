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
import platform
import re
import shutil
import json
import datetime
import time

from common.common import execute_cmd, check_dir_uid_gid, convert_time_to_timestamp, write_content_to_file, \
    get_local_ips, read_tmp_json_file, convert_timestamp_to_time
from common.common_models import SubJobModel
from common.const import JobData, ExecuteResultEnum, SubJobStatusEnum, SubJobTypeEnum, SubJobPolicyEnum, \
    SubJobPriorityEnum
from common.file_common import delete_file_or_dir_specified_user, exec_lchown, exec_lchown_dir_recursively, \
    change_path_permission
from common.util.exec_utils import exec_cp_cmd, check_path_valid, su_exec_rm_cmd
from openGauss.common.base_cmd import BaseCmd
from openGauss.common.common import write_progress_file, read_progress, get_hostname, write_time_file, \
    safe_get_environ, check_injection_char, check_path, get_ids_by_name, check_user_name, execute_cmd_by_user
from openGauss.common.const import logger, Env, CopyInfoKey, SubJobType, NodeDetailRole, ProgressInfo, conf_file, \
    ProtectObject, ResultCode, GsprobackupParam, AuthKey, SubJobPolicy, OpenGaussDeployType
from openGauss.common.error_code import NormalErr, OpenGaussErrorCode
from openGauss.resource.cluster_instance import GaussCluster
from openGauss.common.const import Tool, DELETE_FILE_NAMES_OF_DATA_DIR

if platform.system().lower() == "linux":
    import pwd
    import grp


class ExecRestore:
    def __init__(self, inputs: {}):
        self.input_info = inputs
        self.user_name = safe_get_environ(f"{Env.OPEN_GAUSS_USER}_{JobData.PID}")
        self.inst = GaussCluster(self.user_name, self.input_info.get("env_path"))
        self.data_path = self.inst.get_instance_data_path()
        self._database_type = self.input_info.get("version")
        if not self._database_type:
            self._database_type = self.input_info.get("cluster_version")
        self.init_environment_info()
        self.restore_time = self.input_info.get("restoreTimeStamp")

    @staticmethod
    def write_progress_by_result(result: bool, progress_file: str):
        message = ProgressInfo.SUCCEED if result else ProgressInfo.FAILED
        write_progress_file(message, progress_file)

    @staticmethod
    def copy_file_by_user(user_name, source_path, dest_path):
        if not check_user_name(user_name):
            return False
        ret = exec_cp_cmd(source_path, dest_path, user_name, is_check_white_list=False)
        if not ret:
            logger.error(f"Failed to exec cp cmd.")
            return False
        return True

    @staticmethod
    def service_control(service, option):
        ret, _, _ = execute_cmd(f"systemctl {option} {service}")
        return ret == ResultCode.SUCCESS

    @staticmethod
    def build_sub_job(job_name, job_priority, job_policy, job_info=None):
        return SubJobModel(
            jobId=JobData.JOB_ID, jobType=SubJobTypeEnum.BUSINESS_SUB_JOB.value, jobInfo=job_info,
            jobName=job_name, jobPriority=job_priority, policy=job_policy).dict(by_alias=True)

    @staticmethod
    def delete_useless_files_of_data_dir(tgt_data_path):
        for file_name in DELETE_FILE_NAMES_OF_DATA_DIR:
            del_path = os.path.realpath(os.path.join(tgt_data_path, file_name))
            logger.info(f"Start deleting path: {del_path}")
            if not os.path.exists(del_path):
                logger.warning(f"The path: {del_path} does not exist when trying to delete it.")
                continue

            # 文件不存在会报错
            try:
                os.remove(del_path)
                logger.info(f"Delete file: {del_path} success.")
            except Exception as ex:
                logger.error(f"Delete file {del_path} error {str(ex)}.")

    @staticmethod
    def _build_sub_job(job_name, job_priority, node_id, job_info=""):
        sub_job = SubJobModel(
            jobId=JobData.JOB_ID,
            subJobId="",
            jobType=SubJobTypeEnum.BUSINESS_SUB_JOB.value,
            jobName=job_name,
            jobPriority=job_priority, policy=SubJobPolicyEnum.FIXED_NODE.value, ignoreFailed=False,
            execNodeId=node_id, jobInfo=job_info)
        return sub_job

    def copy_file(self):
        copy_files = os.path.join(self.input_info.get("cache_path"), JobData.JOB_ID)
        for file in os.listdir(copy_files):
            if file in conf_file:
                move_file = os.path.join(copy_files, file)
                shutil.copy(move_file, self.data_path)

    def delete_original(self):
        logger.info('Delete original data!')
        if not self.user_name:
            logger.error("Get user name error!")
            return False
        for file in os.listdir(self.data_path):
            delete_file = os.path.join(self.data_path, file)
            delete_file_or_dir_specified_user(self.user_name, delete_file)
        logger.info('Delete original complete!')
        return True

    def copy_into_meta(self):
        logger.info('Copy data!')
        if not self.user_name or not check_user_name(self.user_name):
            logger.error("Get user name error!")
            return False
        copy_file = os.path.join(self.input_info.get("cache_path"), JobData.JOB_ID)
        if not check_path(copy_file) and not check_path(self.data_path):
            return False
        if not os.path.exists(self.data_path):
            logger.error(f"Fail to get source file")
            return False
        ret = exec_cp_cmd(self.data_path, copy_file, self.user_name, is_check_white_list=False)
        if not ret:
            logger.error(f"Failed to exec cp cmd.")
            return False
        logger.info("Succeed copy data!")
        return True

    def roll_back_data(self):
        logger.info("Start roll back data!")
        if not self.user_name or not check_user_name(self.user_name):
            logger.error("Get user name error!")
            return False
        copy_file = os.path.join(self.input_info.get("cache_path"), JobData.JOB_ID)
        if not os.path.exists(copy_file):
            logger.error(f'Copy file {copy_file} failed')
            return False
        if not check_path(copy_file) and not check_path(self.data_path):
            return False
        for file in os.listdir(self.data_path):
            delete_file = os.path.join(self.data_path, file)
            delete_file_or_dir_specified_user(self.user_name, delete_file)
        ret = exec_cp_cmd(f"{copy_file}/*", self.data_path, self.user_name, is_check_white_list=False)
        if not ret:
            logger.error(f"Failed to exec cp cmd.")
            return False
        logger.info("Roll back data suc")
        return True

    def delete_copy_data(self):
        logger.info('Delete copy data!')
        user_name = safe_get_environ(f"{Env.OPEN_GAUSS_USER}_{JobData.PID}")
        if not user_name:
            logger.error("Get user name error!")
            return False
        copy_file = os.path.join(self.input_info.get("cache_path"), JobData.JOB_ID)
        if not os.path.exists(copy_file):
            logger.error(f'Copy file {copy_file} failed')
            return False
        delete_file_or_dir_specified_user(user_name, copy_file)
        return True

    def stop_database(self):
        logger.info("Stopping cluster!")
        user_name = safe_get_environ(f"{Env.OPEN_GAUSS_USER}_{JobData.PID}")
        if not user_name:
            logger.error("Get user name error!")
            return False
        base_cmd = BaseCmd(user_name, self.input_info.get("env_path"))
        if ProtectObject.VASTBASE in self.input_info.get("version"):
            local_role = self.input_info.get("local_role")
            if local_role == NodeDetailRole.STANDBY:
                return True
            if local_role == NodeDetailRole.PRIMARY:
                return self.stop_has_dcs()
            data_path = GaussCluster(user_name, self.input_info.get("env_path")).get_instance_data_path()
            if not check_path(data_path):
                return False
            cmd = f"vb_ctl stop -D {data_path}"
        else:
            cmd = "gs_om -t stop"
        ret, std_out = base_cmd.execute_cmd(cmd)
        if not ret:
            logger.error(f"Fail to exec stop with return: {ret}, err: {std_out}")
            return False
        logger.info("Succeed to stop the cluster")
        return True

    def start_database(self):
        logger.info("Starting primary node!")
        user_name = safe_get_environ(f"{Env.OPEN_GAUSS_USER}_{JobData.PID}")
        if not user_name:
            logger.error("Get user name error!")
            return False
        data_path = GaussCluster(user_name, self.input_info.get("env_path")).get_instance_data_path()
        if not check_path(data_path):
            return False
        base_cmd = BaseCmd(user_name, self.input_info.get("env_path"))
        if ProtectObject.VASTBASE in self._database_type:
            local_role = self.input_info.get("local_role")
            if local_role != NodeDetailRole.SINGLE:
                return True
        cmd = f"{self._ctl_tool} start -D {data_path}"
        ret, std_out = base_cmd.execute_cmd(cmd)
        if not ret:
            logger.error("Start primary node failed.")
            return False
        logger.info("Succeed to start the primary node")
        return True

    def start_cluster(self):
        logger.info("Starting cluster!")
        if ProtectObject.VASTBASE in self._database_type:
            return True
        if not self.user_name:
            logger.error("Get user name error!")
            return False
        data_path = self.inst.get_instance_data_path()
        if not check_path(data_path):
            return False
        base_cmd = BaseCmd(self.user_name, self.input_info.get("env_path"))
        cmd = "gs_om -t start"
        ret, std_out = base_cmd.execute_cmd(cmd)
        if not ret:
            logger.error("Maybe failed start, but return true!")
        logger.info("Succeed to start the cluster")
        return True

    def data_rebuild(self):
        logger.info("Doing data rebuild!")
        if ProtectObject.VASTBASE in self._database_type:
            logger.debug(f"Vastbase not need data rebuild. job id: {JobData.JOB_ID}")
            return True
        user_name = safe_get_environ(f"{Env.OPEN_GAUSS_USER}_{JobData.PID}")
        if not user_name:
            logger.error("Get user name error!")
            return False
        inst = GaussCluster(user_name, self.input_info.get("env_path"))
        data_path = inst.get_instance_data_path()
        if not check_path(data_path):
            return False
        cmd_tmp = f'{self._backup_tool} build -D {data_path}'
        if ProtectObject.CMDB in self._database_type:
            cmd_tmp = f'{self._ctl_tool} build -D {data_path}'
        ret, std_out = inst.cmd_obj.execute_cmd(cmd_tmp)
        if not ret:
            logger.error(f"Fail to exec data rebuild with return: {ret}, err: {std_out}")
            return False
        logger.info("Succeed to rebuild data")
        return True

    def prepare_restore_cmd(self):
        user_name = safe_get_environ(f"{Env.OPEN_GAUSS_USER}_{JobData.PID}")
        if not user_name:
            logger.error("Get user name error!")
            return ''
        inst = GaussCluster(user_name, self.input_info.get("env_path"))
        backup_key = self.input_info.get("backup_key")
        data_path = inst.get_instance_data_path()
        if not isinstance(backup_key, str) or not check_injection_char(backup_key) or not check_path(data_path):
            return ''
        cmd = f'{self._backup_tool} restore -B {self.input_info.get("media_path")} -D {data_path} -i ' \
              f'{backup_key} --instance {CopyInfoKey.BACKUP_INSTANCE} -j {GsprobackupParam.DEFAULT_PARALLEL} --progress'
        return cmd

    def do_restore_job_restore(self):
        logger.info('Start to do restore task, subJob type: restore')
        code = ExecuteResultEnum.INTERNAL_ERROR
        body_err = 0
        message = ''
        user_name = safe_get_environ(f"{Env.OPEN_GAUSS_USER}_{JobData.PID}")
        if not user_name:
            logger.error("Get user name error!")
            return code, body_err, message
        local_role = self.input_info.get("local_role")
        progress_file = os.path.join(self.input_info.get("cache_path"), f'{JobData.JOB_ID}{get_hostname()}restore')
        write_progress_file(ProgressInfo.START, progress_file)
        if not local_role:
            logger.error("Failed restore, unknown role of node, it should be primary.")
            write_progress_file(ProgressInfo.FAILED, progress_file)
            return ExecuteResultEnum.INTERNAL_ERROR, body_err, message
        if local_role == NodeDetailRole.STANDBY:
            write_progress_file(ProgressInfo.SUCCEED, progress_file)
            return ExecuteResultEnum.SUCCESS, body_err, message
        if not self.stop_database() or not self.copy_into_meta():
            logger.error('Failed to do restore, at stop or copy')
            message = "Failed to do restore, at stop or copy"
            write_progress_file(ProgressInfo.FAILED, progress_file)
            return code, body_err, message
        self.delete_original()
        cmd = self.prepare_real_cmd(progress_file)
        group_id = pwd.getpwnam(str(user_name)).pw_gid
        database_user_group = grp.getgrgid(group_id).gr_name
        if not exec_lchown_dir_recursively(progress_file, user_name, database_user_group):
            logger.error(f"Change owner for {progress_file} failed.")
            return code, body_err, message
        base_cmd = BaseCmd(user_name, self.input_info.get("env_path"))
        write_time_file(os.path.join(self.input_info.get("cache_path"), f'T{JobData.JOB_ID}'))
        logger.info('Start to do restore cmd')
        ret, std_out = base_cmd.execute_cmd(cmd)
        if not ret:
            logger.error(f"Fail to exec restore with return: {ret}, err: {std_out}")
            write_progress_file(ProgressInfo.FAILED, progress_file)
            return code, body_err, message
        logger.info('Do restore cmd suc')
        if self.restore_time and local_role != NodeDetailRole.STANDBY:
            # restore time不为空时设置recovery conf
            self.set_recovery_conf()
        self.copy_file()
        # vastbase主节点恢复ha配置
        if not self.restore_ha_conf(local_role, user_name):
            logger.error(f"Restore ha conf failed, job id: {JobData.JOB_ID}")
            write_progress_file(ProgressInfo.FAILED, progress_file)
            return code, body_err, message
        self.delete_copy_data()
        write_progress_file(ProgressInfo.SUCCEED, progress_file)
        logger.info('Succeed to do restore task, subJob type: restore')
        return ExecuteResultEnum.SUCCESS, body_err, message

    def do_restore_job_endtask(self):
        logger.info('Start to do restore task, subJob type: endtask')
        progress_file = os.path.join(self.input_info.get("cache_path"), f'{JobData.JOB_ID}{get_hostname()}endtask')
        write_progress_file(ProgressInfo.START, progress_file)
        code = ExecuteResultEnum.INTERNAL_ERROR
        body_err = 0
        message = ''
        user_name = safe_get_environ(f"{Env.OPEN_GAUSS_USER}_{JobData.PID}")
        if not user_name:
            logger.error("Get user name error!")
            return code, body_err, message
        # vastbase启动主节点
        if not self.start_dcs_has():
            write_progress_file(ProgressInfo.FAILED, progress_file)
            return ExecuteResultEnum.INTERNAL_ERROR, body_err, message
        local_role = self.input_info.get("local_role")
        # 判断子任务阶段
        if local_role == NodeDetailRole.PRIMARY or local_role == NodeDetailRole.SINGLE:
            result = self.start_database()
            self.write_progress_by_result(result, progress_file)
            if not result:
                return ExecuteResultEnum.INTERNAL_ERROR, OpenGaussErrorCode.ERR_DATABASE_START, message
        elif local_role == NodeDetailRole.STANDBY:
            write_progress_file(ProgressInfo.SUCCEED, progress_file)
        else:
            write_progress_file(ProgressInfo.FAILED, progress_file)
        logger.info('Succeed to do restore task, subJob type: endtask')
        code = ExecuteResultEnum.SUCCESS
        return code, body_err, message

    def start_dcs_has(self):
        # 单机和非vastbase数据库不需要启动dcs和has
        logger.info("Start dcs and has")
        local_role = self.input_info.get("local_role")
        if ProtectObject.VASTBASE not in self.input_info.get("version") or local_role == NodeDetailRole.SINGLE:
            logger.info(f"Local_role: {local_role}, single node or openGauss or mogdb no thing to do")
            return True
        sub_job_name = self.input_info.get("sub_job_name")
        # restart子任务启动主节点dcs和has，备节点启动dcs
        if sub_job_name == SubJobType.END_TASK:
            return self.end_task_in_vastbase_cluster(local_role)
        # 备节点在end_task任务启动has
        if sub_job_name == SubJobType.RESTART:
            return self.restart_task_in_vastbase_cluster(local_role)
        return True

    def end_task_in_vastbase_cluster(self, local_role):
        # 所有节点启动dcs
        logger.debug(f"Start all node dcs service. job id: {JobData.JOB_ID}")
        if not self.service_control("dcs", "start"):
            logger.error(f"Start dcs failed, job id: {JobData.JOB_ID}")
            return False
        # 主节点启动has
        logger.debug(f"Start primary node has service. job id: {JobData.JOB_ID}")
        if local_role == NodeDetailRole.PRIMARY:
            return self.service_control("has", "start")
        return True

    def restart_task_in_vastbase_cluster(self, local_role):
        # 备节点启动has
        logger.debug(f"Start standby node has service. job id: {JobData.JOB_ID}")
        if local_role == NodeDetailRole.STANDBY:
            return self.service_control("has", "start")
        return True

    def restore_ha_conf(self, local_role, user_name):
        logger.debug(f"Restore ha conf file. job id: {JobData.JOB_ID}")
        if ProtectObject.VASTBASE not in self._database_type or local_role != NodeDetailRole.PRIMARY:
            logger.info(f"Not vastbase or standby node no thing to do. job id: {JobData.JOB_ID}")
            return True
        inst = GaussCluster(user_name, self.input_info.get("env_path"))
        origin_data_dir = os.path.join(self.input_info.get("cache_path"), JobData.JOB_ID)
        source_dir = inst.get_instance_data_path()
        if not check_path(origin_data_dir) and not check_path(source_dir):
            logger.error(f"Bad origin data dir or source dir. job id: {JobData.JOB_ID}")
            return False
        source_conf = os.path.join(source_dir, "pg_hba.conf")
        source_conf_backup = os.path.join(source_dir, "pg_hba.conf.backup")
        delete_file_or_dir_specified_user(user_name, source_conf)
        delete_file_or_dir_specified_user(user_name, source_conf_backup)
        origin_conf = os.path.join(origin_data_dir, "pg_hba.conf")
        origin_conf_backup = os.path.join(origin_data_dir, "pg_hba.conf.backup")
        return self.copy_file_by_user(user_name, origin_conf, source_conf) and \
            self.copy_file_by_user(user_name, origin_conf_backup, source_conf_backup)

    def stop_has_dcs(self):
        logger.debug(f"Do stop has service and dcs service. job id: {JobData.JOB_ID}")
        # 停止has和dcs服务
        if not self.service_control("has", "stop") or not self.service_control("dcs", "stop"):
            logger.error(f"Stop has service or dcs service failed.  job_id: {JobData.JOB_ID}")
            return False

        # 清除dcs缓存
        ret, std_out, _ = execute_cmd("systemctl status dcs")
        if not std_out:
            logger.error(f"Query dcs.service status failed, job_id: {JobData.JOB_ID}")
            return False

        exit_pattern = r'--config-file (.+?) '
        conf_list = re.findall(exit_pattern, std_out)
        if not conf_list:
            running_pattern = r'--config-file (.+?)\n'
            conf_list = re.findall(running_pattern, std_out)
        if not conf_list:
            return False
        conf_file_ = conf_list[0]
        if not os.path.isfile(conf_file_):
            return False
        with open(conf_file_, "r", encoding='UTF-8') as conf_file_obj:
            conf_data = conf_file_obj.read()
        if not conf_data:
            return False
        dir_pattern = r'data-dir: (.+?)\n'
        dir_list = re.findall(dir_pattern, conf_data)
        if not dir_list:
            return False
        member_dir = os.path.join(dir_list[0], "member")
        if not os.path.isdir(member_dir):
            return True
        if not su_exec_rm_cmd(member_dir, check_white_black_list_flag=False):
            logger.error(f"Execute remove command failed! File path: {member_dir}.")
            return False
        logger.debug(f"Succeed to stop has service and dcs service. job id: {JobData.JOB_ID}")
        return True

    def stop_recovery_status(self):
        deploy_type = self.input_info.get("deploy_type", "")
        cluster_version = self.input_info.get("cluster_version", "")
        logger.info(
            f"Restore post job deploy type {deploy_type}, cluster_version: {cluster_version}")
        if deploy_type == OpenGaussDeployType.DISTRIBUTED and ProtectObject.CMDB in cluster_version:
            # CMDB分布式集群由pw_ctl控制恢复进程，无需手动关闭
            logger.info("CMDB not need stop recovery process")
            return True
        logger.info("Stopping recovery status!")
        if ProtectObject.VASTBASE in self._database_type:
            return True
        if not self.user_name:
            logger.error("Get user name error!")
            return False

        local_role = self.input_info.get("local_role")
        # 判断节点类型
        if not local_role:
            logger.error("Get local role error!")
            return False
        if local_role == NodeDetailRole.STANDBY:
            logger.info("No need to stop recovery status in standby node!")
            return True

        user_name = safe_get_environ(f"{Env.OPEN_GAUSS_USER}_{JobData.PID}")
        inst = GaussCluster(user_name, self.input_info.get("env_path"))
        data_port = inst.get_instance_port()

        base_cmd = BaseCmd(self.user_name, self.input_info.get("env_path"))
        cmd = "select pg_is_in_recovery();"
        ret, std_out = base_cmd.execute_sql_cmd(data_port, cmd)
        if not ret:
            logger.error(f"Maybe failed to check recovery status, return {std_out}, but return true!")
            return True

        recovery_status_list = std_out.split("\n")
        if len(recovery_status_list) < 3:
            logger.error("Failed to select pg_is_in_recovery!")
            return False
        if recovery_status_list[2].strip() != "1":
            logger.info("Database is not in recovery process.")
            return True

        cmd = "select pg_xlog_replay_resume();"
        ret, std_out = base_cmd.execute_sql_cmd(data_port, cmd)
        if not ret:
            logger.error(f"Maybe failed stop recovery, return {std_out}, but return true!")
        logger.info(f"Success to stop recovery process.")
        return True

    def do_restore_job_restart(self):
        logger.info('Start to do restore task, subJob type: restart')
        progress_file = os.path.join(self.input_info.get("cache_path"), f'{JobData.JOB_ID}{get_hostname()}restart')
        write_progress_file(ProgressInfo.START, progress_file)
        code = ExecuteResultEnum.INTERNAL_ERROR
        body_err = 0
        message = ''
        user_name = safe_get_environ(f"{Env.OPEN_GAUSS_USER}_{JobData.PID}")
        if not user_name:
            logger.error("Get user name error!")
            return code, body_err, message

        # vastbase启动备节点
        if not self.start_dcs_has():
            write_progress_file(ProgressInfo.FAILED, progress_file)
            return ExecuteResultEnum.INTERNAL_ERROR, body_err, message

        local_role = self.input_info.get("local_role")
        # 判断子任务阶段
        if not local_role:
            write_progress_file(ProgressInfo.FAILED, progress_file)
        if local_role == NodeDetailRole.SINGLE:
            write_progress_file(ProgressInfo.SUCCEED, progress_file)
            return ExecuteResultEnum.SUCCESS, body_err, message
        if local_role == NodeDetailRole.PRIMARY:
            self.start_cluster()
            write_progress_file(ProgressInfo.SUCCEED, progress_file)
            return ExecuteResultEnum.SUCCESS, body_err, message
        result = self.data_rebuild()
        self.write_progress_by_result(result, progress_file)
        logger.info('Succeed to do restore task, subJob type: restart')
        code = ExecuteResultEnum.SUCCESS
        return code, body_err, message

    def do_restore_job_cmdb(self):
        try:
            code, body_err, message = self.do_cmdb_restore()
            return code, body_err, message
        except Exception as err:
            logger.error(f"Do cmdb restore failed, err: {err}")
            progress_file = os.path.join(self.input_info.get("cache_path"), f'{JobData.JOB_ID}{get_hostname()}cmdb')
            write_progress_file(ProgressInfo.FAILED, progress_file)
            return ExecuteResultEnum.INTERNAL_ERROR, 0, ''

    def do_cmdb_restore(self):
        logger.info(f'Start to do cmdb restore task {self.input_info}')
        progress_file = os.path.join(self.input_info.get("cache_path"), f'{JobData.JOB_ID}{get_hostname()}cmdb')
        write_progress_file(ProgressInfo.START, progress_file)
        code = ExecuteResultEnum.INTERNAL_ERROR
        body_err = 0
        message = ''
        user_name = safe_get_environ(f"{Env.OPEN_GAUSS_USER}_{JobData.PID}")
        # 取依赖副本ID及目录
        base_copy_id = self.input_info.get("base_copy_id")
        logger.info(f"Get base copy id: {base_copy_id}")
        if not user_name:
            logger.error("Get user name error!")
            return code, body_err, message
        data_dir = self.input_info.get("media_path")
        data_path = os.path.join(data_dir, base_copy_id)

        target_copy_id = self.input_info.get("target_copy_id")
        logger.info(f"Get target copy id: {target_copy_id}")
        recovery_time = self.get_recovery_time(target_copy_id, data_path)

        # 创建并写入 restore.yml 文件
        restore_config = os.path.join(data_path, 'restore.yml')
        if os.path.exists(restore_config):
            os.remove(restore_config)
        logger.info(f"Get restore config {restore_config}")

        # 取当前节点业务ip
        restore_host = self.get_cur_ndoe_host()

        backup_content = f"backup_host: {restore_host}\nbackup_dir: {data_path}\ntarget_type: time\n" \
                         f"target: '{recovery_time}'"
        logger.info(f"Write restore config {backup_content} to restore_config {restore_config}")
        write_content_to_file(restore_config, backup_content)
        os.chmod(restore_config, 0o444)

        dcs_address = self.input_info.get("dcs_address")
        dcs_port = self.input_info.get("dcs_port")
        restore_cmd = f'ha_ctl restore all -p {data_path} -l http://{dcs_address}:{dcs_port}'
        env_path = self.input_info.get("env_path")
        return_code, std_out, std_err = execute_cmd_by_user(user_name, env_path, restore_cmd)
        logger.info(f"Get restore return code: {return_code}, std out: {std_out},  std err: {std_err}")
        if return_code != ResultCode.SUCCESS:
            write_progress_file(ProgressInfo.FAILED, progress_file)
            logger.error(f"Execute restore failed.")
            return code, body_err, message
        logger.info('Succeed to do restore cmdb restore task')
        write_progress_file(ProgressInfo.SUCCEED, progress_file)
        code = ExecuteResultEnum.SUCCESS
        return code, body_err, message

    def get_recovery_time(self, base_copy_id, data_path):
        # 取恢复时间
        recovery_time = ""
        restore_time_stamp = self.input_info.get("restoreTimeStamp", "")
        logger.info(f"Get recovery time stamp {restore_time_stamp}")
        if restore_time_stamp:
            recovery_time = f"{convert_timestamp_to_time(int(restore_time_stamp))}.000000+08"
        else:
            backup_history_path = os.path.join(data_path, 'backup.history')
            backup_history_infos = read_tmp_json_file(backup_history_path)
            logger.info(f"Get backup history: {backup_history_infos} from {backup_history_path}")
            for backup_history_info in backup_history_infos:
                job_id = backup_history_info.get("JobID", "")
                if job_id != base_copy_id:
                    continue
                recovery_time = backup_history_info.get("Recovery Time", "")
                break
        logger.info(f"Get recovery time {recovery_time}")
        return recovery_time

    def get_cur_ndoe_host(self):
        local_ips = get_local_ips()
        logger.info(f"Get local ips {local_ips}")
        nodes = self.input_info.get("nodes")
        endpoints = [item['endpoint'] for item in nodes]
        logger.info(f"Get cluster nodes {nodes}")
        union = set(local_ips) & set(endpoints)
        union_list = list(union)
        restore_host = union_list[0]
        logger.info(f"Get restore host {restore_host}")
        return restore_host

    def query_restore_progress(self, sub_job_type: str):
        logger.info(f'Start to query restore progress, subJob type: {sub_job_type}')
        progress = 0
        status = SubJobStatusEnum.FAILED.value
        # 判断子任务阶段
        if sub_job_type == SubJobType.PREPARE_RESTORE:
            progress_file = os.path.join(self.input_info.get("cache_path"), f'{JobData.JOB_ID}{get_hostname()}'
                                                                            f'{SubJobType.PREPARE_RESTORE}')
        elif sub_job_type == SubJobType.RESTORE:
            progress_file = os.path.join(self.input_info.get("cache_path"), f'{JobData.JOB_ID}{get_hostname()}restore')
        elif sub_job_type == SubJobType.END_TASK:
            progress_file = os.path.join(self.input_info.get("cache_path"), f'{JobData.JOB_ID}{get_hostname()}endtask')
        elif sub_job_type == SubJobType.RESTART:
            progress_file = os.path.join(self.input_info.get("cache_path"), f'{JobData.JOB_ID}{get_hostname()}restart')
        elif sub_job_type == SubJobType.CMDB_RESTORE:
            progress_file = os.path.join(self.input_info.get("cache_path"), f'{JobData.JOB_ID}{get_hostname()}cmdb')
        else:
            logger.error(f'Failed to do query restore progress, subJob type: {sub_job_type} not exist!')
            return progress, status
        if not os.path.exists(progress_file):
            logger.error("Progress file not exist!")
            return progress, status
        progress, status = read_progress(progress_file)
        return progress, status

    def check_parent(self):
        user_name = safe_get_environ(f"{Env.OPEN_GAUSS_USER}_{JobData.PID}")
        if not user_name:
            logger.error("Get user name error!")
            return NormalErr.FALSE
        if self.input_info.get("backup_type") == "FULL":
            return NormalErr.NO_ERR
        cmd = f'{self._backup_tool} show -B {self.input_info.get("media_path")} ' \
              f'--instance {CopyInfoKey.BACKUP_INSTANCE}'
        base_cmd = BaseCmd(user_name, self.input_info.get("env_path"))
        ret, std_out = base_cmd.execute_cmd(cmd)
        if not ret:
            logger.error(f"Fail to get all id info! err: {std_out}")
            return NormalErr.FALSE
        if self.input_info.get("backup_type") == "LOG":
            backup_key = self.get_backup_key()
            if not backup_key:
                logger.error("PITR restore failed to get backup key")
                return NormalErr.FALSE
            self.input_info["backup_key"] = backup_key
            logger.info(f'Success to get backup key: {backup_key}')
            return NormalErr.NO_ERR
        check_id = self.input_info.get("parent_id")
        check_id.append(self.input_info.get("backup_key"))
        for key in check_id:
            is_find = False
            for copies in std_out.split("\n"):
                if key in copies and "OK" in copies:
                    is_find = True
                    break
            if not is_find:
                logger.error(f'Backup key({key}) not found!')
                return NormalErr.FALSE
        return NormalErr.NO_ERR

    def check_mount(self):
        # 修改目录权限
        logger.info('Start to change mount')
        database_user = safe_get_environ(f"{Env.OPEN_GAUSS_USER}_{JobData.PID}")
        group_id = pwd.getpwnam(str(database_user)).pw_gid
        database_user_group = grp.getgrgid(group_id).gr_name
        database_copy_dir = self.input_info.get("media_path")
        if not exec_lchown_dir_recursively(database_copy_dir, database_user, database_user_group):
            logger.error(f"Change owner for {database_copy_dir} failed.")
            return NormalErr.FALSE
        logger.info('Change mount suc')
        return NormalErr.NO_ERR

    def check_uid_gid(self):
        """
        准备恢复任务
        :return:
        """
        user_name = safe_get_environ(f"{Env.OPEN_GAUSS_USER}_{JobData.PID}")
        if not user_name:
            logger.error("Failed to get user name")
            return NormalErr.FALSE
        try:
            # 获取数据库用户ID和组ID
            user_id, group_id = get_ids_by_name(user_name)
        except Exception:
            logger.exception("Exception when exec pre task")
            return NormalErr.FALSE
        # 检查目录ID
        if not check_dir_uid_gid(self.input_info.get('media_path'), user_id, group_id):
            logger.info(f'Path: {self.input_info.get("media_path")} is not available because of different uid or gid')
            return self.check_mount()
        logger.info('It is same uid or gid')
        return NormalErr.NO_ERR

    def do_prepare_restore(self):
        logger.info('Start to do restore task, subJob type: prepare_restore')
        progress_file = os.path.join(self.input_info.get("cache_path"), f'{JobData.JOB_ID}{get_hostname()}'
                                                                        f'{SubJobType.PREPARE_RESTORE}')
        body_err = 0
        message = ''
        write_progress_file(ProgressInfo.START, progress_file)
        local_role = self.input_info.get("local_role")
        if local_role in (NodeDetailRole.PRIMARY, NodeDetailRole.SINGLE):
            check_result = self.check_mount()
            if check_result != NormalErr.NO_ERR:
                logger.error(f"Check uid and gid error! job id: {JobData.JOB_ID}")
                write_progress_file(ProgressInfo.FAILED, progress_file)
                return ExecuteResultEnum.INTERNAL_ERROR, body_err, message
            check_result = self.check_parent()
            if check_result != NormalErr.NO_ERR:
                logger.error(f"Check chains of backup copies failed. job id {JobData.JOB_ID}")
                write_progress_file(ProgressInfo.FAILED, progress_file)
                return ExecuteResultEnum.INTERNAL_ERROR, body_err, message
        if ProtectObject.VASTBASE not in self._database_type or local_role != NodeDetailRole.STANDBY:
            logger.debug(f"Primary node or single node or OpenGauss or MogDb nothing to do. job id: {JobData.JOB_ID}")
            write_progress_file(ProgressInfo.SUCCEED, progress_file)
            return ExecuteResultEnum.SUCCESS, body_err, message

        # 备节点停止has和dcs
        if not self.stop_has_dcs():
            logger.error(f"Get user name or stop standby ha error! job id: {JobData.JOB_ID}")
            write_progress_file(ProgressInfo.FAILED, progress_file)
            return ExecuteResultEnum.INTERNAL_ERROR, body_err, message

        # 备节点删除数据目录
        user_name = safe_get_environ(f"{AuthKey.TARGET_ENV}{JobData.PID}")
        inst = GaussCluster(user_name, self.input_info.get("env_path"))
        data_path = inst.get_instance_data_path()
        if not check_path(data_path):
            logger.error(f"Bad data path! job id: {JobData.JOB_ID}")
            write_progress_file(ProgressInfo.FAILED, progress_file)
            return ExecuteResultEnum.INTERNAL_ERROR, body_err, message
        for file_name in os.listdir(data_path):
            delete_file_or_dir_specified_user(user_name, os.path.join(data_path, file_name))
        write_progress_file(ProgressInfo.SUCCEED, progress_file)
        logger.debug(f"Do prepare restore success. job id: {JobData.JOB_ID}")
        return ExecuteResultEnum.SUCCESS, body_err, message

    def gen_sub_job(self):
        deploy_type = self.input_info.get("deploy_type", "")
        cluster_version = self.input_info.get("cluster_version", "")
        logger.info(
            f"Start executing generate restore sub job deploy type {deploy_type}, cluster_version: {cluster_version}")
        if deploy_type == OpenGaussDeployType.DISTRIBUTED and ProtectObject.CMDB in cluster_version:
            return self.gen_sub_job_cmdb()
        nodes = self.input_info.get("nodes")
        sub_jobs = list()
        index = 0
        nums = len(nodes)
        for node in nodes:
            node_id = node.get("id")
            node_role = node.get("extendInfo", {}).get("role")
            if node_role == NodeDetailRole.PRIMARY_NUM:
                priority = nums
            else:
                priority = index
                index += 1
            restore_sub_jobs = self._gen_restore_sub_jobs(node_id, priority)
            sub_jobs.extend(restore_sub_jobs)

        logger.info(f"Execute generate sub job task of full copy success. Sub Jobs: {sub_jobs}.")
        return sub_jobs

    def gen_sub_job_cmdb(self):
        """
        执行分发子任务
        """
        logger.info(f"Start to gen cmdb restore sub task. input info: {self.input_info}")
        sub_job_array = []
        # 子任务1：执行恢复
        sub_job = self.build_sub_job(SubJobType.CMDB_RESTORE, SubJobPriorityEnum.JOB_PRIORITY_1,
                                     SubJobPolicy.ANY_NODE.value)
        sub_job_array.append(sub_job)
        logger.info(f"Success to gen cmdb restore sub task. sub_job_array: {sub_job_array}")
        return sub_job_array

    def init_environment_info(self):
        if ProtectObject.OPENGAUSS in self._database_type or ProtectObject.MOGDB in self._database_type:
            self._backup_tool = Tool.GS_PROBACKUP
            self._ctl_tool = Tool.GS_CTL
        elif ProtectObject.CMDB in self._database_type:
            self._backup_tool = Tool.CM_PROBACKUP
            self._ctl_tool = Tool.CM_CTL
        else:
            self._backup_tool = Tool.VB_PROBACKUP
            self._ctl_tool = Tool.VB_CTL

    def prepare_real_cmd(self, progress_file):
        if not self.restore_time:
            cmd = f'{self.prepare_restore_cmd()} >> {progress_file} 2>&1'
        else:
            cmd = f'{self.prepare_pitr_restore_cmd()} >> {progress_file} 2>&1'
        return cmd

    def get_backup_key(self):
        # 获取离选择的可恢复时间点最近的备份id
        user_name = safe_get_environ(f"{Env.OPEN_GAUSS_USER}_{JobData.PID}")
        if not user_name:
            logger.error("Get user name error!")
            return NormalErr.FALSE
        if self.input_info.get("backup_type") != "LOG":
            return NormalErr.NO_ERR
        cmd = f'{self._backup_tool} show -B {self.input_info.get("media_path")} ' \
              f'--instance {CopyInfoKey.BACKUP_INSTANCE} --format=json'
        base_cmd = BaseCmd(user_name, self.input_info.get("env_path"))
        ret, std_out = base_cmd.execute_cmd(cmd)
        if not ret:
            logger.error(f"Fail to get all id info! err: {std_out}")
            return ''
        out_list = json.loads(std_out)
        backups = out_list[0]["backups"]
        for backup in backups:
            copy_time = backup["recovery-time"]
            copy_time = convert_time_to_timestamp(copy_time[:len(copy_time) - len(CopyInfoKey.UTC_TIME_SUFFIX)])
            if copy_time < int(self.restore_time):
                logger.info(
                    f'The selected backup id：{backup["id"]}, its recovery time: {copy_time},  '
                    f'the restore time is: {self.restore_time}')
                return backup["id"]
        logger.error('Backup key not found!')
        return ''

    def prepare_pitr_restore_cmd(self):
        # 准备pitr restore cmd
        backup_key = self.get_backup_key()
        if not backup_key:
            return ''
        user_name = safe_get_environ(f"{Env.OPEN_GAUSS_USER}_{JobData.PID}")
        if not user_name:
            logger.error("Get user name error!")
            return ''
        inst = GaussCluster(user_name, self.input_info.get("env_path"))
        data_path = inst.get_instance_data_path()
        if not isinstance(backup_key, str) or not check_injection_char(backup_key) or not check_path(data_path):
            return ''
        cmd = f'{self._backup_tool} restore -B {self.input_info.get("media_path")} -D {data_path} -i ' \
              f'{backup_key} --instance {CopyInfoKey.BACKUP_INSTANCE} -j {GsprobackupParam.DEFAULT_PARALLEL} --progress'
        logger.info(f"PITR restore cmd: {cmd}")
        return cmd

    def set_recovery_conf(self):
        # 设置recovery.conf
        user_name = safe_get_environ(f"{Env.OPEN_GAUSS_USER}_{JobData.PID}")
        inst = GaussCluster(user_name, self.input_info.get("env_path"))
        data_path = inst.get_instance_data_path()
        # 删除可能的之前备份残留的无用文件
        self.delete_useless_files_of_data_dir(data_path)
        recovery_conf_file = os.path.realpath(
            os.path.join(data_path, "recovery.conf"))
        log_path = self.input_info.get("merged_log_path")
        restore_command = f"restore_command = 'cp {log_path}/%f %p'\n"
        restore_time = (datetime.datetime.fromtimestamp(int(self.restore_time)).strftime('%Y-%m-%d %H:%M:%S') +
                        CopyInfoKey.UTC_TIME_SUFFIX)
        restore_time_cmd = f"recovery_target_time = '{restore_time}'"
        write_content_to_file(recovery_conf_file, restore_command + restore_time_cmd)
        user_id, group_id = get_ids_by_name(user_name)
        os.lchown(recovery_conf_file, user_id, group_id)
        change_path_permission(recovery_conf_file, mode=0o644)
        logger.info(f"Set recovery conf success")

    def _gen_restore_sub_jobs(self, node_id, priority_wight):
        job_args = (
            SubJobType.PREPARE_RESTORE,
            SubJobType.RESTORE,
            SubJobType.END_TASK,
            SubJobType.RESTART
        )
        restore_sub_jobs = []
        for ind, job_name in enumerate(job_args):
            if ind == 3:
                priority = ind + priority_wight + 1
            else:
                priority = ind + 1
            restore_sub_jobs.append(self._build_sub_job(
                job_name=job_name,
                job_priority=priority,
                node_id=node_id).dict(by_alias=True)
                                    )
        return restore_sub_jobs