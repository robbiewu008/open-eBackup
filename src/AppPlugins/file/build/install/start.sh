#!/bin/bash
#
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
#
# Start nas plugin process for agent
SCRIPT_PATH=$(cd $(dirname $0); pwd)
COMMON_PATH=${SCRIPT_PATH}
. ${COMMON_PATH}/common.sh
SCRIPT_NAME=$(basename $0)
if [ -z "$DATA_BACKUP_AGENT_HOME" ]; then
    echo "The environment variable: DATA_BACKUP_AGENT_HOME is empty."
    DATA_BACKUP_AGENT_HOME=${SCRIPT_PATH%/DataBackup/ProtectClient*};
    export DATA_BACKUP_AGENT_HOME
    echo "Set DATA_BACKUP_AGENT_HOME: ${DATA_BACKUP_AGENT_HOME}"
fi
if [ ! -d $DATA_BACKUP_AGENT_HOME ];then
    echo "Agent home dir do not exist"
    exit 1
fi
PLUGIN_INSTALL_PATH=${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient/Plugins
AGENT_FOLDER=${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient/ProtectClient-E
if [ "${OS_TYPE}" = "SunOS" ]; then
    AWK=nawk
else
    AWK=awk
fi
IP_RULE='^(2[0-4][0-9]|25[0-5]|1[0-9][0-9]|[1-9]?[0-9])(\.(2[0-4][0-9]|25[0-5]|1[0-9][0-9]|[1-9]?[0-9])){3}$'
AGENT_START_PORT=59570
AGENT_END_PORT=59600

check_paramters()
{
    if [ $# -ne 5 ];then
        log_echo "ERROR" "The script is asked to be five parameters." >> ${LOG_FILE}
        exit 1
    fi
    logPath="$1"
    startPort="$2"
    endPort="$3"
    agentIp="$4"
    agentPort="$5"

    tmpLogPath=`cd ${logPath} && pwd`
    echo "${tmpLogPath}" | grep -Eq "^${AGENT_FOLDER}"
    if [ $? -ne 0 ]; then
        log_echo "ERROR" "The path not satisfy requirment." >> ${LOG_FILE}
        exit 1
    fi

     echo "${startPort}"  | grep -Eq "^[0-9]{4,}$"
     if [ $? -ne 0 ];then
        log_echo "ERROR" "The start port only require to be a number of more than 1000, please check." >> ${LOG_FILE}
        exit 1
     fi

     echo "${endPort}"  | grep -Eq "^[0-9]{4,}$"
     if [ $? -ne 0 ];then
        log_echo "ERROR" "The end port only require to be a number of more than 1000, please check." >> ${LOG_FILE}
        exit 1
     fi

    if [ ${startPort} -gt ${endPort} ];then
        log_echo "ERROR" "The start port cannot be more than end port." >> ${LOG_FILE}
        exit 1
    fi

    if [ "${agentIp}" = "*['!'@#\$%^\&*'('')'_+]*" ] || [ `echo ${agentIp} | grep -Evc "${IP_RULE}"` -gt 0 ]; then
        log_echo "ERROR" "IP format is wrong, pls check"
        exit 1
    fi

    if [ ${agentPort} -lt ${AGENT_START_PORT} -o ${agentPort} -gt ${AGENT_END_PORT} ];then
        log_echo "ERROR" "The agentPort is wrong, please check it." >> ${LOG_FILE}
        exit 1
    fi
    log_echo "INFO" "The parameter verification is successful.." >> ${LOG_FILE}
}

set_ulimit()
{
    cur_ulimit=`ulimit -n`
    ulimit -n 1048576
    if [ $? -ne 0 ];then
        log_echo "WARN" "Failed to set ulimit -n" >> ${LOG_FILE}
        return 1
    fi
    ulimit -Hn 1048576
    if [ $? -ne 0 ];then
        ulimit -n ${cur_ulimit}
        log_echo "WARN" "Failed to set ulimit -Hn" >> ${LOG_FILE}
        return 1
    fi
    ulimitSoft=`ulimit -n`
    ulimitHard=`ulimit -Hn`
    log_echo "INFO" "Set ulimit soft ${ulimitSoft}, ulimit hard ${ulimitHard}" >> ${LOG_FILE}
}

check_plugin_exist()
{
    executable_file="$1"
    if [ "${OS_TYPE}" = "AIX" ] || [ "${OS_TYPE}" = "SunOS" ]; then  # AIX ps不支持-w选项
        curPid=`ps -ef | grep -E "./bin/${executable_file}" | grep -v start.sh | grep -v grep | ${AWK} '{print $2}'`
    else
        curPid=`ps -efww | grep -E "./bin/${executable_file}\>" | grep -v start.sh | grep -v grep | ${AWK} '{print $2}'`
    fi
    # 若插件进程存在，则先杀掉插件进程
    if [ -n "${curPid}" ]; then
        log_echo "WARNING" "File process already exist. Process id is ${curPid}, stop it first." >> ${LOG_FILE}
        kill -9 ${curPid}
    fi
}

start_plugin()
{
    logPath="$1"
    startPort="$2"
    endPort="$3"
    agentIp="$4"
    agentPort="$5"
    cd ${SCRIPT_PATH}/bin
    executable_file=AgentPlugin
    if [ ! -f ${executable_file} ];then
        if [ ! -f "${PLUGIN_NAME}" ]; then
            log_echo "ERROR" "Not exist executable file." >> ${LOG_FILE}
            exit 1
        else
           executable_file=${PLUGIN_NAME}
        fi
    fi

    check_plugin_exist ${executable_file}
    chmod +x ${executable_file}

    curPort=${startPort}
    while [ ${curPort} -le ${endPort} ]; do
        netstat -anp | grep -q "\<$curPort\>"
        if [ $? -ne 0 ];then
           break;
        fi
        ((curPort++))
    done

    LD_LIBRARY_PATH=${AGENT_FOLDER}/bin:${SCRIPT_PATH}/lib/3rd:${SCRIPT_PATH}/lib/platform:${SCRIPT_PATH}/lib:${SCRIPT_PATH}/lib/ext:${SCRIPT_PATH}/lib:${SCRIPT_PATH}/lib/agent_sdk:${SCRIPT_PATH}/lib/service/:${SCRIPT_PATH}/lib/dme:${SCRIPT_PATH}/lib/dme/3rd:${SCRIPT_PATH}/lib/dme/platform:${LD_LIBRARY_PATH}
    export LD_LIBRARY_PATH

    set_ulimit
    
    ${SCRIPT_PATH}/bin/${executable_file} "${logPath}" "${curPort}" "${endPort}" "${agentIp}" "${agentPort}"  &
    i=0
    while ((i < 3))  # 查询进程号3次
    do
        sleep 1  # 等待进程启动
        if [ "${OS_TYPE}" = "AIX" ] || [ "${OS_TYPE}" = "SunOS" ]; then  # AIX ps不支持-w选项
            curPid=`ps -ef | grep -E "./bin/${executable_file}" | grep -v start.sh | grep -v grep | ${AWK} '{print $2}'`
        else
            curPid=`ps -efww | grep -E "./bin/${executable_file}\>" | grep -v start.sh | grep -v grep | ${AWK} '{print $2}'`
        fi
        if [ -n "${curPid}" ]; then
            log_echo "INFO" "Start file process sucessfully. Process id is ${curPid}" >> ${LOG_FILE}
            return 0
        fi
        let i+=1
    done
    log_echo "ERROR" "Failed to start the file process." >> ${LOG_FILE}
    return 1
}

main()
{
    PLUGIN_NAME=`get_plugin_name`
    if [ $? -ne 0 ]; then
        exit 1
    fi
    LOG_FILE_PREFIX=`get_plugin_log_path`
    if [ $? -ne 0 ]; then
        exit 1
    fi

    LOG_FILE=${LOG_FILE_PREFIX}/start.log
    check_paramters "$@"
    if [ $? -ne 0 ];then
        return 1
    fi
    start_plugin "$@"
    if [ $? -eq 0 ]; then
        echo "[$(date "+%Y-%m-%d %H:%M:%S")][INFO][ Start agent plugin ${PLUGIN_NAME} successfully ]" >> ${LOG_FILE}
        return 0
    fi
    echo "[$(date "+%Y-%m-%d %H:%M:%S")][ERROR][ Failed to start agent plugin ${PLUGIN_NAME}. ]" >> ${LOG_FILE}
    return 1
}

main "$@"