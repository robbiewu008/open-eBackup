#!/bin/sh
set +x

#@dest:  application agent for oracle
#@date:  2009-04-07
#@authr: 
USAGE="Usage: ./oracletest.sh AgentRoot PID"

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
LOG_FILE_NAME="${LOG_PATH}/oracletest.log"
#for GetOracleVersion
ORA_VERSION=
PREVERSION=
VERSION=
ORCLVESION="${STMP_PATH}"/ORCV"${PID}.txt"
SQLEXIT="${STMP_PATH}/EXITSQL${PID}.sql"
#for GetOracleInstanceStatus
CHECK_INSTANCE_STATUS="${STMP_PATH}/CheckInstanceStatus${PID}.sql"
#STARTED - After STARTUP NOMOUNT
#MOUNTED - After STARTUP MOUNT or ALTER DATABASE CLOSE
#OPEN - After STARTUP or ALTER DATABASE OPEN
#OPEN MIGRATE - After ALTER DATABASE OPEN { UPGRADE | DOWNGRADE }
INSTANCESTATUS=""
DBUSER=""
DBUSERPWD=""
#********************************define these for the functions of agent_sbin_func.sh********************************

#********************************define these for local script********************************
DBNAME=""
DBINSTANCE=""

TESTQRYDBOPENMODEFILE="${STMP_PATH}/TestDBOpenMode${PID}.sql"
TESTQRYDBOPENMODERSTFILE="${STMP_PATH}/TestDBOpenModeRST${PID}.txt"
TESTQRYDBOPENMODE_READ="READ"
LEN_READ=4
# oracle temp file
ORATMPINFO="${STMP_PATH}/ORATMPINFO${PID}.txt"
QUERYNAMECRIPT="$STMP_PATH/CheckASM$PID.sql"
QUERYNAMECRIPTRST="$STMP_PATH/CheckASMRST$PID.txt"
PARAM_CONTENT=`ReadInputParam`
test -z "$PARAM_CONTENT"              && ExitWithError "PARAM_FILE"
#********************************define these for local script********************************


#create the sql that get the open mode of the DB
CreatCheckDBReadSQL()
{
    echo "select open_mode from v\$database;" > "$1" 
    echo "exit" >> "$1"
}

#check database can be read
CheckDBReadability()
{
    TMPFILE=$1
    CHECKVAL=$2
    NAMEEQ=0
    TMPLINE=`cat "${TMPFILE}" | ${MYAWK} '{print $1}'`
    for i in ${TMPLINE} 
    do 
        LINE=$i
        NOWDBNAME=`echo $LINE | tr '[a-z]' '[A-Z]'`
        NOWDBNAME=`RDsubstr $NOWDBNAME 1 $LEN_READ`
        if [ "${NOWDBNAME}" = "${CHECKVAL}" ]
        then
            NAMEEQ=1
            break
        fi
    done
    if [ $NAMEEQ -eq 0 ]
    then
        return 0
    else
        return 1
    fi
}

TestDBConnect()
{
    Log "Start to check oracle instance status."
    GetOracleInstanceStatus "${DBINSTANCE}"
    RET_CODE=$?
    if [ "${RET_CODE}" -ne "0" ]
    then
        Log "Get instance status failed."
        exit ${RET_CODE}
    fi

    #STARTED - After STARTUP NOMOUNT
    #MOUNTED - After STARTUP MOUNT or ALTER DATABASE CLOSE
    #OPEN - After STARTUP or ALTER DATABASE OPEN
    #OPEN MIGRATE - After ALTER DATABASE OPEN { UPGRADE | DOWNGRADE }
    if [ ! "`RDsubstr $INSTANCESTATUS 1 4`" = "OPEN" ]
    then
        Log "Instance status($INSTANCESTATUS) no open."
        exit ${ERROR_INSTANCE_NOSTART}
    fi
    Log "end to check oracle instance status."

    # check v$database， open_mode if have read
    CreatCheckDBReadSQL ${TESTQRYDBOPENMODEFILE}
    Log "Exec SQL to check database status of read write."
    OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${TESTQRYDBOPENMODEFILE}" "${TESTQRYDBOPENMODERSTFILE}" "${DBINSTANCE}" 30 0 >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ "${RET_CODE}" -ne "0" ]
    then
        Log "Connect to database(${DBINSTANCE}) failed on test."
        DeleteFile "${TESTQRYDBOPENMODEFILE}"
        DeleteFile "${TESTQRYDBOPENMODERSTFILE}"
        exit ${RET_CODE}
    else
        CheckDBReadability "${TESTQRYDBOPENMODERSTFILE}" "${TESTQRYDBOPENMODE_READ}"
        if [ "$?" != "1" ]
        then
            Log "Check DB(${DBINSTANCE}) readability failed."
            DeleteFile "${TESTQRYDBOPENMODEFILE}"
            DeleteFile "${TESTQRYDBOPENMODERSTFILE}"
            exit $ERROR_SCRIPT_EXEC_FAILED
        else 
            DeleteFile "${TESTQRYDBOPENMODEFILE}"
            DeleteFile "${TESTQRYDBOPENMODERSTFILE}"
            Log "Connect to database(${DBINSTANCE}) successful on test."
            exit 0
        fi
    fi
}

