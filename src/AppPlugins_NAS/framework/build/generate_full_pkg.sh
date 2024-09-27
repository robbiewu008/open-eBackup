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
umask 0022
# entry of CI pipeline
CURRENT_SCRIPT="${BASH_SOURCE[0]}"
SCRIPT_PATH=$(cd $(dirname ${CURRENT_SCRIPT});pwd)
LCRP_CONFIG_PATH=${SCRIPT_PATH}/LCRP/conf
source ${SCRIPT_PATH}/common/common.sh
SCRIPT_NAME="${BASH_SOURCE[0]##*/}"
DEFAULT_COMPONENT_VERSION="1.1.0"
EXT_PKG_DOWNLOAD_PATH=${PLUGIN_ROOT_DIR}/ext_pkg
LINUX_SYSTEM_TYPE_LIST="x86_64 aarch64 "

function operate_cmc_pkg()
{
    local product="dorado"
    local componentVersion="${DEFAULT_COMPONENT_VERSION}"
    local arget_type="$1"
    local code_branch="$2"
    local componentType="$3"
    local arch_type="$4"
    if [ -z "${CODE_BRANCH}" -o -z ${componentType} ]; then
        log_echo "ERROR" "Some variable is empty, please check"
        return 1
    fi

    log_echo "DEBUG" "Use branch ${code_branch}"
    log_echo "DEBUG" "Component Version:${componentVersion}"
    log_echo "DEBUG" "Component Type:${componentType}"

    OS_TYPE=$(uname -s)

    mkdir -p ${EXT_PKG_DOWNLOAD_PATH}/${componentType}
    cd ${LCRP_CONFIG_PATH}
    if [ "X$arget_type" == "Xpush" ];then
        # INTERNAL_PLUGIN由云龙流水线定义，1：内置；0：外置
        if [ "${INTERNAL_PLUGIN}" == "1" ]; then
            OS_TYPE="${OS_TYPE}/Internal"
        fi
        artget push -d pkg_into_cmc.xml -p "{'componentVersion':'${componentVersion}','PRODUCT':'${product}', \
        'CODE_BRANCH':'${code_branch}','COMPONENT_TYPE':'${componentType}', 'ARCH':'${OS_TYPE}'}" \
        -ap "${PLUGIN_PACKAGE_PATH}/${PLUGIN_NAME}.tar.xz" -user ${cmc_user} -pwd ${cmc_pwd}
    else
        if [ "${INTERNAL_PLUGIN}" == "1" ]; then
            ARCH="${OS_TYPE}/${arch_type}/Internal"
        else
            ARCH="${OS_TYPE}/${arch_type}"
        fi
        artget pull -d nas_3rd_from_cmc.xml -p "{'componentVersion':'${componentVersion}','PRODUCT':'${product}', \
        'CODE_BRANCH':'${code_branch}','COMPONENT_TYPE':'${componentType}', 'ARCH':'${ARCH}/${PLUGIN_NAME}*.tar.xz'}" \
        -ap ${EXT_PKG_DOWNLOAD_PATH}/${componentType}/ -user ${cmc_user} -pwd ${cmc_pwd}
    fi
    if [ $? -ne 0 ]; then
        log_echo "ERROR" "handle cmc pkg error"
        return 1
    fi

    log_echo "INFO" "Finish to $arget_type pkg"
    return 0
}

function uncompress_pkg()
{
    rm -rf ${PLUGIN_PACKAGE_PATH}
    mkdir -p ${PLUGIN_PACKAGE_PATH}
    cd ${PLUGIN_PACKAGE_PATH}
    for pkg in $(ls -1 ${EXT_PKG_DOWNLOAD_PATH}/Plugins/${PLUGIN_NAME}_*.tar.xz)
    do
        local pkg_name=$(echo ${pkg} | awk -F '/' '{print $NF}' | awk -F '.' '{print $1}')
        if [ -z ${pkg_name} ];then
            log_echo "pkg[${pkg}] not exist"
            continue
        fi
        mkdir -p ${pkg_name}
        tar -xvf ${pkg} -C ${pkg_name}
        if [ $? -ne 0 ];then
            log_echo "pkg[${pkg}] format error"
            continue
        fi
        local local_type=$(echo "${pkg_name}" | sed "s/${PLUGIN_NAME}_//")
        local inner_pkg=$(ls -1 ${pkg_name}/*.tar.xz | grep "${PLUGIN_NAME}" | tail -1)
        if [ -z "${inner_pkg}" ];then
            log_echo "pkg[${pkg_name}] not exist"
            continue
        fi
        mv -f ${inner_pkg}  "${pkg_name}/${PLUGIN_NAME}_$(uname -s)_${local_type}.tar.xz"
        cp -rf ${pkg_name}/*.tar.xz .
        cp -rf ${pkg_name}/*.sh .
        cp -rf ${pkg_name}/*.json .
        rm -rf ${pkg_name}
    done

    echo "Repacking plugins..."
    local upload_pkg_name="${PLUGIN_NAME}.tar"
    echo "tar -cvf ${upload_pkg_name} *.tar.xz *.sh *.json"
    tar -cvf ${upload_pkg_name} *.tar.xz *.sh *.json
    echo "xz -v ${upload_pkg_name}"
    xz -v ${upload_pkg_name}
    rm -rf ${PLUGIN_NAME}_*.tar.xz *.sh *.json
}

function main()
{
    CODE_BRANCH="$1"
    if [ -z "${CODE_BRANCH}" ];then
        log_echo "Please input branch name."
        exit 1
    fi
    if [ -z "${APP_NAME}" ];then
        log_echo "Please input app name."
        exit 1
    fi
    if [ "${PLUGIN_NAME}X" == "X" ];then
        PLUGIN_NAME=$(cat ${PLUGIN_ROOT_DIR}/../plugins/${APP_NAME}/conf/plugin_*.json | grep name | tail -1 | awk -F \" '{print $4}')
        if [ "${PLUGIN_NAME}X" == "X" ]; then
            log_echo "ERROR" "plugin name is empty"
            return 1
        fi
    fi
    rm -rf ${EXT_PKG_DOWNLOAD_PATH}/
    log_echo "Begin down 3rd from cmc"
    for system_type in $LINUX_SYSTEM_TYPE_LIST
    do
        operate_cmc_pkg pull ${CODE_BRANCH} Plugins ${system_type}
        if [ $? -ne 0 ]; then
            log_echo "Download artifact error"
            continue
        fi
    done
    uncompress_pkg

   operate_cmc_pkg push ${CODE_BRANCH} Plugins
}

main "$@"
exit $?