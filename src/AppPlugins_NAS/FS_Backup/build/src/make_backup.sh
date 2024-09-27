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
BUILD_ROOT=$(cd "${SCRIPT_PATH}"; pwd)
BACKUP_ROOT=$(cd "${SCRIPT_PATH}/../.."; pwd)
BACKUP_MODULE_DIR=${BACKUP_ROOT}/Module

# 引入common.sh
COMMON_PATH=${SCRIPT_PATH}/../common
. ${COMMON_PATH}/common.sh

#################### 获 取 入 参 ##########################
# 设置默认值
MODULE_PATH=${SCRIPT_PATH}/../../Module
type=
MODE="FILE"

for option
do
    case $option in

    -path=* | --path=*)
      MODULE_PATH=`expr "x$option" : "x-*path=\(.*\)"`
      ;;
    
    -type=* | --type=*)
      type=`expr "x$option" : "x-*type=\(.*\)"`
      ;;

    -mode=* | --mode=*)
      MODE=`expr "x$option" : "x-*mode=\(.*\)"`
      ;;

    esac
done

log_echo "INFO" "MODULE PATH is: ${MODULE_PATH}"
log_echo "INFO" "Compile TYPE: ${TYPE}"
log_echo "INFO" "mode: ${MODE}"
#################### 获 取 入 参 ##########################

make_backup() {

    find . -name *.sh | xargs dos2unix
    if [ $MODULE_PATH = ${BACKUP_MODULE_DIR} ]; then
        sh ${BACKUP_MODULE_DIR}/build/download_3rd.sh
    fi
    COMPILE_PARA="-D ${MODE}=ON "
    if [ "${type}" = "LLT" ]; then
        echo "This is LLT compile" > build-llt_mark
        COMPILE_PARA=" ${COMPILE_PARA} -D LLT=ON "
    elif [ "${type}" = "ASAN" ]; then
        echo "This is ASAN compile" > build-asan_mark
        COMPILE_PARA=" ${COMPILE_PARA} -D ASAN=ON "
    elif [ "${type}" = "DTFUZZ" ]; then
        echo "This is DTFUZZ compile" > build-dtfuzz_mark
        COMPILE_PARA=" ${COMPILE_PARA} -D DTFUZZ=ON "
    elif [ "${type}" = "TSAN" ]; then
        echo "This is TSAN compile" > build-asan_mark
        COMPILE_PARA=" ${COMPILE_PARA} -D TSAN=ON "
    elif [[ "${type}" = "RELEASE"  || "${type}" = "Release" || "${type}" = "release" ]]; then
        echo "This is Release compile" > build-asan_mark
        COMPILE_PARA=" ${COMPILE_PARA} -D CMAKE_BUILD_TYPE=Release "
    elif [ "${type}" = "CLEAN" ]; then
        echo "This is compile clean"
        rm -rf  "${BUILD_ROOT}/build-cmake-file"
        exit 0
    else
        COMPILE_PARA=" ${COMPILE_PARA} "
    fi
    COMPILE_PARA=" ${COMPILE_PARA} -D MODULE_ROOT_PATH=${MODULE_PATH}"
    cd ${BACKUP_ROOT}
    mkdir -p "${BACKUP_ROOT}/build-cmake"
    cd "${BACKUP_ROOT}/build-cmake"

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
        numProc=4
    elif [ "${OS_TYPE}" = "SunOS" ]; then
        numProc=4
    else
        numProc=$(cat /proc/cpuinfo | grep processor | wc -l)
    fi
    make -j${numProc}
    if [ $? -ne 0 ]; then
        log_echo "ERROR" "make error"
        exit 1
    fi
}

make_backup
exit $?