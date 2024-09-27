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

###### BACKUP_SCENE ######
BACKUP_SCENE=-1
BACKUP_SCENE_EXTERNAL=0
BACKUP_SCENE_INTERNAL=1

###### REGISTER_STATUS ######
REGISTER_STATUS=-1
REGISTER_STATUS_RUN=0
REGISTER_STATUS_FAIL=1
REGISTER_STATUS_SUCC=2
BACKUP_ROLE=-1
BACKUP_ROLE_HOST=0
BACKUP_ROLE_VMWARE=2
BACKUP_ROLE_GENERAL_PLUGIN=4
BACKUP_ROLE_SANCLIENT_PLUGIN=5

LOG_USER=$LOGNAME
if [ "root" = "${LOG_USER}" ]; then
    echo "You can not execute this script with user root."
    exit 1
fi

sysName=`uname -s`
if [ "$sysName" = "SunOS" ]; then
    MYAWK=nawk
else
    MYAWK=awk
fi

#use AGENT_ROOT env param
AGENT_ROOT_PATH="${AGENT_ROOT}"
SANCLIENT_AGENT_ROOT_PATH="${DATA_BACKUP_SANCLIENT_HOME}/DataBackup/SanClient/ProtectClient-E/"
CURRENT_PATH=`dirname $0` && cd $CURRENT_PATH && CURRENT_PATH=`pwd`

