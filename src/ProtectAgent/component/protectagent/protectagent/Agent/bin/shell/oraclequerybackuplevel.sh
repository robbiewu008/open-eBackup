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

#@dest:  query oracle backup level
#@date:  2021-0-09
#@authr: 
USAGE="Usage: ./oraclequerybackuplevel.sh AgentRoot PID"

AGENT_ROOT_PATH=$1
PID=$2
PARAM_NUM=$3
. "${AGENT_ROOT_PATH}/sbin/agent_sbin_func.sh"
. "${AGENT_ROOT_PATH}/sbin/oraclefunc.sh"
LOG_FILE_NAME="${LOG_PATH}/oraclequerybackuplevel.log"
PARAM_CONTENT=`ReadInputParam`
test -z "$PARAM_CONTENT"              && ExitWithError "PARAM_FILE"

ORATMPINFO="${STMP_PATH}/ORATMPINFO${PID}.txt"

DBINSTANCE=`GetValue "${PARAM_CONTENT}" InstanceName`
DBNAME=`GetValue "${PARAM_CONTENT}" AppName`
DBUSER=`GetValue "${PARAM_CONTENT}" UserName`
DBUSERPWD=`GetValue "${PARAM_CONTENT}" Password`
ASMINSTANCENAME=`GetValue "${PARAM_CONTENT}" ASMInstanceName`

test "$DBNAME" = "${ERROR_PARAM_INVALID}"                           && ExitWithError "DBNAME"
test "$DBINSTANCE" = "${ERROR_PARAM_INVALID}"                       && ExitWithError "DBINSTANCE"
test "$DBUSER" = "${ERROR_PARAM_INVALID}"                           && ExitWithError "DBUSER"
test "$ASMINSTANCENAME" = "${ERROR_PARAM_INVALID}"                  && ExitWithError "ASMINSTANCENAME"

if [ -z "${ASMINSTANCENAME}" ]; then
    ASMINSTANCENAME="+ASM"
fi
ASMUSER=`GetValue "${PARAM_CONTENT}" ASMUserName`
ASMUSERPWD=`GetValue "${PARAM_CONTENT}" ASMPassword`
test "$ASMUSER" = "${ERROR_PARAM_INVALID}"                           && ExitWithError "ASMUSER"
LEVEL=`GetValue "${PARAM_CONTENT}" Level`
BACKUP=`GetValue "${PARAM_CONTENT}" DataPath`
test "$LEVEL" = "${ERROR_PARAM_INVALID}"                           && ExitWithError "LEVEL"
test "$BACKUP" = "${ERROR_PARAM_INVALID}"                          && ExitWithError "BACKUP"

BACKUP=`RedirectBackPath ${BACKUP}`

Log "INNERPID=${PID};DBINSTANCE=${DBINSTANCE};DBName=${DBNAME};DBUSER=${DBUSER};\
ASMInst=${ASMSIDNAME};ASMUSER=${ASMUSER};BACKUP=$BACKUP;level=${LEVEL};"

MainBackupPath=`echo ${BACKUP} | $MYAWK -F ";" '{print $1}'`
test -z "${MainBackupPath}" && ExitWithError "main data path"
ADDITIONAL="${MainBackupPath}/additional"
LOG_IS_BACKED_UP="$LOG_IS_VALID and name like '$MainBackupPath/log/arch_%'"

GetOracleUser ${ASMSIDNAME}
GetUserShellType ${ORA_DB_USER} ${ORA_GRID_USER}
GetOracleBasePath ${ORA_DB_USER}
GetOracleHomePath ${ORA_DB_USER}
GetOracleVersion ${ORA_DB_USER}
VERSION=`echo $PREVERSION | tr -d '.'`
GetOracleCluster
DBINSTANCE=`GetRealInstanceName ${DBINSTANCE}`
Log "Check cluster [DBISCLUSTER=${DBISCLUSTER}], DBINSTANCE=${DBINSTANCE}."


if [ "$LEVEL" != "0" ]; then
    if [[ "$LEVEL" != "1" ]] && [[ "$LEVEL" != "2" ]]; then
        LEVEL=0
        Log "set level=0 full backup"
    fi
fi

GetDBIDName ${DBINSTANCE}
if [ -f "$ADDITIONAL/dbinfo" ]; then
    OLDDBID=`cat $ADDITIONAL/dbinfo | $MYAWK -F ";" '{print $1}'`
    if [ $DBID -ne $OLDDBID ]; then
        Log "DBID is different from the last backup  must all backup."
        LEVEL=0
    fi
fi


echo "BackupLevel;${LEVEL}" >> "${RESULT_FILE}"
Log "BackupLevel; ${LEVEL}"

#chmod 640 RESULT_FILE
if [ -f  "${RESULT_FILE}" ]; then
    chmod 640 "${RESULT_FILE}"
fi

Log "query oracle backup level success."
exit 0