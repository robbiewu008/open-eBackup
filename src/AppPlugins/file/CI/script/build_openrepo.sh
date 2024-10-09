#!/bin/sh
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
FILE_ROOT_DIR=$(cd $(dirname $0)/../..; pwd)
FRAMEWORK_DIR=$(cd "${FILE_ROOT_DIR}/../common/framework"; pwd)
MODULE_PATH=$(cd "${FILE_ROOT_DIR}/../common/Module"; pwd)
COMMON_PATH=${FRAMEWORK_DIR}/build/common
. ${COMMON_PATH}/common.sh
SCRIPT_NAME=$(basename $0)
build_type=$1
if [ -z "${build_type}" ];then
    build_type="Debug"
fi

main()
{
    # build framework
    sh ${FRAMEWORK_DIR}/build/build.sh "OPENSOURCE"
    if [ $? -ne 0 ]; then
        log_echo "ERROR" "Building plugin framework failed"
        exit 1
    fi

    # build Module
    sh ${MODULE_PATH}/build/build_module.sh "-type=${build_type}"
    if [ $? -ne 0 ]; then
        log_echo "ERROR" "Building Module failed"
        exit 1
    fi

    # build backup
    sh ${FILE_ROOT_DIR}/build/build_backup.sh "-type=${build_type}"
    if [ $? -ne 0 ]; then
        log_echo "ERROR" "Building backup module failed"
        exit 1
    fi

    # build scanner
    sh ${FILE_ROOT_DIR}/build/build_scanner.sh "-type=${build_type}"
    if [ $? -ne 0 ]; then
        log_echo "ERROR" "Building plugin scanner failed"
        exit 1
    fi

    # build file plugin
    sh ${FILE_ROOT_DIR}/build/build_file.sh "${build_type}"
    if [ $? -ne 0 ]; then
        log_echo "ERROR" "Building file lib failed"
        exit 1
    fi
    exit 0
}

main