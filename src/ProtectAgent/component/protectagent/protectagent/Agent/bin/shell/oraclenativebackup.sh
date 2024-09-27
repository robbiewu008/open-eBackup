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
#@function: backup oracle db and log

AGENT_ROOT_PATH=$1
PID=$2
PARAM_NUM=$3
. "${AGENT_ROOT_PATH}/sbin/agent_sbin_func.sh"
. "${AGENT_ROOT_PATH}/sbin/oraclefunc.sh"
SYSNAME=`uname -s`
SCRIPTPID=$$
echo ${SCRIPTPID} > ${SCRIPTPID_FILE}
#********************************define these for local script********************************
#for GetUserShellType
ORACLE_SHELLTYPE=
GRID_SHELLTYPE=
RDADMIN_SHELLTYPE=
#for log
LOG_FILE_NAME="${LOG_PATH}/oraclenativebackup.log"
#for GetOracleVersion
ORA_VERSION=
PREVERSION=
VERSION=
ORCLVESION="${STMP_PATH}/ORCV${PID}.txt"
SQLEXIT="${STMP_PATH}/EXITSQL${PID}.sql"
#for GetValue

# global var, for kill monitor
PARAM_CONTENT=`ReadInputParam`
test -z "$PARAM_CONTENT"              && ExitWithError "PARAM_FILE"

BACKUPSQLFILE="${STMP_PATH}/BACKUPORACLE${PID}.sql"
ORATMPINFO="${STMP_PATH}/ORATMPINFO${PID}.txt"

SQLRMANFILE="${STMP_PATH}/BackupRman${PID}.sql"
RMANFILERST="${STMP_PATH}/BackupRmanRST${PID}.txt"

#********************************define these for local script********************************
# define for the function GetOraUserByInstName
ORA_DB_USER=
ORA_GRID_USER=

CheckOracleIsInstall
RST=$?
if [ $RST -ne 0 ]; then
    exit $RST
fi
BACKUPLEVEL=

# specific value define
SEPARATOR="      -       "

##############################get input param begin###############################
DBNAME=`GetValue "${PARAM_CONTENT}" AppName`
DBUUID=`GetValue "${PARAM_CONTENT}" DBUUID`
DBINSTANCE=`GetValue "${PARAM_CONTENT}" InstanceName`
DBUSER=`GetValue "${PARAM_CONTENT}" UserName`
DBUSERPWD=`GetValue "${PARAM_CONTENT}" Password`
IN_ORACLE_HOME=`GetValue "${PARAM_CONTENT}" OracleHome`
ASMUSER=`GetValue "${PARAM_CONTENT}" ASMUserName`
ASMUSERPWD=`GetValue "${PARAM_CONTENT}" ASMPassword`
CHANNELS=`GetValue "${PARAM_CONTENT}" Channel`
BACKUP=`GetValue "${PARAM_CONTENT}" DataPath`
ARCHIVE=`GetValue "${PARAM_CONTENT}" LogPath`
METADATAPATH=`GetValue "${PARAM_CONTENT}" MetaDataPath`
# level 0 is full backup, 1 is differential increment , 2 is cumulative increment
LEVEL=`GetValue "${PARAM_CONTENT}" Level`
QOS=`GetValue "${PARAM_CONTENT}" Qos`
StorType=`GetValue "${PARAM_CONTENT}" storType`
DBType=`GetValue "${PARAM_CONTENT}" dbType`
ENCALGO=`GetValue "${PARAM_CONTENT}" EncAlgo`
ENCKEY=`GetValue "${PARAM_CONTENT}" EncKey`
KEEPDAYS=`GetValue "${PARAM_CONTENT}" ArchiveLogKeepDays`

test "$DBNAME" = "${ERROR_PARAM_INVALID}"                           && ExitWithError "DBNAME"
test "$DBUUID" = "${ERROR_PARAM_INVALID}"                           && ExitWithError "DBUUID"
test "$DBINSTANCE" = "${ERROR_PARAM_INVALID}"                       && ExitWithError "DBINSTANCE"
test "$DBUSER" = "${ERROR_PARAM_INVALID}"                           && ExitWithError "DBUSER"
test "$IN_ORACLE_HOME" = "${ERROR_PARAM_INVALID}"                   && ExitWithError "IN_ORACLE_HOME"
test "$ASMUSER" = "${ERROR_PARAM_INVALID}"                          && ExitWithError "ASMUSER"
test "$CHANNELS" = "${ERROR_PARAM_INVALID}"                         && ExitWithError "CHANNELS"
test "$BACKUP" = "${ERROR_PARAM_INVALID}"                           && ExitWithError "BACKUP"
test "$ARCHIVE" = "${ERROR_PARAM_INVALID}"                          && ExitWithError "ARCHIVE"
test "$METADATAPATH" = "${ERROR_PARAM_INVALID}"                     && ExitWithError "METADATAPATH"
test "$LEVEL" = "${ERROR_PARAM_INVALID}"                            && ExitWithError "LEVEL"
test "$QOS" = "${ERROR_PARAM_INVALID}"                              && ExitWithError "QOS"
test "$StorType" = "${ERROR_PARAM_INVALID}"                         && ExitWithError "StorType"
test "$DBType" = "${ERROR_PARAM_INVALID}"                           && ExitWithError "DBType"
test "$ENCALGO" = "${ERROR_PARAM_INVALID}"                          && ExitWithError "ENCALGO"
test "$KEEPDAYS" = "${ERROR_PARAM_INVALID}"                         && ExitWithError "KEEPDAYS"

