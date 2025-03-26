#!/bin/bash

CURRENT_SCRIPT="${BASH_SOURCE[0]}"
SCRIPT_PATH=$(dirname ${CURRENT_SCRIPT})
BUILD_ROOT=$(cd "${SCRIPT_PATH}/.."; pwd)

source ${SCRIPT_PATH}/common/common.sh
DEPENDPATH="${BUILD_ROOT}/data_move_engine"
TYPYLIST="LLT ASAN TSAN RELEASE CLEAN"
TYPE=""
COMPILE_PARA=""

function help()
{
    echo "This script executable method as folows:"
    echo "${CURRENT_SCRIPT} TYPE DME_PATH:When data_move_engine path is changed and cmake type is required, this command will be used."
    echo "${CURRENT_SCRIPT} TYPE :When only cmake type is required, this command will be used."
    echo "${CURRENT_SCRIPT} DME_PATH :When only data_move_engine path is changed, this command will be used."
    echo "${CURRENT_SCRIPT} :When the default data_move_engine path and cmake type are used, this command will work."
}

function get_paras()
{
    local paraNum="$#"
    if [ ${paraNum} -eq 0 ];then
        log_echo "INFO" "DME PATH will use the default path: ${DEPENDPATH}"
        log_echo "INFO" "TYPE will use the default compile type"
        return 0
    fi

    if [ ${paraNum} -eq 1 ];then
        local para_1=$(echo "$1" |tr 'a-z' 'A-Z')
        local is_type=$(echo "${TYPYLIST}" | grep -c "${para_1}")
        if [ ${is_type} -gt 0 ];then
            TYPE="${para_1}"
            return 0
        fi
        local dme_path="$1"
        if [ -d ${dme_path} ];then
            DEPENDPATH="${dme_path}"
            COMPILE_PARA=" ${COMPILE_PARA} -DDME_PATH=${DEPENDPATH}"
            return 0
        fi
        help
        exit 1
    fi

    for type in ${TYPYLIST};do
       local para_1=$(echo "$1" |tr 'a-z' 'A-Z')
       if [ "X${para_1}" == "X${type}" ];then
           TYPE=${type}
           break
       fi
    done
    if [ "X${TYPE}" == "X" ];then
        help
        exit 1
    fi

    local dme_path="$2"
    if [ -d ${dme_path} ];then
        DEPENDPATH="${dme_path}"
        COMPILE_PARA=" ${COMPILE_PARA} -DDME_PATH=${DEPENDPATH}"
        return 0
    fi
    help
    exit 1
}

function make_scanner() {
    get_paras "$@"
    log_echo "INFO" "DME PATH will use value: ${DEPENDPATH}"
    log_echo "INFO" "TYPE will use the value: ${TYPE}"
    cd ${BUILD_ROOT}
    mkdir -p "${BUILD_ROOT}/build-cmake"
    cd "${BUILD_ROOT}/build-cmake"
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
        rm -rf  "${BUILD_ROOT}/build-cmake"
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
