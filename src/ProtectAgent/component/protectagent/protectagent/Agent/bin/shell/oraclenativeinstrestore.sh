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
#@function: restre DB by rman.
USAGE="Usage: ./oraclenativeinstrestore.sh AgentRoot PID"

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
LOG_FILE_NAME="${LOG_PATH}/oraclenativeinstrestore.log"
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
BACKUP=`GetValue "${PARAM_CONTENT}" DataPath`
ARCHIVE=`GetValue "${PARAM_CONTENT}" LogPath`
METADATAPATH=`GetValue "${PARAM_CONTENT}" MetaDataPath`
RECOVERTARGET=`GetValue "${PARAM_CONTENT}" recoverTarget`
RECOVERPATH=`GetValue "${PARAM_CONTENT}" recoverPath`
RECOVERORDER=`GetValue "${PARAM_CONTENT}" recoverOrder`
RECOVERNUM=`GetValue "${PARAM_CONTENT}" recoverNum`
CHANNELS=`GetValue "${PARAM_CONTENT}" Channel`
PIT_TIME=`GetValue "${PARAM_CONTENT}" pitTime`
PIT_SCN=`GetValue "${PARAM_CONTENT}" pitScn`
ASMSIDNAME=`GetValue "${PARAM_CONTENT}" ASMInstanceName`
DBType=`GetValue "${PARAM_CONTENT}" dbType`
ENCALGO=`GetValue "${PARAM_CONTENT}" EncAlgo`
ENCKEY=`GetValue "${PARAM_CONTENT}" EncKey`
PFILEPID=`GetValue "${PARAM_CONTENT}" pfilePID`

test "$DBNAME" = "${ERROR_PARAM_INVALID}"                           && ExitWithError "DBNAME"
test "$DBINSTANCE" = "${ERROR_PARAM_INVALID}"                       && ExitWithError "DBINSTANCE"
test "$DBUSER" = "${ERROR_PARAM_INVALID}"                           && ExitWithError "DBUSER"
test "$ORACLE_HOME" = "${ERROR_PARAM_INVALID}"                      && ExitWithError "ORACLE_HOME"
test "$ASMUSER" = "${ERROR_PARAM_INVALID}"                          && ExitWithError "ASMUSER"
test "$BACKUP" = "${ERROR_PARAM_INVALID}"                           && ExitWithError "BACKUP"
test "$ARCHIVE" = "${ERROR_PARAM_INVALID}"                          && ExitWithError "ARCHIVE"
test "$METADATAPATH" = "${ERROR_PARAM_INVALID}"                     && ExitWithError "METADATAPATH"
test "$RECOVERTARGET" = "${ERROR_PARAM_INVALID}"                    && ExitWithError "RECOVERTARGET"
test "$RECOVERPATH" = "${ERROR_PARAM_INVALID}"                      && ExitWithError "RECOVERPATH"
test "$RECOVERORDER" = "${ERROR_PARAM_INVALID}"                     && ExitWithError "RECOVERORDER"
test "$RECOVERNUM" = "${ERROR_PARAM_INVALID}"                       && ExitWithError "RECOVERNUM"
test "$CHANNELS" = "${ERROR_PARAM_INVALID}"                         && ExitWithError "CHANNELS"
test "$PIT_TIME" = "${ERROR_PARAM_INVALID}"                         && ExitWithError "PIT_TIME"
test "$PIT_SCN" = "${ERROR_PARAM_INVALID}"                          && ExitWithError "PIT_SCN"
test "$ASMSIDNAME" = "${ERROR_PARAM_INVALID}"                       && ExitWithError "ASMSIDNAME"
test "$DBType" = "${ERROR_PARAM_INVALID}"                           && ExitWithError "DBType"
test "$ENCALGO" = "${ERROR_PARAM_INVALID}"                          && ExitWithError "ENCALGO"
test "$PFILEPID" = "${ERROR_PARAM_INVALID}"                         && ExitWithError "PFILEPID"

