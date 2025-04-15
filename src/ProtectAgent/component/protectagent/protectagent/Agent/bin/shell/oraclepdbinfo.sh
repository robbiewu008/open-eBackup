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
LOG_FILE_NAME="${LOG_PATH}/oraclepdbinfo.log"
#for GetOracleVersion
ORA_VERSION=
PREVERSION=
VERSION=
ORCLVESION="${STMP_PATH}/ORCV${PID}.txt"
SQLEXIT="${STMP_PATH}/EXITSQL${PID}.sql"
#for GetOracleInstanceStatus
CHECK_INSTANCE_STATUS="${STMP_PATH}/CheckInstanceStatus${PID}.sql"
QUERYCDBSCRIPT="$STMP_PATH/QueryCDB${PID}.sql"
QUERYCDBSCRIPTRST="$STMP_PATH/QueryCDBRST${PID}.sql"

INSTANCESTATUS=""
#********************************define these for local script********************************
DBINSTANCE=""
DBUSER=""
DBUSERPWD=""
PDBNAME=""

RESULT_TMP_FILE="${RESULT_FILE}.tmp"
#######################################set file name##################################
QUERYNAMECRIPT="$STMP_PATH/QueryFileName$PID.sql"
QUERYNAMECRIPTRST="${STMP_PATH}/QueryFileNameRST$PID.txt"

# global variable
RST_SEPARATOR=";"
PARAM_FILE=`ReadInputParam`
test -z "$PARAM_FILE"              && ExitWithError "PARAM_FILE"

