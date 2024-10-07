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
WORKSPACE=$(cd "${CURRENT_DIR}/.."; pwd)
COMPOENT_VERSION=1.2.1RC1
OPEN_SRC_PATH=${WORKSPACE}/third_open_src
EXT_PKG_DOWNLOAD_PATH=${WORKSPACE}/ext_pkg

# 初始化参数，主要适配cmc的要求的配置
function init_env_para()
{
   system_name=$(cat /etc/os-release 2>/dev/null | grep -E "\<ID\>" | awk -F "=" '{print $2}'| tr -d '"')
   if [ "X${system_name}" == "Xeuleros" ];then
       SYSTEM_NAME="Euler2.9"
       OS_TYPE="ARM"
   else
       SYSTEM_NAME="CentOS7.9"
       OS_TYPE="X86"
   fi
}

# 从cmc上下载开源三方和platform编译包
function download_pkg_from_cmc()
{
    local ext_pkg_path=${EXT_PKG_DOWNLOAD_PATH}
    [ -d ${ext_pkg_path} ] && rm -rf ${ext_pkg_path}
    mkdir -p ${ext_pkg_path}
    artget pull -d ${WORKSPACE}/build/LCRP/conf/opensrc_platofrm_dependencies.xml -p "{'COMPOENT_VERSION':'${COMPOENT_VERSION}','PRODUCT':'dorado', \
    'THIRD_BRANCH':'BR_Dev','OS_TYPE':'${OS_TYPE}','SYSTEM_NAME':'${SYSTEM_NAME}'}" \
    -ap ${ext_pkg_path} -user ${cmc_user} -pwd ${cmc_pwd}
}

# 适配解压开源三方包到third_open_src目录
function uncompress_3rd_pkg()
{
    local pkg_name=OceanProtect_X8000_${COMPOENT_VERSION}_Opensrc_3rd_SDK_${SYSTEM_NAME}_${OS_TYPE}.tar.gz
    local ext_pkg_path=${EXT_PKG_DOWNLOAD_PATH}
    cd ${ext_pkg_path}
    if [ ! -f "${pkg_name}" ];then
        echo "${pkg_name} not exist, pls check"
        exit 1
    fi
    mkdir -p ${ext_pkg_path}/3rd
    tar -zxvf ${pkg_name} -C ${ext_pkg_path}/3rd
    if [ $? -ne 0 ];then
        echo "Uncompress ${pkg_name} failed, pls check"
        exit 1
    fi
    cd - >/dev/null

    [ -d ${WORKSPACE}/third_open_src ] && rm -rf ${WORKSPACE}/third_open_src/*_rel
    for rel_pkg_name in $(ls -1 ${ext_pkg_path}/3rd/)
    do
        local pkg_full_path=${ext_pkg_path}/3rd/${rel_pkg_name}
        if [ ! -z "${rel_pkg_name}" -a  -f "${pkg_full_path}" ];then
            tar -zxvf "${pkg_full_path}" -C ${WORKSPACE}/third_open_src
        fi
    done

    local agent_open_src_list="libevent|thrift"
    for rel_pkg_name in $(ls -1 ${ext_pkg_path}/3rd/agent/)
    do
        if [ $(echo "${rel_pkg_name}" | grep -Ec "${agent_open_src_list}") -eq 0 ];then
            continue
        fi
        local pkg_full_path=${ext_pkg_path}/3rd/agent/${rel_pkg_name}
        if [ ! -z "${rel_pkg_name}" -a  -f "${pkg_full_path}" ];then
            tar -zxvf "${pkg_full_path}" -C ${WORKSPACE}/third_open_src
        fi
    done
    cp -rf  ${WORKSPACE}/third_open_src/thrift_agent_rel/.libs/ ${WORKSPACE}/third_open_src/thrift_rel
    rm -rf ${WORKSPACE}/third_open_src/thrift_agent_rel
}

# 适配解压platform包到platform目录
function uncompress_platform_pkg()
{
    local pkg_name=OceanProtect_X8000_${COMPOENT_VERSION}_Platform_SDK_${SYSTEM_NAME}_${OS_TYPE}.tar.gz
    local ext_pkg_path=${EXT_PKG_DOWNLOAD_PATH}
    cd ${ext_pkg_path}
    if [ ! -f "${pkg_name}" ];then
        echo "${pkg_name} not exist, pls check"
        exit 1
    fi
    mkdir -p ${ext_pkg_path}/platform
    tar -zxvf ${pkg_name} -C ${ext_pkg_path}/platform
    if [ $? -ne 0 ];then
        echo "Uncompress ${pkg_name} failed, pls check"
        exit 1
    fi
    cd - >/dev/null

    [ -d ${WORKSPACE}/platform ] && rm -rf ${WORKSPACE}/platform/*_rel
    for rel_pkg_name in $(ls -1 ${ext_pkg_path}/platform/)
    do
        local pkg_full_path=${ext_pkg_path}/platform/${rel_pkg_name}
        if [ ! -z "${rel_pkg_name}" -a  -f "${pkg_full_path}" ];then
            tar -zxvf "${pkg_full_path}" -C ${WORKSPACE}/platform
        fi
    done
}

function main()
{
    init_env_para

    download_pkg_from_cmc

    uncompress_platform_pkg

    uncompress_3rd_pkg
}

main "$@"