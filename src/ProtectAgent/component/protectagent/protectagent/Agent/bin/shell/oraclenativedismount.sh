#!/bin/sh
set +x
#@function: start oracle with livemount.
USAGE="Usage: ./oraclenativedismount.sh AgentRoot PID"

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
LOG_FILE_NAME="${LOG_PATH}/oraclenativedismount.log"
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

SYS_TEM=`uname -s`
#********************************define these for the functions of agent_sbin_func.sh********************************

#********************************define these for local script********************************
DataSharePath=`GetValue "${PARAM_CONTENT}" dataShareMountPath`
LogSharePath=`GetValue "${PARAM_CONTENT}" logShareMountPath`
StorageIP=`GetValue "${PARAM_CONTENT}" storageIp`
DBNAME=`GetValue "${PARAM_CONTENT}" AppName`
DBUUID=`GetValue "${PARAM_CONTENT}" DBUUID`

test "$DataSharePath" = "${ERROR_PARAM_INVALID}"                && ExitWithError "DataSharePath"
test "$LogSharePath" = "${ERROR_PARAM_INVALID}"                 && ExitWithError "LogSharePath"
test "$StorageIP" = "${ERROR_PARAM_INVALID}"                    && ExitWithError "StorageIP"
test "$DBNAME" = "${ERROR_PARAM_INVALID}"                       && ExitWithError "DBNAME"
test "$DBUUID" = "${ERROR_PARAM_INVALID}"                       && ExitWithError "DBUUID"

Log "PID=${PID};DBNAME=${DBNAME};DBUUID=${DBUUID};DataSharePath=${DataSharePath};LogSharePath=${LogSharePath};StorageIP=${StorageIP}."

MountPoints=
if [ ! -z "${DBUUID}" ]; then
    for mp in `mount | grep ${DBUUID} | $MYAWK '{print $3}'`; do
        MountPoints="${MountPoints} ${mp}"
    done
fi

for mp in ${MountPoints}; do
    sourceFS=`mount | grep ${mp}`
    if [ ! -z "${sourceFS}" ]; then
        umount -f ${mp} >> $LOG_FILE_NAME 2>&1
        if [ $? -ne 0 ]; then
            Log "umount ${mp} failed."
            if [ $SYS_TEM != "AIX" ]; then
                sleep 60
                umount -l ${mp} >> $LOG_FILE_NAME 2>&1
            fi
        else
            Log "umount ${mp} succ."
        fi
    fi
done
Log "Dismount medium successfully."
exit 0