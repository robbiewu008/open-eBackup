#!/bin/bash
#
#  This file is a part of the open-eBackup project.
#  This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
#  If a copy of the MPL was not distributed with this file, You can obtain one at
#  http://mozilla.org/MPL/2.0/.
# 
#  Copyright (c) [2024] Huawei Technologies Co.,Ltd.
# 
#  THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
#  EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
#  MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
#

# 初始化环境变量
source ./common.sh

function copy_lib_2_special_path()
{
    local libPath="${PROJECT_ROOT_PATH}/build-cmake"
    local count=$(find ${libPath} -name "*.so" 2>/dev/null | wc -l)
    if [ $count -gt 0 ]; then
        mkdir -p ${PLUGIN_VIRT_LIB_PATH}
        find ${libPath} -name "*.so" | xargs -I {} cp -arf {} "${PLUGIN_VIRT_LIB_PATH}"
    fi
}

function DownloadMoudleRel()
{
    cd ${MODULE_PATH}/build/
    echo "download Module_rel from branch: " ${MODULE_BRANCH}
    sh download_lib_from_cmc.sh ${MODULE_BRANCH}
    cd -
}

function python_check()
{
    if [ ! -f "/lib64/libpython3.so" ]; then
        echo "Do not find libpython3.so, ln python3."
        if [ -f "/lib64/libpython3.9.so.1.0" ]; then
            echo "Link python3.9.so to 3.so!"
            ln -s /lib64/libpython3.9.so.1.0 /lib64/libpython3.so
        fi
    fi
}

function cmake_all()
{
    log_echo "INFO" "begin to build virtualization plugin"
    local type="$1"
    COMPILE_PARA=""
    BUILD_MODULE=""
    cd ${PROJECT_ROOT_PATH}
    mkdir -p "${PROJECT_ROOT_PATH}/build-cmake"
    cd "${PROJECT_ROOT_PATH}/build-cmake"
    rm -rf build-*
    if [ "${type}" = "LLT" ]; then
        echo "This is LLT compile" > build-llt_mark
        RUN_LLT="true"
        BUILD_MODULE="LLT"
    elif [ "${type}" = "DTFUZZ" ]; then
        echo "This is DTFUZZ compile" > build-dtfuzz_mark
        COMPILE_PARA=" -D DTFUZZ=ON "
    elif [ "${type}" = "ASAN" ]; then
        echo "This is ASAN compile" > build-asan_mark
        COMPILE_PARA=" -D ASAN=ON "
    elif [ "${type}" = "UBSAN" ]; then
        echo "This is UBSAN compile" > build-asan_mark
        COMPILE_PARA=" -D UBSAN=ON "
    elif [ "${type}" = "TSAN" ]; then
        echo "This is TSAN compile" > build-asan_mark
        COMPILE_PARA=" -D TSAN=ON "
    elif [ "${type}" = "TRACEPOINT" ]; then
        echo "This is TRACEPOINT compile"
        COMPILE_PARA=" -D TRACEPOINT=ON "
    elif [[ "${type}" = "Release"  || "$BUILD_TYPE" = "Release" || "$BUILD_TYPE" = "release" ]]; then
        echo "This is Release compile" > build-asan_mark
        COMPILE_PARA=" -D CMAKE_BUILD_TYPE=Release "
    else
        COMPILE_PARA=" "
    fi

    if [ "${type}" = "TRACEPOINT" ]; then
        DownloadMoudleRel
    fi    

    log_echo "INFO" "cmake ${COMPILE_PARA} ${PROJECT_ROOT_PATH}/plugins/virtualization"
    if [ -z ${BUILD_MODULE} ]; then
        cmake ${COMPILE_PARA} ${PROJECT_ROOT_PATH}/plugins/virtualization -DCMAKE_EXPORT_COMPILE_COMMANDS=YES -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
    else
        cmake ${COMPILE_PARA} ${PROJECT_ROOT_PATH}/plugins/virtualization -DCMAKE_EXPORT_COMPILE_COMMANDS=YES -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -D${BUILD_MODULE}=ON
    fi

    if [ $? -ne 0 ]; then
        log_echo "ERROR" "cmake error"
        exit 1
    fi

    numProc=$(cat /proc/cpuinfo | grep processor | wc -l)
    make -j${numProc}
    if [ $? -ne 0 ]; then
        log_echo "ERROR" "make error"
        exit 1
    fi

    # 编译完拷贝库
    copy_lib_2_special_path

    if [ "$RUN_LLT" = "true" ]; then
        pushd ${PROJECT_ROOT_PATH}/plugins/virtualization
        sh make-test.sh
        if [ $? != 0 ]; then
            popd
            return 1
        fi
        popd
    fi
}

function main()
{
    local type="$1"
    if [ "X${type}" == "Xclean" ]; then
        rm -rf ${PROJECT_ROOT_PATH}/build-cmake
        rm -rf ${PLUGIN_VIRT_LIB_PATH}/*.so
        rm -rf ${PROJECT_ROOT_PATH}/deps
        pushd ${FRAMEWORK_PATH}/build
        sh build_framework.sh clean
        popd
        log_echo "INFO" "Finish to clean build-make folder"
        return 0
    fi

    pushd ${FRAMEWORK_PATH}/build
    sh build_framework.sh
    if [ $? -ne 0 ]; then
      log_echo "ERROR" "build_framework.sh failed."
      return 1
    fi
    popd

    ### 虚拟化开源下载+安装 start
    sh ${PROJECT_ROOT_PATH}/plugins/virtualization/build/download_virt_deps.sh
    if [ $? -ne 0 ]; then
        log_echo "ERROR" "download_virt_deps.sh failed."
        return 1
    fi
    sh ${PROJECT_ROOT_PATH}/plugins/virtualization/build/extracte_virt_deps.sh
    if [ $? -ne 0 ]; then
        log_echo "ERROR" "extracte_virt_deps.sh failed."
        return 1
    fi
    sh ${PROJECT_ROOT_PATH}/plugins/virtualization/build/install_yamlcpp.sh
    if [ $? -ne 0 ]; then
        log_echo "ERROR" "install_yamlcpp.sh failed."
        return 1
    fi

    # 编译vrmVBSTools.jar包
    sh ${MODULE_PATH}/build/vbstool_make.sh
    if [ $? -ne 0 ]; then
        log_echo "ERROR" "vbstool_make.sh failed."
        return 1
    fi
    ### 虚拟化开源下载+安装 end
    python_check
    cmake_all "$@"
    return $?
}

main "$@"
exit $?
