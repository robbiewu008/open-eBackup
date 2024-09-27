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
# set +x

sysName=`uname -s`
SHELL_TYPE_SH="/bin/sh"
ERROR_DB_FREEZE_YES=0
ERROR_DB_FREEZE_NO=1
ERROR_DB_FREEZE_UNKNOWN=2
#common 5-19
ERROR_SCRIPT_EXEC_FAILED=5
ERROR_RESULT_FILE_NOT_EXIST=6
ERROR_TMP_FILE_IS_NOT_EXIST=7
ERROR_PATH_WRONG=8
ERROR_PARAM_WRONG=9
ERROR_DB_USERPWD_WRONG=10
ERROR_INSTANCE_NOSTART=11
ERROR_PARAM_INVALID=12

ERROR_INSUFFICIENT_WRONG=15
ERROR_NOSUPPORT_DBFILE_ON_BLOCKDEVICE=16
ERROR_DEVICE_FILESYS_MOUNT_FAILED=17
ERROR_DEVICE_FILESYS_UNMOUNT_FAILED=18
ERROR_BACKUP_INVALID=19    # backup data is invalid, .e.g backup additional is invalid
#ORACLE error code 20-69
ERROR_ORACLE_ASM_DBUSERPWD_WRONG=21
ERROR_ORACLE_ASM_INSUFFICIENT_WRONG=22
ERROR_ORACLE_ASM_INSTANCE_NOSTART=23
ERROR_ORACLE_NOARCHIVE_MODE=24
ERROR_ORACLE_OVER_ARCHIVE_USING=25
ERROR_ORACLE_ASM_DISKGROUP_ALREADYMOUNT=26
ERROR_ORACLE_ASM_DISKGROUP_NOTMOUNT=27
ERROR_ORACLE_APPLICATION_OVER_MAX_LINK=28

ERROR_ORACLE_DB_ALREADY_INBACKUP=29
ERROR_ORACLE_DB_INHOT_BACKUP=30
ERROR_ORACLE_DB_ALREADYRUNNING=31
ERROR_ORACLE_DB_ALREADYMOUNT=32
ERROR_ORACLE_DB_ALREADYOPEN=33
ERROR_ORACLE_DB_ARCHIVEERROR=34
ERROR_ORACLE_BEGIN_HOT_BACKUP_FAILED=35
ERROR_ORACLE_END_HOT_BACKUP_FAILED=36
ERROR_ORACLE_BEGIN_HOT_BACKUP_TIMEOUT=37
ERROR_DB_ENDSUCC_SOMETBNOT_INBACKUP=38
ERROR_ASM_NO_STARTUP_TNS=39
ERROR_ORACLE_NOT_MOUNTED=40
ERROR_ORACLE_NOT_OPEN=41
ERROR_ORACLE_TRUNCATE_ARCHIVELOG_FAILED=42
ERROR_ORACLE_TNS_PROTOCOL_ADAPTER=43
#oracle not install
ERROR_ORACLE_NOT_INSTALLED=44
ERROR_ORACLE_ANOTHER_STARTING=45
ERROR_ORACLE_DB_BUSY=46

ERROR_SCRIPT_ORACLE_INST_NOT_CDB=47
ERROR_SCRIPT_ORACLE_PDB_NOT_EXIT=48
ERROR_SCRIPT_START_PDB_FAILED=49
ERROR_DB_FILE_NOT_EXIST=50
ERROR_ORACLE_DB_NOT_COMPLETE_SHUTDOWN=51
ERROR_ORACLE_RESTORE_FAILED=52
ERROR_ORACLE_RESTORE_SUCC_OPEN_FAILED=53
ERROR_ORACLE_GET_OPEN_MODE_FAILED=54
ERROR_ORACLE_GET_OPEN_MODE_NOT_MATCH=55
ERROR_ORACLE_BACKUP_FAILED=56
ERROR_ORACLE_EXPIRE_FAILED=57
ERROR_ORACLE_DISMOUNT_FAILED=58
ERROR_DEL_ARCHIVELOG_FAILED=59

