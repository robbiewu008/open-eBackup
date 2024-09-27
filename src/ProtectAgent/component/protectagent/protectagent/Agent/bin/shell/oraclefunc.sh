#!/bin/sh
# set +x

HOSTAGENT_UDEV_FILE=/etc/udev/rules.d/99-oracle-asmdevices.rules

NecessaryTSArr="SYSAUX SYSTEM UNDOTBS1 UNDOTBS2 USERS"
LOG_IS_VALID="(RESETLOGS_ID=(select RESETLOGS_ID from v\$database_incarnation where STATUS='CURRENT')) and (deleted = 'NO') and (ARCHIVED='YES') and (STATUS != 'U')"

# opration such as backup, restore, delete ...
MAX_TASKEXEC_TIME=360000

# opration such as shutdown, startup, reset ...
MAX_OPDB_TIME=600

RmanCMD="${STMP_PATH}/RmanCMD${PID}.sql"
RmanRST="${STMP_PATH}/RmanRST${PID}.txt"
SqlCMD="${STMP_PATH}/SqlCMD${PID}.sql"
SqlRST="${STMP_PATH}/SqlRST${PID}.txt"
GRIDTMPINFO="${STMP_PATH}/GRIDtmp${PID}.txt"
IN_GRID_HOME=
SQLEXIT="${STMP_PATH}/EXITSQL${PID}.sql"
ORCLVESION="${STMP_PATH}/ORCV${PID}.txt"
RMAN_ENC_SECTION=

# check oracle install, 0-installed  1-uninstalled
CheckOracleIsInstall()
{
    INVENTORY_LOC=/etc/oraInst.loc
    if [ "${sysName}" = "HP-UX" ] || [ "${sysName}" = "SunOS" ]; then
        INVENTORY_LOC=/var/opt/oracle/oraInst.loc
    fi
    
    # check file exists
    if [ ! -f "${INVENTORY_LOC}" ]; then
        Log "oraInst.loc is not exists."
        return ${ERROR_ORACLE_NOT_INSTALLED}
    fi
    
    INVENTORY_PATH=`cat "${INVENTORY_LOC}" | grep "inventory_loc" | ${MYAWK} -F "=" '{print $2}'`
    if [ -z "${INVENTORY_PATH}" ]; then
        Log "Can not get inventory_loc path."
        return ${ERROR_ORACLE_NOT_INSTALLED}
    fi
    
    if [ -d "${INVENTORY_PATH}" ]; then
        return 0
    else
        Log "${INVENTORY_PATH} diretory not exists, oracle is not installed."
        return ${ERROR_ORACLE_NOT_INSTALLED}
    fi
}

CHMOD()
{
    source=
    arg2=
    arg=
    if [ $# -eq 3 ]; then
        arg=$1
        arg2=$2
        source=$3
    elif [ $# -eq 2 ]; then
        source=$2
        arg2=$1
    fi
    if [ -L "$source" ]; then
        Log "source file  is a link file can not copy."
        return
    fi
    chmod $arg $arg2 $source  >>${LOG_FILE_NAME} 2>&1
}

CheckOracleUserExists()
{
    # ----------su - oracle---------
    Log "Begin su - oracle!"
    su - oracle -c "${EXPORT_ORACLE_ENV}date"
    if [ "$?" != "0" ]
    then
        Log "su - oracle error!"
        return $ERROR_SCRIPT_EXEC_FAILED
    fi
    Log "End su - oracle!"
    return 0
}

GetOracleCluster()
{
    DBISCLUSTER=0
    ORACLE_LOC=/etc/oracle/ocr.loc
    if [ "${sysName}" = "HP-UX" ] || [ "${sysName}" = "SunOS" ]; then
        ORACLE_LOC=/var/opt/oracle/ocr.loc
    fi

    if [ -f "${ORACLE_LOC}" ]; then
        CLUSTER_FLAG=`cat "${ORACLE_LOC}" | grep "local_only" | ${MYAWK} -F "=" '{print $2}'`
        CLUSTER_FLAG=`echo ${CLUSTER_FLAG} | tr [a-z] [A-Z]`
        if [ "${CLUSTER_FLAG}" = "FALSE" ]; then
            DBISCLUSTER=1  # oracle rac
        elif [ "${CLUSTER_FLAG}" = "TRUE" ]; then
            DBISCLUSTER=2  # oracle restart
        fi
    fi
    Log "Check cluster [DBISCLUSTER=${DBISCLUSTER}]."
}

GetOracleVersion()
{
    local oracleUser=$1
    if [ -z "${oracleUser}" ];then
        GetOracleUser
        oracleUser=${ORA_DB_USER}
    fi
    # ----------get oracle version---------
    CreateSTMPFile "${SQLEXIT}"
    ORA_VERSION="--"
    echo "exit" > "${SQLEXIT}"
    chown ${oracleUser} ${SQLEXIT}
    CreateOracleSTMPFile "$ORCLVESION"
    su - ${oracleUser} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV}sqlplus /nolog @\"$SQLEXIT\" >> \"${ORCLVESION}\"" >> "${LOG_FILE_NAME}" 2>&1
    tmpfile="${STMP_PATH}/stmpfile${PID}"
    CreateSTMPFile $tmpfile
    su - ${oracleUser} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV}cat \"${ORCLVESION}\"" | grep "SQL\*Plus: Release" | $MYAWK -F " " '{print $3}' > $tmpfile 2>&1
    ORA_VERSION=`cat "$tmpfile"`
    DeleteFile "${ORCLVESION}"
    DeleteFile "${SQLEXIT}"
    DeleteFile "${tmpfile}"

    if [ "${ORA_VERSION}" = "${SPACE}" ]; then
        ORA_VERSION="--"
    fi
    
    if [ "${ORA_VERSION}" != "--" ]; then
        PREVERSION=`RDsubstr $ORA_VERSION 1 4`
    fi
    
    Log "oracle version($ORA_VERSION), preVersion($PREVERSION)."
}

SQLPlusTimeOut()
{
    if [ ! "$1" = "-1" ]; then
        "$MONITOR_FILENAME" $$ sqlplus "$1" "${AGENT_ROOT_PATH}" >>"${LOG_FILE_NAME}" 2>&1 &
    fi
}

RmanTimeOut()
{
    if [ ! "$1" = "-1" ]; then
        "$MONITOR_FILENAME" $$ rman "$1" "${AGENT_ROOT_PATH}" >>"${LOG_FILE_NAME}" 2>&1 &
    fi
}

GetOracle_homeOfSG()
{
    local CUR_DIR parent_dir workdir
    workdir=$1
    INSTNAME=$2
    cd ${workdir}
    CUR_DIR=`pwd`
    for x in `ls $CUR_DIR`
    do
        if [ -f "$CUR_DIR/$x" ]
        then
            if [ "${x}" = "haoracle.conf" ]
            then
                Log "INFO:The haoracle.conf file is $x=====$INSTNAME"
                SG_SID_NAME=`cat $CUR_DIR/$x|grep -v '#'|grep '^SID_NAME'|$MYAWK -F "=" '{print $2}'`
                if [ "$SG_SID_NAME" = "$INSTNAME" ]
                then
                    Log "==$SG_SID_NAME==$INSTNAME"
                    IN_ORACLE_HOME=`cat $CUR_DIR/$x|grep -v '#'|grep '^ORACLE_HOME'|$MYAWK -F "=" '{print $2}'`
                    return
                fi
            fi
        elif [ -d "$x" ]
        then
            cd "$x";
            GetOracle_homeOfSG $CUR_DIR/$x $INSTNAME
            if [ "$IN_ORACLE_HOME" != "" ]
            then
                return
            fi
            cd ..
        fi
    done
}

WriteWarnInfoToFile()
{
    echo "$1;$2" >> "${WARING_FILE}"
}

WriteErrInfoToFile()
{
    rm -rf "${ERRDETAIL_FILE}"
    local line=
    while read line; do
        if [[ ${line} = RMAN-* ]]; then
            echo "${line}" >> "${ERRDETAIL_FILE}"
        fi
        if [[ ${line} = ORA-* ]]; then
            echo "${line}" >> "${ERRDETAIL_FILE}"
        fi
    done < "$1"
}

# create New Method to Execute Sql
OracleExeSql()
{
    db_user=$1
    db_pwd=$2
    sql_file=$3
    rst_file=$4
    instance_name=$5
    timeout_second=$6
    is_silence=$7
    temp_file=${TMP_PATH}/tmprst${PID}

    if [ "${ORA_DB_USER}" = "" ]; then
        GetOracleUser
    fi
    chown ${ORA_DB_USER} "${sql_file}"
    chmod 700 "${sql_file}"
    CreateSTMPFile "${rst_file}"
    SQLPlusTimeOut ${timeout_second}
    if [ "${is_silence}" -eq "1" ]; then
        silent_mode="-S"
        echo "set pagesize 0" > "${sql_file}.bak"
        echo "set feedback off" >> "${sql_file}.bak"
        echo "set newpage none" >> "${sql_file}.bak"
        echo "set heading off" >> "${sql_file}.bak"
        cat ${sql_file} >> "${sql_file}.bak"
        mv "${sql_file}.bak" ${sql_file}
    fi

    if [ "`cat /etc/passwd | grep "^${ORA_DB_USER}:" | $MYAWK -F "/" '{print $NF}'`" = "csh" ]; then
        str_set_env="setenv ORACLE_SID ${instance_name};"
        if [ "${IN_ORACLE_HOME}" != "" ]; then
            str_set_env="${str_set_env} setenv ORACLE_HOME ${IN_ORACLE_HOME};"
        fi
    else
        str_set_env="ORACLE_SID=${instance_name}; export ORACLE_SID;"
        if [ "${IN_ORACLE_HOME}" != "" ]; then
            str_set_env="${str_set_env} ORACLE_HOME=${IN_ORACLE_HOME}; export ORACLE_HOME;"
        fi
    fi
    if [ "${db_user}" = "" ] || [ "${db_pwd}" = "" ]; then
        su - ${ORA_DB_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV} ${str_set_env} sqlplus -L ${silent_mode} '/ as sysdba' @'${sql_file}' > '${temp_file}'" >> "${LOG_FILE_NAME}" 2>&1
    else
        echo ${db_pwd} | su - ${ORA_DB_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV} ${str_set_env} sqlplus -L ${silent_mode} '${db_user} as sysdba' @'${sql_file}' > '${temp_file}'" >> "${LOG_FILE_NAME}" 2>&1
        # 因为密码是输入进去的，会导致结果文件第一行为空行
        su - ${ORA_DB_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV} sed -i '1d' ${temp_file}"
    fi
    # not check result code, because when user password error, the result code is 1,but need to check result
    # now check result file exist
    if [ ! -f "$temp_file" ]; then
        KillProcMonitor $$
        Log "Execute sql script failed."
        return $ERROR_SCRIPT_EXEC_FAILED
    fi
    su - ${ORA_DB_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV}cat \"$temp_file\"" >>"${rst_file}" 2>&1
    su - ${ORA_DB_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV}rm -rf \"$temp_file\""
    KillProcMonitor $$

    local oraErrMap="ORA-12560~TNS protocol adapter error.~${ERROR_ORACLE_TNS_PROTOCOL_ADAPTER}#\
    ORA-01034~ORACLE not available.~${ERROR_INSTANCE_NOSTART}#\
    ORA-00020~can not connect, maximum number of processes exceeded.~${ERROR_ORACLE_APPLICATION_OVER_MAX_LINK}#\
    ORA-01017~invalid username/password; logon denied.~${ERROR_DB_USERPWD_WRONG}#\
    ORA-01031~insufficient privileges.~${ERROR_INSUFFICIENT_WRONG}#\
    ORA-01123~cannot start online backup; media recovery not enabled.~${ERROR_ORACLE_NOARCHIVE_MODE}#\
    ORA-01146~cannot start online backup - file 1 is already in backup.~${ERROR_ORACLE_DB_ALREADY_INBACKUP}#\
    ORA-01142~cannot end online backup - none of the files are in backup.~${ERROR_ORACLE_DB_INHOT_BACKUP}#\
    ORA-01081~cannot start already-running.~${ERROR_ORACLE_DB_ALREADYRUNNING}#\
    ORA-01100~database already mounted.~${ERROR_ORACLE_DB_ALREADYMOUNT}#\
    ORA-01531~a database already open by the instance.~${ERROR_ORACLE_DB_ALREADYOPEN}#\
    ORA-01507~database not mounted.~${ERROR_ORACLE_NOT_MOUNTED}#\
    ORA-01109~database not open.~${ERROR_ORACLE_NOT_OPEN}#\
    ORA-10997~another startup/shutdown operation of this instance inprogress.~${ERROR_ORACLE_ANOTHER_STARTING}#\
    ORA-01154~database busy. Open, close, mount, and dismount not allowed now.~${ERROR_ORACLE_DB_BUSY}#\
    ORA-01012~database is not completely shut down.~${ERROR_ORACLE_DB_NOT_COMPLETE_SHUTDOWN}#\
    ORA-01034~database is not completely shut down.~${ERROR_ORACLE_DB_NOT_COMPLETE_SHUTDOWN}#\
    ORA-32006~execsql have some parameters have been deprecated.~0"

    local index=1
    local errInfo=
    while true; do 
        errInfo=`echo ${oraErrMap} | cut -d "#" -f${index}`
        if [ "${errInfo}" = "" ]; then
            break
        fi
        index=`expr $index + 1`
        local errFlag=`echo $errInfo | ${MYAWK} -F "~" '{print $1}'`
        local errDetail=`echo $errInfo | ${MYAWK} -F "~" '{print $2}'`
        local errCode=`echo $errInfo | ${MYAWK} -F "~" '{print $3}'`

        local ERROR_NUM=`cat "${rst_file}" | grep "${errFlag}" | wc -l`
        if [ ${ERROR_NUM} -ne 0 ]; then
            Log "Sql($instance_name) execute script failed: ${errDetail}"
            WriteErrInfoToFile "${rst_file}"
            return ${errCode}
        fi
    done

    #ORA-28002: the password will expire within 7 days
    #ORA-32004: obsolete and/or deprecated parameter(s) specified
    HAVEERROR=`cat "${rst_file}" | grep -v "ORA-28002" | grep -v "ORA-32004" | grep "^ORA-" | wc -l`
    if [ "$HAVEERROR" -ne "0" ]; then
        Log "=====================Database($DBNAME) exec script failed.==========================="
        cat "${rst_file}" >> "${LOG_FILE_NAME}"
        WriteErrInfoToFile "${rst_file}"
        Log "=====================Database($DBNAME) exec script failed.==========================="
        return $ERROR_ORACLE_EXESQL_FAILED
    fi

    return 0
}

# **************************************** Exec RMAN Script *************************
RmanExeScript()
{
    db_user=$1
    db_pwd=$2
    sql_file=$3
    rst_file=$4
    instance_name=$5
    timeout_second=$6
    ORA_DB_USER=$7
    is_enc=$8
    temp_file=${TMP_PATH}/tmprst${PID}

    if [ "${ORA_DB_USER}" = "" ]; then
        GetOracleUser
    fi
    chown ${ORA_DB_USER} "${sql_file}"
    chmod 700 "${sql_file}"
    CreateSTMPFile "${rst_file}"
    RmanTimeOut ${timeout_second}

    if [ "`cat /etc/passwd | grep "^${ORA_DB_USER}:" | $MYAWK -F "/" '{print $NF}'`" = "csh" ]; then
        str_set_env="setenv ORACLE_SID ${instance_name};"
        if [ "${IN_ORACLE_HOME}" != "" ]; then
            str_set_env="${str_set_env} setenv ORACLE_HOME ${IN_ORACLE_HOME};"
        fi
    else
        str_set_env="ORACLE_SID=${instance_name}; export ORACLE_SID;"
        if [ "${IN_ORACLE_HOME}" != "" ]; then
            str_set_env="${str_set_env} ORACLE_HOME=${IN_ORACLE_HOME}; export ORACLE_HOME;"
        fi
    fi

    if [ -z "${ISEncBK}" ] || [ ${ISEncBK} -eq 0 ] || [ -z "${is_enc}" ]; then
        if [ "${db_user}" = "" ] || [ "${db_pwd}" = "" ]; then
            su - ${ORA_DB_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV} ${str_set_env} rman target '/' cmdfile '${sql_file}' log '${temp_file}'" >> "${LOG_FILE_NAME}" 2>&1
        else
            echo ${db_pwd} | su - ${ORA_DB_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV} ${str_set_env} rman target '${db_user}' cmdfile '${sql_file}' log '${temp_file}'" >> "${LOG_FILE_NAME}" 2>&1
        fi
    else
        if [ "${db_user}" = "" ] || [ "${db_pwd}" = "" ]; then
            su - ${ORA_DB_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV} ${str_set_env} rman target '/' << EOF
`echo ${RMAN_ENC_SECTION}`
`cat ${sql_file}`
EOF" > "${rst_file}"
        else
            echo ${db_pwd} | su - ${ORA_DB_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV} ${str_set_env} rman target '${db_user}' << EOF
`echo ${RMAN_ENC_SECTION}`
`cat ${sql_file}`
EOF" > "${rst_file}"
        fi
    fi

    RMAN_RET=$?
    KillProcMonitor $$
    if [ -f "$temp_file" ]; then
        su - ${ORA_DB_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV}cat \"${temp_file}\"" >> "${rst_file}" 2>&1
        su - ${ORA_DB_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV}rm -rf \"${temp_file}\""
    fi
    local oraErrMap="ORA-12560~TNS protocol adapter error.~${ERROR_ORACLE_TNS_PROTOCOL_ADAPTER}#\
    ORA-01034~ORACLE not available.~${ERROR_INSTANCE_NOSTART}#\
    ORA-00020~can not connect, maximum number of processes exceeded.~${ERROR_ORACLE_APPLICATION_OVER_MAX_LINK}#\
    ORA-01017~invalid username/password; logon denied.~${ERROR_DB_USERPWD_WRONG}#\
    ORA-01031~insufficient privileges.~${ERROR_INSUFFICIENT_WRONG}#\
    ORA-01123~cannot start online backup; media recovery not enabled.~${ERROR_ORACLE_NOARCHIVE_MODE}#\
    ORA-01146~cannot start online backup - file 1 is already in backup.~${ERROR_ORACLE_DB_ALREADY_INBACKUP}#\
    ORA-01142~cannot end online backup - none of the files are in backup.~${ERROR_ORACLE_DB_INHOT_BACKUP}#\
    ORA-01081~cannot start already-running.~${ERROR_ORACLE_DB_ALREADYRUNNING}#\
    ORA-01100~database already mounted.~${ERROR_ORACLE_DB_ALREADYMOUNT}#\
    ORA-01531~a database already open by the instance.~${ERROR_ORACLE_DB_ALREADYOPEN}#\
    ORA-01507~database not mounted.~${ERROR_ORACLE_NOT_MOUNTED}#\
    ORA-01109~database not open.~${ERROR_ORACLE_NOT_OPEN}"

    local index=1
    local errInfo=
    while true; do 
        errInfo=`echo ${oraErrMap} | cut -d "#" -f${index}`
        if [ "${errInfo}" = "" ]; then
            break
        fi
        index=`expr $index + 1`
        local errFlag=`echo $errInfo | ${MYAWK} -F "~" '{print $1}'`
        local errDetail=`echo $errInfo | ${MYAWK} -F "~" '{print $2}'`
        local errCode=`echo $errInfo | ${MYAWK} -F "~" '{print $3}'`

        local ERROR_NUM=`cat "${rst_file}" | grep "${errFlag}" | wc -l`
        if [ ${ERROR_NUM} -ne 0 ]; then
            Log "RMAN($instance_name) execute script failed: ${errDetail}"
            WriteErrInfoToFile "${rst_file}"
            return ${errCode}
        fi
    done
    
    # backup failed
    if [ "$RMAN_RET" -ne "0" ];  then
        Log "=====================RMAN($instance_name) execute script failed.==========================="
        cat "${rst_file}" >> "${LOG_FILE_NAME}"
        WriteErrInfoToFile "${rst_file}"
        Log "=====================RMAN($instance_name) execute script failed.==========================="
        return $ERROR_ORACLE_EXERMAN_FAILED
    fi

    # ORA-28002: the password will expire within 7 days
    # ORA-32004: obsolete and/or deprecated parameter(s) specified
    ###### some time, backup file failed with error ORA-19504+ORA-27054, don't know reason, avoid it #########
    # ORA-19504: failed to create file 
    # ORA-27054: NFS file system where the file is created or resides is not mounted with correct options
    # Linux-x86_64 Error: 13: Permission denied
    ###############
    HAVEERROR=`cat "${rst_file}" | grep -v "ORA-28002" | grep -v "ORA-32004" | grep -v "ORA-27054" | grep -v "ORA-19504" | grep "^ORA-" | wc -l`
    if [ "$HAVEERROR" -ne "0" ];  then
        Log "=====================RMAN($instance_name) execute script failed.==========================="
        cat "${rst_file}" >> "${LOG_FILE_NAME}"
        WriteErrInfoToFile "${rst_file}"
        Log "=====================RMAN($instance_name) execute script failed.==========================="
        return $ERROR_ORACLE_EXERMAN_FAILED
    fi
    
    return 0
}

