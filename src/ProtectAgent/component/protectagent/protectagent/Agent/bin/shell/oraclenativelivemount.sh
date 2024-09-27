#!/bin/sh
set +x
#@function: start oracle with livemount.
USAGE="Usage: ./oraclenativelivemount.sh AgentRoot PID"

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
LOG_FILE_NAME="${LOG_PATH}/oraclenativelivemount.log"
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
RESTORESQLFILE="${STMP_PATH}/LIVEMOUNTORACLE${PID}.txt"
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
IN_ORACLE_HOME=`GetValue "${PARAM_CONTENT}" OracleHome`
ASMUSER=`GetValue "${PARAM_CONTENT}" ASMUserName`
ASMUSERPWD=`GetValue "${PARAM_CONTENT}" ASMPassword`
CHANNELS=`GetValue "${PARAM_CONTENT}" Channel`
# asm diskname contain '+' in ASM mode
BACKUP=`GetValue "${PARAM_CONTENT}" DataPath`
ARCHIVE=`GetValue "${PARAM_CONTENT}" LogPath`
METADATAPATH=`GetValue "${PARAM_CONTENT}" MetaDataPath`
RECOVERTARGET=`GetValue "${PARAM_CONTENT}" recoverTarget`
RECOVERPATH=`GetValue "${PARAM_CONTENT}" recoverPath`
RECOVERORDER=`GetValue "${PARAM_CONTENT}" recoverOrder`
ASMSIDNAME=`GetValue "${PARAM_CONTENT}" ASMInstanceName`
DBType=`GetValue "${PARAM_CONTENT}" dbType`
ENCALGO=`GetValue "${PARAM_CONTENT}" EncAlgo`
ENCKEY=`GetValue "${PARAM_CONTENT}" EncKey`
PFILEPID=`GetValue "${PARAM_CONTENT}" pfilePID`
RECOVERNUM=`GetValue "${PARAM_CONTENT}" recoverNum`
ISSTARTDB=`GetValue "${PARAM_CONTENT}" startDb`

test "$DBNAME" = "${ERROR_PARAM_INVALID}"                           && ExitWithError "DBNAME"
test "$DBINSTANCE" = "${ERROR_PARAM_INVALID}"                       && ExitWithError "DBINSTANCE"
test "$DBUSER" = "${ERROR_PARAM_INVALID}"                           && ExitWithError "DBUSER"
test "$IN_ORACLE_HOME" = "${ERROR_PARAM_INVALID}"                   && ExitWithError "IN_ORACLE_HOME"
test "$ASMUSER" = "${ERROR_PARAM_INVALID}"                          && ExitWithError "ASMUSER"
test "$CHANNELS" = "${ERROR_PARAM_INVALID}"                         && ExitWithError "CHANNELS"
test "$BACKUP" = "${ERROR_PARAM_INVALID}"                           && ExitWithError "BACKUP"
test "$ARCHIVE" = "${ERROR_PARAM_INVALID}"                          && ExitWithError "ARCHIVE"
test "$METADATAPATH" = "${ERROR_PARAM_INVALID}"                     && ExitWithError "METADATAPATH"
test "$RECOVERTARGET" = "${ERROR_PARAM_INVALID}"                    && ExitWithError "RECOVERTARGET"
test "$RECOVERPATH" = "${ERROR_PARAM_INVALID}"                      && ExitWithError "RECOVERPATH"
test "$RECOVERORDER" = "${ERROR_PARAM_INVALID}"                     && ExitWithError "RECOVERORDER"
test "$ASMSIDNAME" = "${ERROR_PARAM_INVALID}"                       && ExitWithError "ASMSIDNAME"
test "$DBType" = "${ERROR_PARAM_INVALID}"                           && ExitWithError "DBType"
test "$ENCALGO" = "${ERROR_PARAM_INVALID}"                          && ExitWithError "ENCALGO"
test "$PFILEPID" = "${ERROR_PARAM_INVALID}"                         && ExitWithError "PFILEPID"
test "$RECOVERNUM" = "${ERROR_PARAM_INVALID}"                       && ExitWithError "RECOVERNUM"
test "$ISSTARTDB" = "${ERROR_PARAM_INVALID}"                        && ExitWithError "ISSTARTDB"

BACKUP=`RedirectBackPath ${BACKUP}`
# get first data path for meta data path when there are several mount path
MainBackupPath=
GetMainDataPath

