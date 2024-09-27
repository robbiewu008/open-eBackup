#!/bin/sh
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
set +x
#!/usr/bin/expect

#@function: mount nas share path
 
AGENT_ROOT_PATH=$1
PID=$2
PARAM_NUM=$3
. "${AGENT_ROOT_PATH}/sbin/agent_sbin_func.sh"
 
#for log
LOG_FILE_NAME="${LOG_PATH}/sanclientaction.log"

BLOCK_LIST="^/$\|^/bin$\|^/bin/.*\|^/boot$\|^/boot/.*\|^/dev$\|^/dev/.*\|^/etc$\|^/etc/.*\|^/lib$\|^/lib/.*\|^/lost+found$\|^/lost+found/.*\|^/media$\|^/media/.*\|^/mnt$\|^/opt$\|^/proc$\|^/proc/.*\|^/root$\|^/root/.*\|^/sbin$\|^/sbin/.*\|^/selinux$\|^/selinux/.*\|^/srv$\|^/srv/.*\|^/sys$\|^/sys/.*\|^/usr$\|^/usr/.*\|^/var$\|^/var/.*\|^/run$\|^/run/.*"
WHITE_LIST=("^/mnt/databackup/.*/.*$")

CheckWhiteList() {
    InputPath="$1"
    if [ ! -e "${InputPath}" ]; then
        Log "Path: ${InputPath} is not exists."
        return 2
    fi
    AbsolutePath=`realpath ${InputPath}`
    Log "AbsolutePath:${AbsolutePath}"
    for wdir in "${WHITE_LIST[@]}"; do
        if [[ $AbsolutePath =~ $wdir ]]; then
            return 0
        fi
    done
    return 1
}

Log "********************************Start to copy sanclient log meta file********************************"
 
# global var, for kill monitor
PARAM_CONTENT=`ReadInputParam`
test -z "$PARAM_CONTENT"              && ExitWithError "PARAM_FILE"
 
#for GetValue
SrcPath=`GetValue "${PARAM_CONTENT}" srcPath`
DesPath=`GetValue "${PARAM_CONTENT}" desPath`

CheckWhiteList "${SrcPath}"
if [ $? -ne 0 ]; then
    Log "ERROR: SrcPath[${SrcPath}] is not in the whiteList."
    exit ${ERROR_MOUNT_FS}
fi

CheckWhiteList "${DesPath}"
if [ $? -ne 0 ]; then
    Log "ERROR: DesPath[${DesPath}] is not in the whiteList."
    exit ${ERROR_MOUNT_FS}
fi

if [ -f "${SrcPath}" ]; then
    fileName=`basename ${SrcPath}`
    DesFile=${DesPath}"/"${fileName}
    CheckWhiteList "${DesFile}"
    if [ $? -eq 1 ]; then
        Log "ERROR: DesFile[${DesFile}] is not in the whiteList."
        exit ${ERROR_MOUNT_FS}
    fi
fi

Log "SrcPath is ${SrcPath}"
Log "DesPath is ${DesPath}"

cp -f "${SrcPath}" "${DesPath}"