BACKUP=`RedirectBackPath ${BACKUP}`
# get first data path for meta data path when there are several mount path
MainBackupPath=
GetMainDataPath
ADDITIONAL="${MainBackupPath}/additional"
BACKUP_TEMP="${MainBackupPath}/tmp"
Log "BACKUPTEMP=${BACKUP_TEMP};ADDITIONAL=$ADDITIONAL;MainBackupPath=${MainBackupPath}"

Log "PID=${PID};DBINSTANCE=${DBINSTANCE};DBName=${DBNAME};DBUSER=${DBUSER};\
ASMInst=${ASMSIDNAME};ASMUSER=${ASMUSER};ORACLE_HOME=$ORACLE_HOME;DataPath=$BACKUP;LogPath=$ARCHIVE;\
METADATAPATH=${METADATAPATH};PIT_SCN=${PIT_SCN};PIT_TIME=${PIT_TIME};RECOVERORDER=${RECOVERORDER};\
RECOVERNUM=${RECOVERNUM};DBType=${DBType};RECOVERPATH=${RECOVERPATH};RECOVERTARGET=${RECOVERTARGET};pfilePID=${PFILEPID}."

GetOracleUser ${ASMSIDNAME}
GetUserShellType ${ORA_DB_USER} ${ORA_GRID_USER}
GetOracleBasePath ${ORA_DB_USER}
GetOracleHomePath ${ORA_DB_USER}
GetOracleVersion ${ORA_DB_USER}
VERSION=`echo $PREVERSION | tr -d '.'`
if [ `RDsubstr ${ORA_VERSION} 1 2`  -le 11 ]; then
    exit $ERROR_ORACLE_DB_EXIST
fi
GetOracleCluster
CheckSqlPlusStatus
CheckRmanStatus
CheckMountPath

# initial asm instance name if backup or archive is ASM diskgroup
if [ "${DBType}" = "0" ]; then
    ASMSIDNAME=`ps -ef | grep "asm_...._$ASMSIDNAME" | grep -v "grep" | $MYAWK -F '+' '{print $NF}'`
    ASMSIDNAME=`echo ${ASMSIDNAME} | $MYAWK -F " " '{print $1}'`
    ASMSIDNAME="+"${ASMSIDNAME}
    Log "ASM instance: ${ASMSIDNAME}"
fi
#********************************check parameter valid begin********************************
# check channels param
CheckParamChannels

CheckDGExit

# prepare PIT
COPY_RESTORE=0
PreparePIT

# target instance not exist
CheckInsNotExist
[ $? -ne 0 ] && ExitWithErrorCode "instance already exist" $ERROR_ORACLE_DB_EXIST

# check parameter valid
CheckParams4Restore

# check restore params
BKVersion=`cat $ADDITIONAL/dbinfo | $MYAWK -F ";" '{print $7}'`
Log "restore version is ${VERSION}, while backup version is ${BKVersion}."
if [ "`RDsubstr ${VERSION} 1 2`" != "`RDsubstr ${BKVersion} 1 2`" ]; then
    Log "restore version not match."
    exit $ERROR_ORACLE_VERSION_DISMATCH
fi

BKAllVersion=`cat $ADDITIONAL/dbinfo | $MYAWK -F ";" '{print $10}'`
UPGRADE=0
CompareOracleVersion $ORA_VERSION $BKAllVersion
if [ $? -eq 1 ]; then
    UPGRADE=1
fi

INCAR_NUM=`cat $ADDITIONAL/dbinfo | $MYAWK -F ";" '{print $8}'`
if [ "${INCAR_NUM}" = "" ]; then
    Log "Get backup copy incranation failed."
    exit $ERROR_SCRIPT_EXEC_FAILED
fi
#********************************check parameter valid begin********************************

#********************************prepare for instance restore begin********************************


ISEncBK=0
if [ ! -z "${ENCALGO}" ] && [ ! -z "${ENCKEY}" ]; then
    ISEncBK=1
fi

