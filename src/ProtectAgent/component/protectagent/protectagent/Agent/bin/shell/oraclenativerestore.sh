#!/bin/sh
set +x
#@function: restre DB by rman.
USAGE="Usage: ./oraclenativerestore.sh AgentRoot PID"

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
RECOVERNUM=0
#for Log
LOG_FILE_NAME="${LOG_PATH}/oraclenativerestore.log"
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
CHANNELS=`GetValue "${PARAM_CONTENT}" Channel`
PIT_TIME=`GetValue "${PARAM_CONTENT}" pitTime`
PIT_SCN=`GetValue "${PARAM_CONTENT}" pitScn`
BACKUP=`GetValue "${PARAM_CONTENT}" DataPath`
ARCHIVE=`GetValue "${PARAM_CONTENT}" LogPath`
METADATAPATH=`GetValue "${PARAM_CONTENT}" MetaDataPath`
RECOVERTARGET=`GetValue "${PARAM_CONTENT}" recoverTarget`
RECOVERPATH=`GetValue "${PARAM_CONTENT}" recoverPath`
RECOVERORDER=`GetValue "${PARAM_CONTENT}" recoverOrder`
StorType=`GetValue "${PARAM_CONTENT}" storType`
ASMSIDNAME=`GetValue "${PARAM_CONTENT}" ASMInstanceName`
DBType=`GetValue "${PARAM_CONTENT}" dbType`
ENCALGO=`GetValue "${PARAM_CONTENT}" EncAlgo`
ENCKEY=`GetValue "${PARAM_CONTENT}" EncKey`
PFILEPID=`GetValue "${PARAM_CONTENT}" pfilePID`
RestoreBy=`GetValue "${PARAM_CONTENT}" RestoreBy`
RECOVERNUM=`GetValue "${PARAM_CONTENT}" recoverNum`

test "$DBNAME" = "${ERROR_PARAM_INVALID}"                           && ExitWithError "DBNAME"
test "$DBINSTANCE" = "${ERROR_PARAM_INVALID}"                       && ExitWithError "DBINSTANCE"
test "$DBUSER" = "${ERROR_PARAM_INVALID}"                           && ExitWithError "DBUSER"
test "$ORACLE_HOME" = "${ERROR_PARAM_INVALID}"                      && ExitWithError "ORACLE_HOME"
test "$ASMUSER" = "${ERROR_PARAM_INVALID}"                          && ExitWithError "ASMUSER"
test "$CHANNELS" = "${ERROR_PARAM_INVALID}"                         && ExitWithError "CHANNELS"
test "$PIT_TIME" = "${ERROR_PARAM_INVALID}"                         && ExitWithError "PIT_TIME"
test "$PIT_SCN" = "${ERROR_PARAM_INVALID}"                          && ExitWithError "PIT_SCN"
test "$BACKUP" = "${ERROR_PARAM_INVALID}"                           && ExitWithError "BACKUP"
test "$ARCHIVE" = "${ERROR_PARAM_INVALID}"                          && ExitWithError "ARCHIVE"
test "$METADATAPATH" = "${ERROR_PARAM_INVALID}"                     && ExitWithError "METADATAPATH"
test "$RECOVERTARGET" = "${ERROR_PARAM_INVALID}"                    && ExitWithError "RECOVERTARGET"
test "$RECOVERPATH" = "${ERROR_PARAM_INVALID}"                      && ExitWithError "RECOVERPATH"
test "$RECOVERORDER" = "${ERROR_PARAM_INVALID}"                     && ExitWithError "RECOVERORDER"
test "$StorType" = "${ERROR_PARAM_INVALID}"                         && ExitWithError "StorType"
test "$ASMSIDNAME" = "${ERROR_PARAM_INVALID}"                       && ExitWithError "ASMSIDNAME"
test "$DBType" = "${ERROR_PARAM_INVALID}"                           && ExitWithError "DBType"
test "$ENCALGO" = "${ERROR_PARAM_INVALID}"                          && ExitWithError "ENCALGO"
test "$PFILEPID" = "${ERROR_PARAM_INVALID}"                         && ExitWithError "PFILEPID"
test "$RestoreBy" = "${ERROR_PARAM_INVALID}"                        && ExitWithError "RestoreBy"
test "$RECOVERNUM" = "${ERROR_PARAM_INVALID}"                       && ExitWithError "RECOVERNUM"


