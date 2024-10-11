#!/bin/bash
########################################
#  This file is part of the open-eBackup project.
# Copyright (c) 2024 Huawei Technologies Co.,Ltd.
#
# open-eBackup is licensed under MPL v2.
# You can use this software according to the terms and conditions of the MPL v2.
# You may obtain a copy of MPL v2 at:
#
#          https://www.mozilla.org/en-US/MPL/2.0
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the MPL v2 for more details.
########################################
set -x
CURRENT_PATH=$(cd `dirname $0`; pwd)
echo "CURRENT_PATH"
source $CURRENT_PATH/open_comm_param.sh
PM_MS_DIR=${CURRENT_PATH}/..
OM_PATH=${PM_MS_DIR}/../../../open-source-obligation/Infrastructure_OM/om
LCRP_XML_PATH=${PM_MS_DIR}/conf
CI_DIR=${PM_MS_DIR}/../ci
sh ${CI_DIR}/script/open_comm_param.sh

function build_image() {

    NAME=om
    L_TAG=${NAME}':'${MS_IMAGE_TAG}
    MS_NAME=$(echo ${L_TAG} | sed 's/:/-/g')
    echo "Begin to build $MS_NAME"

    mkdir -p "${PM_MS_DIR}/tmp/$MS_NAME/"

    if [ -f "${OM_PATH}/pkg/$MS_NAME.tar.gz" ]; then
        cp -rf "${OM_PATH}/pkg/${MS_NAME}.tar.gz"  "${PM_MS_DIR}/tmp/$MS_NAME/"
        if [ $? -ne 0 ]; then
            echo "copy $MS_NAME.tar.gz failed"
            exit 1
        fi
    else
        echo " $MS_NAME.tar.gz not exited"
        exit 1
    fi

    echo "Run docker build --rm -t $L_TAG -f ${CI_DIR}/build/om/dockerfiles/open-ebackup_$NAME.dockerfile ${PM_MS_DIR}/tmp/$MS_NAME/"
    docker build --rm -t $L_TAG -f "${CI_DIR}/build/om/dockerfiles/open-ebackup_$NAME.dockerfile" "${PM_MS_DIR}/tmp/$MS_NAME/"
    if [ $? -ne 0 ]; then
        echo "docker build open-ebackup_${NAME}.dockerfile failed"
        exit 1
    fi

    echo "docker build open-ebackup_${NAME}.dockerfile success"

    return 0
}

function main()
{
    build_image
}

main
