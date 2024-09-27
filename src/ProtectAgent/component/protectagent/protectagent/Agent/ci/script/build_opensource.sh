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

set -ex
systemtype=$1
systemtypeage=$2
packagetype=$3

OUTPUT_CODE_DIR=${WORKSPACE}/output/code
OUTPUT_DIR_ASAN=${WORKSPACE}/output/finalpkg_ASAN/
SDK_DIR=${WORKSPACE}/../../../../open-source-obligation/PluginSDK/${systemtypeage}/
AGENT_PKG=${WORKSPACE}/../../../../open-source-obligation/dependency/Linux+
OPENSOURCE_BIN_PATH=${WORKSPACE}/../../../../open-source-obligation/Agent
cd ${WORKSPACE}/
export HOME=${WORKSPACE}
OUTPUT_DIR=${WORKSPACE}/output/finalpkg/
if [ ! -d ${OUTPUT_DIR} ];then
    mkdir -p ${OUTPUT_DIR}
fi

if [ ! -d ${SDK_DIR} ];then
    mkdir -p ${SDK_DIR}
fi
if [ ! -d ${AGENT_PKG} ];then
    mkdir -p ${AGENT_PKG}
fi

#set bep
if [ "${BEP}" == "YES" ]; then
	echo "start set Bep_Time!"
	source bep_env.sh -s ${WORKSPACE}/Agent/ci/conf/bep_env.conf
fi

if [ "${systemtypeage}" != "ASAN" ];then
    sh ${WORKSPACE}/Agent/build/download_opensrc.sh copy
    cd ${WORKSPACE}/
    source ${WORKSPACE}/Agent/build/env.sh
    if [[ ${packagetype} == 'sdk' ]] ;then
        sh ${WORKSPACE}/Agent/build/agent_make_cmake.sh sdk_no_opensrc
        [[ ! -d ${WORKSPACE}/tmp ]] && mkdir ${WORKSPACE}/tmp || echo "tmp 目录存在"
        cp ${WORKSPACE}/Agent/plugin_sdk.tar.gz ${SDK_DIR}
    else
        if [ "$1" == "sanclient" ];then
            echo "SanClient compile and packing..."
            sh ${WORKSPACE}/Agent/build/sanclient_pack_backup.sh no_opensrc
        else
            echo "Agent compile and packing..."
            sh ${WORKSPACE}/Agent/build/agent_pack_backup.sh no_opensrc
        fi
        echo "Copy output packages..."
        cp ${WORKSPACE}/AGENT_PACK_TEMP/*.tar.xz ${AGENT_PKG}
        echo "Build done."
    fi
else
    cd ${WORKSPACE}
    source ${WORKSPACE}/Agent/build/env.sh
    cp -f OPENSOURCE_BIN_PATH ./
    sh ${WORKSPACE}/Agent/build/agent_pack_backup.sh ASAN
    cp ${WORKSPACE}/AGENT_PACK_TEMP/*.tar.xz ${OUTPUT_DIR}
fi