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
FILE_HOME=$(cd "${CURRENT_DIR}/.."; pwd)
BUILD_DIR=$(cd "${FILE_HOME}/CI/script"; pwd)
TEST_DIR=$(cd "${FILE_HOME}/test"; pwd)
PLUGIN_ROOT_PATH=$(cd "${FILE_HOME}/../.."; pwd)

if [ -z "${MODULE_ROOT_PATH}" ];then
    MODULE_ROOT_PATH=${FILE_HOME}/../../Module
fi
if [ ! -d "${MODULE_ROOT_PATH}" ];then
    MODULE_ROOT_PATH=$(cd "${MODULE_ROOT_PATH}"; pwd)
fi
if [ ! -d "${MODULE_ROOT_PATH}" ];then
    echo "ERROR: Module path no exist"
    exit 1
fi
BUILD_TYPE=$1
if [ -z "${BUILD_TYPE}" ];then
    BUILD_TYPE="LLT"
fi

DT_UTILS_DIR=$(cd "${MODULE_ROOT_PATH}/dt_utils"; pwd)

function clean_test()
{
    rm -rf ${FILE_HOME}/build-cmake
    rm -rf ${TEST_DIR}/build
    rm -rf ${TEST_DIR}/bin
    rm -rf ${TEST_DIR}/log
}

function compile_file()
{
    if [ -d ${FILE_HOME}/build-cmake ] && [ -d ${PLUGIN_ROOT_PATH}/FS_Backup/build-cmake ]  \
        && [ -f ${MODULE_ROOT_PATH}/dt_utils/gmock/googletest-release/lib/libgmock.a ];then
        echo "INFO: Already compiled"
        return 0
    fi

    # build framework
    sh ${FILE_HOME}/build/build_framework.sh
    if [ $? -ne 0 ]; then
        log_echo "ERROR" "Building plugin framework failed"
        exit 1
    fi

    # build Scanner
    sh ${FILE_HOME}/build/build_scanner.sh "${BUILD_TYPE}"
    if [ $? -ne 0 ]; then
        log_echo "ERROR" "Building plugin scanner failed"
        exit 1
    fi

    # build backup
    sh ${FILE_HOME}/build/build_backup.sh "${BUILD_TYPE}"
    if [ $? -ne 0 ]; then
        log_echo "ERROR" "Building backup module failed"
        exit 1
    fi

    # build file plugin
    sh ${FILE_HOME}/build/build_file.sh "${BUILD_TYPE}"
    if [ $? -ne 0 ]; then
        echo "ERROR" "Building file service failed"
        exit 1
    fi
    echo "INFO: Compile file service successfully"
    return 0
}

function build_dtutils()
{
    # note: 如果已存在编译生成文件则不会重复编译
    sh ${DT_UTILS_DIR}/mockcpp/build/build.sh
}

function build_file_test()
{
    cd ${TEST_DIR}
    mkdir -p "${TEST_DIR}/build"
    cd "${TEST_DIR}/build"

    # cmake & make
    if [ -d "${MODULE_ROOT_PATH}" ];then
        COMPILE_PARA="-DMODULE_ROOT_PATH=${MODULE_ROOT_PATH}"
    fi
    if [ X$BUILD_TYPE == XDTFUZZ ];then
        COMPILE_PARA="-D DTFUZZ=ON $COMPILE_PARA"
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

    compile_file
    if [ $? -ne 0 ];then
        exit 1
    fi
    build_dtutils
    build_file_test
    local ret=$?
    echo "Compile backup LLT code success"
    return $ret
}

main "$@"
exit $?

