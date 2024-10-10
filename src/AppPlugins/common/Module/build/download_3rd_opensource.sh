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
SCRIPT_PATH=$(dirname ${BASH_SOURCE[0]})
CURRENT_DIR=$(cd "${SCRIPT_PATH}"; pwd)
MODULE_ROOT=$(cd "${CURRENT_DIR}/.."; pwd)
COMPOENT_VERSION=1.2.1RC1
OPEN_SRC_PATH=${MODULE_ROOT}/third_open_src
EXT_PKG_DOWNLOAD_PATH=${MODULE_ROOT}/ext_pkg
OBLIGATION_ROOT=$1
if [ -z "$OBLIGATION_ROOT" ]; then
    echo "ERROR: Please provide open-source-obligation path"
    exit 1
fi

# 日构建环境使用branch
if [ -z "$THIRD_BRANCH" ];then
    THIRD_BRANCH=$branch
fi
if [ -z "$THIRD_BRANCH" ]; then
    THIRD_BRANCH="debug_OceanProtect_DataBackup_1.6.0_openeBackup_v2"
fi

init_env_para()
{
   system_name=$(uname -p)
   if [ "X${system_name}" == "Xaarch64" ];then
       SYSTEM_NAME="CentOS7.6"
       OS_TYPE="ARM"
   elif [ "X${CENTOS}" == "X6" ]; then
       SYSTEM_NAME="CentOS6.10"
       OS_TYPE="X86"
   else
       SYSTEM_NAME="CentOS7.6"
       OS_TYPE="X86"
   fi
}

# 拷贝开源三方包到third_open_src目录
uncompress_3rd_pkg()
{
    local pkg_name=${OBLIGATION_ROOT}/ThirdParty/${SYSTEM_NAME}/${OS_TYPE}/third_party_groupware/OceanProtect_X8000_${COMPOENT_VERSION}_Opensrc_3rd_SDK_${SYSTEM_NAME}_${OS_TYPE}.tar.gz
    local ext_pkg_path=${EXT_PKG_DOWNLOAD_PATH}
    cd ${ext_pkg_path}
    if [ ! -f "${pkg_name}" ];then
        echo "${pkg_name} not exist, pls check"
        exit 1
    fi
    mkdir -p ${ext_pkg_path}/3rd
    tar -zxf ${pkg_name} -C ${ext_pkg_path}/3rd
    if [ $? -ne 0 ];then
        echo "Uncompress ${pkg_name} failed, pls check"
        exit 1
    fi
    cd - >/dev/null

    [ -d ${MODULE_ROOT}/third_open_src ] && rm -rf ${MODULE_ROOT}/third_open_src/*_rel
    for rel_pkg_name in $(ls -1 ${ext_pkg_path}/3rd/)
    do
        local pkg_full_path=${ext_pkg_path}/3rd/${rel_pkg_name}
        if [ ! -z "${rel_pkg_name}" -a  -f "${pkg_full_path}" ];then
            tar -zxf "${pkg_full_path}" -C ${MODULE_ROOT}/third_open_src
        fi
    done

    local agent_open_src_list="libevent|thrift|openssl"
    for rel_pkg_name in $(ls -1 ${ext_pkg_path}/3rd/agent/)
    do
        if [ $(echo "${rel_pkg_name}" | grep -Ec "${agent_open_src_list}") -eq 0 ];then
            continue
        fi
        local pkg_full_path=${ext_pkg_path}/3rd/agent/${rel_pkg_name}
        if [ ! -z "${rel_pkg_name}" -a  -f "${pkg_full_path}" ];then
            tar -zxf "${pkg_full_path}" -C ${MODULE_ROOT}/third_open_src
        fi
    done
    mkdir -p ${MODULE_ROOT}/third_open_src/thrift_rel
    cp -rf  ${MODULE_ROOT}/third_open_src/thrift_agent_rel/.libs/* ${MODULE_ROOT}/third_open_src/thrift_rel
    rm -rf ${MODULE_ROOT}/third_open_src/thrift_agent_rel
    mv -f  ${MODULE_ROOT}/third_open_src/libevent_rel ${MODULE_ROOT}/third_open_src/libevent_rel_libs
    mkdir -p  ${MODULE_ROOT}/third_open_src/libevent_rel
    cp -rf ${MODULE_ROOT}/third_open_src/libevent_rel_libs/.libs/* ${MODULE_ROOT}/third_open_src/libevent_rel
    rm -rf ${MODULE_ROOT}/third_open_src/libevent_rel_libs
    cp -rf  ${MODULE_ROOT}/third_open_src/openssl_agent_rel/*.a ${MODULE_ROOT}/third_open_src/thrift_rel/lib
    rm -rf ${MODULE_ROOT}/third_open_src/openssl_agent_rel
}

# 适配解压platform包到platform目录
uncompress_platform_pkg()
{
    local pkg_name=${OBLIGATION_ROOT}/ThirdParty/${SYSTEM_NAME}/${OS_TYPE}/platform/OceanProtect_X8000_${COMPOENT_VERSION}_Platform_SDK_${SYSTEM_NAME}_${OS_TYPE}.tar.gz
    local ext_pkg_path=${EXT_PKG_DOWNLOAD_PATH}
    cd ${ext_pkg_path}
    if [ ! -f "${pkg_name}" ];then
        echo "${pkg_name} not exist, pls check"
        exit 1
    fi
    mkdir -p ${ext_pkg_path}/platform
    tar -zxf ${pkg_name} -C ${ext_pkg_path}/platform
    if [ $? -ne 0 ];then
        echo "Uncompress ${pkg_name} failed, pls check"
        exit 1
    fi
    cd - >/dev/null

    [ -d ${MODULE_ROOT}/platform ] && rm -rf ${MODULE_ROOT}/platform/*_rel
    for rel_pkg_name in $(ls -1 ${ext_pkg_path}/platform/)
    do
        local pkg_full_path=${ext_pkg_path}/platform/${rel_pkg_name}
        if [ ! -z "${rel_pkg_name}" -a  -f "${pkg_full_path}" ];then
            tar -zxf "${pkg_full_path}" -C ${MODULE_ROOT}/platform
        fi
    done
}

build_kmcv3()
{
    cd ${MODULE_ROOT}/src/KMCv3_infra/
    sh build.sh
    if [ $? -ne 0 ];then
        echo "build kmcv3 failed, pls check"
        exit 1
    fi
    cd -
    

}

function main()
{
    init_env_para

    uncompress_platform_pkg

    uncompress_3rd_pkg

    build_kmcv3
}

main "$@"

cd - >/dev/null
rm -rf ${MODULE_ROOT}/ext_pkg