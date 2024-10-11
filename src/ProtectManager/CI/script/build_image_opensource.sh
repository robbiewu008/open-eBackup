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
BASE_PATH="$(
        cd "$(dirname "$BASH_SOURCE")/../../"
        pwd
    )"

G_BUILD_LIST=""
G_FIST_BUILD=""
G_App_Common="PM_Data_Protection_Service PM_Database_Version_Migration PM_Resource_Manager PM_Config"

echo "MS_IMAGE_TAG=${MS_IMAGE_TAG}"

echo harbor_project=${harbor_project}
echo tag_image=${tag_image}

echo "modify PM version."
sh ${BASE_PATH}/CI/script/common.sh

function copy_pkg() {
    mkdir -p ${BASE_PATH}/pkg/mspkg

    local L_COMPONENTS_DIR="${BASE_PATH}/component"
    for DIR_NAME in $(ls ${L_COMPONENTS_DIR}); do
        if [ -d ${L_COMPONENTS_DIR}/${DIR_NAME}/pkg ]; then
            echo "Copy pkgs from ${L_COMPONENTS_DIR}/${DIR_NAME}/pkg to ${BASE_PATH}/pkg/mspkg, pkg:"
            ls -l "${L_COMPONENTS_DIR}/${DIR_NAME}/pkg"
            cp -f "${L_COMPONENTS_DIR}/${DIR_NAME}/pkg/"*.tar.gz "${BASE_PATH}/pkg/mspkg"
        fi
    done

    echo -e "\nAfter copy pkg, ${BASE_PATH}/pkg/mspkg contains:"
    ls -l ${BASE_PATH}/pkg/mspkg

    rm -rf ${BASE_PATH}/pkg/image
    mkdir ${BASE_PATH}/pkg/image
}

function build_image() {
    local L_MS_NAME=$1
    echo "Begin to build $L_MS_NAME"
    MS_DIR=${L_MS_NAME}

    if [[ ${G_BUILD_LIST} =~ ${L_MS_NAME} ]]; then
        echo "Already build ${L_MS_NAME}, skip"
        return 0
    fi

    if [ ! -e "${BASE_PATH}/tmp/$L_MS_NAME/mstmp/" ]; then
        mkdir -p "${BASE_PATH}/tmp/$L_MS_NAME/mstmp/"
    fi

    if [ ! -f "${BASE_PATH}/pkg/mspkg/$L_MS_NAME.tar.gz" ]; then
        echo "${BASE_PATH}/pkg/mspkg/$L_MS_NAME.tar.gz not found for build"
        return 1
    fi

    if [ -f "${BASE_PATH}/pkg/mspkg/$L_MS_NAME.tar.gz" ]; then
        tar xvf "${BASE_PATH}/pkg/mspkg/${MS_DIR}.tar.gz" -C "${BASE_PATH}/tmp/${MS_DIR}/mstmp/"
        ls -l "${BASE_PATH}/tmp/${MS_DIR}/mstmp/"
        if [ $? != 0 ]; then
            echo "untar $L_MS_NAME.tar.gz failed"
            return 1
        fi
    else
        echo " $L_MS_NAME.tar.gz not exited"
        exit 1
    fi

    if [ -f "${BASE_PATH}/build/dockerfiles/$L_MS_NAME.name" ]; then
        L_TAG=$(cat "${BASE_PATH}/build/dockerfiles/$L_MS_NAME.name")
        L_IMAGE_NAME=$(echo ${L_TAG} | awk -F ':' '{print $1}')
    else
        return 1
    fi

    docker images | grep $L_IMAGE_NAME | tr -s ' ' | cut -d ' ' -f 2 | xargs -I {} # docker rmi -f $L_IMAGE_NAME:{}

    echo "Run docker build -t:$L_TAG -f "${BASE_PATH}/build/dockerfiles/$L_MS_NAME.dockerfile" "${BASE_PATH}/tmp/${MS_DIR}/mstmp/""

    if [ 'OceanCyber' == "${BUILD_PKG_TYPE}" ] && [ 'PM_System_Base_Service' == "${L_MS_NAME}" ]; then
        echo "Run rm DataProtect_${Version}_client.zip"
        rm -f "${BASE_PATH}/tmp/${MS_DIR}/mstmp/DataProtect_${Version}_client.zip"
        touch "${BASE_PATH}/tmp/${MS_DIR}/mstmp/DataProtect_${Version}_client.zip"
        ls -l "${BASE_PATH}/tmp/${MS_DIR}/mstmp/"
    fi

    docker build --rm -t $L_TAG -f "${BASE_PATH}/build/dockerfiles/${L_MS_NAME}_opensource.dockerfile" "${BASE_PATH}/tmp/${MS_DIR}/mstmp/"

    if [ $? -ne 0 ]; then
        echo "docker build $L_MS_NAME.dockerfile failed"
        exit 1
    fi

    G_BUILD_LIST="$G_BUILD_LIST $L_MS_NAME"
    echo "docker build $L_MS_NAME.dockerfile success"

    return 0
}

function build_all_image() {
    PKG_NAME=$(ls "${BASE_PATH}/pkg/mspkg" | grep "tar.gz" | grep -v PM_API_Gateway )
    for NAME in ${G_FIST_BUILD} ${PKG_NAME}
    do
        MS_NAME=$(basename $NAME .tar.gz)
        build_image $MS_NAME
        if [ $? -ne 0 ];then
            echo "docker image build failed"
        fi
    done
    build_image "PM_Config"
    if [ $? -ne 0 ];then
        echo "docker image build failed"
    fi
}

function build_ms_image() {
    MS_NAME=$1
    if [[ "${G_App_Common}" =~ "${MS_NAME}" ]]; then
        build_image ${MS_NAME}
    else
        build_image ${MS_NAME}
    fi
}

function main() {
    copy_pkg
    if [ $# = 0 ]; then
        build_all_image
    else
        MS_NAME=$1
        export BUILD_PKG_TYPE=$2
        export PRODUCT_IMAGE_PATH=$3
        build_ms_image ${MS_NAME}
    fi
}

echo "#########################################################"
echo "   Begin build ${Service_Name} image pkg"
echo "#########################################################"

main $@

echo "#########################################################"
echo "  Success build ${Service_Name} image pkg"
echo "#########################################################"
