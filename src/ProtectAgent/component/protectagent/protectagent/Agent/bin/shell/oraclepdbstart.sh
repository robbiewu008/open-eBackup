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

############################################################################################
#program name:          oraclepdbinfo.sh     
#function:              Stat. oracle portfolio information,including
#                       the path of database, total size and free size
#                       and tablespace name,total size and free size
#author:                
############################################################################################

USAGE="Usage: ./oraclepdbinfo.sh AgentRoot PID"

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
LOG_FILE_NAME="${LOG_PATH}/oraclepdbstart.log"
#for GetOracleVersion
ORA_VERSION=
PREVERSION=
VERSION=
ORCLVESION="${STMP_PATH}/ORCV${PID}.txt"
SQLEXIT="${STMP_PATH}/EXITSQL${PID}.sql"
#for GetOracleInstanceStatus
CHECK_INSTANCE_STATUS="${STMP_PATH}/CheckInstanceStatus${PID}.sql"

CHECK_PDB_LIST="${STMP_PATH}/CheckPDBList${PID}.sql"
CHECK_PDB_LISTRST="${STMP_PATH}/CheckPDBListRST${PID}.txt"

ORCLVESION="${STMP_PATH}/ORCV${PID}.txt"

QUERYCDBSCRIPT="$STMP_PATH/QueryCDB${PID}.sql"
QUERYCDBSCRIPTRST="${STMP_PATH}/QueryCDBRST${PID}.txt"

START_PDB="${STMP_PATH}/StartPDB${PID}.sql"
START_PDBRST="${STMP_PATH}/StartPDBRST${PID}.txt"

INSTANCESTATUS=""
FIND_PDB_CONID=
FIND_PDB_STATUS=
FIND_PDB_NAME=

#********************************define these for local script********************************
DBINSTANCE=""
DBUSER=""
DBUSERPWD=""
PDBNAME=""

# global variable
RST_SEPARATOR=";"

PARAM_FILE=`ReadInputParam`
test -z "$PARAM_FILE"              && ExitWithError "PARAM_FILE"
RESULT_TMP_FILE="${RESULT_FILE}.tmp"
touch "${RESULT_TMP_FILE}"
touch "${RESULT_FILE}"

# ************************** Find PDB ***********************************
CheckPdbNameExist()
{
    local ORA_INSTANCENAME=$1
    local ORA_PDB_NAME=$2
    if [ "${ORA_PDB_NAME}" = "" ]
    then
        Log "PDB name is NUll"
        return 1
    fi
    
    CHECK_PDB_LIST="${STMP_PATH}/CheckPDBList${PID}.sql"
    CHECK_PDB_LISTRST="${STMP_PATH}/CheckPDBListRST${PID}.txt"
    touch ${CHECK_PDB_LIST}
    touch ${CHECK_PDB_LISTRST}
    
    echo "select name from v\$pdbs where NAME='${ORA_PDB_NAME}';" > "${CHECK_PDB_LIST}"
    echo "exit" >> "${CHECK_PDB_LIST}"
    
    Log "Exec SQL to check PDB exist."
    OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${CHECK_PDB_LIST}" "${CHECK_PDB_LISTRST}" "${ORA_INSTANCENAME}" 30 0 >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ ${RET_CODE} -ne 0 ]
    then
        DeleteFile "${CHECK_PDB_LIST}" "${CHECK_PDB_LISTRST}"
        exit ${RET_CODE}
    else  
        tmpPDBNAME=`sed -n '/----------/,/^ *$/p' "${CHECK_PDB_LISTRST}" | sed -e '/----------/d' -e '/^ *$/d'`
        if [ "${tmpPDBNAME}" = "${ORA_PDB_NAME}" ] #if server put the pdbname is not NULL, just get the info of this pdb
        then
            Log "ORA_PDB_NAME: ${ORA_PDB_NAME}, tmpPDBNAME: ${tmpPDBNAME}, get it."
            DeleteFile "${CHECK_PDB_LIST}" "${CHECK_PDB_LISTRST}"
            return 0
        fi
        DeleteFile "${CHECK_PDB_LIST}" "${CHECK_PDB_LISTRST}"
    fi

    return 1
}

