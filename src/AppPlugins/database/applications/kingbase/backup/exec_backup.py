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

from common.cleaner import clear
from common.logger import Logger
from common.parse_parafile import ParamFileUtil
from common.common_models import ActionResult
from common.const import ExecuteResultEnum, SysData, SubJobStatusEnum
from common.common import output_result_file
from kingbase.backup.kb_backup import KbBackup
from kingbase.common.const import BodyErrCode
from kingbase.common.error_code import ErrorCode as ErrCode


LOGGER = Logger().get_logger("kingbase.log")


def get_backup_task_param(pid, job_id):
    try:
        param_dict = ParamFileUtil.parse_param_file(pid)
    except Exception as ex:
        LOGGER.error(f"Failed to parse param file, job id: {job_id}. Exception info: {ex}.")
        output = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR.value,
                              bodyErr=BodyErrCode.PLUGIN_CANNOT_BACKUP_ERR.value,
                              message="Failed to parse param file.")
        output_result_file(pid, output.dict(by_alias=True))
        return {}
    return param_dict


def exec_backup_task(command, pid, job_id, sub_job_id):
    """
    备份任务主函数
    :param command: str，命令名称
    :param pid: str，执行命令id
    :param job_id: str，任务id
    :param sub_job_id: str，子任务id
    :return: None
    """
    param_dict = get_backup_task_param(pid, job_id)
    if not param_dict:
        LOGGER.error(f"Failed to parse param file, job id: {job_id}.")
        return
    params = {
        "command": command,
        "pid": pid,
        "job_id": job_id,
        "sub_job_id": sub_job_id,
        "param_dict": param_dict
    }
    kb_backup = KbBackup(params)
    func_dict = {
        "AllowBackupInLocalNode": kb_backup.allow_backup_in_local_node,
        "QueryJobPermission": kb_backup.query_job_permission,
        "CheckBackupJobType": kb_backup.check_backup_job_type,
        "BackupPrerequisite": kb_backup.backup_prerequisite,
        "BackupPrerequisiteProgress": kb_backup.query_prerequisite_progress,
        "Backup": kb_backup.backup,
        "BackupProgress": kb_backup.query_backup_progress,
        "QueryBackupCopy": kb_backup.query_backup_copy,
        "BackupPostJob": kb_backup.backup_post_job,
        "BackupPostJobProgress": kb_backup.query_post_job_progress,
        "AbortJob": kb_backup.abort_job,
        "AbortJobProgress": kb_backup.query_abort_job_progress
    }
    if command not in func_dict:
        LOGGER.error(f"Invalid command {command}, job id: {job_id}.")
        output = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR.value,
                              bodyErr=BodyErrCode.PLUGIN_CANNOT_BACKUP_ERR.value, message=f"Invalid command {command}.")
        output_result_file(pid, output.dict(by_alias=True))
        return
    LOGGER.info(f"Started to exec task {command}, job id: {job_id}.")
    try:
        result = func_dict.get(command)()
    except Exception as ex:
        LOGGER.exception(f"Failed to exec task {command}, job id: {job_id}. Exception info: {ex}.")
        kb_backup.set_backup_result(SubJobStatusEnum.FAILED.value, ErrCode.BACKUP_FAILED,
                                    "Failed to execute backup.")
        output = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR.value,
                              bodyErr=ErrCode.BACKUP_FAILED, message="Failed to execute backup.")
        output_result_file(pid, output.dict(by_alias=True))
        return
    if not result:
        LOGGER.error(f"Failed to exec task {command}, job id: {job_id}.")
    else:
        LOGGER.info(f"Succeed to exec task {command}, job id: {job_id}.")
    if command != "QueryJobPermission" and not command.endswith("Progress") and command != "QueryBackupCopy":
        output = kb_backup.get_action_result()
        output_result_file(pid, output.dict(by_alias=True))


if __name__ == '__main__':
    SysData.SYS_STDIN = sys.stdin.readline()
    args = sys.argv[1:]
    if len(args) < 2 or len(args) > 4:
        LOGGER.error(f"Param error, the number of parameters is {len(args)}.")
        sys.exit(BodyErrCode.INVALID_PARAMETER_ERR.value)
    _command, _pid, _job_id, _sub_job_id = "", "", "", ""
    if len(args) == 2:
        _command, _pid, _job_id, _sub_job_id = args[0], args[1], "", ""
    if len(args) == 3:
        _command, _pid, _job_id, _sub_job_id = args[0], args[1], args[2], ""
    if len(args) == 4:
        _command, _pid, _job_id, _sub_job_id = args[0], args[1], args[2], args[3]
    try:
        exec_backup_task(_command, _pid, _job_id, _sub_job_id)
    except Exception as err:
        LOGGER.exception(f"Failed to exec command {_command}, pid: {_pid}, job id: {_job_id}, "
                         f"sub job id: {_sub_job_id}. Exception info: {err}.")
        sys.exit(BodyErrCode.INVALID_PARAMETER_ERR.value)
    finally:
        clear(SysData.SYS_STDIN)
