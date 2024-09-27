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
# build AppPlugins_NAS
SCRIPT_PATH=$(cd $(dirname $0); pwd)
COMMON_PATH=${SCRIPT_PATH}/common  # COMMON_PATH 传递给common.sh
. ${COMMON_PATH}/common.sh
SCRIPT_NAME=$(basename $0)

if [ "${OS_TYPE}" = "AIX" ]; then
    libSuffix=*.a
    cpFlag=-UHrf
elif [ "${OS_TYPE}" = "SunOS" ]; then
    libSuffix=*.so
     cpFlag=-rf
else
    libSuffix=*.so
    cpFlag=-arf
fi

build_secodefuzz()
{
    dos2unix ${PLUGIN_ROOT_DIR}/third_open_src/secodefuzz/build.sh
    chmod u+x ${PLUGIN_ROOT_DIR}/third_open_src/secodefuzz/build.sh
    sh ${PLUGIN_ROOT_DIR}/third_open_src/secodefuzz/build.sh
    if [ $? -ne 0 ];then
        log_echo "ERROR" "Compile Osecodefuzz[${PLUGIN_ROOT_DIR}/third_open_src/secodefuzz] failed"
        return 1
    fi
}

cmake_all()
{
    log_echo "INFO" "begin to build NAS"
    typeset type="$1"
    COMPILE_PARA=""
    cd ${PLUGIN_ROOT_DIR}
    mkdir -p "${PLUGIN_ROOT_DIR}/build-cmake"
    cd "${PLUGIN_ROOT_DIR}/build-cmake"
    rm -rf build-*
    if [ "${type}" = "LLT" ]; then
        echo "This is LLT compile" > build-llt_mark
        COMPILE_PARA=" -D LLT=ON "
    elif [ "$1" = "DTFUZZ" ]; then
        echo "This is DTFUZZ compile" > build-dtfuzz_mark
        build_secodefuzz
        COMPILE_PARA=" -D DTFUZZ=ON "
    elif [ "${type}" = "ASAN" ]; then
        echo "This is ASAN compile" > build-asan_mark
        COMPILE_PARA=" -D ASAN=ON "
    elif [ "${type}" = "TSAN" ]; then
        echo "This is TSAN compile" > build-asan_mark
        COMPILE_PARA=" -D TSAN=ON "
    elif [[ "${type}" = "Release"  || "$BUILD_TYPE" = "Release" || "$BUILD_TYPE" = "release" ]]; then
        echo "This is Release compile" > build-asan_mark
        COMPILE_PARA=" -D CMAKE_BUILD_TYPE=Release "
    else
        COMPILE_PARA=" "
    fi

    if [ ! -z "${EXEC_FILE_NAME}" ];then
        COMPILE_PARA="${COMPILE_PARA} -D EXEC_FILE_NAME=${EXEC_FILE_NAME}"
    fi
    # 内置插件，需要编译特殊处理
    if [ "X${PLUGIN_TYPE}" == "X1" ];then
        COMPILE_PARA="${COMPILE_PARA} -D INTERNAL_PLUGIN=ON"
    fi
    log_echo "INFO" "cmake ${COMPILE_PARA} .."
    cmake ${COMPILE_PARA} ${PLUGIN_ROOT_DIR}/src
    if [ $? -ne 0 ]; then
        log_echo "ERROR" "cmake error"
        exit 1
    fi

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

copy_framework_lib_2_special_path()
{
    typeset libPath="${PLUGIN_ROOT_DIR}/build-cmake"
    typeset count=$(find ${libPath} -name ${libSuffix} 2>/dev/null | wc -l)
    if [ $count -gt 0 ]; then
        mkdir -p ${PLUGIN_FRAMEWORK_LIB_PATH}
        find ${libPath} -name ${libSuffix} | xargs -I {} cp ${cpFlag} {} "${PLUGIN_FRAMEWORK_LIB_PATH}"
    fi
}

create_output_pkg_dir()
{
    # 拷贝lib
    rm -rf ${PLUGIN_PACKAGE_PATH}
    mkdir -p ${PLUGIN_PACKAGE_PATH}
    if [ "X${PLUGIN_TYPE}" == "X1" ]; then
        # for NasPlugin
        cp -rf ${PLUGIN_ROOT_DIR}/lib ${PLUGIN_PACKAGE_PATH}
    else
        mkdir -p ${PLUGIN_PACKAGE_PATH}/lib
        mkdir -p ${PLUGIN_PACKAGE_PATH}/lib/3rd
        if [ -d ${PLUGIN_ROOT_DIR}/lib/dme ]; then
            # for FusionComputePlugin
            cp -rf ${PLUGIN_ROOT_DIR}/lib/dme ${PLUGIN_PACKAGE_PATH}/lib
        fi
    fi

    # 拷贝可执行文件到bin
    mkdir -p ${PLUGIN_PACKAGE_PATH}/bin
    typeset cmakeDir=${PLUGIN_ROOT_DIR}/build-cmake
    typeset executable_file=AgentPlugin
    if [ ! -f ${cmakeDir}/startup/${executable_file} ];then
        log_echo "ERROR" "Not exist executable file."
        exit 1
    fi
    cp -f ${cmakeDir}/startup/${executable_file} ${PLUGIN_PACKAGE_PATH}/bin

    # 拷贝启动脚本
    if [ -d ${PLUGIN_ROOT_DIR}/build/bin ];then
        cp -rf ${PLUGIN_ROOT_DIR}/build/bin/* ${PLUGIN_PACKAGE_PATH}/bin
        dos2unix ${PLUGIN_PACKAGE_PATH}/bin/* 2>/dev/null
        chmod 500 ${PLUGIN_PACKAGE_PATH}/bin/*
    fi

    # 拷贝框架配置文件 app_lib.json
    mkdir -p ${PLUGIN_PACKAGE_PATH}/conf
    cp -rf ${PLUGIN_ROOT_DIR}/conf/app_lib.json  ${PLUGIN_PACKAGE_PATH}/conf/app_lib.json 2>/dev/null

    # 启动脚本
    cp -rf ${PLUGIN_ROOT_DIR}/build/install/*.sh ${PLUGIN_PACKAGE_PATH}/
    dos2unix ${PLUGIN_PACKAGE_PATH}/*.sh 2>/dev/null
    chmod 500 ${PLUGIN_PACKAGE_PATH}/*.sh

    mkdir -p ${PLUGIN_PACKAGE_PATH}/lib/service
    mkdir -p ${PLUGIN_PACKAGE_PATH}/script
    mkdir -p ${PLUGIN_PACKAGE_PATH}/install
}

main()
{
    typeset type="$1"
    if [ "X${type}" == "Xclean" ]; then
        rm -rf ${PLUGIN_ROOT_DIR}/build-cmake
        rm -rf ${PLUGIN_FRAMEWORK_LIB_PATH}/$libSuffix
        rm -rf ${PLUGIN_PACKAGE_PATH}
        log_echo "INFO" "Finish to clean build-make folder"
        return 0
    fi

    cmake_all "$@"
    typeset ret=$?
    copy_framework_lib_2_special_path
    if [ $? -ne 0 ]; then
        log_echo "ERROR" "copy lib error"
    fi

    create_output_pkg_dir
    if [ $? -ne 0 ]; then
        log_echo "ERROR" "create pkg path error"
    fi
    return $ret
}

main "$@"
exit $?