# **************************************** Exec ASM Instance SQL *************************
ASMExeSql()
{
    asm_user=$1
    asm_pwd=$2
    sql_file=$3
    rst_file=$4
    instance_name=$5
    timeout_second=$6
    is_silence=$7
    ASM_DB_USER=$8
    temp_file=${TMP_PATH}/tmprst${PID}

    if [ "${ASM_DB_USER}" = "" ]; then
        if [ "$VERSION" -ge "112" ]; then
            GetOracleUser
            ASM_DB_USER=${ORA_GRID_USER}
        else
            ASM_DB_USER=oracle
        fi
    fi
    chown ${ASM_DB_USER} "${sql_file}"
    chmod 700 "${sql_file}"
    CreateSTMPFile "${rst_file}"
    SQLPlusTimeOut ${timeout_second}
    if [ "${is_silence}" -eq "1" ]; then
        silent_mode="-S"
        sed -i "1i set pagesize 0" "${sql_file}"
        sed -i "1i set feedback off" "${sql_file}"
        sed -i "1i set newpage none" "${sql_file}"
        sed -i "1i set heading off" "${sql_file}"
    fi

    if [ "`cat /etc/passwd | grep "^${ASM_DB_USER}:" | $MYAWK -F "/" '{print $NF}'`" = "csh" ]; then
        str_set_env="setenv ORACLE_SID ${instance_name};"
    else
        str_set_env="ORACLE_SID=${instance_name}; export ORACLE_SID;"
    fi
    if [ "$VERSION" -ge "112" ]; then
        asm_shell_type=${GRID_SHELLTYPE}
        asm_role="sysasm"
    else
        asm_shell_type=${ORACLE_SHELLTYPE}
        asm_role="sysdba"
    fi
    if [ "${asm_user}" = "" ] || [ "${asm_pwd}" = "" ]; then
        su - ${ASM_DB_USER} ${asm_shell_type} -c "${EXPORT_ORACLE_ENV} ${str_set_env} sqlplus -L ${silent_mode} '/ as ${asm_role}' @'${sql_file}' > '${temp_file}'" >> "${LOG_FILE_NAME}" 2>&1
    else
        echo ${asm_pwd} | su - ${ASM_DB_USER} ${asm_shell_type} -c "${EXPORT_ORACLE_ENV} ${str_set_env} sqlplus -L ${silent_mode} '${asm_user} as ${asm_role}' @'${sql_file}' > '${temp_file}'" >> "${LOG_FILE_NAME}" 2>&1
        # 因为密码是输入进去的，会导致结果文件第一行为空行
        su - ${ASM_DB_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV} sed -i '1d' ${temp_file}"
    fi

    # not check result code, because when user password error, the result code is 1,but need to check result
    # now check result file exist
    if [ ! -f "$temp_file" ]; then
        KillProcMonitor $$
        Log "asm execute sql script failed."
        return $ERROR_SCRIPT_EXEC_FAILED
    fi
    su - ${ASM_DB_USER} ${db_shell_type} -c "${EXPORT_GRID_ENV}cat \"$temp_file\"" >> "${rst_file}" 2>&1
    su - ${ASM_DB_USER} ${db_shell_type} -c "${EXPORT_GRID_ENV}rm -rf \"$temp_file\""
    KillProcMonitor $$

    local oraErrMap="ORA-12560~TNS protocol adapter error.~${ERROR_ORACLE_TNS_PROTOCOL_ADAPTER}#\
    ORA-01034~ORACLE not available.~${ERROR_INSTANCE_NOSTART}#\
    ORA-00020~can not connect, maximum number of processes exceeded.~${ERROR_ORACLE_APPLICATION_OVER_MAX_LINK}#\
    ORA-15013~already mount.~${ERROR_ORACLE_ASM_DISKGROUP_ALREADYMOUNT}#\
    ORA-15001~diskgroup not mount or not exist.~${ERROR_ORACLE_ASM_DISKGROUP_NOTMOUNT}"

    local index=1
    local errInfo=
    while true; do
        errInfo=`echo ${oraErrMap} | cut -d "#" -f${index}`
        if [ "${errInfo}" = "" ]; then
            break
        fi
        index=`expr ${index} + 1`
        local errFlag=`echo $errInfo | ${MYAWK} -F "~" '{print $1}'`
        local errDetail=`echo $errInfo | ${MYAWK} -F "~" '{print $2}'`
        local errCode=`echo $errInfo | ${MYAWK} -F "~" '{print $3}'`

        local ERROR_NUM=`cat "${rst_file}" | grep "${errFlag}" | wc -l`
        if [ ${ERROR_NUM} -ne 0 ]; then
            Log "Sql($instance_name) execute script failed: ${errDetail}"
            WriteErrInfoToFile "${rst_file}"
            return ${errCode}
        fi
    done

    HAVEERROR=`cat "${rst_file}" | grep "^ORA-" | wc -l`
    if [ "$HAVEERROR" -ne "0" ]; then
        Log "=====================ASM(${ASMS_INSTNAME}) exec script failed.==========================="
        cat "${rst_file}" >> "${LOG_FILE_NAME}"
        WriteErrInfoToFile "${rst_file}"
        Log "=====================ASM(${ASMS_INSTNAME}) exec script failed.==========================="
        return $ERROR_ORACLE_EXEASMCMD_FAILED
    fi
    return 0
}

# ************************** check instance status ***********************************
GetOracleInstanceStatus()
{
    ORA_INSTANCENAME=$1
    CHECK_INSTANCE_STATUS="${STMP_PATH}/CheckInstanceStatus${PID}.sql"
    CHECK_INSTANCE_STATUSRST="${STMP_PATH}/CheckInstanceStatusRST${PID}.txt"

    echo "select status from v\$instance;" > "${CHECK_INSTANCE_STATUS}"
    echo "exit" >> "${CHECK_INSTANCE_STATUS}"
    
    Log "Exec SQL to get status of instance."
    OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${CHECK_INSTANCE_STATUS}" "${CHECK_INSTANCE_STATUSRST}" "${ORA_INSTANCENAME}" 600 0 >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ "${RET_CODE}" != "0" ]; then
        DeleteFile "${CHECK_INSTANCE_STATUS}"
        DeleteFile "${CHECK_INSTANCE_STATUSRST}"
        return ${RET_CODE}
    else        
        #STARTED - After STARTUP NOMOUNT
        #MOUNTED - After STARTUP MOUNT or ALTER DATABASE CLOSE
        #OPEN - After STARTUP or ALTER DATABASE OPEN
        #OPEN MIGRATE - After ALTER DATABASE OPEN { UPGRADE | DOWNGRADE }
        #get result like below, and get the content between ----- and black
        #   Connected to:
        #   Oracle Database 11g Enterprise Edition Release 11.2.0.3.0 - 64bit Production
        #   With the Partitioning, OLAP, Data Mining and Real Application Testing options
        #
        #
        #   STATUS
        #   ------------
        #   OPEN
        #
        #   Disconnected from Oracle Database 11g Enterprise Edition Release 11.2.0.3.0 - 64bit Production
        INSTANCESTATUS=`sed -n '/----------/,/^ *$/p' "${CHECK_INSTANCE_STATUSRST}" | sed -e '/----------/d' -e '/^ *$/d' | ${MYAWK} '{print $1}'`
        Log "INSTANCESTATUS=${INSTANCESTATUS}."
        DeleteFile "${CHECK_INSTANCE_STATUS}"
        DeleteFile "${CHECK_INSTANCE_STATUSRST}"
    fi
    
    return 0
}