BACKUP=`RedirectBackPath ${BACKUP}`
# get first data path for meta data path when there are several mount path
MainBackupPath=
GetMainDataPath

ADDITIONAL="${MainBackupPath}/additional"
BACKUP_TEMP="${MainBackupPath}/tmp"
Log "BACKUPTEMP=${BACKUP_TEMP};ADDITIONAL=$ADDITIONAL;MainBackupPath=${MainBackupPath}"

Log "PID=${PID};DBINSTANCE=${DBINSTANCE};DBName=${DBNAME};DBUSER=${DBUSER};\
ASMInst=${ASMSIDNAME};ASMUSER=${ASMUSER};ORACLE_HOME=$ORACLE_HOME;DBCHANNEL=$CHANNELS;\
PIT_TIME=$PIT_TIME;PIT_SCN=$PIT_SCN;BACKUP=$BACKUP;ARCHIVE=$ARCHIVE;METADATAPATH=${METADATAPATH};\
RECOVERTARGET=${RECOVERTARGET};RECOVERPATH=${RECOVERPATH};RECOVERORDER=${RECOVERORDER};\
StorType=${StorType};MainBackupPath=${MainBackupPath};pfilePID=${PFILEPID};RestoreBy=${RestoreBy};RECOVERNUM=$RECOVERNUM"

GetOracleUser
GetUserShellType ${ORA_DB_USER} ${ORA_GRID_USER}
GetOracleBasePath ${ORA_DB_USER}
GetOracleHomePath ${ORA_DB_USER}
GetOracleVersion ${ORA_DB_USER}
VERSION=`echo $PREVERSION | tr -d '.'`
GetOracleCluster
CheckSqlPlusStatus
CheckRmanStatus
CheckMountPath

#********************************prepare for restore begin********************************
CheckDGExit

CheckInsNotExist
[ $? -ne 0 ] && ExitWithErrorCode "instance already exist" $ERROR_ORACLE_DB_EXIST

# check channels param
CheckParamChannels

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

# prepare PIT
COPY_RESTORE=0
PreparePIT

# check parameter valid
CheckParams4Restore

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

Log "PIT to restore is $PIT, scn_dbf_max is `cat $ADDITIONAL/scn_dbf_max`"
if [ `echo $PIT | grep -c scn` -ne 0 ]; then
    if [ `echo $PIT | $MYAWK '{print $2}'` -lt `cat $ADDITIONAL/scn_dbf_max | $MYAWK '{print $1}'` ]; then
        Log "Base media snapshot used to restore is newer than PIT to restore $PIT."
    fi
else
    if [[ `echo $PIT | $MYAWK -F \' '{print $2}'` < `$MYAWK '{gsub("_", " ", $2); print $2}' $ADDITIONAL/scn_dbf_max` ]]; then
        Log "Base media snapshot used to restore is newer than PIT to restore."
    fi
fi

# define params for restore
DBPW_FILE="${IN_ORACLE_HOME}/dbs/orapw${DBINSTANCE}"
PFILE_NAME="${IN_ORACLE_HOME}/dbs/ebackup-pfile${DBINSTANCE}.ora"
ENV_FILE="${STMP_PATH}/oracle_env${PID}"
SCN_DBF_MAX="${STMP_PATH}/scn_dbf_max${PID}"
DATA_FILES="${STMP_PATH}/datafiles${PID}"
CTL_FILES="${STMP_PATH}/ctlfiles${PID}"
LOG_FILES="${STMP_PATH}/logfiles${PID}"
resetlogs_id=`cat $ADDITIONAL/dbinfo | $MYAWK -F ";" '{print $9}'`
taskType=0

#********************************prepare for restore end********************************

