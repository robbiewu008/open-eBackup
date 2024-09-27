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
#@function: restre DB by rman.
USAGE="Usage: ./oraclenativemovedbf.sh AgentRoot PID"

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
LOG_FILE_NAME="${LOG_PATH}/oraclenativemovedbf.log"
#for GetOracleVersion
ORA_VERSION=
PREVERSION=
VERSION=
ORCLVESION="${STMP_PATH}/ORCV${PID}.txt"
SQLEXIT="${STMP_PATH}/EXITSQL${PID}.sql"


# global var, for kill monitor
PARAM_CONTENT=`ReadInputParam`
test -z "$PARAM_CONTENT"              && ExitWithError "PARAM_FILE"
# oracle temp file
RESTORESQLFILE="${STMP_PATH}/RESTOREORACLE${PID}.sql"
QUERYCTRFILES="${STMP_PATH}/QUERYCTRFILES${PID}.sql"
STARTDBSQLFILE="${STMP_PATH}/RESTORESTARTDB${PID}.sql"
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
ORACLE_HOME=`GetValue "${PARAM_CONTENT}" OracleHome`
ASMUSER=`GetValue "${PARAM_CONTENT}" ASMUserName`
ASMUSERPWD=`GetValue "${PARAM_CONTENT}" ASMPassword`
ASMSIDNAME=`GetValue "${PARAM_CONTENT}" ASMInstanceName`
DBType=`GetValue "${PARAM_CONTENT}" dbType`
STARTDB=`GetValue "${PARAM_CONTENT}" startDB`
RECOVERTARGET=`GetValue "${PARAM_CONTENT}" recoverTarget`
RECOVERPATH=`GetValue "${PARAM_CONTENT}" recoverPath`
RECOVERORDER=`GetValue "${PARAM_CONTENT}" recoverOrder`
RECOVERNUM=`GetValue "${PARAM_CONTENT}" recoverNum`
BACKUP=`GetValue "${PARAM_CONTENT}" DataPath`
ARCHIVE=`GetValue "${PARAM_CONTENT}" LogPath`
METADATAPATH=`GetValue "${PARAM_CONTENT}" MetaDataPath`

test "$DBNAME" = "${ERROR_PARAM_INVALID}"                           && ExitWithError "DBNAME"
test "$DBINSTANCE" = "${ERROR_PARAM_INVALID}"                       && ExitWithError "DBINSTANCE"
test "$DBUSER" = "${ERROR_PARAM_INVALID}"                           && ExitWithError "DBUSER"
test "$ORACLE_HOME" = "${ERROR_PARAM_INVALID}"                      && ExitWithError "ORACLE_HOME"
test "$ASMUSER" = "${ERROR_PARAM_INVALID}"                          && ExitWithError "ASMUSER"
test "$ASMSIDNAME" = "${ERROR_PARAM_INVALID}"                       && ExitWithError "ASMSIDNAME"
test "$DBType" = "${ERROR_PARAM_INVALID}"                           && ExitWithError "DBType"
test "$STARTDB" = "${ERROR_PARAM_INVALID}"                          && ExitWithError "STARTDB"
test "$RECOVERTARGET" = "${ERROR_PARAM_INVALID}"                    && ExitWithError "RECOVERTARGET"
test "$RECOVERPATH" = "${ERROR_PARAM_INVALID}"                      && ExitWithError "RECOVERPATH"
test "$RECOVERORDER" = "${ERROR_PARAM_INVALID}"                     && ExitWithError "RECOVERORDER"
test "$RECOVERNUM" = "${ERROR_PARAM_INVALID}"                       && ExitWithError "RECOVERNUM"
test "$BACKUP" = "${ERROR_PARAM_INVALID}"                           && ExitWithError "BACKUP"
test "$ARCHIVE" = "${ERROR_PARAM_INVALID}"                          && ExitWithError "ARCHIVE"
test "$METADATAPATH" = "${ERROR_PARAM_INVALID}"                     && ExitWithError "METADATAPATH"

BACKUP=`RedirectBackPath ${BACKUP}`

Log "PID=${PID};DBINSTANCE=${DBINSTANCE};DBName=${DBNAME};DBUSER=${DBUSER};\
ASMInst=${ASMSIDNAME};ASMUSER=${ASMUSER};ORACLE_HOME=$ORACLE_HOME;\
DataPath=$BACKUP;LogPath=$ARCHIVE;METADATAPATH=${METADATAPATH};RECOVERPATH=${RECOVERPATH};\
RECOVERORDER=${RECOVERORDER};RECOVERNUM=${RECOVERNUM}."

if [ "${RECOVERORDER}" != "${RECOVERNUM}" ]; then
    Log "Only last node will mv dbf, curent process is (${RECOVERORDER}"/"${RECOVERNUM})."
    exit 0
