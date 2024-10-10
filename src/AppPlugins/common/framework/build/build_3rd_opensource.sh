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
# build third-party software
SCRIPT_PATH=$(cd $(dirname $0); pwd)
FRAMEWORK_ROOT_DIR=$(cd "${SCRIPT_PATH}/.."; pwd)
MODULE_ROOT=$(cd "${FRAMEWORK_ROOT_DIR}/../Module" 2>/dev/null; pwd)
COMMON_PATH=${SCRIPT_PATH}/common
. ${COMMON_PATH}/branch.sh
. ${COMMON_PATH}/common_artget.sh
SCRIPT_NAME=$(basename $0)
OBLIGATION_ROOT=${binary_path}
if [ -z "$OBLIGATION_ROOT" ]; then
    log_echo "ERROR" "Please export binary_path={open-source-obligation path}"
    exit 1
fi

download_module_opensrc() {
    log_echo "DEBUG" "make module open src"
    if [ ! -d ${MODULE_ROOT} ];then
        log_echo "ERROR" "Failed to download module opensrc "
        exit 1
    fi

    if [ ! -f ${MODULE_ROOT}/build/download_3rd_opensource.sh ];then
        log_echo "ERROR" "no exist the script of module opensrc."
        exit 1
    fi

    sh ${MODULE_ROOT}/build/download_3rd_opensource.sh ${OBLIGATION_ROOT}
    if [ $? -ne 0 ]; then
        log_echo "ERROR" "download open src failed"
        exit 1
    fi
}

main()
{
    if [ "X$1" == "Xclean" ];then
        rm -rf ${MODULE_ROOT}/third_open_src/*_rel
        rm -rf ${MODULE_ROOT}/platform/*_rel
        rm -rf ${PLUGIN_FRAMEWORK_LIB_PATH}/agent_sdk
        return 0
    fi
    if [ "${OS_TYPE}" = "AIX" ] || [ "${OS_TYPE}" = "SunOS" ]; then
        log_echo "WARNING" "##########download_module_opensrc need to implement##########"
    else
        download_module_opensrc
        if [ $? -ne 0 ];then
            log_echo "ERROR" "download third pkg failed"
            return 1
        fi
    fi

    # 拷贝thrift目录下的boost库供插件可执行文件使用
    if [ "${OS_TYPE}" = "AIX" ]; then
        libName="*.a"
    else
        libName="*.so*"
    fi
    mkdir -p ${PLUGIN_FRAMEWORK_LIB_PATH}/3rd
    cp -rf ${MODULE_ROOT}/third_open_src/boost_rel/lib/$libName ${PLUGIN_FRAMEWORK_LIB_PATH}/3rd
    if [ "${OS_TYPE}" = "AIX" ]; then
        cp -rf ${MODULE_ROOT}/third_open_src/boost_rel/lib/*.so ${PLUGIN_FRAMEWORK_LIB_PATH}/3rd
    fi
    cp -rf ${MODULE_ROOT}/platform/SecureCLib_rel/lib/$libName ${PLUGIN_FRAMEWORK_LIB_PATH}/3rd
    cp -rf ${MODULE_ROOT}/third_open_src/jsoncpp_rel/libs/$libName ${PLUGIN_FRAMEWORK_LIB_PATH}/3rd
    cp -rf ${MODULE_ROOT}/third_open_src/openssl_rel/lib/$libName ${PLUGIN_FRAMEWORK_LIB_PATH}/3rd
    cp -rf ${MODULE_ROOT}/third_open_src/icu_rel/libs/lib/$libName ${PLUGIN_FRAMEWORK_LIB_PATH}/3rd
    if [ "X${PLUGIN_TYPE}" == "X1" ];then # internal
        cp -rf ${MODULE_ROOT}/third_open_src/curl_rel/lib/*.so* ${PLUGIN_FRAMEWORK_LIB_PATH}/3rd
        cp -rf ${MODULE_ROOT}/third_open_src/c-ares_rel/lib/*.so* ${PLUGIN_FRAMEWORK_LIB_PATH}/3rd
        cp -rf ${MODULE_ROOT}/third_open_src/libssh2_rel/lib/*.so* ${PLUGIN_FRAMEWORK_LIB_PATH}/3rd
    fi

    # 插件统一使用BR_Dev分支的agent SDK
    if [ "${OS_TYPE}" = "AIX" ] || [ "${OS_TYPE}" = "SunOS" ]; then
        log_echo "WARNING" "##########download_Agent_SDK need to implement##########"
        cd ${PLUGIN_ROOT_DIR}/dep/agent_sdk
    else
        arch_type=$(uname -m)
        mkdir -p ${FRAMEWORK_ROOT_DIR}/dep/agent_sdk
        tar xzf ${OBLIGATION_ROOT}/PluginSDK/Linux/${arch_type}/plugin_sdk.tar.gz -C ${FRAMEWORK_ROOT_DIR}/dep/agent_sdk
        cd ${FRAMEWORK_ROOT_DIR}/dep/agent_sdk
    fi
    mkdir -p ${PLUGIN_FRAMEWORK_LIB_PATH}/agent_sdk
    cp -rf lib/libpluginsdk*  ${PLUGIN_FRAMEWORK_LIB_PATH}/agent_sdk
    cd ${PLUGIN_ROOT_DIR}
}

main "$@"
