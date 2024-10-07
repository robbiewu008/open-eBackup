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
# artget操作封装
CURRENT_SCRIPT="${BASH_SOURCE[0]}"
SCRIPT_PATH=$(dirname ${BASH_SOURCE[0]})
LCRP_CONFIG_PATH=${SCRIPT_PATH}/../LCRP/conf

source ${SCRIPT_PATH}/common.sh

OPEN_SRC_EXT_PKG_PATH=${EXT_PKG_DOWNLOAD_PATH}/nas_open_src
OPEN_SRC_INNER_PACKET_PATH=${SCANNER_ROOT_PATH}/third_open_src/output_pkg
SCRIPT_NAME="${BASH_SOURCE[0]##*/}"
DEFAULT_COMPONENT_VERSION="1.1.0"

function init_artget_env()
{
    if [ -z ${LCRP_HOME} ]; then
        log_echo "ERROR" "Cant't find LCRP_HOME env, please config LCRP tool first"
        exit 1
    fi
    local arch_type=$(uname -m)
    if [ "$arch_type" == "aarch64" ]; then
        ARCH="euler-arm"
    else
        ARCH="euler-x86"
    fi
    local system_name=$(cat /etc/os-release 2>/dev/null | grep -E "\<ID\>" | awk -F "=" '{print $2}'| tr -d '"')
    if [ "X${system_name}" == "Xcentos" ];then
        ARCH="x86_64"
    fi
}

function download_artifact()
{
    local product="$1"
    local code_branch="$2"
    local componentType="$3"
    if [ -z "${PRODUCT}" -o -z "${CODE_BRANCH}" -o -z ${componentType} ]; then
        log_echo "ERROR" "Some variable is empty, please check"
        return 1
    fi

    local componentVersion="$4"
    if [ -z "${componentVersion}" ]; then
        componentVersion="${DEFAULT_COMPONENT_VERSION}"
    fi

    log_echo "DEBUG" "Product name ${product}"
    log_echo "DEBUG" "Use branch ${code_branch}"
    log_echo "DEBUG" "Component Version:${componentVersion}"
    log_echo "DEBUG" "Component Type:${componentType}"

    init_artget_env

    mkdir -p ${EXT_PKG_DOWNLOAD_PATH}/${componentType}
    cd ${LCRP_CONFIG_PATH}
    artget pull -d pkg_from_cmc.xml -p "{'componentVersion':'${componentVersion}','PRODUCT':'${product}', \
    'CODE_BRANCH':'${code_branch}','COMPONENT_TYPE':'${componentType}', 'ARCH':'${ARCH}'}" \
    -ap ${EXT_PKG_DOWNLOAD_PATH}/${componentType} -user ${cmc_user} -pwd ${cmc_pwd} > /dev/null

    if [ $? -ne 0 ]; then
        log_echo "ERROR" "Download artifact from cmc error"
        return 1
    fi

    log_echo "INFO" "Finish to download pkgs from cmc"
    echo "The pkgs in ${EXT_PKG_DOWNLOAD_PATH}/${componentType} :"
    ls -l ${EXT_PKG_DOWNLOAD_PATH}/${componentType}
    return 0
}

function download_opensource()
{
    cd ${LCRP_CONFIG_PATH}
    mkdir -p ${OPEN_SRC_EXT_PKG_PATH}
    artget pull -os dependency_opensource.xml -ap ${OPEN_SRC_EXT_PKG_PATH} -user ${cmc_user} -pwd ${cmc_pwd} -at opensource > /dev/null
    if [ $? -ne 0 ]; then
        log_echo "ERROR" "download third package from opensource-central failed!"
        return 1
    fi
    log_echo "INFO" "Finish to download pkgs from cmc"
    echo "The pkgs in ${OPEN_SRC_EXT_PKG_PATH} :"
    ls -l ${OPEN_SRC_EXT_PKG_PATH}
    return 0
}

