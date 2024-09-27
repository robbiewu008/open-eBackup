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
from common.common import exter_attack
from common.util.check_utils import is_valid_id
from mongodb import LOGGER
from mongodb.job_manager import JobManager


class Api:
    def __init__(self, func_type, pid, job_id, sub_job_id):
        self.interface = func_type
        self.pid = pid
        self.job_id = job_id
        self.sub_job_id = sub_job_id

    def create_job_manager(self):
        job_manager = JobManager(self.pid, self.job_id, self.sub_job_id)
        return job_manager

    @exter_attack
    def run(self):
        job_manager = self.create_job_manager()
        job_manager.run(self.interface)


def main(gbase_args):
    """
    功能描述：主任务执行
    参数：
    @fun_type：argv[0] 方法类型
    @pid：argv[1] PID
    @job_id：argv[2] 主任务任务ID
    @sub_job_id: argv[3] 子任务ID
    例：python3 api.py fun_type pid job_id sub_job_id
    """
    fun_type, pid, *other_info = gbase_args

    if not is_valid_id(pid):
        LOGGER.warn(f'pid is invalid')
        clear(sys.stdin)
        return 1

    job_id = ''
    sub_job_id = ''
    if len(other_info) > 0:
        job_id = other_info[0]
        if not is_valid_id(job_id):
            LOGGER.warn(f'job_id is invalid')
            clear(sys.stdin)
            return 1
    if len(other_info) > 1:
        sub_job_id = other_info[1]
        if not is_valid_id(sub_job_id):
            LOGGER.warn(f'sub_job_id is invalid')
            clear(sys.stdin)
            return 1

    api = Api(fun_type, pid, job_id, sub_job_id)
    LOGGER.info('Function: %s start, pid: %s, job_id: %s, sub_job_id: %s.', fun_type, pid, job_id, sub_job_id)
    try:
        api.run()
    except Exception as exception_str:
        LOGGER.exception(f'Function: {fun_type} exception {exception_str}.')
        return 1
    LOGGER.info('Function: %s end, pid: %s.', fun_type, pid)
    return 0


if __name__ == '__main__':
    if len(sys.argv) >= 3:
        args = sys.argv[1:]
        sys.exit(main(args))
