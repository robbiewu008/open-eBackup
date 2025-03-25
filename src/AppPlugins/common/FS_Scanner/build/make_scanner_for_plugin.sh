#!/bin/bash

CURRENT_SCRIPT="${BASH_SOURCE[0]}"
SCRIPT_PATH=$(dirname ${CURRENT_SCRIPT})
BUILD_ROOT=$(cd "${SCRIPT_PATH}/.."; pwd)
SCANNER_MODULE_DIR="${BUILD_ROOT}/Module"
source ${SCRIPT_PATH}/common/common.sh
source ${COMMON_PATH}/branch.sh

MODULE_PATH=${SCANNER_MODULE_DIR}
TYPE=""
MODE="FILE"
COMPILE_PARA=""

function get_paras()
{
    for option; do
        case $option in
            -path=* | --path=*)
            MODULE_PATH=`expr "x$option" : "x-*path=\(.*\)"`
            ;;
            -type=* | --type=*)
            TYPE=`expr "x$option" : "x-*type=\(.*\)"`
            ;;
            -mode=* | --mode=*)
            MODE=`expr "x$option" : "x-*mode=\(.*\)"`
            ;;
        esac
    done
}

function download_module_opensrc()
{
    sh ${BUILD_ROOT}/build/download_module_from_cmc.sh ${MODULE_BRANCH}
    if [ $? -ne 0 ]; then
        log_echo "ERROR" "download module failed"
        exit 1
    fi
}

function make_scanner()
{
    log_echo "INFO" "MODULE PATH will use value: ${MODULE_PATH}"
    log_echo "INFO" "TYPE will use the value: ${TYPE}"
    log_echo "INFO" "MODE: ${MODE}"

    cd ${BUILD_ROOT}
    mkdir -p "${BUILD_ROOT}/build-cmake-file"
    cd "${BUILD_ROOT}/build-cmake-file"

    COMPILE_PARA=" ${COMPILE_PARA} -D MODULE_ROOT_DIR=${MODULE_PATH}"
    COMPILE_PARA=" ${COMPILE_PARA} -D ${MODE}=ON "

    if [ "${TYPE}" = "LLT" ]; then
        echo "This is LLT compile" > build-llt_mark
        COMPILE_PARA=" ${COMPILE_PARA} -D LLT=ON "
    elif [ "${TYPE}" = "ASAN" ]; then
        echo "This is ASAN compile" > build-asan_mark
        COMPILE_PARA=" ${COMPILE_PARA} -D ASAN=ON "
    elif [ "${TYPE}" = "TSAN" ]; then
        echo "This is TSAN compile" > build-asan_mark
        COMPILE_PARA=" ${COMPILE_PARA} -D TSAN=ON "
    elif [[ "${TYPE}" = "Release"  || "$BUILD_TYPE" = "Release" || "$BUILD_TYPE" = "release" ]]; then
        echo "This is Release compile" > build-asan_mark
        COMPILE_PARA=" ${COMPILE_PARA} -D CMAKE_BUILD_TYPE=Release "
    elif [ "${TYPE}" = "CLEAN" ]; then
        echo "This is compile clean"
        rm -rf  "${BUILD_ROOT}/build-cmake-file"
        exit 0
    else
        COMPILE_PARA=" ${COMPILE_PARA} "
    fi

    log_echo "INFO" "cmake ${COMPILE_PARA} .."
    cmake ${COMPILE_PARA} ..
    numProc=$(cat /proc/cpuinfo | grep processor | wc -l)
    make -j${numProc}
    if [ $? -ne 0 ]; then
        log_echo "ERROR" "make error"
        exit 1
    fi
}

function main() {
    get_paras "$@"

    if [ $MODULE_PATH = ${SCANNER_MODULE_DIR} ]; then
        download_module_opensrc
    fi

    make_scanner
}

main "$@"
exit $?


