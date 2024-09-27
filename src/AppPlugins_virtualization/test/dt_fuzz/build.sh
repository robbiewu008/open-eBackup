#!/bin/sh
#
#  This file is a part of the open-eBackup project.
#  This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
#  If a copy of the MPL was not distributed with this file, You can obtain one at
#  http://mozilla.org/MPL/2.0/.
# 
#  Copyright (c) [2024] Huawei Technologies Co.,Ltd.
# 
#  THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
#  EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
#  MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
#
set +x

SCRIPT_PATH=$(dirname ${BASH_SOURCE[0]})
CURRENT_DIR=$(cd "${SCRIPT_PATH}"; pwd)
VIRT_HOME=$(cd "${CURRENT_DIR}/../.."; pwd)
TEST_DIR=$(cd "${VIRT_HOME}/test"; pwd)
PLUGIN_ROOT_PATH=$(cd "${VIRT_HOME}/../.."; pwd)
FRAMEWORK_PATH=${PLUGIN_ROOT_PATH}/framework

if [ -z "${MODULE_ROOT_PATH}" ];then
    MODULE_ROOT_PATH=${PLUGIN_ROOT_PATH}/Module
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

function clean_test()
{
    rm -rf ${VIRT_HOME}/build-cmake
    rm -rf ${TEST_DIR}/build
    rm -rf ${TEST_DIR}/bin
    rm -rf ${TEST_DIR}/log
}

function compile_virt()
{
    if [ -d ${PLUGIN_ROOT_PATH}/build-cmake ] && [ -f ${MODULE_ROOT_PATH}/dt_utils/gmock/googletest-release/lib/libgmock.a ];then
        echo "INFO: Already compiled"
        return 0
    fi

    # build framework
    pushd ${FRAMEWORK_PATH}/build
    sh build_framework.sh "${BUILD_TYPE}"
    popd

    # build virt plugin
    pushd ${VIRT_HOME}/build
    sh build.sh
    if [ $? -ne 0 ]; then
        echo "ERROR" "Building file service failed"
        exit 1
    fi
    popd

   echo "INFO" "Building virtualization service success."
   return 0
}

function build_virt_test()
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

    compile_virt
    if [ $? -ne 0 ];then
        exit 1
    fi

    build_virt_test
    local ret=$?
    echo "Compile backup LLT code success"
    return $ret
}

main "$@"
exit $?

