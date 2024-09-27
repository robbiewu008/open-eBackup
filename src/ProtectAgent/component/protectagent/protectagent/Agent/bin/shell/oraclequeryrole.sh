#!/bin/sh
set +x

#@dest:  query oracle host role for oracle 
#@date:  2020-07-04
#@authr: 
USAGE="Usage: ./oraclequeryrole.sh AgentRoot PID"

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
LOG_FILE_NAME="${LOG_PATH}/oraclequeryrole.log"
#for GetOracleVersion
ORA_VERSION=
PREVERSION=
VERSION=
ORCLVESION="${STMP_PATH}"/ORCV"${PID}.txt"
SQLEXIT="${STMP_PATH}/EXITSQL${PID}.sql"
# oracle temp file
ORATMPINFO="${STMP_PATH}/ORATMPINFO${PID}.txt"
#STARTED - After STARTUP NOMOUNT
#MOUNTED - After STARTUP MOUNT or ALTER DATABASE CLOSE
#OPEN - After STARTUP or ALTER DATABASE OPEN
#OPEN MIGRATE - After ALTER DATABASE OPEN { UPGRADE | DOWNGRADE }
INSTANCESTATUS=""
DBUSER=""
DBUSERPWD=""
#********************************define these for the functions of agent_sbin_func.sh********************************

#********************************define these for local script********************************
QUERYNAMECRIPT="${STMP_PATH}/CheckASM$PID.sql"
QUERYNAMECRIPTRST="${STMP_PATH}/CheckASMRST$PID.txt"
PARAM_CONTENT=`ReadInputParam`
test -z "$PARAM_CONTENT"              && ExitWithError "PARAM_FILE"
#********************************define these for local script********************************

#############################################################
DBNAME=`GetValue "${PARAM_CONTENT}" AppName`
test "$DBNAME" = "${ERROR_PARAM_INVALID}"                           && ExitWithError "DBNAME"

Log "DBNAME=$DBNAME;PID=${PID}"

GetOracleUser
# get user shell type
GetUserShellType ${ORA_DB_USER} ${ORA_GRID_USER}

#get oracle home path
GetOracleHomePath ${ORA_DB_USER}

#get Oracle version
GetOracleVersion ${ORA_DB_USER}
VERSION=`echo $PREVERSION | tr -d '.'`

# get cluster configuration
GetOracleCluster

DeleteFile "${RESULT_FILE}"
# rac mode
if [ "${DBISCLUSTER}" = "1" ]; then
    GetORA_CRS_HOME ${VERSION} ${ORA_GRID_USER}

    # cluster running status
    ${ORA_CRS_HOME}/bin/crsctl status res -t
    if [ $? -ne 0 ]; then
        Log "RAC is not running."
        exit 1
    fi

    su - ${ORA_DB_USER} -c "${EXPORT_ORACLE_ENV}srvctl status database -db ${DBNAME}" | grep "not running"
    if [ $? -eq 0 ]; then
        Log "Database is not running."
        exit 1
    fi

    hosts=
    for host in `su - ${ORA_DB_USER} -c "${EXPORT_ORACLE_ENV}srvctl status database -db ${DBNAME}" | ${MYAWK} '{print $NF}'`; do
        if [ -z "$hosts" ]; then
            hosts=$host
        else
            hosts=${hosts},${host}
        fi
    done
    echo ${hosts} > "${RESULT_FILE}"

    shosts=
    for host in `su - ${ORA_DB_USER} -c "${EXPORT_ORACLE_ENV}srvctl status asm" | ${MYAWK} '{print $NF}' | sed 's/,/ /g'`; do
        if [ -z "$shosts" ]; then
            shosts=$host
        else
            shosts=${shosts},${host}
        fi
    done
    echo ${shosts} > "${RESULT_FILE}"
else
    # check ASM and database running status
    su - ${ORA_DB_USER} -c "${EXPORT_ORACLE_ENV}srvctl status database -db ${DBNAME}" | grep "is running"
    if [ $? -ne 0 ]; then
        Log "database is not running."
        exit 1
    fi

    echo `hostname` > "${RESULT_FILE}"
    echo `hostname` > "${RESULT_FILE}"
fi

#chmod 640 RESULT_FILE
if [ -f "${RESULT_FILE}" ]; then
    chmod 640 "${RESULT_FILE}"
fi

Log "query oracle host role success."
exit 0