ERROR_ORACLE_EXESQL_FAILED=61
ERROR_ORACLE_EXERMAN_FAILED=62
ERROR_ORACLE_EXEASMCMD_FAILED=63
ERROR_ORACLE_RECOVERPATH_NOT_EXIT=64
ERROR_ORACLE_DISKGROUP_NOT_EXIT=65
ERROR_DISCONNECT_STORAGE_NETWORK=66
ERROR_MOUNTPATH_OCCUPIED=67
ERROR_SCRIPT_ORACLEHOME_LOST=68;
ERROR_SCRIPT_ORACLEBASE_LOST=69;

#DB2 error code 70-99
ERROR_DB2_SUSPEND_IO_FAILED=70
ERROR_DB2_RESUME_IO_FAILED=71
ERROR_DB2_SUSPEND_IO_TIMEOUT=72
#INSTANCE NOT EXITST
ERROR_SCRIPT_INSTANCE_NOT_EXIST=136
#CLUSTER error code 160-189
ERROR_CLUSTER_SERVICE_NOSTART=160
ERROR_CLUSTER_DB_NOT_INCLUSTER=161
ERROR_CLUSTER_RESOURCE_STATUS_ABNORMAL=162

ERROR_CLUSTER_RESOURCE_ONLINE_FAILED=163
ERROR_CLUSTER_RESOURCE_OFFLINE_FAILED=164
ERROR_NOT_ACTIVE_NODE=165

# inner error code, not return to RD server
ERROR_CREATE_VG=170
ERROR_CREATE_LV=171
ERROR_CREATE_FS=172
ERROR_MOUNT_FS=173
ERROR_CREATE_RAW_FAILED=174
ERROR_RESTORE_FAILED=175
ERROR_RESTORE_SUCC_OPEN_FAILED=176
ERROR_GET_OPEN_MODE_FAILED=177
ERROR_GET_OPEN_MODE_NOT_MATCH=178
ERROR_BACKUP_FAILED=179
ERROR_ORACLE_RESTORE_DBSTARTED=180

# oracle
ERROR_ORACLE_DB_NOT_EXIST=181
ERROR_ORACLE_DB_EXIST=182
ERROR_ORACLE_COPYFILES=183

ERROR_MOUNTPATH=185
ERROR_ORACLE_VERSION_DISMATCH=186
ERROR_VG_CREATEBY_OTHER_DISK=187
ERROR_ASM_ASMDGNAME=188

RESULT_TMP_FILE_PREFIX=result_tmp
SCRIPTPID_TMP_FILE_PREFIX=scriptpid_tmp
MAXLOGSIZE=52428800
LOGFILE_SUFFIX="gz"
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${AGENT_ROOT_PATH}/bin && export LD_LIBRARY_PATH
BACKLOGCOUNT=`${AGENT_ROOT_PATH}/bin/xmlcfg read System log_count`

TIME_FORMAT="'YYYY-MM-DD_HH24:MI:SS'"

#global path
BIN_PATH="${AGENT_ROOT_PATH}/bin"
SBIN_PATH="${AGENT_ROOT_PATH}/sbin"
TMP_PATH="${AGENT_ROOT_PATH}/tmp"
LOG_PATH="${AGENT_ROOT_PATH}/log"
CONF_PATH="${AGENT_ROOT_PATH}/conf"
RESULT_FILE="${TMP_PATH}/${RESULT_TMP_FILE_PREFIX}${PID}"
ERRDETAIL_FILE="${TMP_PATH}/result_errordetail${PID}"
WARING_FILE="${TMP_PATH}/result_warninginfo"
SCRIPTPID_FILE="${TMP_PATH}/${SCRIPTPID_TMP_FILE_PREFIX}${PID}"
SCRIPTRST_FILE="${TMP_PATH}/RST${PID}.txt"
LOG_ERR_FILE=${AGENT_ROOT_PATH}/tmp/errormsg.log
SANCLIENT_LOG_ERR_FILE="/opt/DataBackup/SanCliet/ProtectClient-E/tmp/errormsg.log"
SANCLIENT_INSTALL_DIR="/opt/DataBackup/SanClient"
# global var, for kill monitor
MONITOR_FILENAME="${SBIN_PATH}/procmonitor.sh"
SPACE=