NFSPATH=`echo ${BACKUP} | $MYAWK -F ";" '{print $1}'`
BACKUP=`RedirectBackPath ${BACKUP}`
ASMSIDNAME=`GetValue "${PARAM_CONTENT}" ASMInstanceName`
test "$ASMSIDNAME" = "${ERROR_PARAM_INVALID}"                         && ExitWithError "ASMSIDNAME"

Log "SCRIPTPID=${SCRIPTPID};INNERPID=${PID};DBINSTANCE=${DBINSTANCE};DBName=${DBNAME};DBUSER=${DBUSER};\
ASMInst=${ASMSIDNAME};ASMUSER=${ASMUSER};IN_ORACLE_HOME=$IN_ORACLE_HOME;BACKUP=$BACKUP;ARCHIVE=$ARCHIVE;level=${LEVEL};\
channel=${CHANNELS};QOS=$QOS;DBType=${DBType};KEEPDAYS=${KEEPDAYS}"

MainBackupPath=`echo ${BACKUP} | $MYAWK -F ";" '{print $1}'`
ADDITIONAL="${MainBackupPath}/additional"
BACKUP_TEMP="${MainBackupPath}/tmp"
test -z "$BACKUP"      && ExitWithError "data path"
test -z "$ARCHIVE"     && ExitWithError "log path"
test -z "$ADDITIONAL"  && ExitWithError "additional path"
test -z "$DBINSTANCE"  && ExitWithError "oracle instance name"
test -z "${MainBackupPath}" && ExitWithError "main data path"
LOG_IS_BACKED_UP="$LOG_IS_VALID and name like '$MainBackupPath/log/arch_%'"

##############################get input param end###############################

#********************************check host enviroment begin********************************
GetOracleUser
GetUserShellType ${ORA_DB_USER} ${ORA_GRID_USER}
GetOracleBasePath ${ORA_DB_USER}
GetOracleHomePath ${ORA_DB_USER}
GetOracleVersion ${ORA_DB_USER}
VERSION=`echo $PREVERSION | tr -d '.'`
GetOracleCluster
DBINSTANCE=`GetRealInstanceName ${DBINSTANCE}`
Log "Check cluster [DBISCLUSTER=${DBISCLUSTER}], DBINSTANCE=${DBINSTANCE}."
CheckSqlPlusStatus
CheckRmanStatus
CheckMountPath

ISEncBK=0
if [ ! -z "${ENCALGO}" ] && [ ! -z "${ENCKEY}" ]; then
    ISEncBK=1
fi

if [ ${ISEncBK} -eq 1 ]; then
    RMAN_ENC_SECTION="configure encryption for database on; configure encryption algorithm '${ENCALGO}'; set encryption on identified by \"${ENCKEY}\" only; set decryption identified by \"${ENCKEY}\";"
fi

QueryFromVDatabase()
{
    echo "set linesize 999" > $SqlCMD
    echo "COL LOG_MODE FORMAT a20" >> $SqlCMD
    echo "COL DB_UNIQUE_NAME FORMAT a20" >> $SqlCMD
    echo "select a.LOG_MODE, a.DBID, a.DB_UNIQUE_NAME, a.OPEN_MODE, b.INCARNATION#, b.RESETLOGS_ID from 
    v\$database a, v\$database_incarnation b where a.LAST_OPEN_INCARNATION#=b.INCARNATION# and b.STATUS='CURRENT';" >> "${SqlCMD}"
    echo "exit" >> "${SqlCMD}"
    OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${SqlCMD}" "${SqlRST}" "${DBINSTANCE}" 30 1 "${ORA_DB_USER}" >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ ${RET_CODE} -ne 0 ]; then
        Log "QueryFromVDatabase failed, ret=${RET_CODE}"
        DeleteFile ${SqlCMD} ${SqlRST}
        exit $RET_CODE
    fi
    logMode=`cat ${SqlRST} | $MYAWK '{print $1}'`
    DBID=`cat ${SqlRST} | $MYAWK '{print $2}'`
    UNIQ_DBNAME=`cat ${SqlRST} | $MYAWK '{print $3}'`
    open_mode=`cat ${SqlRST} | $MYAWK '{print $4$5}'`
    INCARNATION_NUMBER=`cat ${SqlRST} | $MYAWK '{print $6}'`
    resetlogs_id=`cat ${SqlRST} | $MYAWK '{print $7}'`
    DeleteFile ${SqlCMD} ${SqlRST}
}
QueryFromVDatabase
if [ "${logMode}" != "ARCHIVELOG" ]; then
    Log "Archive Mode=No Archive Mode, check archive mode failed."
    exit ${ERROR_ORACLE_NOARCHIVE_MODE}