#Create temporary sql script function, eg: CrtTmpSql ResultFile SqlScrpitFile Sql Section
CrtTmpSql()
{
    if [ $# -ne 4 ]
    then
        Log "Create tempory sql script failed, number of parameter is not correctly."
        return $ERROR_SCRIPT_EXEC_FAILED
    fi

    echo "set serveroutput on" > "$2"
    echo "set echo off" >> "$2"
    echo "set feedback off" >> "$2"
    echo "set heading off" >> "$2"
    echo "set verify off" >> "$2"
    echo "set trimspool off" >> "$2"
    echo "set trims on" >> "$2"
    echo "spool $1" >> "$2"
    echo "declare cursor cur_tblspace is $3" >> "$2"
    echo "begin" >> "$2"
    echo "    for ct in cur_tblspace loop" >> "$2"
    echo "  dbms_output.put_line('' || ct.$4 || '');" >> "$2"
    echo "  end loop;" >> "$2"
    echo "end;" >> "$2"
    echo "/" >> "$2"
    echo "spool off;" >> "$2"
    echo "exit;" >> "$2"
}

# ************************** get database backup tablespace count ***********************************
GetBackupModeTBCount()
{
    CrtTmpSql "${BKFILENAME}" "${TMPBKSCRIPT}" "${QRY_BACKUP_TB_SQL}" "status"
    if [ $? -ne 0 ]
    then
        Log "Create tmp sql error."
        DeleteFile "${BKFILENAME}" "${TMPBKSCRIPT}"
        return $ERROR_SCRIPT_EXEC_FAILED
    fi
    
    DeleteFile "${BKFILENAME}"
    touch "${BKFILENAME}"
    CHMOD 666 "${BKFILENAME}"
    Log "Exec SQL to get backup tablespace."
    OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${TMPBKSCRIPT}" "${TMPBKSCRIPTRST}" "${DBINSTANCE}" 60 0 >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ "$RET_CODE" -ne "0" ]
    then
        DeleteFile "${BKFILENAME}" "${TMPBKSCRIPT}" "${TMPBKSCRIPTRST}"
        Log "Get active tablespace count failed."
        return $RET_CODE
    fi
    ACTIVE_TB_COUNT=`cat "${BKFILENAME}" | wc -l`
    DeleteFile "${BKFILENAME}" "${TMPBKSCRIPT}" "${TMPBKSCRIPTRST}"
    return 0
}

# ************************** get database not backup tablespace count ***********************************
GetNotBackupModeTBCount()
{
    GetOracleCDBType ${DBINSTANCE}
    RET_CODE=$?
    if [[ $RET_CODE -ne 0 ]] && [[ $RET_CODE -ne 1 ]]
    then
        return $RET_CODE
    elif [ $RET_CODE -eq 0 ]
    then
        QRY_NOT_BACKUP_TB_SQL="select * from v\$backup where status='NOT ACTIVE' and con_id not in (select con_id from v\$pdbs where name='PDB\$SEED' or open_mode!='READ WRITE');"
    fi
    
    CrtTmpSql "${BKFILENAME}" "${TMPBKSCRIPT}" "${QRY_NOT_BACKUP_TB_SQL}" "status"
    if [ $? -ne 0 ]
    then
        Log "Create tmp sql error."
        DeleteFile "${BKFILENAME}" "${TMPBKSCRIPT}"
        return $ERROR_SCRIPT_EXEC_FAILED
    fi

    DeleteFile ${BKFILENAME}
    touch "${BKFILENAME}"
    CHMOD 666 "${BKFILENAME}"
    Log "Exec SQL to get no backup tablespace."
    OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${TMPBKSCRIPT}" "${TMPBKSCRIPTRST}" "${DBINSTANCE}" 60 0 >> "${LOG_FILE_NAME}" 2>&1
    if [ $? -ne 0 ]
    then
        DeleteFile "${BKFILENAME}" "${TMPBKSCRIPT}" "${TMPBKSCRIPTRST}"
        Log "Get not active tablespace count failed."
        return $ERROR_SCRIPT_EXEC_FAILED
    fi
    NOT_ACTIVE_TB_COUNT=`cat "${BKFILENAME}" | wc -l`
    DeleteFile "${BKFILENAME}" "${TMPBKSCRIPT}" "${TMPBKSCRIPTRST}"
    return 0
}

# **************************************** Create ASM start SQL ***************************
CreateASMStartSql()
{
    echo "startup;" > "$1"
    echo "exit" >> "$1"
}

GetOracleBasePath()
{
    local ORA_DB_RUN_USER=$1
    if [ -z "${ORA_DB_RUN_USER}" ];then
        GetOracleUser
        ORA_DB_RUN_USER=${ORA_DB_USER}
    fi
    CreateOracleSTMPFile ${ORATMPINFO}
    su - ${ORA_DB_RUN_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV}echo \$ORACLE_BASE >> \"${ORATMPINFO}\""
    IN_ORACLE_BASE=`su - ${ORA_DB_RUN_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV}cat \"\${ORATMPINFO}\""`
    if [ -z "$IN_ORACLE_BASE" ]; then
        Log "ORACLE_BASE Environment var is not exit."
        exit $ERROR_SCRIPT_ORACLEBASE_LOST
    fi
    DeleteFile "${ORATMPINFO}"
    Log "ORACLE_BASE=$IN_ORACLE_BASE"
}

GetOracleHomePath()
{
    local ORA_DB_RUN_USER=$1
    if [ -z "${ORA_DB_RUN_USER}" ];then
        GetOracleUser
        ORA_DB_RUN_USER=${ORA_DB_USER}
    fi
    CreateOracleSTMPFile ${ORATMPINFO}
    su - ${ORA_DB_RUN_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV}echo \$ORACLE_HOME >> \"${ORATMPINFO}\""
    IN_ORACLE_HOME=`su - ${ORA_DB_RUN_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV}cat \"\${ORATMPINFO}\""`
    if [ -z "$IN_ORACLE_HOME" ]; then
        Log "IN_ORACLE_HOME Environment var is not exit."
        exit $ERROR_SCRIPT_ORACLEHOME_LOST
    fi
    DeleteFile "${ORATMPINFO}"
    Log "ORACLE_HOME=$IN_ORACLE_HOME"
}

GetGridHomePath()
{
    if [ -z "${ORA_GRID_USER}" ];then
        GetOracleUser
    fi
    Log "Begin get grid HOME path!"
    CreateGridSTMPFile ${GRIDTMPINFO}
    su - ${ORA_GRID_USER} ${GRID_SHELLTYPE} -c "${EXPORT_GRID_ENV}echo \$ORACLE_HOME >> \"${GRIDTMPINFO}\""
    IN_GRID_HOME=`su - ${ORA_GRID_USER} ${GRID_SHELLTYPE} -c "${EXPORT_GRID_ENV}cat \"\${GRIDTMPINFO}\""`
    if [ -z "$IN_GRID_HOME" ]; then
        Log "IN_GRID_HOME Environment var is not exit."
        DeleteFile "${GRIDTMPINFO}"
        exit $ERROR_SCRIPT_ORACLEHOME_LOST
    fi
    DeleteFile "${GRIDTMPINFO}"
    Log "GRID_HOME=$IN_GRID_HOME."
}

GetORA_CRS_HOME()
{
    ORACLE_VERSION=$1
    GRID_USER=$2
    if [ -z "${GRID_USER}" ];then
        GetOracleUser
        GRID_USER=${ORA_GRID_USER}
    fi
    
    Log "Begin get oralce crs home path!"
    if [ "$ORACLE_VERSION" -ge "112" ]
    then
        CreateGridSTMPFile ${ORATMPINFO}
        su - ${GRID_USER} ${GRID_SHELLTYPE} -c "${EXPORT_GRID_ENV}echo \$ORACLE_HOME >> \"${ORATMPINFO}\""
        ORA_CRS_HOME=`su - ${GRID_USER} ${GRID_SHELLTYPE} -c "${EXPORT_GRID_ENV}cat \"${ORATMPINFO}\""`
        DeleteFile "${ORATMPINFO}"
    else
        ORA_CRS_HOME=${ORA_CRS_HOME}
    fi
    Log "ORA_CRS_HOME=${ORA_CRS_HOME}"
}

GetArchiveLogMode()
{
    DB_SID=$1

    GET_ARCHIVE_LOG_MODE_SQL="${STMP_PATH}/get_archive_log_mode${PID}.sql"
    ARCHIVE_LOG_MODE_RST="${STMP_PATH}/archive_log_mode_rst${PID}.txt"

    Log "Begin get archive log mode."
    echo "select LOG_MODE from v\$database;" > "${GET_ARCHIVE_LOG_MODE_SQL}"
    echo "exit" >> "${GET_ARCHIVE_LOG_MODE_SQL}"
    
    Log "Exec sql script to get archive log mode."
    OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${GET_ARCHIVE_LOG_MODE_SQL}" "${ARCHIVE_LOG_MODE_RST}" ${DB_SID} 30 0 >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ "$RET_CODE" -ne "0" ]
    then
        Log "Get archive log list failed."
        DeleteFile "${GET_ARCHIVE_LOG_MODE_SQL}" "${ARCHIVE_LOG_MODE_RST}"
        return ${RET_CODE}
    fi
    
    # non archive log mode
    ISARCHIVE=`cat "${ARCHIVE_LOG_MODE_RST}" | grep "NOARCHIVELOG"`
    if [ "$?" -eq "0" ]
    then
        Log "Non archive log mode."
        DeleteFile "${GET_ARCHIVE_LOG_MODE_SQL}" "${ARCHIVE_LOG_MODE_RST}"
        return 0
    fi
    
    DeleteFile "${GET_ARCHIVE_LOG_MODE_SQL}" "${ARCHIVE_LOG_MODE_RST}"
    Log "Get archive log mode succ."
    return 1
}

#
# need ${LOGIN_AUTH} ${QUERYCDBSCRIPT} ${QUERYCDBSCRIPTRST}
#           ${QUERYCDBSCRIPT} ${QUERYCDBSCRIPTRST}
# $1 $ORA_INSTANCENAME
# return 
# 1- not CDB
# 0- CDB
# else error code
#
GetOracleCDBType()
{    
    ORA_INSTANCENAME=$1
    
    #get Oracle version and check if it's 12
    GetOracleVersion ${ORA_DB_USER}
    VERSION=`echo $PREVERSION | $MYAWK -F "." '{print $1}'`
    Log "VERSION=${VERSION}"
    
    if [[ "${VERSION}" = "10" ]] || [[ "${VERSION}" = "11" ]];
    then
        Log "Oracle version($ORA_VERSION) not supports CDB."
        return 1
    fi

    QUERYCDBSCRIPT="$STMP_PATH/QueryCDB$PID.sql"
    QUERYCDBSCRIPTRST="$STMP_PATH/QueryCDBRST$PID.txt"
    #get CDB type
    echo "select cdb from v\$database;" > "${QUERYCDBSCRIPT}"
    echo "exit" >> "${QUERYCDBSCRIPT}"
    
    Log "Exec SQL to get CDB type of instance."
    OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${QUERYCDBSCRIPT}" "${QUERYCDBSCRIPTRST}" "${ORA_INSTANCENAME}" 30 0 >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ "${RET_CODE}" != "0" ]
    then
        DeleteFile "${QUERYCDBSCRIPT}" "${QUERYCDBSCRIPTRST}"
        exit ${RET_CODE}
    else
        ORACLECDBTYPE=`sed -n '/---/,/^ *$/p' "${QUERYCDBSCRIPTRST}" | sed -e '/---/d' -e '/^ *$/d'`
        Log "ORACLECDBTYPE=${ORACLECDBTYPE}."
        DeleteFile "${QUERYCDBSCRIPT}" "${QUERYCDBSCRIPTRST}"
    fi
    if [ "${ORACLECDBTYPE}" = "YES" ]
    then
        return 0
    else
        return 1
    fi
}

GetPDBStatus()
{
    local ORA_INSTANCENAME=$1
    local ORA_PDB_NAME=$2
    
    PDB_STATUS_FILE="${STMP_PATH}/PDBStatus${PID}.sql"
    PDB_STATUS_FILERST="${STMP_PATH}/PDBStatusRST${PID}.txt"
    
    touch ${PDB_STATUS_FILE}
    touch ${PDB_STATUS_FILERST}
    
    echo "select open_mode from v\$pdbs where NAME='${ORA_PDB_NAME}';" > "${PDB_STATUS_FILE}"
    echo "exit" >> "${PDB_STATUS_FILE}"
    
    Log "Exec SQL to get PDB List."
    OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${PDB_STATUS_FILE}" "${PDB_STATUS_FILERST}" "${ORA_INSTANCENAME}" 30 0 >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ ${RET_CODE} -ne 0 ]
    then
        DeleteFile "${PDB_STATUS_FILE}" "${PDB_STATUS_FILERST}"
        exit ${RET_CODE}
    else        
        #   OPEN_MODE
        #   ----------
        #   MOUNTED
        ORA_PDB_STATUS=`sed -n '/----------/,/^ *$/p' "${PDB_STATUS_FILERST}" | sed -e '/----------/d' -e '/^ *$/d'`
        DeleteFile "${PDB_STATUS_FILE}" "${PDB_STATUS_FILERST}"
        return 0
    fi
    DeleteFile "${PDB_STATUS_FILE}" "${PDB_STATUS_FILERST}"
    return 1
}

GetPDBNameByConID()
{
    local ORA_INSTANCENAME=$1
    local CON_ID=$2
    PDB_NAME_FILE="${STMP_PATH}/PDBName${PID}.sql"
    PDB_NAME_FILERST="${STMP_PATH}/PDBNameRST${PID}.txt"
    echo "select name from v\$pdbs where con_id='${CON_ID}';" > "${PDB_NAME_FILE}"
    echo "exit" >> "${PDB_NAME_FILE}"

    OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${PDB_NAME_FILE}" "${PDB_NAME_FILERST}" "${ORA_INSTANCENAME}" 30 0 >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ ${RET_CODE} -ne 0 ]; then
        DeleteFile "${PDB_NAME_FILE}" "${PDB_NAME_FILERST}"
        Log "Exec SQL to get PDB name by con_id failed, ret=${RET_CODE}."
        exit ${RET_CODE}
    else
        ORA_PDB_NAME=`sed -n '/----------/,/^ *$/p' "${PDB_NAME_FILERST}" | sed -e '/----------/d' -e '/^ *$/d'`
        DeleteFile "${PDB_NAME_FILE}" "${PDB_NAME_FILERST}"
        return 0
    fi 
}

GetOracleUser()
{
    ORA_DB_USER=`ls -l /etc/oratab | $MYAWK '{print $3}'`
    if [ -z "${ORA_DB_USER}" ]; then
        Log "Get oracle user failed, set default user oracle."
        ORA_DB_USER=oracle
    fi
    ORA_DB_GROUP=`groups $ORA_DB_USER | ${MYAWK} '{print $3}'`

    local crshome=`cat /etc/oracle/olr.loc 2>/dev/null | grep "crs_home" | $MYAWK -F= '{print $NF}'`
    if [ -z "$crshome" ]; then
        Log "Get crshome failed, set default user grid."
        ORA_GRID_USER=grid
    else
        ORA_GRID_USER=`ls -l ${crshome}/bin/oracle 2>/dev/null | $MYAWK '{print $3}'`
        if [ -z "${ORA_GRID_USER}" ]; then
            Log "Get grid user by ${crshome} failed, set default user grid."
            ORA_GRID_USER=grid
        fi
    fi
    ORA_GRID_GROUP=`groups $ORA_GRID_USER 2>/dev/null | ${MYAWK} '{print $3}'`
}

# need to get realy instance name
# not cluster, instance name is same as parameter
# cluster, instance name need to get instace number, eg. instance dbtest, dbtest1 or dbtest2
GetRealInstanceName()
{
    local InstName=$1
    if [ ${DBISCLUSTER} -eq 0 ]; then
        echo ${InstName}
    else
        local dbNum=`ps -ef | grep -nE "^ora_...._${InstName}$|ora_...._${InstName}" | grep -v grep | $MYAWK '{print $NF}' | uniq | $MYAWK -F "_" '{print $NF}' | uniq | wc -l`
        if [ "${dbNum}" -ne "1" ]; then
            Log "instance ${InstName} have invalid number about ${InstName}." 
            Log "`ps -ef | grep -nE "^ora_...._${InstName}$|ora_...._${InstName}" | grep -v grep | $MYAWK '{print $NF}' | uniq`"
            echo ${InstName}
        else
            echo `ps -ef | grep -nE "^ora_...._${InstName}$|ora_...._${InstName}" | grep -v grep | $MYAWK '{print $NF}' | uniq | \
            grep "^ora_pmon" | $MYAWK -F "ora_pmon_" '{print $NF}' | uniq`
        fi
    fi
}

CheckSqlPlusStatus()
{
    su - $ORA_DB_USER $ORACLE_SHELLTYPE -c "${EXPORT_ORACLE_ENV}which sqlplus" >> "${LOG_FILE_NAME}" 2>&1 || ExitWithError "check sqlplus failed"
}

CheckRmanStatus()
{
    su - $ORA_DB_USER $ORACLE_SHELLTYPE -c "${EXPORT_ORACLE_ENV}which rman" >> "${LOG_FILE_NAME}" 2>&1 || ExitWithError "check rman status failed"
}

ora_err()
{
    echo "$@" | $MYAWK -F : '/^ORA-[0-9]+/{print $1; exit}'
}

GetBCTStatus()
{
    DB_SID=$1
    Log "Begin get block_change_tracking status."
    BCT_STATUS_FILE="${STMP_PATH}/BCTStatus${PID}.sql"
    BCT_STATUS_FILERST="${STMP_PATH}/BCTStatusRST${PID}.txt"

    echo "select status from v\$block_change_tracking;" > "${BCT_STATUS_FILE}"
    echo "exit" >> "${BCT_STATUS_FILE}"
    
    Log "Exec sql script to get BCT status."
    OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${BCT_STATUS_FILE}" "${BCT_STATUS_FILERST}" ${DB_SID} 30 0 >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ "$RET_CODE" -ne "0" ]
    then
        Log "Get BCT status failed, ret=$RET_CODE"
        cat $BCT_STATUS_FILERST | grep "No such file or directory" >> "${LOG_FILE_NAME}" 2>&1
        if [ ! -z $? ]; then
            SetBCTStatusDisable $DB_SID
            BCT_STATUS="DISABLED"
        else
            exit ${RET_CODE}
            DeleteFile "${BCT_STATUS_FILE}" "${BCT_STATUS_FILERST}"
        fi
    else 
        BCT_STATUS=`sed -n '/----------/,/^ *$/p' "${BCT_STATUS_FILERST}" | sed -e '/----------/d' -e '/^ *$/d'`
    fi
    DeleteFile "${BCT_STATUS_FILE}" "${BCT_STATUS_FILERST}"
    Log "Get BCT status succ, BCTStatus=$BCT_STATUS"
    return 0
}

SetBCTStatusDisable()
{
    DB_SID=$1
    Log "Begin set block_change_tracking disable status."
    BCT_STATUS_FILE="${STMP_PATH}/BCTStatus${PID}.sql"
    BCT_STATUS_FILERST="${STMP_PATH}/BCTStatusRST${PID}.txt"

    echo "alter database disable block change tracking;" > "${BCT_STATUS_FILE}"
    echo "exit" >> "${BCT_STATUS_FILE}"
    
    Log "Exec sql script to disable BCT status."
    OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${BCT_STATUS_FILE}" "${BCT_STATUS_FILERST}" ${DB_SID} 30 0 >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ "$RET_CODE" -ne "0" ]
    then
        Log "set BCT status failed, ret=$RET_CODE"
        DeleteFile "${BCT_STATUS_FILE}" "${BCT_STATUS_FILERST}"
        exit ${RET_CODE}
    fi
    
    DeleteFile "${BCT_STATUS_FILE}" "${BCT_STATUS_FILERST}"
    Log "set BCT status succ, BCTStatus=Disable"
    return 0
}

GetDBCreateFileDest()
{
    DB_SID=$1
    Log "Begin get db_create_file_dest."
    GET_BCT_FILE="${STMP_PATH}/GetDBCreateFileDest${PID}.sql"
    GET_BCT_FILERST="${STMP_PATH}/GetDBCreateFileDestRST${PID}.txt"

    echo "select value from v\$parameter where name='db_create_file_dest';" > "${GET_BCT_FILE}"
    echo "exit" >> "${GET_BCT_FILE}"
    Log "Exec sql script to get db_create_file_dest."
    OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${GET_BCT_FILE}" "${GET_BCT_FILERST}" ${DB_SID} 30 0 >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ "$RET_CODE" -ne "0" ]
    then
        Log "Get db_create_file_dest failed, ret=$RET_CODE"
        DeleteFile "${GET_BCT_FILE}" "${GET_BCT_FILERST}"
        exit ${RET_CODE}
    fi
    DBCreateFileDest=`sed -n '/----------/,/^ *$/p' "${GET_BCT_FILERST}" | sed -e '/----------/d' -e '/^ *$/d'`
    DeleteFile "${GET_BCT_FILE}" "${GET_BCT_FILERST}"
    Log "Get db_create_file_dest succ, DBCreateFileDest=$DBCreateFileDest"
    return 0
}

EnableBCT()
{
    DB_SID=$1
    ORACLE_HOME=$2
    GetBCTStatus $DB_SID

    if [ "${BCT_STATUS}" != "ENABLED" ]; then
        BCT_FILEPATH=""
        GetDBCreateFileDest $DB_SID

        if [ "$DBCreateFileDest" = "" ]; then
            if [ ! -z "$ORACLE_BASE" ]; then
                BCT_FILEPATH="$ORACLE_BASE"
            else
                BCT_FILEPATH=$ORACLE_HOME
            fi
            BCT_FILEPATH="$BCT_FILEPATH/$DB_SID"
            mkdir -p $BCT_FILEPATH
            chown -h -R ${ORA_DB_USER}:${ORA_DB_GROUP} $BCT_FILEPATH
        else
            BCT_FILEPATH=${DBCreateFileDest}
        fi
        Log "Get Change Tracking file path is ${BCT_FILEPATH}."
        
        # enable BCT
        ENABLE_BCT_FILE="${STMP_PATH}/EnableBCT${PID}.sql"
        ENABLE_BCT_FILERST="${STMP_PATH}/EnableBCTRST${PID}.txt"

        echo "alter database enable block change tracking using file '$BCT_FILEPATH/rman_change_track.f' reuse;" > ${ENABLE_BCT_FILE}
        echo "exit" >> ${ENABLE_BCT_FILE}

        OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${ENABLE_BCT_FILE}" "${ENABLE_BCT_FILERST}" ${DB_SID} ${MAX_OPDB_TIME} 0 >> "${LOG_FILE_NAME}" 2>&1
        RET_CODE=$?
        if [ "$RET_CODE" -ne "0" ]
        then
            Log "Enable BCT failed, `cat ${ENABLE_BCT_FILERST}`."
            DeleteFile "${ENABLE_BCT_FILE}" "${ENABLE_BCT_FILERST}"
            return ${RET_CODE}
        fi
        DeleteFile "${ENABLE_BCT_FILE}" "${ENABLE_BCT_FILERST}"
    else
        Log "BCT is already enabled."
    fi
    return 0
}

GetOpenMode()
{
    DB_SID=$1
    Log "Begin to get openmode."
    OPEN_MODE_FILE="${STMP_PATH}/OpenMode${PID}.sql"
    OPEN_MODE_FILERST="${STMP_PATH}/OpenModeRST${PID}.txt"

    echo "select OPEN_MODE from v\$database;" > "${OPEN_MODE_FILE}"
    echo "exit" >> "${OPEN_MODE_FILE}"
    Log "Exec sql script to get open mode."
    OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${OPEN_MODE_FILE}" "${OPEN_MODE_FILERST}" ${DB_SID} 30 0 >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ "$RET_CODE" -eq "$ERROR_ORACLE_DB_NOT_COMPLETE_SHUTDOWN" ]
    then
        OpenMode="idle"
        DeleteFile "${OPEN_MODE_FILE}" "${OPEN_MODE_FILE}"
        Log "Get OpenMode value succ, OpenMode=$OpenMode"
    return 0
    elif [ "$RET_CODE" -eq "${ERROR_ORACLE_NOT_MOUNTED}" ]
    then
        OpenMode="nomount"
        DeleteFile "${OPEN_MODE_FILE}" "${OPEN_MODE_FILE}"
        Log "Get OpenMode value succ, OpenMode=$OpenMode"
    return 0
    elif [ "$RET_CODE" -ne "0" ]
    then
        Log "Get OpenMode failed, ret_code=$RET_CODE"
        DeleteFile "${OPEN_MODE_FILE}" "${OPEN_MODE_FILERST}"
        return ${RET_CODE}    
    else
        OpenMode=`sed -n '/----------/,/^ *$/p' "${OPEN_MODE_FILERST}" | sed -e '/----------/d' -e '/^ *$/d'`
        if [ "$OpenMode" = "MOUNTED" ]; then
            OpenMode="mount"
        elif [ "$OpenMode" = "READ WRITE" ]; then
            OpenMode="open"
        else
            OpenMode="Unknown"
        fi
        DeleteFile "${OPEN_MODE_FILE}" "${OPEN_MODE_FILE}"
        Log "Get OpenMode value succ, OpenMode=$OpenMode"
    return 0
    fi
    
}

Spile()
{
    DB_SID=$1
    GetOpenMode $DB_SID
    RET_CODE=$?
    if [ "$RET_CODE" -ne "0" ]
    then
        Log "GetOpenMode failed, ret=$RET_CODE"
        return ${RET_CODE}
    fi

    Log "Begin to get Spile."
    SPILE_FILE="${STMP_PATH}/OpenMode${PID}.sql"
    SPILE_FILERST="${STMP_PATH}/OpenModeRST${PID}.txt"
    touch ${SPILE_FILE}
    touch ${SPILE_FILERST}
    echo "select value from v\$parameter where name='spfile';" > "${SPILE_FILE}"
    echo "exit" >> "${SPILE_FILE}"
    Log "Exec sql script to get open mode."
    OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${SPILE_FILE}" "${SPILE_FILERST}" ${DB_SID} 30 0 >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ "$RET_CODE" -ne "0" ]
    then
        Log "Get Spile failed."
        DeleteFile "${SPILE_FILE}" "${SPILE_FILERST}"
        return ${RET_CODE}
    fi
    SpileValue=`sed -n '/----------/,/^ *$/p' "${SPILE_FILERST}" | sed -e '/----------/d' -e '/^ *$/d'`
    DeleteFile "${SPILE_FILE}" "${SPILE_FILERST}"

    Log "Get Spile Value succ,SpileValue=$SpileValue."
    return 0
}

GetDBfiles()
{
    DB_SID=$1
    local fileType=$2
    GET_DBFILES_SQL="${STMP_PATH}/oracledbfiles${PID}.sql"
    DBFILE_RST="${STMP_PATH}/oracledbfilesRst${PID}.txt"
    echo "set linesize 999" > "${GET_DBFILES_SQL}"
    if [ "${fileType}" = "0" ]; then
        Log "Begin to get control files ."
        echo "select name from v\$controlfile;" >> "${GET_DBFILES_SQL}"
    elif [ "${fileType}" = "1" ]; then
        Log "Begin to get spfile files."
        echo "select VALUE from v\$parameter where name='spfile';" >> "${GET_DBFILES_SQL}"
    elif [ "${fileType}" = "2" ]; then
        Log "Begin to get data guard configuration files."
        echo "select destination || '/*' from v\$archive_dest where destination like '/%';" >> "${GET_DBFILES_SQL}"    
    else
        Log "file type ${fileType} is not supported."
        return
    fi
    echo "exit;" >> "${GET_DBFILES_SQL}"

    OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${GET_DBFILES_SQL}" "${DBFILE_RST}" "${DB_SID}" 30 0 "${ORA_DB_USER}" >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ "$RET_CODE" -ne "0" ]; then
        Log "Get oracle files failed, ret="$RET_CODE""
        DeleteFile "${GET_DBFILES_SQL}" "${DBFILE_RST}"
        exit ${RET_CODE}
    fi
    TMPLINE=`sed -n '/----------/,/^ *$/p' "${DBFILE_RST}" | sed -e '/----------/d' -e '/^ *$/d'`
    DeleteFile "${GET_DBFILES_SQL}" "${DBFILE_RST}"
    if [ "${fileType}" = "0" ]; then
        echo "${TMPLINE}" > $ADDITIONAL/ctrlfiles
    elif [ "${fileType}" = "1" ]; then
        echo "${TMPLINE}" > $ADDITIONAL/spfile
    elif [ "${fileType}" = "2" ]; then
        echo "${TMPLINE}" > $ADDITIONAL/dataguardconffiles
    else
        Log "file type ${fileType} is not supported."
        return
    fi
}

GetLogGroupIDAndFiles()
{
    DB_SID=$1
    DEST_FILE=$2
    GET_LOGFILES_SQL="${STMP_PATH}/oraclelogfiles${PID}.sql"
    LOGFILE_RST="${STMP_PATH}/oraclelogfilesRst${PID}.txt"
    echo "set linesize 999" > "${GET_LOGFILES_SQL}"
    echo "col MEMBER for a255" >> "${GET_LOGFILES_SQL}"
    echo "select GROUP#, MEMBER from v\$logfile order by GROUP#;" >> "${GET_LOGFILES_SQL}"
    echo "exit;" >> "${GET_LOGFILES_SQL}"

    OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${GET_LOGFILES_SQL}" "${LOGFILE_RST}" "${DB_SID}" 30 1 "${ORA_DB_USER}" >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ "$RET_CODE" -ne "0" ]; then
        Log "Get oracle log file list failed, ret=$RET_CODE"
        DeleteFile "${GET_LOGFILES_SQL}" "${LOGFILE_RST}"
        exit ${RET_CODE}
    fi
    [ ! -f "$DEST_FILE" ] && CreateDir "$DEST_FILE"
    echo "" > "$DEST_FILE/logfiles"
    while read line
    do
        [ -z "$line" ] && continue
        echo $line | $MYAWK -v OFS=";" '{print $1,$2}' >> "$DEST_FILE/logfiles"
    done < "$LOGFILE_RST"
    DeleteFile "${GET_LOGFILES_SQL}" "${LOGFILE_RST}"
}

GetFilePathAndTableSpace()
{
    # 0 datafile path, 1 tempfile path
    # restriction
    # 0 VERSION: oracle vesion, format, 11gR2=>11.2, 12cR1=>12.1
    DB_SID=$1
    local fileType=$2
    GET_FSANDTS_SQL="${STMP_PATH}/oraclefsandts${PID}.sql"
    FSANDTS_RST="${STMP_PATH}/oraclefsandtsRst${PID}.txt"
    echo "set linesize 999" > "${GET_FSANDTS_SQL}"
    if [ "${fileType}" = "0" ]; then
        Log "Begin to get DB datafile path and tablespace information."
        # need to get data file list of cdb and pdb when version begger than 12c
        if [ "$VERSION" -ge "121" ]; then
            echo "col tsName for a30" >> "${GET_FSANDTS_SQL}"
            echo "col tsFile for a520" >> "${GET_FSANDTS_SQL}"
            echo "SELECT t.CON_ID CON_ID, t.Name tsName, f.File# fNo, f.Name tsFile" >> "${GET_FSANDTS_SQL}"
            echo "FROM V\$TABLESPACE t, V\$DATAFILE f WHERE t.TS# = f.TS# and t.CON_ID=f.CON_ID" >> "${GET_FSANDTS_SQL}"
            echo "ORDER BY t.CON_ID;" >> "${GET_FSANDTS_SQL}"
        else
            echo "col TABLESPACE_NAME for a30" >> "${GET_FSANDTS_SQL}"
            echo "col FILE_NAME for a520" >> "${GET_FSANDTS_SQL}"
            echo "select 0,TABLESPACE_NAME,FILE_ID,FILE_NAME from dba_data_files;" >> "${GET_FSANDTS_SQL}"
        fi
    else
        Log "file type ${fileType} is not supported."
        return
    fi
    echo "exit;" >> "${GET_FSANDTS_SQL}"

    OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${GET_FSANDTS_SQL}" "${FSANDTS_RST}" "${DB_SID}" 30 1 "${ORA_DB_USER}" >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ "$RET_CODE" -ne "0" ]; then
        Log "Get oracle file list failed, ret=$RET_CODE"
        DeleteFile "${GET_FSANDTS_SQL}" "${FSANDTS_RST}"
        exit ${RET_CODE}
    fi
    
    [ -f "$ADDITIONAL/dbfiles" ] && DeleteFile "$ADDITIONAL/dbfiles"
    touch "$ADDITIONAL/dbfiles"
    while read line
    do
        [ -z "$line" ] && continue
        echo $line | $MYAWK -v OFS=";" '{print $1,$2,$3,$4}' >> $ADDITIONAL/dbfiles
    done < "$FSANDTS_RST"
    DeleteFile "${GET_FSANDTS_SQL}" "${FSANDTS_RST}"
}

ShutDownDB()
{
    DB_SID=$1
    # ShutDown abort
    [ -z "${DB_SID}" ] && ExitWithError "DB inatance"
    SHUTDOWN_FILE="${STMP_PATH}/ShutDownDB${PID}.sql"
    SHUTDOWN_FILERST="${STMP_PATH}/ShutDownDBRST${PID}.txt"
    echo "shutdown immediate;" > ${SHUTDOWN_FILE}
    echo "exit" >> ${SHUTDOWN_FILE}

    Log "Begin to shutdown database, instance(${DB_SID})."
    OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${SHUTDOWN_FILE}" "${SHUTDOWN_FILERST}" ${DB_SID} ${MAX_OPDB_TIME} 0 ${ORA_DB_USER} >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ "$RET_CODE" -ne "0" ] && [ $RET_CODE -ne $ERROR_ORACLE_NOT_OPEN ]; then
        DeleteFile "${SHUTDOWN_FILE}" "${SHUTDOWN_FILERST}"
        exit ${RET_CODE}
    fi
    DeleteFile "${SHUTDOWN_FILE}" "${SHUTDOWN_FILERST}"
    return 0
}

GetDBIDName()
{
    DBINSTANCE=$1
    SQLQUERYDBIDNAME="${STMP_PATH}/QueryDBIDName${PID}.sql"
    QUERYDBIDNAMERST="${STMP_PATH}/QueryDBIDNameRST${PID}.txt"
    echo "set linesize 300;" > "$SQLQUERYDBIDNAME"
    echo "select DBID, db_unique_name from v\$database;" >> "$SQLQUERYDBIDNAME"
    echo "exit" >> "$SQLQUERYDBIDNAME"

    Log "INFO:Exec SQL to get DBID and DBName."
    OracleExeSql "${DBUSER}" "${DBUSERPWD}" ${SQLQUERYDBIDNAME} ${QUERYDBIDNAMERST} ${DBINSTANCE} 30 0 ${ORA_DB_USER} >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ ${RET_CODE} -ne 0 ]; then
        Log "ERROR:Exec SQL to get DBID and DBName failed,ret=${RET_CODE}, `cat ${QUERYDBIDNAMERST}`."
        DeleteFile "${SQLQUERYDBIDNAME}" "${QUERYDBIDNAMERST}"
        exit $RET_CODE
    fi
    DBID=`sed -n '/----------/,/^ *$/p' "${QUERYDBIDNAMERST}" | sed -e '/----------/d' -e '/^ *$/d' | $MYAWK '{print $1}'`
    UNIQ_DBNAME=`sed -n '/----------/,/^ *$/p' "${QUERYDBIDNAMERST}" | sed -e '/----------/d' -e '/^ *$/d' | $MYAWK '{print $2}'`
    DeleteFile "${SQLQUERYDBIDNAME}" "${QUERYDBIDNAMERST}"
    return 0
}

