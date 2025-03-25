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
#@function: start oracle with livemount.
USAGE="Usage: ./oraclenativeclivemount.sh AgentRoot PID"

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
LOG_FILE_NAME="${LOG_PATH}/oraclenativeclivemount.log"
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
ASMUSER=`GetValue "${PARAM_CONTENT}" ASMUserName`
ASMUSERPWD=`GetValue "${PARAM_CONTENT}" ASMPassword`
ASMSIDNAME=`GetValue "${PARAM_CONTENT}" ASMInstanceName`

test "$DBNAME" = "${ERROR_PARAM_INVALID}"                       && ExitWithError "DBNAME"
test "$DBINSTANCE" = "${ERROR_PARAM_INVALID}"                   && ExitWithError "DBINSTANCE"
test "$DBUSER" = "${ERROR_PARAM_INVALID}"                       && ExitWithError "DBUSER"
test "$ASMUSER" = "${ERROR_PARAM_INVALID}"                      && ExitWithError "ASMUSER"
test "$ASMSIDNAME" = "${ERROR_PARAM_INVALID}"                   && ExitWithError "ASMSIDNAME"

Log "PID=${PID};DBINSTANCE=${DBINSTANCE};DBName=${DBNAME};DBUSER=${DBUSER};ASMInst=${ASMSIDNAME};ASMUSER=${ASMUSER};IN_ORACLE_HOME=$IN_ORACLE_HOME"

StopRACInstance()
{
    CheckInstAuth "${DBINSTANCE}"
    AuthType=$?
    if [ ${AuthType} -eq 1 ]; then
        su - ${ORA_DB_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV}srvctl stop database -d ${DBNAME}" >> "${LOG_FILE_NAME}" 2>&1
        su - ${ORA_DB_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV}srvctl remove database -d ${DBNAME} -f" >> "${LOG_FILE_NAME}" 2>&1
    else
        StopSingleInstance
    fi
}

StopSingleInstance()
{
    SHUTDOWNSCRIPT="${STMP_PATH}/ShutdownSql$PID.sql"
    SHUTDOWNRST="${STMP_PATH}/ShutdownTmp$PID.txt"
    local HAVEPROCESS=`ps -aef | grep -v grep | sed 's/ *$//g' | grep "ora_...._$DBINSTANCE$" |wc -l`
    if [ "$HAVEPROCESS" -ne "0" ]; then
        # **************************Create ShutDown DataBase Sql****************************************
        CreateShutdownSql $SHUTDOWNSCRIPT

        Log "Exec SQL to shutdown database."
        OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${SHUTDOWNSCRIPT}" "${SHUTDOWNRST}" "${DBINSTANCE}" 300 0 >> "${LOG_FILE_NAME}" 2>&1
        RET_CODE=$?
        if [ "${RET_CODE}" -eq "${ERROR_ORACLE_NOT_OPEN}" ]; then
            Log "Shutdown database(${DBINSTANCE}) have already shutdown."
        elif [ "${RET_CODE}" -ne "0" ]; then
            Log "Shutdown database(${DBINSTANCE}) file failed, errorcode=${RET_CODE}, error:`cat ${SHUTDOWNRST}`."
            exit ${RET_CODE}
        else
            Log "shutdown database successful."
        fi
        DeleteFile "${SHUTDOWNSCRIPT}" "${SHUTDOWNRST}"
    else
        Log "The Instance isn't Started."
    fi
}

GetOracleUser ${ASMSIDNAME}
# get user shell type
GetUserShellType ${ORA_DB_USER} ${ORA_GRID_USER}

# get oracle base path
GetOracleBasePath ${ORA_DB_USER}

# get oracle home path
GetOracleHomePath ${ORA_DB_USER}

# get Oracle version
GetOracleVersion ${ORA_DB_USER}
VERSION=`echo $PREVERSION | tr -d '.'`

# get cluster configuration
GetOracleCluster

# check parameter valid
test -z "$DBINSTANCE"  && ExitWithError "oracle instance name"

# stop oracle database
if [ "${DBISCLUSTER}" = "1" ]; then
    # get ORA_CRS_HOME
    GetORA_CRS_HOME ${VERSION} ${ORA_GRID_USER}
    StopRACInstance
elif [ ${DBISCLUSTER} -eq 2 ]; then
    StopSingleInstance
    su - ${ORA_DB_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV}srvctl remove database -d ${DBNAME} -f" >> "${LOG_FILE_NAME}" 2>&1
else
    StopSingleInstance
fi

# remove oracle admin database folder
if [ "${IN_ORACLE_BASE}" != "" ]; then
    DB_NAME_LOWER=`echo ${DBNAME} | tr '[A-Z]' '[a-z]'`
    if [ -d "${IN_ORACLE_BASE}/admin/${DB_NAME_LOWER}" ]; then
        su - ${ORA_DB_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV}mv -f \"${IN_ORACLE_BASE}/admin/${DB_NAME_LOWER}\" \"${IN_ORACLE_BASE}/admin/${DB_NAME_LOWER}_`date '+%Y%m%d%H%M%S'`\""
    fi
fi

if [ ! -z ${IN_ORACLE_HOME} ]; then
    if [ -f "${IN_ORACLE_HOME}/dbs/init${DBINSTANCE}.ora" ]; then
        su - ${ORA_DB_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV}rm -rf ${IN_ORACLE_HOME}/dbs/init${DBINSTANCE}.ora"
    fi
fi

exit 0