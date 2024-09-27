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
BACKUP_HOME=$(cd "${CURRENT_DIR}/.."; pwd)
BUILD_ROOT_DIR=$(cd "${BACKUP_HOME}/build"; pwd)
TEST_DIR=$(cd "${BACKUP_HOME}/test"; pwd)
MODULE_ROOT_PATH=${BACKUP_HOME}/Module
DT_UTILS_DIR=$(cd "${MODULE_ROOT_PATH}/dt_utils"; pwd)

source ${BUILD_ROOT_DIR}/common/branch.sh

function clean_test()
{
    rm -rf ${BACKUP_HOME}/build-cmake
    rm -rf ${TEST_DIR}/build
}

function check_module_exist()
{
    if [ -z "${MODULE_ROOT_PATH}" ];then
        MODULE_ROOT_PATH=${BACKUP_HOME}/Module
    fi
    if [ ! -d "${MODULE_ROOT_PATH}" ];then
        MODULE_ROOT_PATH=${BACKUP_HOME}/Module
    fi
    if [ ! -d "${MODULE_ROOT_PATH}" ];then
        echo "ERROR: Module path no exist"
        exit 1
    fi
}

function compile_backup()
{
    if [ -d ${BACKUP_HOME}/build-cmake ]  \
        && [ -f ${MODULE_ROOT_PATH}/dt_utils/gmock/googletest-release/lib/libgmock.a ];then
        echo "INFO: Already compiled"
        return 0
    fi

    sh ${BUILD_ROOT_DIR}/download_module_from_cmc.sh ${MODULE_BRANCH}
    if [ $? -ne 0 ];then
        echo "ERROR: Failed to download 3rd of Module"
        exit 1
    fi

    mkdir -p "${BACKUP_HOME}/build-cmake"
    cd "${BACKUP_HOME}/build-cmake"
    COMPILE_PARA="-DLLT=ON -DNAS=ON -DOBS=ON -DMODULE_ROOT_PATH=${MODULE_ROOT_PATH} "
    cmake ${COMPILE_PARA} ..
    numProc=$(cat /proc/cpuinfo | grep processor | wc -l)
    make -j${numProc}
    if [ $? -ne 0 ];then
        echo "ERROR: Compile backup failed"
        exit 1
    fi

    echo "INFO: Compile backup successfully"
    return 0
}

function build_dtutils()
{
    # note: 如果已存在编译生成文件则不会重复编译
    sh ${DT_UTILS_DIR}/mockcpp/build/build.sh
}

function build_backup_test()
{

    cd ${TEST_DIR}
    mkdir -p "${TEST_DIR}/build"
    cd "${TEST_DIR}/build"

    # cmake & make
    if [ -d "${MODULE_ROOT_PATH}" ];then
        COMPILE_PARA="-DMODULE_ROOT_PATH=${MODULE_ROOT_PATH}"
    fi
    COMPILE_PARA="-DNAS=ON -DVOLUME=ON -DOBS=ON "${COMPILE_PARA}
    cmake  ${COMPILE_PARA} ..
    numProc=$(cat /proc/cpuinfo | grep processor | wc -l)
    make -j${numProc}
    if [ $? -ne 0 ]; then
        echo "ERROR: make error"
        exit 1
    fi
}

function download_data_and_scanout_from_cmc
{
    cd ${TEST_DIR}/conf
    if [ -d "${TEST_DIR}/conf/data" -a -d "${TEST_DIR}/conf/scan" -a -d "${TEST_DIR}/conf/backup_out" ]; then
        return 0
    fi

    if [ ! -f "backup.tar.gz" ]; then
        artget pull "ProtectAgent-Client 1.0" -ru software -user ${cmc_user} -pwd ${cmc_pwd} -rp "llt/backup.tar.gz" -ap ${TEST_DIR}/conf
    fi

    tar zxvf backup.tar.gz
}

function main()
{
    if [ X$1 == Xclean ];then
        clean_test
        exit 0
    fi

    check_module_exist
    compile_backup
    build_dtutils
    build_backup_test
    download_data_and_scanout_from_cmc
    local ret=$?
    echo "Compile backup LLT code success"
    return $ret
}

main "$@"
exit $?