#********************************restore function define begin********************************
StartupMount()
{
    Log "Running RMAN to Startup Mount"
    dbid=`cat $ADDITIONAL/dbinfo | $MYAWK -F ";" '{print $1}'`
    echo "set dbid=${dbid}" > ${RESTORESQLFILE}
    echo "startup nomount pfile='${MainBackupPath}/ebackup-$DBNAME-pfile.ora';" >> ${RESTORESQLFILE}
    echo "restore controlfile from '$MainBackupPath/controlfile.ctl';" >> ${RESTORESQLFILE}
    echo "startup mount;" >> ${RESTORESQLFILE}
    echo "delete noprompt expired datafilecopy all;" >> ${RESTORESQLFILE}
    echo "delete noprompt expired backupset;" >> ${RESTORESQLFILE}
    echo "quit;" >> ${RESTORESQLFILE}
    RmanExeScript "${DBUSER}" "${DBUSERPWD}" "${RESTORESQLFILE}" "${ORATMPINFO}" "${DBINSTANCE}" 180 "${ORA_DB_USER}" >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    DeleteFile ${RESTORESQLFILE} ${ORATMPINFO}
    if [ ${RET_CODE} -ne 0 ]; then
        Log "Startup Mount ${DBNAME}-${DBINSTANCE} failed, error=${RET_CODE}."
        exit ${RET_CODE}
    else
        Log "Startup Mount ${DBNAME}-${DBINSTANCE} succ."
    fi
}

BuildCatlogDatafile()
{
    echo "list datafilecopy all;" > ${RmanCMD}
    echo "quit;" >> ${RmanCMD}
    RmanExeScript "${DBUSER}" "${DBUSERPWD}" "${RmanCMD}" "${RmanRST}" "${DBINSTANCE}" 30 "${ORA_DB_USER}" >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ ${RET_CODE} -ne 0 ]; then
        Log "list datafilecopy all failed, error=${RET_CODE}."
        DeleteFile ${RmanCMD} ${RmanRST}
        exit ${RET_CODE}
    fi
    datafilePath="${STMP_PATH}/datafilePath${PID}.txt"
    cat ${RmanRST} | grep "/tmp/advbackup/data/" > ${datafilePath}
    DeleteFile ${RmanCMD} ${RmanRST}

    ipUseinfo="${STMP_PATH}/ipUseinfo${PID}.txt"
    for dataPath in `echo $BACKUP | sed 's/;/ /g'`; do
        num=`cat ${datafilePath} | grep ${dataPath} | wc -l`
        echo "${dataPath}=${num}" >> ${ipUseinfo}
    done

    for line in `cat $ADDITIONAL/dbfiles`; do
        fileID=`echo ${line} | ${MYAWK} -F ";" '{print $3}'`
        if [ -z ${fileID} ]; then
            continue
        fi
        cat ${datafilePath} | grep "/data/${fileID}/FNO"
        if [ $? -ne 0 ]; then
            FindMinUsePath
            echo "    catalog start with '${path}/${fileID}' noprompt;" >> ${RESTORESQLFILE}
        fi
    done
    DeleteFile ${datafilePath} ${ipUseinfo}
}

FindMinUsePath()
{
    num=0
    for line in `cat ${ipUseinfo}`; do
        if [ ${num} -eq 0 ]; then
            path=`echo ${line} | ${MYAWK} -F "=" '{print $1}'`
            num=`echo ${line} | ${MYAWK} -F "=" '{print $2}'`
        else
            if [ ${num} -gt `echo ${line} | ${MYAWK} -F "=" '{print $2}'` ]; then
                path=`echo ${line} | ${MYAWK} -F "=" '{print $1}'`
                num=`echo ${line} | ${MYAWK} -F "=" '{print $2}'`
            fi
        fi
    done
    num=`expr ${num} + 1`
    grep -v ${path} ${ipUseinfo} > ${ORATMPINFO}
    cat ${ORATMPINFO} > ${ipUseinfo}
    echo "${path}=${num}" >> ${ipUseinfo}
}

