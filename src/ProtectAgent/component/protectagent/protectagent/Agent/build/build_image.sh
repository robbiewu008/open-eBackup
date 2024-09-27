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
set +x
########流水线##########
#MS_IMAGE_TAG=
#harbor_project=
#tag_image=
#############传入参数############
AGENT_BRANCH=$1
INF_BRANCH=$2
#######################################################
product=dorado
#######################################################
plugin_agent_branch=${NAS_Plugins_BRANCH}
#######################################################
# agent_agent_branch="CreateImage_1220"
if [ "${BUILD_TYPE}" == "ASAN" ];then
    plugin_componentType="Plugins_ASAN"
else
    plugin_componentType="Plugins"
fi
agent_componentType="mspkg"
#######################################################
VERSION=${MS_IMAGE_TAG}
agent_branch=$(echo ${AGENT_BRANCH} | tr [A-Z] [a-z])
inf_branch=$(echo ${INF_BRANCH} | tr [A-Z] [a-z])
IMAGE_NAME=protectagent
BASE_TAG="oceanprotect-dataprotect-1.0.rc1:base"
AGENT_TAG=
DEFAULT_COMPONENT_VERSION="1.1.0"
if [ -z "$BUILD_PKG_TYPE" ]; then
    BUILD_PKG_TYPE=$1
fi
echo BUILD_PKG_TYPE=${BUILD_PKG_TYPE}

##################################
CURRENT_PATH=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
BASE_PATH=${CURRENT_PATH}/../..
INSTALL_ROOT=${CURRENT_PATH}/DataProtect_client


############################################################################
function check_param()
{
    if [ -z "${MS_IMAGE_TAG}" ] || [ -z "${harbor_project}" ]; then
        echo "Some variable is empty, please check."
    fi
}

function init_image_env()
{
    echo harbor_project=${harbor_project}
    echo tag_image=${tag_image}
    echo agent_branch=${agent_branch}
    echo inf_branch=${inf_branch}

    echo "docker login harbor"
    echo "MS_IMAGE_TAG=${MS_IMAGE_TAG}"
    docker login harbors.inhuawei.com -u User -p User1234
    AGENT_TAG=${IMAGE_NAME}:${VERSION}
}


function download_agent_from_cmc()
{
    sh download_agent.sh ${product} ${AGENT_BRANCH} ${agent_componentType} ${DEFAULT_COMPONENT_VERSION} ${BUILD_PKG_TYPE}
    if [ $? -ne 0 ]; then
        echo "Failed to download the agent package from the CMC."
        exit 1
    fi
}

function download_plugin_from_cmc()
{
    sh download_plugin.sh ${product} ${plugin_agent_branch} ${plugin_componentType} ${DEFAULT_COMPONENT_VERSION} ${BUILD_PKG_TYPE}
    if [ $? -ne 0 ]; then
        echo "Failed to download the plugin package from the CMC."
        exit 1
    fi
}

function clean_all_docker() 
{
    echo "Clearing all docker images"
    docker images | tr -s ' ' | cut -d ' ' -f 3 | xargs -I {} docker rmi -f {}
}

function import_base_image() 
{
    echo ${BASE_TAG}
    echo "pull base-image from harbor"
    docker pull harbors.inhuawei.com/a8000/${inf_branch}/${tag_image}/${BASE_TAG}
    if [ $? -ne 0 ]; then
        echo "pull base image failed"
        exit 1
    fi
    
    docker tag harbors.inhuawei.com/a8000/${inf_branch}/${tag_image}/${BASE_TAG} ${BASE_TAG}
    echo "download base image success"

    echo "cleanring running container!"
    docker ps -aq > docker_id.txt
    if [ -s docker_id.txt ];then
        docker stop $(docker ps -qa)
        docker rm -f $(docker ps -qa)
    fi

    echo "Clearing the image tag=none"
    docker images | grep "<none>" | tr -s ' ' | cut -d ' ' -f 3 | xargs -I {} docker rmi -f {}
}