fi

# get first data path for meta data path when there are several mount path
MainBackupPath=
GetMainDataPath
test -z "${MainBackupPath}" && ExitWithError "main data path"

ADDITIONAL="${MainBackupPath}/additional"
BACKUP_TEMP="${MainBackupPath}/tmp"
Log "BACKUPTEMP=${BACKUP_TEMP};ADDITIONAL=$ADDITIONAL;GetMainDataPath=${GetMainDataPath}"

GetOracleUser
GetUserShellType ${ORA_DB_USER} ${ORA_GRID_USER}
GetOracleBasePath ${ORA_DB_USER}
GetOracleHomePath ${ORA_DB_USER}
GetOracleVersion ${ORA_DB_USER}
VERSION=`echo $PREVERSION | tr -d '.'`
GetOracleCluster
CheckMountPath

# initial asm instance name if backup or archive is ASM diskgroup
if [ "${DBType}" = "0" ]; then
    ASMSIDNAME=`ps -ef | grep "asm_...._$ASMSIDNAME" | grep -v "grep" | $MYAWK -F '+' '{print $NF}'`
    ASMSIDNAME=`echo ${ASMSIDNAME} | $MYAWK -F " " '{print $1}'`
    ASMSIDNAME="+"${ASMSIDNAME}
    Log "ASM instance: ${ASMSIDNAME}"
fi

#********************************function define begin********************************
CheckOMFEnable()
{
    OMFEnable=0
    CheckOMFSql="${STMP_PATH}/CheckOMF${PID}.sql"
    CheckOMFRst="${STMP_PATH}/CheckOMF${PID}.txt"
    echo "set linesize 300" >${CheckOMFSql}
    echo "select value from v\$parameter where name='db_create_file_dest';" >> ${CheckOMFSql}
    echo "exit;" >> ${CheckOMFSql}
    OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${CheckOMFSql}" "${CheckOMFRst}" "${DBINSTANCE}" 30 1 "${ORA_DB_USER}" >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ "$RET_CODE" -ne "0" ]; then
        Log "CheckOMFEnable failed, ret="$RET_CODE"."
        DeleteFile "${CheckOMFSql}" "${CheckOMFRst}"
        exit ${RET_CODE}
    fi

    local OMFPath=`cat ${CheckOMFRst}`
    if [ ! -z "${OMFPath}" ]; then
        Log "OMFPath=${OMFPath}, OMF is enable"
        OMFEnable=1
    fi
    DeleteFile "${CheckOMFSql}" "${CheckOMFRst}"
}

BuildMoveNonCDBSql()
{
    echo "" > "$1"
    for line in `cat ${DATA_FILES}`; do
        con_id=`echo $line | ${MYAWK} -F ";" '{print $1}'`
        tablespace_name=`echo $line | ${MYAWK} -F ";" '{print $2}'`
        fileno=`echo $line | ${MYAWK} -F ";" '{print $3}'`
        dbfFullPath=`echo $line | ${MYAWK} -F ";" '{print $4}'`
        TSRealName=`ls ${MainBackupPath}/${fileno}/FNO-${fileno}_TS-*.dbf | grep -i $tablespace_name | $MYAWK -F '/' '{print $NF}'` >> ${LOG_FILE_NAME} 2>& 1
        [ -z "$TSRealName" ] && ExitWithError "get tablespace name is empty. "
        Log "TSRealName=${TSRealName};dbfFullPath=${dbfFullPath}"
        if [ -f "$dbfFullPath" ] ; then
            rm -rf "$dbfFullPath" >> ${LOG_FILE_NAME} 2>& 1
        fi
        if [ ${OMFEnable} -eq 0 ]; then
            if [ -z "${RECOVERPATH}" ]; then
                echo "alter database move datafile '${MainBackupPath}/${fileno}/${TSRealName}' to '${dbfFullPath%/*}/${TSRealName}' REUSE KEEP;" >> "$1"
            else
                CreateDir "${RECOVERPATH}/datafile"
                echo "alter database move datafile '${MainBackupPath}/${fileno}/${TSRealName}' to '${RECOVERPATH}/datafile/${TSRealName}' REUSE KEEP;" >> "$1"
            fi
        else
            if [ -z "${RECOVERPATH}" ]; then
                if [ `RDsubstr ${dbfFullPath} 1 1` = "+" ]; then
                    echo "alter session set db_create_file_dest='${dbfFullPath%%/*}';" >> "$1"
                fi
            else
                if [ `RDsubstr ${RECOVERPATH} 1 1` = "+" ]; then
                    echo "alter session set db_create_file_dest='${RECOVERPATH%%/*}';" >> "$1"
                fi
            fi
            echo "alter database move datafile ${fileno};" >> "$1"
        fi
    done
    echo "exit;" >> "$1"
}

