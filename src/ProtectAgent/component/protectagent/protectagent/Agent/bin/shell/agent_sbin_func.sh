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
SYS_NAME=`uname -s`
AGENT_ROOT_PATH=${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient/ProtectClient-E
SANCLIENT_ROOT_PATH=${DATA_BACKUP_SANCLIENT_HOME}/DataBackup/SanClient/ProtectClient-E
if [ "${UPGRADE_FLAG_TEMPORARY}" = "1" ]; then
    CURRENT_PATH=`pwd`
else
    CURRENT_PATH=`dirname ${BASH_SOURCE}` && cd $CURRENT_PATH && CURRENT_PATH=`pwd`
fi
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
ERROR_CHECK_ARCHIVELOG_FAILED=60

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
ERROR_START_RACDB=189
ERROR_UMOUNT_FS=190
ERROR_CHECK_MOUNT_FS=191
ERROR_MOUNTED_OTHER_FS=192

ERROR_CREATE_LV=193
ERROR_CREATE_PV=194
ERROR_CREATE_VG=195
ERROR_SCAN_ADAPTER=196
ERROR_SCAN_DISK=197
ERROR_VARYONVG=198
ERROR_MOUNT_FAILED=199
ERROR_MOUNTPATH_BLOCk=200
ERROR_POINT_MOUNTED=201

#rdaget user
AGENT_USER=rdadmin
AGENT_GROUP=rdadmin
SANCLIENT_USER=sanclient
SANCLIENT_GROUP=sanclient
BACKUP_ROLE_SANCLIENT_PLUGIN=5

#sanclient
ERROR_TARGETCLI_STATUS=220
ERROR_QLA2XXX_STAUTS=222

ERROR_AGENT_DISK_NOT_ENOUGH=230

RESULT_TMP_FILE_PREFIX=result_tmp
SCRIPTPID_TMP_FILE_PREFIX=scriptpid_tmp

if [ "$SYS_NAME" = "SunOS" ]; then
    MYAWK=nawk
    MYGREP=/usr/xpg4/bin/grep
else
    MYAWK=awk
    MYGREP=grep
fi

TESTCFG_BACK_ROLE=`su - ${SANCLIENT_USER} 2>/dev/null -s ${SHELL_TYPE_SH} -c "sed '/^BACKUP_ROLE=/!d;s/.*=//' ${CURRENT_PATH}/../conf/testcfg.tmp "`
if [ "${TESTCFG_BACK_ROLE}" = "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ]  || [ "${CLIENT_BACK_ROLE}" = "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ]; then
    AGENT_GROUP=${SANCLIENT_GROUP}
    AGENT_USER=${SANCLIENT_USER}
    AGENT_ROOT_PATH=${DATA_BACKUP_SANCLIENT_HOME}/DataBackup/SanClient/ProtectClient-E
fi
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${AGENT_ROOT_PATH}/bin && export LD_LIBRARY_PATH
MAXLOGSIZE=52428800
LOGFILE_SUFFIX="gz"

TIME_FORMAT="'YYYY-MM-DD_HH24:MI:SS'"
#global path
BIN_PATH="${AGENT_ROOT_PATH}/bin"
SBIN_PATH="${AGENT_ROOT_PATH}/sbin"
TMP_PATH="${AGENT_ROOT_PATH}/tmp"
STMP_PATH="${AGENT_ROOT_PATH}/stmp"
LOG_PATH="${AGENT_ROOT_PATH}/slog"
CONF_PATH="${AGENT_ROOT_PATH}/conf"
RESULT_FILE="${STMP_PATH}/${RESULT_TMP_FILE_PREFIX}${PID}"
ERRDETAIL_FILE="${STMP_PATH}/result_errordetail${PID}"
WARING_FILE="${STMP_PATH}/result_warninginfo"
SCRIPTPID_FILE="${STMP_PATH}/${SCRIPTPID_TMP_FILE_PREFIX}${PID}"
SCRIPTRST_FILE="${STMP_PATH}/RST${PID}.txt"
AGENT_SHELL_PATH=${INSTALL_DIR}
LOG_ERR_FILE=${AGENT_ROOT_PATH}/stmp/errormsg.log
ERRCODE_FILE=${AGENT_ROOT_PATH}/slog/install.errcode
DATATURBO_INSTALL_BY_AGENT=/opt/oceanstor/dataturbo/.INSTALL_BY_AGENT
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
ERR_UPGRADE_FAIL_CHECK_HOSTSN=1577210037
#####upgrade errcode end #####

ASM_PASSWORD="ASMUSER_PASSWORD${PID}"
DB_PASSWORD="DBUSER_PASSWORD${PID}"
DB_ENCKEY="DB_ENCKEY${PID}"
WAIT_SEC=60

AUTO_START_TYPE=0               # 0, unsurpport; 1, rpm; 2, dpkg;

if [ "$SYS_NAME" = "SunOS" ]; then
    MYAWK=nawk
    MYGREP=/usr/xpg4/bin/grep
else
    MYAWK=awk
    MYGREP=grep
fi

if [ "`ls -al /bin/sh | ${MYAWK} '{print $NF}'`" = "bash" ]; then
    EXPORT_ENV="if [ -f ~/.profile ]; then . ~/.profile; fi;"
else
    EXPORT_ENV=""
fi

TESTCFG_BACK_ROLE=`su - ${SANCLIENT_USER} 2>/dev/null -s ${SHELL_TYPE_SH} -c "sed '/^BACKUP_ROLE=/!d;s/.*=//' ${CONF_PATH}/../conf/testcfg.tmp "`
if [ "${TESTCFG_BACK_ROLE}" = "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ]  || [ "${CLIENT_BACK_ROLE}" = "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ]; then
    AGENT_GROUP=${SANCLIENT_GROUP}
    AGENT_USER=${SANCLIENT_USER}
    AGENT_SHELL_PATH=${SANCLIET_AGENT_SHELL_PATH}
fi

AGENT_HOME_DIR="/home/${AGENT_USER}"
if [ "${SYS_NAME}" = "SunOS" ]; then
    AGENT_HOME_DIR="/export/home/${AGENT_USER}"
fi

AGENT_HOME_ENV_FILE="${AGENT_HOME_DIR}/.profile"

if [ "${SYS_NAME}" = "SunOS" ] || [ "${SYS_NAME}" = "AIX" ]; then
    BACKLOGCOUNT=`su - ${AGENT_USER} -c "${EXPORT_ENV}${AGENT_ROOT_PATH}/sbin/xmlcfg read System log_count"`
else
    BACKLOGCOUNT=`su -m ${AGENT_USER} -s ${SHELL_TYPE_SH} -c "${EXPORT_ENV}${AGENT_ROOT_PATH}/sbin/xmlcfg read System log_count"`
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

CheckIpLegality()
{
    if [ $# -ne 1 ]; then
        echo 0
    fi

    OLDIFS="$IFS"

    if [ ${SYS_NAME} != "AIX" ]; then
        IFS=$' '
    fi

    VERIFY_IP_IPV4="^[0-9]\{1,3\}\.\([0-9]\{1,3\}\.\)\{2\}[0-9]\{1,3\}$"
    VERIFY_IP_IPV6="^\s*((([0-9A-Fa-f]{1,4}:){7}([0-9A-Fa-f]{1,4}|:))|(([0-9A-Fa-f]{1,4}:){6}(:[0-9A-Fa-f]{1,4}|((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3})|:))|(([0-9A-Fa-f]{1,4}:){5}(((:[0-9A-Fa-f]{1,4}){1,2})|:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3})|:))|(([0-9A-Fa-f]{1,4}:){4}(((:[0-9A-Fa-f]{1,4}){1,3})|((:[0-9A-Fa-f]{1,4})?:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){3}(((:[0-9A-Fa-f]{1,4}){1,4})|((:[0-9A-Fa-f]{1,4}){0,2}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){2}(((:[0-9A-Fa-f]{1,4}){1,5})|((:[0-9A-Fa-f]{1,4}){0,3}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){1}(((:[0-9A-Fa-f]{1,4}){1,6})|((:[0-9A-Fa-f]{1,4}){0,4}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(:(((:[0-9A-Fa-f]{1,4}){1,7})|((:[0-9A-Fa-f]{1,4}){0,5}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:)))(%.+)?\s*$"

    IP=`echo $1 | $MYAWK '{ gsub(/;/," "); print $0 }'`
    for ipItem in $IP; do
        chkResult=`CheckIsIpv6 ${ipItem}`
        if [ $chkResult -eq 0 ]; then
            echo "$ipItem" | ${MYGREP} -E ${VERIFY_IP_IPV6} 1>/dev/null 2>&1
        else
            echo "$ipItem" | ${MYGREP} "${VERIFY_IP_IPV4}" 1>/dev/null 2>&1
        fi

        if [ $? -ne 0 ]; then
            echo 0
            IFS="$OLDIFS"
            return
        fi
    done
    echo 1
    IFS="$OLDIFS"
}

ReadInputParam()
{
    n=0
    while [ ${n} -lt ${PARAM_NUM} ];do
        let n+=1
        if [ "${SYS_NAME}" = "AIX" ] || [ "${SYS_NAME}" = "SunOS" ]; then
            if read -s PARAM; then
                PARAM_CONTENT="${PARAM_CONTENT} ${PARAM}"
            else
                Log "The actual value is less than ${PARAM_NUM}."
                break
            fi
        else
            if read -t 5 -s PARAM; then
                PARAM_CONTENT="${PARAM_CONTENT} ${PARAM}"
            else
                Log "The actual value is less than ${PARAM_NUM}."
                break
            fi
        fi
    done
    echo "${PARAM_CONTENT}"
}

GetValue()
{
    if [ $# -ne 2 ]
    then
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi
    SPECIAL_CHARS="[\`$;|&<>\!]"
    for item in $1; do
        key=`echo $item | $MYAWK -F "=" '{print $1}'`
        value=`echo $item | $MYAWK -F "=" '{print $2}'`
        if [ $key = $2 ]; then
            if [ "$2" != "Password" ] && [ "$2" != "ASMPassword" ]; then
                if [ "$2" = "storageIp" ] || [ "$2" = "dataOwnerIps" ] || [ "$2" = "dataOtherIps" ] || [ "$2" = "logOwnerIps" ] || [ "$2" = "logOtherIps" ]; then
                    chkResult=`CheckIpLegality ${value}`
                    if [ $chkResult -eq 0 ]; then
                        echo ${ERROR_PARAM_INVALID}
                        break
                    fi
                elif [ "$2" = "DataPath" ] || [ "$2" = "LogPath" ] || [ "$2" = "MetaDataPath" ]; then
                    tmpValue=`echo $value | $MYAWK '{ gsub(/;/," "); print $0 }'`
                    echo "$tmpValue" | grep ${SPECIAL_CHARS} 1>/dev/null 2>&1
                    if [ $? -eq 0 ]; then
                        echo ${ERROR_PARAM_INVALID}
                        break
                    fi
                else
                    echo "$value" | grep ${SPECIAL_CHARS} 1>/dev/null 2>&1
                    if [ $? -eq 0 ]; then
                        echo ${ERROR_PARAM_INVALID}
                        break
                    fi
                fi
            fi
            echo "$value"
            break
        fi
    done
}

GetMountParamValue()
{
    if [ $# -ne 2 ]; then
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi
    SPECIAL_CHARS="[\`$;|&<>\!]"
    for item in $1; do
        key=`echo $item | $MYAWK -F ":" '{print $1}'`
        value=`echo $item | $MYAWK -F ":" '{print $2}'`
        if [ $key = $2 ]; then
            #Verify input parameters
            echo "$value" | grep ${SPECIAL_CHARS} 1>/dev/null 2>&1
            if [ $? -eq 0 ]; then
                echo ${ERROR_PARAM_INVALID}
                break
            fi
            echo "$value"
            break
        fi
    done
}

GetFileOrDirOwner()
{
    if [ ! -f "$1" ] && [ ! -d "$1" ]; then
        echo ""
    fi
    owner=`ls -l $1 | $MYAWK '{print $3}'`
    echo $owner
}

DeleteFile()
{
    if [ $# -lt 1 ]; then
        return 0
    fi

    for i in "$@"; do
        if [ -f "${i}" ]; then
            rm -fr "${i}"
        fi
    done

    return 0
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
        Log "source file  is a link file can not chmod."
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
    if [ $# -lt 2 ]; then 
        echo "The copy paramter's num is incorrect."
        Log "The copy paramter's num is incorrect."
        exit 1
    fi

    if [ "$SYSNAME_JUDGE" = "SunOS" ]; then
        cmd="cp -R -P $*"
        $cmd
    else
        cmd="cp -d $*"
        $cmd
    fi
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
        chmod 640 "${LOG_ERR_FILE}"
        chown root:${AGENT_GROUP} "${LOG_ERR_FILE}"
    fi
    echo "logDetail=$1" > ${LOG_ERR_FILE}
    echo "logInfo=$2" >> ${LOG_ERR_FILE}
    echo "logDetailParam=$3" >> ${LOG_ERR_FILE}
}

Log()
{
    if [ -z "${LOG_FILE_NAME}" ]; then
        return
    fi

    if [ ! -f "${LOG_FILE_NAME}" ]; then
        touch "${LOG_FILE_NAME}"
        CHMOD 600 "${LOG_FILE_NAME}" 
    fi
    
    DATE=`date +%y-%m-%d--%H:%M:%S`
    if [ "SunOS" = "$SYS_NAME" ]; then
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
    if [ ${LOGFILESIZE} -gt ${MAXLOGSIZE} ]; then
        if [ -f "${BACKLOGNAME}" ]; then
            rm -f "${BACKLOGNAME}"
        fi

        while [ $NUMBER -ge 0 ]; do
            if [ $NUMBER -eq 0 ]; then
                gzip -f -q -9 "${LOG_FILE_NAME}"
                BACKLOGNAME="${LOG_FILE_NAME}.${LOGFILE_SUFFIX}"
            else
                BACKLOGNAME="${LOG_FILE_NAME}.${NUMBER}.${LOGFILE_SUFFIX}"                 
            fi

            if [ -f "${BACKLOGNAME}" ]; then
                DestNum=`expr $NUMBER + 1`
                mv -f "${BACKLOGNAME}" "${LOG_FILE_NAME}.${DestNum}.${LOGFILE_SUFFIX}" 
                CHMOD 440 "${LOG_FILE_NAME}.${DestNum}.${LOGFILE_SUFFIX}" 
            fi

            NUMBER=`expr $NUMBER - 1`
        done
    fi

    if [ ! -f "${LOG_FILE_NAME}" ]; then
        touch "${LOG_FILE_NAME}" 
        CHMOD 600 "${LOG_FILE_NAME}"
    fi
}

VerifySpecialChar()
{
    SPECIAL_CHARS="[\`$;|&<>\!]"
    for arg in $*
    do
        echo "$arg" | grep ${SPECIAL_CHARS} 1>/dev/null 2>&1
        if [ $? -eq 0 ]; then
            Log "The variable[$arg] cannot contain special characters."
            exit 1
        fi
    done
}

SUExecCmd()
{
    cmd=$1
    if [ "${SYS_NAME}" = "SunOS" ] || [ "${SYS_NAME}" = "AIX" ]; then
        su - ${AGENT_USER} -c "${cmd}" >>${LOG_FILE_NAME} 2>&1
    else
        su - ${AGENT_USER} -s ${SHELL_TYPE_SH} -c "${EXPORT_ENV}${cmd}" >>${LOG_FILE_NAME} 2>&1
    fi
    ret=$?
    return $ret
}

SUExecCmdWithOutput()
{
    cmd=$1
    if [ "${SYS_NAME}" = "SunOS" ] || [ "${SYS_NAME}" = "AIX" ]; then
        output=`su - ${AGENT_USER} -c "${EXPORT_ENV}${cmd}"`
    else
        output=`su -m ${AGENT_USER} -s ${SHELL_TYPE_SH} -c "${EXPORT_ENV}${cmd}"`
    fi
    ret=$?
    VerifySpecialChar "$output"
    echo $output
    return $ret
}

JudgeAutoStartType()
{
    AUTO_START_TYPE=0
    which rpm
    if [ $? = 0 ];then
        AUTO_START_TYPE=1
        Log "Judge auto start type like rpm."
    fi
    which dpkg >/dev/null 2>&1
    if [ $? = 0 ];then
        AUTO_START_TYPE=2
        Log "Judge auto start type like dpkg."
    fi

    if [ -f /etc/redhat-release ] || [ -f /etc/centos-release ] || [ -f /etc/bclinux-release ] || [ -f /etc/kylin-release ] || [ -f /etc/openEuler-release ]; then
        AUTO_START_TYPE=1
        Log "Judge auto start type is common type."
    fi

    if [ -f "/etc/debian_version" ] || [ -n "`cat /etc/issue | grep 'Linux'`" ]; then
        AUTO_START_TYPE=2
        Log "Judge auto start type is debian or /etc/issue have Linux."
    fi
}


AutoStartUseService()
{
    systemd_file=/usr/lib/systemd/system/rdagent.service
    echo "[Unit]" > ${systemd_file}
    echo "Description=agent auto start" >> ${systemd_file}
    echo "Wants=network-online.target" >> ${systemd_file}
    echo "[Service]" >> ${systemd_file}
    echo "User=root" >> ${systemd_file}
    echo "Type=forking" >> ${systemd_file}
    echo "ExecStart= ${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient/start.sh" >> ${systemd_file}
    echo "[Install]" >> ${systemd_file}
    echo "WantedBy=multi-user.target" >> ${systemd_file}
    systemctl daemon-reload
    systemctl enable rdagent
}

SetAutoStartSystemctl()
{
    if [ -f "/etc/redhat-release" ]; then
        str_version=`cat /etc/redhat-release 2>/dev/null | ${MYAWK} -F '(' '{print $1}' | ${MYAWK} '{print $NF}'`
        if [ "`RDsubstr ${str_version} 1 1`" = "8" ] || [ "`RDsubstr ${str_version} 1 1`" = "9" ]; then
            # redhat 8
            systemd_file=/usr/lib/systemd/system/rdagent.service
            echo "[Unit]" > ${systemd_file}
            echo "Description=agent auto start" >> ${systemd_file}
            echo "Wants=network-online.target" >> ${systemd_file}
            echo "[Service]" >> ${systemd_file}
            echo "User=root" >> ${systemd_file}
            echo "Type=forking" >> ${systemd_file}
            str_cmd="su - ${AGENT_USER} -s ${SHELL_TYPE_SH} ${UNIX_CMD} -c \\\"${AGENT_ROOT_PATH}/bin/monitor &\\\" > /dev/null"
            echo "ExecStart=/bin/sh -c \"ps -u ${AGENT_USER} | grep -v grep | grep monitor > /dev/null; [ \$? -ne 0 ] && ${str_cmd}\"" >> ${systemd_file}
            echo "[Install]" >> ${systemd_file}
            echo "WantedBy=multi-user.target" >> ${systemd_file}
            systemctl daemon-reload
            systemctl enable rdagent
            return
        fi
    fi

    if [ -f "/etc/os-release" ]; then
        cat /etc/os-release | grep "SUSE Linux Enterprise" | grep 15
        if [ $? = 0 ]; then
            AutoStartUseService
            return
        fi
    fi

    if [ -f "/etc/os-release" ]; then
        cat /etc/os-release | grep "EulerOS 2.0" | grep -E "SP10|SP8"
        if [ $? = 0 ]; then
            AutoStartUseService
            return
        fi
    fi

    if [ -f /etc/SuSE-release ]; then
        if [ ! -f /etc/rc.d/after.local ]; then
            touch /etc/rc.d/after.local
            echo '#!/bin/sh' >> /etc/rc.d/after.local
        fi

        CHMOD 755 /etc/rc.d/after.local
        
        RM_IMCTL_LINENO=""
        RM_IMCTL_LINENO=`cat /etc/rc.d/after.local | grep -n -w "cat /proc/devices | grep im_ctldev" | head -n 1 | $MYAWK -F ':' '{print $1}'`
        IOMIRROR_EXIST=0
        lsinitrd /boot/initrd-`uname -r` | grep iomirror >>${LOG_FILE_NAME} 2>&1
        IOMIRROR_EXIST=$?
        if [ "${RM_IMCTL_LINENO}" = "" ] && [ ${IOMIRROR_EXIST} -eq 0 ]; then
            echo "mknod /dev/im_ctldev c \`cat /proc/devices | grep im_ctldev | cut -f1 -d' '\` 0" >> /etc/rc.d/after.local
        fi

        RM_LINENO=`cat /etc/rc.d/after.local | grep -n -w "su - ${AGENT_USER} -s ${SHELL_TYPE_SH} ${UNIX_CMD} -c" | grep -w "${AGENT_ROOT_PATH}/bin/monitor"`
        if [ "${RM_LINENO}" = "" ]; then
            echo "su - ${AGENT_USER} -s ${SHELL_TYPE_SH} ${UNIX_CMD} -c \"${AGENT_ROOT_PATH}/bin/monitor &\"  > /dev/null"  >> /etc/rc.d/after.local
        fi
        systemctl status rc-local | grep inactive >/dev/null 2>&1
        if [ $? -eq 0 ]; then 
            echo "The system service (rc-local.service) is disabled, which may affect the automatic startup function."
            Log "The system service (rc-local.service) is disabled, which may affect the automatic startup function."
        fi
        return
    elif [ -f /etc/euleros-release ]; then
        if [ ! -f /etc/rc.local ]; then
            touch /etc/rc.local
            echo '#!/bin/sh' >> /etc/rc.local
        fi

        CHMOD 755 /etc/rc.local

        LINKSTATUS=`ls -l /etc/rc.local | grep /etc/rc.d/rc.local` 
        RM_LINENO=`cat /etc/rc.local | grep ".${AGENT_SHELL_PATH}/start.sh &"`
        RM_LINENOE=`cat /etc/rc.local | grep "exit 0"`
        if [ "${RM_LINENO}" = "" ] && [ "${RM_LINENOE}" = "" ]; then
            echo ". ${AGENT_SHELL_PATH}/start.sh &" >> /etc/rc.local
        elif [ "${RM_LINENO}" = "" ] && [ "${RM_LINENOE}" != "" ]; then
            sed -i '/exit 0/i\./opt/DataBackup/ProtectClient/start.sh &' /etc/rc.d/rc.local
        fi
        if [ "${LINKSTATUS}" = "" ]; then
            ln -sf /etc/rc.d/rc.local /etc/rc.local
            systemctl status rc-local | grep inactive >/dev/null 2>&1
            if [ $? -eq 0 ]; then 
                echo "The system service (rc-local.service) is disabled, which may affect the automatic startup function."
                Log "The system service (rc-local.service) is disabled, which may affect the automatic startup function."
            fi
        fi
        return
    fi

    JudgeAutoStartType
    if [ "${AUTO_START_TYPE}" = "1" ]; then
        # redhat & centos & bclinux & kylin & NeoKylin & openEuler
        if [ ! -f /etc/rc.d/rc.local ]; then
            touch /etc/rc.d/rc.local
            echo '#!/bin/sh' >> /etc/rc.d/rc.local
        fi
        
        CHMOD 755 /etc/rc.d/rc.local
        RM_IMCTL_LINENO=""
        RM_IMCTL_LINENO=`cat /etc/rc.d/rc.local | grep -n -w "cat /proc/devices | grep im_ctldev" | head -n 1 | $MYAWK -F ':' '{print $1}'`
        IOMIRROR_EXIST=0
        lsinitrd /boot/initramfs-`uname -r`.img | grep iomirror >>${LOG_FILE_NAME} 2>&1
        IOMIRROR_EXIST=$?
        if [ "${RM_IMCTL_LINENO}" = "" ] && [ ${IOMIRROR_EXIST} -eq 0 ]; then
            echo "mknod /dev/im_ctldev c \`cat /proc/devices | grep im_ctldev | cut -f1 -d' '\` 0" >> /etc/rc.d/rc.local
        fi

        RM_LINENO=`cat /etc/rc.d/rc.local | grep -n -w "su - ${AGENT_USER} -s ${SHELL_TYPE_SH} ${UNIX_CMD} -c" | grep -w "${AGENT_ROOT_PATH}/bin/monitor"`
        if [ "${RM_LINENO}" = "" ]; then
            echo "su - ${AGENT_USER} -s ${SHELL_TYPE_SH} ${UNIX_CMD} -c \"${AGENT_ROOT_PATH}/bin/monitor &\"  > /dev/null"  >> /etc/rc.d/rc.local
        fi
        systemctl status rc-local | grep inactive >/dev/null 2>&1
        if [ $? -eq 0 ]; then 
            echo "The system service (rc-local.service) is disabled, which may affect the automatic startup function."
            Log "The system service (rc-local.service) is disabled, which may affect the automatic startup function."
        fi
    elif [ "${AUTO_START_TYPE}" = "2" ]; then
        #ubuntu & Astra Linux & debian & ROCKY
        if [ ! -f /etc/rc.local ]; then
            touch /etc/rc.local
            echo '#!/bin/sh' >> /etc/rc.local
        fi
        CHMOD 755 /etc/rc.local

        cat /etc/rc.local | head -1 | grep "^#!" | grep "/bin/sh" 1>/dev/null 2>&1
        if [ $? -ne 0 ]; then
            if [ -s /etc/rc.local ]; then
                sed -i '1i #!/bin/sh' /etc/rc.local
            else
                echo '#!/bin/sh' >> /etc/rc.local
            fi
        fi
        sed -i '/exit 0/d' /etc/rc.local

        RM_LINENO=`cat /etc/rc.local | grep -n -w "su - ${AGENT_USER} -s ${SHELL_TYPE_SH} ${UNIX_CMD} -c" | grep -w "${AGENT_ROOT_PATH}/bin/monitor"`
        if [ "${RM_LINENO}" = "" ]; then
            echo "su - ${AGENT_USER} -s ${SHELL_TYPE_SH} ${UNIX_CMD} -c \"${AGENT_ROOT_PATH}/bin/monitor &\"  > /dev/null"  >> /etc/rc.local
        fi
        systemctl status rc-local >/dev/null 2>&1
        if [ $? -ne 0 ]; then
            echo "The system service (rc-local.service) is disabled, which may affect the automatic startup function."
            Log "The system service (rc-local.service) is disabled, which may affect the automatic startup function."
        fi
    else
        Log "Unsupport OS."
    fi
}

SetAutoStartInitd()
{
    start_script=/etc/init.d/agentstart
    echo "#! /bin/sh" > ${start_script}
    echo "ps -u ${AGENT_USER} | grep -v grep | grep monitor" >> ${start_script}
    echo "if [ \$? -ne 0 ];then" >> ${start_script}
    echo "    su - ${AGENT_USER} -s ${SHELL_TYPE_SH} ${UNIX_CMD} -c \"${AGENT_ROOT_PATH}/bin/monitor &\"  > /dev/null"  >> ${start_script}
    echo "fi" >> ${start_script}
    CHMOD 755 ${start_script}

    if [ -d /etc/rc1.d ]; then
        rm -rf /etc/rc1.d/S99agentstart
        rm -rf /etc/rc2.d/S99agentstart
        rm -rf /etc/rc3.d/S99agentstart
        rm -rf /etc/rc4.d/S99agentstart
        rm -rf /etc/rc5.d/S99agentstart
        rm -rf /etc/rc6.d/S99agentstart

        ln -s ${start_script} /etc/rc1.d/S99agentstart
        ln -s ${start_script} /etc/rc2.d/S99agentstart
        ln -s ${start_script} /etc/rc3.d/S99agentstart
        ln -s ${start_script} /etc/rc4.d/S99agentstart
        ln -s ${start_script} /etc/rc5.d/S99agentstart
        ln -s ${start_script} /etc/rc6.d/S99agentstart
    else
        rm -rf /etc/init.d/rc1.d/S99agentstart
        rm -rf /etc/init.d/rc2.d/S99agentstart
        rm -rf /etc/init.d/rc3.d/S99agentstart
        rm -rf /etc/init.d/rc4.d/S99agentstart
        rm -rf /etc/init.d/rc5.d/S99agentstart
        rm -rf /etc/init.d/rc6.d/S99agentstart

        ln -s ${start_script} /etc/init.d/rc1.d/S99agentstart
        ln -s ${start_script} /etc/init.d/rc2.d/S99agentstart
        ln -s ${start_script} /etc/init.d/rc3.d/S99agentstart
        ln -s ${start_script} /etc/init.d/rc4.d/S99agentstart
        ln -s ${start_script} /etc/init.d/rc5.d/S99agentstart
        ln -s ${start_script} /etc/init.d/rc6.d/S99agentstart
    fi
}

SetAutoStart()
{
    if [ ${BACKUP_SCENE} -eq ${BACKUP_SCENE_INTERNAL} ]; then
        Log "Internal install."
        return
    fi

    if [ "Linux" = "$SYS_NAME" ]; then
        which systemctl 1>/dev/null 2>&1
        if [ $? -eq 0 ]; then
            SetAutoStartSystemctl
        else
            SetAutoStartInitd
        fi
    elif [ "AIX" = "$SYS_NAME" ]; then
        if [ -f /etc/rc_rdagent.local ]
        then
            rm /etc/rc_rdagent.local
        fi
        
        touch /etc/rc_rdagent.local
        CHMOD 755 /etc/rc_rdagent.local
        
        echo "su - ${AGENT_USER} -c \"${AGENT_ROOT_PATH}/bin/monitor &\" > /dev/null">>/etc/rc_rdagent.local
        
        if [ -f /etc/inittab ]
        then
            lsitab -a | grep /etc/rc_rdagent.local >/dev/null 2>&1
            if [ $? -ne 0 ]
            then
                mkitab  rdagent:2:once:/etc/rc_rdagent.local
                init q
            fi
        fi
    elif [ "HP-UX" = "$SYS_NAME" ]; then
        START_SCRIPT=/sbin/init.d/AgentStart
        AGENT_CONF=/etc/rc.config.d/Agentconf
        SCRIPT=/sbin/rc3.d/S999AgentStart
        
        rm -rf "${SCRIPT}"
        rm -rf "${START_SCRIPT}"
        rm -rf "${AGENT_CONF}"
        
        echo "#!/sbin/sh" >       $START_SCRIPT
        echo "AGENT_ROOT=${AGENT_ROOT_PATH}"  >> $START_SCRIPT
        echo "AGENT_USER=rdadmin" >> $START_SCRIPT
        echo "AGENT_CONF=/etc/rc.config.d/Agentconf"    >> $START_SCRIPT
        echo "PATH=/bin:$PATH" >> $START_SCRIPT
        echo "export PATH"     >> $START_SCRIPT
        echo "if [ -f \$AGENT_CONF ];then"   >> $START_SCRIPT
        echo "    . \$AGENT_CONF"            >> $START_SCRIPT
        echo "else"            >> $START_SCRIPT
        echo "    echo ERROT: \$AGENT_CONF defaults file missing." >> $START_SCRIPT
        echo "fi"              >> $START_SCRIPT
        echo "if [ \$APPPROXY -eq 1 ];then" >> $START_SCRIPT
        echo "    MONITOR_FLAG=\$(ps -u \${AGENT_USER} | grep -v grep | grep monitor)" >> $START_SCRIPT
        echo "    if [ \"\${MONITOR_FLAG}\" = \"\" ];then" >> $START_SCRIPT
        echo "        nohup su - \${AGENT_USER} -s ${SHELL_TYPE_SH} ${UNIX_CMD} -c \"\${AGENT_ROOT}/bin/monitor &\" > /dev/null" >> $START_SCRIPT
        echo "    fi" >> $START_SCRIPT
        echo "fi"     >> $START_SCRIPT
                
        touch $AGENT_CONF
        echo "APPPROXY=1" >> $AGENT_CONF

        CHMOD 755 $START_SCRIPT
        CHMOD 755 $AGENT_CONF
        ln -s $START_SCRIPT $SCRIPT
    elif [ "SunOS" = "$SYS_NAME" ]; then
        START_SCRIPT=/etc/init.d/agentstart
        SCRIPT=/etc/rc2.d/S99agentstart
        
        rm -rf "${SCRIPT}"
        rm -rf "${START_SCRIPT}"
        
        echo "#!/sbin/sh" >       $START_SCRIPT
        echo "AGENT_ROOT=${AGENT_ROOT_PATH}"  >> $START_SCRIPT
        echo "AGENT_USER=rdadmin" >> $START_SCRIPT
        echo "PATH=/bin:$PATH" >> $START_SCRIPT
        echo "export PATH"     >> $START_SCRIPT
        echo "MONITOR_FLAG=\`ps -u \${AGENT_USER} | grep -v grep | grep monitor\`" >> $START_SCRIPT
        echo "if [ \"\${MONITOR_FLAG}\" = \"\" ];then" >> $START_SCRIPT
        echo "    nohup su - \${AGENT_USER} -c \"\${AGENT_ROOT}/bin/monitor &\" > /dev/null" >> $START_SCRIPT
        echo "fi" >> $START_SCRIPT
        
        CHMOD 755 $START_SCRIPT
        
        ln -s $START_SCRIPT $SCRIPT 
    else
        Log "Unsupport OS."
    fi
}

RDsubstr()
{
    from=$2
    if [ $# -eq 2 ]
    then
        echo $1 | $MYAWK -v bn="$from" '{print substr($1,bn)}'    
    elif [ $# -eq 3 ]
    then
        len=$3
        echo $1 | $MYAWK -v bn="$from" -v ln="$len" '{print substr($1,bn,ln)}'
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

CheckEntropyEnough()
{
    entropyValue=`cat /proc/sys/kernel/random/entropy_avail` >/dev/null 2>&1
    if [ "${entropyValue}" = "" ]; then
        return 1
    fi
    Log "Current entropy_avail is ${entropyValue}."
    if [ ${entropyValue} -gt 1000 ]; then
        return 0
    fi
    return 1
}

GetUserShellType()
{
    oracleUser=$1
    gridUser=$2
    [ -z "${oracleUser}" ] && oracleUser=oracle
    [ -z "${gridUser}" ] && gridUser=grid

    if [ "${SYS_NAME}" = "AIX" ] || [ "$SYS_NAME" = "HP-UX" ]; then
        SHELLTYPE=`cat /etc/passwd | grep "^${oracleUser}:" | $MYAWK -F "/" '{print $NF}'`
        if [ "$SHELLTYPE" = "bash" ]; then
            ORACLE_SHELLTYPE=-l
        fi
        SHELLTYPE=`cat /etc/passwd | grep "^${gridUser}:" | $MYAWK -F "/" '{print $NF}'`
        if [ "$SHELLTYPE" = "bash" ]; then
            GRID_SHELLTYPE=-l
        fi
        SHELLTYPE=`cat /etc/passwd | grep "^rdadmin:" | $MYAWK -F "/" '{print $NF}'`
        if [ "$SHELLTYPE" = "bash" ]; then
            RDADMIN_SHELLTYPE=-l
        fi     
    fi
}

PrintPrompt()
{
    if [ "${SYS_NAME}" = "Linux" ]; then
        echo -n ">> "
    else
        echo ">> \c"
    fi
}

#VgName:abc,a-b
#LvName:def,c-d
#Vg_Lv_Name:abc-def,a--b-c--d
#return VgName
AnalyseVgName_Linux_LVM()
{
    Vg_Lv_Name=$1
    LEN=${#Vg_Lv_Name}
    NUM1=1
    while [ $NUM1 -le $LEN ] 
    do
        STR1=`echo $Vg_Lv_Name | cut -b $NUM1`
        if [ "$STR1" = "-" ]
        then
            NUM2=`expr $NUM1 + 1`
            STR2=`echo $Vg_Lv_Name | cut -b $NUM2`
            if [ "$STR2" = "-" ] 
            then
                NUM1=`expr $NUM1 + 2`
                continue
            fi
            break
        fi
        NUM1=`expr $NUM1 + 1`
    done
    NUM1=`expr $NUM1 - 1`
    NUM2=`expr $NUM1 + 2`
    Vg_Name=`echo $Vg_Lv_Name | cut -b 1-$NUM1`
    LEN=${#Vg_Name}
    NUM1=1
    while [ $NUM1 -le $LEN ]
    do
        STR1=`echo $Vg_Name | cut -b $NUM1`
        if [ "$STR1" = "-" ]
        then
            NUM2=`expr $NUM1 + 1`
            STR2=`echo $Vg_Name | cut -b $NUM2`
            if [ "$STR2" = "-" ] 
            then
                NUM2=`expr $NUM2 + 1`
                Vg_Name=`echo $Vg_Name | cut -b 1-$NUM1,$NUM2-`
                LEN=${#Vg_Name}
            fi
        fi
        NUM1=`expr $NUM1 + 1`
    done
    echo "$Vg_Name"
}

#function:check if the linux is redhat7
#param: no
#return: if is redhat7 then return 1, else return 0;
IsRedhat7()
{
    #is not redhat
    if [ ! -f /etc/redhat-release ]
    then
        return 0;
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

#function:Change directory attribute(+i)
#param: $1: +i or -i
#return: 0 on success ,1 on failure
ChangeDirAttribute()
{
    L_OPT="$1"
    if [ "AIX" = "${SYS_NAME}" ] || [ "HP-UX" = "${SYS_NAME}" ] || [ "SunOS" = "${SYS_NAME}" ];then 
        return 0 
    fi
    if [ "+i" = "${L_OPT}" ] || [ "-i" = "${L_OPT}" ];then  
        chattr ${L_OPT} ${AGENT_ROOT_PATH}
        if [ 0 -ne $? ];then
            Log "chattr ${L_OPT} ${AGENT_ROOT_PATH} failed."
            return 1
        fi
        Log "chattr ${L_OPT} ${AGENT_ROOT_PATH} success."
        
        chattr ${L_OPT} ${AGENT_ROOT_PATH}/bin
        if [ 0 -ne $? ];then
            Log "chattr ${L_OPT} ${AGENT_ROOT_PATH}/bin failed."
            return 1
        fi
        Log "chattr ${L_OPT} ${AGENT_ROOT_PATH}/bin success."
        
        return 0
    fi
    
    Log "The operate(${L_OPT}) cannot support."
    return 1
}

#function: Get nginx listen ip and port
GetNginxIPandPort()
{
    NGINX_CONF_FILE="${AGENT_ROOT_PATH}/nginx/conf/nginx.conf"
    if [ ! -f "${NGINX_CONF_FILE}" ]
    then
        Log "The ${NGINX_CONF_FILE} does not exist."
        return 0
    fi
    NGINX_IP_AND_PORT=`SUExecCmdWithOutput "cat ${NGINX_CONF_FILE} 2>/dev/null | grep \"listen\" | sed -n \"1p\" | ${MYAWK} -F\" \" '{print \\$2}'"`
    NGINX_LISTEN_IP=`echo ${NGINX_IP_AND_PORT} | ${MYAWK} -F":" '{print $1}'`
    NGINX_LISTEN_PORT=`echo ${NGINX_IP_AND_PORT} | ${MYAWK} -F":" '{print $2}'`
    return 0
}

#function: Add firewall
#param: $1: nginx listen port.
#return: 0 on success ,1 on failure
SetFirewall()
{
    Log "Begin add firewall."
    
    IPTABLES_PATH=`which iptables 2>/dev/null | $MYAWK '{print $1}'`
    if [ "" = "${IPTABLES_PATH}" ]
    then
        Log "The iptables does not exist, no need add firewall."
        return 0
    fi
    
    L_NGINX_IP=${NGINX_LISTEN_DEFAULT_IP}
    L_NGINX_PORT=$1
    if [ "" = "${L_NGINX_PORT}" ];then
        Log "The nginx port is empty, add firewall failed."
        return 1
    fi
    
    iptables -vnL | grep -w ACCEPT | grep -w "0.0.0.0/0" | grep -w "${L_NGINX_IP}/0" | grep -w "dpt:${L_NGINX_PORT}" | grep -w "tcp" >/dev/null
    if [ $? -ne 0 ]
    then
        Log "Add firewall: iptables -I INPUT -s 0.0.0.0/0 -d ${L_NGINX_IP}/0 -p tcp --dport ${L_NGINX_PORT} -j ACCEPT"
        iptables -I INPUT -s 0.0.0.0/0 -d ${L_NGINX_IP}/0 -p tcp --dport ${L_NGINX_PORT} -j ACCEPT >/dev/null
        if [ $? -ne 0 ]
        then
            Log "Add firewall failed. cmd: iptables -I INPUT -s 0.0.0.0/0 -d ${L_NGINX_IP}/0 -p tcp --dport ${L_NGINX_PORT} -j ACCEPT"
            return 1
        fi
    fi
    
    iptables -vnL | grep -w "ACCEPT" | grep -w "0.0.0.0/0" | grep -w "lo" >/dev/null
    if [ $? -ne 0 ]
    then
        Log "Add firewall: iptables -A INPUT -i lo -j ACCEPT"
        iptables -A INPUT -i lo -j ACCEPT >/dev/null
        if [ $? -ne 0 ]
        then
            Log "Add firewall failed. cmd: iptables -A INPUT -i lo -j ACCEPT"
            return 1
        fi
    fi
    
    Log "Add firewall success."
    return 0
}

#function: get OS version
#param: None
#return: echo version, TYPER_VER, REDHAT_6.5 ORACLELINUX_7.5 CENTOS_7.2.1511
getOSVersion()
{
    if [ ! -f "/etc/redhat-release" ]
    then
        Log "OS is not support."
        return
    fi

    if [ -f "/etc/oracle-release" ]
    then
        VERSION=`cat /etc/oracle-release 2>/dev/null | ${MYAWK} '{print $NF}'`
        DRIVER_OS_VERSION=${VERSION}
        echo "ORACLELINUX_${VERSION}"
        return
    fi
    
    cat /etc/redhat-release | grep "CentOS Linux release" >> /dev/null
    if [ $? -eq 0 ]
    then
        VERSION=` cat /etc/redhat-release 2>/dev/null | ${MYAWK} -F '(' '{print $1}' | ${MYAWK} '{print $NF}'`
        DRIVER_OS_VERSION=${VERSION}
        echo "CENTOS_${VERSION}"
        return
    fi
    
    cat /etc/redhat-release | grep "Red Hat Enterprise Linux Server release" >> /dev/null
    if [ $? -eq 0 ]
    then
        VERSION=`cat /etc/redhat-release 2>/dev/null | ${MYAWK} -F '(' '{print $1}' | ${MYAWK} '{print $NF}'`
        DRIVER_OS_VERSION=${VERSION}
        echo "REDHAT_${VERSION}"
        return
    fi
    
    Log "UnSupoort OS `cat /etc/redhat-release`"
}

getWWNBy83pge()
{
    deviceName="${1}"
    g_wwn83pge=""
    sgResult="`su - rdadmin -s ${SHELL_TYPE_SH} -c "${EXPORT_ENV}${CMD_TOOL} getwwn ${1}" 2>&1`"
    Log "sgResult=${sgResult}"
    if [ `echo "${sgResult}"|grep -E -c "agentcli is locked"` -ne 0 ] || [ `echo "${sgResult}"|grep -E -c "Init .* failed."` -ne 0 ]
    then
        Log "agentcli excute failed, do nothing."
        exit 1
    fi
    
    if [ `echo "${sgResult}" | grep -E -c "Designation descriptor number"` -eq 0 ]
    then
        Log "inquiry WNN failed! [$1, dev/${sgDev}]"
        return 2
    fi
    g_wwn83pge=`echo "${sgResult}" | sed -n "s/^.*\[0[x|X]\(.*\)\].*$/\1/p"`
    g_wwn83pge="`echo ${g_wwn83pge} | tr [a-z] [A-Z]`"
}

ExitWithError()
{
    Log "$1 is invalid."
    exit $ERROR_PARAM_INVALID
}

ExitWithErrorCode()
{
    Log "[Err]: $1"
    exit $2
}

CheckDirRWX()
{
    ckUser=$1
    dirArry=`echo $2 | sed 's/;/ /g'`
    for ckDir in ${dirArry}; do
        if [ "`RDsubstr ${ckDir} 1 1`" = "/" ]; then
            su - ${ckUser} -c "[ ! -z \"$ckDir\" -a -d \"$ckDir\" -a -r \"$ckDir\" -a -w \"$ckDir\" -a -x \"$ckDir\" ] || exit 1"
            return $?
        fi
    done
    return 0
}

AddUnixTimestamp()
{
    time=`echo $1 | tr "_" " "`
    if [ $SYS_NAME = "Linux" ]; then
        su - rdadmin -s $SHELL_TYPE_SH $RDADMIN_SHELLTYPE -c "${EXPORT_ENV}$AGENT_ROOT_PATH/bin/agentcli  UnixTimestamp \"$time\" Date2Unix"
    else
        su - rdadmin $RDADMIN_SHELLTYPE -c "${EXPORT_ENV}$AGENT_ROOT_PATH/bin/agentcli  UnixTimestamp \"$time\" Date2Unix"
    fi
}


UnixTimestampToDate()
{
    time=$1
    if [ $SYS_NAME = "Linux" ]; then
        su - rdadmin -s $SHELL_TYPE_SH $RDADMIN_SHELLTYPE -c "${EXPORT_ENV}$AGENT_ROOT_PATH/bin/agentcli  UnixTimestamp \"$time\" Unix2Date"
    else
        su - rdadmin $RDADMIN_SHELLTYPE -c "${EXPORT_ENV}$AGENT_ROOT_PATH/bin/agentcli UnixTimestamp \"$time\" Unix2Date"
    fi
}

# check multipath type
# return 0 -- ultrapath
# return 1 -- device mapper multipath
CheckMultiPathType()
{
    # check ultrapath
    UP=`rpm -qa | grep UltraPath`
    if [ $? -eq 0 ]
    then
        Log "UltraPath:${UP} is installed."
        return 0
    fi
    
    if [ -f /etc/linx-release ]    # no rocky check rpm, rocky check process
    then
        return 1
    else
        # check device mapper multipath
        DMMP=`rpm -qa | grep device-mapper`
        if [ $? -eq 0 ]
        then
            Log "DMMP:${DMMP} is installed."
            return 1
        fi
    fi
    
    Log "No multipath sw is installed."
    return 0
}

RefreshVolCapacity()
{
    diskName=$1

    # refresh disk capacity
    CheckMultiPathType
    ret=$?
    if [ ${ret} -eq 2 ]; then
        exit 1
    fi
    
    # get disk name
    diskTmp=`echo ${diskName} | ${MYAWK} -F '/' '{print $NF}'`
    if [ -z "${diskTmp}" ]; then
        Log "ERR:${diskName} get disk name failed."
        exit 1
    fi

    if [ ${ret} -eq 0 ]; then
        # ultraPath
        echo 1 > /sys/block/${diskTmp}/device/rescan
    else
        # multipath
        multipath -ll >> $LOG_FILE_NAME
        echo 1 > /sys/block/${diskTmp}/device/rescan
        diskWWN=`ls -l /dev/disk/by-id/ | grep "${diskTmp}" | ${MYAWK} -F' ' '{print $(NF-2)}' | ${MYAWK} -F'-' '{print $NF}'`
        if [ -z "$diskWWN" ]; then
            Log "ERR:${diskName} get disk wwn failed."
            exit 1
        fi
        diskDM=`ls -l /dev/disk/by-id/ | grep "dm-uuid-mpath-${diskWWN}" | ${MYAWK} -F'/' '{print $NF}'`
            if [ -z "$diskDM" ]; then
            Log "ERR:${diskName} get disk dm name failed."
            exit 1
        fi
        multipathd resize map ${diskDM}
    fi
}

CheckIsIpv6()
{
    ipAddr=$1
    chkResult=`expr index "${ipAddr}" ":"`
    if [ $chkResult -ne 0 ];then
        Log "This is a ipv6:${ipAddr}"
        echo 0
    else
        echo 1
    fi
}

AddPortFilter_rh6()
{
    portFilter=$1
    index=0
    firewallNeedClose=0

    while [ 1 ]
    do
        firewall_port=`service iptables status | grep $portFilter`
        fireware_status=`service iptables status | ${MYAWK} '{printf $4}'`
        if [ "$fireware_status" = "not" ]; then
            service iptables start >/dev/nul
            firewallNeedClose=1
        fi

        if [ ! "$fireware_status" = "not" ]; then
            Port_state=`netstat -anp | grep ${portFilter}`
            if [ "${Port_state}" = "" ]; then
                echo "The port of nginx does not exist."
                break
            else
                if [ "${firewall_port}" = "" ]; then
                    /sbin/iptables -I INPUT -p tcp --dport ${portFilter} -j ACCEPT >/dev/nul
                    service iptables save >/dev/nul
                    service iptables restart >/dev/nul
                else
                    Log "The firewall port ${portFilter} had been opened."
                    break
                fi
            fi
            if [ "${firewall_port}" = "" ]; then
                index=`expr $index + 1`
            else
                echo "The Nginx listening port ${portFilter} is successfully added to the firewall rule chain."
                break
            fi
        else
            index=`expr $index + 1`
        fi

        if [ $index -eq 3 ]; then
            echo "Warning: The firewall is enabled failed.please Manually enable the firewall and add the port input and output functions."
            break
        fi
    done

    if [ $firewallNeedClose -eq 1 ]; then
        service iptables stop >/dev/null 
    fi
}

AddPortFilter_SUSE()
{
    portFilter=$1
    index=0
    firewallNeedClose=0

    while [ 1 ]
    do
        if [ -f "/etc/sysconfig/SuSEfirewall2" ]; then
            firewall_port=`grep ${portFilter} /etc/sysconfig/SuSEfirewall2`
            fireware_status=`rcSuSEfirewall2 status | ${MYAWK} '{printf $NF}'`
            if [ "$fireware_status" = "..unused" ]; then
                rcSuSEfirewall2 start >/dev/nul 2>&1
                firewallNeedClose=1
            fi
        fi

        if [ "$fireware_status" = "..running" ]; then
            Port_state=`netstat -anp | grep ${portFilter}`
            if [ "${Port_state}" = "" ]; then
                echo "The port of nginx does not exist."
                break
            else
                if [ "${firewall_port}" = "" ]; then
                    sed -i '/FW_SERVICES_EXT_TCP="ssh"/aFW_SERVICES_EXT_TCP="59526"' /etc/sysconfig/SuSEfirewall2
                    rcSuSEfirewall2 restart >/dev/nul 2>&1
                else
                    Log "The firewall port ${portFilter} had been opened."
                    break
                fi
            fi
            if [ "${firewall_port}" = "" ]; then
                index=`expr $index + 1`
            else
                echo "The Nginx listening port ${portFilter} is successfully added to the firewall rule chain."
                break
            fi
        else
            index=`expr $index + 1`
        fi

        if [ $index -eq 3 ]; then
            echo "Warning: The firewall is enabled failed.please Manually enable the firewall and add the port input and output functions."
            break
        fi
    done

    if [ $firewallNeedClose -eq 1 ]; then
        rcSuSEfirewall2 stop >/dev/nul 2>&1
    fi
}
AddPortFilter()
{
    portFilter=$1
    index=0
    firewallNeedClose=0

    while [ 1 ]
    do
        firewall_port=`firewall-cmd --zone=public --list-ports | grep ${portFilter}`
        fireware_status=`systemctl status firewalld | grep "Active" | ${MYAWK} -F " " '{print $3}'`
        if [ "${fireware_status}" = "(dead)" ]; then
            systemctl start firewalld
            firewallNeedClose=1
        fi
        if [ "${fireware_status}" = "(running)" ]; then
            Port_state=`netstat -anp | grep ${portFilter}`
            if [ "${Port_state}" = "" ]; then
                echo "The port of nginx does not exist."
                break
            else
                if [ "${firewall_port}" = "" ]; then
                    firewall-cmd --zone=public --add-port=${portFilter}/tcp --permanent >/dev/null
                    firewall-cmd --reload >/dev/null
                else
                    Log "The firewall port ${portFilter} had been opened."
                    break
                fi
            fi
            
            if [ "${firewall_port}" = "" ]; then
                index=`expr $index + 1`
            else
                echo "The Nginx listening port ${portFilter} is successfully added to the firewall rule chain."
                break
            fi
        else
            index=`expr $index + 1`
        fi

        if [ $index -eq 3 ]; then
            echo "Warning: The firewall is enabled failed.please Manually enable the firewall and add the port input and output functions."
            break
        fi
    done

    if [ $firewallNeedClose -eq 1 ]; then 
        systemctl stop firewalld
    fi
}

CheckFirewallActivity()
{
    Flag_Firewall_Activity="false"
    if [ "Linux" = "${SYS_NAME}" ]; then
        which ufw 1>/dev/null 2>&1
        if [ $? -eq 0 ]; then
            ufw status | grep -v "inactive" | grep "active" 1>/dev/null 2>&1
            if [ $? -eq 0 ]; then
                Flag_Firewall_Activity="true"
            fi
        else
            which systemctl 1>/dev/null 2>&1
            if [ $? -eq 0 ]; then
                Is_Active=`systemctl status firewalld | grep "Active" | ${MYAWK} -F " " '{print $3}'`
                if [ "${Is_Active}" = "(running)" ]; then
                    Flag_Firewall_Activity="true"
                fi
            else
                service iptables status 1>/dev/null 2>&1
                if [ $? -eq 0 ]; then
                    Flag_Firewall_Activity="true"
                fi
            fi
        fi
    elif [ "HP-UX" = "${SYS_NAME}" ]; then
        if [ -f "/etc/rc.config.d/ifconf" ]; then
            Is_Active=`cat /etc/rc.config.d/ifconf | grep "IPF_START" | ${MYAWK} -F "=" '{print $2}'`
            if [ "${Is_Active}" = "1" ]; then
                Flag_Firewall_Activity="true"
            fi
        fi
    elif [ "AIX" = "${SYS_NAME}" ]; then
        Flag_Firewall_Activity="false"
    elif [ "SunOS" = "${SYS_NAME}" ]; then
        Is_Active=`svcs -a |grep network |grep "ipfilter" | ${MYAWK} -F " " '{print $1}'`
        if [ "${Is_Active}" = "enable" ]; then
            Flag_Firewall_Activity="true"
        fi
    fi
    
    if [ "${Flag_Firewall_Activity}" = "true" ]; then
        echo "The firewall is active, the ProtectManager may not be able to access to the agent client."
        return 1
    fi

    return 0
}

RemovePortFilter()
{
    portFilter=$1
    Log "Remove port filter:${portFilter}"
    firewallNeedClose=0
    fireware_status=`systemctl status firewalld | grep "Active" | ${MYAWK} -F " " '{print $3}'`
    if [ "${fireware_status}" = "(dead)" ]; then
        systemctl start firewalld
        firewallNeedClose=1
    fi
    while [ 1 ]
    do
        firewall_port=`firewall-cmd --zone=public --list-ports | grep ${portFilter}`
        fireware_status=`systemctl status firewalld | grep "Active" | ${MYAWK} -F " " '{print $3}'`
        if [ "${fireware_status}" = "(dead)" ]; then
            systemctl start firewalld
        fi

        if [ "${fireware_status}" = "(running)" ]; then
            Port_state=`netstat -anp | grep ${portFilter}`
            if [ "${Port_state}" = "" ]; then
                echo "The port of nginx does not exist."
                break
            else
                firewall-cmd --zone=public --remove-port=${portFilter}/tcp --permanent 1>/dev/null 2>&1
                firewall-cmd --reload 1>/dev/null 2>&1
            fi
            
            if [ "${firewall_port}" = "" ]; then
                index=`expr $index + 1`
            else
                Log "The firewall port ${portFilter} is removed success."
                break
            fi
        else
            index=`expr $index + 1`
        fi

        if [ $index -eq 3 ]; then
            echo "Warning: The firewall is enabled failed.please Manually enable the firewall and add the port input and output functions."
            break
        fi
    done

    if [ $firewallNeedClose -eq 1 ]; then 
        systemctl stop firewalld
    fi
}

KillSonPids()
{
	pids=`ps -ef | ${MYAWK} '{if($3=='$1'){print $2} }'`;
        kill -9 $1 2>/dev/null
	if [ -n "$pids" ]; then
		for pid in $pids
		do
		    KillSonPids $pid
		done
	fi
}

# Command timeout function
Timeout()
{
    waitsec=${WAIT_SEC}
    `$*` & pid=$!
    if [ "$SYS_NAME" = "SunOS" ]; then
        `sleep $waitsec && KillSonPids $pid` & watchdog=$!
    else
        `sleep $waitsec && kill -9 $pid 2>/dev/null` & watchdog=$!
    fi
    if wait $pid 2>/dev/null; then
        if [ "$SYS_NAME" = "SunOS" ]; then
            KillSonPids $watchdog
        else
            kill -9 $watchdog 2>/dev/null
        fi
        wait $watchdog 2>/dev/null
        return 0
    else
        status=$?
        pid_exit=`ps -ef | ${MYAWK} '{print $2}'| grep -w $pid`
        if [ ! $pid_exit ]; then
            if [ "$SYS_NAME" = "SunOS" ]; then
                KillSonPids $watchdog
            else
                kill -9 $watchdog 2>/dev/null
            fi
            wait $watchdog 2>/dev/null
        fi
        return $status
    fi
}

TimeoutWithoutOutput()
{
    waitsec=10
    `$* >/dev/null 2>&1` & pid=$!
    if [ "$SYS_NAME" = "SunOS" ]; then
        `sleep $waitsec && KillSonPids $pid` & watchdog=$!
    else
        `sleep $waitsec && kill -9 $pid 2>/dev/null` & watchdog=$!
    fi
    if wait $pid 2>/dev/null; then
        if [ "$SYS_NAME" = "SunOS" ]; then
            KillSonPids $watchdog
        else
            kill -9 $watchdog 2>/dev/null
        fi
        wait $watchdog 2>/dev/null
        return 0
    else
        status=$?
        pid_exit=`ps -ef | ${MYAWK} '{print $2}'| grep -w $pid`
        if [ ! $pid_exit ]; then
            if [ "$SYS_NAME" = "SunOS" ]; then
                KillSonPids $watchdog
            else
                kill -9 $watchdog 2>/dev/null
            fi
            wait $watchdog 2>/dev/null
        fi
        return $status
    fi
}

TimeoutWithOutput()
{
    porcessId=""
    cmd=""
    if [ $# -eq 2 ]; then
        cmd=$1
        porcessId=$2
    elif [ $# -eq 3 ];then
        cmd="$1 $2"
        porcessId=$3
    fi
    waitsec=10
    `${cmd} >> ${STMP_PATH}/return_${porcessId}` & pid=$!
    if [ "$SYS_NAME" = "SunOS" ]; then
        `sleep $waitsec && KillSonPids $pid` & watchdog=$!
    else
        `sleep $waitsec && kill -9 $pid 2>/dev/null` & watchdog=$!
    fi
    if wait $pid 2>/dev/null; then
        if [ "$SYS_NAME" = "SunOS" ]; then
            KillSonPids $watchdog
        else
            kill -9 $watchdog 2>/dev/null
        fi
        wait $watchdog 2>/dev/null
        returnVal=`cat ${STMP_PATH}/return_${porcessId}`
        rm -rf ${STMP_PATH}/return_${porcessId}
        echo "${returnVal}"
    else
        status=$?
        pid_exit=`ps -ef | ${MYAWK} '{print $2}'| grep -w $pid`
        if [ ! $pid_exit ]; then
            if [ "$SYS_NAME" = "SunOS" ]; then
                KillSonPids $watchdog
            else
                kill -9 $watchdog 2>/dev/null
            fi
            wait $watchdog 2>/dev/null
        fi
        rm -rf ${STMP_PATH}/return_${porcessId}
        echo ""
    fi
}

TimeoutMountWithOutput()
{
    cmd="$*"
    waitsec=${WAIT_SEC}
    porcessId=$$
    `${cmd} >> ${STMP_PATH}/return_${porcessId} 2>&1` & pid=$!
    if [ "$SYS_NAME" = "SunOS" ]; then
        `sleep $waitsec && KillSonPids $pid` & watchdog=$!
    else
        `sleep $waitsec && kill -9 $pid 2>/dev/null` & watchdog=$!
    fi
    if wait $pid 2>/dev/null; then
        if [ "$SYS_NAME" = "SunOS" ]; then
            KillSonPids $watchdog
        else
            kill -9 $watchdog 2>/dev/null
        fi
        wait $watchdog 2>/dev/null
    else
        status=$?
        pid_exit=`ps -ef | awk '{print $2}'| grep -w $pid`
        if [ ! $pid_exit ]; then
            if [ "$SYS_NAME" = "SunOS" ]; then
                KillSonPids $watchdog
            else
                kill -9 $watchdog 2>/dev/null
            fi
            wait $watchdog 2>/dev/null
        fi
    fi
    returnVal=`cat ${STMP_PATH}/return_${porcessId}`
    rm -rf ${STMP_PATH}/return_${porcessId}
    echo "${returnVal}"
}

IpAddrVerify()
{
    if [ "$SYS_NAME" = "SunOS" ]; then
        Log "SunOS unsupport regex."
        return 0
    fi
    ipAddr=$1
    echo "$ipAddr" | grep -E "^([a-fA-F0-9]{1,4}(:|\.)){0,7}(:|::)?([a-fA-F0-9]{1,4}(:|\.)){0,7}([a-fA-F0-9]{1,4})?$"
    if [ "$?" = "0" ];then
        Log "$ipAddr verify success."
        return 0
    else
        Log "$ipAddr verify failed."
        return 1
    fi
}

VerifyAndPing()
{
    ipAddr=$1
    IpAddrVerify $ipAddr
    if [ $? -ne 0 ]; then
        Log "$ipAddr verify failed."
        return 1
    fi
    echo ${ipAddr} | grep "\\:" >/dev/null 2>&1
    if [ 0 -eq $? ]; then
        echo "IPV6"
        if [ "${SYS_NAME}" = "SunOS" ]; then
            ping6 -s ${ipAddr} 56 1 > /dev/null 2>&1
        else
            ping6 -c 1 -w 3 ${ipAddr} > /dev/null 2>&1
        fi
        return $?
    fi
    echo ${ipAddr} | grep "\\." >/dev/null 2>&1
    if [ 0 -eq $? ]; then
        echo "IPV4"
        if [ "${SYS_NAME}" = "SunOS" ]; then
            ping -s ${ipAddr} 56 1 > /dev/null 2>&1
        else
            ping -c 1 -w 3 ${ipAddr} > /dev/null 2>&1
        fi
        return $?
    fi
    return 1
}

IpPing()
{
    pingCmd=$1
    nic=$2
    ip=$3
    if [ "${SYS_NAME}" = "SunOS" ]; then
        ${pingCMD} -i ${nic} -s ${ip} 56 1 > /dev/null 2>&1
    elif [ "${SYS_NAME}" = "AIX" ] || [ "${SYS_NAME}" = "HP-UX" ]; then
        ${pingCMD} -c 1 -o ${nic} ${ip} > /dev/null 2>&1
    else
        ${pingCMD} -c 1 -W 3 -I ${nic} ${ip} > /dev/null 2>&1
    fi
    return $?
}

KillDataProcess()
{
    # Stop the dataprocess process.
    if [ "$SYS_NAME" = "Linux" ]; then
        dataprocess_pid=`ps -afx | grep dataprocess | grep -v grep | $MYAWK '{print $1}'`
        if [ -n "$dataprocess_pid" ]; then
            Log "Send kill to dataprocess, call: kill -9 $dataprocess_pid"
            kill -9 $dataprocess_pid
        fi
    fi
}

DeleteAgentUserFile()
{
    if [ $# -lt 1 ]; then
        return 0
    fi

    for i in "$@"; do
        if [ -f "${i}" ]; then
            su - ${AGENT_USER} -s ${SHELL_TYPE_SH} -c "rm -rf ${i}"
        fi
    done
}

CreateSTMPFile()
{
    File="$1"
    if [ -f "$File" ]; then
        rm -rf $File
    fi
    touch ${File}
    chmod 640 ${File}
}

CreateOracleSTMPFile()
{
    CreateSTMPFile $1
    if [ -z "${ORA_DB_USER}" ]; then
        GetOracleUser
    fi
    chown ${ORA_DB_USER}:${ORA_DB_GROUP} ${File}
}

CreateGridSTMPFile()
{
    CreateSTMPFile $1
    if [ -z "${ORA_GRID_USER}" ]; then
        GetOracleUser
    fi
    chown ${ORA_GRID_USER}:${ORA_GRID_GROUP} ${File}
}

# 检查CPU是否支持constant_tsc特性
CheckCpuConstantTsc()
{
    cpu_tsc=`cat /proc/cpuinfo | grep -o constant_tsc`
    if [ ! -z "${cpu_tsc}" ]; then
        return 0
    fi
    return 1
}

#检查是否为分布式场景
CheckDistributedScenes()
{
    DEPLOY_TYPE=`cat ${INSTALL_PACKAGE_PATH}/conf/client.conf | grep "deploy_type" | ${MYAWK} -F '=' '{print $NF}'`
    if [ "$DEPLOY_TYPE" == "d7" ]; then
        Log "The Distributed Scenes do not support dataturbo."
        return 1
    else
        return 0
    fi
}

#开关关闭时将跳过datatubo
SkipDatatuboInstall()
{
    #由conf文件里的参数来判断是否跳过datatubo
    IS_ENABLE_DATATURBO=`cat ${INSTALL_PACKAGE_PATH}/conf/client.conf | grep "is_enable_dataturbo" | ${MYAWK} -F '=' '{print $NF}'`
    if [ "$IS_ENABLE_DATATURBO" == "false" ] || [ "$IS_ENABLE_DATATURBO" == "" ]; then
        Log "Service close and not install datatubo."
        return 1
    else
        return 0
    fi
}

PrepareDataTurboPkg()
{
    dataturboZipPath=$1
    # dataturbo安装包不存在
    if [ ! -f $dataturboZipPath ]; then
        Log "Missing dataturbo installation package."
        return 1
    fi

    # 解压压缩包到指定目录
    dataturboTmp=$2
    rm -rf $dataturboTmp
    mkdir -p $dataturboTmp
    unzip -o $dataturboZipPath -d $dataturboTmp > /dev/null
    mv $dataturboTmp/OceanStor_DataTurbo_*_Linux $dataturboTmp/dataturbo

    if [ ! -d "${dataturboTmp}/dataturbo" ]; then
        Log "Dataturbo pkg(${dataturboTmp}/dataturbo) not exit."
        return 1
    fi
    return 0
}

IsDataTurboInstalled()
{
    # debin系列操作系统用dpkg
    if [ -f "/etc/debian_version" ]; then
        dpkg -l | grep dataturbo | grep ^ii >/dev/null 2>&1
        if [ $? -ne 0 ]; then
            Log "No dataturbo have been installed."
            return 1
        fi
        Log "dataturbo have been installed."
        return 0
    fi

    # 默认使用rpm查询
    rpm -q dataturbo >/dev/null 2>&1
    if [ $? -ne 0 ]; then
        Log "No dataturbo have been installed."
        return 1
    fi
    Log "dataturbo have been installed."
    return 0
}

clearDataturboMountPath()
{
    Log "Begin to clear dataturbo mount path"
    dataturbo_mount_dirs=`mount | grep "/mnt/databackup" | grep "Dataturbo" | ${MYAWK} '{print $3}'`
    for mount_dir in $dataturbo_mount_dirs; do
        Log "umount path ${mount_dir}"
        if [ "${SYS_NAME}" = "AIX" ]; then
            umount $mount_dir
        else
            umount -l $mount_dir
        fi
    done
}

# 大于
VersionGt()
{
    test "`echo "$@" | tr " " "\n" | sort -V | head -n 1`" != "$1";
}

#小于
VersionLt()
{ 
    test "`echo "$@" | tr " " "\n" | sort -rV | head -n 1`" != "$1"; 
}

# 大于等于
VersionGe()
{
    test "`echo "$@" | tr " " "\n" | sort -rV | head -n 1`" = "$1";
}

IsDataturboVersionCanUpgrade()
{
    pkgType="rpm"
    curPkgVersion=""
    curPkgRelease=""
    processorType=`uname -m`
    if [ -f "/etc/debian_version" ]; then
        pkgType="deb"
        curPkg=`dpkg -l | grep dataturbo | ${MYAWK} '{print $3}'`
        # curPkg: 1.1.RC1-202302201206
        curPkgVersion=`echo ${curPkg} | ${MYAWK} -F '-' '{print $1}'`
        curPkgRelease=`echo ${curPkg} | ${MYAWK} -F '-' '{print $2}'`
    else
        pkgType="rpm"
        curPkg=`rpm -qa dataturbo 2>/dev/null`
        # curPkg: dataturbo-1.1.RC1-202302201206.x86_64
        curPkgVersion=`echo ${curPkg} | ${MYAWK} -F '-' '{print $2}'`
        curPkgRelease=`echo ${curPkg} | ${MYAWK} -F '-' '{print $3}' | ${MYAWK} -F '.' '{print $1}'`
    fi

    dataturboNew=$1
    pkgNew=`ls ${dataturboNew}/dataturbo/packages/ | grep dataturbo | grep ${pkgType} | grep ${processorType}`
    # pkgNew: oceanstor_dataturbo_1.5.RC1_linux_x86_64_202407231912.rpm
    newPkgVersion=`echo ${pkgNew} | ${MYAWK} -F '_' '{print $3}'`    # 1.5.RC1
    newPkgRelease=${pkgNew##*_}         # 202407231912.rpm
    newPkgRelease=`echo ${newPkgRelease} | ${MYAWK} -F '.' '{print $1}'`     # 202407231912

    Log "Get dataturbo pkg. cur pkg(${curPkgVersion}-${curPkgRelease}), new pkg(${newPkgVersion}-${newPkgRelease})."
    ShowInfo "Get dataturbo pkg. cur pkg(${curPkgVersion}-${curPkgRelease}), new pkg(${newPkgVersion}-${newPkgRelease})."

    # 当前版本小于升级版本直接升级
    if  VersionLt ${curPkgVersion} ${newPkgVersion}; then
        return 0
    # 如果当前版本大于升级版本返回1 
    elif VersionGt ${curPkgVersion} ${newPkgVersion}; then
        Log "Current version is greater than or equal to upgrade version."
        return 1
    # 如果当前版本等于升级版本则比较release号(时间戳)
    else 
        if VersionGe ${curPkgRelease} ${newPkgRelease}; then
            Log "Current release version ${curPkgRelease} is greater than or equal to upgrade release version ${newPkgRelease}."
            return 1
        fi
    fi
    return 0
}

UnInstallDataTurbo()
{
    systemctl stop dataturbo >> ${LOG_FILE_NAME}
    # debian系列操作系统
    if [ -f "/etc/debian_version" ]; then
        dpkg -l | grep dataturbo | grep ^ii >/dev/null 2>&1
        if [ $? -ne 0 ]; then
            ShowInfo "Not install dataturbo."
            Log "Not install dataturbo."
            return 0
        fi

        dpkg -r dataturbo >> ${LOG_FILE_NAME}
        if [ $? -ne 0 ]; then
            ShowError "Uninstall dataturbo fail."
            Log "Uninstall dataturbo fail."
            return 1
        fi
        dpkg --purge dataturbo >> ${LOG_FILE_NAME}
        Log "Uninstall dataturbo success."
        return 0
    fi

    # 默认使用rpm卸载
    rpm -q dataturbo >/dev/null 2>&1
    if [ $? -ne 0 ]; then
        ShowInfo "Not install dataturbo."
        Log "Not install dataturbo."
        return 0
    fi
    rpm -e --nodeps dataturbo >> ${LOG_FILE_NAME}
    if [ $? -ne 0 ]; then
        ShowError "Uninstall dataturbo fail."
        Log "Uninstall dataturbo fail."
        return 1
    fi
    Log "Uninstall dataturbo success."
    return 0
}

InstallDataTurbo()
{
    dataturboTmp="/opt/oceanstor/DataTurboTmp"
    PrepareDataTurboPkg $1 $dataturboTmp
    if [ $? -ne 0 ]; then
        ShowError "Prapare dataTurbo pkg fail."
        Log "Prapare dataTurbo pkg fail."
        return 1
    fi
    # dataturbo 使用默认使用低档位， 其他挡位用户自行调整
    echo 1 | $dataturboTmp/dataturbo/install.sh -s dataturbo >> ${LOG_FILE_NAME}
    status=$?
    if [ $status -eq 0 ]; then
        touch $DATATURBO_INSTALL_BY_AGENT
        ShowInfo "Install dataturbo success."
        Log "Exec install dataturbo success."
        return 0
    fi
    if [ $status -eq 99 ]; then
        ShowError "The system do not support dataturbo."
        Log "The system do not support dataturbo."
        return 1
    fi
    # 安装dataturbo资源不足
    if [ $status -eq 100 ]; then
        ShowError "lack of resources of installing dataturbo."
        Log "lack of resources of installing dataturbo"
        return 1
    fi
    ShowError "Exec dataturbo install.sh fail."
    Log "Exec dataturbo install.sh fail."
    UnInstallDataTurbo
    return 1
}

CheckDataturboProcess()
{
    # 增加重试机制查询dataturbo进程状态
    retryTime=30
    while [ $retryTime -ge 0 ]
    do
        dataturbo_status=`systemctl is-active dataturbo`
        dataturbo_info=`systemctl status dataturbo`
        if [ $dataturbo_status = "active" ]; then
            Log "dataturbo status is active. get dataturbo info is ${dataturbo_info}"
            return 0
        fi
        
        if [ $dataturbo_status = "failed" ]; then
            Log "dataturbo status is failed. get dataturbo info is ${dataturbo_info}"
            return 1
        fi

        retryTime=`expr $retryTime - 1`
        sleep 1
    done
    return 1
}

UpgradeDataTurbo()
{
    dataturboTmp="/opt/oceanstor/DataTurboTmp"
    dataturboCur="/opt/oceanstor"
    PrepareDataTurboPkg $1 $dataturboTmp
    if [ $? -ne 0 ]; then
        ShowError "Prapare dataTurbo pkg fail."
        Log "Prapare dataTurbo pkg fail."
        return 1
    fi

    # 比较版本，同版本不升级
    IsDataturboVersionCanUpgrade $dataturboTmp
    if [ $? -ne 0 ]; then
        ShowInfo "The dataturbo new version is not greater then cur version, will not upgrade."
        Log "The dataturbo new version is not greater then cur version, will not upgrade."
        return 0
    fi
    ShowInfo "Need upgrade dataturbo to new version."
    Log "Need upgrade dataturbo to new version."

    # 执行dataturbo升级 升级前需要停服务
    systemctl stop dataturbo >> ${LOG_FILE_NAME}
    $dataturboTmp/dataturbo/upgrade.sh >> ${LOG_FILE_NAME}
    if [ $? -ne 0 ]; then
        # dataturbo升级失败会回滚，回滚后需要手动拉起
        ShowError "Upgrade dataturbo failed and need restart cur version dataturbo service."
        Log "Upgrade dataturbo failed and need restart cur version dataturbo service."
        systemctl start dataturbo >> ${LOG_FILE_NAME}
        if [ $? -ne 0 ]; then
            ShowError "Start dataturbo service fail."
            Log "Start dataturbo service fail."
        fi
        return 1
    fi
    touch $DATATURBO_INSTALL_BY_AGENT
    Log "Exec upgrade dataturbo success."

    # 升级成功后需要手动拉新版本dataturbo起否则视为升级失败
    systemctl start dataturbo >> ${LOG_FILE_NAME}
    if [ $? -ne 0 ]; then
        # 防止Dataturbo进程重试导致误判
        CheckDataturboProcess
        if [ $? -ne 0 ]; then
            ShowError "Start dataturbo service fail."
            Log "Start dataturbo service fail."
            return 1
        fi
    fi
    Log "Start dataturbo service success."
    return 0
}

DisableHttpProxy()
{
    TMP_RESULT=`SUExecCmdWithOutput "${AGENT_ROOT_PATH}/sbin/xmlcfg read System enable_http_proxy"`
    if [ ${TMP_RESULT} = "0" ]; then
        Log "Disable http proxy."
        unset http_proxy
        unset https_proxy
        SUExecCmd "echo unset http_proxy >> ${AGENT_HOME_ENV_FILE}"
        SUExecCmd "echo unset https_proxy >> ${AGENT_HOME_ENV_FILE}"
    fi
}

JudgeRandomNumType()
{
    DISABLE_TRUE_RANDOM_NUM=1
    ENABLE_TRUE_RANDOM_NUM=0

    CheckEntropyEnough
    if [ $? -eq 0 ]; then
        SUExecCmd "${AGENT_ROOT_PATH}/sbin/xmlcfg write Security disable_safe_random_number ${ENABLE_TRUE_RANDOM_NUM}"
    else
        SUExecCmd "${AGENT_ROOT_PATH}/sbin/xmlcfg write Security disable_safe_random_number ${DISABLE_TRUE_RANDOM_NUM}"
    fi
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