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
import sys
import time
import shutil
from common.common import output_result_file
from common.common_models import ActionResult, SubJobDetails, LogDetail
from common.const import JobData, ExecuteResultEnum, RepositoryDataTypeEnum, SubJobStatusEnum
from common.parse_parafile import ParamFileUtil
from common.util.check_utils import is_valid_id, check_path_in_white_list
from common.util.exec_utils import su_exec_rm_cmd, exec_mkdir_cmd
from common.util.backup import query_progress, backup_files
from openGauss.common.common import read_progress, query_speed, check_path, safe_get_environ, \
    safe_check_injection_char, get_hostname, record_err_code, query_err_code, safe_remove_path
from openGauss.common.const import logger, Env, CopyDirectory, SubJobType, NodeDetailRole, WhitePath, BackupStatus
from openGauss.common.error_code import NormalErr
from openGauss.common.opengauss_param import JsonParam
from openGauss.resource.cluster_instance import GaussCluster
from openGauss.restore.instance_check_restore_type import CheckRestore
from openGauss.restore.instance_exec_restore import ExecRestore


class RestoreOutput:
    def __init__(self):
        self.output_code = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, bodyErr=0, message='')
        self.progress_info = SubJobDetails(taskId='', subTaskId='', taskStatus=SubJobStatusEnum.FAILED.value,
                                           logDetail=[], progress=0, dataSize=0, speed=0, extendInfo=None)
        self.err_log = LogDetail(logInfo='', logInfoParam=[], logTimestamp=0, logDetail=0, logDetailParam=[],
                                 logDetailInfo=[], logLevel=3)

    @staticmethod
    def param_check(pid, job_id, context):
        if not safe_check_injection_char(pid, job_id, context["backup_key"]):
            logger.error(f"Bad pid or Bad job id or Bad backup key. pid: {pid}, job id: {job_id}")
            return False
        for parent_id in context["parent_id"]:
            if not safe_check_injection_char(parent_id):
                logger.error(f"Bad parent id. pid: {pid}, job id: {job_id}")
                return False
        env_path = context["env_path"]
        if env_path and not (os.path.isfile(env_path) and check_path(env_path)):
            logger.error(f"Bad env path. pid: {pid}, job id: {job_id}")
            return False
        logger.info(f"传入的context为：{context}")
        meta_path = context["meta_path"]
        if meta_path and os.path.islink(meta_path):
            logger.error(f"Bad meta path. pid: {pid}, job id: {job_id}")
            return False
        media_path = context["media_path"]
        if media_path and os.path.islink(media_path):
            logger.error(f"Bad media path. pid: {pid}, job id: {job_id}")
            return False
        cache_path = context["cache_path"]
        return not cache_path or check_path(cache_path)

    @staticmethod
    def get_local_role(param_dict):
        nodes = param_dict.get("job", {}).get("targetEnv", {}).get("nodes", [])
        if not nodes:
            return NodeDetailRole.NOTHING
        nodes_nums = len(nodes)
        if nodes_nums == 1:
            return NodeDetailRole.SINGLE
        hostname = get_hostname()
        has_primary = False
        default_primary = False
        for ind, node in enumerate(nodes):
            role = node.get("extendInfo", {}).get("role")
            if role == NodeDetailRole.PRIMARY_NUM:
                has_primary = True
            if node.get("name", "") != hostname:
                continue
            if has_primary:
                return NodeDetailRole.PRIMARY if role == NodeDetailRole.PRIMARY_NUM else NodeDetailRole.STANDBY
            if ind == nodes_nums - 1:
                default_primary = True
        if has_primary:
            return NodeDetailRole.STANDBY
        return NodeDetailRole.PRIMARY if default_primary else NodeDetailRole.STANDBY


    @staticmethod
    def get_media_path(context, job_dict):
        # 获取实例备份所在目录和cache仓目录
        if context["backup_type"] == "LOG":
            copy_dict = job_dict.get("copies", [{}])[-2]
        else:
            copy_dict = job_dict.get("copies", [{}])[-1]
        repositories_info = copy_dict.get("repositories", [])
        for reps in repositories_info:
            path = reps.get("path", [])[0]
            if reps.get("repositoryType") == RepositoryDataTypeEnum.DATA_REPOSITORY and check_path(path):
                context["meta_path"] = path
                context["media_path"] = os.path.join(path, CopyDirectory.INSTANCE_DIRECTORY)
            if reps.get("repositoryType") == RepositoryDataTypeEnum.CACHE_REPOSITORY:
                if check_path(path):
                    context["cache_path"] = path
        logger.info(f"Get media path success.")
        return context

    @staticmethod
    def get_copy_mount_paths(copy_dict: {}, repo_type):
        # 根据目标仓获取对应目录
        copy_mount_paths = []
        # 当需要获取日志仓时 备份副本类型需为LOG
        if (repo_type == RepositoryDataTypeEnum.LOG_REPOSITORY.value and
                copy_dict.get("extendInfo", {}).get("backupType") != "LOG"):
            return copy_mount_paths
        for repo in copy_dict.get("repositories", []):
            tmp_repo_type = repo.get("repositoryType")
            if tmp_repo_type != repo_type:
                continue
            if not repo.get("path"):
                logger.error(f"The path value in repository is empty, repository type: {tmp_repo_type}.")
                raise Exception("The path value in repository is empty")
            copy_mount_paths.append(repo.get("path")[0])
        if not copy_mount_paths or not check_path_in_white_list(copy_mount_paths[0]):
            logger.error(f"The copy mount path list: {copy_mount_paths} is incorrect.")
            raise Exception("The copy mount path list is empty")
        logger.info(f"Get copy mount path success, paths: {copy_mount_paths}, repository type: {repo_type}.")
        return copy_mount_paths


    @staticmethod
    def get_restore_status(job_id):
        restore_status = False
        while True:
            time.sleep(10)
            query_status, restore_progress, data_size = query_progress(job_id)
            logger.info(f"Get result: status:{query_status}, progress:{restore_progress}, data_size:{data_size}")
            if query_status == BackupStatus.COMPLETED:
                logger.info(f"Restore completed, jobId: {job_id}.")
                restore_status = True
                break
            elif query_status == BackupStatus.RUNNING:
                continue
            elif query_status == BackupStatus.FAILED:
                logger.error(f"Restore failed, jobId: {job_id}.")
                restore_status = False
                break
            else:
                logger.error(f"Backup failed, status error jobId: {job_id}.")
                restore_status = False
                break
        return restore_status


    @staticmethod
    def copy_files(os_user: str, src_path: str, target_path: str, wildcard=".", job_id=""):
        logger.info(f"Start copying file: {src_path} to path: {target_path}")
        if os.path.isdir(src_path):
            src_path = f"{src_path}" if src_path.endswith("/") else f"{src_path}/"
        res = backup_files(job_id, [src_path], target_path, write_meta=True)
        if not res:
            logger.error(f"Failed to start backup, jobId: {job_id}.")
            return False
        return RestoreOutput.get_restore_status(job_id)

    @staticmethod
    def merge_log_copies(os_user, log_copies, merged_path, job_id=""):
        # 下发多个日志备份时 需合并日志文件
        if os.path.exists(merged_path):
            safe_remove_path(merged_path)
        logger.info(f"Try to create copy merge path {merged_path} by user {os_user}.")
        ret = exec_mkdir_cmd(merged_path, os_user, 0o700, is_check_white_list=False)

        if not ret:
            logger.error(f"Create copy merge path failed")
            raise Exception("Create copy merge path failed.")
        logger.info("Create copy merge path success.")
        for tmp_copy in log_copies:
            RestoreOutput.copy_files(os_user, tmp_copy, merged_path, wildcard=".", job_id=job_id)
        # 检查merged_path下有哪些文件
        contents = os.listdir(merged_path)
        logger.info(f"merged_path下存在文件: {contents}")
        logger.info("Merge log copies success.")


    @staticmethod
    def handle_log_copy(job_dict):
        # 生成日志合并目录并将日志拷到对应目录下
        copies = job_dict.get("copies", [{}])
        cache_path = RestoreOutput.get_copy_mount_paths(copies[-2], RepositoryDataTypeEnum.CACHE_REPOSITORY.value)[0]
        return os.path.realpath(os.path.join(cache_path, "merged_logs"))

    @staticmethod
    def pre_merge_logs(job_dict, input_log_info):
        copies = job_dict.get("copies", [{}])
        log_copy_paths = []
        for tmp_copy in copies[1:]:
            tmp_log_copies = RestoreOutput.get_copy_mount_paths(tmp_copy, RepositoryDataTypeEnum.LOG_REPOSITORY.value)
            log_copy_paths.extend(tmp_log_copies)
        tgt_db_os_user = copies[-1].get("extendInfo", {}).get("userName")
        # 日志副本路径去重
        log_copy_paths = list(set(log_copy_paths))
        if log_copy_paths:
            pre_merged_path = input_log_info["merged_log_path"]
            RestoreOutput.merge_log_copies(tgt_db_os_user, log_copy_paths, pre_merged_path, job_id=JobData.JOB_ID)
        else:
            logger.error("No log copy mount path exists for point-in-time recovery.")
            raise Exception("No log copy mount path exists for point-in-time recovery.")


    @staticmethod
    def parse_param():
        """
        解析参数文件，获取恢复所需信息
        :return: dict()
        """
        logger.info("Parsing params from json file and ini file")
        context = dict()
        try:
            param_dict = JsonParam.parse_param_with_jsonschema(JobData.PID)
        except Exception:
            logger.error("Failed to parse param file")
            output = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, message="Failed to parse param file")
            output_result_file(JobData.PID, output.dict(by_alias=True))
            return {}, context
        local_role = RestoreOutput.get_local_role(param_dict)
        if local_role == NodeDetailRole.NOTHING:
            logger.error(f"Get local role failed. jod id: {JobData.PID}")
            return {}, context
        context["local_role"] = local_role
        job_dict = param_dict.get("job", {})
        copy_dict = job_dict.get("copies", [{}])[-1]
        context["restore_type"] = job_dict.get("jobParam", {}).get("restoreType")
        context["nodes"] = job_dict.get("targetEnv", {}).get("nodes", [])
        extend_dict = copy_dict.get("extendInfo", {})
        if not extend_dict.get("backupIndexId"):
            extend_dict = copy_dict.get("extendInfo", {}).get("extendInfo", {})
        context["backup_key"] = extend_dict.get("backupIndexId")
        context["parent_id"] = extend_dict.get("parentCopyId")
        context["backup_type"] = extend_dict.get("backupType")
        context["conf_file"] = extend_dict.get("pg_probackup.conf")
        context["version"] = extend_dict.get("protectObject", {}).get("type")
        context["pre_user"] = extend_dict.get("userName")
        context["restoreTimeStamp"] = job_dict.get("extendInfo", {}).get("restoreTimestamp", "")
        # 仅当任意时间点恢复时生成日志合并目录
        if context["restoreTimeStamp"]:
            logger.info(f'The restore timestamp: {context["restoreTimeStamp"]}')
            context["merged_log_path"] = new_instance.handle_log_copy(job_dict)
        env_path = job_dict.get("targetEnv", {}).get("extendInfo", {}).get("envPath")
        context["env_path"] = env_path if check_path(env_path) else ""
        if param_dict.get("subJob", {}):
            context["sub_job_name"] = param_dict.get("subJob", {}).get("jobName")
        return job_dict, RestoreOutput.get_media_path(context, job_dict)


