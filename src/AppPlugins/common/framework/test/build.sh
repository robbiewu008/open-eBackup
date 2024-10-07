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
SCRIPT_PATH=$(dirname ${BASH_SOURCE[0]})
CURRENT_DIR=$(cd "${SCRIPT_PATH}"; pwd)
FRAMEWORK_HOME=$(cd "${CURRENT_DIR}/.."; pwd)
TEST_DIR=$(cd "${FRAMEWORK_HOME}/test"; pwd)

if [ -z "${MODULE_ROOT_PATH}" ];then
    MODULE_ROOT_PATH=${FRAMEWORK_HOME}/../Module
fi
if [ ! -d "${MODULE_ROOT_PATH}" ];then
    MODULE_ROOT_PATH=$(cd "${MODULE_ROOT_PATH}"; pwd)
fi
if [ ! -d "${MODULE_ROOT_PATH}" ];then
    echo "ERROR: Module path no exist"
    exit 1
fi

DT_UTILS_DIR=$(cd "${MODULE_ROOT_PATH}/dt_utils"; pwd)
TEST_DIR=$(cd "${FRAMEWORK_HOME}/test"; pwd)

function clean_test()
{
    rm -rf ${FRAMEWORK_HOME}/build-cmake
    rm -rf ${TEST_DIR}/build
    rm -rf ${TEST_DIR}/bin
    rm -rf ${TEST_DIR}/log
}

function compile_frameowrk()
{
    # build plugin framework plugin
    sh ${FRAMEWORK_HOME}/build/build.sh LLT
    if [ $? -ne 0 ]; then
        log_echo "ERROR" "Building plugin framework failed"
        exit 1
    fi
    echo "INFO: Compile plugin framework successfully"
    return 0
}

function build_dtutils()
{
    # note: 如果已存在编译生成文件则不会重复编译
    sh ${DT_UTILS_DIR}/mockcpp/build/build.sh
}

function build_frameowrk_test()
{
    cd ${TEST_DIR}
    mkdir -p "${TEST_DIR}/build"
    cd "${TEST_DIR}/build"

    # cmake & make
    if [ -d "${MODULE_ROOT_PATH}" ];then
        COMPILE_PARA="-DMODULE_ROOT_PATH=${MODULE_ROOT_PATH}"
    fi
    cmake  ${COMPILE_PARA} ..
    numProc=$(cat /proc/cpuinfo | grep processor | wc -l)
    make -j${numProc}
    if [ $? -ne 0 ]; then
        echo "ERROR: make error"
        exit 1
    fi
}

function main()
{
    if [ X$1 == Xclean ];then
        clean_test
        exit 0
    fi

    # compile_frameowrk
    # if [ $? -ne 0 ];then
    #     exit 1
    # fi
    build_dtutils
    build_frameowrk_test
    local ret=$?
    echo "Compile backup LLT code success"
    return $ret
}

main "$@"
exit $?

