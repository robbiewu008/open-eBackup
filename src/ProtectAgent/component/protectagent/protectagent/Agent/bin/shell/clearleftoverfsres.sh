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

############################################################################################
#program name:          clearfsleftoverfsres.sh
#function:              clear file system left over resource oracle backup media
#time:                  2020-11-14
# function              description
# rework:               First Programming
############################################################################################

##############parameter###############
AGENT_ROOT_PATH=$1
PID=$2
PARAM_NUM=$3
NeedLogFlg=1

#load the agent function library script
. "${AGENT_ROOT_PATH}/sbin/agent_sbin_func.sh"
LOG_FILE_NAME="${LOG_PATH}/clearleftoverfsres.log"

PARAM_FILE=`ReadInputParam`
test -z "$PARAM_FILE"              && ExitWithError "PARAM_FILE"

VGDATA=`GetValue "${PARAM_FILE}" VGDATA`
LVDATA=`GetValue "${PARAM_FILE}" LVDATA`
VGLOG=`GetValue "${PARAM_FILE}" VGLOG`
LVLOG=`GetValue "${PARAM_FILE}" LVLOG`
PathData=`GetValue "${PARAM_FILE}" PATHDATA`
PathLog=`GetValue "${PARAM_FILE}" PATHLOG`
Log "VGDATA=$VGDATA;LVDATA=$LVDATA;PathData=$PathData;VGLOG=$VGLOG;LVLOG=$LVLOG;PathLog=$PathLog."

if [ -z "$VGDATA" ] || [ -z "$LVDATA" ] || [ -z "$PathData" ] || [ -z "$VGLOG" ] || [ -z "$LVLOG" ] || [ -z "$PathLog" ]; then
    Log "parameter is invalid."
    exit 1
fi

ClearLVMInfo()
{
    vgName=$1
    lvName=$2
    mountPath=$3

    pvs 2>&1 | grep "Error reading device /dev/${vgName}/${lvName}" >> "${LOG_FILE_NAME}"
    local errRead=$?
    pvs 2>&1 | grep "${vgName}/${lvName}: read failed" >> "${LOG_FILE_NAME}"
    local errIO=$?
    if [ $errRead -eq 0 ] || [ $errIO -eq 0 ]; then
        which dmsetup
        if [ $? -ne 0 ]; then
            Log "there are some IO error on ${vgName}/${lvName}, and dmsetup don't exist."
            exit 1
        fi

        mount | grep "$mountPath" >> "${LOG_FILE_NAME}"
        if [ $? -eq 0 ]; then
            umount "$mountPath" >> "${LOG_FILE_NAME}"
            Log "umount path $mountPath when there are some IO error on ${vgName}/${lvName}."
        fi

        dmsetup ls | grep "${vgName}-${lvName}" >> "${LOG_FILE_NAME}"
        dmsetup remove "${vgName}-${lvName}" >> "${LOG_FILE_NAME}"
        vgscan --cache >> "${LOG_FILE_NAME}"
        pvscan --cache >> "${LOG_FILE_NAME}"
    fi
}

# remove vg when unmap lun directly
if [ "`uname`" = "Linux" ]; then
    ClearLVMInfo $VGDATA $LVDATA $PathData
    ClearLVMInfo $VGLOG $LVLOG $PathLog
fi

Log "clear filesystem leftover resource succ."
exit 0
