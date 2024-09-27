#!/bin/sh
set +x
. "$2/bin/agent_thirdpartyfunc.sh"
function Main()
{    
    Log "[INFO]:Begin to freeze mysql."
    
    which mysql
    if [ $? -ne 0 ]
    then
        Exit 1 -log "[INFO]:mysql is not isInstall." -ret "mysql is not isInstall"
    fi
    service mysql status | grep "not running" >>"${LOG_FILE_NAME}" 2>&1
    ret1=$?
    service mysqld status | grep "dead" >>"${LOG_FILE_NAME}" 2>&1
    ret2=$?
    if [[ $ret1 -eq 0 || $ret2 -eq 0 ]]
    then
        Exit 1 -log "[INFO]:mysql is not started." -ret "mysql is not started"
    fi  
    
    GetValue "${INPUT_PARAMETER_LIST}" MysqlUser
    MYSQL_USER=$ArgValue
    if [ "$MYSQL_USER" = "" ]
    then
        Exit 1 -log "[ERROR]:Get mysql user failed." -ret "Get mysql user failed."
    fi
    
    GetValue "${INPUT_PARAMETER_LIST}" MysqlPassword
    MYSQL_PASSWORD=$ArgValue
    if [ "$MYSQL_PASSWORD" = "" ]
    then
        Exit 1 -log "[ERROR]:Get mysql password failed." -ret "Get mysql password failed."
    fi
    MYSQL_TEMP_FILE_NAME="${AGENT_THIRDPARTY_TMPPATH}/mysqlfreeze${PID}.tmp"
    
    mysql -u$MYSQL_USER -p$MYSQL_PASSWORD -e "show processlist;" > "${MYSQL_TEMP_FILE_NAME}" 2>&1
    if [ $? -ne 0 ]
    then
        cat ${MYSQL_TEMP_FILE_NAME} >>"${LOG_FILE_NAME}"
        [ -f ${MYSQL_TEMP_FILE_NAME} ] && rm -rf ${MYSQL_TEMP_FILE_NAME}
        Exit 1 -log "[ERROR]:Show process list failed." -ret "[ERROR]:Show process list failed."
    fi

    process_id=`cat ${MYSQL_TEMP_FILE_NAME} | grep "select 1 and sleep(60)" | ${MYAWK} -F " " '{print $1}'`
    if [ "$process_id" != "" ]
    then
        cat ${MYSQL_TEMP_FILE_NAME} >>"${LOG_FILE_NAME}"
        [ -f ${MYSQL_TEMP_FILE_NAME} ] && rm -rf ${MYSQL_TEMP_FILE_NAME}
        Exit 1 -log "[ERROR]:MySQL already been freezed " -ret "MySQL already been freezed "
    fi
    
    echo "flush tables with read lock;select 1 and sleep(60);" | mysql -u$MYSQL_USER -p$MYSQL_PASSWORD & >> "${LOG_FILE_NAME}" 2>&1
    if [ $? -ne 0 ]
    then
        Exit 1 -log "[ERROR]:Freeze mysql failed." -ret "Freeze mysql failed."
    fi  
    
    checkTime=0
    while [ 1 ]
    do
        mysql -u$MYSQL_USER -p$MYSQL_PASSWORD -e "show processlist;" > "${MYSQL_TEMP_FILE_NAME}" 2>&1
        if [ $? -ne 0 ]
        then
            cat ${MYSQL_TEMP_FILE_NAME} >>"${LOG_FILE_NAME}"
            [ -f ${MYSQL_TEMP_FILE_NAME} ] && rm -rf ${MYSQL_TEMP_FILE_NAME}
            Exit 1 -log "[ERROR]:Show process list failed." -ret "Show process list failed."
        fi
        
        process_id=`cat ${MYSQL_TEMP_FILE_NAME} | grep "select 1 and sleep(60)" | ${MYAWK} -F " " '{print $1}'`
        if [ "$process_id" = "" ]
        then
            checkTime=`expr $checkTime + 1`
            sleep 1
        else
            break
        fi
        
        if [ $checkTime -eq 10 ]
        then
            cat "${MYSQL_TEMP_FILE_NAME}" >>"${LOG_FILE_NAME}" 2>&1
            [ -f ${MYSQL_TEMP_FILE_NAME} ] && rm -rf ${MYSQL_TEMP_FILE_NAME}
            Exit 1 -log "[ERROR]:Mysql is not freeze." -ret "Mysql is not freeze."
        fi
    done
    
    [ -f ${TEMP_FILE_NAME} ] && rm -rf ${TEMP_FILE_NAME}
    [ -f ${MYSQL_TEMP_FILE_NAME} ] && rm -rf ${MYSQL_TEMP_FILE_NAME}
    Exit 0 -log "[INFO]:Finish freeze mysql." 
}
Main