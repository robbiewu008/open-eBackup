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
from common.process_handle import process_handle
from mysql import log
from mysql.src.resource.resource_ability import ResourceAbility

fun = {
    'CheckApplication': ResourceAbility.check_application,
    'ListApplicationResource': ResourceAbility.list_application_resource,
    'VerifyClusterNode': ResourceAbility.verify_cluster_node,
    'QueryCluster': ResourceAbility.query_cluster
}


# MySQL Plugin API 入口
if __name__ == '__main__':
    """
    功能描述:执行MySQL插件主程序
    参数:
    @1: 请求类型
    @2: 请求的ID
    @3: 主任务ID
    @4: 子任务ID
    输入:
    标准输入数据为外部程序通过管道向插件程序注入的各项参数数据（可能携带敏感数据）
    样例:
    python3 mysql_api.py Backup 1657539915784 2e0c6c0b faa3e2c0
    """
    # 打印日志
    log.info('Enter MySQL Plugin process')
    process_handle(log, sys.argv, sys.stdin, fun)