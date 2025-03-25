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
#@function: backup oracle archive log

AGENT_ROOT_PATH=$1
PID=$2
PARAM_NUM=$3
. "${AGENT_ROOT_PATH}/sbin/agent_sbin_func.sh"
. "${AGENT_ROOT_PATH}/sbin/oraclefunc.sh"

#********************************define these for local script********************************
#for GetUserShellType
ORACLE_SHELLTYPE=
GRID_SHELLTYPE=
RDADMIN_SHELLTYPE=
#for log
LOG_FILE_NAME="${LOG_PATH}/oraclenativearchlogback.log"
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

SQLRMANFILE="${STMP_PATH}/RmanLogBackup${PID}.sql"
RMANFILERST="${STMP_PATH}/RmanLogBackupRST${PID}.txt"

#********************************define these for local script********************************
# define for the function GetOraUserByInstName
ORA_DB_USER=
ORA_GRID_USER=

CheckOracleIsInstall
RST=$?
if [ $RST -ne 0 ]; then
    exit $RST
fi

# specific value define
SEPARATOR="      -       "

#############################################################
DBNAME=`GetValue "${PARAM_CONTENT}" AppName`
DBUUID=`GetValue "${PARAM_CONTENT}" DBUUID`
DBINSTANCE=`GetValue "${PARAM_CONTENT}" InstanceName`
DBUSER=`GetValue "${PARAM_CONTENT}" UserName`
DBUSERPWD=`GetValue "${PARAM_CONTENT}" Password`
IN_ORACLE_HOME=`GetValue "${PARAM_CONTENT}" OracleHome`
ASMUSER=`GetValue "${PARAM_CONTENT}" ASMUserName`
ASMUSERPWD=`GetValue "${PARAM_CONTENT}" ASMPassword`
CHANNELS=`GetValue "${PARAM_CONTENT}" Channel`
ARCHIVE=`GetValue "${PARAM_CONTENT}" LogPath`
QOS=`GetValue "${PARAM_CONTENT}" Qos`
TRUNCATELOG=`GetValue "${PARAM_CONTENT}" truncateLog`
ASMSIDNAME=`GetValue "${PARAM_CONTENT}" ASMInstanceName`
DBType=`GetValue "${PARAM_CONTENT}" dbType`
ENCALGO=`GetValue "${PARAM_CONTENT}" EncAlgo`
ENCKEY=`GetValue "${PARAM_CONTENT}" EncKey`
KEEPDAYS=`GetValue "${PARAM_CONTENT}" ArchiveLogKeepDays`

test "$DBNAME" = "${ERROR_PARAM_INVALID}"                          && ExitWithError "DBNAME"
test "$DBUUID" = "${ERROR_PARAM_INVALID}"                          && ExitWithError "DBUUID"
test "$DBINSTANCE" = "${ERROR_PARAM_INVALID}"                      && ExitWithError "DBINSTANCE"
test "$DBUSER" = "${ERROR_PARAM_INVALID}"                          && ExitWithError "DBUSER"
test "$IN_ORACLE_HOME" = "${ERROR_PARAM_INVALID}"                  && ExitWithError "IN_ORACLE_HOME"
test "$ASMUSER" = "${ERROR_PARAM_INVALID}"                         && ExitWithError "ASMUSER"
test "$CHANNELS" = "${ERROR_PARAM_INVALID}"                        && ExitWithError "CHANNELS"
test "$ARCHIVE" = "${ERROR_PARAM_INVALID}"                         && ExitWithError "ARCHIVE"
test "$QOS" = "${ERROR_PARAM_INVALID}"                             && ExitWithError "QOS"
test "$TRUNCATELOG" = "${ERROR_PARAM_INVALID}"                     && ExitWithError "TRUNCATELOG"
test "$ASMSIDNAME" = "${ERROR_PARAM_INVALID}"                      && ExitWithError "ASMSIDNAME"
test "$DBType" = "${ERROR_PARAM_INVALID}"                          && ExitWithError "DBType"
test "$ENCALGO" = "${ERROR_PARAM_INVALID}"                         && ExitWithError "ENCALGO"
test "$KEEPDAYS" = "${ERROR_PARAM_INVALID}"                        && ExitWithError "KEEPDAYS"

