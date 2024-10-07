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
SCRIPT_PATH=$(dirname ${CURRENT_SCRIPT})
source ${SCRIPT_PATH}/common/branch.sh
SCRIPT_NAME="${CURRENT_SCRIPT##*/}"
BACKUP_ROOT_PATH=$(cd "${SCRIPT_PATH}/.."; pwd)

MODULE_REPO="ssh://git@szv-y.codehub.huawei.com:2222/dpa/CBB/CPP/Module.git"

function log_echo()
{
    local message="$1"
    echo "[$(date "+%Y-%m-%d %H:%M:%S")][${SCRIPT_NAME}][$(whoami)] ${message}"
}

function clean_pkg()
{
    rm -rf "${BACKUP_ROOT_PATH}/Module"
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

    cd ${BACKUP_ROOT_PATH}
    git_download_code "${MODULE_REPO}" "${MODULE_BRANCH}" Module
    cd - >/dev/null

    return $?
}

main "$@"
exit $?