# ************************** Start PDB ***********************************
StartThisOraclePDB()
{
    local ORA_INSTANCENAME=$1
    local ORA_PDB_NAME=$2
    if [ "${ORA_PDB_NAME}" = "" ]
    then
        DeleteFile "${START_PDB}" "${START_PDBRST}" "${RESULT_TMP_FILE}"
        return 1
    fi
    if [ "${ORA_PDB_NAME}" = "PDB\$SEED" ]
    then
        Log "Cannot start SEEDPDB(${ORA_PDB_NAME}) to READ WRITE."
        DeleteFile "${START_PDB}" "${START_PDBRST}" "${RESULT_TMP_FILE}"
        return 1 
    fi
    
    GetPDBStatus ${DBINSTANCE} ${ORA_PDB_NAME}
    RET_CODE=$?
    if [ ${RET_CODE} -eq 0 ] && [ "${ORA_PDB_STATUS}" = "READ WRITE" ]
    then
        DeleteFile  "${START_PDB}" "${START_PDBRST}" "${RESULT_TMP_FILE}"
        return 0
    fi

    echo "alter pluggable database ${ORA_PDB_NAME} open;" > "${START_PDB}"    
    echo "exit" >> "${START_PDB}"
    
    Log "Exec SQL to start this PDB."
    OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${START_PDB}" "${START_PDBRST}" "${ORA_INSTANCENAME}" 600 0 >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ ${RET_CODE} -ne 0 ]
    then
        DeleteFile "${START_PDB}" "${START_PDBRST}"
        Log "Start PDB failed, PDB name: ${ORA_PDB_NAME}."
        exit ${RET_CODE}
    fi
    
    GetPDBStatus ${DBINSTANCE} ${ORA_PDB_NAME}
    RET_CODE=$?
    if [ ${RET_CODE} -ne 0 ] || [ "${ORA_PDB_STATUS}" != "READ WRITE" ]
    then
        DeleteFile "${START_PDB}" "${START_PDBRST}"
        return 1
    fi
    
    Log "Start PDB success, PDB name: ${ORA_PDB_NAME}."
    DeleteFile "${START_PDB}" "${START_PDBRST}"
    return 0
}

touch ${LOG_FILE_NAME}


DBINSTANCE=`GetValue "${PARAM_FILE}" InstanceName`
IN_ORACLE_HOME=`GetValue "${PARAM_FILE}" OracleHome`
DBUSERL=`GetValue "${PARAM_FILE}" UserName`
DBUSER=`echo "$DBUSERL" | tr '[A-Z]' '[a-z]'`
DBUSERPWD=`GetValue "${PARAM_FILE}" Password`
PDBNAME=`GetValue "${PARAM_FILE}" PDBName`

Log "SubAppName=$DBINSTANCE;AppName=$DBNAME;UserName=$DBUSER;IN_ORACLE_HOME=$IN_ORACLE_HOME;PDBNAME=${PDBNAME};"

#get user shell type
GetUserShellType

#get Oracle version
GetOracleVersion
VERSION=`echo $PREVERSION | tr -d '.'`

Log "Start to check oracle instance status."
GetOracleInstanceStatus ${DBINSTANCE}
RET_CODE=$?
if [ "${RET_CODE}" -ne "0" ]
then
    Log "Get instance status failed."
    DeleteFile "$ArgFile" "${RESULT_TMP_FILE}"
    exit ${RET_CODE}
fi

if [ ! "`RDsubstr $INSTANCESTATUS 1 4`" = "OPEN" ]
then
    Log "Instance status($INSTANCESTATUS), error."
    DeleteFile "$ArgFile" "${RESULT_TMP_FILE}"
    exit ${ERROR_INSTANCE_NOSTART}
fi
Log "end to check oracle instance status, status is: OPEN, normal."

Log "Start to check oracle database type."
GetOracleCDBType ${DBINSTANCE}
ORACLE_IS_CDB=$?
if [ ${ORACLE_IS_CDB} -ne 0 ]
then
    Log "ORACLE_TMP_VERSION: $ORACLE_TMP_VERSION, this is not a CDB, start PDB failed." 
    DeleteFile "$ArgFile" "${RESULT_TMP_FILE}"
    exit ${ERROR_SCRIPT_ORACLE_INST_NOT_CDB}   
fi
Log "end to check oracle database type, normal."

Log "Start to check oracle PDB exist."
CheckPdbNameExist ${DBINSTANCE} ${PDBNAME}
RET_CODE=$?
if [ ${RET_CODE} -ne 0 ]
then
    Log "Get PDB failed, pdb not exist, return code: ${ERROR_SCRIPT_ORACLE_PDB_NOT_EXIT}."
    DeleteFile "$ArgFile" "${RESULT_TMP_FILE}"
    exit ${ERROR_SCRIPT_ORACLE_PDB_NOT_EXIT}
fi
Log "end to get oracle PDB, ${PDBNAME} exist."

Log "Start to start this oracle PDB(${PDBNAME})."
StartThisOraclePDB ${DBINSTANCE} ${PDBNAME}
RET_CODE=$?
if [ ${RET_CODE} -ne 0 ]
then
    Log "Start this oracle PDB failed."
    DeleteFile "$ArgFile" "${RESULT_TMP_FILE}"
    exit ${ERROR_SCRIPT_START_PDB_FAILED}   
fi
Log "end to start this oracle PDB."

cat "${RESULT_TMP_FILE}" | uniq > "${RESULT_FILE}"
DeleteFile "$ArgFile" "${RESULT_TMP_FILE}"
Log "start PDB successful."

#chmod 640 RESULT_FILE
if [ -f "${RESULT_FILE}" ]; then
    chmod 640 "${RESULT_FILE}"
fi

exit 0