fi
#********************************check host enviroment end********************************

#********************************prepare for backup begin********************************
Log "create directory"
CreateDir $BACKUP
CreateDir $BACKUP_TEMP
CreateDir $ARCHIVE
CreateDir $ADDITIONAL
CreateDir $ADDITIONAL/dbs
CreateDir $MainBackupPath/log
chown -R root $ADDITIONAL
chmod 700 $ADDITIONAL
Log "check directory"
CheckDirRWX "$ORA_DB_USER" "$BACKUP"      || ExitWithError "check data path"
CheckDirRWX "$ORA_DB_USER" "$BACKUP_TEMP" || ExitWithError "check backup tmp path"
CheckDirRWX "$ORA_DB_USER" "$ARCHIVE"     || ExitWithError "check log path"
CheckDirRWX "$ORA_DB_USER" "$IN_ORACLE_HOME"  || ExitWithError "check oracle home path"

Log "check parameter"
if [ "${dbType}" = "0" ]; then
    df -h | grep " ${METADATAPATH}$" >> "${LOG_FILE_NAME}" 2>&1
    if [ $? -ne 0 ]; then
        Log "${METADATAPATH} is not mounted."
        exit 1
    fi
fi
CheckParamChannels

Log "check backup level"
if [ -f "$ADDITIONAL/dbinfo" ]; then
    OLDDBID=`cat $ADDITIONAL/dbinfo | $MYAWK -F ";" '{print $1}'`
    if [ $DBID -ne $OLDDBID ]; then
        Log "DBID is different from the last backup, set level=0 full backup"
        LEVEL=0
    fi
fi
if [ "$LEVEL" != "0" ]; then
    if [ "$LEVEL" != "1" ] && [ "$LEVEL" != "2" ]; then
        LEVEL=0
        Log "set level=0 full backup"
    fi
    if [ "$LEVEL" = "1" ] || [ "$LEVEL" = "2" ]; then
        if [ ! -f "$ADDITIONAL/first_backup_success" ]; then 
            LEVEL=0
            Log "the last full backup not success must do full backup"
        fi
    fi
fi

GetFromSCN ${DBINSTANCE}
if [ -z "${from_scn}" ]; then
    LEVEL=0
fi

# All backup files must be deleted during full backup.
if [ ${LEVEL} -eq 0 ]; then
    Log "Before full backup, delete all backup data file."
    su - ${ORA_DB_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV}/bin/rm -f $MainBackupPath/**/FNO-*_TS-*.dbf " >> ${LOG_FILE_NAME} 2>&1
    su - ${ORA_DB_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV}/bin/rm -f $MainBackupPath/**/fno-*_ts-*.dbf " >> ${LOG_FILE_NAME} 2>&1
    su - ${ORA_DB_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV}/bin/rm -f $MainBackupPath/**/.FNO-*_TS-*.dbf " >> ${LOG_FILE_NAME} 2>&1
    su - ${ORA_DB_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV}/bin/rm -f $MainBackupPath/**/.fno-*_ts-*.dbf " >> ${LOG_FILE_NAME} 2>&1
    su - ${ORA_DB_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV}/bin/rm -f $MainBackupPath/fno-*_ts-*.dbf " >> ${LOG_FILE_NAME} 2>&1
    su - ${ORA_DB_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV}/bin/rm -f $MainBackupPath/FNO-*_TS-*.dbf " >> ${LOG_FILE_NAME} 2>&1
fi

Log "write bakup metadata"
echo ${DBID}";"${UNIQ_DBNAME}";"${DBINSTANCE}";"${ASMUSER}";"${ASMSIDNAME}";"\
${DBISCLUSTER}";"${VERSION}";"${INCARNATION_NUMBER}";"${resetlogs_id}";"${ORA_VERSION} > $ADDITIONAL/dbinfo
echo "ORACLE_BASE=${IN_ORACLE_BASE}" > $ADDITIONAL/env_file
echo "ORACLE_HOME=${IN_ORACLE_HOME}" >> $ADDITIONAL/env_file
#********************************prepare for backup end********************************

