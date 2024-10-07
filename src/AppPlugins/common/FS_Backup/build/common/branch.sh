#!/bin/bash
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
# 当前工程使用的所有版本信息
if [ -z "${branch}" ];then
    BACKUP_BRANCH=develop_backup_software_1.6.0RC1
else
    BACKUP_BRANCH=${branch}
fi
echo "[$(date "+%Y-%m-%d %H:%M:%S")][INFO]  Current BACKUP_BRANCH is ${BACKUP_BRANCH}"

# MODULE的代码仓分支
if [ -z "${MODULE_BRANCH}" ]; then
    if [ -z "${branch}" ];then
        MODULE_BRANCH=develop_backup_software_1.6.0RC1
    else
        MODULE_BRANCH=${branch}
    fi
fi
echo "[$(date "+%Y-%m-%d %H:%M:%S")][INFO]  Current MODULE_BRANCH is ${MODULE_BRANCH}"
