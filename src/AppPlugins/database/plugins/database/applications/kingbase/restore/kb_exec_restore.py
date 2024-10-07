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
from common.common import exter_attack, output_result_file
from common.common_models import ActionResult
from common.const import ExecuteResultEnum, SysData
from common.logger import Logger
from common.number_const import NumberConst
from common.parse_parafile import ParamFileUtil
from kingbase.common.const import BodyErrCode
from kingbase.common.models import RestoreProgress
from kingbase.restore.kb_full_restore import FullRestore
from kingbase.restore.kb_pitr_restore import PitrRestore
from kingbase.restore.kb_restore_service import KingbaseRestoreService

LOGGER = Logger().get_logger("kingbase.log")


def process_restore_task(pid, param_dict, kb_restore_base_inst):
    job_dict = param_dict.get("job", {})
    if KingbaseRestoreService.is_restore_cluster(job_dict):
        # 集群恢复任务，根据子任务名称执行任务
        sub_job_name = ""
        if "subJob" in param_dict:
            sub_job_name = param_dict.get("subJob", {}).get("jobName")
        LOGGER.info(f"Target instance is cluster when executing restore job, pid: {pid}, "
                    f"sub_job_name: {sub_job_name}.")
        # 集群前置子任务
        if sub_job_name == "prepare":
            kb_restore_base_inst.exec_pre_subtask()
        elif sub_job_name == "restore":
            kb_restore_base_inst.exec_task()
        elif sub_job_name == "init":
            kb_restore_base_inst.exec_init_sys_rman_task()
        elif sub_job_name == "post":
            kb_restore_base_inst.exec_post_task()
        else:
            LOGGER.error(f"Unsupported sub job name: {sub_job_name} for cluster instance recovery, pid: {pid}, "
                         f"job_id: {kb_restore_base_inst.job_id}, sub_job_id: {kb_restore_base_inst.sub_job_id}")
            output = ActionResult(
                code=ExecuteResultEnum.INTERNAL_ERROR.value, body_err=BodyErrCode.INVALID_PARAMETER_ERR.value,
                message=f"Unsupported sub job name: {sub_job_name} for cluster instance recovery.")
            output_result_file(pid, output.dict(by_alias=True))
    else:
        # 单实例恢复任务
        kb_restore_base_inst.exec_task()


def process_restore_pre_task(pid, param_dict, kb_restore_base_inst):
    job_dict = param_dict.get("job", {})
    cache_path = KingbaseRestoreService.get_cache_mount_path(job_dict)
    # 集群前置任务需拆分子任务，不在此处进行
    if KingbaseRestoreService.is_restore_cluster(job_dict):
        LOGGER.info(f"Target instance is cluster when executing restore prerequisite job, pid: {pid}.")
        KingbaseRestoreService.write_progress_info(cache_path, "QueryRestorePreProcess",
                                                   RestoreProgress(progress=NumberConst.HUNDRED, message="completed"))
    else:
        # 单实例前置任务
        kb_restore_base_inst.exec_pre_task()


def allow_backup_in_local_node(pid):
    LOGGER.info("Start exec allow_backup_in_local_node")
    output_result_file(
        pid,
        ActionResult(code=ExecuteResultEnum.SUCCESS.value, bodyErr=None, bodyErrParams=None).dict(
            by_alias=True)
    )


@exter_attack
def distribute_task(task_name, pid, job_id, param_dict, sub_job_id=None):
    restore_timestamp = param_dict.get("job", {}).get("extendInfo", {}).get("restoreTimestamp")
    # 按时间点恢复
    job_dict = param_dict.get("job", {})
    if restore_timestamp:
        LOGGER.info(f"The current task is point-in-time recovery, restore time stamp: {restore_timestamp}.")
        kb_restore_base_inst = PitrRestore(pid, job_id, sub_job_id, job_dict)
    else:
        # 全量恢复或增量恢复
        LOGGER.info(f"The current task is not point-in-time recovery, restore time stamp: {restore_timestamp}.")
        kb_restore_base_inst = FullRestore(pid, job_id, sub_job_id, job_dict)
    LOGGER.info(f"task_name:{task_name}")

    if task_name == 'RestoreGenSubJob':
        KingbaseRestoreService.gen_sub_job(pid, job_id, param_dict)
    elif task_name == "AllowRestoreInLocalNode":
        allow_backup_in_local_node(pid)
    elif task_name == "RestorePrerequisite":
        process_restore_pre_task(pid, param_dict, kb_restore_base_inst)
    elif task_name == "Restore":
        process_restore_task(pid, param_dict, kb_restore_base_inst)
    elif task_name == "RestorePostJob":
        kb_restore_base_inst.exec_post_task()
    elif task_name in ("QueryRestorePreProcess", "QueryRestoreProcess", "QueryRestorePostProcess"):
        kb_restore_base_inst.report_progress(task_name)
    else:
        LOGGER.error(f"Unsupported job action param: {task_name},  pid: {pid}, job_id: {job_id}, "
                     f"sub_job_id: {sub_job_id}")
        output = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR.value,
                              body_err=BodyErrCode.INVALID_PARAMETER_ERR.value,
                              message=f"Unsupported job action param: {task_name}.")
        output_result_file(pid, output.dict(by_alias=True))


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
    LOGGER.info(f"Start executing task, task name: {task_name}, pid: {pid}, job_id: {job_id}, "
                f"sub_job_id: {input_sub_job_id}.")
    try:
        param_dict = ParamFileUtil.parse_param_file(pid)
    except Exception:
        LOGGER.exception(f"Parse param file failed, task name: {task_name}, pid: {pid}.")
        err_msg = "Parse parameter file failed."
        KingbaseRestoreService.record_task_result(pid, err_msg, code=ExecuteResultEnum.INTERNAL_ERROR.value)
        return 1

    try:
        distribute_task(task_name, pid, job_id, param_dict, sub_job_id=input_sub_job_id)
    except Exception:
        LOGGER.exception(f"Distribute restore task failed. Task Name: {task_name}, Pid: {pid}, Job ID: {job_id}, "
                         f"Sub Job ID: {input_sub_job_id}.")
        err_msg = f"Distribute {task_name} task failed."
        KingbaseRestoreService.record_task_result(pid, err_msg, code=ExecuteResultEnum.INTERNAL_ERROR.value)
        return 1
    LOGGER.info(f"Execute task success. Task Name: {task_name}, Pid: {pid}, Job ID: {job_id}, "
                f"Sub Job ID: {input_sub_job_id}.")
    return 0


if __name__ == '__main__':
    try:
        sys.exit(main())
    finally:
        cleaner.clear(SysData.SYS_STDIN)
        LOGGER.info('Clearing data successfully')
