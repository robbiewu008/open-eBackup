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
# 可复用的公共操作，在此脚本进行封装
# *注意*：若要兼容bash和ksh，必需使用. 的方式引入common，并事先定义COMMON_PATH!
if [ -z "${COMMON_PATH}" ]; then
    COMMON_PATH="$(cd $(dirname ${BASH_SOURCE[0]}); pwd)"  # 该命令在bash中获取的才是common路径，ksh中为调用者路径
fi
PLUGIN_ROOT_DIR=$(cd "${COMMON_PATH}/../.."; pwd)

# 外部包存放路径
EXT_PKG_DOWNLOAD_PATH=${PLUGIN_ROOT_DIR}/ext_pkg

# 依赖的DME框架包外层根目录名称
DME_ROOT_DIR_NAME=dep/dme

# 编译出包的路径
PLUGIN_PACKAGE_PATH=${PLUGIN_ROOT_DIR}/output_pkg

# 依赖的DME框架Framework代码根目录名称
FRAMEWORK_ROOT_PATH=${PLUGIN_ROOT_DIR}/${DME_ROOT_DIR_NAME}/Framework

# 出包路径
PLUGIN_FRAMEWORK_LIB_PATH=${PLUGIN_ROOT_DIR}/lib

# 插件类型，0 为外置，1 为内置。默认是0外置
PLUGIN_TYPE=0

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
    typeset type="DEBUG"
    typeset message="$1"
    if [ "$#" -eq 2 ];then
       type="$1"
       message="$2"
    fi
    echo "[$(date "+%Y-%m-%d %H:%M:%S")][${type}][$(whoami)][${SCRIPT_NAME}] ${message}"
}

# AIX中sed不支持-i选项，使用下面方法替换
sed_local_modify()
{
    typeset script=$1
    typeset fileName=$2
    if [ "${OS_TYPE}" = "AIX" ] || [ "${OS_TYPE}" = "SunOS" ]; then
        sed "${script}" ${fileName} > ${fileName}.bak
        mv ${fileName}.bak ${fileName}
    else
        sed -i "${script}" ${fileName}
    fi
}

# 插件名称可由插件自行定义
if [ -z "${PLUGIN_NAME}" ];then
    PLUGIN_NAME="${PLUGIN_NAME}"
    if [ ! -z "${PLUGIN_NAME}" ];then
        log_echo "DEBUG" "Current plugin name is ${PLUGIN_NAME}"
    fi
fi

# 默认生成的二进制名称
EXEC_FILE_NAME=AgentPlugin