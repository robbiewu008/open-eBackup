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
SCRIPT_NAME=$(basename $0)
SCRIPT_PATH=$(cd $(dirname $0); pwd)
BUILD_ROOT=$(cd "${SCRIPT_PATH}/.."; pwd)
# 脚本参数类型
TYPE_LIST="LLT ASAN TSAN Release Debug"
MODULE_LIST=""
COMPLIE_PARA=""
#################### 获 取 入 参 ##########################
# 设置默认值
TYPE=
NAS="OFF"

for option
do
    case $option in
    
    -type=* | --type=*)
      TYPE=`expr "x$option" : "x-*type=\(.*\)"`
      ;;

    -NAS=* | --NAS=*)
      NAS=`expr "x$option" : "x-*NAS=\(.*\)"`
      ;;

    esac
done

echo "[INFO] Compile TYPE: ${TYPE}"
echo "[INFO] NAS: ${NAS}"
#################### 获 取 入 参 ##########################

help()
{
    echo "This script executable method as folows:"
    echo "${SCRIPT_NAME} TYPE=TYPE_VALUE MODULE=MODULE_VAULE"
    echo "TYPE_VALUE is one of type list [${TYPE_LIST}], the default is Debug"
    echo "MODULE is one or more module of list[${MODULE_LIST}],if more, the module vaule is split by dot, the default is compiling all module"
}

get_paras()
{
    if [ -z "${TYPE}" ];then
        echo "use debug"
    else
        echo "${TYPE_LIST}" | grep -q "${TYPE}"
        if [ $? -ne 0 ];then
            help
            exit 1
        fi
        if [ "${TYPE}" = "LLT" ]; then
            echo "This is LLT compile" > build-llt_mark
            COMPILE_PARA=" -D LLT=ON "
        elif [ "${TYPE}" = "ASAN" ]; then
            echo "This is ASAN compile" > build-asan_mark
            COMPILE_PARA=" -D ASAN=ON "
        elif [ "${TYPE}" = "TSAN" ]; then
            echo "This is TSAN compile" > build-asan_mark
            COMPILE_PARA=" -D TSAN=ON "
        elif [[ "${TYPE}" = "Release"  || "$BUILD_TYPE" = "Release" || "$BUILD_TYPE" = "release" ]]; then
            echo "This is Release compile" > build-asan_mark
            COMPILE_PARA=" -D CMAKE_BUILD_TYPE=Release "
        else
            COMPILE_PARA=" "
        fi
    fi
    if [ ${NAS} == "ON" ]; then
        COMPILE_PARA="${COMPILE_PARA} -DNAS=ON"
    fi
    echo ${COMPILE_PARA}
    return 0
}

build_module() {
    get_paras "$@"
    rm -rf ${BUILD_ROOT}/Module_rel
    cd ${BUILD_ROOT}
    mkdir -p "${BUILD_ROOT}/build-cmake"
    cd "${BUILD_ROOT}/build-cmake"

    # 获取逻辑cpu个数
    if [ "$(uname -s)" = "AIX" ]; then
        export OBJECT_MODE=64 && export CFLAGS=-maix64 && export CXXFLAGS=-maix64
        libName="*.a"
        array=$(bindprocessor -q)
        numProc=0
        for element in ${array[@]}
        do
            if [ $element -ge 0 ] 2>/dev/null; then
                let numProc+=1
            fi
        done
        numProc=4
    elif [ "$(uname -s)" = "SunOS" ]; then
        libName="*.so"
        numProc=4
    else
        libName="*.so"
        numProc=$(cat /proc/cpuinfo | grep processor | wc -l)
    fi
    # cmake & make
    cmake ${COMPILE_PARA} ../src
    make -j${numProc}
    if [ $? -ne 0 ]; then
        echo "ERROR" "make error"
        exit 1
    fi
    # 把Module的头文件和动态库打包到Module_rel，用于upload_pkg_to_cmc上传Module包到cmc上
    module_list=`ls ${BUILD_ROOT}/src/ | grep -v CMakeLists.txt`
    mkdir -p ${BUILD_ROOT}/Module_rel/lib/
    mkdir -p ${BUILD_ROOT}/lib/
    for module in ${module_list};do
        if [ -d ${BUILD_ROOT}/build-cmake/${module} ]; then
            find ${BUILD_ROOT}/build-cmake/${module} -name *${module}${libName} | xargs -I{} cp -f {} ${BUILD_ROOT}/Module_rel/lib
            find ${BUILD_ROOT}/build-cmake/${module} -name *${module}${libName} | xargs -I{} cp -f {} ${BUILD_ROOT}/lib
        fi

    done
    mkdir -p ${BUILD_ROOT}/Module_rel/src/
    for module in ${module_list};do
        mkdir -p ${BUILD_ROOT}/Module_rel/src/${module}
        cd ${BUILD_ROOT}/src
        for header_file in $(find ${module} -name "*.h" -o -name "*.hpp");do
            header_folder=${header_file#*/}
            header_folder=$(dirname ${header_folder})
            if [ ! -d ${BUILD_ROOT}/Module_rel/src/${module}/${header_folder} ];then
                mkdir -p ${BUILD_ROOT}/Module_rel/src/${module}/${header_folder}
            fi
            cp -rf ${header_file} ${BUILD_ROOT}/Module_rel/src/${module}/${header_folder}
        done
    done

}

main()
{
    build_module "$@"
}
main "$@"
exit 0
exit $?
