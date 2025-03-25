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
AGENT_ROOT_PATH=$1
PID=$2
PARAM_NUM=$3
if [ -z "${DATA_BACKUP_AGENT_HOME}" ]; then
    DATA_BACKUP_AGENT_HOME="/opt"
fi
AGENT_INSTALL_PATH=${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient
SANCLIENT_INSTALL_PATH="${DATA_BACKUP_AGENT_HOME}/DataBackup/SanClient"
DOWNLOAD_DIR=${DATA_BACKUP_AGENT_HOME}/modify
SHELL_TYPE_SH="/bin/sh"
AGENT_ROOT_PATH=${AGENT_INSTALL_PATH}/ProtectClient-E
LOG_FILE_NAME=${AGENT_ROOT_PATH}/slog/modify_pre.log
CURRENT_HOSTSN_FILE=/etc/HostSN/HostSN
CONF_BACKUP_HOSTSN_FILE=${AGENT_ROOT_PATH}/conf/HostSN
. "${AGENT_ROOT_PATH}/sbin/agent_sbin_func.sh"

########################################################################################
# Function Definition
LogError()
{
    #$1=errorInfo,$2=errorCode, $3=errorDetailParam
    LogErr "$1" "$2" "job_log_agent_storage_update_prepare_fail_label" "$3"
}

# Check disk space
CheckFreeDiskSpace()
{
    size=`expr $1 \* 4`
    # make sure at least 2GB(1024 * 2048) space left
    FREE_SPACE_MIN=2097152
    if [ ${size} -gt ${FREE_SPACE_MIN} ]; then
        FREE_SPACE_MIN=${size};
    fi
    CHECK_DIR=${DATA_BACKUP_AGENT_HOME}
    sysName=`uname -s`
    if [ "${sysName}" = "Linux" ]; then
        FREE_SPACE=`df -k ${CHECK_DIR} | grep -n '' | ${MYAWK} 'END{print $4}'`
    elif [ "${sysName}" = "AIX" ]; then
        FREE_SPACE=`df -k ${CHECK_DIR} | grep -n '' | ${MYAWK} '{print $3}' | sed -n '2p'`
    elif [ "${sysName}" = "HP-UX" ]; then
        FREE_SPACE=`df -k ${CHECK_DIR} | grep -w 'free' | ${MYAWK} '{print $1}'`
    elif [ "${sysName}" = "SunOS" ]; then
        FREE_SPACE=`df -k ${CHECK_DIR} | ${MYAWK} '{print $4}' | sed -n '2p'`
    fi
    Log "FREE_SPACE_MIN(${FREE_SPACE_MIN}) and FREE_SPACE(${FREE_SPACE})."
    if [ ${FREE_SPACE_MIN} -gt ${FREE_SPACE} ]; then
        Log "No enough space left on the device."
        return 1
    fi
    
    Log "Check free disk space successfully."
    return 0
}

# Make download directory
MakeDownloadDir()
{
    if [ -d "${DOWNLOAD_DIR}" ]; then
        rm -rf "${DOWNLOAD_DIR}"
    fi
    mkdir -p ${DOWNLOAD_DIR}
    chmod 700 ${DOWNLOAD_DIR}

    return 0
}

# check HostSN
CheckHostSN()
{
    if [ ! -f "${CURRENT_HOSTSN_FILE}" ]; then
        Log "Current HostSN(${CURRENT_HOSTSN_FILE}) file is missing."
        return 1;
    fi
    if [ ! -f "${CONF_BACKUP_HOSTSN_FILE}" ]; then
        Log "Backup HostSN(${CONF_BACKUP_HOSTSN_FILE}) file is missing."
        return 1;
    fi
    current_hostsn=`SUExecCmdWithOutput "cat \"${CURRENT_HOSTSN_FILE}\""`
    if [ -z "${current_hostsn}" ]; then
        Log "current_hostsn is NULL."
        return 1;
    fi
    backup_hostsn=`SUExecCmdWithOutput "cat \"${CONF_BACKUP_HOSTSN_FILE}\""`
    if [ -z "${backup_hostsn}" ]; then
        Log "backup_hostsn is NULL."
        return 1;
    fi
    if [ "${backup_hostsn}" != "${current_hostsn}" ]; then
        Log "backup_hostsn(${backup_hostsn}) is not equal current_hostsn(${current_hostsn})."
        return 1;
    fi
    Log "backup_hostsn(${backup_hostsn}) is equal current_hostsn(${current_hostsn})."
    return 0
}
#################################################################################
##  main process
#################################################################################
Log "Modify check begin."
PARAM_CONTENT=`ReadInputParam`
test -z "$PARAM_CONTENT"              && ExitWithError "PARAM_FILE"

#for GetValue
PackageSize=`GetValue "${PARAM_CONTENT}" packageSize`

Log "packageSize=${PackageSize}";
test -z "$PackageSize"               && ExitWithError "PackageSize"

CheckHostSN
if [ $? -ne 0 ]; then
    LogError "Failed to check hostsn." ${ERR_MODIFY_FAIL_CHECK_HOSTSN}
    exit 1
fi

MakeDownloadDir
if [ $? -ne 0 ]; then
    LogError "Failed to make download directory." ${ERR_MODIFY_FAIL_VERIFICATE_PARAMETER}
    exit 1
fi

CheckFreeDiskSpace ${PackageSize}
if [ $? -ne 0 ]; then
    LogError "Failed to check free disk space." ${ERROR_AGENT_DISK_NOT_ENOUGH}
    exit ${ERROR_AGENT_DISK_NOT_ENOUGH}
fi

Log "Modify check successfully."
exit 0
