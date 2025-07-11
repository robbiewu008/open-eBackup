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

class CommErrCode:
    # 错误场景：执行数据库备份或者恢复操作时，由于执行命令异常，操作失败。
    # 原因：执行命令（{0}）异常({1})。
    # 建议：请联系技术支持工程师协助解决。
    # 参数列表：
    # 1.{0}：执行命令
    # 2.{0}：执行命令的异常输出
    FAILED_EXECUTE_COMMAND = 1577209989

    # 错误场景：任务执行过程中，请求参数中部分参数非法，导致任务失败。
    # 原因：请求参数非法。
    # 建议：请收集日志并联系技术支持工程师协助解决。
    PARAMS_IS_INVALID = 1593988925

    # 错误场景：操作失败。
    # 原因：操作失败。
    # 建议：请联系技术支持工程师协助解决。
    OPERATION_FAILED = 1677929219