ADDITIONAL="${MainBackupPath}/additional"
BACKUP_TEMP="${MainBackupPath}/tmp"
Log "BACKUPTEMP=${BACKUP_TEMP};ADDITIONAL=$ADDITIONAL;MainBackupPath=${MainBackupPath}"
RECOVERPATH=$MainBackupPath

# initial asm instance name if backup or archive is ASM diskgroup
if [ "${DBType}" = "0" ]; then
    ASMSIDNAME=`ps -ef | grep "asm_...._$ASMSIDNAME" | grep -v "grep" | $MYAWK -F '+' '{print $NF}'`
    ASMSIDNAME=`echo ${ASMSIDNAME} | $MYAWK -F " " '{print $1}'`
    ASMSIDNAME="+"${ASMSIDNAME}
    Log "ASM instance: ${ASMSIDNAME}"
fi

Log "PID=${PID};DBINSTANCE=${DBINSTANCE};DBNAME=${DBNAME};DBUSER=${DBUSER};\
ASMInst=${ASMSIDNAME};ASMUSER=${ASMUSER};IN_ORACLE_HOME=$IN_ORACLE_HOME;\
BACKUP=$BACKUP;ARCHIVE=${ARCHIVE};METADATAPATH=${METADATAPATH};RECOVERORDER=${RECOVERORDER};\
CHANNELS=${CHANNELS};RECOVERPATH=${RECOVERPATH};pfilePID=${PFILEPID};RECOVERNUM=$RECOVERNUM ISSTARTDB=$ISSTARTDB."
GetOracleUser
GetUserShellType ${ORA_DB_USER} ${ORA_GRID_USER}
GetOracleBasePath ${ORA_DB_USER}
GetOracleHomePath ${ORA_DB_USER}
GetOracleVersion ${ORA_DB_USER}
VERSION=`echo $PREVERSION | tr -d '.'`
GetOracleCluster
CheckMountPath

