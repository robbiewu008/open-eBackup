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
from common.const import SysData
from common.util.check_utils import is_valid_id


def process_handle(log, arg, std, fun):
    """
    功能描述: 处理进程
    参数:
    @log: 日志
    @arg: 参数列表
    @std: 标准输入
    @fun: 功能映射
    注意:
    标准输入数据为外部程序通过管道向插件程序注入的各项参数数据（可能携带敏感数据）
    样例:
    python3 xxxx.py Backup 1657539915784 2e0c6c0b faa3e2c0
    """
    # 打印参数长度
    arg_size = len(arg)
    log.info(f'Arguments length={arg_size}')

    for line in sys.stdin:
        SysData.SYS_STDIN = line
        break

    if arg_size >= 3:
        # sys.stdin标准输入的数据为外部程序通过管道向插件程序注入的各项参数数据（可能携带敏感数据）
        std_in = std.readline()
        # 如果sys.stdin标准输入的数据为空，则记录日志
        if len(std_in) == 0:
            log.warn('System stdin read line is empty')

        # 获取主要参数
        req_tp = arg[1]
        req_id = arg[2]
        if not is_valid_id(req_id):
            log.warn(f'req_id is invalid')
            clear(std_in)
            sys.exit(1)

        job_id = ''
        sub_id = ''

        # 获取主任务ID
        if arg_size >= 4:
            job_id = arg[3]
            if not is_valid_id(job_id):
                log.warn(f'job_id is invalid')
                clear(std_in)
                sys.exit(1)
        else:
            log.info(f'Can not fetch job_id')

        # 获取子任务ID
        if arg_size >= 5:
            sub_id = arg[4]
            if not is_valid_id(sub_id):
                log.warn(f'sub_id is invalid')
                clear(std_in)
                sys.exit(1)
        else:
            log.info(f'Can not fetch sub_id')

        # 打印日志
        log.info(f'Arguments={req_tp}, {req_id}, {job_id}, {sub_id}')

        # 交给具体函数操作
        try:
            fun[req_tp](req_id, job_id, sub_id, std_in)
        except Exception:
            log.exception(f"Run {req_tp} exception.")
        finally:
            clear(std_in)
    else:
        # 参数长度错误
        log.info(f'Arguments length={arg_size} error')