GetIncarnations()
{
    local DBINSTANCE=$1
    SQLQUERYDBINCAR="${STMP_PATH}/QueryDBIncar${PID}.sql"
    QUERYDBINCARRST="${STMP_PATH}/QueryDBIncarRST${PID}.txt"

    echo "SELECT INCARNATION# FROM V\$DATABASE_INCARNATION WHERE STATUS='CURRENT';" >> $SQLQUERYDBINCAR
    echo "exit" >> $SQLQUERYDBINCAR

    Log "INFO: Exec SQL to get database incarnation."
    OracleExeSql "${DBUSER}" "${DBUSERPWD}" ${SQLQUERYDBINCAR} ${QUERYDBINCARRST} ${DBINSTANCE} 30 1 ${ORA_DB_USER} >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ ${RET_CODE} -ne 0 ]; then
        Log "ERROR:Exec SQL to get incranation failed, ret=${RET_CODE}, `cat ${QUERYDBINCARRST}`."
        DeleteFile "${SQLQUERYDBINCAR}" "${QUERYDBINCARRST}"
        exit $RET_CODE
    fi

    INCARNATION_NUMBER=`cat $QUERYDBINCARRST | sed -r 's/[ \t]+//g'`
    DeleteFile "${SQLQUERYDBINCAR}" "${QUERYDBINCARRST}"
    Log "current incarnation is ${INCARNATION_NUMBER}."
    return 0
}

GetBackupStatusAndSize()
{
    DBINSTANCE=$1
    SQLQUERYBKSTATUSANDSIZE="${STMP_PATH}/QueryBKStatusSize${PID}.sql"
    QUERYBKSTATUSANDSIZERST="${STMP_PATH}/QueryBKStatusSizeRST${PID}.txt"
    echo "set linesize 300;" > "$SQLQUERYBKSTATUSANDSIZE"
    echo "select p.* from(select TRIM (output_bytes_display)size, STATUS from V\$RMAN_BACKUP_JOB_DETAILS WHERE start_time > TRUNC (SYSDATE) - 240 order by start_time desc)p where rownum = 1;" >> "$SQLQUERYBKSTATUSANDSIZE"
    echo "exit" >> "$SQLQUERYBKSTATUSANDSIZE"

    OracleExeSql "${DBUSER}" "${DBUSERPWD}" ${SQLQUERYBKSTATUSANDSIZE} ${QUERYBKSTATUSANDSIZERST} ${DBINSTANCE} 30 0 ${ORA_DB_USER} >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ ${RET_CODE} -ne 0 ]; then
        DeleteFile "${SQLQUERYBKSTATUSANDSIZE}" "${QUERYBKSTATUSANDSIZERST}"
        return $RET_CODE
    fi
    SIZE=`sed -n '/----------/,/^ *$/p' "${QUERYBKSTATUSANDSIZERST}" | sed -e '/----------/d' -e '/^ *$/d' | $MYAWK '{print $1}'`
    STATUS=`sed -n '/----------/,/^ *$/p' "${QUERYBKSTATUSANDSIZERST}" | sed -e '/----------/d' -e '/^ *$/d' | $MYAWK '{print $2}'`
    DeleteFile "${SQLQUERYBKSTATUSANDSIZE}" "${QUERYBKSTATUSANDSIZERST}"
    return 0
}

# Current database system minimum scn
GetMinSysSCN()
{
    DBINSTANCE=$1
    SQLMinSysSCN="${STMP_PATH}/MinSysSCN${PID}.sql"
    MinSysSCNRST="${STMP_PATH}/MinSysSCNRST${PID}.txt"
    echo "set linesize 300;" > "$SQLMinSysSCN"
    echo "select scn from (select min(FIRST_CHANGE#) as scn "'from v$archived_log'" where $LOG_IS_VALID);" >> "$SQLMinSysSCN"
    echo "exit" >> "$SQLMinSysSCN"

    OracleExeSql "${DBUSER}" "${DBUSERPWD}" ${SQLMinSysSCN} ${MinSysSCNRST} ${DBINSTANCE} 30 0 ${ORA_DB_USER} >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ ${RET_CODE} -ne 0 ]; then
        Log "get system min scn failed, `cat ${MinSysSCNRST}`."
        DeleteFile "${SQLMinSysSCN}" "${MinSysSCNRST}"
        exit $RET_CODE
    fi
    min_sys_scn=`sed -n '/----------/,/^ *$/p' "${MinSysSCNRST}" | sed -e '/----------/d' -e '/^ *$/d' | $MYAWK '{print $1}'`
    Time_min_sys_scn=`sed -n '/----------/,/^ *$/p' "${MinSysSCNRST}" | sed -e '/----------/d' -e '/^ *$/d' | $MYAWK '{print $2}'`
    scn_before_backup= "$min_sys_scn $Time_min_sys_scn `AddUnixTimestamp $Time_min_sys_scn`"
    Log "Get system min scn is $min_sys_scn."

    DeleteFile "${SQLMinSysSCN}" "${MinSysSCNRST}"
}

GetFromSCN()
{
    DBINSTANCE=$1
    SQLFromScn="${STMP_PATH}/FromScn${PID}.sql"
    FromScnRST="${STMP_PATH}/FromScnRST${PID}.txt"
    echo "set linesize 300;" > "$SQLFromScn"
    echo "select max(next_change#) from v\$archived_log where $LOG_IS_BACKED_UP;" >> "$SQLFromScn"
    echo "exit" >> "$SQLFromScn"

    OracleExeSql "${DBUSER}" "${DBUSERPWD}" ${SQLFromScn} ${FromScnRST} ${DBINSTANCE} 30 0 ${ORA_DB_USER} >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ ${RET_CODE} -ne 0 ]; then
        DeleteFile "${SQLFromScn}" "${FromScnRST}"
        Log "ERROR:Exec SQL to Get From Scn failed,ret=${RET_CODE}"
        exit $RET_CODE
    fi
    from_scn=`sed -n '/----------/,/^ *$/p' "${FromScnRST}" | sed -e '/----------/d' -e '/^ *$/d' | $MYAWK '{print $1}'`
    DeleteFile "${SQLFromScn}" "${FromScnRST}"
    Log "Get from_scn is $from_scn."
    return 0
}

GetDbfMaxSCN()
{
    DBNAME=$1
    UPPER_DBNAME=$(echo $1 | tr '[a-z]' '[A-Z]')    
    Scn_dbf_max=0
    Time_scn_dbf_max=0
    # check first
    SQLGetTagCount="${STMP_PATH}/GetTagCount${PID}.sql"
    GetTagCountRST="${STMP_PATH}/GetTagCountRST${PID}.txt"
    echo "set linesize 300;" > "$SQLGetTagCount"
    echo "col NAME for a255;" >> "$SQLGetTagCount"
    echo "select count(*) from v\$datafile_copy where tag='EBACKUP-$UPPER_DBNAME-DATA';" >> "$SQLGetTagCount"
    echo "exit" >> "$SQLGetTagCount"
    OracleExeSql "${DBUSER}" "${DBUSERPWD}" ${SQLGetTagCount} ${GetTagCountRST} ${DBINSTANCE} 30 0 ${ORA_DB_USER} >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ ${RET_CODE} -ne 0 ]; then
        DeleteFile "${SQLGetTagCount}" "${GetTagCountRST}"
        return $RET_CODE
    fi
    TagCount=`sed -n '/----------/,/^ *$/p' "${GetTagCountRST}" | sed -e '/----------/d' -e '/^ *$/d' | $MYAWK '{print $1}'`
    DeleteFile "${SQLGetTagCount}" "${GetTagCountRST}"
    if [ "${TagCount}" = "0" ]; then
        Log "WARN:get max scn failed, result is `cat ${GetTagCountRST}.`"
        scn_dbf_max="$Scn_dbf_max $Time_scn_dbf_max `AddUnixTimestamp $Time_scn_dbf_max`"
        return 0
    fi

    SQLDBFMaxScn="${STMP_PATH}/DBFMaxScn${PID}.sql"
    DBFMaxScnRST="${STMP_PATH}/DBFMaxScnRST${PID}.txt"
    echo "set linesize 300;" > "$SQLDBFMaxScn"
    echo "col NAME for a255;" >> "$SQLDBFMaxScn"
    echo "select scn, to_char(scn_to_timestamp(scn), $TIME_FORMAT)
        from (select max(checkpoint_change#) as scn "'from v$datafile_copy'" where tag='EBACKUP-$UPPER_DBNAME-DATA' and (status='A') );" >> "$SQLDBFMaxScn"
    echo "exit" >> "$SQLDBFMaxScn"
    
    OracleExeSql "${DBUSER}" "${DBUSERPWD}" ${SQLDBFMaxScn} ${DBFMaxScnRST} ${DBINSTANCE} 30 0 ${ORA_DB_USER} >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ ${RET_CODE} -ne 0 ]; then
        DeleteFile "${SQLDBFMaxScn}" "${DBFMaxScnRST}"
        return $RET_CODE
    fi
    Scn_dbf_max=`sed -n '/----------/,/^ *$/p' "${DBFMaxScnRST}" | sed -e '/----------/d' -e '/^ *$/d' | $MYAWK '{print $1}'`
    Time_scn_dbf_max=`sed -n '/----------/,/^ *$/p' "${DBFMaxScnRST}" | sed -e '/----------/d' -e '/^ *$/d' | $MYAWK '{print $2}'`
    scn_dbf_max="$Scn_dbf_max $Time_scn_dbf_max `AddUnixTimestamp $Time_scn_dbf_max`"
    DeleteFile "${SQLDBFMaxScn}" "${DBFMaxScnRST}"
    return 0
}

# **************************************** Create ShutDown SQL ************************
CreateShutdownSql()
{
    echo "shutdown abort;" > "$1"
    echo "exit" >> "$1"
}
# **************************************** Create Recover SQL *************************
CreateRecoverSql()
{
    echo "startup nomount;" > "$1"
    echo "exit" >> "$1"
}

CreateEndBackupSql()
{
    echo "alter database end backup;" > "$1"
    echo "exit" >> "$1"
}

# **************************************** Create Startup SQL *************************
CreateStartupSql()
{
    echo "alter database mount;" > "$1"
    echo "exit" >> "$1"
}
# **************************************** Create Mount SQL ***************************
CreateOpenSql()
{
    echo "alter database open;" > "$1"
    echo "exit" >> "$1"
}

PrepareConfFile()
{
    # restriction
    # 1. backup path        BACKUP
    # 2. log path           ARCHIVE
    # 3. oracle user        ORA_DB_USER
    # 4. oracle group       ORA_DB_GROUP
    # 5. grid user          ORA_GRID_USER
    # 6. asm instance       ASMSIDNAME
    # 7. dbname             DBNAME
    # 8. pfile              PFILE_NAME
    # 9. pwd file           DBPW_FILE
    # 10.scn file           SCN_DBF_MAX
    # 11.data file list     DATA_FILES
    # 12.ctl file list      CTL_FILES
    # 13.log file list      LOG_FILES
    # 14.temp file list     TEMP_FILES
    # 15.meta data path     METADATAPATH
    
    Log "Begin to copy backup meta file"
    # check archive log directory
    if [ "`RDsubstr ${ARCHIVE} 1 1`" = "/" ]; then
        CheckDirRWX "$ORA_DB_USER" "$ARCHIVE" || chown -h -R ${ORA_DB_USER}:${ORA_DB_GROUP} "$ARCHIVE"
    fi

    CheckDirRWX "$ORA_DB_USER" "$backupMetaPath"  || chown -h -R ${ORA_DB_USER}:${ORA_DB_GROUP} "$backupMetaPath"
    # copy old pfile
    CreatePfilefromTempfile

    # copy oracle env file
    cp -d -f "$ADDITIONAL/env_file" ${ENV_FILE}
    [ $? -eq 0 ] || ExitWithErrorCode "copy env file failed." $ERROR_BACKUP_INVALID
    
    # copy old scn file
    cp -d -f "$ADDITIONAL/scn_dbf_max" ${SCN_DBF_MAX}
    [ $? -eq 0 ] || ExitWithErrorCode "copy scn max from fs failed." $ERROR_BACKUP_INVALID

    # copy data list file
    cp -d -f "$ADDITIONAL/dbfiles" ${DATA_FILES}
    [ $? -eq 0 ] || ExitWithErrorCode "copy data list file from fs failed." $ERROR_BACKUP_INVALID

    # copy control list file
    cp -d -f "$ADDITIONAL/ctrlfiles" ${CTL_FILES}
    [ $? -eq 0 ] || ExitWithErrorCode "copy control list file from fs failed." $ERROR_BACKUP_INVALID

    # copy log list file
    cp -d -f "$ADDITIONAL/logfiles" ${LOG_FILES}
    [ $? -eq 0 ] || ExitWithErrorCode "copy log list file from fs failed." $ERROR_BACKUP_INVALID

    # copy pwd file, pw file is not required
    local oldpw=`ls $ADDITIONAL/dbs/orapw*`
    if [ ! -z "${oldpw}" ]; then
        cp -d -f $oldpw ${DBPW_FILE} && chown -h ${ORA_DB_USER}:${ORA_DB_GROUP} ${DBPW_FILE}
        [ $? -eq 0 ] || ExitWithErrorCode "copy old pw file from fs failed." $ERROR_BACKUP_INVALID
    fi
}

