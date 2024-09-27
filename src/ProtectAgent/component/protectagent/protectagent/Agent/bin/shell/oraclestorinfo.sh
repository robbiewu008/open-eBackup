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
#@function: get DB capacity, Four capacities need to be returned.
USAGE="Usage: ./oraclestorinfo.sh AgentRoot PID"

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
LOG_FILE_NAME="${LOG_PATH}/oraclestorinfo.log"
#for GetOracleVersion
ORA_VERSION=
PREVERSION=
VERSION=
ORCLVESION="${STMP_PATH}/ORCV${PID}.txt"
SQLEXIT="${STMP_PATH}/EXITSQL${PID}.sql"
#for GetDBCapacity
GET_DB_CAPACITY_SQL="${STMP_PATH}/oraclestorinfo${PID}.sql"
DB_CAPACITY_RST="${STMP_PATH}/oraclestorinfoRst${PID}.txt"

# global var, for kill monitor
PARAM_CONTENT=`ReadInputParam`
test -z "$PARAM_CONTENT"              && ExitWithError "PARAM_FILE"
# oracle temp file
ORATMPINFO="${STMP_PATH}/ORATMPINFO${PID}.txt"
#********************************define these for the functions of agent_sbin_func.sh********************************

#********************************define these for local script********************************
ParseDeviceName()
{
    filePath=$1
    if [ "`RDsubstr ${filePath} 1 1`" = "/" ]; then
        deviceName=`df -P ${filePath} | grep '/' | ${MYAWK} '{print $1}'`
        echo "${fsDeviceList}" | grep "#${deviceName}#"
        if [ $? -eq 0 ]; then
            return
        fi
        fsDeviceList="${fsDeviceList}#${deviceName}#"
    elif [ "`RDsubstr ${filePath} 1 1`" = "+" ]; then
        deviceName=${filePath%%/*}
        deviceName=`RDsubstr ${deviceName} 2`
        echo "${asmDeviceList}" | grep "'${deviceName}'"
        if [ $? -eq 0 ]; then
            return
        fi
        asmDeviceList="${asmDeviceList}, '${deviceName}'"
    else
        Log "invalid path=${filePath}."
    fi
}

GetDeviceSize()
{
    for fsDevice in `echo ${fsDeviceList} | sed 's/#/ /g'`; do
        df -Pm | grep ^${fsDevice} > ${SqlRST}
        while read line; do
            actualName=`echo ${line} | ${MYAWK} '{print $1}'`
            if [ "${fsDevice}" = "${actualName}" ]; then
                deviceSize=`echo ${line} | ${MYAWK} '{print $2}' | $MYAWK -F "." '{print $1}'`
                break
            fi
        done < ${SqlRST}
        allDeviceSize=`expr ${allDeviceSize} + ${deviceSize}`
    done
    if [ ! -z "${asmDeviceList}" ]; then
        if [ ${AuthType} -eq 0 ]; then
            dg_info_file="${STMP_PATH}/dg_info${PID}.txt"
            echo lsdg | su - ${ORA_GRID_USER} ${GRID_SHELLTYPE} -c "export ORACLE_SID=${ASMSIDNAME}; asmcmd" > "${dg_info_file}"

            local index=0
            local index_total=0
            for str in `cat ${dg_info_file} | sed -n '1p' | awk '{$1="";print $0}'`; do
                let index+=1
                if [ "$str" == "Total_MB" ]; then
                    index_total=${index}
                fi
            done

            for dg_name in `echo "${asmDeviceList}" | sed 's/,//g' | sed "s/'//g"`; do
                totalSize=`cat ${dg_info_file} | grep " ${dg_name}/" | awk -v i=${index_total} '{print $i}'`
                allDeviceSize=`expr ${allDeviceSize} + ${totalSize}`
            done
            DeleteFile "${dg_info_file}"
        else
            echo "select max(total_mb) from v\$asm_diskgroup_stat where name in (${asmDeviceList:2});" > ${SqlCMD}
            echo "exit" >> ${SqlCMD}
            OracleExeSql "${DBUSER}" "${DBUSERPWD}" ${SqlCMD} ${SqlRST} ${DBINSTANCE} 30 1 ${ORA_DB_USER} >> "${LOG_FILE_NAME}" 2>&1
            RET_CODE=$?
            if [ "$RET_CODE" -ne "0" ]; then
                Log "query asm capacity failed."
                DeleteFile "${SqlCMD}" "${SqlRST}"
                exit ${RET_CODE}
            fi
            totalSize=`cat ${SqlRST}`
            allDeviceSize=`expr ${allDeviceSize} + ${totalSize}`
        fi
    fi
    DeleteFile "${SqlCMD}" "${SqlRST}"
}

GetDatafileInfo()
{
    allUsedSize=0
    allDeviceSize=0
    fsDeviceList=
    asmDeviceList=
    # 1
    echo "set linesize 200" > ${SqlCMD}
    echo "COL SIZE_MB FORMAT 9999999999" >> ${SqlCMD}
    echo "col name for a100" >> ${SqlCMD}
    echo "select ROUND(BYTES/1024/1024,2) SIZE_MB, name from v\$datafile;" >> ${SqlCMD}
    echo "exit" >> ${SqlCMD}
    OracleExeSql "${DBUSER}" "${DBUSERPWD}" ${SqlCMD} ${SqlRST} ${DBINSTANCE} 30 1 ${ORA_DB_USER} >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ "$RET_CODE" -ne "0" ]; then
        Log "query datafile capacity failed."
        DeleteFile "${SqlCMD}" "${SqlRST}"
        exit ${RET_CODE}
    fi
    while read line; do
        usedSize=`echo ${line} | ${MYAWK} '{print $1}'`
        allUsedSize=`expr ${allUsedSize} + ${usedSize}`

        filePath=`echo ${line} | ${MYAWK} '{print $2}'`
        ParseDeviceName "${filePath}"
    done < ${SqlRST}
    DeleteFile "${SqlCMD}" "${SqlRST}"
    GetDeviceSize

    Log "file system db file capacity:FILEUSEDCAPACITY=${allUsedSize}MB, FILEALLCAPACITY=${allDeviceSize}MB."
    echo "datacap;${allUsedSize};${allDeviceSize}" >> "${RESULT_FILE}"
}

GetLogfileInfo()
{
    allUsedSize=0
    allDeviceSize=0
    fsDeviceList=
    asmDeviceList=

    echo "select ROUND(sum(BLOCKS*BLOCK_SIZE)/1024/1024, 0) from v\$archived_log \
    where name not like '/tmp/advbackup/%' and ARCHIVED='YES' and STATUS='A';" > ${SqlCMD}
    echo "exit" >> ${SqlCMD}
    OracleExeSql "${DBUSER}" "${DBUSERPWD}" ${SqlCMD} ${SqlRST} ${DBINSTANCE} 30 1 ${ORA_DB_USER} >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ "$RET_CODE" -ne "0" ]; then
        Log "query archive_dest failed."
        DeleteFile "${SqlCMD}" "${SqlRST}"
        exit ${RET_CODE}
    fi
    allUsedSize=`cat ${SqlRST}`

    echo "set linesize 300;" > ${SqlCMD}
    echo "col DESTINATION for a255;" >> ${SqlCMD}
    echo "select DESTINATION from v\$archive_dest where STATUS='VALID';" > ${SqlCMD}
    echo "exit" >> ${SqlCMD}
    OracleExeSql "${DBUSER}" "${DBUSERPWD}" ${SqlCMD} ${SqlRST} ${DBINSTANCE} 30 1 ${ORA_DB_USER} >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ "$RET_CODE" -ne "0" ]; then
        Log "query archive_dest failed."
        DeleteFile "${SqlCMD}" "${SqlRST}"
        exit ${RET_CODE}
    fi
    if [ "`cat ${SqlRST}`" = "USE_DB_RECOVERY_FILE_DEST" ]; then
        echo "set linesize 300;" > ${SqlCMD}
        echo "col NAME for a200;" >> ${SqlCMD}
        echo "select NAME, SPACE_LIMIT/1024/1024 from V\$RECOVERY_FILE_DEST;" >> ${SqlCMD}
        echo "exit" >> ${SqlCMD}
        OracleExeSql "${DBUSER}" "${DBUSERPWD}" ${SqlCMD} ${SqlRST} ${DBINSTANCE} 30 1 ${ORA_DB_USER} >> "${LOG_FILE_NAME}" 2>&1
        RET_CODE=$?
        if [ "$RET_CODE" -ne "0" ]; then
            Log "query archive_dest failed."
            DeleteFile "${SqlCMD}" "${SqlRST}"
            exit ${RET_CODE}
        fi
        fastrecoveryPath=`cat ${SqlRST} | ${MYAWK} '{print $1}'`
        fastrecoverySize=`cat ${SqlRST} | ${MYAWK} '{print $2}'`
        ParseDeviceName "${fastrecoveryPath}"
        GetDeviceSize
        if [ ${fastrecoverySize} -le ${allDeviceSize} ]; then
            allDeviceSize=${fastrecoverySize}
        fi
    else
        while read line; do
            if [ "${line}" = "USE_DB_RECOVERY_FILE_DEST" ]; then
                continue
            fi
            ParseDeviceName "${line}"
        done < ${SqlRST}
        GetDeviceSize
    fi
    DeleteFile "${SqlCMD}" "${SqlRST}"

    Log "file system log file capacity:FILEUSEDCAPACITY=${allUsedSize}MB, FILEALLCAPACITY=${allDeviceSize}MB."
    echo "logcap;${allUsedSize};${allDeviceSize}" >> "${RESULT_FILE}"
}

GetDBInsLst()
{
    Log "Exec SQL to get database list."
    echo "set linesize 300" > ${GET_DB_CAPACITY_SQL}
    echo "select instance_name from gv\$instance;" >> ${GET_DB_CAPACITY_SQL}
    echo "exit;" >> ${GET_DB_CAPACITY_SQL}
    OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${GET_DB_CAPACITY_SQL}" "${DB_CAPACITY_RST}" "${DBINSTANCE}" 30 0 "${ORA_DB_USER}" >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ ${RET_CODE} -ne 0 ]; then
        DeleteFile "${DB_CAPACITY_RST}" "${GET_DB_CAPACITY_SQL}"
        Log "Get DBInsLst failed."
        exit 1
    fi

    TMPLINE=`sed -n '/----------/,/^ *$/p' "${DB_CAPACITY_RST}" | sed -e '/----------/d' -e '/^ *$/d'`
    local pos=0
    for insName in ${TMPLINE}; do
        if [ ${pos} -eq 0 ]; then
            DBInsLst=${insName}
            pos=1
        else
            DBInsLst=${DBInsLst}";"${insName}
        fi
    done
    DeleteFile "${DB_CAPACITY_RST}" "${GET_DB_CAPACITY_SQL}"
    Log "Get DB list is ${DBInsLst}."
    echo "dbInsLst;${DBInsLst}" >> "${RESULT_FILE}"
}

GetDBType()
{
    # 0:ASM, 1:FileSystem
    DBTYPE=1

    Log "Exec SQL to query database storage type."
    echo "set linesize 300" > ${GET_DB_CAPACITY_SQL}
    echo "select name from v\$datafile;" >> ${GET_DB_CAPACITY_SQL}
    echo "select MEMBER from v\$logfile;" >> ${GET_DB_CAPACITY_SQL}
    echo "select name from v\$controlfile;" >> ${GET_DB_CAPACITY_SQL}
    echo "select VALUE from v\$parameter where name='spfile';" >> ${GET_DB_CAPACITY_SQL}
    echo "exit" >> ${GET_DB_CAPACITY_SQL}

    OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${GET_DB_CAPACITY_SQL}" "${DB_CAPACITY_RST}" "${DBINSTANCE}" 30 0 "${ORA_DB_USER}" >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ ${RET_CODE} -eq 0 ]; then
        Log "Query for DB storage type success"
        #analyse result
        for line in `cat "${DB_CAPACITY_RST}"`; do
            if [ "`RDsubstr ${line} 1 1`" = "+" ]; then
                DBTYPE=0
                Log "Storage type is ASM."
                break
            fi
        done
        DeleteFile "${DB_CAPACITY_RST}" "${GET_DB_CAPACITY_SQL}"
    else
        Log "Query for DB storage type failed"
        DeleteFile "${DB_CAPACITY_RST}" "${GET_DB_CAPACITY_SQL}"
        exit ${RET_CODE}
    fi

    # write result
    echo "dbtype;${DBTYPE}" >> "${RESULT_FILE}"
}

# define for the function GetOraUserByInstName
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
IN_ORACLE_HOME=`GetValue "${PARAM_CONTENT}" OracleHome`
ASMUSER=`GetValue "${PARAM_CONTENT}" ASMUserName`
ASMUSERPWD=`GetValue "${PARAM_CONTENT}" ASMPassword`
ASMSIDNAME=`GetValue "${PARAM_CONTENT}" ASMInstanceName`

test "$DBNAME" = "${ERROR_PARAM_INVALID}"                           && ExitWithError "DBNAME"
test "$DBINSTANCE" = "${ERROR_PARAM_INVALID}"                       && ExitWithError "DBINSTANCE"
test "$DBUSER" = "${ERROR_PARAM_INVALID}"                           && ExitWithError "DBUSER"
test "$IN_ORACLE_HOME" = "${ERROR_PARAM_INVALID}"                   && ExitWithError "IN_ORACLE_HOME"
test "$ASMUSER" = "${ERROR_PARAM_INVALID}"                          && ExitWithError "ASMUSER"
test "$ASMSIDNAME" = "${ERROR_PARAM_INVALID}"                       && ExitWithError "ASMSIDNAME"

if [ -z "${ASMSIDNAME}" ]
then
    ASMSIDNAME="+ASM"
fi

Log "DBINSTANCE=${DBINSTANCE};DBName=${DBNAME};DBUSER=${DBUSER};ASMInst=${ASMSIDNAME};ASMUSER=${ASMUSER};IN_ORACLE_HOME=$IN_ORACLE_HOME"

DeleteFile ${RESULT_FILE}

GetOracleUser
GetUserShellType ${ORA_DB_USER} ${ORA_GRID_USER}
GetOracleHomePath ${ORA_DB_USER}
GetOracleVersion ${ORA_DB_USER}
VERSION=`echo $PREVERSION | tr -d '.'`
GetOracleCluster
DBINSTANCE=`GetRealInstanceName ${DBINSTANCE}`
Log "Check cluster [DBISCLUSTER=${DBISCLUSTER}], DBINSTANCE=${DBINSTANCE}."

CheckInstAuth "${DBINSTANCE}"
AuthType=$?

# data file storage inforamtion
GetDatafileInfo

# log file storage inforamtion
GetLogfileInfo

# get database storage type
GetDBType

# get dbinstance list
GetDBInsLst

Log "oracle get DB capacity exec success."

#chmod 640 RESULT_FILE
if [ -f "${RESULT_FILE}" ]; then
    chmod 640 "${RESULT_FILE}"
fi

exit 0