#********************************backup data file begin********************************
CrossCheckDBF()
{
    Log "Begin to crosscheck datafilecopy"
    local UpperDBNAME=$(echo $DBNAME | tr '[a-z]' '[A-Z]')
    SQLRmanCrossCheck="${STMP_PATH}/RmanCrossCheck${PID}.sql"
    RmanCrossCheckRST="${STMP_PATH}/CrossCheckRST${PID}.txt"

    echo "crosscheck datafilecopy tag 'EBACKUP-$UpperDBNAME-DATA';" > ${SQLRmanCrossCheck}
    echo "exit;" >> ${SQLRmanCrossCheck}

    Log "INFO:Begin to crosscheck datafile"
    RmanExeScript "${DBUSER}" "${DBUSERPWD}" "${SQLRmanCrossCheck}" "${RmanCrossCheckRST}" "${DBINSTANCE}" 180 "${ORA_DB_USER}" >> "${LOG_FILE_NAME}" 2>&1
    RMAN_RET_CODE=$?
    DeleteFile ${SQLRmanCrossCheck} ${RmanCrossCheckRST}
    if [ ${RMAN_RET_CODE} -ne 0 ]; then
        Log "ERROR:exec crosscheck datafilecopy failed, ret=${RMAN_RET_CODE}"
    fi
}
CrossCheckDBF

GetBackupFromSCN()
{
    MinSysSCNSQL="${STMP_PATH}/MinSysSCN${PID}.sql"
    MinSysSCNRST="${STMP_PATH}/MinSysSCNRST${PID}.txt"
    echo 'select max(CHECKPOINT_CHANGE#) as scn from v$datafile;' > "$MinSysSCNSQL"
    echo "exit" >> "$MinSysSCNSQL"

    OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${MinSysSCNSQL}" "${MinSysSCNRST}" "${DBINSTANCE}" 30 1 "${ORA_DB_USER}" >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ ${RET_CODE} -ne 0 ]; then
        Log "get system min data file scn failed, `cat ${MinSysSCNRST}`."
        DeleteFile "${MinSysSCNSQL}" "${MinSysSCNRST}"
        exit $RET_CODE
    fi
    datafileMinSCN=`cat ${MinSysSCNRST} | tr -d " "`
    Log "Get system min data file scn is $datafileMinSCN."
    DeleteFile "${MinSysSCNSQL}" "${MinSysSCNRST}"
}
GetBackupFromSCN
test -z "$datafileMinSCN" && ExitWithError "min data file scn"
    
Log "INFO:create pfile by sqlplus"
echo "create pfile='${MainBackupPath}/ebackup-$DBNAME-pfile.ora' from spfile;" > "${SqlCMD}"
echo "exit" >> "$SqlCMD"
OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${SqlCMD}" "${SqlRST}" "${DBINSTANCE}" 60 0 "${ORA_DB_USER}" >> "${LOG_FILE_NAME}" 2>&1
RET_CODE=$?
if [ ${RET_CODE} -ne 0 ]; then
    Log "create pfile failed, ret=${RET_CODE}."
    DeleteFile ${SqlCMD} ${SqlRST}
    exit ${RET_CODE}
fi
DeleteFile ${SqlCMD} ${SqlRST}

if [ $LEVEL -eq 0 ] && [ -f "$ADDITIONAL/first_backup_success" ]; then
    rm -rf "$ADDITIONAL/first_backup_success"
