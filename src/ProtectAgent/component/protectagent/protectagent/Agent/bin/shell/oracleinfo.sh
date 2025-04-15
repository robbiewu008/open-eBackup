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
#program name:          oracleinfo.sh     
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

. "${AGENT_ROOT_PATH}/sbin/agent_sbin_func.sh"
. "${AGENT_ROOT_PATH}/sbin/oraclefunc.sh"

#===define these for the functions of agent_sbin_func.sh and oraclefunc.sh===
#for GetUserShellType
ORACLE_SHELLTYPE=
GRID_SHELLTYPE=
RDADMIN_SHELLTYPE=
#for log
LOG_FILE_NAME="${LOG_PATH}/oracleinfo.log"
#for GetOracleVersion
ORA_VERSION=
ORCLVESION="${STMP_PATH}/ORCV${PID}.txt"
SQLEXIT="${STMP_PATH}/EXITSQL${PID}.sql"
SQLQUERYASM="${STMP_PATH}/SQLQUERYASM${PID}.sql"
#for GetClusterType
DBISCLUSTER=0
#for GetOracleBasePath,GetOracleHomePath
IN_ORACLE_BASE=
IN_ORACLE_HOME=
# oracle temp file
ORATMPINFO="${STMP_PATH}/ORATMPINFO${PID}.txt"
SIDTMPINFO="${STMP_PATH}/SIDTMPINFO${PID}.txt"
#===define these for the functions of agent_sbin_func.sh===
G_ORACLE_VERSION122="12.2"

#===define these for local script===
PARAMFILENAME=
ORAPWDFILENAME=
#===define these for local script===

CheckOracleIsInstall
RST=$?
if [ $RST -ne 0 ]; then
    exit $RST
fi

GetOracleUser ${ASMSIDNAME}
#get user shell type
GetUserShellType ${ORA_DB_USER} ${ORA_GRID_USER}

#get oracle base path
GetOracleBasePath ${ORA_DB_USER}

#get oracle home path
GetOracleHomePath ${ORA_DB_USER}

#get Oracle version
GetOracleVersion ${ORA_DB_USER}
VERSION=`echo $PREVERSION | tr -d '.'`
# get cluster flat
GetOracleCluster

CheckParameterFile()
{
    PARAM_FILE_NAME=$1
    #first copy init file to tmp directory
    if [ ! -f "${PARAM_FILE_NAME}" ]
    then
        Log "CheckParameterFile:`basename ${PARAM_FILE_NAME}` not exists."
        return 0
    fi

    TMP_PARAM_FILE="$STMP_PATH/paramFile${STRSID}$PID.ora"
    cp -d -f "${PARAM_FILE_NAME}" "${TMP_PARAM_FILE}"

    ISFIND=0
    for line in `cat "${TMP_PARAM_FILE}"`
    do
        TMP=`echo ${line} | tr [a-z] [A-Z]`
        echo $TMP | grep "SPFILE" > /dev/null
        if [ "$?" != "0" ]
        then
            echo $TMP | grep "CONTROL_FILES=" > /dev/null
            if [ "$?" != "0" ]
            then
                continue
            fi
        fi

        TMP=`echo $TMP | $MYAWK -F "=" '{print $2}'`
        TMP=`echo $TMP | sed 's/ //g'`
        TMP=`RDsubstr $TMP 2 1`

        if [ "$TMP" = "+" ]
        then
            ISFIND=1
            break
        fi
    done

    DeleteFile "${TMP_PARAM_FILE}"

    if [ "$ISFIND" = "1" ]
    then
        return 1
    fi
    
    return 0
}

# -------------------------------------------------------
# function: CheckIsASM()
# description: check Oracle is ASM type
# para: instants 
# return: isASM
# -------------------------------------------------------
CheckIsASM()
{
    STRSID=$1
    #first copy init file to tmp directory
    CheckParameterFile "${IN_ORACLE_HOME}/dbs/init${STRSID}.ora"
    if [ "$?" -eq "1" ]
    then
       return 1
    fi

    CheckParameterFile "${IN_ORACLE_HOME}/dbs/spfile${STRSID}.ora"
    if [ "$?" -eq "1" ]
    then
       return 1
    fi

   return 0
}

