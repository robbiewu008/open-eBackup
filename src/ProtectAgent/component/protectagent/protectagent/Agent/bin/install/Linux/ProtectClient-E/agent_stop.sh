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

LOG_USER=$LOGNAME
if [ "root" = "${LOG_USER}" ]; then
    echo "You can not execute this script with user root."
    exit 1
fi

#use AGENT_ROOT env param
AGENT_ROOT_PATH=${AGENT_ROOT}
LOG_FILE_NAME="${AGENT_ROOT_PATH}/log/agent_stop.log"
. "${AGENT_ROOT_PATH}/bin/agent_bin_func.sh"
EXPLUG_PID_PATH="${AGENT_ROOT_PATH}/conf/PluginPid"
WILD_PROC_LIST_KILL="monitor rdagent nginx"

SYS_NAME=`uname -s`
if [ "$SYS_NAME" = "SunOS" ]; then
    MYAWK=nawk
else
    MYAWK=awk
fi

GetPidsByName()
{
    PROC_NAME="$1"
    if [ "$1" = "dataprocess" ]; then
        if [ "$SYS_NAME" = "Linux" ]; then
            PROC_IDS=`ps -afx | grep dataprocess | grep -v grep | $MYAWK '{print $1}'`
        else
            return    # unix dont support vmware
        fi
    else
        if [ "${SYS_NAME}" = "AIX" ]; then
            PROC_IDS=`ps -u ${LOGNAME} | grep -v grep | grep $PROC_NAME | $MYAWK '{print $2}'`
        else
            PROC_IDS=`ps -u ${LOGNAME} | grep -v grep | grep $PROC_NAME | $MYAWK '{print $1}'`
        fi
    fi

    RESULT=""
    for PROC_ID in ${PROC_IDS}
    do
        #check specified PID
        TEST_RET=`ps -p  ${PROC_ID} | grep ${PROC_ID}`
        if [ "*${TEST_RET}" != "*" ]; then
            RESULT="${RESULT} ${PROC_ID}"
        fi
    done
    
    echo ${RESULT}
}

WaitExternalPlugStop()
{
    if [ ! -d ${EXPLUG_PID_PATH} ]; then
        return 0
    fi
    pluginPidFiles=`ls ${EXPLUG_PID_PATH}`
    timeout=60
    for pluginName in $pluginPidFiles
    do
        Log "INFO: Begin to check whether $pluginName stoped."
        counter=0
        while [ $counter -lt $timeout ]
        do
            if [ ! -f "${EXPLUG_PID_PATH}"/"${pluginName}" ]; then
                Log "INFO: $pluginName stopped."
                break
            fi
            sleep 1
            counter=`expr ${counter} + 1`
        done
        if [ $counter -eq $timeout ]; then
            Log "WARN: $pluginName stop failed."
        fi
    done
}

#kill -9 all processed now
KillProcessByNameForce()
{
    PROC_LIST="$1"
    for PROC_NAME in $PROC_LIST
    do
        if [ "nginx" = "${PROC_NAME}" ]; then
            if [ -f "${AGENT_ROOT_PATH}/nginx/logs/nginx.pid" ]; then
                NGINX_PPID=`cat ${AGENT_ROOT_PATH}/nginx/logs/nginx.pid`
            fi
            #check nginx exist or not
            if [ "*" != "*${NGINX_PPID}" ]; then
                NGINX_EXIST=`ps -u ${LOGNAME} -ef | grep ${NGINX_PPID} | grep -v grep`
                if [ "*" != "*${NGINX_EXIST}" ]; then
                    #kill The parent process of nginx
                    kill -9 ${NGINX_PPID}
                    Log "INFO: Send kill to parent $PROC_NAME, call: kill -9 ${NGINX_PPID}"
                fi
            fi
        fi
        
        PROC_ID=`GetPidsByName $PROC_NAME`
        
        if [ "*$PROC_ID" = "*" ]; then
            continue
        fi
        
        if [ "${PROC_NAME}" = "rdagent" ]; then
            Log "INFO: Send to kill rdagent, call kill -USR1 $PROC_ID."
            kill -USR1 $PROC_ID
            WaitExternalPlugStop
        fi
        Log "INFO: Send kill to $PROC_NAME, call: kill -9 $PROC_ID."
        kill -9 $PROC_ID
    done
}

if [ $# = 1 ]; then
    KillProcessByNameForce "$1"
else
    KillProcessByNameForce "${WILD_PROC_LIST_KILL}"
fi

# If the dataprocess process exists, stop it.
echo "The DataBackup ProtectAgent service has been successfully stopped."
Log "The DataBackup ProtectAgent service has been successfully stopped."
exit 0