RunRestore()
{
    echo "RESET DATABASE TO INCARNATION ${INCAR_NUM};" > ${RESTORESQLFILE}
    echo "RUN" >> ${RESTORESQLFILE}
    echo "{" >> ${RESTORESQLFILE}
    i=1
    while [ $i -le $CHANNELS ]; do
        echo "    allocate channel eRestore`printf "%02d" $i` type disk;" >> ${RESTORESQLFILE}
        i=`expr $i + 1`
    done
    echo "    configure device type disk parallelism $CHANNELS;" >> ${RESTORESQLFILE}
    echo "    catalog start with '$MainBackupPath/spfile.bs' noprompt;" >> ${RESTORESQLFILE}
    if [ ${COPY_RESTORE} -eq 1 ]; then
        echo "    catalog start with '$MainBackupPath/log' noprompt;" >> ${RESTORESQLFILE}
    else
        echo "    catalog start with '${ARCHIVE}/resetlogs_id${resetlogs_id}' noprompt;" >> ${RESTORESQLFILE}
    fi
    BuildCatlogDatafile
    spfilePath=`cat $ADDITIONAL/spfile | sed -n '1p'`
    if [ "`RDsubstr ${spfilePath} 1 1`" = "+" ]; then
        spfileDG=`echo ${spfilePath} | $MYAWK -F "/" '{print $1}'`
        echo "    restore spfile to '${spfileDG}' from '${MainBackupPath}/spfile.bs';" >> ${RESTORESQLFILE}
    else
        echo "    restore spfile;" >> ${RESTORESQLFILE}
    fi
    echo "    restore database;" >> ${RESTORESQLFILE}
    echo "    recover database until $PIT;" >> ${RESTORESQLFILE}
    i=1
    while [ $i -le $CHANNELS ]; do
        echo "    release channel eRestore`printf "%02d" $i`;" >> ${RESTORESQLFILE}
        i=`expr $i + 1`
    done
    echo "}" >> ${RESTORESQLFILE}

    RmanExeScript "${DBUSER}" "${DBUSERPWD}" "${RESTORESQLFILE}" "${ORATMPINFO}" "${DBINSTANCE}" -1 "${ORA_DB_USER}" 1 >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ ${RET_CODE} -ne 0 ]; then
        Log "Restore database-${DBINSTANCE} failed, error=${RET_CODE}."
        DeleteFile ${RESTORESQLFILE} ${ORATMPINFO}
        if [ "${ERROR_SCRIPT_EXEC_FAILED}" = "${RET_CODE}" ]; then
            exit ${ERROR_RESTORE_FAILED}
        else
            exit ${RET_CODE}
        fi
    else
        Log "Restore ${DBNAME}-${DBINSTANCE} succ."
        DeleteFile ${RESTORESQLFILE} ${ORATMPINFO}
    fi
}