# ************************** Find PDB List ***********************************
GetOraclePDBList()
{
    ORA_INSTANCENAME=$1
    local ORACLE_PDB_NAME=$2
    
    CHECK_PDB_LIST="${STMP_PATH}/CheckPDBList${PID}.sql"
    CHECK_PDB_LISTRST="${STMP_PATH}/CheckPDBListRST${PID}.txt"
    touch ${CHECK_PDB_LIST}
    touch ${CHECK_PDB_LISTRST}
    
    echo "set pagesize 300;" > "${CHECK_PDB_LIST}"  # max pdb is 252
    echo "set linesize 300;" >> "${CHECK_PDB_LIST}" # adjust col size 
    echo "col name for a100;" >> "${CHECK_PDB_LIST}"
    echo "col open_mode for a50;" >> "${CHECK_PDB_LIST}"
    echo "select con_id,name,open_mode from v\$pdbs;" >> "${CHECK_PDB_LIST}"
    echo "exit" >> "${CHECK_PDB_LIST}"
    
    Log "Exec SQL to get PDB List."
    OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${CHECK_PDB_LIST}" "${CHECK_PDB_LISTRST}" "${ORA_INSTANCENAME}" 30 0 >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ ${RET_CODE} -ne 0 ]
    then
        DeleteFile "${CHECK_PDB_LIST}" "${CHECK_PDB_LISTRST}"
        exit ${RET_CODE}
    else 
        #   >select con_id,name from v$pdbs;
        #   CON_ID	    NAME	
        #   ---------- ---------- 
        #   2 4165023779 PDB$SEED			     READ ONLY
        #   3 2358941778 PDB12C1			     MOUNTED
        #   4 2054691292 PDB12C2			     MOUNTED
        #   5 2104474211 PDB12C3			     MOUNTED
        #   Disconnected from Oracle Database 11g Enterprise Edition Release 11.2.0.3.0 - 64bit Production
        PDBTAB="${LOG_PATH}/pdbtabs${PID}.txt"
        ONEPDBINFO="${LOG_PATH}/onepdbinfo${PID}.txt"
        touch ${PDBTAB}
        touch ${ONEPDBINFO}
        sed -n '/----------/,/^ *$/p' "${CHECK_PDB_LISTRST}" > "${PDBTAB}" 
        cat -n ${PDBTAB} >> "${LOG_FILE_NAME}"
        
        LINE_CNT=`cat ${PDBTAB} | wc -l`
        Log "LINE_CNT: ${LINE_CNT}"
        
        for((x=2;x<${LINE_CNT};x++));  #line first just include "-----------" the last line include nothing
        do	
            sed -n "${x}p" "${PDBTAB}" > "${ONEPDBINFO}"
        
            tmpCONID=`sed -n '1p' "${ONEPDBINFO}" | $MYAWK '{print $1}'`
            tmpPDBNAME=`sed -n '1p' "${ONEPDBINFO}" | $MYAWK '{print $2}'`
            
            tmpSTATUS_1=`sed -n '1p' "${ONEPDBINFO}" | $MYAWK '{print $3}'` #READ
            tmpSTATUS_2=""
            if [ "${tmpSTATUS_1}" = "READ" ]
            then
                tmpSTATUS_2=`sed -n '1p' "${ONEPDBINFO}" | $MYAWK '{print $4}'` #ONLY
                tmpSTATUS="${tmpSTATUS_1} ${tmpSTATUS_2}"
            elif [ "${tmpSTATUS_1}" = "" ]
            then
                Log "Error: get this pdb($tmpPDBNAME) status failed, status: $tmpSTATUS_1."
                DeleteFile "${CHECK_PDB_LIST}" "${CHECK_PDB_LISTRST}" "${PDBTAB}" "${ONEPDBINFO}"
                exit $ERROR_SCRIPT_EXEC_FAILED
            else
                tmpSTATUS="${tmpSTATUS_1}"
            fi
            
            if [ "${ORACLE_PDB_NAME}" = "" ]
            then
                Log "CONID: ${tmpCONID}, PDBNAME: ${tmpPDBNAME}, STATUS: ${tmpSTATUS}"
                echo "${tmpCONID}${RST_SEPARATOR}${tmpPDBNAME}${RST_SEPARATOR}${tmpSTATUS}" >> "${RESULT_TMP_FILE}"
            else
                Log "PDBNAME: ${ORACLE_PDB_NAME}, tmpPDBNAME: ${tmpPDBNAME}."
                if [ "${tmpPDBNAME}" = "${ORACLE_PDB_NAME}" ] #if server put the pdbname is not NULL, just get the info of this pdb
                then
                    Log "PDBNAME: ${ORACLE_PDB_NAME}, tmpPDBNAME: ${tmpPDBNAME}, get it."
                    Log "CONID: ${tmpCONID}, PDBNAME: ${tmpPDBNAME}, STATUS: ${tmpSTATUS}"
                    echo "${tmpCONID}${RST_SEPARATOR}${tmpPDBNAME}${RST_SEPARATOR}${tmpSTATUS}" >> "${RESULT_TMP_FILE}"
                    DeleteFile "${CHECK_PDB_LIST}" "${CHECK_PDB_LISTRST}" "${PDBTAB}" "${ONEPDBINFO}"
                    return 0
                else
                   continue 
                fi
            fi
        done
        DeleteFile "${CHECK_PDB_LIST}" "${CHECK_PDB_LISTRST}" "${PDBTAB}" "${ONEPDBINFO}"
    fi

    return 0
}

touch ${LOG_FILE_NAME}
#################Entry of script to query the information of oracle portfolio###########
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
    exit ${RET_CODE}
fi

if [ ! "`RDsubstr $INSTANCESTATUS 1 4`" = "OPEN" ]
then
    Log "Instance status($INSTANCESTATUS) no open."
    exit ${ERROR_INSTANCE_NOSTART}
fi
Log "end to check oracle instance status, status is: OPEN, normal."

touch "${RESULT_TMP_FILE}"

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

Log "Start to get oracle PDB list."
GetOraclePDBList ${DBINSTANCE} ${PDBNAME}
RET_CODE=$?
if [ ${RET_CODE} -ne 0 ]
then
    Log "Get PDB list failed."
    exit ${RET_CODE}
fi
Log "end to get oracle PDB list."

cat "${RESULT_TMP_FILE}" | uniq > "${RESULT_FILE}"
##delete temporary file
DeleteFile "$ArgFile" "$QUERYNAMECRIPTRST" "$QUERYNAMECRIPT" "${RESULT_TMP_FILE}"

Log "Stat. oracle PDB information successful."

#chmod 640 RESULT_FILE
if [ -f  "${RESULT_FILE}" ]; then
    chmod 640 "${RESULT_FILE}"
fi

exit 0