if [ ${ISEncBK} -eq 1 ]; then
    if [ `RDsubstr ${ORA_VERSION} 1 2` -gt 11 ]; then
        RMAN_ENC_SECTION="configure encryption algorithm '${ENCALGO}';"
    fi
    RMAN_ENC_SECTION="${RMAN_ENC_SECTION} set decryption identified by '${ENCKEY}';"
fi

PFILE_NAME="${IN_ORACLE_HOME}/dbs/ebackup-pfile${DBINSTANCE}.ora"
DBPW_FILE="${IN_ORACLE_HOME}/dbs/orapw${DBINSTANCE}"
SCN_DBF_MAX="${STMP_PATH}/scn_dbf_max${PID}"
DATA_FILES="${STMP_PATH}/datafiles${PID}"
CTL_FILES="${STMP_PATH}/ctlfiles${PID}"
LOG_FILES="${STMP_PATH}/logfiles${PID}"
ENV_FILE="${STMP_PATH}/oracle_env${PID}"
resetlogs_id=`cat $ADDITIONAL/dbinfo | $MYAWK -F ";" '{print $9}'`
taskType=2

# delete same name logfile
for Logfile in `cat ${ADDITIONAL}/logfiles`; do
    Logfile=`echo $Logfile | ${MYAWK} -F ";" '{print $NF}'`
    logname=`echo $Logfile | ${MYAWK} -F "/" '{print $NF}'`
    if [ ${DBISCLUSTER} -eq 0 ]; then
        if [ -f $Logfile ]; then
            su - ${ORA_DB_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV}rm -rf $Logfile"
        fi
    else 
        ret=`su - ${ORA_GRID_USER} -c "${EXPORT_GRID_ENV}export ORACLE_SID=${ASMSIDNAME}; echo ls $Logfile | asmcmd"`
        echo $ret | grep $logname
        if [ $? -eq 0 ]; then
            su - ${ORA_GRID_USER} -c "${EXPORT_GRID_ENV}export ORACLE_SID=${ASMSIDNAME}; echo rm -rf $Logfile | asmcmd"
        fi
    fi
done

# prepare database pfile
PrepareConfFile

# try to replace ORACLE_HOME and ORACLE_BASE
ReplaceFileByNewEnv


