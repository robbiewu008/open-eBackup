#!/bin/bash

SCRIPT_PATH=$(cd $(dirname $0); pwd)
BUILD_ROOT=$(cd "${SCRIPT_PATH}/.."; pwd)
MODULE_DIR=${BUILD_ROOT}/Module
COMMON_PATH=${SCRIPT_PATH}/common
. ${COMMON_PATH}/common.sh

DEPENDPATH="${BUILD_ROOT}/data_move_engine"
TYPYLIST="LLT ASAN TSAN RELEASE CLEAN"
TYPE=""
COMPILE_PARA=""

#################### 获 取 入 参 ##########################
# 设置默认值
MODULE_PATH=${MODULE_DIR}
TYPE=
NAS="OFF"

for option
do
    case $option in

    -path=* | --path=*)
      MODULE_PATH=`expr "x$option" : "x-*path=\(.*\)"`
      ;;
    
    -type=* | --type=*)
      TYPE=`expr "x$option" : "x-*type=\(.*\)"`
      ;;

    -NAS=* | --NAS=*)
      NAS=`expr "x$option" : "x-*NAS=\(.*\)"`
      ;;

    esac
done
#################### 获 取 入 参 ##########################

make_scanner() {

    COMPILE_PARA=" ${COMPILE_PARA} -D MODULE_ROOT_DIR=${MODULE_PATH}"
    log_echo "INFO" "MODULE PATH will use value: ${MODULE_PATH}"
    log_echo "INFO" "TYPE will use the value: ${TYPE}"
    log_echo "INFO" "NAS: ${NAS}"

    cd ${BUILD_ROOT}
    mkdir -p "${BUILD_ROOT}/build-cmake-file"
    cd "${BUILD_ROOT}/build-cmake-file"
    if [ $NAS = "ON" ]; then
        COMPILE_PARA=" ${COMPILE_PARA} -D NAS=ON "
    else
        COMPILE_PARA=" ${COMPILE_PARA} -D FILE=ON -D CMAKE_EXPORT_COMPILE_COMMANDS=1"
    fi

    if [ "${TYPE}" = "LLT" ]; then
        echo "This is LLT compile" > build-llt_mark
        COMPILE_PARA=" ${COMPILE_PARA} -D LLT=ON "
    elif [ "${TYPE}" = "ASAN" ]; then
        echo "This is ASAN compile" > build-asan_mark
        COMPILE_PARA=" ${COMPILE_PARA} -D ASAN=ON "
    elif [ "${TYPE}" = "TSAN" ]; then
        echo "This is TSAN compile" > build-asan_mark
        COMPILE_PARA=" ${COMPILE_PARA} -D TSAN=ON "
    elif [[ "${TYPE}" = "Release"  || "${BUILD_TYPE}" = "Release" || "${BUILD_TYPE}" = "release" ]]; then
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
    # 获取逻辑cpu个数
    if [ "${OS_TYPE}" = "AIX" ]; then
        array=$(bindprocessor -q)
        numProc=0
        for element in ${array[@]}
        do
            if [ $element -ge 0 ] 2>/dev/null; then
                let numProc+=1
            fi
        done
        numProc=4
    elif [ "${OS_TYPE}" = "SunOS" ]; then
        numProc=4
    else
        numProc=$(cat /proc/cpuinfo | grep processor | wc -l)
    fi
    make -j${numProc}
    if [ $? -ne 0 ]; then
        log_echo "ERROR" "make error"
        exit 1
    fi

}

main()
{
    make_scanner
}

make_scanner
exit $?
