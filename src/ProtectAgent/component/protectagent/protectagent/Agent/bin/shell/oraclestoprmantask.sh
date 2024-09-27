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
#@function: stop rman task

AGENT_ROOT_PATH=$1
PID=$2
PARAM_NUM=$3
. "${AGENT_ROOT_PATH}/sbin/agent_sbin_func.sh"
. "${AGENT_ROOT_PATH}/sbin/oraclefunc.sh"

#********************************define these for local script********************************
#for GetUserShellType
ORACLE_SHELLTYPE=
GRID_SHELLTYPE=
RDADMIN_SHELLTYPE=
#for log
LOG_FILE_NAME="${LOG_PATH}/oraclestoprmantask.log"
#for GetOracleVersion
ORA_VERSION=
PREVERSION=
VERSION=
ORCLVESION="${STMP_PATH}/ORCV${PID}.txt"
SQLEXIT="${STMP_PATH}/EXITSQL${PID}.sql"
ORATMPINFO="${STMP_PATH}/ORATMPINFO${PID}.txt"

# global var, for kill monitor
PARAM_CONTENT=`ReadInputParam`
test -z "$PARAM_CONTENT"              && ExitWithError "PARAM_FILE"
#********************************define these for local script********************************
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
RmanTaskType=`GetValue "${PARAM_CONTENT}" RmanTaskType`
TaskInnerID=`GetValue "${PARAM_CONTENT}" TaskInnerID`


test "$DBNAME" = "${ERROR_PARAM_INVALID}"                           && ExitWithError "DBNAME"
test "$DBINSTANCE" = "${ERROR_PARAM_INVALID}"                       && ExitWithError "DBINSTANCE"
test "$DBUSER" = "${ERROR_PARAM_INVALID}"                           && ExitWithError "DBUSER"
test "$RmanTaskType" = "${ERROR_PARAM_INVALID}"                     && ExitWithError "RmanTaskType"
test "$TaskInnerID" = "${ERROR_PARAM_INVALID}"                      && ExitWithError "TaskInnerID"

Log "PID=${PID};DBINSTANCE=${DBINSTANCE};DBName=${DBNAME};\
DBUSER=${DBUSER};RmanTaskType=$RmanTaskType;TaskInnerID=${TaskInnerID}"

GetOracleUser
GetUserShellType ${ORA_DB_USER} ${ORA_GRID_USER}
GetOracleHomePath ${ORA_DB_USER}
GetOracleVersion ${ORA_DB_USER}
VERSION=`echo $PREVERSION | tr -d '.'`
GetOracleCluster
DBINSTANCE=`GetRealInstanceName ${DBINSTANCE}`
Log "Check cluster [DBISCLUSTER=${DBISCLUSTER}], DBINSTANCE=${DBINSTANCE}."
GetRmanPIDSQL="${STMP_PATH}/GetRmanPID${PID}.sql"
GetRmanPIDRST="${STMP_PATH}/GetRmanPIDRST${PID}.txt"

GetRmanPID()
{
    echo "set linesize 999" > ${GetRmanPIDSQL}
    if [ $RmanTaskType -eq 0 ]; then
        echo "SELECT p.spid FROM v\$process p,v\$session s WHERE p.addr = s.paddr AND client_info LIKE '%rman%eBackup%';" >> ${GetRmanPIDSQL}
    elif [ $RmanTaskType -eq 1 ]; then
        echo "SELECT p.spid FROM v\$process p,v\$session s WHERE p.addr = s.paddr AND client_info LIKE '%rman%eBackup%';" >> ${GetRmanPIDSQL}
    elif [ $RmanTaskType -eq 2 ]; then
        echo "SELECT p.spid FROM v\$process p,v\$session s WHERE p.addr = s.paddr AND client_info LIKE '%rman%eRestore%';" >> ${GetRmanPIDSQL}
    elif [ $RmanTaskType -eq 3 ]; then
        echo "SELECT p.spid FROM v\$process p,v\$session s WHERE p.addr = s.paddr AND client_info LIKE '%rman%eInsrestore%';" >> ${GetRmanPIDSQL}
    fi
    echo "exit;" >> ${GetRmanPIDSQL}
    OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${GetRmanPIDSQL}" "${GetRmanPIDRST}" "${DBINSTANCE}" 30 1 "${ORA_DB_USER}" >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ "$RET_CODE" -ne "0" ]; then
        Log "Get rman process id failed, ret=$RET_CODE, msg=`cat ${GetRmanPIDRST}`."
        DeleteFile "${GetRmanPIDSQL}" "${GetRmanPIDRST}"
        exit ${RET_CODE}
    fi
    DeleteFile "${GetRmanPIDSQL}"
    if [ -s ${GetRmanPIDRST} ]; then
        Log "Get Rman PID success."
        echo 1
    else
        Log "get Rman PID is empty."
        echo 0
    fi
    return 0
}
ISProcessExist=`GetRmanPID`
while [ "${ISProcessExist}" = "1" ]; do
    for line in `cat ${GetRmanPIDRST}`; do
        Log "kill process: `ps -ef | grep ${line} | grep -v grep`"
        kill -9 ${line} >> "${LOG_FILE_NAME}" 2>&1
    done
    sleep 10
    ISProcessExist=`GetRmanPID`
done
DeleteFile ${GetRmanPIDRST}

sleep 10
KillRman()
{
    local PID=`ps -ef | grep ${TaskInnerID}.sql | grep -v "grep" | sed -n '$p' | ${MYAWK} '{print $2}'`
    Log "get Rman PID is ${PID}."
    if [ ! -z "${PID}" ]; then
        rmanPID=`pstree -p ${PID} | grep "rman" | sed -r "s/.*rman\((\w*)\).*/\1/"`
        if [ $? -eq 0 ] && [ ! -z "${rmanPID}" ]; then
            kill -9 ${rmanPID}
        else
            kill -9 ${PID}
        fi
    fi
}
KillRman

sleep 10
KillTaskShell()
{
    if [ $RmanTaskType -eq 0 ]; then
        local PID=`ps -ef | grep oraclenativearchiveback.sh | grep -v "grep" | grep ${TaskInnerID}  | sed -n '$p' | ${MYAWK} '{print $2}'`
    elif [ $RmanTaskType -eq 1 ]; then
        local PID=`ps -ef | grep oraclenativebackup.sh | grep -v "grep" | grep ${TaskInnerID}  | sed -n '$p' | ${MYAWK} '{print $2}'`
    elif [ $RmanTaskType -eq 2 ]; then
        local PID=`ps -ef | grep oraclenativerestore.sh | grep -v "grep" | grep ${TaskInnerID}  | sed -n '$p' | ${MYAWK} '{print $2}'`
    elif [ $RmanTaskType -eq 3 ]; then
        local PID=`ps -ef | grep oraclenativeinstrestore.sh | grep -v "grep" | grep ${TaskInnerID}  | sed -n '$p' | ${MYAWK} '{print $2}'`
    fi
    Log "get task PID is ${PID}."
    if [ ! -z "${PID}" ]; then
        kill -9 ${PID}
    fi
}
KillTaskShell

Log "RMAN task stopped successfully."
exit 0