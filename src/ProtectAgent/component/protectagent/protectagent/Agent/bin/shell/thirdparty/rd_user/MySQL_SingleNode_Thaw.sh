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
. "$2/bin/agent_thirdpartyfunc.sh"

function Main()
{
    Log "[INFO]:Begin to thaw mysql."
    
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
        Exit 1 -log "[ERROR]:mysql is not started." -ret "mysql is not started."
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
    TEMP_FILE_NAME="${AGENT_THIRDPARTY_TMPPATH}/mysqlunfreeze${PID}.tmp"
    mysql -u$MYSQL_USER -p$MYSQL_PASSWORD -e "show processlist;" >> "${TEMP_FILE_NAME}" 2>&1
    if [ $? -ne 0 ]
    then
        cat ${TEMP_FILE_NAME} >>"${LOG_FILE_NAME}"
        [ -f ${TEMP_FILE_NAME} ] && rm -rf ${TEMP_FILE_NAME}
        Exit 1 -log "[ERROR]:show process list failed." -ret "show process list failed."
    fi
    
    process_id=`cat ${TEMP_FILE_NAME} | grep "select 1 and sleep(60)" | ${MYAWK} -F " " '{print $1}'`
    if [ "$process_id" = "" ]
    then
        [ -f ${TEMP_FILE_NAME} ] && rm -rf ${TEMP_FILE_NAME}
        Exit 1 -log "[ERROR]:Get freeze process_id failed." -ret "Get freeze process_id failed."
    fi
    
    cat ${TEMP_FILE_NAME} | grep "select 1 and sleep(60)" | ${MYAWK} -F " " '{print $1}'| while read pid
    do
         Log "[INFO]:Try to stop sql process ${pid}."
	 mysql -u$MYSQL_USER -p$MYSQL_PASSWORD -e "kill $pid;" >> "${LOG_FILE_NAME}" 2>&1
	 if [ $? -ne 0 ]
	 then
	     [ -f ${TEMP_FILE_NAME} ] && rm -rf ${TEMP_FILE_NAME}
	     Exit 1 -log "[ERROR]:Thaw mysql failed.PIDs is ${process_ids}" -ret "[ERROR]:Thaw mysql failed.PIDs is ${process_ids}"
	 fi   
	 Log "[INFO]:Stop sql process ${pid} success."
     done 
    
    [ -f ${TEMP_FILE_NAME} ] && rm -rf ${TEMP_FILE_NAME}
    Exit 0 -log "[INFO]:Finish thaw mysql." 
}
Main