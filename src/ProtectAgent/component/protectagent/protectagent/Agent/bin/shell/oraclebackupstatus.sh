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

############################################################################################
#program name:          oraclebackupstatus.sh     
#function:              Oracle database message collect
#author:                
#time:                  
#function and description:  
# function              description
# rework:               First Programming
# author:
# time:
# explain:
############################################################################################
AGENT_ROOT_PATH=$1
PID=$2
PARAM_NUM=$3
IN_ORACLE_HOME=""
. "${AGENT_ROOT_PATH}/sbin/agent_sbin_func.sh"
. "${AGENT_ROOT_PATH}/sbin/oraclefunc.sh"

#===define these for the functions of agent_sbin_func.sh and oraclefunc.sh===
#for GetUserShellType
ORACLE_SHELLTYPE=
GRID_SHELLTYPE=
RDADMIN_SHELLTYPE=

#for log
LOG_FILE_NAME="${LOG_PATH}/oraclebackupstatus.log"
SQLQUERYBACKUPSTATUS="${STMP_PATH}/QueryBackupStatus${PID}.sql"
SQLQUERYBACKUPSTATUSRST="${STMP_PATH}/QueryBackupStatusRST${PID}.txt"
SQLQUERYBACKUPPROGRESS="${STMP_PATH}/QueryBackupProgress${PID}.sql"
SQLQUERYBACKUPPROGRESSRST="${STMP_PATH}/QueryBackupProgressRST${PID}.txt"
#for GetOracleVersion
ORA_VERSION=
PREVERSION=
VERSION=
ORCLVESION="${STMP_PATH}/ORCV${PID}.txt"
SQLEXIT="${STMP_PATH}/EXITSQL${PID}.sql"
#for GetValue
PARAM_CONTENT=`ReadInputParam`
test -z "$PARAM_CONTENT"              && ExitWithError "PARAM_FILE"
# oracle temp file
ORATMPINFO="${STMP_PATH}/ORATMPINFO${PID}.txt"

# define for the function GetOraUserByInstName
ORA_DB_USER=
ORA_GRID_USER=

CheckOracleIsInstall
RST=$?
if [ $RST -ne 0 ]; then
    exit $RST
fi

#############################################################
DBNAME=`GetValue "${PARAM_CONTENT}" AppName`
DBINSTANCE=`GetValue "${PARAM_CONTENT}" InstanceName`
DBUSER=`GetValue "${PARAM_CONTENT}" UserName`
DBUSERPWD=`GetValue "${PARAM_CONTENT}" Password`
ASMUSER=`GetValue "${PARAM_CONTENT}" ASMUserName`
ASMUSERPWD=`GetValue "${PARAM_CONTENT}" ASMPassword`
TASKQUERYTYPE=`GetValue "${PARAM_CONTENT}" queryTaskMode`
ASMSIDNAME=`GetValue "${PARAM_CONTENT}" ASMInstanceName`

test "$DBNAME" = "${ERROR_PARAM_INVALID}"                      && ExitWithError "DBNAME"
test "$DBINSTANCE" = "${ERROR_PARAM_INVALID}"                  && ExitWithError "DBINSTANCE"
test "$DBUSER" = "${ERROR_PARAM_INVALID}"                      && ExitWithError "DBUSER"
test "$ASMUSER" = "${ERROR_PARAM_INVALID}"                     && ExitWithError "ASMUSER"
test "$TASKQUERYTYPE" = "${ERROR_PARAM_INVALID}"               && ExitWithError "TASKQUERYTYPE"
test "$ASMSIDNAME" = "${ERROR_PARAM_INVALID}"                  && ExitWithError "ASMSIDNAME"


if [ -z "${ASMSIDNAME}" ]; then
    ASMSIDNAME="+ASM"
fi

Log "PID=${PID};DBUSER=$DBUSER;DBNAME=$DBNAME;DBINSTANCE=$DBINSTANCE;TASKQUERYTYPE=${TASKQUERYTYPE}"

GetOracleUser ${ASMSIDNAME}
# get user shell type
GetUserShellType ${ORA_DB_USER} ${ORA_GRID_USER}

#get oracle home path
GetOracleHomePath ${ORA_DB_USER}

#get Oracle version
GetOracleVersion ${ORA_DB_USER}
VERSION=`echo $PREVERSION | tr -d '.'`

# get cluster configuration
GetOracleCluster

# get really instance name when cluser or single
GetRealInstanceName "${DBINSTANCE}" "${DBISCLUSTER}"
if [ $? -ne 0 ]; then
    Log "Get Really instance name failed, instance name ${DBINSTANCE}."
fi

CrtQueryBackupSpeedSql()
{
    echo "set linesize 300" > "$1"
    echo "COL OUTPUT_BYTES_PER_SEC_DISPLAY FORMAT a50" >> "$1"
    echo "COL SIZE_KB FORMAT 999999999999" >> "$1"
    echo "select p.* from(select OUTPUT_BYTES_PER_SEC_DISPLAY, ROUND(OUTPUT_BYTES/1024,0) SIZE_KB from v\$rman_backup_job_details \
    where COMMAND_ID='ProtectAgent_Backup' order by SESSION_KEY desc)p where rownum = 1;" >> "$1"
    echo "exit" >> "$1"
}

CrtQueryBackupProgressSql()
{
    echo "set linesize 300;" > "$1"
    echo "select p.* from(SELECT SOFAR, TOTALWORK, ROUND(SOFAR/TOTALWORK*100,2) progress 
    FROM V\$SESSION_LONGOPS where opname like 'RMAN%aggregate%input%' and totalwork<>0 and SOFAR <> TOTALWORK 
    order by start_time desc)p where rownum = 1;" >> "$1"
    echo "exit" >> "$1"
}

