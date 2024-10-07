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
# build file plugin
FILE_ROOT_DIR=$(cd $(dirname $0)/..; pwd)
FRAMEWORK_DIR=$(cd "${FILE_ROOT_DIR}/../../framework"; pwd)
COMMON_PATH=${FRAMEWORK_DIR}/build/common
. ${COMMON_PATH}/common.sh
SCRIPT_NAME=$(basename $0)

cmake_all()
{
    log_echo "INFO" "begin to build NAS"
    typeset type="$1"
    COMPILE_PARA=" -DCMAKE_EXPORT_COMPILE_COMMANDS=1"
    cd ${FILE_ROOT_DIR}
    mkdir -p "${FILE_ROOT_DIR}/build-cmake"
    cd "${FILE_ROOT_DIR}/build-cmake"
    rm -rf build-*
    if [ "${type}" = "LLT" ]; then
        echo "This is LLT compile" > build-llt_mark
        COMPILE_PARA=" -D LLT=ON "
    elif [ "${type}" = "DTFUZZ" ]; then
        echo "This is DTFUZZ compile" > build-dtfuzz_mark
        COMPILE_PARA=" -D DTFUZZ=ON "
    elif [ "${type}" = "ASAN" ]; then
        echo "This is ASAN compile" > build-asan_mark
        COMPILE_PARA=" -D ASAN=ON "
    elif [ "${type}" = "TSAN" ]; then
        echo "This is TSAN compile" > build-asan_mark
        COMPILE_PARA=" -D TSAN=ON "
    elif [[ "${type}" = "Release"  || "$BUILD_TYPE" = "Release" || "$BUILD_TYPE" = "release" ]]; then
        echo "This is Release compile" > build-asan_mark
        COMPILE_PARA=" -D CMAKE_BUILD_TYPE=Release "
    else
        COMPILE_PARA=${COMPILE_PARA}
    fi

    log_echo "INFO" "cmake ${COMPILE_PARA} .."
    cmake ${COMPILE_PARA} ../src
    if [ $? -ne 0 ]; then
        log_echo "ERROR" "cmake error"
        exit 1
    fi
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

# 删除sdk的冗余头文件，减少sdk的头文件引入
delete_sdk_header()
{
    log_echo "INFO" "delete useless header file"
    cd ${FRAMEWORK_DIR}/dep/agent_sdk/include/common
    ls |grep -v Defines.h | grep -v Types.h | xargs rm -f
    # 临时去掉重复的定义
    strings="MP_TRUE MP_FALSE MP_SUCCESS MP_FAILED MP_ERROR MP_NOEXISTS MP_TIMEOUT MP_TASK_FAILED_NEED_RETRY MP_TASK_COMPLETE \
    MP_TASK_RUNNING MP_EAGAIN MP_REDO MP_INC_TO_FULL MP_ARCHIVE_TOO_MUCH_CONNECTION MP_TASK_FAILED_NO_REPORT MP_INVALID_HANDLE"

    for str in ${strings[@]};
    do
        sed_local_modify '/'${str}'/d' Types.h
    done
    cd -
}

main()
{
    typeset type="$1"
    if [ "X${type}" == "Xclean" ]; then
        rm -rf ${FILE_ROOT_DIR}/build-cmake
        log_echo "INFO" "Finish to clean build-make folder"
        return 0
    fi
    delete_sdk_header
    cmake_all "$@"
    return $?
}


main "$@"
exit $?