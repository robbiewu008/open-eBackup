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
from common.logger import Logger
from common.const import ExecuteResultEnum, SysData
from common.common import output_result_file
from common.common_models import ActionResult
from common.util.check_utils import is_valid_id
from postgresql.backup.pg_backup import PgBackup
from postgresql.common.const import ErrorCode
from postgresql.common.util.pg_param import JsonParam

LOGGER = Logger().get_logger("postgresql.log")


def exec_backup_task(command, pid, job_id, sub_job_id):
    try:
        param_dict = JsonParam.parse_param_with_jsonschema(pid)
    except Exception:
        LOGGER.exception(f"Failed to parse param file, job id: {job_id}.")
        output = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR.value,
                              body_err=ErrorCode.PLUGIN_CANNOT_BACKUP_ERR.value,
                              message="Failed to parse param file.")
        output_result_file(pid, output.dict(by_alias=True))
        return
    pg_backup = PgBackup(pid, job_id, sub_job_id, param_dict)

    func_dict = {
        "AllowBackupInLocalNode": pg_backup.allow_backup_in_local_node,
        "QueryJobPermission": pg_backup.query_job_permission,
        "CheckBackupJobType": pg_backup.check_backup_job_type,
        "BackupPrerequisite": pg_backup.backup_prerequisite,
        "BackupPrerequisiteProgress": pg_backup.query_prerequisite_progress,
        "Backup": pg_backup.backup,
        "BackupProgress": pg_backup.query_backup_progress,
        "BackupPostJob": pg_backup.backup_post_job,
        "BackupPostJobProgress": pg_backup.query_post_job_progress,
        "QueryBackupCopy": pg_backup.query_backup_copy,
        "AbortJob": pg_backup.abort_job,
        "AbortJobProgress": pg_backup.query_abort_job_progress
    }
    backup_func = func_dict.get(command)
    if not backup_func:
        LOGGER.error(f"Cannot found cmd, job id: {job_id}.")
        return
    LOGGER.info(f"Started to Exec task {command}, job id: {job_id}.")
    result = backup_func()
    if not result:
        LOGGER.error(f"Failed to Exec task {command}, job id: {job_id}.")
        output = pg_backup.get_action_result()
    else:
        LOGGER.info(f"Succeed to Exec task {command}, job id: {job_id}.")
        output = ActionResult(code=ExecuteResultEnum.SUCCESS.value)
    if command != "QueryBackupCopy" and command != "QueryJobPermission" and not command.endswith("Progress"):
        output_result_file(pid, output.dict(by_alias=True))


if __name__ == '__main__':
    args = sys.argv[1:]
    for line in sys.stdin:
        SysData.SYS_STDIN = line
        break
    if len(args) < 2 or len(args) > 4:
        LOGGER.error(f"Param error, the number of parameters is {len(args)}, args: {args}.")
        sys.exit(ErrorCode.INVALID_PARAMETER_ERR.value)
    JOB_ID = ""
    if len(args) > 2:
        JOB_ID = args[2]
    SUB_JOB_ID = ""
    if len(args) > 3:
        SUB_JOB_ID = args[3]
    # 校验pid,job_id,sub_job_id
    if not is_valid_id(args[1]):
        LOGGER.warn(f'pid is invalid!')
        sys.exit(1)
    if not is_valid_id(JOB_ID):
        LOGGER.warn(f'job_id is invalid!')
        sys.exit(1)
    if not is_valid_id(SUB_JOB_ID):
        LOGGER.warn(f'sub_job_id is invalid!')
        sys.exit(1)
    try:
        exec_backup_task(args[0], args[1], JOB_ID, SUB_JOB_ID)
    except Exception as err:
        LOGGER.error(f"Exec command error: {err}, job id: {JOB_ID}.")
        sys.exit(ErrorCode.INVALID_PARAMETER_ERR.value)
    finally:
        cleaner.clear(SysData.SYS_STDIN)
        LOGGER.info('Clearing data successfully')