# query control file list
QueryCtrlFilesFromDB()
{
    echo "set linesize 600" > $QUERYCTRFILES
    echo "select name from v\$controlfile;" >> $QUERYCTRFILES
    echo "exit" >> $QUERYCTRFILES 

    Log "INFO:Exec SQL to get control files of database."
    OracleExeSql "${DBUSER}" "${DBUSERPWD}" ${QUERYCTRFILES} ${ORATMPINFO} ${DBINSTANCE} 30 0 ${ORA_DB_USER} >> "${LOG_FILE_NAME}" 2>&1
    RET=$?
    if [ $RET != 0 ]; then
        Log "ERROR:query control file database(${DBINSTANCE}) file failed."
        DeleteFile ${QUERYCTRFILES} ${ORATMPINFO}
        exit ${RET}
    fi

    conFiles=`sed -n '/----------/,/^ *$/p' "${ORATMPINFO}" | sed -e '/----------/d' -e '/^ *$/d'`
    DeleteFile ${QUERYCTRFILES} ${ORATMPINFO}

    control_files=$(for p in `echo "${conFiles}"`; do printf \'$p\',; done | sed 's/,$//')
    ShutDownDB ${DBINSTANCE}
    if [ "$DBISCLUSTER" -eq "1" ]; then
        su - ${ORA_DB_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV}srvctl stop database -d ${DBNAME}" >> "${LOG_FILE_NAME}" 2>&1
    fi
}
    
SetCtrlFiles()
{
    echo "startup nomount" > $STARTDBSQLFILE
    echo "alter system set control_files=$control_files scope=spfile;" >> $STARTDBSQLFILE
    echo "shutdown immediate;" >> $STARTDBSQLFILE
    echo "exit" >> $STARTDBSQLFILE

    Log "INFO:Exec SQL to set control files."
    OracleExeSql "${DBUSER}" "${DBUSERPWD}" ${STARTDBSQLFILE} ${ORATMPINFO} ${DBINSTANCE} 600 0 ${ORA_DB_USER} >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ "$RET" -ne "0" ] && [ $RET_CODE -ne $ERROR_ORACLE_NOT_OPEN ] && [ $RET_CODE -ne $ERROR_ORACLE_NOT_MOUNTED ]; then
        Log "ERROR:Exec SQL to set database(${DBINSTANCE}) control files failed"
        DeleteFile ${STARTDBSQLFILE} ${ORATMPINFO}
        exit ${RET_CODE}
    fi
    DeleteFile ${STARTDBSQLFILE} ${ORATMPINFO}
    if [ "$DBISCLUSTER" -eq "1" ]; then
        su - ${ORA_DB_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV}srvctl stop database -d ${DBNAME}" >> "${LOG_FILE_NAME}" 2>&1
    fi
}

CheckBCTAndOpenDB()
{
    # disable CBT
    echo "startup mount;" > $STARTDBSQLFILE
    echo "select status from v\$block_change_tracking;" >> ${STARTDBSQLFILE}
    echo "exit;" >> ${STARTDBSQLFILE}

    OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${STARTDBSQLFILE}" "${ORATMPINFO}" "${DBINSTANCE}" 600 1 "${ORA_DB_USER}" >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    BCT_STATUS=
    if [ "$RET_CODE" -ne "0" ]; then
        Log "query cbt status failed, ret="$RET_CODE"."
    else
        BCT_STATUS="` sed -n '$p' "$ORATMPINFO" `"
    fi
    DeleteFile ${STARTDBSQLFILE} ${ORATMPINFO}

    if [ "${BCT_STATUS}" = "DISABLED" ]; then
        echo "ALTER DATABASE OPEN RESETLOGS;" > ${STARTDBSQLFILE}
    else
        echo "alter database disable BLOCK CHANGE TRACKING;" > ${STARTDBSQLFILE}
        echo "ALTER DATABASE OPEN RESETLOGS;" >> ${STARTDBSQLFILE}
    fi
    # open resetlogs
    Log "Begin to open database resetlogs, BCT_STATUS=$BCT_STATUS."

    OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${STARTDBSQLFILE}" "${ORATMPINFO}" "${DBINSTANCE}" 600 1 "${ORA_DB_USER}" >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    BCT_STATUS=
    if [ ${RET_CODE} -ne 0 ];then
        Log "open resetlogs database-${DBINSTANCE} failed, error=${RET_CODE}, `cat ${ORATMPINFO}`."
        exit ${RET_CODE}
    else
        Log "open resetlogs database-${DBINSTANCE} succ."
        DeleteFile ${STARTDBSQLFILE} ${ORATMPINFO}
    fi
    Log "exec DB-${DBNAME} restore success"
}

RestoreByRman()
{
    Log "begin building RMAN restore script"
    PrepareDBEnv "${MainBackupPath}/ebackup-$DBNAME-pfile.ora" "${DBNAME}"
    
    StartupMount

    RunRestore

    # query DB status
    QueryCtrlFilesFromDB

    # set control files
    SetCtrlFiles

    # Check BCT status and  openDB
    CheckBCTAndOpenDB
}
#********************************restore function define end********************************
CheckInstAuth "${DBINSTANCE}"
AuthType=$?
if [ "$RECOVERORDER" -eq "1" ] && [ "$DBISCLUSTER" -eq "1" ] && [ "$RECOVERTARGET" -eq "2" ]; then
    su - ${ORA_DB_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV}srvctl remove database -d ${DBNAME} -f" >> "${LOG_FILE_NAME}" 2>&1