CreateDir()
{
    local dirArry=`echo $1 | sed 's/;/ /g'`
    for dir in ${dirArry}; do
        if [ "`RDsubstr ${dir} 1 1`" = "/" ]; then
            if [ ! -d ${dir} ]; then
                local name=
                local minPath=""
                for name in `echo ${dir} | sed 's/\// /g'`; do
                    minPath="${minPath}/${name}"
                    [ ! -d "${minPath}" ] && break
                done
                mkdir -p "${dir}"
                chown -h -R ${ORA_DB_USER}:${ORA_DB_GROUP} ${minPath}
            else
                chown -h  ${ORA_DB_USER}:${ORA_DB_GROUP} ${dir}
            fi
        elif [ "`RDsubstr ${dir} 1 1`" = "+" ]; then
            local dgName=${dir%%/*}
            dgName=`RDsubstr ${dgName} 2`
            echo lsdg ${dgName} | su - ${ORA_GRID_USER} -c "${EXPORT_GRID_ENV}export ORACLE_SID=${ASMSIDNAME}; asmcmd" | grep "${dgName}"
            if [ $? -eq 0 ]; then
                local childPaths=${dir#*/}
                local tmpDir=${dir%%/*}
                if [ "${childPaths}" != "${tmpDir}" ]; then
                    childPaths=`echo ${childPaths} | sed "s#/# #g"`
                    for str in $childPaths; do
                        tmpDir="$tmpDir/$str"
                        su - ${ORA_GRID_USER} -c "${EXPORT_GRID_ENV}export ORACLE_SID=${ASMSIDNAME}; echo mkdir ${tmpDir} | asmcmd" >> "${LOG_FILE_NAME}" 2>&1
                    done
                fi
            fi
        fi
    done
}

PrepareDBEnv()
{
    # restriction
    # 1. asm instance       ASMSIDNAME
    Log "Begin to prepare DB env"
    local pFile=$1
    # create adump direcory
    local ADUMP_DIR=`cat ${pFile} | grep audit_file_dest | ${MYAWK} -F"'" '{print $2}'`
    CreateDir ${ADUMP_DIR}
    
    # create diag rdbms directory
    local DIAG_DIR=`cat ${pFile} | grep diagnostic_dest | ${MYAWK} -F"'" '{print $2}'`
    CreateDir ${DIAG_DIR}

    # create archive dest
    local RECOVERY_DIR=`cat ${pFile} | grep db_recovery_file_dest= | ${MYAWK} -F"'" '{print $2}'`
    CreateDir ${RECOVERY_DIR}

    # create archive dest  and   set owner
    local t_dbName=`echo $2| tr 'a-z' 'A-Z'`
    local RECOVERY_DB_DIR="${RECOVERY_DIR}/${t_dbName}"
    CreateDir ${RECOVERY_DB_DIR}
}

# copy bakcup files use specificDir
CopyfilesBySpecificDir()
{
    dbfPath=$1
    taskType=$2
    if [ "`RDsubstr ${dbfPath} 1 1`" = "/" ]; then
        for dbfFile in `ls ${MainBackupPath}/FNO-*_TS-*.dbf`; do
            tablespace_name=`echo $dbfFile | $MYAWK -F "/" '{print $NF}' | $MYAWK -F "_TS-" '{print $NF}' | $MYAWK -F "." '{print $1}'`
            echo "${NecessaryTSArr[@]}" | grep -i "$tablespace_name" >> ${LOG_FILE_NAME} 2>& 1
            [ $? -ne 0 ] && Log "get tablespace is $tablespace_name, will skip it." && continue
            Log "begin to copy $dbfFile to $dbfPath."
            \cp -d -f -v $dbfFile $dbfPath >> ${LOG_FILE_NAME} 2>& 1
            chown -h ${ORA_DB_USER}:${ORA_DB_GROUP} $dbfFile >> ${LOG_FILE_NAME} 2>& 1
            Log "finish to copy $dbfFile to $dbfPath."
        done
        chown -h ${ORA_DB_USER}:${ORA_DB_GROUP} $dbfPath/FNO-*_TS-*.dbf >> ${LOG_FILE_NAME} 2>& 1
        Log "Finished to copy datafile to $dbfPath."
    else
        for dbFileName in `ls ${MainBackupPath}/FNO-*_TS-*.dbf`; do
            tablespace_name=`echo $dbFileName | $MYAWK -F "_ts-" '{print $NF}' | $MYAWK -F "." '{print $1}'` 
            echo "${NecessaryTSArr[@]}" | grep -i "$tablespace_name" >> ${LOG_FILE_NAME} 2>& 1
            [ $? -ne 0 ] && Log "get tablespace is $tablespace_name, will skip it." && continue
            su - ${ORA_GRID_USER} -c "${EXPORT_GRID_ENV}export ORACLE_SID=${ASMSIDNAME}; echo cp ${MainBackupPath}/fno-*_ts-$tablespace_name.dbf $dbfPath | asmcmd" >> ${LOG_FILE_NAME} 2>& 1
            [ $? -ne 0 ] && ExitWithErrorCode "ASM copy $TSRealName failed." $ERROR_ORACLE_COPYFILES
            Log "begin to copy $dbfFileName to $dbfPath."
        done

        [ "`RDsubstr ${dbfPath} 1 1`" = "/" ] && chown -h ${ORA_DB_USER}:asmadmin ${dbfPath}
    fi
}

CopyCtrFiles()
{
    local ctrFilePath=`$MYAWK 'BEGIN{FS="/"; OFS="/"}{$NF=""; print}' ${CTL_FILES} | sed -n "1p"`
    dstCtrFilePath=`echo ${ctrFilePath%*/}`
    local UpperDBNAME=$(echo ${DBNAME} | tr '[a-z]' '[A-Z]')
    if [ ! -z "${RECOVERPATH}" ];then
        if [ $OMFEnable -eq 1 ]; then
            dstCtrFilePath="${RECOVERPATH}/${UpperDBNAME}/controlfile"
        else
            dstCtrFilePath="${RECOVERPATH}"
        fi
    fi
    Log "begin to copy ctrFile to $dstCtrFilePath ${CTL_FILES}."
    [ ! -d $dstCtrFilePath ] && CreateDir $dstCtrFilePath
    if [ "`RDsubstr ${dstCtrFilePath} 1 1`" = "/" ]; then
        cp -d -f  "${MainBackupPath}/controlfile.ctl" "${dstCtrFilePath}/controlfile.ctl" >> ${LOG_FILE_NAME} 2>& 1
        chown -h ${ORA_DB_USER}:${ORA_DB_GROUP} "${dstCtrFilePath}/controlfile.ctl"
    else
        su - ${ORA_GRID_USER} -c "${EXPORT_GRID_ENV}export ORACLE_SID=${ASMSIDNAME}; echo cp ${MainBackupPath}/controlfile.ctl ${dstCtrFilePath}/controlfile.ctl | asmcmd" >> ${LOG_FILE_NAME} 2>& 1
        [ $? -ne 0 ] && ExitWithErrorCode "ASM copy controlfile failed." $ERROR_ORACLE_COPYFILES
        [ "`RDsubstr ${dstCtrFilePath} 1 1`" = "/" ] && chown -h ${ORA_DB_USER}:asmadmin "${dstCtrFilePath}/controlfile.ctl"
        [ "`RDsubstr ${RECOVERPATH} 1 1`" = "/" ] && chown -h ${ORA_DB_USER}:asmadmin ${RECOVERPATH}
    fi
}

ReplaceBaseAndHome()
{
    # 1.oracle_base         IN_ORACLE_BASE
    # 2.oracle_home         IN_ORACLE_HOME
    oldBase=$1
    oldHome=$3
    fileName=$2
    Log "Begin to replace oracle and oracle base "
    sed "s#${oldHome}#${IN_ORACLE_HOME}#g" ${fileName} > ${fileName}.bak
    [ $? -eq 0 ] || ExitWithErrorCode "replace pfile oracle home path failed" $ERROR_BACKUP_INVALID
    mv ${fileName}.bak ${fileName}
    [ $? -eq 0 ] || ExitWithErrorCode "replace pfile oracle home path failed" $ERROR_BACKUP_INVALID

    # replace oracle base path
    sed "s#${oldBase}#${IN_ORACLE_BASE}#g" ${fileName} > ${fileName}.bak
    [ $? -eq 0 ] || ExitWithErrorCode "replace pfile oracle home path failed" $ERROR_BACKUP_INVALID
    mv ${fileName}.bak ${fileName}
    [ $? -eq 0 ] || ExitWithErrorCode "replace pfile oracle base path failed" $ERROR_BACKUP_INVALID
}

ReplaceFileByNewEnv()
{
    # 1. pfile              PFILE_NAME
    # 2. environment file   ENV_FILE
    # 3.data file list      DATA_FILES
    # 4.ctl file list       CTL_FILES
    # 5.log file list       LOG_FILES
    # 6.temp file list      TEMP_FILES
    # 7.oracle_base         IN_ORACLE_BASE
    # 8.oracle_home         IN_ORACLE_HOME
    Log "Begin to replace oracle home and oracle base"
    [ ! -f "${ENV_FILE}" ] && ExitWithErrorCode "no oracle environment file exists" $ERROR_BACKUP_INVALID
    oraBasePath=`grep "ORACLE_BASE=" ${ENV_FILE} | sed -n "1p" | $MYAWK -F= '{print $2}'`
    oraHomePath=`grep "ORACLE_HOME=" ${ENV_FILE} | sed -n "1p" | $MYAWK -F= '{print $2}'`
    [ -z "${oraBasePath}" ] && ExitWithErrorCode "old oracle base path" $ERROR_BACKUP_INVALID
    [ -z "${oraHomePath}" ] && ExitWithErrorCode "old oracle home path" $ERROR_BACKUP_INVALID

    ReplaceBaseAndHome ${oraBasePath} ${PFILE_NAME} ${oraHomePath}
    ReplaceBaseAndHome ${oraBasePath} ${DATA_FILES} ${oraHomePath}
    ReplaceBaseAndHome ${oraBasePath} ${CTL_FILES} ${oraHomePath}
    ReplaceBaseAndHome ${oraBasePath} ${LOG_FILES} ${oraHomePath}
}

CheckDBExists()
{
    # check instance running
    CheckDBClose

    # need to check $ORACLE_BASE/admin
    grep "^${DBINSTANCE}:" /etc/oratab
    if [ $? -eq 0 ]; then
        Log "instance ${DBINSTANCE} is already exists."
        return 0
    fi
    return 0
}

CheckDBClose()
{
    dbNum=`ps -ef | grep "ora_...._${DBINSTANCE}$" | grep -v grep | wc -l`
    if [ "${dbNum}" -ge "1" ]; then
        GetOracleInstanceStatus ${DBINSTANCE}
        RET=$?
        if [ $RET -eq 0 ]; then
            Log "instance ${DBINSTANCE} is running."
            return 1
        fi
    fi

    Log "instance ${DBINSTANCE} is not running."
    return 0
}

DoTrunCateLog() 
{
    DB_SID=$1
    TRUNCATE_LOG_SCN=$2

    # get archive_dest list
    ARCHIVE_DEST_LIST="${STMP_PATH}/ArchiveDestList${PID}.txt"
    DeleteFile "${ARCHIVE_DEST_LIST}"
    touch "${ARCHIVE_DEST_LIST}"
    GetArchiveDestLst ${ARCHIVE_DEST_LIST}

    TrunCateLogSQL="${STMP_PATH}/TrunCateLog${PID}.sql"
    TrunCateLogRST="${STMP_PATH}/TrunCateLogRST${PID}.txt"
    for archDest in `cat ${ARCHIVE_DEST_LIST}`; do
        echo "delete force noprompt archivelog until scn $TRUNCATE_LOG_SCN like '${archDest}/%';" >> "$TrunCateLogSQL"
    done
    echo "exit;" >> "$TrunCateLogSQL"
    Log "Exec RMAN to delete archivelog of database."
    RmanExeScript "${DBUSER}" "${DBUSERPWD}" ${TrunCateLogSQL} ${TrunCateLogRST} ${DB_SID} -1 ${ORA_DB_USER} >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ ${RET_CODE} -ne 0 ]
    then
        Log "Delete archivelog of database-${DB_SID} failed, msg=`cat ${TrunCateLogRST}`."
        DeleteFile ${TrunCateLogSQL} ${TrunCateLogRST} ${ARCHIVE_DEST_LIST}
        if [ "${ERROR_SCRIPT_EXEC_FAILED}" = "${RET_CODE}" ] 
        then
            exit ${ERROR_ORACLE_TRUNCATE_ARCHIVELOG_FAILED}
        else
            exit ${RET_CODE}
        fi
    else
        Log "Delete archivelog of database-${DB_SID} succ."
        DeleteFile ${TrunCateLogSQL} ${TrunCateLogRST} ${ARCHIVE_DEST_LIST}
    fi
}

# replace control file list in pfile using backup path
ModifyPfile()
{
    OMFEnable=0
    dstFilePath=$1
    omfDest=`grep "db_create_file_dest" ${PFILE_NAME} | $MYAWK -F "=" '{print $2}'`
    omfDest=${omfDest#*\'}
    omfDest=${omfDest%*\'}
    if [ ! -z $omfDest ]; then
        OMFEnable=1
    fi
    UpperDBNAME=$(echo ${DBNAME} | tr '[a-z]' '[A-Z]')
    if [ -z "${dstFilePath}" ]; then
        if [ $OMFEnable -eq 1 ]; then
            dstFilePath=$omfDest
        fi
        local ctrlFile=`$MYAWK 'BEGIN{FS="/"; OFS="/"}{$NF=""; print}' ${CTL_FILES} | sed -n "1p"`
        ctrlFile="\'${ctrlFile%*/}/controlfile.ctl\'"
    else
        if [ $taskType -eq 1 ]; then
            ctrlFile="\'${dstFilePath}/controlfile.ctl\'"
        else
            if [ $OMFEnable -eq 1 ]; then 
                CreateDir "${dstFilePath}/${UpperDBNAME}/controlfile/"
                ctrlFile="\'${dstFilePath}/${UpperDBNAME}/controlfile/controlfile.ctl\'"
            else
                ctrlFile="\'${dstFilePath}/controlfile.ctl\'"
            fi
        fi
    fi
    if [ -z "$dstFilePath" ];then
        dstFilePath=`echo ${ctrlFile%/*/*/*}`
    fi
    # Modify *.control_files
    Log "get ctrFile list is ${ctrlFile}."
    sed "s#.*control_files=.*#\*.control_files=${ctrlFile}#g" ${PFILE_NAME} > ${PFILE_NAME}.bak 
    mv ${PFILE_NAME}.bak ${PFILE_NAME}
    [ $? -eq 0 ] || ExitWithErrorCode "replace *.control_files in pfile failed" $ERROR_BACKUP_INVALID
    # Modify *.db_create_file_dest
    if [ ! -z ${omfDest} ]; then
        if [ ${taskType} = 1 ]; then
            sed "s#.*db_create_file_dest=.*#\*.db_create_file_dest='${dstFilePath}'#g" ${PFILE_NAME} > ${PFILE_NAME}.bak
            mv ${PFILE_NAME}.bak ${PFILE_NAME}
        else
            if [ "`RDsubstr ${dstFilePath} 1 1`" = "/" ]; then
                CreateDir ${dstFilePath}
                sed "s#.*db_create_file_dest=.*#\*.db_create_file_dest='${dstFilePath}'#g" ${PFILE_NAME} > ${PFILE_NAME}.bak
                mv ${PFILE_NAME}.bak ${PFILE_NAME}
            elif [ "`RDsubstr ${dstFilePath} 1 1`" = "+" ]; then
                dgName=`RDsubstr ${omfDest} 2`
                echo lsdg ${dgName} | su - ${ORA_GRID_USER} ${GRID_SHELLTYPE} -c "${EXPORT_GRID_ENV}export ORACLE_SID=${ASMSIDNAME}; asmcmd" | grep "${dgName}"
                if [ $? -ne 0 ]; then
                    sed "s#.*db_create_file_dest=.*#\*.db_create_file_dest='${dstFilePath%%/*}'#g" ${PFILE_NAME} > ${PFILE_NAME}.bak
                    mv ${PFILE_NAME}.bak ${PFILE_NAME}
                fi
            fi
        fi
    fi
    # Modify *.log_archive_dest_n
    archiveDestList=`grep "log_archive_dest" ${PFILE_NAME}`
    for archiveDest in ${archiveDestList}; do
        log_archive_dest=`echo ${archiveDest} | $MYAWK -F "=" '{print $1}'`
        log_archive_dest=${log_archive_dest#*.}
        archiveDest=`echo ${archiveDest} | $MYAWK -F "${log_archive_dest}=" '{print $2}'`
        archiveDest=${archiveDest#*\'}
        archiveDest=${archiveDest%*\'}
        archiveDest=`echo ${archiveDest} | $MYAWK -F "=" '{print $2}'`
        if [ ! -z ${archiveDest} ]; then
            if [ ${taskType} = 1 ]; then
                sed "s#.*${log_archive_dest}=.*#\*.${log_archive_dest}='LOCATION=${dstFilePath}'#g" ${PFILE_NAME} > ${PFILE_NAME}.bak
                mv ${PFILE_NAME}.bak ${PFILE_NAME}
            else
                if [ "`RDsubstr ${dstFilePath} 1 1`" = "/" ]; then
                    CreateDir ${dstFilePath}
                    sed "s#.*${log_archive_dest}=.*#\*.${log_archive_dest}='LOCATION=${dstFilePath}'#g" ${PFILE_NAME} > ${PFILE_NAME}.bak
                    mv ${PFILE_NAME}.bak ${PFILE_NAME}
                elif [ "`RDsubstr ${dstFilePath} 1 1`" = "+" ]; then
                    dgName=`RDsubstr ${archiveDest} 2`
                    echo lsdg ${dgName} | su - ${ORA_GRID_USER} ${GRID_SHELLTYPE} -c "${EXPORT_GRID_ENV}export ORACLE_SID=${ASMSIDNAME}; asmcmd" | grep "${dgName}"
                    if [ $? -ne 0 ] ; then
                        sed "s#.*${log_archive_dest}=.*#\*.${log_archive_dest}='LOCATION=${dstFilePath%%/*}'#g" ${PFILE_NAME} > ${PFILE_NAME}.bak
                        mv ${PFILE_NAME}.bak ${PFILE_NAME}
                    fi
                fi
            fi
        fi
    done
    # Modify *.db_recovery_file_dest
    fastRecoverDest=`grep "db_recovery_file_dest=" ${PFILE_NAME} | $MYAWK -F "=" '{print $2}'`
    fastRecoverDest=${fastRecoverDest#*\'}
    fastRecoverDest=${fastRecoverDest%*\'}
    if [ ! -z ${fastRecoverDest} ]; then
        if [ "`RDsubstr ${dstFilePath} 1 1`" = "/" ]; then
            CreateDir ${dstFilePath}
            sed "s#.*db_recovery_file_dest=.*#\*.db_recovery_file_dest='${dstFilePath}'#g" ${PFILE_NAME} > ${PFILE_NAME}.bak 
            mv ${PFILE_NAME}.bak ${PFILE_NAME}
        elif [ "`RDsubstr ${dstFilePath} 1 1`" = "+" ]; then
            dgName=`RDsubstr ${fastRecoverDest} 2`
            echo lsdg ${dgName} | su - ${ORA_GRID_USER} ${GRID_SHELLTYPE} -c "${EXPORT_GRID_ENV}export ORACLE_SID=${ASMSIDNAME}; asmcmd" | grep "${dgName}"
            if [ $? -ne 0 ]; then
                sed "s#.*db_recovery_file_dest=.*#\*.db_recovery_file_dest='${dstFilePath%%/*}'#g" ${PFILE_NAME} > ${PFILE_NAME}.bak
                mv ${PFILE_NAME}.bak ${PFILE_NAME}
            fi
        fi
    fi

    # RAC to Single-Instance
    if [ 0 -eq ${DBISCLUSTER} ] || [ 2 -eq ${DBISCLUSTER} ]; then
        sed "s#.*cluster_database.*#\*.cluster_database=FALSE#g" ${PFILE_NAME} > ${PFILE_NAME}.bak
        mv ${PFILE_NAME}.bak ${PFILE_NAME}
    fi
    chown -h ${ORA_DB_USER}:${ORA_DB_GROUP} ${PFILE_NAME}
}

CreateSpfile()
{
    SpfileLocation=$1
    if [ -z "${SpfileLocation}" ]; then
        local tmpStr=`cat $ADDITIONAL/spfile`
        SpfileLocation=`echo ${tmpStr%/*}`
        [ -z "${SpfileLocation}" ] && "Spfile path"
        CreateDir ${SpfileLocation}
    fi
    
    Log "Begin to create spfile to new location ${SpfileLocation}."

    if [ "${SpfileLocation}/ebackup-spfile${LowerCaseDBNAME}.ora" = "${PFILE_NAME}" ]; then
        Log "Location is the same ${PFILE_NAME}."
        return 0
    fi
    
    local LowerCaseDBNAME=`echo ${DBNAME} | tr 'A-Z' 'a-z'`
    CreateSpfileSQL="${STMP_PATH}/CreateSpfile${PID}.sql"
    CreateSpfileRST="${STMP_PATH}/CreateSpfileRST${PID}.txt"
    echo "STARTUP NOMOUNT PFILE='${PFILE_NAME}';" > ${CreateSpfileSQL}
    echo "CREATE SPFILE='${SpfileLocation}/ebackup-spfile${LowerCaseDBNAME}.ora' FROM PFILE='${PFILE_NAME}';" >> ${CreateSpfileSQL}
    echo "SHUTDOWN ABORT;" >> ${CreateSpfileSQL}
    echo "exit;" >> ${CreateSpfileSQL}

    OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${CreateSpfileSQL}" "${CreateSpfileRST}" "${DBINSTANCE}" ${MAX_OPDB_TIME} 0 "${ORA_DB_USER}" >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ "$RET_CODE" -ne "0" ]; then
        cat ${CreateSpfileRST} >> ${LOG_FILE_NAME}
        Log "Create spfile failed, ret="$RET_CODE", msg `cat ${CreateSpfileRST}`."
        DeleteFile "${CreateSpfileSQL}" "${CreateSpfileRST}"
        exit ${RET_CODE}
    fi
    DeleteFile "${CreateSpfileSQL}" "${CreateSpfileRST}"
    Log "Create spfile succ."
}

DeleteExpiredArchivelog()
{
    echo "crosscheck archivelog all;" > "${RmanCMD}"
    echo "delete noprompt expired archivelog all;" >> "${RmanCMD}"
    echo "delete noprompt archivelog like '${ARCHIVE}/%';" >> "${RmanCMD}"
    db_mount_pt="${MainBackupPath%/*}"
    echo "delete noprompt archivelog like '${db_mount_pt%/*}/%';" >> "${RmanCMD}"
    echo "exit;" >> "${RmanCMD}"
    RmanExeScript "${DBUSER}" "${DBUSERPWD}" ${RmanCMD} ${RmanRST} ${DBINSTANCE} -1 ${ORA_DB_USER} >> "${LOG_FILE_NAME}" 2>&1
    RMAN_RET_CODE=$?
    DeleteFile ${RmanCMD} ${RmanRST}
    if [ ${RMAN_RET_CODE} -ne 0 ]; then
        Log "ERROR:delete expired archivelog failed."
    else
        Log "INFO:delete expired archivelog succ."
    fi
}

ExecDBRestore()
{
    taskType=$1
    local ISEncBK=$2
    local incraNum=$3
    local copyRestore=$4

    Log "Begin to exec DB restore taskType $taskType to $PIT, ISEncBK is ${ISEncBK}."
    RECOVERY_FILE="${STMP_PATH}/recoverOracle${PID}.sql"
    RECOVERY_FILE_RST="${STMP_PATH}/recoverOracleRst${PID}.txt"
    DeleteFile ${RECOVERY_FILE}
    touch ${RECOVERY_FILE}

    BuildDBRestoreRmanScript
    # exec rman script
    Log "Begin to restore database."
    RmanExeScript "${DBUSER}" "${DBUSERPWD}" ${RECOVERY_FILE} ${RECOVERY_FILE_RST} ${DBINSTANCE} -1 ${ORA_DB_USER} 1 >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ ${RET_CODE} -ne 0 ];then
        Log "recover database-${DBINSTANCE} failed, error=${RET_CODE}."
        DeleteFile ${RECOVERY_FILE} ${RECOVERY_FILE_RST}
        exit ${RET_CODE}
    else
        Log "recover database-${DBINSTANCE} succ."
        DeleteFile ${RECOVERY_FILE} ${RECOVERY_FILE_RST}
    fi
    # disable CBT
        # modify cluster property
        if [ ${UPGRADE} -eq 1 ] && [ ${DBISCLUSTER} -eq 1 ]; then
            ModifyClusterProperty FALSE
            StartUpOracleMount
        fi

        echo "select status from v\$block_change_tracking;" > ${RECOVERY_FILE}
        echo "exit;" >> ${RECOVERY_FILE}
        OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${RECOVERY_FILE}" "${RECOVERY_FILE_RST}" "${DBINSTANCE}" 30 1 "${ORA_DB_USER}" >> "${LOG_FILE_NAME}" 2>&1
        RET_CODE=$?
        BCT_STATUS=
        if [ $RET_CODE -ne 0 ]; then
            Log "query cbt status failed, ret="$RET_CODE"."
        else
            BCT_STATUS="`cat ${RECOVERY_FILE_RST}`"
        fi
        DeleteFile ${RECOVERY_FILE} ${RECOVERY_FILE_RST}

        if [ "${BCT_STATUS}" != "DISABLED" ]; then
            echo "alter database disable BLOCK CHANGE TRACKING;" > ${RECOVERY_FILE}
        fi
        if [ ${UPGRADE} -eq 1 ]; then
            echo "ALTER DATABASE OPEN RESETLOGS UPGRADE;" >> ${RECOVERY_FILE}
        else
            echo "ALTER DATABASE OPEN RESETLOGS;" >> ${RECOVERY_FILE}
        fi
        Log "Begin to open database resetlogs, BCT_STATUS=$BCT_STATUS."
        OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${RECOVERY_FILE}" "${RECOVERY_FILE_RST}" "${DBINSTANCE}" -1 0 "${ORA_DB_USER}" >> "${LOG_FILE_NAME}" 2>&1
        RET_CODE=$?
        if [ ${RET_CODE} -ne 0 ];then
            Log "open resetlogs database-${DBINSTANCE} failed, error=${RET_CODE}."
            DeleteFile ${RECOVERY_FILE} ${RECOVERY_FILE_RST}
            exit ${RET_CODE}
        else
            Log "open resetlogs database-${DBINSTANCE} succ."
            DeleteFile ${RECOVERY_FILE} ${RECOVERY_FILE_RST}
        fi

        DeleteExpiredArchivelog

    DeleteFile ${PFILE_NAME}
    Log "exec DB-${DBNAME} restore success"
}


ExecDBRestore11()
{
    taskType=$1
    local ISEncBK=$2
    local incraNum=$3
    local copyRestore=$4

    RECOVERY_FILE="${STMP_PATH}/recoverOracle${PID}.sql"
    RECOVERY_FILE_RST="${STMP_PATH}/recoverOracleRst${PID}.txt"
    DeleteFile ${RECOVERY_FILE}
    touch ${RECOVERY_FILE}

    if [ $taskType = 1 -o $taskType = 2 ] || [ $taskType = 0 -a ! -z "${RECOVERPATH}" ]; then
        Log "INFO:Running SqlPlus to Rename File"
        echo "    startup mount" >> ${RECOVERY_FILE}
        RenameLogfile
        echo "    shutdown abort" >> ${RECOVERY_FILE}
        # exec SqlPlus script
        OracleExeSql  "${DBUSER}" "${DBUSERPWD}" "${RECOVERY_FILE}" "${RECOVERY_FILE_RST}" "${DBINSTANCE}" 30 0 "${ORA_DB_USER}" >> "${LOG_FILE_NAME}" 2>&1
        Sqlcode=$?
        if [ ${Sqlcode} -ne 0 ]; then
            cat ${RECOVERY_FILE_RST} >> ${LOG_FILE_NAME}
            Log "Rename database-${DBINSTANCE} failed, sqlpluserror=${Sqlcode}."
            DeleteFile ${RECOVERY_FILE} ${RECOVERY_FILE_RST}
            exit ${Sqlcode}
        fi
    fi
    DeleteFile ${RECOVERY_FILE} ${RECOVERY_FILE_RST}
    Log "Begin to restore database."
    BuildDBRestoreRmanScript
    # exec rman script
    RmanExeScript "${DBUSER}" "${DBUSERPWD}" ${RECOVERY_FILE} ${RECOVERY_FILE_RST} ${DBINSTANCE} -1 ${ORA_DB_USER} 1 >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ ${RET_CODE} -ne 0 ]; then
        cat ${RECOVERY_FILE_RST} >> ${LOG_FILE_NAME}
        Log "restore database-${DBINSTANCE} failed, error=${RET_CODE}."
        DeleteFile ${RECOVERY_FILE} ${RECOVERY_FILE_RST}
        exit ${RET_CODE}
    else
        Log "restore database-${DBINSTANCE} succ."
    fi

    # disable BCT
        if [ ${UPGRADE} -eq 1 ] && [ ${DBISCLUSTER} -eq 1 ]; then
            ModifyClusterProperty FALSE
            StartUpOracleMount
        fi
        DeleteFile ${RECOVERY_FILE} ${RECOVERY_FILE_RST}
        Log "Begin to query bct status ."
        echo "select status from v\$block_change_tracking;" > ${RECOVERY_FILE}
        echo "exit;" >> ${RECOVERY_FILE}
        # exec SqlPlus script
        OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${RECOVERY_FILE}" "${RECOVERY_FILE_RST}" "${DBINSTANCE}" 30 1 "${ORA_DB_USER}" >> "${LOG_FILE_NAME}" 2>&1
        RET_CODE=$?
        BCT_STATUS=
        if [ "$RET_CODE" -ne "0" ]; then
            Log "query bct status failed, ret="$RET_CODE"."
        else
            BCT_STATUS="`cat ${RECOVERY_FILE_RST}`"
        fi
    
        # open resetlogs
        Log "Begin to open database resetlogs, BCT_STATUS=$BCT_STATUS."
        DeleteFile ${RECOVERY_FILE} ${RECOVERY_FILE_RST}
        if [ "${BCT_STATUS}" != "DISABLED" ]; then
            echo "alter database disable BLOCK CHANGE TRACKING;" > ${RECOVERY_FILE}
        fi
        if [ ${UPGRADE} -eq 1 ]; then
            echo "ALTER DATABASE OPEN RESETLOGS UPGRADE;" >> ${RECOVERY_FILE}
        else
            echo "ALTER DATABASE OPEN RESETLOGS;" >> ${RECOVERY_FILE}
        fi
        # exec SqlPlus script
        OracleExeSql  "${DBUSER}" "${DBUSERPWD}" "${RECOVERY_FILE}" "${RECOVERY_FILE_RST}" "${DBINSTANCE}" -1 0 "${ORA_DB_USER}" >> "${LOG_FILE_NAME}" 2>&1
        Sqlcode=$?
        if [ ${Sqlcode} -ne 0 ]; then
            cat ${RECOVERY_FILE_RST} >> ${LOG_FILE_NAME}
            Log "alter database resetlogs -${DBINSTANCE} failed, sqlpluserror=${Sqlcode}."
            DeleteFile ${RECOVERY_FILE} ${RECOVERY_FILE_RST}
            exit ${Sqlcode}
        fi
        
        DeleteExpiredArchivelog

    DeleteFile ${RECOVERY_FILE} ${RECOVERY_FILE_RST}
    Log "exec DB-${DBNAME} restore success"
}

BuildDBRestoreRmanScript()
{
    local channelName=
    if [ $taskType = 0 ]; then
        channelName="eRestore"
    elif [ $taskType = 1 ]; then 
        PIT="scn `cat ${ADDITIONAL}/scn_dbf_max | ${MYAWK} '{print $1}'`"
        channelName="eLivemount"
    elif [ $taskType = 2 ]; then
        channelName="eInsrestore"
    fi
    Log "Begin to exec DB restore taskType ${channelName} to ${PIT}, ISEncBK is ${ISEncBK}."
    TEMPDBID=`cat ${ADDITIONAL}/dbinfo | $MYAWK -F ";" '{print $1}'`
    if [ ${ISEncBK} -eq 1 ]; then
        RMAN_ENC_SECTION="set dbid=$TEMPDBID; startup mount; $RMAN_ENC_SECTION "
    else 
        echo "set dbid=$TEMPDBID" >> ${RECOVERY_FILE}
        echo "startup mount;" >> ${RECOVERY_FILE}
    fi
    echo "RESET DATABASE TO INCARNATION ${incraNum};" >> ${RECOVERY_FILE}
    echo "delete noprompt expired datafilecopy all;" >> ${RECOVERY_FILE}
    echo "delete noprompt expired backupset;" >> ${RECOVERY_FILE}
    echo "RUN" >> ${RECOVERY_FILE}
    echo "{" >> ${RECOVERY_FILE}
    echo "    SET COMMAND ID TO 'ProtectAgent_Restore';" >> ${RECOVERY_FILE}
    i=1
    while [ $i -le $CHANNELS ]; do
        echo "    allocate channel ${channelName}`printf "%02d" $i` type disk;" >> ${RECOVERY_FILE}
        i=`expr $i + 1`
    done
    Catalog
    NewnameDatafile
    if [ `RDsubstr ${ORA_VERSION} 1 2` -gt 11 ] && [ ! -z "${RECOVERPATH}" ]; then
        RenameLogfile
    fi
    echo "    SET UNTIL ${PIT};" >> ${RECOVERY_FILE}
    echo "    RESTORE DATABASE;" >> ${RECOVERY_FILE}
    RenameDatafile
    echo "    RECOVER DATABASE;" >> ${RECOVERY_FILE}
    i=1
    while [ $i -le $CHANNELS ]; do
        echo "    release channel ${channelName}`printf "%02d" $i`;" >> ${RECOVERY_FILE}
        i=`expr $i + 1`
    done
    echo "}" >> ${RECOVERY_FILE}
}

Catalog()
{
    if [ ${COPY_RESTORE} -eq 1 ]; then
        echo "    catalog start with '$MainBackupPath/log' noprompt;" >> ${RECOVERY_FILE}
    else
        echo "    catalog start with '${ARCHIVE}/resetlogs_id${resetlogs_id}' noprompt;" >> ${RECOVERY_FILE}
    fi
    local pathNum=`echo ${BACKUP} | sed 's/;/ /g' | wc -w`
    local index_datafile=0
    local index_path=
    local line=
    for line in `cat $ADDITIONAL/dbfiles`; do
        index_path=`expr ${index_datafile} % ${pathNum} + 1`
        index_datafile=`expr ${index_datafile} + 1`
        local fileID=`echo ${line} | ${MYAWK} -F ";" '{print $3}'`
        local path=`echo ${BACKUP} | cut -d ";" -f${index_path}`
        echo "    catalog start with '${path}/${fileID}' noprompt;" >> ${RECOVERY_FILE}
    done
}

NewnameDatafile()
{   
    local UpperDBNAME=$(echo ${DBNAME} | tr '[a-z]' '[A-Z]')
    local targetPath=
    if [ ${taskType} -eq 0 ] && [ ! -z "${RECOVERPATH}" ]; then
        targetPath=${RECOVERPATH}
    fi
    if [ ${taskType} -eq 1 ] || [ ${taskType} -eq 2 ]; then
        targetPath=`echo ${BACKUP} | $MYAWK -F ";" '{print $1}'`
    fi
    if [ -z "${targetPath}" ]; then
        return
    fi
    local line=
    for line in `cat $ADDITIONAL/dbfiles`; do
        local tablespace_name=`echo ${line} | ${MYAWK} -F ";" '{print $2}'`
        local fileID=`echo ${line} | ${MYAWK} -F ";" '{print $3}'`
        if [ ${taskType} -eq 0 ] && [ ! -z "${RECOVERPATH}" ]; then
            if [ $OMFEnable -eq 0 ]; then 
                echo "    SET NEWNAME FOR DATAFILE ${fileID} to '${targetPath}/FNO-${fileID}_TS-${tablespace_name}.dbf';" >> ${RECOVERY_FILE}
            else
                CreateDir "${targetPath}/${UpperDBNAME}/datafile"
                echo "    SET NEWNAME FOR DATAFILE ${fileID} to '${targetPath}/${UpperDBNAME}/datafile/FNO-${fileID}_TS-${tablespace_name}.dbf';" >> ${RECOVERY_FILE}
            fi
        else
            echo "    SET NEWNAME FOR DATAFILE ${fileID} to '${targetPath}/${fileID}/FNO-${fileID}_TS-${tablespace_name}.dbf';" >> ${RECOVERY_FILE}
        fi
    done
}

RenameLogfile()
{
    local line=
    local TimeStamp=`date +%s`
    local UpperDBNAME=$(echo ${DBNAME} | tr '[a-z]' '[A-Z]')
    if [ $OMFEnable -eq 1 ]; then
        local temppath="${RECOVERPATH}/${UpperDBNAME}"
    fi
    for line in `cat $ADDITIONAL/logfiles`; do
        local srcPath=`echo ${line} | $MYAWK -F ";" '{print $2}'`
        local logname=`echo $srcPath | $MYAWK -F "/" '{print $NF}'`
        if [ $OMFEnable -eq 1 ];then
            CreateDir "$temppath/onlinelog"
            local dstPath="${temppath}/onlinelog/${logname}"
        else
            local dstPath="${RECOVERPATH}/${logname}"
        fi
        [ ${srcPath} = ${dstPath} ]  && continue
            
        grep "${srcPath}" ${RECOVERY_FILE}
        if [ $? -ne 0 ]; then
            echo "    ALTER DATABASE RENAME FILE '${srcPath}' to '${dstPath}-${TimeStamp}';" >> ${RECOVERY_FILE}
        fi
    done
}

RenameDatafile()
{
    local targetPath=
    if [ ${taskType} -eq 0 ] && [ ! -z "${RECOVERPATH}" ]; then
        targetPath="${RECOVERPATH}"
    fi
    if [ ${taskType} -eq 1 ] || [ ${taskType} -eq 2 ]; then
        targetPath=`echo ${BACKUP} | $MYAWK -F ";" '{print $1}'`
    fi
    if [ -z "${targetPath}" ]; then
        return
    fi
    local line=
    local UpperDBNAME=$(echo ${DBNAME} | tr '[a-z]' '[A-Z]')
    if [ `RDsubstr ${ORA_VERSION} 1 2` -gt 11 ]; then
        for line in `cat $ADDITIONAL/dbfiles`; do
            local tablespace_name=`echo ${line} | ${MYAWK} -F ";" '{print $2}'`
            local fileID=`echo ${line} | ${MYAWK} -F ";" '{print $3}'`
            local srcPath=`echo ${line} | $MYAWK -F ";" '{print $4}'`
            if [ ${taskType} -eq 0 ] && [ ! -z "${RECOVERPATH}" ]; then
                if [ $OMFEnable -eq 0 ]; then 
                    targetName="${targetPath}/FNO-${fileID}_TS-${tablespace_name}.dbf"
                else
                    targetName="${targetPath}/${UpperDBNAME}/datafile/FNO-${fileID}_TS-${tablespace_name}.dbf"
                fi
            else
                targetName="${targetPath}/${fileID}/FNO-${fileID}_TS-${tablespace_name}.dbf"
            fi
            if [ "${srcPath}" != "${targetName}" ]; then
                echo "    ALTER DATABASE RENAME FILE '${srcPath}' to '${targetName}';" >> ${RECOVERY_FILE}
            fi
        done
    else
        echo "    SWITCH DATAFILE ALL;" >> ${RECOVERY_FILE}
    fi
    
}

CheckParams4Restore()
{
    # check parameter valid
    test -z "$BACKUP"      && ExitWithError "data path"
    if [ $COPY_RESTORE -ne 1 ]; then
        test -z "$ARCHIVE"     && ExitWithError "log path"
    fi
    test -z "$DBINSTANCE"  && ExitWithError "oracle instance name"
    test -z "$RECOVERORDER" && ExitWithError "RECOVERORDER"
    
    if [ -z "$CHANNELS" ] || [ $CHANNELS = 0 ]; then
        CHANNELS=4
        Log "Setting channels number to $CHANNELS by default"
    fi
    
    # default startDB
    if [[ "$STARTDB" != "0" ]] && [[  "$STARTDB" != "1"  ]]; then 
        STARTDB=1
        Log "Setting startDB to $STARTDB by default"
    fi
    
    # check backup directory, need to support asm diskgroup
    local dataPaths=`echo $BACKUP | sed 's/;/ /g'`
    for dataPath in ${dataPaths}; do
        [ -z "$dataPath" ] && continue
        CheckDirRWX "$ORA_DB_USER" "$dataPath"
        if [ $? -ne 0 ]; then
            chown -h -R ${ORA_DB_USER}:${ORA_DB_GROUP} "$dataPath"
        fi
    done

    if [ $COPY_RESTORE -ne 1 ]; then
        CheckDirRWX "$ORA_DB_USER" "$ARCHIVE"
        if [ $? -ne 0 ]; then
            chown -h -R ${ORA_DB_USER}:${ORA_DB_GROUP} "$ARCHIVE"
        fi
    fi
}

PreparePIT()
{
    if [ -z "$PIT_TIME" -a -z "$PIT_SCN" ]; then
        PIT_SCN=`cat $ADDITIONAL/scn_dbf_max | $MYAWK '{print $1}'`
    fi

    if [ -z "${ARCHIVE}" ]; then
        Log "ARCHIVE is null, restore by copy."
        COPY_RESTORE=1
    fi
    if [ ${RestoreBy} -eq 1 ]; then
        Log "full backup, restore by copy "
        COPY_RESTORE=1
    fi
    # convert timestamp to local time
    if [ ! -z "${PIT_TIME}" ]; then
        PITTIME=`UnixTimestampToDate ${PIT_TIME}`
        Log "convert ${PIT_TIME} to ${PITTIME}."
    fi
    
    if [ "$PIT_SCN" != "0" ]; then
        PIT="scn $PIT_SCN"
    else
        PIT='time "to_date'"('$PITTIME', 'YYYY-MM-DD HH24:MI:SS')"'"'
    fi
    Log "Get PIT is ${PIT}, COPY_RESTORE=${COPY_RESTORE}."
}

# create directories
CreateDBDirs()
{
  local fileName=$1
  Log "Creating directories for database files ($1)."
  CREATEDBDIIRS="${STMP_PATH}/CreateDBDir${PID}.txt"
  $MYAWK 'BEGIN{FS="/"; OFS="/"}{$NF=""; print}' $fileName | $MYAWK -F ";" '{print $NF}' | sort -u > $CREATEDBDIIRS
  for line in `cat ${CREATEDBDIIRS}`; do
    CreateDir $line
  done
  rm -f $CREATEDBDIIRS
}

# 0 represent the same instance, 1 represent different instance
IsSameInstance()
{
    DB_SID=$1
    local srcSID=`cat $ADDITIONAL/dbinfo | $MYAWK -F ";" '{print $3}'`
    if [ "$DB_SID" = "$srcSID" ]; then
        return 0
    else
        return 1
    fi
}

# check instance exist:1.instance not running 
CheckInsNotExist()
{
    CheckDBClose
    RET=$?
    return $RET
}

CheckRecoverPathExist()
{
    if [ "`RDsubstr ${RECOVERPATH} 1 1`" = "/" ]; then
        ls -l $RECOVERPATH >> "${LOG_FILE_NAME}" 2>&1
        [ $? -ne 0 ] && ExitWithErrorCode "recover path not exist" $ERROR_ORACLE_RECOVERPATH_NOT_EXIT

        chown -h ${ORA_DB_USER}:${ORA_DB_GROUP} $RECOVERPATH >> "${LOG_FILE_NAME}" 2>&1
        CHMOD 750 $RECOVERPATH >> "${LOG_FILE_NAME}" 2>&1
    else
        su - ${ORA_GRID_USER} -c "${EXPORT_GRID_ENV}export ORACLE_SID=${ASMSIDNAME}; echo ls ${RECOVERPATH} | asmcmd" > ${RmanRST} 2>&1
        cat ${RmanRST} | grep "ASMCMD-"
        local ret=`echo $?`
        DeleteFile ${RmanRST}
        if [ ${ret} -eq 0 ]; then
            ExitWithErrorCode "asm recover path not exist" $ERROR_ORACLE_RECOVERPATH_NOT_EXIT
        fi
    fi
    Log "Check RecoverPath exist Succ." 
}

CheckRecoverPathNotProductDir()
{
    local productDirs=`$MYAWK 'BEGIN{FS="/"; OFS="/"}{$NF=""; print}' ${ADDITIONAL}/dbfiles | $MYAWK -F ";" '{print $NF}' | sort -u`
    for line in $productDirs
    do
        if ["$line" = "$RECOVERPATH/" ]; then
            Log "line is $line while recover path is $RECOVERPATH."
            ExitWithErrorCode "recover path have files" $ERROR_PARAM_INVALID
        fi
    done
    Log "Check RecoverPath not the same as product dir Succ."
}

ResetDB2ScnBefore()
{
    local SCN=$1
    local INCARNATION=
    Log "Begin to reset database ${DBNAME} to scn $SCN before."
    LIST_INCARNATION_FILE="${STMP_PATH}/listincarnation${PID}.sql"
    LIST_INCARNATION_FILE_RST="${STMP_PATH}/listincarnationRst${PID}.txt"
    echo "startup mount;" > $LIST_INCARNATION_FILE
    echo "list incarnation of database ${DBNAME};" >> $LIST_INCARNATION_FILE
    echo "exit;" >> $LIST_INCARNATION_FILE

    RmanExeScript "${DBUSER}" "${DBUSERPWD}" ${LIST_INCARNATION_FILE} ${LIST_INCARNATION_FILE_RST} ${DBINSTANCE} -1 ${ORA_DB_USER} >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ ${RET_CODE} -ne 0 ];then
        Log "list incarnation of database ${DBNAME} failed, error=${RET_CODE}, `cat ${LIST_INCARNATION_FILE_RST}`."
        DeleteFile ${LIST_INCARNATION_FILE} ${LIST_INCARNATION_FILE_RST}
        exit ${RET_CODE}
    fi
    Log "list incarnation of database ${DBNAME} succ."
    
    TMPLINE=`sed -n '/----------/,/^ *$/p' "${LIST_INCARNATION_FILE_RST}" | sed -e '/----------/d' -e '/^ *$/d' | $MYAWK '{print $1";"$5";"$6}'`
    DeleteFile ${LIST_INCARNATION_FILE} ${LIST_INCARNATION_FILE_RST}
    for DATAFILEPATH in ${TMPLINE}; do
        Log "Get incarnation info is ${DATAFILEPATH}."
        DBKey=`echo $DATAFILEPATH | $MYAWK -F ";" '{print $1}'`
        Status=`echo $DATAFILEPATH | $MYAWK -F ";" '{print $2}'`
        ReSetScn=`echo $DATAFILEPATH | $MYAWK -F ";" '{print $3}'`
        if [ $ReSetScn -gt $SCN ] && [ "${Status}" = "CURRENT" ]; then
            INCARNATION=`expr $DBKey - 1`
            Log "get INCARNATION=$INCARNATION."
            [ $INCARNATION -lt 1 ] && ExitWithError "INCARNATION is invaild, value is $INCARNATION"
            Log "Begin to reset database ${DBNAME} to INCARNATION $INCARNATION."
            RESETDB_FILE="${STMP_PATH}/resetDB${PID}.sql"
            RESETDB_FILE_RST="${STMP_PATH}/resetDBRst${PID}.txt"
            echo "reset database to incarnation $INCARNATION;" > $RESETDB_FILE
            echo "shutdown immediate;" >> $RESETDB_FILE
            echo "exit;" >> $RESETDB_FILE
            RmanExeScript "${DBUSER}" "${DBUSERPWD}" ${RESETDB_FILE} ${RESETDB_FILE_RST} ${DBINSTANCE} -1 ${ORA_DB_USER} >> "${LOG_FILE_NAME}" 2>&1
            RET_CODE=$?
            if [ ${RET_CODE} -ne 0 ]; then
                Log "reset database ${DBNAME} failed, error=${RET_CODE}, `cat ${RESETDB_FILE_RST}`."
                DeleteFile ${RESETDB_FILE} ${RESETDB_FILE_RST}
                exit ${RET_CODE}
            fi
            DeleteFile ${RESETDB_FILE} ${RESETDB_FILE_RST}
            Log "reset database ${DBNAME} to INCARNATION $INCARNATION succ."
            return 0
        fi
    done
    Log "no need to reset DB."
    ShutDownDB ${DBINSTANCE}
    return $?
}

# check database open status
CheckDBIsOpen()
{
    GetOracleInstanceStatus ${DBINSTANCE}
    RET=$?
    if [ $RET -ne 0 ]; then
        Log "Database restore successfully, but start database failed, ret=$RET."
        exit ${ERROR_RESTORE_SUCC_OPEN_FAILED}
    fi
    if [ "${INSTANCESTATUS}" = "OPEN" ]; then
        Log "Database restored and opened successfully"
        return 0
    else
        Log "Database restore successfully, but start database failed, status=$INSTANCESTATUS."
        exit ${ERROR_RESTORE_SUCC_OPEN_FAILED}
    fi
}

AddDBInfo2OraTab()
{
    grep ${DBNAME} /etc/oratab >> "${LOG_FILE_NAME}" 2>&1
    [ $? -ne 0 ] && echo $DBNAME":"$IN_ORACLE_HOME":N" >> /etc/oratab 
}

OpenPDBByName()
{
    local PdbName=$1
    OpenPDBSQL="${STMP_PATH}/OpenPDB${PID}.sql"
    OpenPDBRST="${STMP_PATH}/OpenPDBRST${PID}.txt"
    echo "alter pluggable database ${PdbName} open;" > ${OpenPDBSQL}
    echo "exit;" >> ${OpenPDBSQL}

    OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${OpenPDBSQL}" "${OpenPDBRST}" "${DBINSTANCE}" ${MAX_OPDB_TIME} 0 "${ORA_DB_USER}" >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ "$RET_CODE" -ne "0" ]; then
        Log "start PDB(${PdbName}) failed, ret="$RET_CODE", `cat ${OpenPDBRST}`."
        DeleteFile "${OpenPDBSQL}" "${OpenPDBRST}"
        exit ${RET_CODE}
    fi
    DeleteFile "${OpenPDBSQL}" "${OpenPDBRST}"
    Log "Open database(${PdbName}) succ."
}

OpenAllPDBs()
{
    OpenAllPDBSQL="${STMP_PATH}/OpenAllPDB${PID}.sql"
    OpenAllPDBRST="${STMP_PATH}/OpenAllPDBRST${PID}.txt"
    echo "alter pluggable database all open;" > ${OpenAllPDBSQL}
    echo "exit;" >> ${OpenAllPDBSQL}

    OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${OpenAllPDBSQL}" "${OpenAllPDBRST}" "${DBINSTANCE}" ${MAX_OPDB_TIME} 0 "${ORA_DB_USER}" >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ "$RET_CODE" -ne "0" ]; then
        Log "open PDB failed, ret="$RET_CODE"."
        DeleteFile "${OpenAllPDBSQL}" "${OpenAllPDBRST}"
        exit ${RET_CODE}
    fi
    DeleteFile "${OpenAllPDBSQL}" "${OpenAllPDBRST}"
    Log "Open PDB succ."
}

RestartDB()
{
    RestartDBSQL="${STMP_PATH}/RestartDB${PID}.sql"
    RestartDBRST="${STMP_PATH}/RestartDBRST${PID}.txt"
    echo "shutdown abort;" > ${RestartDBSQL}
    echo "startup;" >> ${RestartDBSQL}
    echo "exit;" >> ${RestartDBSQL}

    Log "Begin to restart DB."
    OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${RestartDBSQL}" "${RestartDBRST}" "${DBINSTANCE}" ${MAX_OPDB_TIME} 0 "${ORA_DB_USER}" >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ "$RET_CODE" -ne "0" ]; then
        Log "Restart DB failed, ret="$RET_CODE"."
        DeleteFile "${RestartDBSQL}" "${RestartDBRST}"
        exit ${RET_CODE}
    fi
    DeleteFile "${RestartDBSQL}" "${RestartDBRST}"
    Log "Restart DB succ."
}

StartDB()
{
    StartDBSQL="${STMP_PATH}/StartDB${PID}.sql"
    StartDBRST="${STMP_PATH}/DtartDBRST${PID}.txt"
    echo "startup;" > ${StartDBSQL}
    echo "exit;" >> ${StartDBSQL}
    
    Log "Begin to start DB."
    OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${StartDBSQL}" "${StartDBRST}" "${DBINSTANCE}" ${MAX_OPDB_TIME} 0 "${ORA_DB_USER}" >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ "$RET_CODE" -ne "0" ]; then
        Log "Start DB failed, ret="$RET_CODE", errmsg=`cat ${StartDBRST}`."
        DeleteFile "${StartDBSQL}" "${StartDBRST}"
        exit ${RET_CODE}
    fi
    DeleteFile "${StartDBSQL}" "${StartDBRST}"
    Log "Start DB succ."
}

CheckInstAuth()
{
    INST_NAME=$1
    CheckInstSQL="${STMP_PATH}/CheckInst${PID}.sql"
    CheckInstSQLRST="${STMP_PATH}/CheckInst${PID}.txt"
    echo "exit;" > ${CheckInstSQL}

    OracleExeSql "" "" "${CheckInstSQL}" "${CheckInstSQLRST}" "${INST_NAME}" 10 1 "${ORA_DB_USER}" >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    DeleteFile "${CheckInstSQL}" "${CheckInstSQLRST}"
    if [ "$RET_CODE" -ne "0" ]; then
        Log "Check ${INST_NAME} OS auth type failed."
        return 0
    fi
    Log "Check ${INST_NAME} OS auth type succ."
    return 1
}

CreateInitFile()
{
    SpfileLocation=$1
    if [ -z "${SpfileLocation}" ]; then
        local tmpStr=`cat $ADDITIONAL/spfile`
        SpfileLocation=`echo ${tmpStr%/*}`
        [ -z "${SpfileLocation}" ] && "Spfile path"
        CreateDir ${SpfileLocation}
    fi
    
    local LowerCaseDBNAME=`echo ${DBNAME} | tr 'A-Z' 'a-z'`
    Log "Begin to create oracle init file init${DBINSTANCE}.ora, SpfileLocation is ${SpfileLocation}."
    [ -z "$DBINSTANCE" ] && ExitWithError "db instance is empty"
    su - $ORA_DB_USER -c "${EXPORT_ORACLE_ENV}rm -f ${IN_ORACLE_HOME}/dbs/init${DBINSTANCE}.ora" >> ${LOG_FILE_NAME} 2>&1
    su - $ORA_DB_USER -c "${EXPORT_ORACLE_ENV}touch ${IN_ORACLE_HOME}/dbs/init${DBINSTANCE}.ora" >> ${LOG_FILE_NAME} 2>&1
    su - $ORA_DB_USER -c "${EXPORT_ORACLE_ENV}echo \"SPFILE="\'${SpfileLocation}/ebackup-spfile${LowerCaseDBNAME}.ora\'"\" > ${IN_ORACLE_HOME}/dbs/init${DBINSTANCE}.ora"
}

GetArchiveDestLst()
{
    Log "Begin check archive dest directory."
    ARCHIVEDESTSQL="${STMP_PATH}/ArchiveDestSQL${PID}.sql"
    ARCHIVEDESTRST="${STMP_PATH}/ArchiveDestRST${PID}.txt"
    echo "set linesize 300;" > "${ARCHIVEDESTSQL}"
    echo "col DESTINATION for a255;" >> "${ARCHIVEDESTSQL}"
    echo "select DESTINATION from v\$archive_dest where STATUS='VALID';" >> "${ARCHIVEDESTSQL}"
    echo "exit" >> "${ARCHIVEDESTSQL}"

    OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${ARCHIVEDESTSQL}" "${ARCHIVEDESTRST}" "${DBINSTANCE}" 30 0 >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ "$RET_CODE" -ne "0" ]; then
        Log "Get Archive log dest list failed, msg=`cat ${ARCHIVEDESTRST}`"
        DeleteFile "${ARCHIVEDESTSQL}" "${ARCHIVEDESTRST}"
        exit ${RET_CODE}
    fi
        
    TMPLINE=`sed -n '/----------/,/^ *$/p' "${ARCHIVEDESTRST}" | sed -e '/----------/d' -e '/^ *$/d'`
    DeleteFile "${ARCHIVEDESTSQL}" "${ARCHIVEDESTRST}"

    for line in ${TMPLINE}; do
        if [ "${line}" = "USE_DB_RECOVERY_FILE_DEST" ]; then
            echo "set linesize 300;" > "${ARCHIVEDESTSQL}"
            echo "col NAME for a255;" >> "${ARCHIVEDESTSQL}"
            echo "select NAME from V\$RECOVERY_FILE_DEST;" >> "${ARCHIVEDESTSQL}"
            echo "exit" >> "${ARCHIVEDESTSQL}"

            Log "Exec SQL to get name of archive dest."
            OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${ARCHIVEDESTSQL}" "${ARCHIVEDESTRST}" "${DBINSTANCE}" ${MAX_OPDB_TIME} 0 >> "${LOG_FILE_NAME}" 2>&1
            if [ "$RET_CODE" -ne "0" ]; then
                DeleteFile "${ARCHIVEDESTSQL}" "${ARCHIVEDESTRST}" "$1"
                Log "Get RECOVERY_FILE_DEST failed, msg=`cat ${ARCHIVEDESTRST}`."
                exit ${RET_CODE}
            fi
            STRARCHIVEDEST=`sed -n '/----------/,/^ *$/p' "${ARCHIVEDESTRST}" | sed -e '/----------/d' -e '/^ *$/d' | ${MYAWK} '{print $1}'`
            Log "STRARCHIVEDEST=${STRARCHIVEDEST}."
            echo ${STRARCHIVEDEST} >> $1
        elif [ ! -z "${line}" ]; then
            echo "${line}" >> $1
        else
            Log "get line is empty, continue."
        fi
    done
    DeleteFile "${ARCHIVEDESTSQL}" "${ARCHIVEDESTRST}"
    Log "Get Archive Dest Lst is `cat $1`."
}

CheckParamChannels()
{
    # set default params if not exist
    if [ -z "$CHANNELS" ] || [ "$CHANNELS" = "0" ]; then 
    export CHANNELS=4
    Log "Setting channels number to $CHANNELS by default"
    fi

    [ $CHANNELS -gt 254 ] && ExitWithError "channels value"
}

ModifySpfileName()
{
    local ORACLE_HOME=$1
    local ORACLE_INSTANCE=$2

    Log "Start rename spfile "

    local File=`find ${ORACLE_HOME}/dbs -name "spfile${ORACLE_INSTANCE}.ora"`
    local Date=`date "+%Y%m%d-%H%M%S"`
    local NewFile="${ORACLE_HOME}/dbs/spfile${ORACLE_INSTANCE}_${Date}.ora"
    if [ ! -z ${File} ]; then
        Log "Start rename spfile ${File} to ${NewFile}"
        res=`su - ${ORA_DB_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV}mv -f ${File} ${NewFile}"`
        if [ ${res} -ne 0 ]; then
            Log "Rname Spfile fail"
        fi
    else
        Log "No Find spfile."
    fi
}

GetMainDataPath()
{
    local dataPaths=`echo $BACKUP | sed 's/;/ /g'`
    for dataPath in ${dataPaths}; do
        MainBackupPath=${dataPath}
        break
    done
}

CreatePfilefromTempfile()
{
    if [ -z "${PFILEPID}" ]; then
        local oldPFile=`ls ${MainBackupPath}/ebackup-*-pfile.ora`
        [ ! -f "$oldPFile" ] && ExitWithErrorCode "old pfile is not exists." $ERROR_BACKUP_INVALID
        su - ${ORA_DB_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV}cp -d -f ${oldPFile} ${PFILE_NAME}"
        [ $? -eq 0 ] || ExitWithErrorCode "copy pfile from fs failed." $ERROR_BACKUP_INVALID
        Log "copy pfile from ${MainBackupPath}"
    else
        local temppfile=`find ${TMP_PATH} -name "pfile${PFILEPID}"`
        if [ -z "${temppfile}" ]; then 
            Log "temp pfile is not exit "
            exit $ERROR_BACKUP_INVALID
        fi
        Log "create pfile from temppfile pfile${PFILEPID}"
        su - ${ORA_DB_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV}rm -rf ${PFILE_NAME}"
        cp -d -f ${temppfile} ${PFILE_NAME} && chown -h ${ORA_DB_USER}:${ORA_DB_GROUP} ${PFILE_NAME}
        if [ $? -ne 0 ]; then
            Log "create pfile from temppfile fail "
            exit $ERROR_BACKUP_INVALID
        fi
        Log "create pfile from temppfile success "
        DeleteAgentUserFile $temppfile
    fi
}

GetRACNodeNum()
{
    local girdUserPath=`cat /etc/passwd | grep "${ORA_GRID_USER}" | $MYAWK -F":" '{print $6}'`
    local gridHome=`cat ${girdUserPath}/.bash_profile | grep "ORACLE_HOME=" | $MYAWK -F "=" '{print $NF}'`
    local nodeNum=`su - ${ORA_GRID_USER} ${GRID_SHELLTYPE} -c "${EXPORT_GRID_ENV}${gridHome}/bin/crsctl status server | grep NAME | wc -l"`
    Log "GetRACNodeNum=${nodeNum}"
    echo ${nodeNum}
}

SetRacPwdFile()
{
    if [ ${taskType} -eq 1 ]; then
        return
    fi
    dgName=${SpfileLocation%%/*}
    dstPwdPath="${dgName}/${DBNAME}/PASSWORD/pwd${DBNAME}"
    su - ${ORA_GRID_USER} -c "${EXPORT_GRID_ENV}export ORACLE_SID=${ASMSIDNAME}; echo rm -rf  ${dstPwdPath} | asmcmd" >> "${LOG_FILE_NAME}" 2>&1
    su - ${ORA_DB_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV}orapwd input_file='${DBPW_FILE}' file='${dstPwdPath}' dbuniquename='${DBNAME}'" >> "${LOG_FILE_NAME}" 2>&1
    su - ${ORA_DB_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV}srvctl modify database -db ${DBNAME} -pwfile '${dstPwdPath}'" >> "${LOG_FILE_NAME}" 2>&1
}

RigsterDBToRAC()
{
    if [ "$RECOVERORDER" = "1" ]; then
        if [ ! -f $ADDITIONAL/livemountOK ] || [ $taskType -eq 2 ]; then
            if [ $ISSTARTDB -ne 0 ]; then
                ShutDownDB ${DBINSTANCE}
            fi
        fi
        su - ${ORA_DB_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV}srvctl add database -d ${DBNAME} -o ${IN_ORACLE_HOME} -p ${SpfileLocation}/ebackup-spfile${DBNAME}.ora" >> "${LOG_FILE_NAME}" 2>&1
    fi
    su - ${ORA_DB_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV}srvctl add instance -d ${DBNAME} -i ${DBINSTANCE} -n `hostname`" >> "${LOG_FILE_NAME}" 2>&1
    if [ $RECOVERNUM -eq ${RECOVERORDER} ]; then
        SetRacPwdFile
        if [ ${AuthType} -eq 0 ] || [ $ISSTARTDB -eq 0 ];then
            return
        fi
        Log "srvctl start database -d ${DBNAME}"
        su - ${ORA_DB_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV}srvctl start database -d ${DBNAME}" >> "${LOG_FILE_NAME}" 2>&1
        RET_CODE=$?
        if [ ${RET_CODE} -ne 0 ];then
            Log "start rac database-${DBNAME} failed, error=${RET_CODE}"
            exit ${ERROR_START_RACDB}
        fi
    fi
}

GetLastRmanTaskSpeed()
{
    Log "Exec SQL to query last RmanTask speed."
    echo "set linesize 300" > "${SqlCMD}"
    echo "COL OUTPUT_BYTES_PER_SEC_DISPLAY FORMAT a50" >> "${SqlCMD}"
    echo "COL SIZE_KB FORMAT 999999999999" >> "${SqlCMD}"
    echo "select p.* from(select OUTPUT_BYTES_PER_SEC_DISPLAY, ROUND(OUTPUT_BYTES/1024,0) SIZE_KB from v\$rman_backup_job_details \
    where COMMAND_ID='ProtectAgent_Backup' order by SESSION_KEY desc)p where rownum = 1;" >> "${SqlCMD}"
    echo "exit" >> "${SqlCMD}"
    OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${SqlCMD}" "${SqlRST}" "${DBINSTANCE}" 30 1 "${ORA_DB_USER}" >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ ${RET_CODE} -ne 0 ]; then
        Log "Query for last RmanTask speed failed, ret=${RET_CODE}"
        DeleteFile "${SqlCMD}" "${SqlRST}"
        exit $RET_CODE
    fi
    TaskSpeed=`cat ${SqlRST} | sed -n '1p' | $MYAWK '{print $1}'`
    BackupSize=`cat ${SqlRST} | sed -n '1p' | $MYAWK '{print $2}'`
    DeleteFile  "${SqlCMD}" "${SqlRST}"
}

GetDatabaseNodeList()
{
    echo "set linesize 300;" > "${SqlCMD}"
    echo "select THREAD# from GV\$INSTANCE where STATUS='OPEN';" >> "${SqlCMD}"
    echo "exit" >> "${SqlCMD}"
    OracleExeSql "${DBUSER}" "${DBUSERPWD}" ${SqlCMD} ${SqlRST} ${DBINSTANCE} 30 1 ${ORA_DB_USER} >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ ${RET_CODE} -ne 0 ]; then
        Log "GetDatabaseNodeList failed, ret=${RET_CODE}"
        DeleteFile "${SqlCMD}" "${SqlRST}"
        exit $RET_CODE
    fi
    nodeList=`cat ${SqlRST}`
}

UpdateArciveLogRange()
{
    GetDatabaseNodeList
    for dirName in `ls -F ${ARCHIVE} | grep "/$" | grep resetlogs_id*`; do
        resetlogs_id=`RDsubstr ${dirName%?} 13`
        LOG_IS_BACKED_UP="(RESETLOGS_ID='${resetlogs_id}') and (deleted = 'NO') and (ARCHIVED='YES') \
            and (STATUS != 'U') and name like '$ARCHIVE/resetlogs_id${resetlogs_id}/arch_%'"
        for node in `echo ${nodeList}`; do
            scnRange=`GetArciveLogScnRangeByTHREAD ${node}`
            Log "resetlogs_id=${resetlogs_id}, node=${node}, all scnRange=${scnRange}"
            if [ -z "${scnRange}" ]; then
                continue
            fi
            echo "node${node}=${scnRange}" >> ${ARCHIVE}/resetlogs_id${resetlogs_id}/log_range.bak
        done
        if [ -f ${ARCHIVE}/resetlogs_id${resetlogs_id}/log_range.bak ]; then
            mv ${ARCHIVE}/resetlogs_id${resetlogs_id}/log_range.bak ${ARCHIVE}/resetlogs_id${resetlogs_id}/log_range
        fi
    done
}

SplicingRange()
{
    paramA=$1
    aMinSCN=`echo "${paramA}" | ${MYAWK} '{print $NF}' | ${MYAWK} -F "~" '{print $1}'`
    aMinTime=`echo "${paramA}" | ${MYAWK} '{print $NF}' | ${MYAWK} -F "~" '{print $2}'`
    aMaxSCN=`echo "${paramA}" | ${MYAWK} '{print $NF}' | ${MYAWK} -F "~" '{print $3}'`
    aMaxTime=`echo "${paramA}" | ${MYAWK} '{print $NF}' | ${MYAWK} -F "~" '{print $4}'`
    paramB=$2
    bMinSCN=`echo "${paramB}" | ${MYAWK} '{print $1}' | ${MYAWK} -F "~" '{print $1}'`
    bMinTime=`echo "${paramB}" | ${MYAWK} '{print $1}' | ${MYAWK} -F "~" '{print $2}'`
    bMaxSCN=`echo "${paramB}" | ${MYAWK} '{print $1}' | ${MYAWK} -F "~" '{print $3}'`
    bMaxTime=`echo "${paramB}" | ${MYAWK} '{print $1}' | ${MYAWK} -F "~" '{print $4}'`

    if [ "${aMaxSCN}" -eq "${bMinSCN}" ]; then
        rst=`echo "${paramA}" | ${MYAWK} '{for (i=1;i<NF;i++)printf("%s ", $i);print ""}'`
        rst="${rst}${aMinSCN}~${aMinTime}~${bMaxSCN}~${bMaxTime}"
        rst="${rst}`echo "${paramB}" | ${MYAWK} '{for (i=2;i<=NF;i++)printf(" %s", $i);print ""}'`"
    else
        rst="${paramA} ${paramB}"
    fi
    echo "${rst}"
}

UpdateArciveLogRangeCurrent()
{
    if [ -f ${ARCHIVE}/resetlogs_id${resetlogs_id}/log_range ]; then
        mv ${ARCHIVE}/resetlogs_id${resetlogs_id}/log_range ${ARCHIVE}/resetlogs_id${resetlogs_id}/log_range.bak
    else
        touch ${ARCHIVE}/resetlogs_id${resetlogs_id}/log_range.bak
    fi

    GetDatabaseNodeList
    LOG_IS_BACKED_UP="(RESETLOGS_ID='${resetlogs_id}') and (deleted = 'NO') and (ARCHIVED='YES') \
        and (STATUS != 'U') and name like '$ARCHIVE/resetlogs_id${resetlogs_id}/arch_%' and next_change#>${from_scn}"
    for node in `echo ${nodeList}`; do
        scnRange=`GetArciveLogScnRangeByTHREAD ${node}`
        Log "resetlogs_id=${resetlogs_id}, node=${node}, new scnRange=${scnRange}"
        if [ -z "${scnRange}" ]; then
            continue
        fi
        oldRange=`cat ${ARCHIVE}/resetlogs_id${resetlogs_id}/log_range.bak | grep "^node${node}=" | ${MYAWK} -F "=" '{print $2}'`
        if [ -z "${oldRange}" ]; then
            echo "node${node}=${scnRange}" >> ${ARCHIVE}/resetlogs_id${resetlogs_id}/log_range.bak
        else
            scnRange=`SplicingRange "${oldRange}" "${scnRange}"`
            sed -i "s/node${node}=.*/node${node}=${scnRange}/" ${ARCHIVE}/resetlogs_id${resetlogs_id}/log_range.bak
        fi
    done
    mv ${ARCHIVE}/resetlogs_id${resetlogs_id}/log_range.bak ${ARCHIVE}/resetlogs_id${resetlogs_id}/log_range
}

GetArciveLogRange()
{
    Log "begin GetArciveLogRange"
    local logbackuprst="logbackuprst"

    for dirName in `ls -F ${ARCHIVE} | grep "/$" | grep resetlogs_id*`; do
        resetlogs_id=`RDsubstr ${dirName%?} 13`
        rm -rf ${ARCHIVE}/resetlogs_id${resetlogs_id}/log_range.bak
        if [ ! -f ${ARCHIVE}/resetlogs_id${resetlogs_id}/log_range ] || [ ! -s ${ARCHIVE}/resetlogs_id${resetlogs_id}/log_range ]; then
            continue
        fi
        resultRange=
        while read line; do
            node=`echo ${line} | ${MYAWK} -F "=" '{print $1}'`
            scnRange=`echo ${line} | ${MYAWK} -F "=" '{print $2}'`
            if [ -z "${scnRange}" ]; then
                continue
            fi
            if [ -z "${resultRange}" ]; then
                resultRange=${scnRange}
            else
                resultRange=`CalIntersection "${resultRange}" "${scnRange}"`
            fi
        done < "${ARCHIVE}/resetlogs_id${resetlogs_id}/log_range"

        for word in `echo ${resultRange}`; do
            minSCN=`echo $word | ${MYAWK} -F "~" '{print $1}'`
            minTime=`echo $word | ${MYAWK} -F "~" '{print $2}'`
            minTimeStamp=`AddUnixTimestamp $minTime`
            maxSCN=`echo $word | ${MYAWK} -F "~" '{print $3}'`
            maxTime=`echo $word | ${MYAWK} -F "~" '{print $4}'`
            maxTimeStamp=`AddUnixTimestamp $maxTime`
            logbackuprst="${logbackuprst};${minSCN};${minTimeStamp};${maxSCN};${maxTimeStamp};${resetlogs_id}"
        done
    done
    echo "${logbackuprst}"
}

GetArciveLogScnRangeByTHREAD()
{
    thread=$1

    echo "set linesize 300;" > "${SqlCMD}"
    echo "select min(sequence#), max(sequence#), min(first_change#), max(next_change#), \
    to_char(min(first_time), $TIME_FORMAT), to_char(max(next_time), $TIME_FORMAT) \
    from v\$archived_log where ${LOG_IS_BACKED_UP} and THREAD#=${thread};" >> "${SqlCMD}"
    echo "exit" >> "${SqlCMD}"
    OracleExeSql "${DBUSER}" "${DBUSERPWD}" ${SqlCMD} ${SqlRST} ${DBINSTANCE} 30 1 ${ORA_DB_USER} >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ ${RET_CODE} -ne 0 ]; then
        DeleteFile "${SqlCMD}" "${SqlRST}"
        exit $RET_CODE
    fi
    local minSequence=`cat ${SqlRST} | $MYAWK '{print $1}'`
    local maxSequence=`cat ${SqlRST} | $MYAWK '{print $2}'`
    local minScn=`cat ${SqlRST} | $MYAWK '{print $3}'`
    local maxScn=`cat ${SqlRST} | $MYAWK '{print $4}'`
    local minTime=`cat ${SqlRST} | $MYAWK '{print $5}'`
    local maxTime=`cat ${SqlRST} | $MYAWK '{print $6}'`
    Log "sequenceRange=${minSequence}~${maxSequence}, scnRange=${minScn}~${maxScn}, timeRange=${minTime}~${maxTime}"

    echo "set linesize 600;" > "${SqlCMD}"
    echo "select * from ( select \
    t.next_change# minscn, lead(t.first_change#,1,0) over(order by t.sequence#) maxscn, \
    t.next mintime, lead(t.first,1,0) over(order by t.sequence#) maxtime, \
    lead(t.sequence#,1,0) over(order by t.sequence#)-t.sequence#-1 bb \
    from (select sequence#, first_change#, next_change#, \
    to_char(first_time, 'YYYY-MM-DD_HH24:MI:SS') first, to_char(next_time, 'YYYY-MM-DD_HH24:MI:SS') next \
    from v\$archived_log where ${LOG_IS_BACKED_UP} and THREAD#=${thread})t ) where bb>0;" >> "${SqlCMD}"
    echo "exit" >> "${SqlCMD}"
    OracleExeSql "${DBUSER}" "${DBUSERPWD}" ${SqlCMD} ${SqlRST} ${DBINSTANCE} 30 1 ${ORA_DB_USER} >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ ${RET_CODE} -ne 0 ]; then
        DeleteFile "${SqlCMD}" "${SqlRST}"
        exit $RET_CODE
    fi
    local missingNum=`cat ${SqlRST} | wc -l`
    Log "missingNum=${missingNum}"

    if [ ${missingNum} -eq 0 ]; then
        local scnRange="${minScn}~${minTime}~${maxScn}~${maxTime}"
    else
        local scnRange="${minScn}~${minTime}"
        for i in $(seq 1 ${missingNum}); do
            t_begin="`cat ${SqlRST} | sed -n "${i}p" | $MYAWK '{print $1}'`~`cat ${SqlRST} | sed -n "${i}p" | $MYAWK '{print $3}'`"
            t_end="`cat ${SqlRST} | sed -n "${i}p" | $MYAWK '{print $2}'`~`cat ${SqlRST} | sed -n "${i}p" | $MYAWK '{print $4}'`"
            if [ ${i} -eq ${missingNum} ]; then
                scnRange="${scnRange}~${t_begin} ${t_end}~${maxScn}~${maxTime}"
            else
                scnRange="${scnRange}~${t_begin} ${t_end}"
            fi
        done
    fi
    if [ ${minSequence} -eq ${maxSequence} ]; then
        scnRange=
    fi
    echo ${scnRange}
}

