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
MODULE_HOME=$(cd "${CURRENT_DIR}/.."; pwd)
BUILD_ROOT_DIR=$(cd "${MODULE_HOME}/build"; pwd)
TEST_DIR=$(cd "${MODULE_HOME}/test"; pwd)

function clean_test()
{
    rm -rf ${MODULE_HOME}/third_open_src/*_rel
    rm -rf ${MODULE_HOME}/platform/*_rel
    rm -rf ${TEST_DIR}/build
}

# 下载 open_src 和 platform
function downlaod_3rd()
{
    local third_opensrc_rel_count=$(ls -1 ${MODULE_HOME}/third_open_src | grep -Ec "_rel")
    local platform_rel_count=$(ls -1 ${MODULE_HOME}/platform | grep -Ec "_rel")
    if [ ${third_opensrc_rel_count} -gt 0 ] && [ ${platform_rel_count} -gt 0 ];then
        echo "INFO: Existed 3rd lib, no need to download it again"
        return 0
    fi
    sh ${BUILD_ROOT_DIR}/download_3rd.sh
    if [ $? -ne 0 ];then
        echo "ERROR: Downlaod 3rd failed"
        exit 1
    fi
    echo "INFO: Downlaod 3rd successfully"
    return 0
}


function get_paras()
{
    # LLT depends
    local paraNum="$#"
    if [ ${paraNum} -eq 0 ];then
        echo "INFO: No extra params"
        return 0
    fi
}

function build_module_test()
 {
    get_paras "$@"
    cd ${TEST_DIR}
    mkdir -p "${TEST_DIR}/build"
    cd "${TEST_DIR}/build"

    # cmake & make
    cmake  ..
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

    downlaod_3rd
    if [ $? -ne 0 ];then
        exit 1
    fi
    build_module_test
    local ret=$?
    echo "Compile module success"
    return $ret
}

main "$@"
exit $?

