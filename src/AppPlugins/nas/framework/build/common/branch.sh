#!/bin/sh
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
if [ -z "${SDK_BRANCH}" ];then
    SDK_BRANCH=${branch}
fi
if [ -z "${SDK_BRANCH}" ];then
    SDK_BRANCH=master_OceanProtect_DataBackup
fi
echo "[$(date "+%Y-%m-%d %H:%M:%S")][INFO]  Current SDK_BRANCH is ${SDK_BRANCH}"

# DME_Framework的代码仓分支
if [ -z "${FRAMEWORK_BRANCH}" ];then
    FRAMEWORK_BRANCH=BR_Dev
fi
echo "[$(date "+%Y-%m-%d %H:%M:%S")][INFO]  Current FRAMEWORK_BRANCH is ${FRAMEWORK_BRANCH}"

# Data_Transmission_Frame的代码仓分支
if [ -z "${TRANSMISSION_BRANCH}" ];then
    TRANSMISSION_BRANCH=BR_Dev
fi
echo "[$(date "+%Y-%m-%d %H:%M:%S")][INFO]  Current TRANSMISSION_BRANCH is ${TRANSMISSION_BRANCH}"

