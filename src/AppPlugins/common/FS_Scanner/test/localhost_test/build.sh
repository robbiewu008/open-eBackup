#!/bin/bash
SCRIPT_PATH=$(dirname ${BASH_SOURCE[0]})
CURRENT_DIR=$(cd "${SCRIPT_PATH}"; pwd)
SCANNER_HOME=$(cd "${CURRENT_DIR}/../.."; pwd)
BUILD_ROOT_DIR=$(cd "${SCANNER_HOME}/build"; pwd)
TEST_DIR=$(cd "${SCANNER_HOME}/test/localhost_test"; pwd)

source ${BUILD_ROOT_DIR}/common/branch.sh

MODULE_ROOT_PATH=${SCANNER_HOME}/Module
if [ ! -d "${MODULE_ROOT_PATH}" ];then
    echo "ERROR: Module path no exist"
    exit 1
fi

DT_UTILS_DIR=$(cd "${MODULE_ROOT_PATH}/dt_utils"; pwd)

function clean_test()
{
    rm -rf ${SCANNER_HOME}/build-cmake-file
    rm -rf ${TEST_DIR}/build
    rm -rf ${TEST_DIR}/log
}

function compile_scanner()
{
    if [ -d ${SCANNER_HOME}/build-cmake-file ] \
        && [ -f ${MODULE_ROOT_PATH}/dt_utils/gmock/googletest-release/lib/libgmock.a ];then
        echo "INFO: Already compiled"
        return 0
    fi

    sh ${BUILD_ROOT_DIR}/download_module_from_cmc.sh ${MODULE_BRANCH}
    if [ $? -ne 0 ]; then
        echo "ERROR: download module failed"
        exit 1
    fi

    mkdir -p "${SCANNER_HOME}/build-cmake-file"
    cd "${SCANNER_HOME}/build-cmake-file"
    COMPILE_PARA=" ${COMPILE_PARA} -D LLT=ON -D FILE=ON -D NAS=ON -D OBS=ON -D MODULE_ROOT_DIR=${MODULE_ROOT_PATH} "
    echo "INFO: cmake ${COMPILE_PARA} .."
    cmake ${COMPILE_PARA} ..
    numProc=$(cat /proc/cpuinfo | grep processor | wc -l)
    make -j${numProc}
    if [ $? -ne 0 ]; then
        echo "ERROR: make error"
        exit 1
    fi

    echo "INFO: Compile backup successfully"
    return 0
}

function build_dtutils()
{
    # note: 如果已存在编译生成文件则不会重复编译
    sh ${DT_UTILS_DIR}/mockcpp/build/build.sh
}

function build_scanner_test()
{
    cd ${TEST_DIR}
    mkdir -p "${TEST_DIR}/build"
    cd "${TEST_DIR}/build"

    # cmake & make
    if [ -d "${MODULE_ROOT_PATH}" ];then
        COMPILE_PARA="-DMODULE_ROOT_PATH=${MODULE_ROOT_PATH}"
    fi
    COMPILE_PARA="-DFILE=ON -DNAS=ON -D OBS=ON "${COMPILE_PARA}
    cmake  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ${COMPILE_PARA} ..
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

    compile_scanner
    if [ $? -ne 0 ];then
        exit 1
    fi
    build_dtutils
    build_scanner_test
    local ret=$?
    echo "Compile backup LLT code success"
    return $ret
}

main "$@"
exit $?

