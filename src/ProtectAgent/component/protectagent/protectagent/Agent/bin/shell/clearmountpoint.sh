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
set +x
# for clear mount points

AGENT_ROOT_PATH=$1
PID=$2
PARAM_NUM=$3
. "${AGENT_ROOT_PATH}/sbin/agent_sbin_func.sh"

LOG_FILE_NAME="${LOG_PATH}/clearmountpoints.log"

WHITE_LIST=(
"^/mnt/databackup/.*/.*$"
)

CheckWhiteList() {
    InputPath="$1"
    if [ ! -e "${InputPath}" ]; then
        Log "ERROR: Path: ${InputPath} is not exists."
        exit 1
    fi
    AbsolutePath=`realpath ${InputPath}`
    Log "AbsolutePath:${AbsolutePath}"
    for wdir in "${WHITE_LIST[@]}"
    do
        if [[ $AbsolutePath =~ $wdir ]]; then
            return 0
        fi
    done
    return 1
}

RemoveDir() {
    # 1. remove dir
    dirName="$1"
    CheckWhiteList "${dirName}"
    if [ $? -ne 0 ]; then
        Log "ERROR(rm): Dir[${dirName}] not in whitelist."
        return 1
    fi
    rm -rf ${dirName} >> $LOG_FILE_NAME 2>&1
}

PARAM_CONTENT=`ReadInputParam`
test -z "$PARAM_CONTENT"              && ExitWithError "PARAM_FILE"

#for GetValue
JobIDPath=`GetValue "${PARAM_CONTENT}" jobIDPath`

Log "JobIDPath=${JobIDPath}";

if [ ! -d "${JobIDPath}" ]; then
    Log "ERROR(rm): Path: ${JobIDPath} is not a dir."
    exit 1
fi

RemoveDir "${JobIDPath}"
if [ $? -ne 0 ]; then
    Log "Remove dir "${JobIDPath}" failed."
    exit 1
fi
exit 0
