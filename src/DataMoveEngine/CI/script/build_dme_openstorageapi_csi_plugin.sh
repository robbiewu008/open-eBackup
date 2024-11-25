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

MS_NAME=dme_openstorageapi_csi_plugin
if [ -z "${MS_IMAGE_TAG}" ]; then
    echo "MS_IMAGE_TAG does not exist."
    exit 1
fi

function build_image() {
    BINARY="$1"
    DOCKER_FILE_PATH="${BASE_PATH}/build/dockerfiles/${MS_NAME}.dockerfile"
    echo "start to build image ${BINARY}"
    docker build \
        --build-arg binary=${BINARY} \
        -t dme-openstorageapi-${BINARY}:${MS_IMAGE_TAG} \
        -f $DOCKER_FILE_PATH \
        "${BASE_PATH}/tmp/${MS_NAME}/mstmp/"
    echo "build image ${binary} success"
}

echo "#########################################################"
echo "   Begin build csi plugin image pkg"
echo "#########################################################"


if [ -f "${BASE_PATH}/pkg/mspkg/${MS_NAME}.tar.gz" ]; then
    mkdir -p ${BASE_PATH}/tmp/${MS_NAME}/mstmp/
    tar xvf "${BASE_PATH}/pkg/mspkg/${MS_NAME}.tar.gz" -C "${BASE_PATH}/tmp/${MS_NAME}/mstmp/"
    if [ $? != 0 ]; then
        echo "untar $MS_NAME.tar.gz failed"
        exit 1
    fi
    chmod -R 750 "${BASE_PATH}/tmp/${MS_NAME}/mstmp/"
else
    echo " $MS_NAME.tar.gz not exited"
    exit 1
fi

if [[ "${Compile_image}" == "Y" ]];then
    binary_list=(oceanprotect-csi csi-attacher snapshot-controller csi-node-driver-registrar csi-provisioner csi-snapshotter)
    for binary in ${binary_list[@]}; do
        build_image "bin/${binary}"
    done
fi

echo "#########################################################"
echo "  Success build csi plugin image pkg"
echo "#########################################################"