Log "PID=${PID};DBINSTANCE=${DBINSTANCE};DBName=${DBNAME};DBUUID=${DBUUID};DBUSER=${DBUSER};\
ASMInst=${ASMSIDNAME};ASMUSER=${ASMUSER};IN_ORACLE_HOME=$IN_ORACLE_HOME;DBCHANNEL=$CHANNELS;ARCHIVE=$ARCHIVE;\
QOS=$QOS;TRUNCATELOG=$TRUNCATELOG;KEEPDAYS=${KEEPDAYS}"

test -z "$ARCHIVE"     && ExitWithError "log path"
test -z "$DBINSTANCE"  && ExitWithError "oracle instance name"
test -z "${DBUUID}"    && ExitWithError "db uuid"

GetOracleUser
GetUserShellType ${ORA_DB_USER} ${ORA_GRID_USER}
GetOracleHomePath ${ORA_DB_USER}
GetOracleVersion ${ORA_DB_USER}
VERSION=`echo $PREVERSION | tr -d '.'`
GetOracleCluster
DBINSTANCE=`GetRealInstanceName ${DBINSTANCE}`
Log "Check cluster [DBISCLUSTER=${DBISCLUSTER}], DBINSTANCE=${DBINSTANCE}."
CheckSqlPlusStatus
CheckRmanStatus
CheckMountPath

QueryFromVDatabase()
{
    echo "set linesize 999" > "${SqlCMD}"
    echo "select a.LOG_MODE, a.OPEN_MODE, a.RESETLOGS_CHANGE#, b.RESETLOGS_ID from 
    v\$database a, v\$database_incarnation b where a.LAST_OPEN_INCARNATION#=b.INCARNATION# and b.STATUS='CURRENT';" >> "${SqlCMD}"
    echo "exit" >> "${SqlCMD}"
    OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${SqlCMD}" "${SqlRST}" "${DBINSTANCE}" 30 1 "${ORA_DB_USER}" >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ ${RET_CODE} -ne 0 ];then
        Log "QueryFromVDatabase failed, ret=${RET_CODE}"
        DeleteFile ${SqlCMD} ${SqlRST}
        exit $RET_CODE
    fi
    logMode=`cat ${SqlRST} | $MYAWK '{print $1}'`
    open_mode=`cat ${SqlRST} | $MYAWK '{print $2$3}'`
    resetlogs_change=`cat ${SqlRST} | $MYAWK '{print $4}'`
    resetlogs_id=`cat ${SqlRST} | $MYAWK '{print $5}'`
    DeleteFile ${SqlCMD} ${SqlRST}
}

QueryFromVDatabase
if [ "${logMode}" != "ARCHIVELOG" ]; then
    Log "Archive Mode=No Archive Mode, check archive mode failed."
    exit ${ERROR_ORACLE_NOARCHIVE_MODE}
fi

CreateDir "${ARCHIVE}/resetlogs_id${resetlogs_id}"
CheckDirRWX "$ORA_DB_USER" "${ARCHIVE}/resetlogs_id${resetlogs_id}"     || ExitWithError "check log path"
CheckDirRWX "$ORA_DB_USER" "$IN_ORACLE_HOME"     || ExitWithError "check oracle home path"
LOG_IS_BACKED_UP="$LOG_IS_VALID and name like '$ARCHIVE/resetlogs_id${resetlogs_id}/arch_%'"

# check channels param
CheckParamChannels
ISEncBK=0
if [ ! -z "${ENCALGO}" ] && [ ! -z "${ENCKEY}" ]; then
    ISEncBK=1
fi

if [ ${ISEncBK} -eq 1 ]; then
    RMAN_ENC_SECTION="configure encryption for database on; configure encryption algorithm '${ENCALGO}'; set encryption on identified by \"${ENCKEY}\" only; set decryption identified by \"${ENCKEY}\";"
fi

