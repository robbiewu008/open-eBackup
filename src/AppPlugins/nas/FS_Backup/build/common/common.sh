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
# 可复用的公共操作，在此脚本进行封装
# *注意*：若要兼容bash和ksh，必需使用. 的方式引入common，并事先定义COMMON_PATH!
if [ -z "${COMMON_PATH}" ]; then
    COMMON_PATH="$(cd $(dirname ${BASH_SOURCE[0]}); pwd)"  # 该命令在bash中获取的才是common路径，ksh中为调用者路径
fi
BACKUP_ROOT_PATH=$(cd "${COMMON_PATH}/../.."; pwd)


# 外部包存放路径
EXT_PKG_DOWNLOAD_PATH=${BACKUP_ROOT_PATH}/ext_pkg


# 依赖的DME框架Framework代码根目录名称
FRAMEWORK_ROOT_PATH=${BACKUP_ROOT_PATH}/${DME_ROOT_DIR_NAME}/DME_Framework

# 出包路径
NAS_PACKAGE_PATH=${BACKUP_ROOT_PATH}/output_pkg

# 产品名称
PRODUCT="dorado"

# 系统类型
OS_TYPE=$(uname -s)
if [ "${OS_TYPE}" = "AIX" ]; then
    export OBJECT_MODE=64
    export CFLAGS=-maix64 && export CXXFLAGS=-maix64  # AIX Cmake必需先声明这两个变量，否则cmake的test会编译报错
fi

log_echo()
{
    local type="DEBUG"
    local message="$1"
    if [ "$#" -eq 2 ];then
       type="$1"
       message="$2"
    fi
    echo "[$(date "+%Y-%m-%d %H:%M:%S")][${type}][$(whoami)][${SCRIPT_NAME}] ${message}"
}
