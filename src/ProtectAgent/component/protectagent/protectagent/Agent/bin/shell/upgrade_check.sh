#!/bin/sh
set +x
AGENT_ROOT_PATH=$1
PID=$2
PARAM_NUM=$3
AGENT_INSTALL_PATH=${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient
DOWNLOAD_DIR=${DATA_BACKUP_AGENT_HOME}/upgrade
SHELL_TYPE_SH="/bin/sh"
BACKUP_ROLE_SANCLIENT_PLUGIN=5
# agent role
SANCLIENT_ROLE=sanclient
AGENT_ROLE=rdadmin

CURRENT_PATH=`dirname $0` && cd $CURRENT_PATH && CURRENT_PATH=`pwd`
TESTCFG_BACK_ROLE=`su - ${SANCLIENT_ROLE} 2>/dev/null -s ${SHELL_TYPE_SH} -c "sed '/^BACKUP_ROLE=/!d;s/.*=//' ${CURRENT_PATH}/../conf/testcfg.tmp "`
if [ "${TESTCFG_BACK_ROLE}" = "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ] ; then
    AGENT_INSTALL_PATH=${DATA_BACKUP_SANCLIENT_HOME}/DataBackup/SanClient
    DOWNLOAD_DIR=${DATA_BACKUP_SANCLIENT_HOME}/upgradeSanclient
fi
AGENT_ROOT_PATH=${AGENT_INSTALL_PATH}/ProtectClient-E
LOG_FILE_NAME=${AGENT_ROOT_PATH}/slog/upgrade_pre.log
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
    # make sure at least 2GB(1024 * 2048) space left
    FREE_SPACE_MIN=2097152
    # total 12 times of package size is required
    old_size=`expr $1 \* 4`
    old_log_size=`expr ${FREE_SPACE_MIN} \* 2`
    backup_size=`expr $old_size + $old_log_size`
    remained_size=`expr $1 \* 2`
    size=`expr $backup_size + $remained_size`
    
    if [ ${size} -gt ${FREE_SPACE_MIN} ]; then
        FREE_SPACE_MIN=${size};
    fi
    CHECK_DIR=${DATA_BACKUP_AGENT_HOME}
    if [ "${TESTCFG_BACK_ROLE}" = "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ] ; then
        CHECK_DIR=${DATA_BACKUP_SANCLIENT_HOME}
    fi
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

CheckIsMultipleAgents()
{
    MULTIPLE_AGENT=
    id -u ${SANCLIENT_ROLE} >/dev/null 2>&1;
    if [ $? -eq 0 ]; then
        MULTIPLE_AGENT=0
    else
        return 1
    fi

    id -u ${AGENT_ROLE} >/dev/null 2>&1
    if [ $? -eq 0 ]; then
        MULTIPLE_AGENT=0
    else
        return 1
    fi
    return ${MULTIPLE_AGENT}
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
    # 存在两种代理时操作系统为linux
    CheckIsMultipleAgents
    if [ $? -eq 0 ]; then
        sanclient_output=`su - ${SANCLIENT_ROLE} -s ${SHELL_TYPE_SH} -c "cat ${CURRENT_HOSTSN_FILE}"`
        rdadmin_output=`su - ${AGENT_ROLE} -s ${SHELL_TYPE_SH} -c "cat ${CURRENT_HOSTSN_FILE}"`
        if [ -n "${sanclient_output}" ]; then
            current_hostsn=${sanclient_output}
        else
            current_hostsn=${rdadmin_output}
        fi
    else
        current_hostsn=`SUExecCmdWithOutput "cat \"${CURRENT_HOSTSN_FILE}\""`
        if [ -z "${current_hostsn}" ]; then
            Log "current_hostsn is NULL."
            return 1;
        fi
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
Log "Upgrade check begin."
PARAM_CONTENT=`ReadInputParam`
test -z "$PARAM_CONTENT"              && ExitWithError "PARAM_FILE"

#for GetValue
PackageSize=`GetValue "${PARAM_CONTENT}" packageSize`

Log "packageSize=${PackageSize}";
test -z "$PackageSize"               && ExitWithError "PackageSize"

CheckHostSN
if [ $? -ne 0 ]; then
    LogError "Failed to check hostsn." ${ERR_UPGRADE_FAIL_CHECK_HOSTSN}
    exit 1
fi

MakeDownloadDir
if [ $? -ne 0 ]; then
    LogError "Failed to make download directory." ${ERR_UPGRADE_FAIL_VERIFICATE_PARAMETER}
    exit 1
fi

CheckFreeDiskSpace ${PackageSize}
if [ $? -ne 0 ]; then
    LogError "Failed to check free disk space." ${ERROR_AGENT_DISK_NOT_ENOUGH}
    exit ${ERROR_AGENT_DISK_NOT_ENOUGH}
fi

Log "Upgrade check successfully."
exit 0