fi
Log "INFO:Running RMAN to backup"
BuildFullbackupSql()
{
    FullbackupSql="${STMP_PATH}/FullbackupSql${PID}.sql"
    echo "set linesize 999" > ${SqlCMD}
    echo "col tsName for a30" >> ${SqlCMD}
    echo "col tsFile for a520" >> ${SqlCMD}
    if [ `RDsubstr ${ORA_VERSION} 1 2` -gt 11 ]; then
        echo "SELECT f.bytes/1024/1024 fSize, f.File# fNo, t.Name tsName, f.Name tsFile \
        FROM V\$TABLESPACE t, V\$DATAFILE f WHERE t.TS# = f.TS# and t.CON_ID = f.CON_ID order by bytes desc;" >> ${SqlCMD}
    else
        echo "SELECT f.bytes/1024/1024 fSize, f.File# fNo, t.Name tsName, f.Name tsFile \
        FROM V\$TABLESPACE t, V\$DATAFILE f WHERE t.TS# = f.TS# order by bytes desc;" >> ${SqlCMD}
    fi
    echo "exit;" >> ${SqlCMD}
    OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${SqlCMD}" "${SqlRST}" "${DBINSTANCE}" 60 1 "${ORA_DB_USER}" >> ${LOG_FILE_NAME} 2>&1
    Sqlcode=$?
    if [ ${Sqlcode} -ne 0 ]; then
        Log "Get database-${DBINSTANCE} datafile list failed, sqlpluserror=${Sqlcode}."
        DeleteFile ${SqlCMD} ${SqlRST}
        echo "    backup as copy incremental level 0 tag 'EBACKUP-${DBNAME}-DATA' database format '${MainBackupPath}/FNO-%f_TS-%N.dbf';" > ${FullbackupSql}
        return
    fi
    dbfNum=`cat ${SqlRST} | wc -l`
    local line=
    local pathNum=`echo ${BACKUP} | sed 's/;/ /g' | wc -w`
    local index_datafile=0
    local index_path=
    if [ ${LEVEL} -eq 0 ]; then
        if [ ${SYSNAME} = "AIX" ]; then
            echo "    backup as copy incremental level 0 tag 'EBACKUP-${DBNAME}-DATA' " > ${FullbackupSql}
        else
            echo -n "    backup as copy incremental level 0 tag 'EBACKUP-${DBNAME}-DATA' " > ${FullbackupSql}
        fi
        initSize=`cat ${FullbackupSql} | wc -c`
        curSize=${initSize}
        while read line; do
            local fNo="`echo $line | ${MYAWK} '{print $2}'`"
            index_path=`expr ${index_datafile} % ${pathNum} + 1`
            index_datafile=`expr ${index_datafile} + 1`
            local path=`echo ${BACKUP} | cut -d ";" -f${index_path}`
            CreateDir "${path}/${fNo}"

            tmpStr="    (datafile ${fNo} format '${path}/${fNo}/FNO-%f_TS-%N.dbf')"
            tmpStrSize=`echo ${tmpStr} | wc -c`
            totle=`expr $curSize + $tmpStrSize `
            if [ $totle -gt 4096 ]; then
                echo ";" >> ${FullbackupSql}
                if [ ${SYSNAME} = "AIX" ]; then 
                    echo "    backup as copy incremental level 0 tag 'EBACKUP-${DBNAME}-DATA' " >> ${FullbackupSql}
                else
                    echo -n "    backup as copy incremental level 0 tag 'EBACKUP-${DBNAME}-DATA' " >> ${FullbackupSql}
                fi
                curSize=${initSize}
            fi
            if [ ${SYSNAME} = "AIX" ]; then
                echo "\n${tmpStr}" >> ${FullbackupSql}
            else
                echo -en "\n${tmpStr}" >> ${FullbackupSql}
            fi
            curSize=`expr $curSize + $tmpStrSize `
        done < ${SqlRST}
        echo ";" >> ${FullbackupSql}
    else
        if [ ${SYSNAME} = "AIX" ]; then
            echo "    backup as copy incremental level 0 tag 'EBACKUP-${DBNAME}-DATA' " > ${FullbackupSql}
        else
            echo -n "    backup as copy incremental level 0 tag 'EBACKUP-${DBNAME}-DATA' " > ${FullbackupSql}
        fi
        initSize=`cat ${FullbackupSql} | wc -c`
        curSize=${initSize}
        while read line; do
            local fNo="`echo $line | ${MYAWK} '{print $2}'`"
            local fsFile="`echo $line | ${MYAWK} '{print $4}'`"
            grep "${fsFile}" ${ADDITIONAL}/dbfiles
            if [ $? -ne 0 ]; then
                index_path=`expr ${index_datafile} % ${pathNum} + 1`
                index_datafile=`expr ${index_datafile} + 1`
                local path=`echo ${BACKUP} | cut -d ";" -f${index_path}`
                CreateDir "${path}/${fNo}"

                tmpStr="    (datafile ${fNo} format '${path}/${fNo}/FNO-%f_TS-%N.dbf')"
                tmpStrSize=`echo ${tmpStr} | wc -c`
                totle=`expr $curSize + $tmpStrSize `
                if [ $totle -gt 4096 ]; then
                    echo ";" >> ${FullbackupSql}
                    if [ ${SYSNAME} = "AIX" ]; then
                        echo "    backup as copy incremental level 0 tag 'EBACKUP-${DBNAME}-DATA' " >> ${FullbackupSql}
                    else
                        echo -n "    backup as copy incremental level 0 tag 'EBACKUP-${DBNAME}-DATA' " >> ${FullbackupSql}
                    fi
                    curSize=${initSize}
                fi
                if [ ${SYSNAME} = "AIX" ]; then
                    echo "\n${tmpStr}" >> ${FullbackupSql}
                else
                    echo -en "\n${tmpStr}" >> ${FullbackupSql}
                fi
                curSize=`expr $curSize + $tmpStrSize `
            fi
        done < ${SqlRST}
        if [ ${curSize} -eq ${initSize} ]; then
            DeleteFile ${FullbackupSql}
        else
            echo ";" >> ${FullbackupSql}
        fi
    fi
    DeleteFile ${SqlCMD} ${SqlRST}
}
START_BACKUP_TIME=`date "+%Y-%m-%d_%H:%M:%S"`
su - ${ORA_DB_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV}rm -rf $MainBackupPath/log/*"
BuildRunRmanScript()
{
    Log "INFO:Running RMAN to backup"
    BuildFullbackupSql
    DeleteFile ${SQLRMANFILE}
    if [ `RDsubstr ${ORA_VERSION} 1 2`  -gt 12 ]; then
        echo "alter session set events 'trace[krb.*] disk disable, memory disable';" > ${SQLRMANFILE}
    fi
    echo "configure backup optimization off;" >> ${SQLRMANFILE}
    echo "configure controlfile autobackup off;" >> ${SQLRMANFILE}
    echo "set nocfau;" >> ${SQLRMANFILE}
    echo "configure maxsetsize to unlimited;" >> ${SQLRMANFILE}
    if [ ${ISEncBK} -eq 0 ]; then
        echo "configure encryption for database off;" >> ${SQLRMANFILE}
    fi
    echo "RUN" >> ${SQLRMANFILE}
    echo "{" >> ${SQLRMANFILE}
    echo "    SET COMMAND ID TO 'ProtectAgent_Backup';" >> ${SQLRMANFILE}
    if [ ${LEVEL} -eq 0 ]; then
        echo "    change archivelog like '%/epoch-scn_%/arch_%' uncatalog;" >> ${SQLRMANFILE}
    fi
    index=1
    while [ $index -le ${CHANNELS} ]; do
        if [ -z "${QOS}" ] || [ "${QOS}" = "0" ]; then
            echo "    allocate channel eBackup`printf "%02d" $index` type disk;" >> ${SQLRMANFILE}
        else
            if [ ${dbfNum} -ge ${CHANNELS} ] || [ ${dbfNum} -eq 0 ]; then
                realQOS=`expr ${QOS} \* 1024 / ${CHANNELS}`
            else
                realQOS=`expr ${QOS} \* 1024 / ${dbfNum}`
            fi
            echo "    allocate channel eBackup`printf "%02d" $index` type disk rate `printf "%d" ${realQOS}`k;" >> ${SQLRMANFILE}
        fi
        index=`expr $index + 1`
    done
    echo "    configure device type disk parallelism ${CHANNELS};" >> ${SQLRMANFILE}
    echo "    backup spfile format '${MainBackupPath}/spfile.bs' tag 'EBACKUP-${DBNAME}-SPFILE' reuse;" >> ${SQLRMANFILE}
    if [  -f ${FullbackupSql} ]; then
        cat ${FullbackupSql} >> ${SQLRMANFILE}
        DeleteFile ${FullbackupSql}
    fi
    if [ ${LEVEL} -eq 1 ]; then
        echo "    backup incremental level 1 cumulative for recover of copy with tag 'EBACKUP-$DBNAME-DATA' database format '$BACKUP_TEMP/%T_%U';" >> ${SQLRMANFILE}
        echo "    recover copy of database with tag 'EBACKUP-$DBNAME-DATA';" >> ${SQLRMANFILE}
        echo "    delete noprompt backup tag 'EBACKUP-$DBNAME-DATA';" >> ${SQLRMANFILE}
    elif [ ${LEVEL} -eq 2 ]; then
        echo "    backup incremental level 1 for recover of copy with tag 'EBACKUP-$DBNAME-DATA' database format '$BACKUP_TEMP/%T_%U';" >> ${SQLRMANFILE}
        echo "    recover copy of database with tag 'EBACKUP-$DBNAME-DATA';" >> ${SQLRMANFILE}
        echo "    delete noprompt backup tag 'EBACKUP-$DBNAME-DATA';" >> ${SQLRMANFILE}
    fi
    echo "    DELETE FORCE NOPROMPT ARCHIVELOG like '${MainBackupPath%/*}/%/log/arch_%';" >> ${SQLRMANFILE}
    if [ "${open_mode}" != "READONLY" ];then
        echo "    sql 'alter system archive log current';" >> ${SQLRMANFILE}
    fi
    echo "    backup as copy archivelog from scn $datafileMinSCN format '$MainBackupPath/log/%U.log' reuse;" >> ${SQLRMANFILE}
    echo "    backup as copy current controlfile format '$MainBackupPath/controlfile.ctl' tag 'EBACKUP-$DBNAME-CTL' reuse;" >> ${SQLRMANFILE}
    echo "}" >> ${SQLRMANFILE}
}
BuildRunRmanScript
RmanExeScript "${DBUSER}" "${DBUSERPWD}" ${SQLRMANFILE} ${RMANFILERST} ${DBINSTANCE} -1 ${ORA_DB_USER} 1 >> "${LOG_FILE_NAME}" 2>&1
RMAN_RET_CODE=$?
if [ ${RMAN_RET_CODE} -ne 0 ]; then
    Log "Backup database-${DBINSTANCE} failed, error=${RMAN_RET_CODE}."
    cat ${RMANFILERST} >> ${LOG_FILE_NAME}
    DeleteFile ${SQLRMANFILE} ${RMANFILERST}
    if [ "${ERROR_SCRIPT_EXEC_FAILED}" = "${RMAN_RET_CODE}" ]; then
        exit ${ERROR_BACKUP_FERROR_ORACLE_BACKUP_FAILEDAILED}
    else
        exit ${RMAN_RET_CODE}
    fi
else
    Log "Backup database-${DBINSTANCE} succ."
    DeleteFile ${SQLRMANFILE} ${RMANFILERST}
fi
END_BACKUP_TIME=`date "+%Y-%m-%d_%H:%M:%S"`
#********************************backup data file end********************************
Log "backup start time is ${START_BACKUP_TIME} end time is ${END_BACKUP_TIME}"
#********************************after backup data file begin******************************
GetCopyDataSCN()
{
    DataCopyPath=${MainBackupPath%/*}
    DataCopyPath=${DataCopyPath%/*}
    Log "Begin to GetCopyDataSCN"
    echo "set linesize 300;" > "$SQLRMANFILE"
    echo "select max(CHECKPOINT_CHANGE#), timestamp_to_scn(max(COMPLETION_TIME)), to_char(max(COMPLETION_TIME), $TIME_FORMAT) \
    from v\$datafile_copy where (status = 'A') and name like '${DataCopyPath}%.dbf';" >> "$SQLRMANFILE"
    echo "exit" >> "$SQLRMANFILE"
    OracleExeSql "${DBUSER}" "${DBUSERPWD}" ${SQLRMANFILE} ${RMANFILERST} ${DBINSTANCE} 30 1 ${ORA_DB_USER} >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ ${RET_CODE} -ne 0 ] || [ ! -s ${RMANFILERST} ]; then
        Log "GetCopyDatafileSCN  failed,ret=${RET_CODE}"
        DeleteFile "${SQLRMANFILE}" "${RMANFILERST}"
        exit $RET_CODE
    fi

    copyBeginSCN=`cat ${RMANFILERST} | sed -n '1p' | $MYAWK '{print $1}'`
    dbfSCN=`cat ${RMANFILERST} | sed -n '1p' | $MYAWK '{print $2}'`
    if [ ${copyBeginSCN} -gt ${dbfSCN} ]; then
        Log "COMPLETION_TIME(${dbfSCN}), is less than CHECKPOINT_CHANGE#(${copyBeginSCN})."
        dbfSCN=${copyBeginSCN}
    fi
    dbfTime=`cat ${RMANFILERST} | sed -n '1p' | $MYAWK '{print $3}'`
    dbfTimeStamp=`AddUnixTimestamp $dbfTime`
    echo "$dbfSCN $dbfTime $dbfTimeStamp" > $ADDITIONAL/scn_dbf_max
    Log "dbfSCN=${dbfSCN}  dbfTime=${dbfTime}  dbfTimeStamp=${dbfTimeStamp}"
    DeleteFile "${SQLRMANFILE}" "${RMANFILERST}"
    test -z "$dbfSCN"      && ExitWithError "dbfSCN"
    test -z "$dbfTimeStamp"  && ExitWithError "dbfTimeStamp"
}
GetCopyDataSCN
echo "databackuprst;${dbfSCN};${dbfTimeStamp};${resetlogs_id}" > "${RESULT_FILE}"
Log "databackuprst;${dbfSCN};${dbfTimeStamp};${resetlogs_id}"
echo "databackuprst;${dbfSCN};${dbfTimeStamp};${resetlogs_id}" > "${ARCHIVE}/last_backup_scn"

Log "delete overdue archive log"
DeleteOverdueArchiveLog()
{
    if [ ${KEEPDAYS} -gt 0 ] 2>/dev/null ; then
        echo ${KEEPDAYS}  > ${ADDITIONAL}/archivelogKeepDays
    fi
    KEEPDAYS=`cat ${ADDITIONAL}/archivelogKeepDays`
    if [ ${KEEPDAYS} -gt 0 ] 2>/dev/null ; then
    
        local DateTime=`date "+%Y-%m-%d_%H:%M:%S"`
        DateTime=`AddUnixTimestamp ${DateTime}`
        KEEPDAYS=`expr $KEEPDAYS \* 86400`
        DateTime=`expr ${DateTime} - $KEEPDAYS`
        DateTime=`UnixTimestampToDate ${DateTime}`
        Log "Begin DeleteOverdueArchiveLog: ${DateTime}"

        local sqlFile="${STMP_PATH}/DeleteOverdueArchiveLog${PID}.sql"
        local rstFile="${STMP_PATH}/DeleteOverdueArchiveLog${PID}.txt"
        echo "crosscheck archivelog all;" > $sqlFile
        echo "delete noprompt archivelog until time \"to_date('${DateTime}','YYYY-MM-DD hh24:mi:ss')\" like '${ARCHIVE}/%';" >> $sqlFile
        echo "exit;" >> $sqlFile

        RmanExeScript "${DBUSER}" "${DBUSERPWD}" ${sqlFile} ${rstFile} ${DBINSTANCE} 600 ${ORA_DB_USER} >> "${LOG_FILE_NAME}" 2>&1
        RET_CODE=$?
        if [ ${RET_CODE} -ne 0 ];then
            Log "DeleteOverdueArchiveLog of database ${DBNAME} failed, error=${RET_CODE}, `cat ${rstFile}`."
            DeleteFile ${sqlFile} ${rstFile}
            WriteWarnInfoToFile "dme_databases_deleteoverduearchivelog_failed_label" "${ERROR_DEL_ARCHIVELOG_FAILED}"
        else
            Log "DeleteOverdueArchiveLog of database ${DBNAME} succ."
            DeleteFile ${sqlFile} ${rstFile}

            UpdateArciveLogRange
            logbackuprst=`GetArciveLogRange`
            echo "${logbackuprst}" >> "${RESULT_FILE}"
            Log "${logbackuprst}"
        fi
    fi
}
DeleteOverdueArchiveLog

#get taskreslut avg_speed and total size
GetLastRmanTaskSpeed
echo "taskavgspeed;${TaskSpeed}" >> "${RESULT_FILE}"
Log "taskavgspeed;${TaskSpeed}" 
echo "backupsize;${BackupSize}" >> "${RESULT_FILE}"
Log "backupsize;${BackupSize}" 

GetPfileParam()
{
    Log "Get database pfile parameter"
    local PFILE_PATH="${MainBackupPath}/ebackup-$DBNAME-pfile.ora"
    if [ ! -f ${PFILE_PATH} ]; then
        Log "Error pfile is not exit "
        exit 1
    fi
    ret=`cp -d -f ${PFILE_PATH} ${STMP_PATH}/tempfile${PID}` >> "${LOG_FILE_NAME}" 2>&1
    if [ ${rec} -ne 0 ]; then
        LOG "copy pfile is error "
        exit 1
    fi  
    tempfilepath="tempfile${PID}"
}

GetPfileParam
echo "pfile;${tempfilepath}" >> "${RESULT_FILE}"
Log "pfile;${tempfilepath}" 

echo "BackupLevel;${LEVEL}" >> "${RESULT_FILE}"
Log "BackupLevel; ${LEVEL}"

Log "Recording database control files"
GetDBfiles ${DBINSTANCE} 0

Log "Recording database spfile"
GetDBfiles ${DBINSTANCE} 1

Log "Recording database Data Guard configuration file"
GetDBfiles ${DBINSTANCE} 2

Log "Recording database data files and tablespce"
GetFilePathAndTableSpace ${DBINSTANCE} 0

Log "Recording database logfiles and group"
GetLogGroupIDAndFiles ${DBINSTANCE} $ADDITIONAL

Log "Copying some additional info"
GenerateAdditionalInfo()
{
    DBpwfile=`su - ${ORA_DB_USER} -c "${EXPORT_ORACLE_ENV}srvctl config database -d ${DBNAME}" | grep "Password file" | $MYAWK -F ":" '{print $2}' | $MYAWK '{gsub(/^\s+|\s+$/, "");print}'`
    if [ -z "${DBpwfile}" ]; then
        cp -d -r -p $IN_ORACLE_HOME/dbs/orapw${DBINSTANCE} $ADDITIONAL/dbs/orapw${DBNAME} >> "${LOG_FILE_NAME}" 2>&1
    elif [ "`RDsubstr ${DBpwfile} 1 1`" = "+" ]; then
        su - ${ORA_GRID_USER} -c "${EXPORT_GRID_ENV}export ORACLE_SID=${ASMSIDNAME}; rm -rf ${TMP_PATH}/orapw${DBNAME}; echo pwcopy ${DBpwfile} ${TMP_PATH}/orapw${DBNAME} | asmcmd" >> ${LOG_FILE_NAME} 2>& 1
        mv ${TMP_PATH}/orapw${DBNAME} $ADDITIONAL/dbs
    else
        cp -d -r -p ${DBpwfile} $ADDITIONAL/dbs/orapw${DBNAME} >> "${LOG_FILE_NAME}" 2>&1
    fi

    mkdir -p $ADDITIONAL/netadmin >> "${LOG_FILE_NAME}" 2>&1
    cp -d -r -p $IN_ORACLE_HOME/network/admin/* $ADDITIONAL/netadmin >> "${LOG_FILE_NAME}" 2>&1

    if [ $RMAN_RET_CODE -eq 0 ]; then
        touch $ADDITIONAL/ok >> "${LOG_FILE_NAME}" 2>&1
    fi
}
GenerateAdditionalInfo

if [ $LEVEL -eq 0 ] && [ ! -f "$ADDITIONAL/first_backup_success" ]; then
    touch "$ADDITIONAL/first_backup_success"
fi

GetDirFileinfo $NFSPATH
echo "filelist;filelist${PID}" >> "${RESULT_FILE}"
Log "filelist;filelist${PID}"

Log "Do bakcup success."

#chmod 640 RESULT_FILE
if [ -f "${RESULT_FILE}" ]; then
    chmod 640 "${RESULT_FILE}"
fi

exit $RMAN_RET_CODE
#********************************after backup data file end******************************
