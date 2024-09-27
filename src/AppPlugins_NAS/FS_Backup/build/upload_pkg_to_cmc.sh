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
# build AppPlugins_NAS
CURRENT_SCRIPT="${BASH_SOURCE[0]}"
SCRIPT_PATH=$(cd "$(dirname ${BASH_SOURCE[0]})"; pwd)
SCRIPT_NAME="${BASH_SOURCE[0]##*/}"
BACKUP_ROOT=$(cd "${SCRIPT_PATH}/.."; pwd)
LCRP_CONFIG_PATH=${SCRIPT_PATH}/LCRP/conf

# 产品名称
PRODUCT="dorado"
DEFAULT_COMPONENT_VERSION="1.1.0"

CODE_BRANCH=$1
if [ -z "${CODE_BRANCH}" ]; then
    CODE_BRANCH="${branch}"
fi

function log_echo()
{
    local level="$1"
    local message="$2"
    echo "[$(date "+%Y-%m-%d %H:%M:%S")][$level][${SCRIPT_NAME}][$(whoami)] ${message}"
}

function compress_module_pkg()
{
    mkdir - ${BACKUP_ROOT}/BACKUP_rel
    cd ${BACKUP_ROOT}/BACKUP_rel
    mkdir - p ${BACKUP_ROOT}/BACKUP_rel/lib
    cp -r ${BACKUP_ROOT}/build-cmake/libBackup.so ${BACKUP_ROOT}/BACKUP_rel/lib
    mkdir -p ${BACKUP_ROOT}/BACKUP_rel/Module/
    MODULE_LIST="parser smb_ctx nfs_ctx threadpool metafile_parser system"
    for opensrc in ${MODULE_LIST}; do
        find ${BACKUP_ROOT}/Module/lib -name "*${opensrc}.so*" | xargs -I{} cp -f {} ${BACKUP_ROOT}/BACKUP_rel/Module
    done

    tar -zcvf BACKUP_rel.tar.gz  lib Module
    rm -rf lib Module
    cd - 2>/dev/null
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
    elif [ ${CENTOS} == "6" ]; then 
        ARCH="x86_64_centos6"
    else
        ARCH="x86_64_centos7"
    fi

    cd ${LCRP_CONFIG_PATH}
    artget push -d pkg_into_cmc.xml -p "{'componentVersion':'${componentVersion}','PRODUCT':'${product}', \
    'CODE_BRANCH':'${code_branch}','COMPONENT_TYPE':'${componentType}', 'ARCH':'Linux/${ARCH}'}" \
    -ap "${BACKUP_ROOT}/BACKUP_rel/BACKUP_rel.tar.gz" -user ${cmc_user} -pwd ${cmc_pwd}
    if [ $? -ne 0 ]; then
        log_echo "ERROR" "Upload artifact from cmc error"
        return 1
    fi

    log_echo "INFO" "Finish to upload pkgs into cmc"
    return 0
}

function upload_plugin_pkg()
{
    log_echo "Begin upload libBackup.so from cmc"
    upload_plugin_2_cmc ${PRODUCT} ${CODE_BRANCH} FS_BACKUP
    if [ $? -ne 0 ]; then
        log_echo "upload artifact error"
        exit 1
    fi
}

function main()
{
    compress_module_pkg
    upload_plugin_pkg "$@"
    return $?
}

main "$@"
exit $?