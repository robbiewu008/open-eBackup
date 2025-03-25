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

#@dest:  query oracle tablespace list for oracle 
#@date:  2020-07-04
#@authr: 
USAGE="Usage: ./oraclequerytablespace.sh AgentRoot PID"

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
LOG_FILE_NAME="${LOG_PATH}/oraclequerytablespace.log"
#for GetOracleVersion
ORA_VERSION=
PREVERSION=
VERSION=
SQLEXIT="${STMP_PATH}/EXITSQL${PID}.sql"
# oracle temp file
#STARTED - After STARTUP NOMOUNT
#MOUNTED - After STARTUP MOUNT or ALTER DATABASE CLOSE
#OPEN - After STARTUP or ALTER DATABASE OPEN
#OPEN MIGRATE - After ALTER DATABASE OPEN { UPGRADE | DOWNGRADE }
INSTANCESTATUS=""
DBUSER=""
DBUSERPWD=""
#********************************define these for the functions of agent_sbin_func.sh********************************

#********************************define these for local script********************************
QUERYCRIPT="$STMP_PATH/Query$PID.sql"
QUERYCRIPTRST="$STMP_PATH/Query$PID.txt"
PARAM_CONTENT=`ReadInputParam`
test -z "$PARAM_CONTENT"              && ExitWithError "PARAM_FILE"
#********************************define these for local script********************************

#############################################################
DBINSTANCE=`GetValue "${PARAM_CONTENT}" InstanceName`
DBNAME=`GetValue "${PARAM_CONTENT}" AppName`
DBUSERL=`GetValue "${PARAM_CONTENT}" UserName`
DBUSER=`echo "$DBUSERL" | tr '[A-Z]' '[a-z]'`
DBUSERPWD=`GetValue "${PARAM_CONTENT}" Password`
IN_ORACLE_HOME=`GetValue "${PARAM_CONTENT}" OracleHome`

test "$DBNAME" = "${ERROR_PARAM_INVALID}"                           && ExitWithError "DBNAME"
test "$DBINSTANCE" = "${ERROR_PARAM_INVALID}"                       && ExitWithError "DBINSTANCE"
test "$DBUSER" = "${ERROR_PARAM_INVALID}"                           && ExitWithError "DBUSER"
test "$DBUSERL" = "${ERROR_PARAM_INVALID}"                          && ExitWithError "DBUSERL"
test "$IN_ORACLE_HOME" = "${ERROR_PARAM_INVALID}"                   && ExitWithError "IN_ORACLE_HOME"

Log "DBNAME=$DBNAME;DBUSER=$DBUSER;DBINSTANCE=$DBINSTANCE;PID=${PID};IN_ORACLE_HOME=$IN_ORACLE_HOME"

GetOracleUser
# get user shell type
GetUserShellType ${ORA_DB_USER} ${ORA_GRID_USER}

#get Oracle version
GetOracleVersion ${ORA_DB_USER}
VERSION=`echo $PREVERSION | tr -d '.'`

GetDatabaseUUID()
{
    INST_NAME=$1
    GetDatabaseUUIDSQL="${STMP_PATH}/GetDatabaseUUID${PID}.sql"
    GetDatabaseUUIDRST="${STMP_PATH}/GetDatabaseUUID${PID}.txt"
    echo "set linesize 999;" > "$GetDatabaseUUIDSQL"
    echo "select dbid from v\$database;" >> "$GetDatabaseUUIDSQL"
    echo "exit" >> "$GetDatabaseUUIDSQL"
    OracleExeSql "${DBUSER}" "${DBUSERPWD}" ${GetDatabaseUUIDSQL} ${GetDatabaseUUIDRST} ${INST_NAME} 30 1 ${ORA_DB_USER} >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ ${RET_CODE} -ne 0 ]; then
        Log "GetDatabaseName failed,ret=${RET_CODE}"
        DeleteFile "${GetDatabaseUUIDSQL}" "${GetDatabaseUUIDRST}" "${ERRDETAIL_FILE}"
        echo ""
    else
        dbUUID=`cat ${GetDatabaseUUIDRST}`
        dbName=`echo ${dbName} | tr [A-Z] [a-z]`
        DeleteFile "${GetDatabaseUUIDSQL}" "${GetDatabaseUUIDRST}"
        echo ${dbUUID}
    fi
}
DB_UUID=`GetDatabaseUUID ${DBINSTANCE}`