#********************************prepare for backup begin********************************
# from scn
Log "INFO:Begin to exec SQL Get From Scn"
if [ -f ${ARCHIVE}/last_backup_scn ]; then
    last_backup_resetlogs_id=`cat ${ARCHIVE}/last_backup_scn | $MYAWK -F ";" '{print $4}'`
    if [ "${last_backup_resetlogs_id}" != "${resetlogs_id}" ]; then
        Log "ERROR: last_backup_resetlogs_id(${last_backup_resetlogs_id}) not match current resetlogs_id(${resetlogs_id}), backup data first."
        exit ${ERROR_CHECK_ARCHIVELOG_FAILED}
    fi
    from_scn=`cat ${ARCHIVE}/last_backup_scn | $MYAWK -F ";" '{print $2}'`
    Log "last_backup_scn=${from_scn}"
else
    GetFromSCN ${DBINSTANCE}
    if [ -z "${from_scn}" ]; then
        GetMinSysSCN ${DBINSTANCE}
        if [ ! -z "${min_sys_scn}" ]; then
            from_scn=${min_sys_scn}
        else
            from_scn=${resetlogs_change}
        fi
    fi
fi

CheckLogLost()
{
    GetDatabaseNodeList
    for node in `echo ${nodeList}`; do
        echo "set linesize 999" > "${SqlCMD}"
        echo "COL name FORMAT a500" >> "${SqlCMD}"
        echo "select first_change#, next_change#, name  from v\$archived_log where name is not null and name not like '/tmp/advbackup/%' \
        and THREAD#=${node} and RESETLOGS_ID=${resetlogs_id} and (deleted = 'NO') and (ARCHIVED='YES') and (STATUS = 'A') \
        and next_change#>${from_scn};" >> "${SqlCMD}"
        echo "exit" >> "${SqlCMD}"
        OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${SqlCMD}" "${SqlRST}" "${DBINSTANCE}" 30 1 "${ORA_DB_USER}" >> "${LOG_FILE_NAME}" 2>&1
        RET_CODE=$?
        if [ ${RET_CODE} -ne 0 ]; then
            Log "CheckLogLost failed, ret=${RET_CODE}"
            DeleteFile "${SqlCMD}" "${SqlRST}"
            exit ${ERROR_ORACLE_BACKUP_FAILED}
        fi
        if [ -s "${SqlRST}" ]; then
            first_change=`cat ${SqlRST} | head -1 | $MYAWK '{print $1}'`
            Log "first_change=${first_change}, from_scn=${from_scn}."
            if [ ${first_change} -gt ${from_scn} ]; then
                cat ${SqlRST} | head -1 >> "${LOG_FILE_NAME}"
                Log "ERROR: node(${node}) have log lost, backup failed."
                DeleteFile "${SqlCMD}" "${SqlRST}"
                exit ${ERROR_CHECK_ARCHIVELOG_FAILED}
            fi
        fi
    done
    DeleteFile "${SqlCMD}" "${SqlRST}"
}
CheckLogLost

GetArchiveLogNum()
{
    echo "set linesize 999" > "${SqlCMD}"
    echo "select count(name) from v\$archived_log where name is not null and name not like '%/tmp/advbackup/%'  and next_change# > ${from_scn};" >> "${SqlCMD}"
    echo "exit" >> "${SqlCMD}"
    OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${SqlCMD}" "${SqlRST}" "${DBINSTANCE}" 30 1 "${ORA_DB_USER}" >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ ${RET_CODE} -ne 0 ]; then
        Log "GetArchiveLogNum failed, ret=${RET_CODE}"
        DeleteFile "${SqlCMD}" "${SqlRST}"
        echo 2
    else
        local num=`cat ${SqlRST}`
        let num+=2
        DeleteFile "${SqlCMD}" "${SqlRST}"
        echo ${num}
    fi
}
logNum=`GetArchiveLogNum`
Log "GetArchiveLogNum=${logNum}"
#********************************prepare for backup end********************************

#********************************backup log file begin********************************
Log "INFO:Building RMAN Log backup script"
cat > $SQLRMANFILE << EOF
startup mount
configure backup optimization off;
configure controlfile autobackup off;
set nocfau;
configure maxsetsize to unlimited;
EOF

if [ ${ISEncBK} = 0 ]; then
    cat >> $SQLRMANFILE << EOF
configure encryption for database off;
EOF
fi