CalIntersection()
{
    local paramA=$1
    local paramB=$2
    local rstCalIntersection=
    for wordA in `echo ${paramA}`; do
        local aMinSCN=`echo $wordA | ${MYAWK} -F "~" '{print $1}'`
        local aMinTime=`echo $wordA | ${MYAWK} -F "~" '{print $2}'`
        local aMaxSCN=`echo $wordA | ${MYAWK} -F "~" '{print $3}'`
        local aMaxTime=`echo $wordA | ${MYAWK} -F "~" '{print $4}'`
        for wordB in `echo ${paramB}`; do
            local bMinSCN=`echo $wordB | ${MYAWK} -F "~" '{print $1}'`
            local bMinTime=`echo $wordB | ${MYAWK} -F "~" '{print $2}'`
            local bMaxSCN=`echo $wordB | ${MYAWK} -F "~" '{print $3}'`
            local bMaxTime=`echo $wordB | ${MYAWK} -F "~" '{print $4}'`
            if [[ ${aMinSCN} -gt ${bMaxSCN} ]] || [[ ${bMinSCN} -gt ${aMaxSCN} ]]; then
                continue
            fi

            if [ ${aMinSCN} -gt ${bMinSCN} ]; then
                rstCalIntersection="${rstCalIntersection} ${aMinSCN}~${aMinTime}"
            else
                rstCalIntersection="${rstCalIntersection} ${bMinSCN}~${bMinTime}"
            fi
            if [ ${aMaxSCN} -gt ${bMaxSCN} ]; then
                rstCalIntersection="${rstCalIntersection}~${bMaxSCN}~${bMaxTime}"
            else
                rstCalIntersection="${rstCalIntersection}~${aMaxSCN}~${aMaxTime}"
            fi
        done
    done
    echo ${rstCalIntersection}
}