CheckDBRole()
{
    instStatus=$1
    if [ "${DBISCLUSTER}" = "1" ]; then
        if [ "${instStatus}" = "0" -a "${ASMStatus}" = "1" ]; then
            echo 3
        elif [ "${instStatus}" = "1" -a "${ASMStatus}" = "1" ]; then
            echo 1
        elif [ "${instStatus}" = "0" -a "${ASMStatus}" = "0" ]; then
            echo 2
        else
            echo 0
        fi
    else
        if [ "${instStatus}" = "0" ]; then
            echo 3
        else
            echo 0
        fi
    fi
}

GetDatabaseName()
{
    local INST_NAME=$1
    GetDatabaseNameSQL="${STMP_PATH}/GetDatabaseName${PID}.sql"
    GetDatabaseNameRST="${STMP_PATH}/GetDatabaseName${PID}.txt"
    echo "set linesize 999;" > "$GetDatabaseNameSQL"
    echo "show parameter db_name;" >> "$GetDatabaseNameSQL"
    echo "exit" >> "$GetDatabaseNameSQL"
    OracleExeSql "" "" ${GetDatabaseNameSQL} ${GetDatabaseNameRST} ${INST_NAME} 10 1 ${ORA_DB_USER} >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ ${RET_CODE} -ne 0 ]; then
        Log "GetDatabaseName failed,ret=${RET_CODE}"
        DeleteFile "${GetDatabaseNameSQL}" "${GetDatabaseNameRST}" "${ERRDETAIL_FILE}"
        echo ""
    else
        local dbName=`cat ${GetDatabaseNameRST} | $MYAWK '{print $3}'`
        dbName=`echo ${dbName} | tr [A-Z] [a-z]`
        DeleteFile "${GetDatabaseNameSQL}" "${GetDatabaseNameRST}"
        echo ${dbName}
    fi
}