TestASMInstanceConnect()
{
    #check ASM instance status
    ASMINSTNUM=`ps -ef | grep "asm_...._$DBINSTANCE" | grep -v "grep" | wc -l`
    if [ ${ASMINSTNUM} -eq 0 ]
    then
        Log "${DBINSTANCE} is not open."
        exit ${ERROR_ORACLE_ASM_INSTANCE_NOSTART}
    fi

    ASMSIDNAME=`ps -ef | grep "asm_...._$DBINSTANCE" | grep -v "grep" | ${MYAWK} -F '+' '{print $NF}'`
    ASMSIDNAME=`echo ${ASMSIDNAME} | ${MYAWK} -F " " '{print $1}'`
    ASMSIDNAME="+"${ASMSIDNAME}
    Log "Check ASM instance name $ASMSIDNAME."
    
    # get asm disk group memeber
    echo "select status from v\$instance;" > "${QUERYNAMECRIPT}"
    echo "exit" >> "${QUERYNAMECRIPT}"  
    Log "Exec ASM SQL to check status of ASM instance."
    ASMExeSql "${ASMUSER}" "${ASMUSERPWD}" "${QUERYNAMECRIPT}" "${QUERYNAMECRIPTRST}" "${ASMSIDNAME}" 30 0
    RET_CODE=$?
    DeleteFile "${QUERYNAMECRIPT}" "${QUERYNAMECRIPTRST}"
    if [ "${RET_CODE}" -ne "0" ]
    then
        Log "Test ASM instance failed."
        exit ${RET_CODE}
    else
        Log "Test ASM instance successful."
        exit 0
    fi
}

#############################################################
DBINSTANCE=`GetValue "${PARAM_CONTENT}" InstanceName`
DBNAME=`GetValue "${PARAM_CONTENT}" AppName`
DBUSERL=`GetValue "${PARAM_CONTENT}" UserName`
DBUSER=`echo "$DBUSERL" | tr '[A-Z]' '[a-z]'`
DBUSERPWD=`GetValue "${PARAM_CONTENT}" Password`
IN_ORACLE_HOME=`GetValue "${PARAM_CONTENT}" OracleHome`

test "$DBNAME" = "${ERROR_PARAM_INVALID}"                           && ExitWithError "DBNAME"
test "$DBINSTANCE" = "${ERROR_PARAM_INVALID}"                       && ExitWithError "DBINSTANCE"
test "$DBUSER" = "${ERROR_PARAM_INVALID}"                           && ExitWithError "DBUSER"
test "$DBUSERL" = "${ERROR_PARAM_INVALID}"                          && ExitWithError "DBUSERL"
test "$IN_ORACLE_HOME" = "${ERROR_PARAM_INVALID}"                   && ExitWithError "IN_ORACLE_HOME"

Log "DBNAME=$DBNAME;DBUSER=$DBUSER;DBINSTANCE=$DBINSTANCE;PID=${PID};IN_ORACLE_HOME=$IN_ORACLE_HOME"

GetOracleUser
GetUserShellType ${ORA_DB_USER} ${ORA_GRID_USER}
GetOracleHomePath ${ORA_DB_USER}
GetOracleVersion ${ORA_DB_USER}
VERSION=`echo $PREVERSION | tr -d '.'`
DBINSTANCE=`GetRealInstanceName ${DBINSTANCE}`

#get user shell type
GetUserShellType

#get Oracle version
GetOracleVersion
VERSION=`echo $PREVERSION | tr -d '.'`
PRE_INSTANCE=`RDsubstr $DBINSTANCE 1 1`
if [ "${PRE_INSTANCE}" = "+" ]; then
    TestASMInstanceConnect
else
    TestDBConnect
fi

exit 0