MoveLogFileSql()
{
    Log "move log file start"
    MoveOnlineLogSQL="${STMP_PATH}/MoveOnlineLog${PID}.sql"
    MoveOnlineLogRST="${STMP_PATH}/MoveOnlineRST${PID}.txt"
    echo "" > "$MoveOnlineLogSQL"
    echo "startup mount" >> "$MoveOnlineLogSQL"
    for Logfile in `cat ${ADDITIONAL}/logfiles`; do
        Logfile=`echo $Logfile | ${MYAWK} -F ";" '{print $NF}'`
        CreateDir ${Logfile%/*}
        logname=`echo $Logfile | ${MYAWK} -F "/" '{print $NF}'`
        [ -f $Logfile ] && rm -rf $Logfile >> "${LOG_FILE_NAME}" 2>&1
        for line in `cat ${ADDITIONAL}/livemount/logfiles`; do
            line=`echo $line | ${MYAWK} -F ";" '{print $NF}'`
            local TimeStamp=`date +%s`
            if [ "`RDsubstr ${Logfile} 1 1`" = "/"  ]; then
                cp -d -f $line $Logfile >> "${LOG_FILE_NAME}" 2>&1
                chown -h ${ORA_DB_USER}:${ORA_DB_GROUP} ${Logfile}
            else 
                su - ${ORA_GRID_USER} -c "${EXPORT_GRID_ENV}export ORACLE_SID=${ASMSIDNAME}; echo cp $line $Logfile-${TimeStamp} | asmcmd" >> ${LOG_FILE_NAME} 2>& 1
            fi
            echo $line | grep $logname
            if [ $? -eq 0 ]; then
                if [ "`RDsubstr ${Logfile} 1 1`" = "/" ]; then
                    echo "ALTER DATABASE RENAME FILE '${line}' to '$Logfile';" >> "$MoveOnlineLogSQL"
                else 
                    echo "ALTER DATABASE RENAME FILE '${line}' to '$Logfile-${TimeStamp}';" >> "$MoveOnlineLogSQL"
                fi
                break
            fi
        done
    done
    echo "alter database open ;" >> "$MoveOnlineLogSQL"
    echo "exit;" >> "$MoveOnlineLogSQL"
    OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${MoveOnlineLogSQL}" "${MoveOnlineLogRST}" "${DBINSTANCE}" -1 0 "${ORA_DB_USER}" >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ "$RET_CODE" -ne "0" ]; then
        Log "Move log Online failed, ret="$RET_CODE"."
        DeleteFile "${MoveOnlineLogSQL}" "${MoveOnlineLogRST}"
        exit ${RET_CODE}
    fi
    DeleteFile "${MoveOnlineLogSQL}" "${MoveOnlineLogRST}"
}

# create database path if database is not exist
PrepareDBEnv "${PFILE_NAME}" "${DBNAME}"
#********************************prepare for instance restore end********************************
CheckInstAuth "${DBINSTANCE}"
AuthType=$?
if [ "$RECOVERORDER" -eq "1" ] && [ "$DBISCLUSTER" -eq "1" ] && [ "$RECOVERTARGET" -eq "2" ]; then
    su - ${ORA_DB_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV}srvctl remove database -d ${DBNAME} -f" >> "${LOG_FILE_NAME}" 2>&1
fi
#********************************instance restore begin********************************
if [ "$RECOVERORDER" = "1" ] && [ ! -f $ADDITIONAL/livemountOK ] ; then
    Log "Begin to perform instant restore on the first node."
    
    if [ -z "${RECOVERPATH}" ]; then
        # create dirs for control files
        CreateDBDirs ${DATA_FILES}
        CreateDBDirs ${CTL_FILES}
    else
        CheckRecoverPathExist
    fi

    # 5.modify pfile
    ModifyPfile ${RECOVERPATH}

    # copy control files
    CopyCtrFiles

    # create spfile from pfile
    CreateSpfile ${RECOVERPATH}

    # modify spfile name
    ModifySpfileName ${IN_ORACLE_HOME} ${DBINSTANCE}

    # create init$instance file.
    CreateInitFile ${RECOVERPATH}

    # 6.set DB file
    ExecDBRestore 2 ${ISEncBK} ${INCAR_NUM} ${COPY_RESTORE}

    GetOracleCDBType ${DBINSTANCE}
    ORACLE_IS_CDB=$?
    if [ ${UPGRADE} -eq 1 ]; then
        OracleUpgrade
        if [ ${DBISCLUSTER} -eq 1 ]; then
            ModifyClusterProperty TRUE
            StartDB
        fi
    fi

    if [ "${ORACLE_IS_CDB}" = "0" ]; then
        OpenAllPDBs
    fi
else
    Log "Begin to perform instant restore on other node of cluster"
    CreateInitFile ${RECOVERPATH}
fi

if [ -f $ADDITIONAL/livemountOK ] && [ $RECOVERORDER -eq 1 ]; then
    # modify pfile
    ModifyPfile ${RECOVERPATH}

    # copy control files
    CopyCtrFiles

    # create spfile from pfile
    CreateSpfile ${RECOVERPATH}

    # modify spfile name
    ModifySpfileName ${IN_ORACLE_HOME} ${DBINSTANCE}

    MoveLogFileSql
fi

if [ ${RECOVERTARGET} -eq 0 ]; then
    if [ $RECOVERORDER -ne 1 ]; then
        StartDB
    fi
    CheckDBIsOpen
elif [ ${RECOVERTARGET} -eq 2 ]; then
    if [ ${DBISCLUSTER} -eq 1 ]; then
        RigsterDBToRAC
        if [ ${AuthType} -eq 0 ]; then
            StartDB
        fi
    else
        CheckDBIsOpen
    fi
fi

DeleteFile ${ENV_FILE} ${SCN_DBF_MAX} ${DATA_FILES} ${CTL_FILES} ${LOG_FILES}
Log "Do Instant recovery livemount Succ."
exit 0

#********************************instance restore end********************************
