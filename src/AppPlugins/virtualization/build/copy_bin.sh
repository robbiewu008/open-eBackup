#!/bin/sh
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

EBACK_BASE_DIR="$(cd "$(dirname "$BASH_SOURCE")/../";pwd)"

FRAMEWORK_PATH="${EBACK_BASE_DIR}/../../framework"
OUTPUT_PKG_PATH="${FRAMEWORK_PATH}/output_pkg"

TARGET_BIN_PATH=$1
# 检查参数是否为空
if [ -z "$TARGET_BIN_PATH" ]; then
    echo "Code path is empty, please input the code path."
    exit 1
fi

function copy_bin_files()
{
    if [ ! -d "${TARGET_BIN_PATH}/AppPlugins_virtualization" ]; then
        mkdir -p ${TARGET_BIN_PATH}/AppPlugins_virtualization
    fi
    cp -rf ${OUTPUT_PKG_PATH}/* "${TARGET_BIN_PATH}/AppPlugins_virtualization"
    if [ $? -ne 0 ];then
        echo -e "Copy bin files fail"
        exit 1
    fi

    if [ ! -d "${TARGET_BIN_PATH}/AppPlugins_virtualization" ]; then
        mkdir -p ${TARGET_BIN_PATH}/AppPlugins_virtualization
    fi
    echo "cp -rf ${PROJECT_ROOT_PATH}/lib ${TARGET_BIN_PATH}/AppPlugins_virtualization/"
    cp -rf ${PROJECT_ROOT_PATH}/lib ${TARGET_BIN_PATH}/AppPlugins_virtualization/

    if [ ! -d "${TARGET_BIN_PATH}/AppPlugins_virtualization/deps" ]; then
        mkdir -p ${TARGET_BIN_PATH}/AppPlugins_virtualization/deps
    fi
    cp -rf ${PROJECT_ROOT_PATH}/deps/local ${TARGET_BIN_PATH}/AppPlugins_virtualization/deps
    # 递归拷贝所有Framework依赖的so文件和头文件
    if [ ! -d "${TARGET_BIN_PATH}/AppPlugins_virtualization/framework" ]; then
        mkdir -p ${TARGET_BIN_PATH}/AppPlugins_virtualization/framework
    fi
    echo "cp -rf ${PROJECT_ROOT_PATH}/framework/lib ${TARGET_BIN_PATH}/AppPlugins_virtualization/framework/"
    cp -rf ${PROJECT_ROOT_PATH}/framework/lib ${TARGET_BIN_PATH}/AppPlugins_virtualization/framework/

    echo "cp -rf ${PROJECT_ROOT_PATH}/framework/dep ${TARGET_BIN_PATH}/AppPlugins_virtualization/framework/"
    cp -rf ${PROJECT_ROOT_PATH}/framework/dep ${TARGET_BIN_PATH}/AppPlugins_virtualization/framework/

    echo "cp -rf ${PROJECT_ROOT_PATH}/framework/inc ${TARGET_BIN_PATH}/AppPlugins_virtualization/framework/"
    cp -rf ${PROJECT_ROOT_PATH}/framework/inc ${TARGET_BIN_PATH}/AppPlugins_virtualization/framework/
    # 递归拷贝所有Module依赖的so文件和头文件
    if [ ! -d "${TARGET_BIN_PATH}/AppPlugins_virtualization/Module" ]; then
        mkdir -p ${TARGET_BIN_PATH}/AppPlugins_virtualization/Module
    fi
    cp -rf ${MODULE_OPEN_SRC_PATH} ${TARGET_BIN_PATH}/AppPlugins_virtualization/Module/
    find ${TARGET_BIN_PATH}/AppPlugins_virtualization/Module/src/third_open_src -name "*.cpp" -type f -delete
    cp -rf ${MODULE_PLATFORM_PATH} ${TARGET_BIN_PATH}/AppPlugins_virtualization/Module/
    find ${TARGET_BIN_PATH}/AppPlugins_virtualization/Module/src/platform -name "*.cpp" -type f -delete

    if [ ! -d "${TARGET_BIN_PATH}/AppPlugins_virtualization/Module/src" ]; then
        mkdir -p ${TARGET_BIN_PATH}/AppPlugins_virtualization/Module/src
    fi
    cp -rf ${MODULE_PATH}/src/* ${TARGET_BIN_PATH}/AppPlugins_virtualization/Module/src/
    echo "find ${TARGET_BIN_PATH}/AppPlugins_virtualization/Module/src/ -name *.cpp -type f -delete"
    find ${TARGET_BIN_PATH}/AppPlugins_virtualization/Module/src/ -name "*.cpp" -type f -delete
}

function main()
{
    copy_bin_files
    if [ $? -ne 0 ]; then
        echo "#########################################################"
        echo "   Copy ${MS_NAME} bin to opensource directory failed."
        echo "#########################################################"
        exit 1
    else
        echo "#########################################################"
        echo "   Copy ${MS_NAME} bin to opensource directory completed."
        echo "#########################################################"
    
    fi

}

main "$@"
exit $?