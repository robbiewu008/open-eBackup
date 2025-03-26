#!/bin/bash

CURRENT_SCRIPT="${BASH_SOURCE[0]}"
SCRIPT_PATH=$(dirname ${CURRENT_SCRIPT})
BUILD_ROOT=$(cd "${SCRIPT_PATH}/.."; pwd)
MODULE_DIR=$(cd "$(dirname ${BASH_SOURCE[0]})/../Module"; pwd)

source ${SCRIPT_PATH}/common/common.sh
DEPENDPATH="${BUILD_ROOT}/data_move_engine"
TYPYLIST="LLT ASAN TSAN RELEASE CLEAN"
TYPE=""
COMPILE_PARA=""

function make_scanner() {

    local DEPENDPATH_MODULE=${1:-${MODULE_DIR}}
    COMPILE_PARA=" ${COMPILE_PARA} -D MODULE_ROOT_DIR=${DEPENDPATH_MODULE}"
    log_echo "INFO" "DME PATH will use value: ${DEPENDPATH_MODULE}"
    log_echo "INFO" "TYPE will use the value: ${TYPE}"

    cd ${BUILD_ROOT}
    mkdir -p "${BUILD_ROOT}/build-cmake-file"
    cd "${BUILD_ROOT}/build-cmake-file"
    COMPILE_PARA=" ${COMPILE_PARA} -D NAS=ON "
    TYPE=$2
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

function main()
{
    make_scanner "$@"
}

make_scanner "$@"
exit $?