if [ "${VERSION}" -ge "121" ]; then
    echo "set linesize 600" > "${QUERYCRIPT}"
    echo "col TSNAME for a30" >> "${QUERYCRIPT}"
    echo "col FILENAME for a113" >> "${QUERYCRIPT}"
    echo "col CONNAME for a30" >> "${QUERYCRIPT}"
    echo "select 'NOCDB' CONNAME, '*', C.NAME TSNAME, '*', A.FILE#, '*', A.NAME FILENAME from v\$datafile A, v\$tablespace C where A.CON_ID=0 and A.TS#=C.TS# order by CONNAME, TSNAME;" >> "${QUERYCRIPT}"
    echo "select 'CDB' CONNAME, '*', C.NAME TSNAME, '*', A.FILE#, '*', A.NAME FILENAME from v\$datafile A, v\$tablespace C where A.CON_ID=1 and A.TS#=C.TS# order by CONNAME, TSNAME;" >> "${QUERYCRIPT}"
    echo "select B.NAME CONNAME, '*', C.NAME TSNAME, '*', A.FILE#, '*', A.NAME FILENAME from v\$datafile A, v\$pdbs B, v\$tablespace C where A.TS#=C.TS# and A.CON_ID=B.CON_ID order by CONNAME, TSNAME;" >> "${QUERYCRIPT}"
else
    echo "set linesize 600" > "${QUERYCRIPT}"
    echo "col TSNAME for a30" >> "${QUERYCRIPT}"
    echo "col FILENAME for a113" >> "${QUERYCRIPT}"
    echo "col CONNAME for a30" >> "${QUERYCRIPT}"
    echo "select 'NOCDB' CONNAME, '*', C.NAME TSNAME, '*', A.FILE#, '*', A.NAME FILENAME from v\$datafile A, v\$tablespace C where A.TS#=C.TS# order by TSNAME;" >> "${QUERYCRIPT}"
fi
echo "exit;" >> ${QUERYCRIPT}
OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${QUERYCRIPT}" "${QUERYCRIPTRST}" "${DBINSTANCE}" 30 1 "${ORA_DB_USER}" >> "${LOG_FILE_NAME}" 2>&1
RET_CODE=$?
if [ "$RET_CODE" -ne "0" ]; then
    Log "Get oracle tablesspace file list failed, ret=$RET_CODE"
    DeleteFile "${QUERYCRIPT}" "${QUERYCRIPTRST}"
    exit ${RET_CODE}
fi


DeleteFile "${RESULT_FILE}"
touch "${RESULT_FILE}"
sort -n "${QUERYCRIPTRST}" | uniq  > "${QUERYCRIPT}"
while read line
do
    [ -z "$line" ] && continue
    result=`echo "$line" | $MYAWK -F "*" '{print $1,$2,$3,$4}' | $MYAWK -v OFS=";" '{print $1,$2,$3,$4}'`
    echo "tablespace;${result}" >> "${RESULT_FILE}"
done < "$QUERYCRIPT"
echo "dbid;${DB_UUID}" >> "${RESULT_FILE}"

DeleteFile "${QUERYCRIPT}" "${QUERYCRIPTRST}"

Log "query oracle table space file list success."

#chmod 640 RESULT_FILE
if [ -f  "${RESULT_FILE}" ]; then
    chmod 640 "${RESULT_FILE}"
fi

exit 0
