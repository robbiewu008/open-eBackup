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
#!/bin/bash

CURRENT_SCRIPT="${BASH_SOURCE[0]}"
SCRIPT_PATH=$(dirname ${CURRENT_SCRIPT})
SCRIPT_NAME="${CURRENT_SCRIPT##*/}"
FILE_ROOT_DIR=$(cd "${SCRIPT_PATH}/.."; pwd)
SCANNER_REPO="ssh://git@szv-y.codehub.huawei.com:2222/dpa/CBB/CPP/FS_Scanner.git"
BACKUP_REPO="ssh://git@szv-y.codehub.huawei.com:2222/dpa/CBB/CPP/FS_Backup.git"
MODULE_REPO="ssh://git@szv-y.codehub.huawei.com:2222/dpa/CBB/CPP/Module.git"
MODULE_BRANCH=$1
if [ -z ${MODULE_BRANCH} ]
then
   MODULE_BRANCH=develop_backup_software_1.6.0RC1
fi

function log_echo()
{
    local message="$1"
    echo "[$(date "+%Y-%m-%d %H:%M:%S")][${SCRIPT_NAME}][$(whoami)] ${message}"
}

function clean_pkg()
{
    rm -rf "${FILE_ROOT_DIR}/../../FS_Scanner"
    rm -rf "${FILE_ROOT_DIR}/../../FS_Backup"
    rm -rf "${FILE_ROOT_DIR}/../../Module"
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

    cd ${FILE_ROOT_DIR}/../..
    git_download_code "${MODULE_REPO}" "${MODULE_BRANCH}" "Module"

    # 下载依赖的代码
    git_download_code "${SCANNER_REPO}" "${MODULE_BRANCH}" FS_Scanner

    git_download_code "${BACKUP_REPO}" "${MODULE_BRANCH}" FS_Backup

    cd - >/dev/null
    return 0
}

main "$@"
exit $?