#for 
QRY_NOT_BACKUP_TB_SQL="select status from v\$backup where status = 'NOT ACTIVE';"
QRY_BACKUP_TB_SQL="select status from v\$backup where status = 'ACTIVE';"

#password protect
ENS=""
DES=""
PLAIN_PWD=""

#nginx
NGINX_LISTEN_DEFAULT_IP="0.0.0.0"
NGINX_LISTEN_IP=""
NGINX_LISTEN_PORT=""

#driver
DR_DRIVER_NAME=iomirror
BAK_DRIVER_NAME=ebkbackup
DRIVER_OS_VERSION=""

##### upgrade errcode start #####
ERR_UPGRADE_NETWORK_ERROR=1577209877
ERR_UPGRADE_FAIL_SAVE_CONFIGURE=1577209879
ERR_UPGRADE_FAIL_BACKUP=1577209880
ERR_UPGRADE_FAIL_VERIFICATE_PARAMETER=1577209881
ERR_UPGRADE_FAIL_READ_CONFIGURE=1577209882
ERR_UPGRADE_FAIL_GET_SYSTEM_PARAMETERS=1577209883
ERR_UPGRADE_FAIL_START_PROTECT_AGENT=1577209884
ERR_UPGRADE_FAIL_ADD_USER=1577209885
ERR_UPGRADE_FAIL_ADD_USERGROUP=1577209886
ERR_UPGRADE_INSTALL_PACKAGE_INCOMPLETE=1577209876
ERR_UPGRADE_PACKAGE_NO_MATCH_HOST_SYSTEM=1577209878
ERR_UPGRADE_FAIL_STOP_PROTECT_AGENT=1577209887
ERR_UPGRADE_HOST_MISS_TOOL_PACKAGE=1577209888
ERR_UPGRADE_FAIL_REGISTER_TO_PM=1577209889
ERR_UPGRADE_FAIL_VERIFICATE_CERTIFICATE=1577209890
ERR_UPGRADE_FAIL_PORT_OCCUPY=1577209891
#####upgrade errcode end #####

if [ "$sysName" = "SunOS" ]
then
    MYAWK=nawk
else
    MYAWK=awk
fi

CLIENT_BACK_ROLE=""
TESTCFG_BACK_ROLE=""
CLIENT_BACK_ROLE=`cat ${CURRENT_PATH}/conf/client.conf 2>/dev/null | grep "backup_role" | ${MYAWK} -F '=' '{print $NF}'`
TESTCFG_BACK_ROLE=`cat /opt/DataBackup/SanClient/ProtectClient-E/conf/testcfg.tmp 2>/dev/null | grep BACKUP_ROLE | ${MYAWK} -F '=' '{print $NF}'`
if [ "${CLIENT_BACK_ROLE}" = "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ]  || [ "${TESTCFG_BACK_ROLE}" = "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ] ; then
    LOG_ERR_FILE=${SANCLIENT_LOG_ERR_FILE}
fi