fi
#********************************restore database begin********************************
# Restoring to Source Host and same instance
if [ $RECOVERTARGET = 0 ]; then
    Log "Restoring to Source Host and same instance"

    # use rman to restore
    if [ "$RECOVERORDER" = "1" ]; then
        Log "Begin to restore first node"

        # create directories
        Log "Creating directories for database files"
        CreateDBDirs "$ADDITIONAL/ctrlfiles"
        CreateDBDirs "$ADDITIONAL/dataguardconffiles"
        CreateDBDirs "$ADDITIONAL/logfiles"
        CreateDBDirs "$ADDITIONAL/dbfiles"

        cp -d -r -p $ADDITIONAL/dbs $IN_ORACLE_HOME >> "${LOG_FILE_NAME}" 2>&1
        RestoreByRman

        DeleteExpiredArchivelog

        # get PDB info
        GetOracleCDBType ${DBINSTANCE}
        ORACLE_IS_CDB=$?

        if [ "${ORACLE_IS_CDB}" = "0" ]; then
            OpenAllPDBs
        fi

        Log "Do restore succ by Rman"  
    else
        Log "Begin to restore ${RECOVERORDER} node"
        StartDB
    fi
    CheckDBIsOpen

fi

# Restoring to a new Host
if [ $RECOVERTARGET = 2 ]; then
    Log "Begin to perform restore to new Host."
    # prepare database pfile
    PrepareConfFile 

    # try to replace ORACLE_HOME and ORACLE_BASE
    ReplaceFileByNewEnv

    # create database path if database is not exist
    PrepareDBEnv "${PFILE_NAME}" "${DBNAME}"

    if [ "$RECOVERORDER" = "1" ]; then
        Log "Begin to restore first node of cluster."

        if [ -z "${RECOVERPATH}" ]; then
            CreateDBDirs ${DATA_FILES}
            CreateDBDirs ${CTL_FILES}
            CreateDBDirs ${LOG_FILES}
            CreateDBDirs "$ADDITIONAL/dataguardconffiles"
        else
            CheckRecoverPathExist
            CopyfilesBySpecificDir ${RECOVERPATH} 0
        fi

        # modify pfile
        ModifyPfile ${RECOVERPATH}
        
        # copy control files
        CopyCtrFiles

        # create spfile from pfile
        CreateSpfile ${RECOVERPATH}
        
        # modify spfile
        ModifySpfileName ${IN_ORACLE_HOME} ${DBINSTANCE}

        # create init${DBINSTANCE}.ora
        CreateInitFile ${RECOVERPATH}
        
        if [ `RDsubstr ${ORA_VERSION} 1 2` -gt 11 ]; then
            ExecDBRestore 0 ${ISEncBK} ${INCAR_NUM} ${COPY_RESTORE}
        else
            ExecDBRestore11 0 ${ISEncBK} ${INCAR_NUM} ${COPY_RESTORE}
        fi
        # get PDB info
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
        Log "Begin to restore other node of cluster." 
        CreateInitFile ${RECOVERPATH}
    fi

    if [ ${DBISCLUSTER} -eq 1 ]; then
        RigsterDBToRAC
        if [ ${AuthType} -eq 0 ]; then
            StartDB
        fi
    else
        CheckDBIsOpen
    fi

fi

if [ ${RECOVERTARGET} -eq 0 ]; then
    Log "Do restore succ to Source Host and same instance"
elif [ ${RECOVERTARGET} -eq 2 ]; then
    AddDBInfo2OraTab
    DeleteFile ${ENV_FILE} ${SCN_DBF_MAX} ${DATA_FILES} ${CTL_FILES} ${LOG_FILES}
    Log "Do restore to new host success."
fi

exit 0

#********************************restore database end********************************