GetDatabaseUUID()
{
    INST_NAME=$1
    GetDatabaseUUIDSQL="${STMP_PATH}/GetDatabaseUUID${PID}.sql"
    GetDatabaseUUIDRST="${STMP_PATH}/GetDatabaseUUID${PID}.txt"
    echo "set linesize 999;" > "$GetDatabaseUUIDSQL"
    echo "select dbid from v\$database;" >> "$GetDatabaseUUIDSQL"
    echo "exit" >> "$GetDatabaseUUIDSQL"
    OracleExeSql "" "" ${GetDatabaseUUIDSQL} ${GetDatabaseUUIDRST} ${INST_NAME} 10 1 ${ORA_DB_USER} >> "${LOG_FILE_NAME}" 2>&1
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

GetDatabaseList()
{
    Log "Begin to read oracle mapping info of database and instance info."
    DeleteFile "${ORATMPINFO}"
    if [ "`RDsubstr ${ORA_VERSION} 1 2`" = "10" ]; then
        FILE_LIST=`ls ${IN_ORACLE_BASE}/admin`
        for DB_NAME in ${FILE_LIST}; do
            for SID_NAME in `ls -l ${IN_ORACLE_BASE}/admin/${DB_NAME}/bdump/alert_*.log | $MYAWK '{print $NF}'`; do
                SID_NAME=`echo ${SID_NAME} | sed 's/.*alert_\(.*\)\.log/\1/'`
                if [ -z "${SID_NAME}" ]; then
                    Log "DBNAME=${DB_NAME} could not found alert file."
                    continue
                fi
                echo "${SID_NAME} ${DB_NAME}" >> "${ORATMPINFO}"
            done
        done
    else
        FILE_LIST=`ls ${IN_ORACLE_BASE}/diag/rdbms`
        for DB_NAME in ${FILE_LIST}; do
            for SID_NAME in `ls -l ${IN_ORACLE_BASE}/diag/rdbms/${DB_NAME} | grep "^d" | $MYAWK '{print $NF}'`; do
                echo "${SID_NAME} ${DB_NAME}" >> "${ORATMPINFO}" 
            done
        done
    fi
}

GetSIDList()
{
    Log "Begin to read oracle instance list."
    DeleteFile "${SIDTMPINFO}"
    for SID_NAME_IN in `ls -l ${IN_ORACLE_HOME}/dbs/init*.ora | $MYAWK '{print $NF}'`; do
        SID_NAME=`echo ${SID_NAME_IN} | sed 's/.*init\(.*\)\.ora/\1/'`
        if [ -z "${SID_NAME}" ]; then
            continue
        fi
        echo ${SID_NAME} >> ${SIDTMPINFO}
    done

    for SID_NAME_IN in `ls -l ${IN_ORACLE_HOME}/dbs/spfile*.ora | $MYAWK '{print $NF}'`; do
        SID_NAME=`echo ${SID_NAME_IN} | sed 's/.*spfile\(.*\)\.ora/\1/'`
        if [ -z "${SID_NAME}" ]; then
            continue
        fi
        
        cat "${SIDTMPINFO}" | grep "^${SID_NAME}$" > /dev/null
        if [ $? -eq 0 ]; then
            continue
        fi
        echo ${SID_NAME} >> ${SIDTMPINFO}
    done

    # query online database, some asm database can't found by the above method
    if [ "`RDsubstr ${ORA_VERSION} 1 4`" != "12.2" ]; then
        SIDList="`ps -ef | grep ora_...._ | grep -v grep | $MYAWK '{print $NF}' | $MYAWK -F"_...._" '{print $2}' | uniq`"
        for SID_NAME in ${SIDList}; do 
            grep "^${SID_NAME}$" "${SIDTMPINFO}" > /dev/null
            if [ $? -eq 0 ]; then
                continue
            fi
            echo ${SID_NAME} >> ${SIDTMPINFO}
        done
    fi

    if [ ${DBISCLUSTER} -eq 1 ]; then
        local dbNameList=`su - ${ORA_DB_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV}srvctl config database"`
        local nodeName=`hostname`
        for DB_NAME in `echo ${dbNameList}`; do
            SID_NAME=`su - ${ORA_DB_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV}srvctl status instance -d ${DB_NAME} -n ${nodeName}" | $MYAWK '{print $2}'`
            grep "^${SID_NAME}$" "${SIDTMPINFO}" > /dev/null
            if [ $? -eq 0 ]; then
                continue
            fi
            echo ${SID_NAME} >> ${SIDTMPINFO}
        done
    elif [ ${DBISCLUSTER} -eq 2 ]; then
        local dbNameList=`su - ${ORA_DB_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV}srvctl config database"`
        for DB_NAME in `echo ${dbNameList}`; do
            SID_NAME=`su - ${ORA_DB_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV}srvctl config database -d ${DB_NAME} | grep \"Database instance:\"" | $MYAWK '{print $NF}'`
            grep "^${SID_NAME}$" "${SIDTMPINFO}" > /dev/null
            if [ $? -eq 0 ]; then
                continue
            fi
            echo ${SID_NAME} >> ${SIDTMPINFO}
        done
    fi
}

GetDatabaseList

GetSIDList

# check ASM instance
ASMStatus=0
ASM_INST=`ps -ef | grep asm_...._+ | grep -v grep | awk -F_ '{print $NF}' | uniq`
if [ ! -z "$ASM_INST" ]; then
    ASMStatus=1
fi

Log "Begin to read oracle all information."
DeleteFile "${RESULT_FILE}"

Log "${ORA_VERSION};${IN_ORACLE_HOME}"
echo "${ORA_VERSION};${IN_ORACLE_HOME}" > "${RESULT_FILE}"

while read SID_NAME; do
    if [ -z "${SID_NAME}" ]; then
        continue
    fi
    if [ "`RDsubstr ${SID_NAME} 1 1`" = "+" ]; then
        continue
    fi

    # ----------database status---------
    ps -aef | sed 's/ *$//g' | grep "ora_...._${SID_NAME}$" | grep -v "grep" > /dev/null
    if [ "$?" = "0" ]; then
        STATUS=0 #online
        CheckInstAuth "${SID_NAME}"
        AuthType=$?
        DB_UUID=`GetDatabaseUUID ${SID_NAME}`
    else
        STATUS=1 #outline
        AuthType=0
        DB_UUID=
    fi

    if [ "`RDsubstr ${SID_NAME} 1 4`" = "12.2" ]; then
        ISASM=0
        if [ ${STATUS} -eq 0 ]; then
            echo "show parameter spfile;" > "${SQLQUERYASM}"
            echo "exit" >> "${SQLQUERYASM}"
            ISASM=`su - ${ORA_DB_USER} -c "${EXPORT_ORACLE_ENV}ORACLE_SID=${SID_NAME} ; export ORACLE_SID; sqlplus -L '/ as sysdba' @"${SQLQUERYASM}"" | grep + | awk '{print $NF}' | wc -l`
            DeleteFile "${SQLQUERYASM}"
        fi
    else
        CheckIsASM ${SID_NAME}
        ISASM=$?
    fi
    
    DBRole=`CheckDBRole ${STATUS}`

    # ----------------get database name---------------------
    if [ `cat ${ORATMPINFO} | grep "^${SID_NAME} " | wc -l` -eq 1 ]; then
        DB_NAME=`cat "${ORATMPINFO}" | grep "^${SID_NAME} " | $MYAWK '{print $2}'`
        t_Name=`RDsubstr ${DB_NAME} 1 8`
        Log "${ORA_VERSION};${SID_NAME};${t_Name};${STATUS};${ISASM};${AuthType};${DBRole};${IN_ORACLE_HOME};${DB_UUID}"
        echo "${ORA_VERSION};${SID_NAME};${t_Name};${STATUS};${ISASM};${AuthType};${DBRole};${IN_ORACLE_HOME};${DB_UUID}" >> "${RESULT_FILE}"
    else
        if [ ${STATUS} -eq 0 ]; then
            DB_NAME=`GetDatabaseName ${SID_NAME}`
            if [ ! -z ${DB_NAME} ]; then
                t_Name=`RDsubstr ${DB_NAME} 1 8`
                Log "${ORA_VERSION};${SID_NAME};${t_Name};${STATUS};${ISASM};${AuthType};${DBRole};${IN_ORACLE_HOME};${DB_UUID}"
                echo "${ORA_VERSION};${SID_NAME};${t_Name};${STATUS};${ISASM};${AuthType};${DBRole};${IN_ORACLE_HOME};${DB_UUID}" >> "${RESULT_FILE}"
                continue
            fi
        fi

        for DB_NAME in `cat "${ORATMPINFO}" | grep "^${SID_NAME} " | $MYAWK '{print $2}'`; do
            t_Name=`RDsubstr ${DB_NAME} 1 8`
            Log "${ORA_VERSION};${SID_NAME};${t_Name};${STATUS};${ISASM};${AuthType};${DBRole};${IN_ORACLE_HOME};${DB_UUID}"
            echo "${ORA_VERSION};${SID_NAME};${t_Name};${STATUS};${ISASM};${AuthType};${DBRole};${IN_ORACLE_HOME};${DB_UUID}" >> "${RESULT_FILE}"
        done
    fi

done < "${SIDTMPINFO}"
Log "Get DBlist succ."

DeleteFile "${ORATMPINFO}"
DeleteFile "${SIDTMPINFO}"

#chmod 640 RESULT_FILE
if [ -f  "${RESULT_FILE}" ]; then
    chmod 640 "${RESULT_FILE}"
fi

exit 0
