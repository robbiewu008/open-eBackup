#!/bin/bash
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

if [ -z ${LCRP_HOME} ]; then
    echo "Cant't find LCRP_HOME env, please config LCRP tool first"
    exit 1
fi

echo LCRP_HOME=${LCRP_HOME}

CODE_BRANCH="master"
PRODUCT="dorado"
COMPONENT_TYPE="mspkg"
if [ $# -eq 3 ]; then
    PRODUCT=$1
    CODE_BRANCH=$2
    COMPONENT_TYPE=$3
fi

echo "Product name ${PRODUCT}"
echo "Use branch ${CODE_BRANCH}"
echo "Component type ${COMPONENT_TYPE}"

initEnv() {
    BASE_PATH="$(
        cd "$(dirname "$BASH_SOURCE")/../../"
        pwd
    )"

    local arch_type=$(uname -m)
    if [ "$arch_type" == "aarch64" ]; then
        ARCH="euler-arm"
    else
        ARCH="euler-x86"
    fi

    CI_CONFIG_PATH=${BASE_PATH}/CI/LCRP/conf/
    SOURCE_PKG_PATH=${BASE_PATH}/pkg
}

function copy_pkgs() {
    mkdir -p ${BASE_PATH}/pkg/mspkg
    local L_COMPONENTS_DIR="${BASE_PATH}/component"
    for DIR_NAME in $(ls ${L_COMPONENTS_DIR}); do
        if [ -d ${L_COMPONENTS_DIR}/${DIR_NAME}/pkg ]; then
            echo "Copy pkgs from ${L_COMPONENTS_DIR}/${DIR_NAME}/pkg to ${BASE_PATH}/pkg/mspkg, pkg:"
            ls -l "${L_COMPONENTS_DIR}/${DIR_NAME}/pkg"
            cp -f "${L_COMPONENTS_DIR}/${DIR_NAME}/pkg/"*.tar.gz "${BASE_PATH}/pkg/mspkg"
        fi
    done
	
	echo "copy version file to ${BASE_PATH}/pkg/mspkg/"
	cp ${BASE_PATH}/PM_version ${BASE_PATH}/pkg/mspkg/

    echo -e "\nAfter copy pkg, ${BASE_PATH}/pkg/mspkg contains:"
    ls -l ${BASE_PATH}/pkg/mspkg
    ls -l ${BASE_PATH}/pkg/image
}

function upload_artifact() {
    cd ${CI_CONFIG_PATH}
    pwd
    lcrp.sh u pkgdir_to_cmc_ms.xml "params:{'PRODUCT':'${PRODUCT}', 'CODE_BRANCH':'${CODE_BRANCH}','COMPONENT_TYPE':'${COMPONENT_TYPE}', 'ARCH':'${ARCH}'}" "agentpath:${BASE_PATH}/pkg/${COMPONENT_TYPE}"
    if [ $? -ne 0 ]; then
        echo "Upload artifact to cmc error"
        exit 1
    fi

    if [[ "$Compile_image" == "Y" ]];then
        lcrp.sh u sent_images.xml "params:{'PRODUCT':'${PRODUCT}', 'CODE_BRANCH':'${CODE_BRANCH}','COMPONENT_TYPE':'image','ARCH':'${ARCH}'}" "agentpath:${BASE_PATH}/pkg/image"
        if [ $? -ne 0 ]; then
            echo "Upload images artifact to cmc error"
            exit 1
        fi
    fi
}

initEnv

function main() {
    copy_pkgs
    if [[ "${Upload_CMC}" != "N" ]];then
	    upload_artifact
    fi
}

echo "#########################################################"
echo "   Begin upload ${PRODUCT}/${CODE_BRANCH}/${COMPONENT_TYPE}/${ARCH} pkg to cmc"
echo "#########################################################"

main

echo "#########################################################"
echo "   Success upload ${PRODUCT}/${CODE_BRANCH}/${COMPONENT_TYPE}/${ARCH} pkg to cmc"
echo "#########################################################"
