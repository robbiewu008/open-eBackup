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

# 初始化环境变量
source ./common.sh

BRANCH=${branch}
SYS_ARCH=`uname -m`
 
if [ -z "${BRANCH}" ]; then
    BRANCH="develop_backup_software_1.6.0RC1"
fi

function download_framework()
{
    local FRAMEWORK_GIT=ssh://git@codehub-dg-y.huawei.com:2222/dpa/protectagent/AppPlugins_NAS.git
    if [ -z "${BRANCH}" ]; then
        local BRANCH=master_backup_software_1.6.0RC1
    fi

    if [ -d "${PROJECT_ROOT_PATH}/framework" ]; then
        echo "Framework repo already exists. If you need to download the file again, delete the directory[framework]."
        return 0
    fi
    tmpdir=$(mktemp -d)
    git clone ${FRAMEWORK_GIT} ${tmpdir}
    pushd ${tmpdir}
    git checkout -b ${BRANCH} origin/${BRANCH}
    popd
    cp -r ${tmpdir}/framework ${PROJECT_ROOT_PATH}
    rm -rf ${tmpdir}
}

function download_module()
{
    local MODULE_GIT=ssh://git@szv-y.codehub.huawei.com:2222/dpa/CBB/CPP/Module.git

    if [ -d "${PROJECT_ROOT_PATH}/Module" ]; then
        echo "Module repo already exists. If you need to download the file again, delete the directory[Module]."
        return 0
    fi
    tmpdir=$(mktemp -d)
    git clone ${MODULE_GIT} ${tmpdir}/Module
    pushd ${tmpdir}/Module
    git checkout -b ${BRANCH} origin/${BRANCH}
    popd
    cp -r ${tmpdir}/Module ${PROJECT_ROOT_PATH}
    rm -rf ${tmpdir}
}

function download_artifact()
{
    mkdir -p ${VIRT_DEPS_EXT_PKG_PATH}
    artget pull -os dependency_opensource.xml -ap ${PROJECT_ROOT_PATH} -user ${opensource_user} -pwd ${opensource_pwd} -at opensource
    if [ $? -ne 0 ]; then
        log_echo "ERROR" "download third package from opensource-central failed!"
        return 1
    fi

    artget pull -d dependency_cmc.xml -ap ${PROJECT_ROOT_PATH} -user ${cmc_user} -pwd ${cmc_pwd}
    if [ $? -ne 0 ]; then
        log_echo "ERROR" "download cmc package from general-central failed!"
        return 1
    fi

    log_echo "INFO" "Finish to download pkgs from cmc"
    echo "The pkgs in ${VIRT_DEPS_EXT_PKG_PATH} :"
    ls -l ${VIRT_DEPS_EXT_PKG_PATH}
    return 0
}

download_python3_pluginFrame()
{
    tmpdir=$(mktemp -d)
    # 内置插件不需要打包python
    if [ "${INTERNAL_PLUGIN}" = "1" ]; then
        echo "Internal plugins does not need packag python."
        return 0
    fi
    python3_file=python3.pluginFrame.${SYS_ARCH}.tar.gz
    if [ -e ${PROJECT_ROOT_PATH}/${python3_file} ]; then
        rm -f ${PROJECT_ROOT_PATH}/${python3_file}
    fi
    if [ -z ${OPENSRC_BRANCH} ]; then
        OPENSRC_BRANCH=${branch}
    fi
    if [ "${SYS_ARCH}" == "aarch64" ]; then
        SYSTEM_NAME="CentOS7.6"
        OS_TYPE="ARM"
    else
        SYSTEM_NAME="CentOS6.10"
        OS_TYPE="X86"
    fi
    COMPOENT_VERSION=1.2.1RC1
    opensrc_temp_path=${tmpdir}/opensrc_temp
    artget pull -d dependency_python.xml -p "{'COMPOENT_VERSION':'${COMPOENT_VERSION}','PRODUCT':'dorado', \
    'THIRD_BRANCH':'${OPENSRC_BRANCH}','OS_TYPE':'${OS_TYPE}','SYSTEM_NAME':'${SYSTEM_NAME}'}" \
    -ap ${opensrc_temp_path} -user ${cmc_user} -pwd ${cmc_pwd}
    if [ $? -ne 0 ]; then 
        echo "Download plugin from cmc error"
        exit 1
    fi
    opensrc_pkg_name=OceanProtect_X8000_${COMPOENT_VERSION}_Opensrc_3rd_SDK_${SYSTEM_NAME}_${OS_TYPE}.tar.gz
    cd ${opensrc_temp_path}
    tar -zxvf ${opensrc_pkg_name} python3_rel.tar.gz
    tar -zxvf python3_rel.tar.gz python3_rel/${python3_file}
    cp python3_rel/${python3_file} ${VIRT_DEPS_EXT_PKG_PATH}
    echo "======================download python package from cmc successfully========================="
}
 

function main()
{
    download_framework

    download_module

    download_artifact

    download_python3_pluginFrame
    return $?
}

main
exit $?