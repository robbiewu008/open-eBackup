#!/bin/bash
/# 
#  This file is a part of the open-eBackup project.
#  This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
#  If a copy of the MPL was not distributed with this file, You can obtain one at
#  http://mozilla.org/MPL/2.0/.
# 
#  Copyright (c) [2024] Huawei Technologies Co.,Ltd.
# 
#  THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
#  EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
#  MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# /
umask 0022
# 此脚本初始化所有相关路径信息，建议在其他脚本前执行

# Debug/Release
export BUILD_TYPE=Release
SCRIPT_PATH=$(dirname ${BASH_SOURCE[0]})

PROJECT_ROOT_PATH=$(cd "${SCRIPT_PATH}/../../../"; pwd)
# 虚拟化插件根路径 AppPlugins_NAS/plugins/virtualization/
#VIRT_ROOT_DIR=$(cd "${SCRIPT_PATH}/.."; pwd)

PLUGIN_VIRT_LIB_PATH="${PROJECT_ROOT_PATH}/libs"
FRAMEWORK_PATH="${PROJECT_ROOT_PATH}/framework"
MODULE_PATH="${PROJECT_ROOT_PATH}/Module"

numProc=$(cat /proc/cpuinfo | grep processor | wc -l)

# 虚拟化插件第三方组件依赖包下载路径 AppPlugins_Virtualization/deps/ext_pkg
VIRT_DEPS_EXT_PKG_PATH=${PROJECT_ROOT_PATH}/deps/ext_pkg

# 虚拟化插件第三方组件依赖包 解压代码存放路径 AppPlugins_Virtualization/deps/srcs
VIRT_DEPS_SRCS_PATH=${PROJECT_ROOT_PATH}/deps/src

# 虚拟化插件第三方组件安装路径 AppPlugins_Virtualization/deps/local
VIRT_DEPS_INSTALL_PATH=${PROJECT_ROOT_PATH}/deps/local

YAMLCPP=yaml-cpp-0.8.0

function log_echo()
{
    local type="DEBUG"
    local message="$1"
    if [ "$#" -eq 2 ];then
       type="$1"
       message="$2"
    fi
    echo "[$(date "+%Y-%m-%d %H:%M:%S")][${type}][$(whoami)][${SCRIPT_NAME}] ${message}"
}