BuildMoveCDBSql()
{
    echo "" > "$1"
    for line in `cat ${DATA_FILES}`; do
        con_id=`echo $line | ${MYAWK} -F ";" '{print $1}'`
        tablespace_name=`echo $line | ${MYAWK} -F ";" '{print $2}'`
        fileno=`echo $line | ${MYAWK} -F ";" '{print $3}'`
        dbfFullPath=`echo $line | ${MYAWK} -F ";" '{print $4}'`
        GetPDBNameByConID ${DBINSTANCE} ${con_id}.
        if [ ! -z "${ORA_PDB_NAME}" ]; then
            echo "alter session set CONTAINER=${ORA_PDB_NAME};" >> "$1"
        fi
        TSRealName=`ls ${MainBackupPath}/${fileno}/FNO-${fileno}_TS-*.dbf | grep -i $tablespace_name | $MYAWK -F '/' '{print $NF}'` >> ${LOG_FILE_NAME} 2>& 1
        [ -z $TSRealName ] && ExitWithError "get tablespace name is empty. "
        Log "TSRealName=${TSRealName};dbfFullPath=${dbfFullPath}"
        if [ -f "$dbfFullPath" ] ; then
            rm -rf $dbfFullPath >> ${LOG_FILE_NAME} 2>& 1
        fi
        if [ ${OMFEnable} -eq 0 ]; then
            if [ -z "${RECOVERPATH}" ]; then
                echo "alter database move datafile '${MainBackupPath}/${fileno}/${TSRealName}' to '${dbfFullPath%/*}/${TSRealName}' REUSE KEEP;" >> "$1"
            else
                CreateDir "${RECOVERPATH}/datafile"
                echo "alter database move datafile '${MainBackupPath}/${fileno}/${TSRealName}' to '${RECOVERPATH}/datafile/${TSRealName}' REUSE KEEP;" >> "$1"
            fi
        else
            if [ -z "${RECOVERPATH}" ]; then
                if [ `RDsubstr ${dbfFullPath} 1 1` = "+" ]; then
                    echo "alter session set db_create_file_dest='${dbfFullPath%%/*}';" >> "$1"
                fi
            else
                if [ `RDsubstr ${RECOVERPATH} 1 1` = "+" ]; then
                    echo "alter session set db_create_file_dest='${RECOVERPATH%%/*}';" >> "$1"
                fi
            fi
            echo "alter database move datafile ${fileno};" >> "$1"
        fi
    done
    echo "exit;" >> "$1"
}

MoveDBFOnline()
{
    # alter database move datafile  '/export/home/oracle/user01.dbf' to '+NEWTST/TESTDB2/DATAFILE/user01.dbf';
    MoveDBFOnlineSQL="${STMP_PATH}/MoveDBFOnline${PID}.sql"
    MoveDBFOnlineRST="${STMP_PATH}/MoveDBFOnlineRST${PID}.txt"
    Log "Begin to move data online."
    
    if [ "${ORACLE_IS_CDB}" = "0" ]; then
        BuildMoveCDBSql ${MoveDBFOnlineSQL}
    else
        BuildMoveNonCDBSql ${MoveDBFOnlineSQL}
    fi

    OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${MoveDBFOnlineSQL}" "${MoveDBFOnlineRST}" "${DBINSTANCE}" -1 0 "${ORA_DB_USER}" >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ "$RET_CODE" -ne "0" ]; then
        Log "Move dbf Online failed, ret="$RET_CODE"."
        DeleteFile "${MoveDBFOnlineSQL}" "${MoveDBFOnlineRST}"
        exit ${RET_CODE}
    fi
    DeleteFile "${MoveDBFOnlineSQL}" "${MoveDBFOnlineRST}"
}
#********************************function define end********************************

#********************************prepare for movefile begin********************************
#1. copy datafiles
DATA_FILES="${STMP_PATH}/datafiles4mvdbf${PID}"
cp -d -f "$ADDITIONAL/dbfiles" ${DATA_FILES} >> "${LOG_FILE_NAME}" 2>&1
[ $? -eq 0 ] || ExitWithErrorCode "copy data list file from fs failed." $ERROR_BACKUP_INVALID

#2. check if is CDB
GetOracleCDBType ${DBINSTANCE}
ORACLE_IS_CDB=$?

#3. open pdb while status is mounted. When pdb status is mounted, it can't move datafile online.
if [ "${ORACLE_IS_CDB}" = "0" ]; then
    OpenAllPDBs
fi

CheckOMFEnable

#4. movefile online
MoveDBFOnline

#5. set dbinfo
AddDBInfo2OraTab

DeleteFile ${DATA_FILES}

Log "Move data online success."

exit 0

#********************************prepare for movefile end********************************
