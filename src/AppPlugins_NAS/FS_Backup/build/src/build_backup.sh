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

SCRIPT_PATH=$(cd $(dirname $0); pwd)
BACKUP_ROOT=$(cd "${SCRIPT_PATH}/../.."; pwd)

# 引入common.sh
COMMON_PATH=${SCRIPT_PATH}/../common
. ${COMMON_PATH}/common.sh

if [ -z "${MODULE_ROOT_PATH}" ];then
    MODULE_ROOT_PATH=${BACKUP_ROOT}/Module
fi
if [ ! -d "${MODULE_ROOT_PATH}" ];then
    MODULE_ROOT_PATH=${BACKUP_ROOT}/Module
fi
# if [ ! -d "${MODULE_ROOT_PATH}" ];then
#     log_echo "ERROR" "Module path no exist"
#     exit 1
# fi

# 编译参数
COMPILE_PARA=""

get_paras()
{
    # LLT depends
    local paraNum="$#"
    if [ ${paraNum} -eq 0 ];then
        log_echo "INFO" "No extra params"
        return 0
    fi
}

make_backup() {
    find -name *.sh | xargs dos2unix
    get_paras "$@"
    cd ${BACKUP_ROOT}
    type=$1
    mkdir -p "${BACKUP_ROOT}/build-cmake"
    cd "${BACKUP_ROOT}/build-cmake"
    if [ "X${CENTOS}" != "X6" ]; then
        COMPILE_PARA="-D NAS=ON "
    else
        COMPILE_PARA="-D FILE=ON"
    fi

    if [ -d "${MODULE_ROOT_PATH}" ];then
        COMPILE_PARA="-DMODULE_ROOT_PATH=${MODULE_ROOT_PATH}  $COMPILE_PARA"
    fi
    if [ X$type == XLLT ];then
        COMPILE_PARA="-DLLT=ON $COMPILE_PARA"
    fi
    if [ X$type == XDTFUZZ ];then
        COMPILE_PARA="-D DTFUZZ=ON $COMPILE_PARA"
    fi
    if [[ "${type}" = "Release"  || "${BUILD_TYPE}" = "Release" || "${BUILD_TYPE}" = "release" ]]; then
        echo "This is Release compile" > build-asan_mark
        COMPILE_PARA=" ${COMPILE_PARA} -D CMAKE_BUILD_TYPE=Release "
    fi

    # cmake & make
    cmake ${COMPILE_PARA} ..
    # 获取逻辑cpu个数
    if [ "${OS_TYPE}" = "AIX" ]; then
        array=$(bindprocessor -q)
        numProc=0
        for element in ${array[@]}
        do
            if [ $element -ge 0 ] 2>/dev/null; then
                let numProc+=1
            fi
        done
    else
        numProc=$(cat /proc/cpuinfo | grep processor | wc -l)
    fi
    make -j${numProc}
    if [ $? -ne 0 ]; then
        log_echo "ERROR" "make error"
        exit 1
    fi
}
if [ -z ${MODULE_BRANCH} ];then
    MODULE_BRANCH=develop_backup_software_1.6.0RC1
fi
if [ -z "$1" ] || [ ! -d $1 ];then
    sh ${BACKUP_ROOT}/build/download_module_from_cmc.sh ${MODULE_BRANCH}
    if [ $? -ne 0 ];then
        log_echo "ERROR" "Failed to download 3rd of Module"
        exit 1
    fi
fi
make_backup "$@"
exit $?