RDAGENT_PORT=""
NGINX_PORT=""
TESTCFG_BACK_ROLE=""
SHELL_TYPE_SH="/bin/sh"
TESTCFG_BACK_ROLE=`cat ${CURRENT_PATH}/../conf/testcfg.tmp | grep BACKUP_ROLE | ${MYAWK} -F '=' '{print $NF}'`
if [ "${TESTCFG_BACK_ROLE}" = "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ]; then
    AGENT_ROOT_PATH=${SANCLIENT_AGENT_ROOT_PATH}
fi

NIGINX_BACKUP_CONFIG="${AGENT_ROOT_PATH}/nginx/conf/nginx.conf"
LOG_FILE_NAME="${AGENT_ROOT_PATH}/log/agent_start.log"
. "${AGENT_ROOT_PATH}/bin/agent_bin_func.sh"

########################################################################################
# Function Definition
LogError()
{
    #$1=errorInfo,$2=errorCode, $3=errorDetailParam
    LogErr "$1" "$2" "job_log_agent_storage_update_install_fail_label" "$3"
}

CheckProcess()
{
    if [ ${sysName} = "AIX" ]; then
        ps  -u ${LOGNAME} | grep -v grep | grep $1 1>/dev/null 2>&1
    else
        ps -lu ${LOGNAME} | grep -v grep | grep $1 1>/dev/null 2>&1
    fi
    if [ $? -ne 0 ]; then
        Log "INFO: Process $1 of DataBackup ProtectAgent is not exist."
        return 1
    fi

    Log "INFO: Process $1 of DataBackup ProtectAgent is is already exist."
    return 0
}

# check whether pod can be used
# rdagent: 59540~59559  nginx: 59520~59539
CheckPod()
{
    Port=$1
    
    if [ -z "${Port}" ]; then
        Log "ERROR: nginx port is empty."
        return 1
    fi
    if [ "SunOS" = "${SYS_NAME}" ]; then
        Listeningnum=`netstat -an | grep "${Port}" | $MYAWK '$NF == "ESTABLISHED"     {print $0}' | wc -l`
    else
        TCPListeningnum=`netstat -an | grep ":${Port}" | $MYAWK '$1 == "tcp" && $NF == "LISTEN"     {print $0}' | wc -l`
        UDPListeningnum=`netstat -an | grep ":${Port}" | $MYAWK '$1 == "udp" && $NF == "0.0.0.0:*"  {print $0}' | wc -l`
        Listeningnum=`expr ${TCPListeningnum} + ${UDPListeningnum}`
    fi
    if [ ${Listeningnum} -eq 0 ]; then
        return 0
    fi
    Log "WARN:the port ${Port} has been used by other process."
    return 1
}

GetRandomPort()
{
    BeginSeq=$1
    FinalSeq=$2
    PotrUser=$3
    PortTemp=0

    Diff=`expr ${FinalSeq} - ${BeginSeq}`
    while true
    do
        # generate random port
        NumRandom=`cat /dev/urandom | head -n 10 | cksum | $MYAWK '{print $1}'`
        PortTemp=`expr ${NumRandom} % ${Diff} + ${BeginSeq}`
       
        # check whether the port is free 
        CheckPod ${PortTemp}
        if [ $? -eq 0 ]; then
            if [ "${PotrUser}" = "rdagent" ]; then
                RDAGENT_PORT=${PortTemp}
            elif [ "${PotrUser}" = "nginx" ]; then
                NGINX_PORT=${PortTemp}
            fi
            return 0
        fi
    done
}

SetPort()
{
    PORTREX="^[1-9][0-9]\\{0,4\\}$"
    PORT=0
    PORT_NUMBER=0
    while [ ${PORT_NUMBER} -lt 5 ]
    do
        #read PORT
        if [ "rdagent" = "$1" ]; then
            GetRandomPort 59540 59559 "rdagent"
            PORT=${RDAGENT_PORT}
        elif [ "nginx" = "$1" ]; then
            # VMWare Host Use a fixed port(59526),other use random port.
            if [ ${TESTCFG_BACK_ROLE} -eq ${BACKUP_ROLE_VMWARE} ]; then
                PORT=59526
            else
                if [ -f "${NIGINX_BACKUP_CONFIG}" ]; then
                    PORT=`cat ${NIGINX_BACKUP_CONFIG} | grep "listen" | $MYAWK '{print $2}' | $MYAWK -F ":" '{print $NF}'`
                    netstat -an | grep ${PORT} 1>/dev/null 2>&1
                    if [ $? -eq 0 ]; then
                        echo "The nginx listening port[${PORT}] is occupied, and a new port will be allocated randomly."
                        echo "Please refer to the ProtectAgent deployment chapter [Step 5: Enabling the Input and Output Functions of the Nginx Listening Port] for configuration."
                        Log "The nginx listening port[${PORT}] is occupied, and a new port will be allocated randomly."
                        Log "Please refer to the ProtectAgent deployment chapter [Step 5: Enabling the Input and Output Functions of the Nginx Listening Port] for configuration."
                        if [ "${TESTCFG_BACK_ROLE}" = "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ]; then
                            GetRandomPort 59570 59580 "nginx"
                            PORT=${NGINX_PORT}
                        else
                            GetRandomPort 59520 59539 "nginx"
                            PORT=${NGINX_PORT}
                        fi
                    fi
                elif [ "${TESTCFG_BACK_ROLE}" = "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ]; then
                    GetRandomPort 59570 59580 "nginx"
                    PORT=${NGINX_PORT}
                else
                    GetRandomPort 59520 59539 "nginx"
                    PORT=${NGINX_PORT}
                fi
            fi
        fi

        if [ "${PORT}" = "" ]; then
            if [ "rdagent" = "$1" ]; then
                PORT=8091
            elif [ "nginx" = "$1" ]; then 
                PORT=59526
            fi
        fi

        PORTCHECK=`echo "${PORT}" | grep -n "${PORTREX}"`
        if [ "${PORTCHECK}" = "" ]; then
            PORT_NUMBER=`expr ${PORT_NUMBER} + 1`
            sleep 1
            continue
        elif [ ${PORT} -gt 65535 -o ${PORT} -le 1024 ]; then
            PORT_NUMBER=`expr ${PORT_NUMBER} + 1`
            sleep 1
            continue
        fi

        if [ "Linux" = "${SYS_NAME}" ]; then
            PORTS=`netstat -an | grep ${PORT} | grep -v TIME_WAIT | $MYAWK '{print $4}' | $MYAWK -F ':' '{print $NF}'`
        elif [ "HP-UX" = "${SYS_NAME}" ]; then
            PORTS=`netstat -an | grep ${PORT} | grep -v TIME_WAIT | $MYAWK -F " " '{print $4}' | $MYAWK -F "." '{print $NF}'`
        elif [ "AIX" = "${SYS_NAME}" ]; then
            PORTS=`netstat -an | grep ${PORT} | grep -v TIME_WAIT| $MYAWK -F '.' '{print $2}' | $MYAWK -F ' ' '{print $1}'`
        elif [ "SunOS" = "${SYS_NAME}" ]; then
            PORTS=`netstat -an | grep ${PORT} | grep -v TIME_WAIT | $MYAWK '{print $1}' | $MYAWK -F '.' '{print $NF}'`
        fi

        PORTSTATUS=""
        for PORTEACH in ${PORTS}
        do
            if [ "${PORTEACH}" = "${PORT}" ]; then
                PORTSTATUS=${PORTEACH}
                break
            fi
        done

        if [ "${PORTSTATUS}" != "" ]; then 
            Log "The port number(${PORT}) is used by other process."
            PORT_NUMBER=`expr ${PORT_NUMBER} + 1`
            sleep 1
            continue
        fi

        if [ "rdagent" = "$1" ]; then
            AGENT_PORT=${PORT}
        fi

        break
    done

    if [ ${PORT_NUMBER} = 5 ]; then
        Log "Set port number failed for five times."
        return 1
    fi
    
    if [ "nginx" = "$1" ]; then
        if [ -f "${AGENT_ROOT_PATH}/nginx/conf/nginx.conf" ]; then
            ISIPV6=`cat ${AGENT_ROOT_PATH}/nginx/conf/nginx.conf | grep "listen" | grep "]"`
            if [ "${ISIPV6}" != "" ]; then
                IP=`cat ${AGENT_ROOT_PATH}/nginx/conf/nginx.conf | grep "listen" | $MYAWK '{print $2}' | cut -d ']' -f 1`
                sed  "/listen/s/.*/        listen       ${IP}]:${PORT} ssl;/g" ${AGENT_ROOT_PATH}/nginx/conf/nginx.conf > ${AGENT_ROOT_PATH}/nginx/conf/nginx.conf.bak
            else
                IP=`cat ${AGENT_ROOT_PATH}/nginx/conf/nginx.conf | grep "listen" | $MYAWK '{print $2}' | cut -d ':' -f 1`
                sed  "/listen/s/.*/        listen       ${IP}:${PORT} ssl;/g" ${AGENT_ROOT_PATH}/nginx/conf/nginx.conf > ${AGENT_ROOT_PATH}/nginx/conf/nginx.conf.bak
            fi
        else
            Log "ERROR: ${AGENT_ROOT_PATH}/nginx/conf/nginx.conf is not exsits."
            return 1
        fi
        mv -f ${AGENT_ROOT_PATH}/nginx/conf/nginx.conf.bak ${AGENT_ROOT_PATH}/nginx/conf/nginx.conf
        #change nginx.conf 664 to 640
        chmod 640 ${AGENT_ROOT_PATH}/nginx/conf/nginx.conf
    fi
    Log "Set $1 listening Ip(${IP}) port($PORT) successfully."
    return 0
}

KillSonPids()
{
	pids=`ps -ef | ${MYAWK} '{if($3=='$1'){print $2} }'`;
        kill -9 $1 2>/dev/null
	if [ -n "$pids" ]; then
		for pid in $pids
		do
		    KillSonPids $pid
		done
	fi
}

JudgeRandomNumType()
{
    DISABLE_TRUE_RANDOM_NUM=1
    ENABLE_TRUE_RANDOM_NUM=0

    entropyValue=`cat /proc/sys/kernel/random/entropy_avail` >/dev/null 2>&1
    if [ "${entropyValue}" = "" ]; then
        return 1
    fi
    Log "Current entropy_avail is ${entropyValue}."
    if [ ${entropyValue} -gt 1000 ]; then
        ${AGENT_ROOT_PATH}/sbin/xmlcfg write Security disable_safe_random_number ${ENABLE_TRUE_RANDOM_NUM}
    else
        ${AGENT_ROOT_PATH}/sbin/xmlcfg write Security disable_safe_random_number ${DISABLE_TRUE_RANDOM_NUM}
    fi
}

# Command timeout function
Timeout()
{
    waitsec=30
    `$*` & pid=$!
    `sleep $waitsec && KillSonPids $pid` & watchdog=$!
    if wait $pid 2>/dev/null; then
        KillSonPids $watchdog
        wait $watchdog 2>/dev/null
        return 0
    else
        status=$?
        pid_exit=`ps -ef | ${MYAWK} '{print $2}'| grep -w $pid`
        if [ ! $pid_exit ]; then
            KillSonPids $watchdog
            wait $watchdog 2>/dev/null
        fi
        return $status
    fi
}

StartNginx()
{
    JudgeRandomNumType
    nginxpod=""
    CheckProcess rdnginx
    if [ $? -eq 0 ]; then
        Log "INFO: Process nginx of DataBackup ProtectAgent has already started."
        return 0
    fi
    cd "${AGENT_ROOT_PATH}/nginx"
    OLD_UMASK=`umask`
    umask 077
    Timeout ./rdnginx
    if [ $? -eq 0 ]; then
        umask ${OLD_UMASK}
        Log "INFO: Process nginx of DataBackup ProtectAgent is start successfully."
        if [ -f "${AGENT_ROOT_PATH}/nginx/logs/nginx.pid" ]; then
            CHMOD 600 ${AGENT_ROOT_PATH}/nginx/logs/nginx.pid
        fi
        return 0
    else
        umask ${OLD_UMASK}
        Log "ERROR: Process nginx of DataBackup ProtectAgent start failed."
        return 1
    fi
}

StartAgent()
{
    JudgeRandomNumType
    CheckProcess rdagent
    if [ $? -ne 0 ]; then
        nohup "${AGENT_ROOT_PATH}/bin/rdagent" 1>/dev/null 2>&1 &
        if [ $? -eq 0 ]; then
            sleep 5
            PORT=`${AGENT_ROOT_PATH}/sbin/xmlcfg read System port`
            sed "/fastcgi_pass/s/.*/            fastcgi_pass   127.0.0.1:${PORT};/g" ${AGENT_ROOT_PATH}/nginx/conf/nginx.conf > ${AGENT_ROOT_PATH}/nginx/conf/nginx.conf.bak
            mv -f ${AGENT_ROOT_PATH}/nginx/conf/nginx.conf.bak ${AGENT_ROOT_PATH}/nginx/conf/nginx.conf
            #change nginx.conf 664 to 640
            chmod 640 ${AGENT_ROOT_PATH}/nginx/conf/nginx.conf
            Log "INFO: Process rdagent of DataBackup ProtectAgent start successfully."
            return 0
        else
            Log "ERROR: Process rdagent of DataBackup ProtectAgent start failed."
            return 1
        fi
    fi
    
    Log "INFO: Process rdagent of DataBackup ProtectAgent has already started."
    return 0
}

StartMonitor()
{
    ${AGENT_ROOT_PATH}/sbin/xmlcfg write Security disable_safe_random_number 1
    CheckProcess monitor
    if [ $? -ne 0 ]; then
        nohup "${AGENT_ROOT_PATH}/bin/monitor" 1>/dev/null 2>&1 &
        if [ $? -eq 0 ]; then
            Log "INFO: Process monitor of DataBackup ProtectAgent is successfully started."
            return 0
        else
            Log "ERROR: Process monitor of DataBackup ProtectAgent fails to be started."
            return 1
        fi
    fi
    
    Log "INFO: Process monitor of DataBackup ProtectAgent has already started."
    return 0
}

GetProcessPid()
{
    if [ ${sysName} = "AIX" ]; then
        RDAGENT_PID=`ps -u ${LOGNAME} | grep -v grep | grep $1 | $MYAWK '{print $2}'`
    else
        RDAGENT_PID=`ps -u ${LOGNAME} | grep -v grep | grep $1 | $MYAWK '{print $1}'`
    fi
    if [ $? -ne 0 ]; then
        echo "Query $1 pid in user ${LOGNAME} failed."
        LogError "Query $1 pid in user ${LOGNAME} failed." ${ERR_UPGRADE_FAIL_START_PROTECT_AGENT}
        exit $?
    fi
    
    echo ${RDAGENT_PID}>"${AGENT_ROOT_PATH}/log/$1.pid"
    CHMOD 600 "${AGENT_ROOT_PATH}/log/$1.pid"
}

GetBackupScene()
{
    Log "Start GetBackupScene."
    BACKUP_SCENE_CONFIG=`cat ${AGENT_ROOT_PATH}/conf/testcfg.tmp | grep "BACKUP_SCENE"`
    if [ -z "${BACKUP_SCENE_CONFIG}" ]; then
        BACKUP_SCENE=${BACKUP_SCENE_EXTERNAL}
    else
        BACKUP_SCENE=`cat ${AGENT_ROOT_PATH}/conf/testcfg.tmp | grep "BACKUP_SCENE" | ${MYAWK} -F '=' '{print $NF}'`
    fi

    if [ "${BACKUP_SCENE}" = "" ]; then
        Log "Get backup_scene from config  failed."
        exit 1
    fi
    Log "backup_scene=${BACKUP_SCENE}"
}

GetRegisterStatus()
{
    Log "Start GetRegisterStatus."
    TMP_REGISTER_STATUS=`cat ${AGENT_ROOT_PATH}/conf/testcfg.tmp | grep "register_status"`
    if [ -z "${TMP_REGISTER_STATUS}" ]; then
        REGISTER_STATUS=${REGISTER_STATUS_FAIL}
    else
        REGISTER_STATUS=`cat ${AGENT_ROOT_PATH}/conf/testcfg.tmp | grep "register_status" | ${MYAWK} -F '=' '{print $NF}'`
    fi

    if [ "${REGISTER_STATUS}" = "" ]; then
        Log "Get register_status failed."
        exit 1
    fi
    Log "register_status=${REGISTER_STATUS}."
}

ConfigInternalProxy()
{
    Log "Start ConfigInternalProxy."
    GetBackupScene
    if [ ${BACKUP_SCENE} -eq ${BACKUP_SCENE_EXTERNAL} ]; then
        return
    fi

    GetRegisterStatus
    if [ ${REGISTER_STATUS} -eq ${REGISTER_STATUS_RUN} ]; then
        return
    fi

    Log "ConfigInternalProxy wait."
    wait
}

if [ $# = 1 ]; then
    if [ "$1" = "rdagent" ]; then
        StartAgent
        ret=$?
        if [ ${ret} -ne 0 ]; then
            echo "Process agent of DataBackup ProtectAgent fails to be started."
            LogError "Process agent of DataBackup ProtectAgent fails to be started."  ${ERR_UPGRADE_FAIL_START_PROTECT_AGENT}
        else
            GetProcessPid rdagent
        fi
        exit ${ret}
    elif [ "$1" = "nginx" ]; then
        StartNginx
        exit $?
    elif [ "$1" = "monitor" ]; then
        StartMonitor
        ret=$?
        if [ ${ret} -ne 0 ]; then
            echo "Process monitor of DataBackup ProtectAgent fails to be started."
            LogError "Process monitor of DataBackup ProtectAgent fails to be started." ${ERR_UPGRADE_FAIL_START_PROTECT_AGENT}
        else
            GetProcessPid monitor
        fi
        exit ${ret}
    else
        echo "Invalid parameter."
        LogError "Invalid parameter." ${ERR_UPGRADE_FAIL_VERIFICATE_PARAMETER}
        exit 1
    fi
fi


StartAgent
if [ $? -ne 0 ]; then
    echo "Process agent of DataBackup ProtectAgent fails to be started."
    LogError "Process agent of DataBackup ProtectAgent fails to be started." ${ERR_UPGRADE_FAIL_START_PROTECT_AGENT}
    exit 1
fi

JudgeRandomNumType
CheckProcess rdnginx
if [ $? -ne 0 ]; then
    ${AGENT_ROOT_PATH}/bin/agentcli startnginx 1>/dev/null 2>&1
    if [ $? -ne 0 ]; then
        GetBackupScene
        if [ ${BACKUP_SCENE} -eq ${BACKUP_SCENE_INTERNAL} ]; then
            LogError "Internal nginx start failed,exit."
            exit 1
        fi

        succflag=1
        for i in 1 2 3
        do
            Log "Process nginx of DataBackup ProtectAgent fails to be started.Retry time is:${i}." 
            SetPort nginx
            ret=$?
            if [ ${ret} -ne 0 ]; then
                echo "Process nginx of DataBackup ProtectAgent fails to be started."
                LogError "Process nginx of DataBackup ProtectAgent fails to be started.Reset nginx ip failed" ${ERR_UPGRADE_FAIL_START_PROTECT_AGENT}
            fi
            ${AGENT_ROOT_PATH}/bin/agentcli startnginx 1>/dev/null 2>&1
            ret=$?
            if [ ${ret} -eq 0 ]; then
                succflag=0
                break;
            fi
        done
        if [ $? -ne 0 ]; then
            echo "Process nginx of DataBackup ProtectAgent fails to be started."
            LogError "Process nginx of DataBackup ProtectAgent fails to be started." ${ERR_UPGRADE_FAIL_START_PROTECT_AGENT}
            exit 1
        fi
    fi

fi

GetProcessPid rdagent

StartMonitor
if [ $? -ne 0 ]; then
    echo "Process monitor of DataBackup ProtectAgent fails to be started."
    LogError "Process monitor of DataBackup ProtectAgent fails to be started." ${ERR_UPGRADE_FAIL_START_PROTECT_AGENT}
    exit 1   
fi

GetProcessPid monitor
echo "The DataBackup ProtectAgent service is successfully started."
Log "The DataBackup ProtectAgent service is successfully started."

ConfigInternalProxy

exit 0

