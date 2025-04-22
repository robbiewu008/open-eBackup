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

# open-eBackup-bin
OPEN_ROOT_PATH=${WORKSPACE}
OPEN_OBLIGATION_ROOT_PATH=${binary_path}
if [ -z "$OPEN_OBLIGATION_ROOT_PATH" ]; then
    echo "ERROR: Please export binary_path={open-source-obligation path}"
    exit 1
fi

build_type=$1
OPEN_VIRT_PATH="${OPEN_OBLIGATION_ROOT_PATH}/Plugins/Linux"
REST_VIRT_PATH="${OPEN_ROOT_PATH}/REST_API/src/AppPlugins/virtualization"
source ${REST_VIRT_PATH}/build/common_opensource.sh
FRAMEWORK_PATH="${OPEN_ROOT_PATH}/REST_API/src/AppPlugins/common/framework"
MODULE_PATH="${OPEN_ROOT_PATH}/REST_API/src/AppPlugins/common/Module"
REST_VIRT_LIB_PATH="${REST_VIRT_PATH}/lib"

SYS_NAME=`arch`
if [ "${SYS_NAME}" = "aarch64" ]; then
    VIRT_PACK="VirtualizationPlugin_aarch64.tar.xz"
elif [ "${SYS_NAME}" = "x86_64" ]; then
    VIRT_PACK="VirtualizationPlugin_Linux_x86_64.tar.xz"
else
    echo "ERROR: Unsupport OS."
    exit 1
fi
OPEN_VIRT_PACK="${OPEN_VIRT_PATH}/${SYS_NAME}/${VIRT_PACK}"

YAMLCPP=yaml-cpp-yaml-cpp-0.6.3

Framework_lib_list=(
    ${FRAMEWORK_PATH}/lib
    ${FRAMEWORK_PATH}/lib/agent_sdk
)

Module_lib_list=(
    ${MODULE_PATH}/lz4_rel/lib
    ${MODULE_PATH}/boost_rel/lib
    ${MODULE_PATH}/jsoncpp_rel/libs
    ${MODULE_PATH}/curl_rel/lib/
    ${MODULE_PATH}/thrift_rel/lib
    ${MODULE_PATH}/libaio_rel/lib
    ${MODULE_PATH}/tinyxml2_rel/lib
    ${MODULE_PATH}/SecureCLib_rel/lib
)

Framework_inc_list=(
    ${FRAMEWORK_PATH}/inc
    ${FRAMEWORK_PATH}/inc/common
    ${FRAMEWORK_PATH}/inc/client
    ${FRAMEWORK_PATH}/inc/rpc
    ${FRAMEWORK_PATH}/inc/rpc/certificateservice/
    ${FRAMEWORK_PATH}/inc/thrift_interface
)
Module_inc_list=(
    ${MODULE_PATH}/boost_rel/include
    ${MODULE_PATH}/lz4_rel/include
    ${MODULE_PATH}/jsoncpp_rel/include
    ${MODULE_PATH}/curl_rel/include
    ${MODULE_PATH}/openssl_rel/include
    ${MODULE_PATH}/thrift_rel/include
    ${MODULE_PATH}/libaio_rel/include
    ${MODULE_PATH}/esdk_rel/include
    ${MODULE_PATH}/tinyxml2_rel/include
    ${MODULE_PATH}/libssh2_rel/include
    ${MODULE_PATH}/platform/SecureCLib_rel/include
    ${REST_VIRT_PATH}/deps/local/include
)

FusionStorage_list=(
    ${REST_VIRT_PATH}/vbstool
    ${REST_VIRT_PATH}/vbstool/conf
    ${REST_VIRT_PATH}/vbstool/lib
)

FusionStorage_file_list=(
    ${REST_VIRT_PATH}/vbstool/vrmVBSTool.sh
    ${REST_VIRT_PATH}/vbstool/lib/vrmVBSTool.jar

)

