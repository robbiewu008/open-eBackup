#!/bin/bash
#
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
#
umask 0022
# 此脚本初始化所有相关路径信息，建议在其他脚本前执行

# Debug/Release
export BUILD_TYPE=Release
SCRIPT_PATH=$(dirname ${BASH_SOURCE[0]})

PROJECT_ROOT_PATH=$(cd "${SCRIPT_PATH}/../../../"; pwd)
# 虚拟化插件根路径 AppPlugins_NAS/plugins/virtualization/
#VIRT_ROOT_DIR=$(cd "${SCRIPT_PATH}/.."; pwd)
OPEN_ROOT_PATH=$(cd "${SCRIPT_PATH}/../../../../../"; pwd)

PLUGIN_VIRT_LIB_PATH="${PROJECT_ROOT_PATH}/libs"
FRAMEWORK_PATH="${PROJECT_ROOT_PATH}/framework"
MODULE_PATH="${PROJECT_ROOT_PATH}/Module"
MODULE_OPEN_SRC_PATH="${MODULE_PATH}/third_open_src"
MODULE_PLATFORM_PATH="${MODULE_PATH}/platform"
OPENSOURCE_PATH="${OPEN_ROOT_PATH}/open-source-obligation/AppPlugins_virtualization"
OPENSOURCE_MODULE_PATH="${OPENSOURCE_PATH}/Module"
REST_API_PATH="${OPEN_ROOT_PATH}/REST_API/AppPlugins_virtualization/"
OPEN_VIRT_LIB_PATH="${REST_API_PATH}/libs"

numProc=$(cat /proc/cpuinfo | grep processor | wc -l)

# 虚拟化插件第三方组件依赖包下载路径 AppPlugins_Virtualization/deps/ext_pkg
VIRT_DEPS_EXT_PKG_PATH=${PROJECT_ROOT_PATH}/deps/ext_pkg

# 虚拟化插件第三方组件依赖包 解压代码存放路径 AppPlugins_Virtualization/deps/srcs
VIRT_DEPS_SRCS_PATH=${PROJECT_ROOT_PATH}/deps/src

# 虚拟化插件第三方组件安装路径 AppPlugins_Virtualization/deps/local
VIRT_DEPS_INSTALL_PATH=${PROJECT_ROOT_PATH}/deps/local

YAMLCPP=yaml-cpp-yaml-cpp-0.6.3

Framework_lib_list=(
    ${OPENSOURCE_PATH}/framework/lib
    ${OPENSOURCE_PATH}/framework/lib/agent_sdk
    
    ${OPENSOURCE_PATH}/lib
    ${OPENSOURCE_PATH}/deps/local/lib
)

Module_lib_list=(
    ${MODULE_OPEN_SRC_PATH}/lz4_rel/lib
    ${MODULE_OPEN_SRC_PATH}/boost_rel/lib
    ${MODULE_OPEN_SRC_PATH}/jsoncpp_rel/libs
    ${MODULE_OPEN_SRC_PATH}/curl_rel/lib/
    ${MODULE_OPEN_SRC_PATH}/thrift_rel/lib
    ${MODULE_OPEN_SRC_PATH}/libaio_rel/lib
    ${MODULE_OPEN_SRC_PATH}/tinyxml2_rel/lib
    ${MODULE_PLATFORM_PATH}/SecureCLib_rel/lib
)

Framework_inc_list=(
    ${OPENSOURCE_PATH}/framework/inc
    ${OPENSOURCE_PATH}/framework/inc/common
    ${OPENSOURCE_PATH}/framework/inc/client
    ${OPENSOURCE_PATH}/framework/inc/rpc
    ${OPENSOURCE_PATH}/framework/inc/rpc/certificateservice/
    ${OPENSOURCE_PATH}/framework/inc/thrift_interface
)
Module_inc_list=(
    ${OPENSOURCE_MODULE_PATH}/boost_rel/include
    ${OPENSOURCE_MODULE_PATH}/lz4_rel/include
    ${OPENSOURCE_MODULE_PATH}/jsoncpp_rel/include
    ${OPENSOURCE_MODULE_PATH}/curl_rel/include
    ${OPENSOURCE_MODULE_PATH}/openssl_rel/include
    ${OPENSOURCE_MODULE_PATH}/thrift_rel/include
    ${OPENSOURCE_MODULE_PATH}/libaio_rel/include
    ${OPENSOURCE_MODULE_PATH}/esdk_rel/include
    ${OPENSOURCE_MODULE_PATH}/tinyxml2_rel/include
    ${OPENSOURCE_MODULE_PATH}/libssh2_rel/include
    ${OPENSOURCE_MODULE_PATH}/platform/SecureCLib_rel/include
    ${OPENSOURCE_PATH}/deps/local/include
)

FusionStorage_list=(
    ${OPENSOURCE_PATH}/vbstool
    ${OPENSOURCE_PATH}/vbstool/conf
    ${OPENSOURCE_PATH}/vbstool/lib
)

FusionStorage_file_list=(
    ${OPENSOURCE_PATH}/vbstool/vrmVBSTool.sh
    ${OPENSOURCE_PATH}/vbstool/lib/vrmVBSTool.jar

)

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