# query speed
QueryBackupSpeed()
{
    CrtQueryBackupSpeedSql ${SQLQUERYBACKUPSTATUS}
    Log "Exec SQL to query backup speed."
    OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${SQLQUERYBACKUPSTATUS}" "${SQLQUERYBACKUPSTATUSRST}" "${DBINSTANCE}" 30 1 "${ORA_DB_USER}" >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ ${RET_CODE} -ne 0 ]; then
        Log "Query for Rman backup speed failed, ret=${RET_CODE}"
        DeleteFile "${SQLQUERYBACKUPSTATUSRST}" "${SQLQUERYBACKUPSTATUS}"
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi
    SPEED=`cat ${SQLQUERYBACKUPSTATUSRST} | sed -n '1p' | $MYAWK '{print $1}'`
    BackupSize=`cat ${SQLQUERYBACKUPSTATUSRST} | sed -n '1p' | $MYAWK '{print $2}'`
    Log "Get task speed is ${SPEED}, total size is ${BackupSize}(byte)."
    DeleteFile  "${SQLQUERYBACKUPSTATUSRST}" "${SQLQUERYBACKUPSTATUS}"
}

# query progress
QueryTaskProgress()
{
    CrtQueryBackupProgressSql ${SQLQUERYBACKUPPROGRESS}
    Log "Exec SQL to query backup progress."
    OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${SQLQUERYBACKUPPROGRESS}" "${SQLQUERYBACKUPPROGRESSRST}" "${DBINSTANCE}" 30 1 "${ORA_DB_USER}" >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ ${RET_CODE} -ne 0 ]; then
        Log "Query for Rman task progress failed, ret=${RET_CODE}"
        DeleteFile  "${SQLQUERYBACKUPPROGRESSRST}" "${SQLQUERYBACKUPPROGRESS}"
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi
    SOFAR=`cat ${SQLQUERYBACKUPPROGRESSRST} | $MYAWK '{print $1}'`
    TOTALWORK=`cat ${SQLQUERYBACKUPPROGRESSRST} | $MYAWK '{print $2}'`
    PROGRESS=`cat ${SQLQUERYBACKUPPROGRESSRST} | $MYAWK '{print $3}'`
    DeleteFile "${SQLQUERYBACKUPPROGRESSRST}" "${SQLQUERYBACKUPPROGRESS}"
    Log "get oracle task progress succ."
}

QueryTaskProgress

QueryRestoreSpeed()
{
    echo "set linesize 100;" > ${SQLQUERYBACKUPSTATUS}
    echo "COL SIZE_KB FORMAT 999999999999999;"  >> ${SQLQUERYBACKUPSTATUS}
    echo "COL OPEN_TIME FORMAT a20;"  >> ${SQLQUERYBACKUPSTATUS}
    echo "select ROUND(nvl(BYTES,0)/1024,0) SIZE_KB, to_char(OPEN_TIME, 'YYYY-MM-DD_HH24:MI:SS') \
    from v\$backup_async_io where type='OUTPUT' order by OPEN_TIME;" >> ${SQLQUERYBACKUPSTATUS}
    echo "exit" >> ${SQLQUERYBACKUPSTATUS}
    OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${SQLQUERYBACKUPSTATUS}" "${SQLQUERYBACKUPSTATUSRST}" "${DBINSTANCE}" 30 1 "${ORA_DB_USER}" >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ ${RET_CODE} -ne 0 ]; then
        Log "Query for Rman restore speed failed, ret=${RET_CODE}"
        DeleteFile "${SQLQUERYBACKUPSTATUSRST}" "${SQLQUERYBACKUPSTATUS}" "${ERRDETAIL_FILE}"
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi
    sizes=`cat $SQLQUERYBACKUPSTATUSRST | $MYAWK  '{print $1}'`
    totalSize=0
    for size in ${sizes}; do
        totalSize=`echo ${totalSize} ${size} | $MYAWK '{print ($1 + $2)}'`
    done
    startTime=`cat $SQLQUERYBACKUPSTATUSRST | $MYAWK '{print $2}' | $MYAWK 'NR==1'`
    startTime=`AddUnixTimestamp ${startTime}`
    endTime=`date +%s`
    SPEED="`echo ${totalSize} ${endTime} ${startTime} | $MYAWK '{printf ("%0.2f\n", $1 / ($2 - $3) / 1024)}'`MB"
    HISTORYSPEED=${SPEED}
    Log "startTime=${startTime};endTime=${endTime};totalSize=${totalSize};SPEED=${SPEED}"
    DeleteFile  "${SQLQUERYBACKUPSTATUSRST}" "${SQLQUERYBACKUPSTATUS}"
}

# restore task TASKQUERYTYPE is not null, no need to query task speed
if [ ${TASKQUERYTYPE} -eq 0 ]; then
    QueryBackupSpeed
elif [ ${TASKQUERYTYPE} -eq 1 ]; then
    QueryRestoreSpeed
fi
Log "get task speed is ${SPEED}, SOFAR is ${SOFAR}, TOTALWORK is ${TOTALWORK}, HISTORYSPEED is ${HISTORYSPEED}, BackupSize is ${BackupSize}."
echo "${SPEED};${SOFAR};${TOTALWORK};${PROGRESS};${HISTORYSPEED};${BackupSize}" > "${RESULT_FILE}"

#chmod 640 RESULT_FILE
if [ -f "${RESULT_FILE}" ]; then
    chmod 640 "${RESULT_FILE}"
fi
exit 0