function copy_lib_2_special_path()
{
    local libPath="${REST_VIRT_PATH}/build-cmake"
    local count=$(find ${libPath} -name "*.so" 2>/dev/null | wc -l)
    if [ $count -gt 0 ]; then
        mkdir -p ${REST_VIRT_LIB_PATH}
        find ${libPath} -name "*.so" | xargs -I {} cp -arf {} "${REST_VIRT_LIB_PATH}"
    fi
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

function uncompress_deps
{
    local tmp_path="${REST_VIRT_PATH}/tmps"
    mkdir -p "${tmp_path}"
    cp "${OPEN_VIRT_PACK}" "${tmp_path}"
    xz -d "${tmp_path}/${VIRT_PACK}"
    tar -xvf "${tmp_path}/VirtualizationPlugin_aarch64.tar" -C ${tmp_path}
    xz -d  "${tmp_path}/VirtualizationPlugin_Linux_aarch64.tar.xz"
    tar -xvf "${tmp_path}/VirtualizationPlugin_Linux_aarch64.tar" -C ${tmp_path}

    if [ ! -d "${REST_VIRT_PATH}/lib" ]; then
        mkdir -p ${REST_VIRT_PATH}/lib
    fi
    cp -arf ${tmp_path}/lib/service/* "${REST_VIRT_PATH}/lib"

    if [ ! -d "${REST_VIRT_PATH}/vbstool" ]; then
        mkdir -p ${REST_VIRT_PATH}/vbstool
    fi
    cp -arf ${tmp_path}/vbstool/* "${REST_VIRT_PATH}/vbstool"

    rm -rf "${tmp_path}"
}

function cmake_all()
{
    log_echo "INFO" "begin to build virtualization plugin"
    local type="$1"
    COMPILE_PARA=""
    BUILD_MODULE=""
    cd ${REST_VIRT_PATH}
    mkdir -p "${REST_VIRT_PATH}/build-cmake"
    cd "${REST_VIRT_PATH}/build-cmake"
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
        COMPILE_PARA=" -D CMAKE_BUILD_TYPE=Release -D OPENSOURCE=ON "
    else
        COMPILE_PARA=" "
    fi

    log_echo "INFO" "cmake ${COMPILE_PARA} ${REST_VIRT_PATH}"
    if [ -z ${BUILD_MODULE} ]; then
        cmake ${COMPILE_PARA} ${REST_API_PATH} -DCMAKE_EXPORT_COMPILE_COMMANDS=YES -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
    else
        cmake ${COMPILE_PARA} ${REST_VIRT_PATH} -DCMAKE_EXPORT_COMPILE_COMMANDS=YES -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -D${BUILD_MODULE}=ON
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
        pushd REST_VIRT_PATH
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
        rm -rf ${REST_VIRT_PATH}/build-cmake
        rm -rf ${REST_VIRT_PATH}/lib/*.so
        log_echo "INFO" "Finish to clean build-make folder"
        return 0
    fi
    uncompress_deps
    ### 检查Framework
    for path in "${Framework_lib_list[@]}"
    do
        # 递归检查所有依赖的so文件
        if [ ! -d "$path" ]; then
            echo "$path not exist, failed!"
            exit 1
        fi
    done

    for path in "${Framework_inc_list[@]}"
    do
        # 递归检查所有依赖的.h文件
        if [ ! -d "$path" ]; then
            echo "$path not exist, failed!"
            exit 1
        fi
    done

    ### 虚拟化开源检查
    # vrmVBSTools.jar包相关检查
    for path in "${FusionStorage_list[@]}"
    do
        # 递归检查所有依赖的.h文件
        if [ ! -d "$path" ]; then
            echo "$path not exist, failed!"
            exit 1
        fi
    done

    for file in "${FusionStorage_file_list[@]}"
    do
        # 递归检查所有依赖的.h文件
        if [ ! -f "$file" ]; then
            echo "$file not exist, failed!"
            exit 1
        fi
    done
    ### 虚拟化开源下载+安装 end

    python_check
    cmake_all "$@"
    return $?
}

main "$@"
exit $?
