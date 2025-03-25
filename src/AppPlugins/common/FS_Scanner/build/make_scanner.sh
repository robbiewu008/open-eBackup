#!/bin/bash

CURRENT_SCRIPT="${BASH_SOURCE[0]}"
SCRIPT_PATH=$(dirname ${CURRENT_SCRIPT})
BUILD_ROOT=$(cd "${SCRIPT_PATH}/.."; pwd)

source ${SCRIPT_PATH}/common/common.sh

function make_dme() {
    log_echo "DEBUG" "make dme"
    if [ -d ${BUILD_ROOT}/data_move_engine/Framework ];then
        mv ${BUILD_ROOT}/data_move_engine/Framework ${BUILD_ROOT}/data_move_engine/DME_Framework
    fi

    sh ${BUILD_ROOT}/build/dme/download_frame_third_platform.sh
    if [ $? -ne 0 ]; then
        log_echo "ERROR" "download_frame_third_platform error"
        exit 1
    fi

    sh ${BUILD_ROOT}/build/dme/dme_make_cmake.sh
    if [ $? -ne 0 ]; then
        log_echo "ERROR" "dme_make_cmake error"
        exit 1
    fi
}

function main() {
    local paraNum=$#
    local flag=0
    if [ ${paraNum} -eq 0 ];then
        # 开发模式或编译门禁，需要自编译dme
        flag=0
    elif [ ${paraNum} -eq 1 ];then
        if [ -d "$1" ];then
            flag=1
        else
           flag=0
        fi
    else
        if [ ! -z "$2" -a -d "$2" ];then
            flag=1
        else
            flag=0
        fi
    fi

    # 不存在DME_PATH参数的时候才会编译DME
    if [ X${flag} == "X0" ];then
        # LLT 需要编译自dme
        log_echo "scanner self compile normal"
        make_dme
    else
        # scanner被集成，无需自己编译dme
        log_echo "scanner integrated compile, don't compile dme"
    fi
    chmod +x ${BUILD_ROOT}/build/build_scanner.sh
    sh ${BUILD_ROOT}/build/build_scanner.sh "$@"
}

main "$@"
exit $?