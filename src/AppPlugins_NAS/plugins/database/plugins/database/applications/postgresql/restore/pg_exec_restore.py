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

import sys

from common import cleaner
from common.common import exter_attack
from common.common_models import LogDetail, SubJobDetails
from common.const import ExecuteResultEnum, SysData, ReportDBLabel, SubJobStatusEnum
from common.logger import Logger
from common.util.check_utils import is_valid_id
from postgresql.common.const import PgConst, RestoreAction, RestoreSubJob, InstallDeployType
from postgresql.common.error_code import ErrorCode
from postgresql.common.models import RestoreProgress
from postgresql.common.util.pg_common_utils import PostgreCommonUtils
from postgresql.common.util.pg_param import JsonParam
from postgresql.restore.pg_full_restore_cluster_inst import ClusterInstFullRestore
from postgresql.restore.pg_full_restore_single_inst import SingleInstFullRestore
from postgresql.restore.pg_pit_restore_cluster_inst import ClusterInstPitRestore
from postgresql.restore.pg_pit_restore_single_inst import SingleInstPitRestore
from postgresql.restore.pg_restore_service import PostgreRestoreService

LOGGER = Logger().get_logger(filename="postgresql.log")


@exter_attack
def distribute_task(task_name, pid, job_id, param_dict, sub_job_id=None):
    # 拆分恢复子任务（集群）
    if task_name == "RestoreGenSubJob":
        LOGGER.info(f"[param]RestoreGenSubJob param: {param_dict}")
        PostgreRestoreService.gen_sub_job(pid, job_id, param_dict)
        return
    restore_timestamp = param_dict.get("job", {}).get("extendInfo", {}).get("restoreTimestamp")
    # 按时间点恢复
    if restore_timestamp:
        LOGGER.info(f"The current task is point-in-time recovery, restore time stamp: {restore_timestamp}.")
        if PostgreRestoreService.is_restore_cluster(param_dict):
            pg_restore_base_inst = ClusterInstPitRestore(pid, job_id, sub_job_id, param_dict)
        else:
            pg_restore_base_inst = SingleInstPitRestore(pid, job_id, sub_job_id, param_dict)
    else:
        LOGGER.info(f"The current task is not point-in-time recovery.")
        if PostgreRestoreService.is_restore_cluster(param_dict):
            pg_restore_base_inst = ClusterInstFullRestore(pid, job_id, sub_job_id, param_dict)
        else:
            pg_restore_base_inst = SingleInstFullRestore(pid, job_id, sub_job_id, param_dict)
    cache_path = PostgreRestoreService.get_cache_mount_path(param_dict)
    if task_name == "RestorePrerequisite":
        LOGGER.info(f"[param]RestorePrerequisite param: {param_dict}")
        PostgreCommonUtils.write_progress_info(cache_path, RestoreAction.QUERY_RESTORE_PRE,
                                               RestoreProgress(progress=1, message="restore pre task started"))
        process_restore_pre_task(pid, param_dict, cache_path, pg_restore_base_inst)
    elif task_name == "Restore":
        LOGGER.info(f"[param]Restore param: {param_dict}")
        PostgreCommonUtils.write_progress_info(cache_path, RestoreAction.QUERY_RESTORE,
                                               RestoreProgress(progress=1, message="restore task started"))
        process_restore_task(pid, param_dict, cache_path, pg_restore_base_inst)
    elif task_name == "RestorePostJob":
        LOGGER.info(f"[param]RestorePostJob param: {param_dict}")
        PostgreCommonUtils.write_progress_info(cache_path, RestoreAction.QUERY_RESTORE_POST,
                                               RestoreProgress(progress=1, message="restore post task started"))
        pg_restore_base_inst.exec_post_task()
    else:
        LOGGER.error(f"Unsupported job action param: {task_name}, pid: {pid}, job_id: {job_id}, "
                     f"sub_job_id: {sub_job_id}")
        err_msg = f"Unsupported job action param: {task_name}."
        PostgreRestoreService.record_task_result(pid, err_msg, code=ExecuteResultEnum.INTERNAL_ERROR.value)


def process_restore_task(pid, param_dict, cache_path, pg_restore_base_inst):
    # 主动上报开始执行恢复子任务label
    log_detail = LogDetail(logInfo=ReportDBLabel.RESTORE_SUB_START_COPY, logInfoParam=[pg_restore_base_inst.sub_job_id],
                           logLevel=1)
    PostgreCommonUtils.report_job_details(pid, SubJobDetails(taskId=pg_restore_base_inst.job_id,
                                                             subTaskId=pg_restore_base_inst.sub_job_id, progress=100,
                                                             logDetail=[log_detail],
                                                             taskStatus=SubJobStatusEnum.RUNNING.value).dict(
        by_alias=True))
    if PostgreRestoreService.is_restore_cluster(param_dict):
        # 根据子任务名称执行任务
        sub_job_name = ""
        if "subJob" in param_dict:
            sub_job_name = param_dict.get("subJob", {}).get("jobName")
        LOGGER.info(f"Target instance is cluster when executing restore job, pid: {pid}, "
                    f"sub_job_name: {sub_job_name}.")
        if sub_job_name == RestoreSubJob.PREPARE:
            pg_restore_base_inst.exec_pre_subtask()
        elif sub_job_name == RestoreSubJob.RESTORE:
            pg_restore_base_inst.exec_restore_subtask()
        elif sub_job_name == RestoreSubJob.POST:
            pg_restore_base_inst.exec_post_subtask()
        else:
            LOGGER.error(f"Unsupported sub job name: {sub_job_name} for cluster instance recovery, pid: {pid}, "
                         f"job_id: {pg_restore_base_inst.job_id}, sub_job_id: {pg_restore_base_inst.sub_job_id}")
            PostgreCommonUtils.write_progress_info(cache_path, RestoreAction.QUERY_RESTORE_PRE,
                                                   RestoreProgress(progress=1, message="failed"))
            err_msg = f"Unsupported sub job name: {sub_job_name} for cluster instance recovery."
            PostgreRestoreService.record_task_result(pid, err_msg, code=ExecuteResultEnum.INTERNAL_ERROR.value)
    else:
        pg_restore_base_inst.exec_restore_subtask()


