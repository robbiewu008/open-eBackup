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
#@function: check DB status, Four capacities need to be returned.
# 0 check if close, 1 check if open, 2 check if exists
USAGE="Usage: ./oraclecheckdbstatus.sh AgentRoot PID"

AGENT_ROOT_PATH=$1
PID=$2
PARAM_NUM=$3
IN_ORACLE_HOME=""
. "${AGENT_ROOT_PATH}/sbin/agent_sbin_func.sh"
. "${AGENT_ROOT_PATH}/sbin/oraclefunc.sh"

#********************************define these for the functions of agent_sbin_func.sh********************************
#for GetUserShellType
ORACLE_SHELLTYPE=
GRID_SHELLTYPE=
RDADMIN_SHELLTYPE=
#for Log
LOG_FILE_NAME="${LOG_PATH}/oraclecheckdbstatus.log"
#for GetOracleVersion
ORA_VERSION=
PREVERSION=
VERSION=
ORCLVESION="${STMP_PATH}/ORCV${PID}.txt"
SQLEXIT="${STMP_PATH}/EXITSQL${PID}.sql"
#for GetValue

# global var, for kill monitor
PARAM_CONTENT=`ReadInputParam`
test -z "$PARAM_CONTENT"              && ExitWithError "PARAM_FILE"
# oracle temp file
ORATMPINFO="${STMP_PATH}/ORATMPINFO${PID}.txt"

#********************************define these for the functions of agent_sbin_func.sh********************************

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
# 0 check if close, 1 check if open, 2 check if exists
CMDTYPE=`GetValue "${PARAM_CONTENT}" CheckType`

test "$DBNAME" = "${ERROR_PARAM_INVALID}"                      && ExitWithError "DBNAME"
test "$DBINSTANCE" = "${ERROR_PARAM_INVALID}"                  && ExitWithError "DBINSTANCE"
test "$DBUSER" = "${ERROR_PARAM_INVALID}"                      && ExitWithError "DBUSER"
test "$CMDTYPE" = "${ERROR_PARAM_INVALID}"                     && ExitWithError "CMDTYPE"

Log "PID=${PID};DBINSTANCE=${DBINSTANCE};DBName=${DBNAME};DBUSER=${DBUSER};CMDTYPE=$CMDTYPE"

test -z "${CMDTYPE}" && ExitWithErrorCode "CmdType is NULL"
test -z "${DBINSTANCE}" && ExitWithErrorCode "DBINSTANCE is NULL"

DeleteFile ${RESULT_FILE}


GetOracleUser
GetUserShellType ${ORA_DB_USER} ${ORA_GRID_USER}
GetOracleHomePath ${ORA_DB_USER}
GetOracleVersion ${ORA_DB_USER}
VERSION=`echo $PREVERSION | tr -d '.'`
GetOracleCluster
DBINSTANCE=`GetRealInstanceName ${DBINSTANCE}`
Log "Check cluster [DBISCLUSTER=${DBISCLUSTER}], DBINSTANCE=${DBINSTANCE}."

CheckDBOpen()
{
    # get instace status
    Log "Start to check oracle instance status."
    GetOracleInstanceStatus ${DBINSTANCE}
    RET_CODE=$?
    if [ "${RET_CODE}" -ne "0" ]; then
        Log "Get instance status failed."
        return ${RET_CODE}
    fi

    #STARTED - After STARTUP NOMOUNT
    #MOUNTED - After STARTUP MOUNT or ALTER DATABASE CLOSE
    #OPEN - After STARTUP or ALTER DATABASE OPEN
    #OPEN MIGRATE - After ALTER DATABASE OPEN { UPGRADE | DOWNGRADE }
    if [ ! "`RDsubstr $INSTANCESTATUS 1 4`" = "OPEN" ]; then
        Log "Instance status($INSTANCESTATUS) no open."
        return ${ERROR_INSTANCE_NOSTART}
    fi
    return 0
}

if [ "${CMDTYPE}" = "0" ]; then
    CheckDBClose
    exit $?
elif [ "${CMDTYPE}" = "1" ]; then
    CheckDBOpen
    exit $?
elif [ "${CMDTYPE}" = "2" ]; then
    CheckDBExists
    exit $?
elif [ "${CMDTYPE}" = "3" ]; then
    Log "Start to clear database for restore, mount, instance restore, ${DB_SID}."
    CheckDBClose
    RET=$?
    if [ $RET -eq 0 ]; then
        Log "${DBINSTANCE} is not running."
        exit 0
    fi
    
    ShutDownDB ${DBINSTANCE}
    Log "Clear database for restore, mount, instance restore successfully, ${DB_SID}."
    exit $?
else
    Log "Cmd Type ${CMDTYPE} is invalid."
    exit 1
fi

