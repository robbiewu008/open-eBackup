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
SDK_DIR=${binary_path}/PluginSDK/${systemtypeage}/
AGENT_PKG=${binary_path}/dependency/Linux
OPENSOURCE_BIN_PATH=${binary_path}/Agent

OUTPUT_DIR=${WORKSPACE}/output/finalpkg/
if [ ! -d ${OUTPUT_DIR} ];then
    mkdir -p ${OUTPUT_DIR}
fi

CURRENT_DIR=$(cd "$(dirname $0)" && pwd)
AGENT_HOME=$(readlink -f "${CURRENT_DIR}/../../../")
echo AGENT_HOME=$AGENT_HOME


if [ ! -d ${SDK_DIR} ];then
    mkdir -p ${SDK_DIR}
fi
if [ ! -d ${AGENT_PKG} ];then
    mkdir -p ${AGENT_PKG}
fi

#set bep
if [ "${BEP}" == "YES" ]; then
	echo "start set Bep_Time!"
	source bep_env.sh -s ${AGENT_HOME}/Agent/ci/conf/bep_env.conf
fi

# build
if [ "${systemtypeage}" != "ASAN" ];then
    sh ${AGENT_HOME}/Agent/build/get_open_third_party.sh "${binary_path}/ThirdParty"
    if [ $? != 0 ]; then
        echo "Get open third party fail."
    fi
    cd ${AGENT_HOME}/
    source ${AGENT_HOME}/Agent/build/env.sh
    if [[ ${packagetype} == 'sdk' ]] ;then
        sh ${AGENT_HOME}/Agent/build/agent_make_cmake.sh sdk_no_opensrc
        [[ ! -d ${AGENT_HOME}/tmp ]] && mkdir ${AGENT_HOME}/tmp || echo "tmp 目录存在"
        cp ${AGENT_HOME}/Agent/plugin_sdk.tar.gz ${SDK_DIR}
    else
        if [ "$1" == "sanclient" ];then
            echo "SanClient compile and packing..."
            sh ${AGENT_HOME}/Agent/build/sanclient_pack_backup.sh no_opensrc
        else
            echo "Agent compile and packing..."
            sh ${AGENT_HOME}/Agent/build/agent_pack_backup.sh no_opensrc
        fi
        echo "Copy output packages..."
        cp ${AGENT_HOME}/AGENT_PACK_TEMP/*.tar.xz ${AGENT_PKG}
        echo "Build done."
    fi
else
    cd ${AGENT_HOME}
    source ${AGENT_HOME}/Agent/build/env.sh
    cp -f OPENSOURCE_BIN_PATH ./
    sh ${AGENT_HOME}/Agent/build/agent_pack_backup.sh ASAN
    cp ${AGENT_HOME}/AGENT_PACK_TEMP/*.tar.xz ${OUTPUT_DIR}
fi