function custom_protectagent_package()
{
    cd DataProtect_${VERSION_PKG}_client/ProtectClient-e
    mkdir "./tempDir"
    tar -xf protectclient-Linux-aarch64.tar.xz -C "./tempDir"
    cp -f ${BASE_PATH}/Agent/bin/install/Linux/ProtectClient-E/internal_run.sh ./tempDir/bin
    cp -f ${BASE_PATH}/Agent/bin/install/Linux/ProtectClient-E/get_net_plane_ip.py ./tempDir/bin
    cp -f ${BASE_PATH}/Agent/bin/install/Linux/ProtectClient-E/getpmip.py ./tempDir/bin
    cp -f ${BASE_PATH}/Agent/bin/install/Linux/ProtectClient-E/decrypt.py ./tempDir/bin
    cp -f ${BASE_PATH}/Agent/bin/install/Linux/ProtectClient-E/nodeinfo.py ./tempDir/bin
    cp -f ${BASE_PATH}/Agent/bin/install/Linux/ProtectClient-E/kmc_util.py ./tempDir/bin
    cp -f ${BASE_PATH}/Agent/bin/install/Linux/ProtectClient-E/update_cluster.py ./tempDir/bin
    cp -f ${BASE_PATH}/Agent/bin/install/Linux/ProtectClient-E/check_health.sh ./tempDir/bin
    cp -f ${BASE_PATH}/Agent/bin/shell/mount_oper.sh ./tempDir/sbin
    cp -f ${BASE_PATH}/Agent/bin/shell/change_permission.sh ./tempDir/sbin
    rm -f protectclient-Linux-aarch64.tar.xz
    cd "./tempDir"
    tar -cvf ../protectclient-Linux-aarch64.tar ./*
    xz -v ../protectclient-Linux-aarch64.tar
    cd ${CURRENT_PATH}/client
    rm -rf ${CURRENT_PATH}/client/DataProtect_${VERSION_PKG}_client/ProtectClient-e/tempDir
    zip -q -r DataProtect_${VERSION}_client_linux.zip DataProtect_${VERSION_PKG}_client
}

function prepare_install_package()
{
    # 1. mkdir 配置文件路径、自研包路径
    rm -rf "${INSTALL_ROOT}"
    mkdir ${INSTALL_ROOT}
    mkdir ${INSTALL_ROOT}/conf

    # 2. 解压、拷贝安装包
    cd ${CURRENT_PATH}/client
    VERSION_PKG=$(echo ${VERSION} | awk -F "." '{print $1"."$2"."$3}')
    unzip -q DataProtect_${VERSION_PKG}_client.zip

    mkdir -p ${CURRENT_PATH}/client/DataProtect_${VERSION_PKG}_client
    mkdir -p ${CURRENT_PATH}/client/DataProtect_${VERSION_PKG}_client/ProtectClient-e
    mkdir -p ${CURRENT_PATH}/client/DataProtect_${VERSION_PKG}_client/ProtectClient-e/Plugins
    mkdir -p ${CURRENT_PATH}/client/DataProtect_${VERSION_PKG}_client/third_party_software

    unzip -d ${CURRENT_PATH}/client/DataProtect_${VERSION_PKG}_client PackageScript/package-like-unix.zip
    unzip -d ${CURRENT_PATH}/client/DataProtect_${VERSION_PKG}_client/third_party_software third_party_software/3rd-linux.zip
    cp -rf ProtectClient-e/protectclient-Linux-aarch64.tar.xz ${CURRENT_PATH}/client/DataProtect_${VERSION_PKG}_client/ProtectClient-e
    cp -rf ${CURRENT_PATH}/Plugins/* ${CURRENT_PATH}/client/DataProtect_${VERSION_PKG}_client/ProtectClient-e/Plugins

    custom_protectagent_package
    mv ${CURRENT_PATH}/client/DataProtect_${VERSION_PKG}_client/* ${INSTALL_ROOT}
    cp ${BASE_PATH}/Agent/ci/conf/commonName ${INSTALL_ROOT}/conf
    cp ${BASE_PATH}/Agent/ci/conf/client.conf ${INSTALL_ROOT}/conf
    cp ${CURRENT_PATH}/client/package.json ${INSTALL_ROOT}/conf
}


function build_image() 
{
    echo "Begin to build agent"
    # 1.
    if [ "${BUILD_TYPE}" == "ASAN" ];then
        sed -i 's/RUN sh install.sh \\/RUN sh install.sh ASAN \\/g' "${BASE_PATH}/Agent/build/dockerfile/protectagent.dockerfile"
    fi
    docker build --rm -t ${AGENT_TAG} -f "${BASE_PATH}/Agent/build/dockerfile/protectagent.dockerfile"  "${CURRENT_PATH}"
    if [ $? -ne 0 ]; then
        echo "Docker build agent failed."
        exit 1
    fi

    # 2.
    if [ "$BUILD_PKG_TYPE" = "OceanCyber" ]; then
        docker tag ${AGENT_TAG} harbors.inhuawei.com/${harbor_project}/${agent_branch}/${tag_image}/oceancyber/${AGENT_TAG} 
        docker push harbors.inhuawei.com/${harbor_project}/${agent_branch}/${tag_image}/oceancyber/${AGENT_TAG}
    else
        docker tag ${AGENT_TAG} harbors.inhuawei.com/${harbor_project}/${agent_branch}/${tag_image}/${AGENT_TAG} 
        docker push harbors.inhuawei.com/${harbor_project}/${agent_branch}/${tag_image}/${AGENT_TAG}
    fi
    if [ $? -ne 0 ]; then
        echo "Push image to harbor failed."
        exit 1
    fi

    echo "Begin to build succ."
    return 0
}

function save_docker()
{
    mkdir -p ${BASE_PATH}/pkg/image
    docker save -o  ${BASE_PATH}/pkg/image/${IMAGE_NAME}_docker.tar $(docker images | grep -v "harbor" | grep "protectagent" | tr -s ' ' | awk '{printf "%s:%s ",$1,$2}')
    if [ $? -ne 0 ]; then
        echo "${IMAGE_NAME} docker save failed"
        exit 1
    fi

    if [ -f /usr/bin/pigz ]; then
        echo "Using pigz to zip"
        pigz -6 -p 12 ${BASE_PATH}/pkg/image/${IMAGE_NAME}_docker.tar
        if [ $? -ne 0 ]; then
            echo "${IMAGE_NAME} docker save pigz failed"
            exit 1
        fi
    else
        echo "Using gzip to zip, it's very slow!!!"
        gzip ${BASE_PATH}/pkg/image/${IMAGE_NAME}_docker.tar
        if [ $? -ne 0 ]; then
            echo "${IMAGE_NAME} docker save gzip failed"
            exit 1
        fi
    fi
    echo "${IMAGE_NAME} docker save success."
}

function main() 
{
    # 1.
    download_agent_from_cmc

    download_plugin_from_cmc

    # 2.
    clean_all_docker

    # 3.
    import_base_image

    # 4.
    prepare_install_package

    # 5.
    build_image

    # 6.
    save_docker
}

echo "#########################################################"
echo "   Begin build ${MS_NAME} image pkg"
echo "#########################################################"

init_image_env

main

echo "#########################################################"
echo "Build image pkg succ."
echo "#########################################################"