if __name__ == '__main__':
    logger.info("Running restore main...")
    # 校验
    if len(sys.argv) < 4:
        logger.error("Invalid params to execute script.")
        sys.exit(ExecuteResultEnum.INTERNAL_ERROR)
    func_type = sys.argv[1]
    JobData.PID = sys.argv[2]
    JobData.JOB_ID = sys.argv[3]
    # 校验pid,job_id
    if not is_valid_id(JobData.PID):
        logger.warn(f'pid is invalid!')
        sys.exit(0)
    if not is_valid_id(JobData.JOB_ID):
        logger.warn(f'job_id is invalid!')
        sys.exit(0)
    logger.info(f'Start to exec function: {func_type},PID {JobData.PID}   task ID: {JobData.JOB_ID}')
    Env.OPEN_GAUSS_USER = 'job_targetEnv_auth_authKey'
    new_instance = RestoreOutput()
    if func_type == "AllowRestore":
        new_instance.output_code.code = ExecuteResultEnum.SUCCESS
        output_result_file(JobData.PID, new_instance.output_code.dict(by_alias=True))
        sys.exit(new_instance.output_code.code)
    job_log_dict, input_info = new_instance.parse_param()
    if not input_info:
        logger.error('context err')
        sys.exit(ExecuteResultEnum.INTERNAL_ERROR)
    if not new_instance.param_check(JobData.PID, JobData.JOB_ID, input_info):
        logger.error(f'Bad param. pid: {JobData.PID}')
        sys.exit(ExecuteResultEnum.INTERNAL_ERROR)
    check_ins = CheckRestore(input_info)
    exec_ins = ExecRestore(input_info)
    if func_type == "RestoreGenSubJob":
        sub_jobs = exec_ins.gen_sub_job()
        output_result_file(JobData.PID, sub_jobs)
        sys.exit(0)
    if func_type == "RestorePrerequisite":
        pre_job_err = check_ins.pre_restore_job()
        if input_info.get("backup_type") == "LOG":
            new_instance.pre_merge_logs(job_log_dict, input_info)
        err_code_file = os.path.join(input_info.get("cache_path"), f'{JobData.JOB_ID}errcode')
        record_err_code(pre_job_err.value, err_code_file)
        new_instance.output_code.code = ExecuteResultEnum.SUCCESS
        new_instance.output_code.body_err = 0
        new_instance.output_code.message = ' '
    elif func_type == "RestorePrerequisiteProgress":
        pre_job_err = query_err_code(os.path.join(input_info.get("cache_path"), f'{JobData.JOB_ID}errcode'))
        if pre_job_err == NormalErr.NO_ERR or pre_job_err == NormalErr.WAITING:
            new_instance.err_log.log_detail = None
        else:
            new_instance.err_log.log_detail = pre_job_err
            new_instance.progress_info.log_detail.append(new_instance.err_log)
        progress_file = os.path.join(input_info.get("cache_path"), f'{JobData.JOB_ID}pre_job')
        progress, status = read_progress(progress_file)
        new_instance.progress_info.task_id = JobData.JOB_ID
        new_instance.progress_info.progress = progress
        new_instance.progress_info.task_status = status
        output_result_file(JobData.PID, new_instance.progress_info.dict(by_alias=True))
        sys.exit(0)
    elif func_type == "Restore":
        sub_job_name = input_info.get("sub_job_name")
        logger.info(f"sub job name: {sub_job_name}. pid: {JobData.PID}. job id: {JobData.JOB_ID}")
        if sub_job_name == SubJobType.PREPARE_RESTORE:
            new_instance.output_code.code, new_instance.output_code.body_err, new_instance.output_code.message \
                = exec_ins.do_prepare_restore()
        if sub_job_name == SubJobType.RESTORE:
            new_instance.output_code.code, new_instance.output_code.body_err, new_instance.output_code.message \
                = exec_ins.do_restore_job_restore()
        if sub_job_name == SubJobType.END_TASK:
            new_instance.output_code.code, new_instance.output_code.body_err, new_instance.output_code.message \
                = exec_ins.do_restore_job_endtask()
        if sub_job_name == SubJobType.RESTART:
            new_instance.output_code.code, new_instance.output_code.body_err, new_instance.output_code.message \
                = exec_ins.do_restore_job_restart()
    elif func_type == "RestoreProgress":
        progress, status = exec_ins.query_restore_progress(input_info.get("sub_job_name"))
        new_instance.progress_info.task_id = JobData.JOB_ID
        new_instance.progress_info.sub_task_id = sys.argv[4]
        new_instance.progress_info.progress = progress
        new_instance.progress_info.task_status = status
        inst = GaussCluster(safe_get_environ(f"{Env.OPEN_GAUSS_USER}_{JobData.PID}"), input_info.get("env_path"))
        new_instance.progress_info.data_size, new_instance.progress_info.speed = \
            query_speed(inst.get_instance_data_path(), os.path.join(input_info.get("cache_path"), f'T{JobData.JOB_ID}'))
        output_result_file(JobData.PID, new_instance.progress_info.dict(by_alias=True))
        sys.exit(0)
    elif func_type == "RestorePost":
        restore_post_file = os.path.join(input_info.get("cache_path"), f'T{JobData.JOB_ID}')
        if restore_post_file.startswith(WhitePath.MOUNT_PATH):
            if not su_exec_rm_cmd(restore_post_file):
                logger.warn(f"Execute remove file command failed! File path: {restore_post_file}.")
                sys.exit(0)
        merged_log_path = input_info.get("merged_log_path")
        if os.path.exists(merged_log_path):
            shutil.rmtree(merged_log_path)
            logger.info("Delete the temporary log copy merge directory success.")
        user_name = safe_get_environ(f"{Env.OPEN_GAUSS_USER}_{JobData.PID}")
        inst = GaussCluster(user_name, input_info.get("env_path"))
        status = inst.cluster_state
        if status == "Normal":
            new_instance.output_code.code = ExecuteResultEnum.SUCCESS
        else:
            new_instance.output_code.code = ExecuteResultEnum.INTERNAL_ERROR
        exec_ins.stop_recovery_status()
    elif func_type == "RestorePostProgress":
        user_name = safe_get_environ(f"{Env.OPEN_GAUSS_USER}_{JobData.PID}")
        inst = GaussCluster(user_name, input_info.get("env_path"))
        status = inst.cluster_state
        if status == "Normal":
            new_instance.progress_info.progress = 100
            new_instance.progress_info.task_status = SubJobStatusEnum.COMPLETED
        else:
            new_instance.progress_info.progress = 0
            new_instance.progress_info.task_status = SubJobStatusEnum.FAILED
        new_instance.progress_info.task_id = JobData.JOB_ID
        new_instance.progress_info.sub_task_id = sys.argv[4]
        output_result_file(JobData.PID, new_instance.progress_info.dict(by_alias=True))
        sys.exit(0)
    else:
        logger.error(f'Function {func_type} is not supported!')
    output_result_file(JobData.PID, new_instance.output_code.dict(by_alias=True))
    sys.exit(new_instance.output_code.code)