RedirectBackPath()
{
    backuppath=$1
    testpath=${backuppath%%;*}
    if [ -d "${testpath}/data" ]; then
        backup=`echo ${backuppath} | sed 's/;/ /g'`
        tempbackuppath=""
        for i in ${backup}; do 
            tempbackuppath="${tempbackuppath}${i}/data;"
        done
        backuppath=${tempbackuppath}
    fi 
    echo "${backuppath}"   
}

CheckDGExit()
{
    if [ ! -z "${RECOVERPATH}" ] || [ "$RECOVERTARGET" -ne "2" ]; then
        return 0
    fi
    local DGList=
    local dbfFullPath=
    local DGName=
    for line in `cat $ADDITIONAL/dbfiles`; do
        dbfFullPath=`echo $line | ${MYAWK} -F ";" '{print $4}'`
        if [ `RDsubstr ${dbfFullPath} 1 1` != "+" ]; then
            continue
        fi
        DGName=`echo ${dbfFullPath%%/*}`
        echo "${DGList}" | grep ${DGName} >/dev/null 2>&1
        if [ $? -ne 0 ]; then
            DGList="${DGList} ${DGName}"
        fi
    done
    for line in `cat $ADDITIONAL/ctrlfiles`; do
        dbfFullPath="$line"
        if [ `RDsubstr ${dbfFullPath} 1 1` != "+" ]; then
            continue
        fi
        DGName=`echo ${dbfFullPath%%/*}`
        echo "${DGList}" | grep ${DGName} >/dev/null 2>&1
        if [ $? -ne 0 ]; then
            DGList="${DGList} ${DGName}"
        fi
    done
    for line in `cat $ADDITIONAL/logfiles`; do
        dbfFullPath=`echo $line | ${MYAWK} -F ";" '{print $2}'`
        if [ `RDsubstr ${dbfFullPath} 1 1` != "+" ]; then
            continue
        fi
        DGName=`echo ${dbfFullPath%%/*}`
        echo "${DGList}" | grep ${DGName} >/dev/null 2>&1
        if [ $? -ne 0 ]; then
            DGList="${DGList} ${DGName}"
        fi
    done
    if [ -z "${DGList}" ]; then
        return 0
    fi
    Log "begin CheckDGExit DGList=${DGList}"

    for DGName in `echo ${DGList}`; do
        DGName=`RDsubstr ${DGName} 2`
        echo lsdg ${DGName} | su - ${ORA_GRID_USER} ${GRID_SHELLTYPE} -c "${EXPORT_GRID_ENV}export ORACLE_SID=${ASMSIDNAME}; asmcmd" | grep "${DGName}"
        if [ $? -ne 0 ]; then
            Log "diskgroup ${DGName} does not exist or is not mounted"
            exit ${ERROR_ORACLE_DISKGROUP_NOT_EXIT}
        fi
    done
}

