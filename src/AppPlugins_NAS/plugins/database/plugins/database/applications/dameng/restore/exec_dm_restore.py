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

from dameng.commons.const import SysData
from dameng.restore.dm_restore import DMRestore
from common.logger import Logger
from common.const import JobData
from common.cleaner import clear

log = Logger().get_logger("dameng.log")


def exec_main(dm_args):
    log.info(f"Call backup script paramCnt:{len(dm_args)} args:{dm_args}.")
    for line in sys.stdin:
        SysData.SYS_STDIN = line
        break
    if SysData.SYS_STDIN == '':
        log.error(f"Failed to obtain sys-parameters.")
        return False
    if len(dm_args) < 2:
        log.error(f"Not enough parameters paramCnt:{len(dm_args)} args:{dm_args}.")
        clear(SysData.SYS_STDIN)
        return False
    cmd = dm_args[0]
    p_id = dm_args[1]
    JobData.PID = p_id
    job_id = ""
    sub_job_id = ""
    if len(dm_args) > 2:
        job_id = dm_args[2]
    if len(dm_args) > 3:
        sub_job_id = dm_args[3]
    JobData.JOB_ID = job_id
    JobData.SUB_JOB_ID = sub_job_id
    ret = DMRestore.check_cmd(cmd)
    if not ret:
        clear(SysData.SYS_STDIN)
        return False
    #单个任务有5种状态，初始化，执行，成功，失败，抛出异常,下面是简易的状态机实现
    dm_restore = DMRestore()
    try:
        ret = dm_restore.dispatch_task(cmd) if dm_restore.init_task(p_id, job_id, sub_job_id, cmd) else False
    except Exception as exception_str:
        log.error(f"Exec cmd failed. pid:{p_id} jobId:{job_id}.")
        dm_restore.exec_cmd_when_execpt(cmd)
        clear(SysData.SYS_STDIN)
        return False
    clear(SysData.SYS_STDIN)
    if ret:
        dm_restore.exec_cmd_when_success(cmd)
        return True
    else:
        dm_restore.exec_cmd_when_failed(cmd)
    return False


if __name__ == '__main__':
    args = sys.argv[1:]
    exec_main(args)