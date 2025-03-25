#!/bin/bash
# 编译构建依赖的DME包
CURRENT_SCRIPT="${BASH_SOURCE[0]}"
SCRIPT_PATH=$(dirname ${CURRENT_SCRIPT})
BUILD_ROOT=$(cd "${SCRIPT_PATH}/.."; pwd)

source ${BUILD_ROOT}/common/common.sh
SCRIPT_NAME="${0##*/}"

function gen_code()
{
    cd "${FRAMEWORK_ROOT_PATH}/build"
    sh make-gencode.sh
    if [ $? -ne 0 ]; then
        log_echo "ERROR" "Gen code before compile error"
        exit 1
    fi
    cd -
}

function cmake_all()
{
    sed -i "/tcmalloc_cmpexec/d" "${FRAMEWORK_ROOT_PATH}/src/src/agent/CMakeLists.txt"
    if [ "$1" = "LLT" ]; then
        shift
        mkdir -p "${NAS_ROOT_DIR}/${DME_ROOT_DIR_NAME}/build-llt"
        cd "${NAS_ROOT_DIR}/${DME_ROOT_DIR_NAME}/build-llt"
        PRINT_DIR="${NAS_ROOT_DIR}/${DME_ROOT_DIR_NAME}/build-llt"
        cmake -D LLT=ON ..
    elif [[ "$1" = "Release"  || "$BUILD_TYPE" = "Release" || "$BUILD_TYPE" = "release" ]]; then
        shift
        mkdir -p "${NAS_ROOT_DIR}/${DME_ROOT_DIR_NAME}/build-cmake"
        cd "${NAS_ROOT_DIR}/${DME_ROOT_DIR_NAME}/build-cmake"
        cmake -D CMAKE_BUILD_TYPE=Release ..
    else
        mkdir -p "${NAS_ROOT_DIR}/${DME_ROOT_DIR_NAME}/build-cmake"
        cd "${NAS_ROOT_DIR}/${DME_ROOT_DIR_NAME}/build-cmake"
        cmake ..
    fi

    if [ $? -ne 0 ]; then
        log_echo "ERROR" "cmake error"
        exit 1
    fi
    cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 ..

    numProc=$(cat /proc/cpuinfo | grep processor | wc -l)
    make -j${numProc} $@
    if [ $? -ne 0 ]; then
        log_echo "ERROR" "make error"
        exit 1
    fi
    echo
    echo "#######################################################################################################"
    log_echo "INFO" "Compile success. Please get the outputs from ${PRINT_DIR}"
    echo "#######################################################################################################"
}

function get_dme_lib()
{
    local libPath=${NAS_ROOT_DIR}/${DME_ROOT_DIR_NAME}/build-cmake
    local count=$(ls -1 ${libPath}/Data_Transmission_Frame/*.so 2>/dev/null | wc -l)
    if [ $count -gt 0 ]; then
        mkdir -p ${NAS_ROOT_DIR}/${DME_ROOT_DIR_NAME}/transmission/lib
        cp -arf "${libPath}/Data_Transmission_Frame/"*.so "${NAS_ROOT_DIR}/${DME_ROOT_DIR_NAME}/transmission/lib"
        cp -arf "${NAS_ROOT_DIR}/${DME_ROOT_DIR_NAME}/Data_Transmission_Frame/inc" "${NAS_ROOT_DIR}/${DME_ROOT_DIR_NAME}/transmission/"
    fi
    count=0
    count=$(ls -1 ${libPath}/Data_Transmission_Frame/*.so 2>/dev/null | wc -l)
    if [ $count -gt 0 ]; then
        mkdir -p ${NAS_ROOT_DIR}/${DME_ROOT_DIR_NAME}/framework/lib
        cp -arf "${libPath}/DME_Framework/"*.so "${NAS_ROOT_DIR}/${DME_ROOT_DIR_NAME}/framework/lib"
        cp -arf "${NAS_ROOT_DIR}/${DME_ROOT_DIR_NAME}/DME_Framework/src/inc" "${NAS_ROOT_DIR}/${DME_ROOT_DIR_NAME}/framework/"
    fi

    if [ ! -d ${NAS_ROOT_DIR}/${DME_ROOT_DIR_NAME}/transmission/lib ] \
        || [ ! -d ${NAS_ROOT_DIR}/${DME_ROOT_DIR_NAME}/framework/lib ]; then
        log_echo "ERROR" "Copy dme lib pkg failed, pls check."
        exit 1
    fi
    log_echo "DEBUG" "Copy pkg success."
}

function main() {
    if [ "$1" != "clean" ]; then
        gen_code
    fi

    cmake_all $@

    get_dme_lib
    return $?
}

main $@
exit $?
