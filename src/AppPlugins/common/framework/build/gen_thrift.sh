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
# generate thrift cpp codes
SCRIPT_PATH=$(cd $(dirname $0); pwd)
COMMON_PATH=${SCRIPT_PATH}/common
. ${COMMON_PATH}/common.sh
SCRIPT_NAME=$(basename $0)

MODULE_ROOT_DIR=${PLUGIN_ROOT_DIR}/../Module

THRIFT_CPP_DIR="${PLUGIN_ROOT_DIR}/src/thrift_interface"
THRIFT_INC_DIR="${PLUGIN_ROOT_DIR}/inc/thrift_interface"
THRIFT_EXE_DIR="${MODULE_ROOT_DIR}/third_open_src/thrift_rel/bin"

clean_thrift_cpp()
{
    if [ -d "${THRIFT_CPP_DIR}" ];then
        rm -rf ${THRIFT_CPP_DIR}
    fi
}

generate_thrift_cpp()
{
    if [ ! -f ${THRIFT_EXE_DIR}/thrift ]; then
        log_echo "ERROR" "Executable thrift file not exist"
        exit 1
    fi
    mkdir -p "${THRIFT_CPP_DIR}"

    for file in $(ls -1 ${PLUGIN_ROOT_DIR}/thrift_files/*.thrift | grep -v Example)
    do
        ${THRIFT_EXE_DIR}/thrift --gen cpp -out ${THRIFT_CPP_DIR} $file
    done

   rm -rf ${THRIFT_INC_DIR}
   mkdir -p ${THRIFT_INC_DIR}
   find ${THRIFT_CPP_DIR} -name *.h  | xargs -I {} mv -f {} ${THRIFT_INC_DIR}/
}

main()
{
    local type="$1"
    if [ "X$type" == Xclean ];then
        log_echo "DEBUG" "Begin to clean thrift cpp folder"
        clean_thrift_cpp
        log_echo "DEBUG" "Finish to clean thrift cpp folder"
        return 0
    fi

    log_echo "INFO" "Begin to generate thrift cpp files"
    generate_thrift_cpp
    local skeletonCppCount=$(ls -1 ${THRIFT_CPP_DIR}/*.skeleton.cpp 2>/dev/null | wc -l)
    if [ ${skeletonCppCount} -gt 0 ];then
        ls -1 ${THRIFT_CPP_DIR}/*.skeleton.cpp | xargs -I{} rm -f {}
    fi
    log_echo "INFO" "Finish to generate thrift cpp files"
}

main "$@"