function upload_plugin_2_cmc()
{
    local product="$1"
    local code_branch="$2"
    local componentType="$3"
    if [ -z "${PRODUCT}" -o -z "${CODE_BRANCH}" -o -z ${componentType} ]; then
        log_echo "ERROR" "Some variable is empty, please check"
        return 1
    fi

    local componentVersion="$4"
    if [ -z "${componentVersion}" ]; then
        componentVersion="${DEFAULT_COMPONENT_VERSION}"
    fi

    log_echo "DEBUG" "Product name ${product}"
    log_echo "DEBUG" "Use branch ${code_branch}"
    log_echo "DEBUG" "Component Version:${componentVersion}"
    log_echo "DEBUG" "Component Type:${componentType}"

    local arch_type=$(uname -m)
    if [ "$arch_type" == "aarch64" ]; then
        ARCH="aarch64"
    else
        ARCH="x86_64"
    fi

    mkdir -p ${EXT_PKG_DOWNLOAD_PATH}/${componentType}
    cd ${LCRP_CONFIG_PATH}
    artget push -d pkg_into_cmc.xml -p "{'componentVersion':'${componentVersion}','PRODUCT':'${product}', \
    'CODE_BRANCH':'${code_branch}','COMPONENT_TYPE':'${componentType}', 'ARCH':'Linux'}" \
    -ap "${NAS_PACKAGE_PATH}/${PLUGIN_NAME}.tar.gz" -user ${cmc_user} -pwd ${cmc_pwd}

    if [ $? -ne 0 ]; then
        log_echo "ERROR" "Upload artifact from cmc error"
        return 1
    fi

    log_echo "INFO" "Finish to upload pkgs from cmc"
    return 0
}

function upload_nas_3rd_2_cmc()
{
    local product="$1"
    local code_branch="$2"
    local componentType="Plugins"
    if [ -z "${PRODUCT}" -o -z "${CODE_BRANCH}" ]; then
        log_echo "ERROR" "Some variable is empty, please check"
        return 1
    fi

    local componentVersion="$3"
    if [ -z "${componentVersion}" ]; then
        componentVersion="${DEFAULT_COMPONENT_VERSION}"
    fi
    local src_name="NasOpenSource"

    log_echo "DEBUG" "Product name ${product}"
    log_echo "DEBUG" "Use branch ${code_branch}"
    log_echo "DEBUG" "Component Version:${componentVersion}"
    log_echo "DEBUG" "Component Type:${componentType}"

    init_artget_env

    mkdir -p ${EXT_PKG_DOWNLOAD_PATH}/${componentType}
    cd ${LCRP_CONFIG_PATH}
    artget push -d pkg_into_cmc.xml -p "{'componentVersion':'${componentVersion}','PRODUCT':'${product}', \
    'CODE_BRANCH':'${code_branch}','COMPONENT_TYPE':'${componentType}', 'ARCH':'${ARCH}/${src_name}'}" \
    -ap "${OPEN_SRC_INNER_PACKET_PATH}/" -user ${cmc_user} -pwd ${cmc_pwd}

    if [ $? -ne 0 ]; then
        log_echo "ERROR" "Upload artifact from cmc error"
        return 1
    fi

    log_echo "INFO" "Finish to upload pkgs from cmc"
    return 0
}

