#!/bin/bash
/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
# Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
BASE_PATH="$(
        cd "$(dirname "$BASH_SOURCE")/../../"
        pwd
    )"

DME_BRANCH=$1
INF_BRANCH=$2
MS_NAME=$3
if [ -z "${MS_IMAGE_TAG}" ]; then
    echo "MS_IMAGE_TAG does not exist."
    exit 1
fi 

G_BUILD_LIST=""
G_FIST_BUILD="dme_3rd"
VERSION=${MS_IMAGE_TAG}
code_branch=$(echo ${DME_BRANCH} | tr [A-Z] [a-z])
inf_branch=$(echo ${INF_BRANCH} | tr [A-Z] [a-z])

echo tag_image=${tag_image}
echo code_branch=${code_branch}
echo inf_branch=${inf_branch}

echo "MS_IMAGE_TAG=${MS_IMAGE_TAG}"
L_BASE_TAG="open-ebackup-1.0:base"
L_CBB_PYTHON_TAG="open-ebackup-1.0-cbb-python:base"

function check_dependent_image() {
    echo "Check DME dependent images"
    res=$(docker images $L_BASE_TAG | wc -l)
    if [ $res -eq 1 ]; then
        echo "No $L_BASE_TAG exist"
        exit 1
    fi
    res=$(docker images $L_CBB_PYTHON_TAG | wc -l)
    if [ $res -eq 1 ]; then
        echo "No $L_CBB_PYTHON_TAG exist"
        exit 1
    fi
    echo "Check DME dependent images success"
}

function check_dme_3rd_image() {
    echo "Check dme_3rd image"
    res=$(docker images dme_3rd:$VERSION | wc -l)
    if [ $res -eq 1 ]; then
        echo "No dme_3rd:$VERSION exist"
        exit 1
    fi
    echo "Check dme_3rd image success"
}

function buildall() {
    check_dependent_image
    if [ $? -ne 0 ];then
        echo "Check dependent images failed"
        exit 1
    fi
    DOCKER_NAME=$(ls "${BASE_PATH}/build/dockerfiles" | grep  ".dockerfile")
    for NAME in ${G_FIST_BUILD}.dockerfile ${DOCKER_NAME}
    do
        MS_NAME=$(basename $NAME .dockerfile)
        build_image $MS_NAME
        if [ $? -ne 0 ];then
            echo "$MS_NAME docker iamge build failed"
            exit 1
        fi
    done
}

function buildms() {
    check_dependent_image
    if [ $? -ne 0 ];then
        echo "Check dependent images failed"
        exit 1
    fi
    # 部分微服务，一个微服务编译多个包
    compile_ms=$1
    pkg=$(ls ${BASE_PATH}/component/${compile_ms}/pkg | grep ".tar.gz" )
    all_list=$(ls "${BASE_PATH}/build/dockerfiles" | grep  ".dockerfile")
    
    if [ -f "${BASE_PATH}/component/${compile_ms}/pkg/${G_FIST_BUILD}.tar.gz" ]; then
        build_image ${G_FIST_BUILD}
        if [ $? -ne 0 ];then
            echo "${G_FIST_BUILD} docker image build failed"
            exit 1
        fi
    else
        check_dme_3rd_image
        if [ $? -ne 0 ];then
            echo "No dme_3rd image"
            exit 1
        fi
    fi

    for NAME in ${pkg}
    do
        pkg_name=$(basename ${NAME} .tar.gz)
        MS_NAME=$(echo ${pkg_name} | tr [A-Z] [a-z])
        if [[ "${all_list}" =~ "${MS_NAME}" ]]; then
            build_image ${MS_NAME}
            if [ $? -ne 0 ];then
                echo "${MS_NAME} docker iamge build failed"
                exit 1
            fi
        fi
    done

}

function build_image() {
    MS_NAME=$1
    echo "Begin to build $MS_NAME"

    for BUILDED in $G_BUILD_LIST; do
        if [ $BUILDED == ${MS_NAME} ]; then
            echo "Already build ${MS_NAME}, skip"
            return 0
        fi
    done

    if [ ! -e "${BASE_PATH}/tmp/$MS_NAME/mstmp/" ]; then
        mkdir -p "${BASE_PATH}/tmp/$MS_NAME/mstmp/"
    fi

    if [ ! -f "${BASE_PATH}/pkg/mspkg/$MS_NAME.tar.gz" ]; then
        echo "${BASE_PATH}/pkg/mspkg/$MS_NAME.tar.gz not found for build"
        exit 1
    fi

    if [ -f "${BASE_PATH}/pkg/mspkg/$MS_NAME.tar.gz" ]; then
        tar xvf "${BASE_PATH}/pkg/mspkg/${MS_NAME}.tar.gz" -C "${BASE_PATH}/tmp/${MS_NAME}/mstmp/"
        if [ $? != 0 ]; then
            echo "untar $MS_NAME.tar.gz failed"
            exit 1
        fi
        if [[ $MS_NAME =~ "nginx" ]]
        then
            chmod -R 750 "${BASE_PATH}/tmp/${MS_NAME}/mstmp/"
            chmod -R 550 "${BASE_PATH}/tmp/${MS_NAME}/mstmp/opt/OceanStor/100P/ProtectEngine-E/nginx/script"
            chmod 550 "${BASE_PATH}/tmp/${MS_NAME}/mstmp/opt/OceanStor/100P/ProtectEngine-E/nginx/nginx"
            chmod 700 "${BASE_PATH}/tmp/${MS_NAME}/mstmp/opt/OceanStor/100P/ProtectEngine-E/nginx/html"
        else
            chmod -R 750 "${BASE_PATH}/tmp/${MS_NAME}/mstmp/"
        fi
    else
        echo " $MS_NAME.tar.gz not exited"
        exit 1
    fi

    local L_VER_3RD=${VERSION}
    local L_TAG=$MS_NAME:${VERSION}
    if [ -f "${BASE_PATH}/build/dockerfiles/$MS_NAME.name" ]; then
        L_TAG=$(cat "${BASE_PATH}/build/dockerfiles/$MS_NAME.name")
    fi

    echo "Run docker build --build-arg VERSION_3RD=${L_VER_3RD} -t:$L_TAG -f "${BASE_PATH}/build/dockerfiles/$MS_NAME.dockerfile" "${BASE_PATH}/tmp/${MS_NAME}/mstmp/""
    docker build --rm --build-arg VERSION_3RD=${L_VER_3RD} -t$L_TAG -f "${BASE_PATH}/build/dockerfiles/$MS_NAME.dockerfile" "${BASE_PATH}/tmp/${MS_NAME}/mstmp/"
    if [ $? -ne 0 ]; then
        echo "docker build $MS_NAME.dockerfile failed"
        exit 1
    fi

    G_BUILD_LIST="$G_BUILD_LIST $MS_NAME"
    echo "docker build $MS_NAME.dockerfile success"

    return 0
}

function main() {
    if [ $# = 0 ]; then
        buildall
    else
        buildms $@
    fi
}

echo "#########################################################"
echo "   Begin build ${MS_NAME} image pkg"
echo "#########################################################"

if [[ "${Compile_image}" == "Y" ]];then
    main ${MS_NAME}
    if [ $? -ne 0 ]; then
		echo "docker images build failed!"
		exit 1
	fi
fi


echo "#########################################################"
echo "  Success build ${MS_NAME} image pkg"
echo "#########################################################"
