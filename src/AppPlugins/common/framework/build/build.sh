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
umask 0022
# entry of CI pipeline
SCRIPT_PATH=$(cd $(dirname $0); pwd)
COMMON_PATH=${SCRIPT_PATH}/common
. ${COMMON_PATH}/common.sh
SCRIPT_NAME=$(basename $0)
SYSTEM_LIB_LIST="libstdc++ libgcc_s"
TYPYLIST="LLT ASAN TSAN RELEASE"

strip_file()
{
    path=$1
    files=`ls $path`
    chmod 700 $1
    log_echo "INFO" "Begin to strip so under dir $path"
    for filename in $files
    do
        echo $filename | grep -e ".*so.*"
        if [ $? == 0 ]; then
            chmod 700 $path/$filename
            strip $path/$filename
        fi
    done
}

clean_pkgs()
{
    #clean pkg file
    log_echo "INFO" "Begin to clean compile folders"
    rm -rf ${PLUGIN_PACKAGE_PATH}
    rm -rf ${PLUGIN_ROOT_DIR}/build-cmake
    rm -rf ${EXT_PKG_DOWNLOAD_PATH}

    chmod u+x ${PLUGIN_ROOT_DIR}/build/build_3rd_opensource.sh
    ${PLUGIN_ROOT_DIR}/build/build_3rd_opensource.sh clean

    chmod u+x ${PLUGIN_ROOT_DIR}/build/gen_thrift.sh
    ${PLUGIN_ROOT_DIR}/build/gen_thrift.sh clean

    chmod u+x ${PLUGIN_ROOT_DIR}/build/build_framework.sh
    ${PLUGIN_ROOT_DIR}/build/build_framework.sh clean
}

init_comile_env()
{
    mkdir -p ${PLUGIN_PACKAGE_PATH}
}

build_plugin_opensrc()
{
    cd ${PLUGIN_ROOT_DIR}/build
    chmod u+x ${PLUGIN_ROOT_DIR}/build/build_3rd_opensource.sh
    ${PLUGIN_ROOT_DIR}/build/build_3rd_opensource.sh
    if [ $? -ne 0 ]; then
        log_echo "ERROR" "Compile third libs failed"
        exit 1
    fi
}

genrate_thrift_files()
{
    cd ${PLUGIN_ROOT_DIR}/build
    chmod u+x gen_thrift.sh
    ./gen_thrift.sh
    if [ $? -ne 0 ]; then
        log_echo "ERROR" "Gnerate thrift code failed"
        exit 1
    fi
}

build_plugin_framework()
{
    cd ${PLUGIN_ROOT_DIR}/build
    chmod u+x build_framework.sh
    ./build_framework.sh "$@"
    if [ $? -ne 0 ]; then
        log_echo "ERROR" "Build agent plugin failed"
        exit 1
    fi
}

main()
{
    typeset type="$1"
    if [ "X${type}" == "Xclean" ];then
        clean_pkgs
        exit 0
    fi
    [ $# -gt 1 ] && shift 1 # 门禁使用build.sh传的参数时流程参数，此处需要移位去除流程参数
    init_comile_env

    typeset type_beyond=0
    if [ "X${type}" != "X" ];then
        type_beyond=$(echo "${TYPYLIST}" | grep -ic ${type})
    fi

    # first: build 3rd opensource (build thrift tool, etc.)
    if [ "X${type}" == "X" -o ${type_beyond} -gt 0 ];then
        build_plugin_opensrc
    fi

    # second: generate thrift codes
    if [ "X${type}" == "X" -o "X${type}" == "Xgenerate_thrift" -o ${type_beyond} -gt 0 ];then
        genrate_thrift_files
    fi

    # third: build plugin codes
    if [ "X${type}" == "X" -o "X${type}" == "Xbuild" -o ${type_beyond} -gt 0 ];then
        build_plugin_framework "${type}"
    fi
}

main "$@"