def process_restore_pre_task(pid, param_dict, cache_path, pg_restore_base_inst):
    if PostgreRestoreService.is_restore_cluster(param_dict):
        install_deploy_type = param_dict.get("job", {}).get("targetEnv", {}).get("extendInfo", {}).get(
            "installDeployType", InstallDeployType.PGPOOL)
        if install_deploy_type == InstallDeployType.PATRONI:
            # 如果Patroni版本号是3.2.0就不支持日志恢复
            extend_info = param_dict.get("job", {}).get("extendInfo", {})
            cache_path = PostgreRestoreService.get_cache_mount_path(param_dict)
            if "restoreTimestamp" in extend_info and not PostgreRestoreService.check_patroni_version():
                PostgreCommonUtils.write_progress_info(cache_path, RestoreAction.QUERY_RESTORE_PRE, RestoreProgress(
                    progress=100, message="check patroni version failed",
                    err_code=ErrorCode.PATRONI_VERSION_NOT_SUPPORT,
                    err_code_param=[PgConst.NOT_SUPPORT_PATRONI_VERSION]))
                PostgreRestoreService.record_task_result(
                    pid, "The current version of the patroni has vulnerabilities. Please upgrade the patroni.",
                    err_code=ErrorCode.PATRONI_VERSION_NOT_SUPPORT,
                    body_err_params=[PgConst.NOT_SUPPORT_PATRONI_VERSION])
                return
        LOGGER.info(f"Target instance is cluster when executing restore prerequisite job, pid: {pid}.")
        PostgreCommonUtils.write_progress_info(cache_path, RestoreAction.QUERY_RESTORE_PRE,
                                               RestoreProgress(progress=100, message="completed"))
        err_msg = "The cluster instance does not execute the RestorePrerequisite task."
        PostgreRestoreService.record_task_result(pid, err_msg, code=ExecuteResultEnum.SUCCESS.value)
    else:
        pg_restore_base_inst.exec_pre_task()


@exter_attack
def main():
    args = sys.argv[1:]
    for line in sys.stdin:
        SysData.SYS_STDIN = line
        break
    # 参数个数为3或者4
    if len(args) not in (3, 4):
        LOGGER.error(f"Param number error! Arguments: {args}")
        return 1
    task_name, pid, job_id = args[0], args[1], args[2]
    input_sub_job_id = args[3] if len(args) == 4 else None
    # 校验pid,job_id,input_sub_job_id
    if not is_valid_id(pid):
        LOGGER.warn(f'pid is invalid!')
        sys.exit(1)
    if not is_valid_id(job_id):
        LOGGER.warn(f'job_id is invalid!')
        sys.exit(1)
    if input_sub_job_id is not None:
        if not is_valid_id(input_sub_job_id):
            LOGGER.warn(f'sub_job_id is invalid!')
            sys.exit(1)
    LOGGER.info(f"Start executing task, task name: {task_name}, pid: {pid}, job_id: {job_id}, "
                f"sub_job_id: {input_sub_job_id}.")
    try:
        param_dict = JsonParam.parse_param_with_jsonschema(pid)
    except Exception:
        LOGGER.exception(f"Parse param file failed, task name: {task_name}, pid: {pid}.")
        err_msg = "Parse parameter file failed."
        PostgreRestoreService.record_task_result(pid, err_msg, code=ExecuteResultEnum.INTERNAL_ERROR.value)
        return 1

    if task_name in PgConst.RESTORE_PROGRESS_ACTIONS:
        try:
            PostgreRestoreService.report_progress(task_name, pid, job_id, param_dict, sub_job_id=input_sub_job_id)
        except Exception:
            LOGGER.exception(f"[query]Query task progress failed. Task Name: {task_name}, Pid: {pid}, "
                             f"Job ID: {job_id}, Sub Job ID: {input_sub_job_id}.")
            return 1
    else:
        try:
            distribute_task(task_name, pid, job_id, param_dict, sub_job_id=input_sub_job_id)
        except Exception:
            LOGGER.exception(f"Distribute restore task failed. Task Name: {task_name}, Pid: {pid}, Job ID: {job_id}, "
                             f"Sub Job ID: {input_sub_job_id}.")
            return 1
    LOGGER.info(f"Execute task success. Task Name: {task_name}, Pid: {pid}, Job ID: {job_id}, "
                f"Sub Job ID: {input_sub_job_id}.")
    return 0


if __name__ == '__main__':
    try:
        sys.exit(main())
    finally:
        cleaner.clear(SysData.SYS_STDIN)
        LOGGER.debug('Clear environment variables successfully.')