#********************************check parameter valid begin********************************
InstLen=${#DBNAME}
[ "$InstLen" -gt 8 ] && ExitWithErrorCode "$DBNAME is too lenger."

# check instance not exist
CheckInsNotExist
[ $? -ne 0 ] && ExitWithErrorCode "DB already exist" $ERROR_ORACLE_DB_EXIST

# check channels param
CheckParamChannels

# check parameter valid
test -z "$BACKUP"      && ExitWithError "data path"
test -z "$DBINSTANCE"  && ExitWithError "oracle instance name"
CheckDirRWX "$ORA_DB_USER" "$ARCHIVE" || ExitWithError "no archive directory"

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


RemoveSameNameRacDatabase()
{
    DB_NAME_LOWER=`echo ${DBNAME} | tr '[A-Z]' '[a-z]'`
    if [ -d "${IN_ORACLE_BASE}/admin/${DB_NAME_LOWER}" ]; then
        su - ${ORA_DB_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV}mv -f \"${IN_ORACLE_BASE}/admin/${DB_NAME_LOWER}\" \"${IN_ORACLE_BASE}/admin/${DB_NAME_LOWER}_`date '+%Y%m%d%H%M%S'`\""
    fi
    nodeName=`hostname`
    SID_NAME=`su - ${ORA_DB_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV}srvctl status instance -d ${DBNAME} -n ${nodeName}"`
    echo ${SID_NAME} | grep 'PRCD-1120.*PRCR-1001'
    if [ $? -eq 0 ]; then
        return
    fi
    SID_NAME=`echo ${SID_NAME} | $MYAWK '{print $2}'`
    if [ ! -z "${IN_ORACLE_HOME}" ]; then
        if [ -f "${IN_ORACLE_HOME}/dbs/init${SID_NAME}.ora" ]; then
            su - ${ORA_DB_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV}rm -rf ${IN_ORACLE_HOME}/dbs/init${SID_NAME}.ora"
        fi
    fi
}

if [ "$DBISCLUSTER" -eq 1 ]; then
    RemoveSameNameRacDatabase
fi

#********************************check parameter valid end********************************

#********************************prepare for livemount begin********************************
PIT_SCN=`cat $ADDITIONAL/scn_dbf_max | $MYAWK '{print $1}'`

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

DBPW_FILE="${IN_ORACLE_HOME}/dbs/orapw${DBINSTANCE}"
PFILE_NAME="${IN_ORACLE_HOME}/dbs/ebackup-pfile${DBINSTANCE}.ora"
# format: ORACLE_BASE=
#         ORACLE_HOME=
ENV_FILE="${STMP_PATH}/oracle_env${PID}"
SCN_DBF_MAX="${STMP_PATH}/scn_dbf_max${PID}"
DATA_FILES="${STMP_PATH}/datafiles${PID}"
CTL_FILES="${STMP_PATH}/ctlfiles${PID}"
LOG_FILES="${STMP_PATH}/logfiles${PID}"
resetlogs_id=`cat $ADDITIONAL/dbinfo | $MYAWK -F ";" '{print $9}'`
taskType=1

# prepare database pfile
PrepareConfFile

# try to replace ORACLE_HOME and ORACLE_BASE
ReplaceFileByNewEnv

# create database path if database is not exist
PrepareDBEnv "${PFILE_NAME}" "${DBNAME}"

#********************************prepare for livemount end********************************
CheckInstAuth "${DBINSTANCE}"
AuthType=$?
if [ "$RECOVERORDER" -eq "1" ] && [ "$DBISCLUSTER" -ne "0" ]; then
    su - ${ORA_DB_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV}srvctl remove database -d ${DBNAME} -f" >> "${LOG_FILE_NAME}" 2>&1
fi
#********************************livemount begin********************************
if [ -f $ADDITIONAL/livemountOK ] || [ "$RECOVERORDER" != "1" ]; then
    Log "Begin to perform livemount on the other node or this node have been livemount before."
    CreateInitFile ${MainBackupPath}
else
    Log "Begin to perform livemount on the first node."
    # modify pfile
    ModifyPfile ${MainBackupPath}
    # create spfile from pfile
    CreateSpfile ${MainBackupPath}
    # modify spfile
    ModifySpfileName ${IN_ORACLE_HOME} ${DBINSTANCE}
    # create init${DBINSTANCE}.ora for first node
    CreateInitFile ${MainBackupPath}

    # recover database, start database
    if [ `RDsubstr ${ORA_VERSION} 1 2`  -gt 11 ]; then
        ExecDBRestore 1 ${ISEncBK} ${INCAR_NUM} ${COPY_RESTORE} ${ISSTARTDB}
    else
        ExecDBRestore11 1 ${ISEncBK} ${INCAR_NUM} ${COPY_RESTORE} ${ISSTARTDB}
    fi
    if [ ${ISSTARTDB} -eq 1 ]; then
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
    fi
fi

if  [ ${DBISCLUSTER} -ne 1 ] && [ -f $ADDITIONAL/livemountOK ] && [ $ISSTARTDB -eq 1 ]; then
    StartDB
fi

if [ $ISSTARTDB -eq 0 ] && [ $RECOVERORDER -eq 1 ]; then
    ShutDownDB ${DBINSTANCE}
fi

if [ ${DBISCLUSTER} -eq 1 ]; then
    RigsterDBToRAC
    if [ ${AuthType} -eq 0 ] && [ ${ISSTARTDB} -eq 1 ]; then
        StartDB
    fi
elif [ ${DBISCLUSTER} -eq 2 ]; then
    Log "asm add new database"
    su - ${ORA_DB_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV}srvctl add database -d ${DBNAME} -o ${IN_ORACLE_HOME} -p ${SpfileLocation}/ebackup-spfile${DBNAME}.ora" >> "${LOG_FILE_NAME}" 2>&1
else
    if [ $ISSTARTDB -eq 1 ]; then
        CheckDBIsOpen  
    fi
fi
GetOracleInstanceStatus ${DBINSTANCE}

if [ $INSTANCESTATUS = "OPEN" ] || [ $INSTANCESTATUS = "MOUNT" ]; then
    GetLogGroupIDAndFiles ${DBINSTANCE} "$ADDITIONAL/livemount"
fi

if [ ${UPGRADE} -eq 1 ]; then
    NewDBinfo=`$MYAWK -v VERSION=$ORA_VERSION 'BEGIN{FS=OFS=";"}{$10=VERSION}1' $ADDITIONAL/dbinfo`
    echo $NewDBinfo > $ADDITIONAL/dbinfo
fi

# delete tmp file
DeleteFile ${ENV_FILE} ${SCN_DBF_MAX} ${DATA_FILES} ${CTL_FILES} ${LOG_FILES}
[ -f $ADDITIONAL/livemountOK ] || touch $ADDITIONAL/livemountOK
Log "Livemount to new Host succ"
exit 0

#********************************livemount end********************************