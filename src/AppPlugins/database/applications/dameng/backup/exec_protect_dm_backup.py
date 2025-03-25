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
import os
import sys

from dameng.commons.const import SysData, BackupStepEnum
from dameng.commons.const import ArrayIndex
from dameng.backup.protect_dm_backup import BackUp

from common.logger import Logger
from common.common import ParamConstant, output_execution_result
from common.parse_parafile import ParamFileUtil
from common.const import JobData
from common.cleaner import clear

log = Logger().get_logger("dameng.log")


def main():
    log.info("Running main... ")
    # 1. verify input param
    args = sys.argv[ArrayIndex.INDEX_FIRST_1:]
    if len(args) < ArrayIndex.INDEX_FIRST_3:
        log.error("Param error.")
        return False
    step = args[ArrayIndex.INDEX_FIRST_0]
    JobData.PID = args[ArrayIndex.INDEX_FIRST_1]
    log.info(f"Task info, step: {step}, PID: {JobData.PID}.")
    job_id = args[ArrayIndex.INDEX_FIRST_2]
    sub_job_id = ''
    if len(args) == ArrayIndex.INDEX_FIRST_4:
        sub_job_id = args[ArrayIndex.INDEX_FIRST_3]

    # 2. parse input param
    try:
        param_dict = ParamFileUtil.parse_param_file(JobData.PID)
    except Exception:
        log.exception(f"Job id: {job_id} exec step: {step} failed for failed to parse param file.")
        return False
    backup_type = param_dict.get("job", {}).get("jobParam", {}).get("backupType")
    if not backup_type:
        log.info(f"Failed get backup type. Job id: {job_id}.")
        return False

    # 3. execute task
    backup = BackUp(JobData.PID, job_id, sub_job_id, param_dict)
    job_dict = backup.get_job_dict()
    func = job_dict.get(step, None)
    if not func:
        log.error(f"Job id: {job_id} exec step: {step} failed for step: {step} not support.")
        return False

    result_exec = func()
    if step in [BackupStepEnum.ALLOW_BACKUP_IN_LOCAL_NODE, BackupStepEnum.CHECK_BACKUP_JOB_TYPE,
                BackupStepEnum.GENERATOR_SUB_JOB, BackupStepEnum.STOP_TASK, BackupStepEnum.QUERY_BACKUP_COPY,
                BackupStepEnum.QUERY_SCAN_REPOSITORIES, BackupStepEnum.PRE_PROGRESS, BackupStepEnum.BACKUP_PROGRESS]:
        if not result_exec:
            log.error(f"Job id: {job_id} exec step: {step} failed.")
            return False
        else:
            log.info(f"Job id: {job_id} exec {step} interface succeed.")
        file_name = "{}{}".format("result", JobData.PID)
        result_file = os.path.join(ParamConstant.RESULT_PATH, file_name)
        output_execution_result(result_file, result_exec)
        log.info(f"Exec step: {step} succeed, job id: {job_id}")
    else:
        log.info(f"Exec step: {step} already reported, job id: {job_id}")
    return True


if __name__ == "__main__":
    for line in sys.stdin:
        SysData.SYS_STDIN = line
        break
    if SysData.SYS_STDIN == '':
        log.error(f"Failed to obtain sys-parameters.")
    try:
        RESULT = main()
    except Exception as exception_str:
        clear(SysData.SYS_STDIN)
        log.error(f"Exec fail, exp: {exception_str}")
        sys.exit(1)
    clear(SysData.SYS_STDIN)
    if not RESULT:
        sys.exit(1)
    sys.exit(0)
