# build AppPlugins_NAS
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
#！ /bin/bash

CURRENT_SCRIPT="${BASH_SOURCE[0]}"
SCRIPT_PATH=$(cd $(dirname ${CURRENT_SCRIPT});pwd)
source ${SCRIPT_PATH}/common/branch.sh
SCRIPT_NAME="${CURRENT_SCRIPT##*/}"
NAS_ROOT_DIR=$(cd "${SCRIPT_PATH}/.."; pwd)
DME_FOLDER_NAME=dep/dme
DME_ROOT="${NAS_ROOT_DIR}/${DME_FOLDER_NAME}"
THIRDSRC_FOLDER_NAME=third_open_src
OPEN_SRC="${NAS_ROOT_DIR}/${THIRDSRC_FOLDER_NAME}"
FRAMEWORK_REPO="ssh://git@szv-y.codehub.huawei.com:2222/dpa/CBB/CPP/Framework.git"
TRAS_REPO="ssh://git@szv-y.codehub.huawei.com:2222/dpa/CBB/CPP/Data_Transmission_Frame.git"

function log_echo()
{
    local message="$1"
    echo "[$(date "+%Y-%m-%d %H:%M:%S")][${SCRIPT_NAME}][$(whoami)] ${message}"
}

function clean_pkg()
{
    rm -rf "${DME_ROOT}/Data_Transmission_Frame"
    rm -rf "${DME_ROOT}/Framework"
    log_echo "Finish to clean pkg files"
}

function git_download_code()
{
    local repo="$1"
    if [ -z "${repo}" ];then
        log_echo "not exist the repo[${repo}]"
        return 1
    fi
    local branch="$2"
    local folderName="$3"
    git clone "${repo}" "${folderName}"
    if [ $? -ne 0 ];then
        log_echo "Failed to clone the repo[${repo}]"
        return 1
    fi
    cd "${folderName}"
       git checkout "${branch}"
    cd -
    return $?
}

function main()
{
    # 清理目录
    clean_pkg

    mkdir -p ${DME_ROOT}
    cd ${DME_ROOT}

    # 下载依赖的DME代码
    git_download_code "${FRAMEWORK_REPO}" "SUB_SYSTEM" Framework

    git_download_code "${TRAS_REPO}" "SUB_SYSTEM" Data_Transmission_Frame
    cd - >/dev/null

    return $?
}

main "$@"
exit $?