# Get the specified value from input argument
##############################################################
#
## @Usage Get the specified value from input argument
#
## @Return If existed sepcified value return 0, else return 1
#
## @Description This function gets specified value from input argument
##############################################################
GetValue()
{
    if [ $# -ne 2 ]
    then
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi
    for item in $1; do
        typeset key=`echo $item | $MYAWK -F "=" '{print $1}'`
        typeset value=`echo $item | $MYAWK -F "=" '{print $2}'`
        if [ $key = $2 ]; then
           echo "$value"
           break
        fi
    done
}

CHMOD()
{
    source=
    arg2=
    arg=
    if [ $# -eq 3 ]; then
        arg=$1
        arg2=$2
        source=$3
    elif [ $# -eq 2 ]; then
        source=$2
        arg2=$1
    fi
    if [ -L "$source" ]; then
        Log "source file  is a link file can not copy."
        return
    fi
    chmod $arg $arg2 "$source"  >>${LOG_FILE_NAME} 2>&1
}

CP()
{
    # High to Low, Determining Linked Files
    eval LAST_PARAM=\$$#
    TARGET_FILE=${LAST_PARAM}
    if [ -L "${TARGET_FILE}" ]; then
        echo "The target file is a linked file. Exit the current process."
        Log "The target file is a linked file. Exit the current process."
        exit 1
    fi

    SYSNAME_JUDGE=`uname -s`
    if [ "$SYSNAME_JUDGE" = "SunOS" ]; then
        cp -R -P $*
    else
        cp -d $*
    fi
}

DeleteFile()
{
    if [ $# -lt 1 ]
    then
        return 0
    fi

    for i in "$@"
    do
        if [ -f "${i}" ]
        then
            rm -fr "${i}"
        fi
    done

    return 0
}

LogInfo()
{
    LogStr=$1
    LogStr="[INFO]$1"
    Log $LogStr
}

LogErr() 
{
    #$1=errorInfo,$2=errorCode,$3=errorLable,$4=errorDetailParam
    Log "[ERR] $1"
    if [ $# -ge 3 ];then
        LogErrDetail "$2" "$3" "$4"
    fi
}
LogErrDetail()
{
    if [ ! -f "${LOG_ERR_FILE}" ]
    then
        touch "${LOG_ERR_FILE}"
        chmod 600 "${LOG_ERR_FILE}"
    fi
    echo "logDetail=$1" > ${LOG_ERR_FILE}
    echo "logInfo=$2" >> ${LOG_ERR_FILE}
    echo "logDetailParam=$3" >> ${LOG_ERR_FILE}

}

Log()
{
    if [ ! -f "${LOG_FILE_NAME}" ]; then
        touch "${LOG_FILE_NAME}"
        chmod 600 "${LOG_FILE_NAME}" 
    fi
    
    DATE=`date +%y-%m-%d--%H:%M:%S`
    if [ "SunOS" = "$sysName" ]; then
        command -v whoami >/dev/null
        if [ $? -eq 0 ]; then
            USER_NAME=`whoami`
        else
            USER_NAME=`/usr/ucb/whoami`
        fi
    else
        USER_NAME=`whoami`
    fi
    
    echo "[${DATE}][$$][${USER_NAME}] $1" >> "${LOG_FILE_NAME}"
    
    BACKLOGNAME="${LOG_FILE_NAME}.${BACKLOGCOUNT}.${LOGFILE_SUFFIX}"
    NUMBER=`expr ${BACKLOGCOUNT} - 1` 
    LOGFILESIZE=`ls -l "${LOG_FILE_NAME}" | $MYAWK -F " " '{print $5}'`
    if [ ${LOGFILESIZE} -gt ${MAXLOGSIZE} ]
    then
        if [ -f "${BACKLOGNAME}" ]
        then
            rm -f "${BACKLOGNAME}"
        fi
        
        while [ $NUMBER -ge 0 ]
        do
            if [ $NUMBER -eq 0 ]
            then
                gzip -f -q -9 "${LOG_FILE_NAME}"
                BACKLOGNAME="${LOG_FILE_NAME}.${LOGFILE_SUFFIX}"
            else
                BACKLOGNAME="${LOG_FILE_NAME}.${NUMBER}.${LOGFILE_SUFFIX}"                 
            fi
            
            if [  -f "${BACKLOGNAME}" ]
            then
                DestNum=`expr $NUMBER + 1`
                mv -f "${BACKLOGNAME}" "${LOG_FILE_NAME}.${DestNum}.${LOGFILE_SUFFIX}" 
                chmod 440 "${LOG_FILE_NAME}.${DestNum}.${LOGFILE_SUFFIX}" 
            fi
            
            NUMBER=`expr $NUMBER - 1`
        done
    fi

    if [ ! -f "${LOG_FILE_NAME}" ]
    then
        touch "${LOG_FILE_NAME}" 
        chmod 600 "${LOG_FILE_NAME}"
    fi
}

KillProcMonitor()
{
    MYPPID=$1
    MONITORNAME=procmonitor.sh
    MONITORPIDs=`ps -ef | grep ${MONITORNAME} | grep -v "grep" | $MYAWK -v ppid="${MYPPID}" '{if ($3==ppid) print $2}'`
    
    for MONITORPID in ${MONITORPIDs}
    do
        kill -9 ${MONITORPID}
        Log "kill procMonitor id=${MONITORPID}."
    done

    return 0
}

PrintPrompt()
{
    if [ "${sysName}" = "Linux" ]; then
        echo -n ">> "
    else
        echo ">> \c"
    fi
}

#function:check if the linux is redhat7
#param: no
#return: if is redhat7 then return 1, else return 0;
IsRedhat7()
{
    #is not redhat
    if [ ! -f /etc/redhat-release ]; then
        return 0
    fi

    #redhat
    LINE=`cat /etc/redhat-release`
    REDHAT_VERSION=`echo $LINE|${MYAWK} -F '.' '{print $1}'|${MYAWK} '{print $NF}'`
    if [ "$REDHAT_VERSION" = "7" ]
    then
        return 1
    fi
    return 0
}

#function: get OS version
#param: None
#return: echo version, TYPER_VER, REDHAT_6.5 ORACLELINUX_7.5 CENTOS_7.2.1511
getOSVersion()
{
    if [ ! -f "/etc/redhat-release" ]; then
        Log "OS is not support."
        return
    fi

    if [ -f "/etc/oracle-release" ]; then
        VERSION=`cat /etc/oracle-release 2>/dev/null | ${MYAWK} '{print $NF}'`
        DRIVER_OS_VERSION=${VERSION}
        echo "ORACLELINUX_${VERSION}"
        return
    fi

    cat /etc/redhat-release | grep "CentOS Linux release" >> /dev/null
    if [ $? -eq 0 ]; then
        VERSION=` cat /etc/redhat-release 2>/dev/null | ${MYAWK} -F '(' '{print $1}' | ${MYAWK} '{print $NF}'`
        DRIVER_OS_VERSION=${VERSION}
        echo "CENTOS_${VERSION}"
        return
    fi

    cat /etc/redhat-release | grep "Red Hat Enterprise Linux Server release" >> /dev/null
    if [ $? -eq 0 ]; then
        VERSION=`cat /etc/redhat-release 2>/dev/null | ${MYAWK} -F '(' '{print $1}' | ${MYAWK} '{print $NF}'`
        DRIVER_OS_VERSION=${VERSION}
        echo "REDHAT_${VERSION}"
        return
    fi

    Log "UnSupoort OS `cat /etc/redhat-release`"
}

CheckIsIpv6()
{
    typeset ipAddr=$1
    chkResult=`expr index "${ipAddr}" ":"`
    if [ $chkResult -ne 0 ];then
        Log "This is a ipv6:${ipAddr}"
        echo 0
    fi
    echo 1
}

# echo err message in red color
ShowError()
{
    printf "\\033[1;31m$1\\033[0m\n"
}

# echo info message in green color
ShowInfo()
{
    printf "\\033[1;32m$1\\033[0m\n"
}

# echo warning message in purple color
ShowWarning()
{
    printf "\\033[1;35m$1\\033[0m\n"
}