GetDirFileinfo()
{
    local filelist="$STMP_PATH/filelist${PID}"
    local DirPath=$1;
    DeleteFile $filelist
    Dirpathlength=`echo $DirPath | $MYAWK '{print length($0)}'`
    pathlength=`expr ${Dirpathlength} + 1`
    Log "Start get Dir $DirPath file info ."
    filesizeinfo=`ls -R -l $DirPath | $MYAWK '{print $1";"$9";"$5}'`
    pathprefix=
    for file in $filesizeinfo ; do
        title=`RDsubstr $file 1 1`
        if [ "$title" = "d" ]; then
            continue
        fi
        if [ "$title" = "t" ]; then
            continue
        fi
        if [ "$title" = ";" ]; then
            continue
        fi
        if [ "$title" = "/" ]; then
            filepathlength=`echo $file | $MYAWK '{print length($0)}'`
            filepathlength=`expr ${filepathlength} - 3`
            pathprefix=`RDsubstr $file 1 $filepathlength`
            continue
        fi
        filesize=`echo $file | $MYAWK -F ";" '{print $3}'`
        filename=`echo $file | $MYAWK -F ";" '{print $2}'`
        if [ -z "${filesize}" ] || [ -z "${filename}" ]; then
            continue
        fi
        fullfilename="${pathprefix}/$filename"
        fullfilename=`RDsubstr $fullfilename $pathlength`
        echo "$fullfilename;$filesize" >> "$filelist"

    done
}

CheckMountPath()
{
    if [ ! -z "${METADATAPATH}" ]; then
        local paths=`echo ${METADATAPATH} | sed -e 's/;/ /g'`
        local path=
        for path in ${paths}; do
            mount | grep "${path}" >> "${LOG_FILE_NAME}" 2>&1
            if [ $? -ne 0 ]; then
                ExitWithErrorCode "${path} is not mounted." $ERROR_MOUNTPATH
            fi
            chown -h ${ORA_DB_USER}:${ORA_DB_GROUP} ${path} >> "${LOG_FILE_NAME}" 2>&1
            if [ $? -ne 0 ]; then
                ExitWithErrorCode "operation permission: ${path}." $ERROR_MOUNTPATH
            fi
        done
    fi
}

CompareOracleVersion()
{
    # $1 = $2 return 0
    # $1 > $2 return 1
    # $1 < $2 return 2
    if [ $# -ne 2 ]; then
        return 3
    fi
    Version1=$1
    Version2=$2
    Log "$1 compare with $2"
    for num in 1 2 3 4 5 ; do
        PreVersion1=`echo $Version1 | $MYAWK -F "." -v NUM=$num '{print $NUM}'`
        PreVersion2=`echo $Version2 | $MYAWK -F "." -v NUM=$num '{print $NUM}'`
        if [ $PreVersion1 -eq $PreVersion2 ]; then
            continue
        fi
        if [ $PreVersion1 -gt $PreVersion2 ]; then
            Log "$1 greater than $2"
            return 1
        else
            Log "$1 lower than $2"
            return 2
        fi
    done
    Log "$1 equal $2"
    return 0
}

ModifyClusterProperty()
{
    status=$1
    TEMP_SQL="${STMP_PATH}/TEMP${PID}.sql"
    TEMP_RST="${STMP_PATH}/TEMPRST${PID}.txt"
    TEMPDBID=`cat ${ADDITIONAL}/dbinfo | $MYAWK -F ";" '{print $1}'`
    echo "alter system set cluster_database=$status scope=spfile;" > ${TEMP_SQL}
    echo "shutdown immediate;" >> ${TEMP_SQL}
    Log "Begin to modify cluster property."
    OracleExeSql  "${DBUSER}" "${DBUSERPWD}" "${TEMP_SQL}" "${TEMP_RST}" "${DBINSTANCE}" -1 0 "${ORA_DB_USER}" >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ ${RET_CODE} -ne 0 ] && [ ${RET_CODE} -ne 41 ] ;then
        Log "Modify cluster-${DBINSTANCE} failed, error=${RET_CODE}."
        cat ${TEMP_RST} >> ${LOG_FILE_NAME}
        DeleteFile ${TEMP_SQL} ${TEMP_RST}
        exit ${RET_CODE}
    fi
    Log "Modify cluster-${DBINSTANCE} succ."
    DeleteFile ${TEMP_SQL} ${TEMP_RST}
}

StartUpOracleMount()
{
    TEMP_SQL="${STMP_PATH}/TEMP${PID}.sql"
    TEMP_RST="${STMP_PATH}/TEMPRST${PID}.txt"
    echo "startup mount" > ${TEMP_SQL}
    echo "exit;" >> ${TEMP_SQL}
    OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${TEMP_SQL}" "${TEMP_RST}" "${DBINSTANCE}" -1 1 "${ORA_DB_USER}" >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ ${RET_CODE} -ne 0 ]; then
        Log "Alter database mount failed, ret="${RET_CODE}"."
        cat ${TEMP_RST} >> ${LOG_FILE_NAME}
        DeleteFile ${TEMP_SQL} ${TEMP_RST}
        exit ${RET_CODE}
    fi
    Log "Alter database mount success."
    DeleteFile ${TEMP_SQL} ${TEMP_RST}
}

OracleUpgrade()
{
    TEMP_SQL="${STMP_PATH}/TEMP${PID}.sql"
    TEMP_RST="${STMP_PATH}/TEMPRST${PID}.txt"
    UPGRADE_RST="${STMP_PATH}/upgrade${PID}.txt"
    CreateOracleSTMPFile $UPGRADE_RST
    if [ ${ORACLE_IS_CDB} -eq 0 ]; then
        echo "ALTER PLUGGABLE DATABASE ALL OPEN UPGRADE FORCE;" > ${TEMP_SQL}
        OracleExeSql  "${DBUSER}" "${DBUSERPWD}" "${TEMP_SQL}" "${TEMP_RST}" "${DBINSTANCE}" -1 0 "${ORA_DB_USER}" >> "${LOG_FILE_NAME}" 2>&1
        RET_CODE=$?
        if [ ${RET_CODE} -ne 0 ];then
            Log "Open upgrade pdb database-${DBINSTANCE} failed, error=${RET_CODE}."
            cat ${TEMP_RST} >> ${LOG_FILE_NAME}
            DeleteFile ${TEMP_SQL} ${TEMP_RST}
            exit ${RET_CODE}
        fi
        DeleteFile ${TEMP_SQL} ${TEMP_RST}
        Log "Open upgrade pdb database-${DBINSTANCE} success."
    fi
    if [ `RDsubstr ${ORA_VERSION} 1 2`  -gt 11 ]; then
        su - ${ORA_DB_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV}export ORACLE_SID=${DBINSTANCE}; cd \$ORACLE_HOME/rdbms/admin; \$ORACLE_HOME/perl/bin/perl catctl.pl -n 4 catupgrd.sql >>\"${UPGRADE_RST}\" 2>&1 "
        if [ $? -ne 0 ]; then
            Log "Upgrade database ${DBINSTANCE} failed."
            su - ${ORA_DB_USER} ${ORACLE_SHELLTYPE} -c "${EXPORT_ORACLE_ENV}cat ${UPGRADE_RST}" >> ${LOG_FILE_NAME}
            DeleteFile ${UPGRADE_RST}
            exit $?
        fi
    else 
        echo "spool catupgrade.log" > ${TEMP_SQL}
        echo "@?/rdbms/admin/catupgrd.sql" >> ${TEMP_SQL}
        OracleExeSql  "${DBUSER}" "${DBUSERPWD}" "${TEMP_SQL}" "${TEMP_RST}" "${DBINSTANCE}" -1 0 "${ORA_DB_USER}" >> "${LOG_FILE_NAME}" 2>&1
        RET_CODE=$?
        if [ ${RET_CODE} -ne 0 ]; then
            Log "Upgrade database-${DBINSTANCE} failed, error=${RET_CODE}."
            cat ${TEMP_RST} >> ${LOG_FILE_NAME}
            DeleteFile ${TEMP_SQL} ${TEMP_RST}
        fi
    fi
    echo "startup" > ${TEMP_SQL}
    echo "@?/rdbms/admin/utlrp.sql" >> ${TEMP_SQL}
    OracleExeSql  "${DBUSER}" "${DBUSERPWD}" "${TEMP_SQL}" "${TEMP_RST}" "${DBINSTANCE}" -1 0 "${ORA_DB_USER}" >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ ${RET_CODE} -ne 0 ] && [ ${RET_CODE} -ne 31 ];then
        Log "Check upgrade database-${DBINSTANCE} failed, error=${RET_CODE}."
        cat ${TEMP_RST} >> ${LOG_FILE_NAME}
        DeleteFile ${TEMP_SQL} ${TEMP_RST}
        exit ${RET_CODE}
    fi
    Log "Check upgrade database-${DBINSTANCE} succ."
    ShutDownDB ${DBINSTANCE}
    StartDB
    Log "Upgrade database-${DBINSTANCE} succ."
    DeleteFile ${TEMP_SQL} ${TEMP_RST}
}

GetOracleUserEnvFile()
{
    GetOracleUser
    SHELLTYPE=`cat /etc/passwd | grep "^${ORA_DB_USER}:" | $MYAWK -F "/" '{print $NF}'`
    if [ ${SHELLTYPE} = "ksh" ]; then
        ORACLE_ENV_FILE=".kshrc .profile"
    elif [ ${SHELLTYPE} = "csh" ]; then
        ORACLE_ENV_FILE=".cshrc .login .tcshrc"
    elif [ ${SHELLTYPE} = "bash" ]; then
        ORACLE_ENV_FILE=".profile .bashrc .bash_login .bash_profile"
    elif [ ${SHELLTYPE} = "zsh" ]; then
        ORACLE_ENV_FILE=".zshenv .zprofile .zshrc .zlogin"
    fi
    SHELLTYPE=`cat /etc/passwd | grep "^${ORA_GRID_USER}:" | $MYAWK -F "/" '{print $NF}'`
    if [ -z ${SHELLTYPE} ]; then
        return 0
    fi
    if [ ${SHELLTYPE} = "ksh" ]; then
        GRID_ENV_FILE=".kshrc .profile"
    elif [ ${SHELLTYPE} = "csh" ]; then
        GRID_ENV_FILE=".cshrc .login .tcshrc"
    elif [ ${SHELLTYPE} = "bash" ]; then
        GRID_ENV_FILE=".profile .bashrc .bash_login .bash_profile"
    elif [ ${SHELLTYPE} = "zsh" ]; then
        GRID_ENV_FILE=".zshenv .zprofile .zshrc .zlogin"
    fi
}

GenerateExportCmd()
{
    envCmd=""
    tmpFileList=$1
    if [ -z "${tmpFileList}" ]; then
        return
    fi

    envCmd=""
    index=1 
    while [ 1 ]; do
        envFile=`echo "${tmpFileList}" | $MYAWK -v i="$index" '{print $i}'`
        if [ "${envFile}" = "" ]; then
            break
        fi
        envCmd="${envCmd} if [ -f \"${envFile}\" ]; then source ${envFile}; fi;"
        index=`expr $index + 1`
    done
    echo "${envCmd}"
}

ORACLE_ENV_FILE=
GRID_ENV_FILE=
GetOracleUserEnvFile
EXPORT_ORACLE_ENV=`GenerateExportCmd "${ORACLE_ENV_FILE}"`
EXPORT_GRID_ENV=`GenerateExportCmd "${GRID_ENV_FILE}"`