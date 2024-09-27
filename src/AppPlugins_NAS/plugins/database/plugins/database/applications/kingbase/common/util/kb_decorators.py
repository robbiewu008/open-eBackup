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

import functools

from common.const import ExecuteResultEnum
from common.logger import Logger
from kingbase.common.kb_exception import ErrCodeException
from kingbase.restore.kb_restore_service import KingbaseRestoreService


LOGGER = Logger().get_logger("kingbase.log")


def restore_exception_decorator(func):
    @functools.wraps(func)
    def inner(*args, **kwargs):
        pid = args[0].pid
        try:
            func(*args, **kwargs)
        except ErrCodeException as ex:
            err_code, err_msg = ex.error_code, ex.error_message
            LOGGER.exception(f"Execute task failed, function name: {func.__name__}, error code: {err_code}, "
                             f"error message: {err_msg}.")
            KingbaseRestoreService.write_progress_info_for_ex(str(func.__name__), args[0].job_dict,
                                                              err_code=err_code, err_msg=err_msg)
            KingbaseRestoreService.record_task_result(pid, err_msg, code=ExecuteResultEnum.INTERNAL_ERROR.value,
                                                     err_code=err_code)
        except Exception as ex:
            LOGGER.exception(f"Execute task failed. Function Name: {func.__name__}.")
            # 执行失败更新进度信息
            KingbaseRestoreService.write_progress_info_for_ex(str(func.__name__), args[0].job_dict)
            # 执行失败写结果文件
            err_msg = f"Execute task failed. Error Message: {str(ex)}."
            KingbaseRestoreService.record_task_result(pid, err_msg, code=ExecuteResultEnum.INTERNAL_ERROR.value)

    return inner