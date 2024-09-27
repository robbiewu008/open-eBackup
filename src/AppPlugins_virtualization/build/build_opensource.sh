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
source ./common_opensource.sh

function copy_lib_2_special_path()
{
    local libPath="${REST_API_PATH}/build-cmake"
    local count=$(find ${libPath} -name "*.so" 2>/dev/null | wc -l)
    if [ $count -gt 0 ]; then
        mkdir -p ${OPEN_VIRT_LIB_PATH}
        find ${libPath} -name "*.so" | xargs -I {} cp -arf {} "${OPEN_VIRT_LIB_PATH}"
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

function copy_deps()
{
    local deps_path="${REST_API_PATH}/deps"
    if [ ! -d "${deps_path}" ]; then
        mkdir -p ${deps_path}
    fi
    cp -arf ${OPENSOURCE_PATH}/deps/* ${deps_path}/

    local module_path="${REST_API_PATH}/Module"
    if [ ! -d "${module_path}" ]; then
        mkdir -p ${module_path}
    fi
    cp -arf ${OPENSOURCE_PATH}/Module/* ${module_path}/

    local framework_path="${REST_API_PATH}/framework"
    if [ ! -d "${framework_path}" ]; then
        mkdir -p ${framework_path}
    fi
    cp -arf ${OPENSOURCE_PATH}/framework/* ${framework_path}/

    local vir_lib_path="${REST_API_PATH}/lib"
    if [ ! -d "${vir_lib_path}" ]; then
        mkdir -p ${vir_lib_path}
    fi
    cp -arf ${OPENSOURCE_PATH}/lib/* ${vir_lib_path}/
}

function cmake_all()
{
    copy_deps
    if [ $? -ne 0 ]; then
        log_echo "ERROR" "copy_deps error"
        exit 1
    fi

    log_echo "INFO" "begin to build virtualization plugin"
    local type="$1"
    COMPILE_PARA=""
    BUILD_MODULE=""
    cd ${REST_API_PATH}
    mkdir -p "${REST_API_PATH}/build-cmake"
    cd "${REST_API_PATH}/build-cmake"
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

    log_echo "INFO" "cmake ${COMPILE_PARA} ${REST_API_PATH}/plugins/virtualization"
    if [ -z ${BUILD_MODULE} ]; then
        cmake ${COMPILE_PARA} ${REST_API_PATH}/plugins/virtualization -DCMAKE_EXPORT_COMPILE_COMMANDS=YES -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
    else
        cmake ${COMPILE_PARA} ${REST_API_PATH}/plugins/virtualization -DCMAKE_EXPORT_COMPILE_COMMANDS=YES -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -D${BUILD_MODULE}=ON
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
        pushd ${REST_API_PATH}/plugins/virtualization
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
        rm -rf ${REST_API_PATH}/build-cmake
        rm -rf ${REST_API_PATH}/lib/*.so
        log_echo "INFO" "Finish to clean build-make folder"
        return 0
    fi

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
