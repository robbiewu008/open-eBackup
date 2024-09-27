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
# /
umask 0022

# 初始化环境变量
source ./common.sh

function extract_pkg()
{
    local MODULE_NAME="$1"
    PKG_PATH="${VIRT_DEPS_EXT_PKG_PATH}/${MODULE_NAME}.tar.gz"
    EXTRACTED_FOLDER_PATH="${VIRT_DEPS_SRCS_PATH}/${MODULE_NAME}"

    SRC_VAR=$(ls ${EXTRACTED_FOLDER_PATH} 2>/dev/null)
    if [ -n "${SRC_VAR}" ]; then
        log_echo "INFO" "${MODULE_NAME} Library ${EXTRACTED_FOLDER_PATH} already exists"
        return 0
    fi

    tar zxf ${PKG_PATH} -C ${VIRT_DEPS_SRCS_PATH}
    if [ $? -ne 0 ]; then
        log_echo "ERROR" "tar zxf ${PKG_PATH} to ${VIRT_DEPS_SRCS_PATH} failed"
        return 1
    fi
    log_echo "DEBUG" "Check the ${EXTRACTED_FOLDER_PATH} extracted path"
    ls -l ${EXTRACTED_FOLDER_PATH}
    return 0
}

function main()
{
    mkdir -p ${VIRT_DEPS_SRCS_PATH}
    extract_pkg ${YAMLCPP}
    if [ $? -ne 0 ]; then
        return 1
    fi

    return 0
}

main