cat >> $SQLRMANFILE << EOF
RUN
{
    SET COMMAND ID TO 'ProtectAgent_Backup';
EOF
i=1
while [ $i -le $CHANNELS ]; do
if [ -z "${QOS}" ] || [ "${QOS}" = "0" ]; then
    cat >> $SQLRMANFILE << EOF
    allocate channel eBackup`printf "%02d" $i` type disk format '${ARCHIVE}/resetlogs_id${resetlogs_id}/%T_%U';
EOF
else
    if [ ${logNum} -ge ${CHANNELS} ] || [ ${logNum} -eq 0 ]; then
        realQOS=`expr ${QOS} \* 1024 / ${CHANNELS}`
    else
        realQOS=`expr ${QOS} \* 1024 / ${logNum}`
    fi
    cat >> $SQLRMANFILE << EOF
    allocate channel eBackup`printf "%02d" $i` type disk format '${ARCHIVE}/resetlogs_id${resetlogs_id}/%T_%U' rate `printf "%d" ${realQOS}`k;
EOF
fi
    i=`expr $i + 1`
done

    echo "    configure device type disk parallelism $CHANNELS;" >> ${SQLRMANFILE}
    if [ "${open_mode}" != "READONLY" ];then
        echo "    sql 'alter system archive log current';" >> ${SQLRMANFILE}
    fi
    echo "    backup as copy archivelog from scn $from_scn format '${ARCHIVE}/resetlogs_id${resetlogs_id}/%U.log';" >> ${SQLRMANFILE}

cat >> $SQLRMANFILE << EOF
}
EOF

Log "INFO:Running RMAN to backup achive logs"
RmanExeScript "${DBUSER}" "${DBUSERPWD}" ${SQLRMANFILE} ${RMANFILERST} ${DBINSTANCE} -1 ${ORA_DB_USER} >> "${LOG_FILE_NAME}" 1 2>&1
RMAN_RET_CODE=$?
if [ ${RMAN_RET_CODE} -ne 0 ]; then
    cat ${RMANFILERST} >> ${LOG_FILE_NAME}
    DeleteFile ${SQLRMANFILE} ${RMANFILERST}
    Log "ERROR:Backup database-${DBINSTANCE} archive log failed, error=${RMAN_RET_CODE}."
    if [ "${ERROR_SCRIPT_EXEC_FAILED}" = "${RMAN_RET_CODE}" ]; then
        exit ${ERROR_ORACLE_BACKUP_FAILED}
    else
        exit ${RMAN_RET_CODE}
    fi
else
    Log "INFO:Backup database-${DBINSTANCE} archive log succ."
    DeleteFile ${SQLRMANFILE} ${RMANFILERST}
fi
#********************************backup log file begin********************************

#********************************after backup log file begin******************************

# truncate log
if [[ "$TRUNCATELOG" != "0" ]] && [[ "$TRUNCATELOG" != "1" ]]; then
    TRUNCATELOG=0
    Log "Setting truncatelog to $TRUNCATELOG by default"
fi
if [ "${TRUNCATELOG}" = "0" ]; then
    Log "INFO:Do trun cate log from_scn is $from_scn."
    DoTrunCateLog ${DBINSTANCE} ${from_scn}
fi

#get archive log range
UpdateArciveLogRangeCurrent
logbackuprst=`GetArciveLogRange`
echo "${logbackuprst}" >> "${RESULT_FILE}"
Log "${logbackuprst}"

#get taskreslut avg_speed and total size
GetLastRmanTaskSpeed
echo "taskavgspeed;${TaskSpeed}" >> "${RESULT_FILE}"
Log "taskavgspeed;${TaskSpeed}" 
echo "backupsize;${BackupSize}" >> "${RESULT_FILE}"
Log "backupsize;${BackupSize}" 

#chmod 640 RESULT_FILE
if [ -f  "${RESULT_FILE}" ]; then
    chmod 640 "${RESULT_FILE}"
fi
rm -rf ${ARCHIVE}/last_backup_scn >> "${LOG_FILE_NAME}" 2>&1
Log "Do log bakcup success."

exit $RMAN_RET_CODE
#********************************after backup log file end******************************