function download_nas_3rd_4_cmc()
{
    local product="$1"
    local code_branch="$2"
    local componentType="Plugins"
    if [ -z "${PRODUCT}" -o -z "${CODE_BRANCH}" ]; then
        log_echo "ERROR" "Some variable is empty, please check"
        return 1
    fi

    local componentVersion="$3"
    if [ -z "${componentVersion}" ]; then
        componentVersion="${DEFAULT_COMPONENT_VERSION}"
    fi
    local src_name="NasOpenSource"

    log_echo "DEBUG" "Product name ${product}"
    log_echo "DEBUG" "Use branch ${code_branch}"
    log_echo "DEBUG" "Component Version:${componentVersion}"
    log_echo "DEBUG" "Component Type:${componentType}"

    init_artget_env

    mkdir -p ${EXT_PKG_DOWNLOAD_PATH}/${componentType}
    cd ${LCRP_CONFIG_PATH}
    echo "{'componentVersion':'${componentVersion}','PRODUCT':'${product}', \
    'CODE_BRANCH':'${code_branch}','COMPONENT_TYPE':'${componentType}', 'ARCH':'${ARCH}/${src_name}'}"
    artget pull -d nas_3rd_from_cmc.xml -p "{'componentVersion':'${componentVersion}','PRODUCT':'${product}', \
    'CODE_BRANCH':'${code_branch}','COMPONENT_TYPE':'${componentType}', 'ARCH':'${ARCH}/${src_name}'}" \
    -ap ${EXT_PKG_DOWNLOAD_PATH}/${componentType} -user ${cmc_user} -pwd ${cmc_pwd} > /dev/null

    if [ $? -ne 0 ]; then
        log_echo "ERROR" "Download artifact from cmc error"
        return 1
    fi

    log_echo "INFO" "Finish to download pkgs from cmc"
    echo "The pkgs in ${EXT_PKG_DOWNLOAD_PATH}/${componentType} :"
    ls -l ${EXT_PKG_DOWNLOAD_PATH}/${componentType}
    return 0
}

function download_Agent_SDK()
{
    local product="$1"
    local code_branch="$2"
    local componentType="PluginSDK"
    if [ -z "${product}" -o -z "${code_branch}" ]; then
        log_echo "ERROR" "Some variable is empty, please check"
        return 1
    fi

    local componentVersion="$3"
    if [ -z "${componentVersion}" ]; then
        componentVersion="${DEFAULT_COMPONENT_VERSION}"
    fi
    # 只有BRANCH_LIST列表取各自的分支，其他分支均使用默认SUB_SYSTEM分支
    BRANCH_LIST="master BR_Dev BR_SMOKE"
    DEFAULE_BRANCH="SUB_SYSTEM"
    local is_contain_branch=$(echo "${BRANCH_LIST}" | grep -c "\<${code_branch}\>")
    if [ ${is_contain_branch} -le 0 ];then
        code_branch=${DEFAULE_BRANCH}
    fi
    log_echo "DEBUG" "Product name ${product}"
    log_echo "DEBUG" "Use branch ${code_branch}"
    log_echo "DEBUG" "Component Version:${componentVersion}"
    log_echo "DEBUG" "Component Type:${componentType}"

    local arch_type=$(uname -m)
    if [ "$arch_type" == "aarch64" ]; then
        ARCH="aarch64"
    else
        ARCH="euler-x86"
    fi
    local system_name=$(cat /etc/os-release 2>/dev/null | grep -E "\<ID\>" | awk -F "=" '{print $2}'| tr -d '"')
    if [ "X${system_name}" == "Xcentos" ];then
        ARCH="x86_64"
    fi

    mkdir -p ${EXT_PKG_DOWNLOAD_PATH}/${componentType}
    cd ${LCRP_CONFIG_PATH}
    echo "{'componentVersion':'${componentVersion}','PRODUCT':'${product}', \
    'CODE_BRANCH':'${code_branch}','COMPONENT_TYPE':'${componentType}', 'ARCH':'${ARCH}'}"
    artget pull -d nas_3rd_from_cmc.xml -p "{'componentVersion':'${componentVersion}','PRODUCT':'${product}', \
    'CODE_BRANCH':'${code_branch}','COMPONENT_TYPE':'${componentType}', 'ARCH':'Linux/${ARCH}'}" \
    -ap ${SCANNER_ROOT_PATH}/${DME_ROOT_DIR_NAME}/Agent_SDK -user ${cmc_user} -pwd ${cmc_pwd} > /dev/null

    if [ $? -ne 0 ]; then
        log_echo "ERROR" "Download artifact from cmc error"
        return 1
    fi

    log_echo "INFO" "Finish to download pkgs from cmc"
    echo "The pkgs in ${SCANNER_ROOT_PATH}/${DME_ROOT_DIR_NAME}/Agent_SDK:"
    ls -l ${SCANNER_ROOT_PATH}/${DME_ROOT_DIR_NAME}/Agent_SDK
    return 0
}