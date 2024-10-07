#!/bin/sh
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
MOUNT_SCRIPT_PATH=${DATA_BACKUP_AGENT_HOME}/script/mount_oper.sh
AGENT_FOLDER=${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient/ProtectClient-E
IP_RULE='^(2[0-4][0-9]|25[0-5]|1[0-9][0-9]|[1-9]?[0-9])(\.(2[0-4][0-9]|25[0-5]|1[0-9][0-9]|[1-9]?[0-9])){3}$'
AGENT_START_PORT=59570
AGENT_END_PORT=59600

if [ "${OS_TYPE}" = "SunOS" ]; then
    AWK=nawk
else
    AWK=awk
fi


check_paramters()
{
    if [ $# -ne 5 ];then
        log_echo "ERROR" "The script is asked to be five parameters." >> ${LOG_FILE}
        exit 1
    fi
    typeset logPath="$1"
    typeset startPort="$2"
    typeset endPort="$3"
    typeset agentIp="$4"
    typeset agentPort="$5"

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

    if [[ "${agentIp}" == "*['!'@#\$%^\&*()_+]*" ]] || [ $(echo ${agentIp} | grep -Evc "${IP_RULE}") -gt 0 ]; then
        log_echo "ERROR" "IP format is wrong, pls check"
        exit 1
    fi

    if [ ${agentPort} -lt ${AGENT_START_PORT} -o ${agentPort} -gt ${AGENT_END_PORT} ];then
        log_echo "ERROR" "The agentPort is wrong, please check it." >> ${LOG_FILE}
        exit 1
    fi
    log_echo "INFO" "The parameter verification is successful.." >> ${LOG_FILE}
}

# 如果是内置agent，通过挂载日志目录到插件conf目录，实现conf目录下文件可写
change_conf_path()
{
    echo "[$(date "+%Y-%m-%d %H:%M:%S")][INFO][ start to change conf path. ]" >> ${LOG_FILE}
    typeset pod_name=`cat ${AGENT_FOLDER}/conf/testcfg.tmp | grep PODE_NAME | awk -F "=" '{print $2}'`
    if [ -z ${pod_name} ];then
        echo "[$(date "+%Y-%m-%d %H:%M:%S")][INFO][ got empty pod name, this is external agent mode, no need to mount conf dir. ]" >> ${LOG_FILE}
        return 0
    fi
 
    mount | grep -q "${PLUGIN_INSTALL_PATH}/${PLUGIN_NAME}/conf"
    if [ $? -eq 0 ];then
        echo "[$(date "+%Y-%m-%d %H:%M:%S")][INFO][ the conf directory has been mounted. ]" >> ${LOG_FILE}
        return 0
    fi
    cd ${DATA_BACKUP_AGENT_HOME}/protectagent/${pod_name}/Plugins/
    mkdir -p ${PLUGIN_NAME}/conf
    cp -arf ${PLUGIN_INSTALL_PATH}/${PLUGIN_NAME}/conf/* ${PLUGIN_NAME}/conf/ >/dev/null 2>&1
    cp -arf ${PLUGIN_INSTALL_PATH}/${PLUGIN_NAME}/plugin_attribute_1.0.0.json ${PLUGIN_NAME}/ >/dev/null 2>&1
    sudo ${MOUNT_SCRIPT_PATH} mount_bind "${DATA_BACKUP_AGENT_HOME}/protectagent/${pod_name}/Plugins/${PLUGIN_NAME}/conf" "${PLUGIN_INSTALL_PATH}/${PLUGIN_NAME}/conf"
    sudo ${MOUNT_SCRIPT_PATH} mount_bind "${DATA_BACKUP_AGENT_HOME}/protectagent/${pod_name}/Plugins/${PLUGIN_NAME}/plugin_attribute_1.0.0.json" "${PLUGIN_INSTALL_PATH}/${PLUGIN_NAME}/plugin_attribute_1.0.0.json"
    chmod -R 770 ${PLUGIN_NAME}
    if [ $? -ne 0 ]; then
        echo "[$(date "+%Y-%m-%d %H:%M:%S")][ERROR][ mount conf directory failed, this will affect modifying log levels from PM. ]" >> ${LOG_FILE}
    fi
    echo "[$(date "+%Y-%m-%d %H:%M:%S")][INFO][ Has finished to change conf path. ]" >> ${LOG_FILE}
    return 0
}

check_plugin_exist()
{
    typeset executable_file="$1"
    if [ "${OS_TYPE}" = "AIX" ] || [ "${OS_TYPE}" = "SunOS" ]; then  # ps不支持-w选项
        typeset curPid=$(ps -ef | grep -E "./bin/${executable_file}" | grep -v start.sh | grep -v grep | ${AWK} '{print $2}')
    else
        typeset curPid=$(ps -efww | grep -E "./bin/${executable_file}\>" | grep -v start.sh | grep -v grep | ${AWK} '{print $2}')
    fi
    # 若插件进程存在，则先杀掉插件进程
    if [ -n "${curPid}" ]; then
        log_echo "WARNING" "Nas process already exist. Process id is ${curPid}, stop it first." >> ${LOG_FILE}
        if [ "${BACKUP_SCENE}" == "1" ] && [ "${PLUGIN_NAME}" == "VirtualizationPlugin" ]; then
        # internal agent use exrdadmin user to run and stop virtualizationplugin
            su - exrdadmin -s /bin/bash -c "kill -9 "${curPid}""
        else
            kill -9 ${curPid}
        fi
    fi
}

start_plugin()
{
    typeset logPath="$1"
    typeset startPort="$2"
    typeset endPort="$3"
    typeset agentIp="$4"
    typeset agentPort="$5"
    cd ${SCRIPT_PATH}/bin
    typeset executable_file=AgentPlugin
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

    typeset curPort=${startPort}
    for port in  $(seq ${startPort} ${endPort})
    do
        curPort=${port}
        netstat -an | grep -q "\<$port\>"
        if [ $? -ne 0 ];then
           break;
        fi
    done

    if [ "${PLUGIN_NAME}" == "VirtualizationPlugin" ] && [ "${BACKUP_SCENE}" == "1" ]; then
        # internal agent use exrdadmin user to run virtualizationplugin
        su - exrdadmin -s /bin/bash -c "export LD_LIBRARY_PATH=${AGENT_FOLDER}/bin:${SCRIPT_PATH}/lib/3rd:${SCRIPT_PATH}/lib/platform:${SCRIPT_PATH}/lib:${SCRIPT_PATH}/lib/ext:${SCRIPT_PATH}/lib:${SCRIPT_PATH}/lib/agent_sdk:${SCRIPT_PATH}/lib/service/:${SCRIPT_PATH}/lib/dme:${SCRIPT_PATH}/lib/dme/3rd:${SCRIPT_PATH}/lib/dme/platform:${LD_LIBRARY_PATH};${SCRIPT_PATH}/bin/${executable_file} "${logPath}" "${curPort}" "${endPort}" "${agentIp}" "${agentPort}" & cd - 2> /dev/null"
    else
        export LD_LIBRARY_PATH=${AGENT_FOLDER}/bin:${SCRIPT_PATH}/lib/3rd:${SCRIPT_PATH}/lib/platform:${SCRIPT_PATH}/lib:${SCRIPT_PATH}/lib/ext:${SCRIPT_PATH}/lib:${SCRIPT_PATH}/lib/agent_sdk:${SCRIPT_PATH}/lib/service/:${SCRIPT_PATH}/lib/dme:${SCRIPT_PATH}/lib/dme/3rd:${SCRIPT_PATH}/lib/dme/platform:${LD_LIBRARY_PATH}
        ${SCRIPT_PATH}/bin/${executable_file} "${logPath}" "${curPort}" "${endPort}" "${agentIp}" "${agentPort}"  &
        cd - 2> /dev/null
    fi

    i=0
    while ((i < 3))  # 查询进程号3次
    do
        sleep 1  # 等待进程启动
        if [ "${OS_TYPE}" = "AIX" ] || [ "${OS_TYPE}" = "SunOS" ]; then  # AIX ps不支持-w选项
            curPid=$(ps -ef | grep -E "./bin/${executable_file}" | grep -v start.sh | grep -v grep | ${AWK} '{print $2}')
        else
            curPid=$(ps -efww | grep -E "./bin/${executable_file}\>" | grep -v start.sh | grep -v grep | ${AWK} '{print $2}')
        fi
        if [ -n "${curPid}" ]; then
            log_echo "INFO" "Start nas process sucessfully. Process id is ${curPid}" >> ${LOG_FILE}
            return 0
        fi
        let i+=1
    done
    log_echo "ERROR" "Failed to start the nas process." >> ${LOG_FILE}
    return 1
}

# 虚拟化外置代理进入python虚拟环境
enter_virtual()
{
    if [ "${PLUGIN_NAME}" != "VirtualizationPlugin" ] || [ "${BACKUP_SCENE}" == "1" ]; then
        echo "INFO" "The current env is internal agent or plugin is not VirtualizationPlugin. No need enter virtual."
        return 0
    fi
    ${PLUGIN_INSTALL_PATH}/VirtualizationPlugin
    INSTALL_PATH="${PLUGIN_INSTALL_PATH}/VirtualizationPlugin/install"
    PYTHON_PATH="${PLUGIN_INSTALL_PATH}/VirtualizationPlugin/install/build_python"
    export LD_LIBRARY_PATH=${PYTHON_PATH}/lib:${INSTALL_PATH}:${LD_LIBRARY_PATH}
    export VIRTUALENVWRAPPER_PYTHON=${PYTHON_PATH}/bin/python3.10
    export VIRTUALENVWRAPPER_VIRTUALENV=${PYTHON_PATH}/bin/virtualenv
    export WORKON_HOME=${INSTALL_PATH}/.virtualenvs
    source ${PYTHON_PATH}/bin/virtualenvwrapper.sh
    workon virtual_plugin_env
    if [ $? -ne 0 ];then
        echo "ERROR" "Enter virtual env failed."
        return 1
    fi
    echo "INFO" "Enter virtual env success."
    return 0
}

main()
{
    PLUGIN_NAME=$(get_plugin_name)
    if [ $? -ne 0 ]; then
        exit 1
    fi
    LOG_FILE_PREFIX=$(get_plugin_log_path)
    if [ $? -ne 0 ]; then
        exit 1
    fi
    BACKUP_SCENE=`cat /opt/DataBackup/ProtectClient/ProtectClient-E/conf/testcfg.tmp | grep "BACKUP_SCENE" | cut -d'=' -f2`

    LOG_FILE=${LOG_FILE_PREFIX}/start.log
    change_conf_path
    check_paramters "$@"
    if [ $? -ne 0 ];then
        return 1
    fi
    enter_virtual
    start_plugin "$@"
    if [ $? -eq 0 ]; then
        echo "[$(date "+%Y-%m-%d %H:%M:%S")][INFO][ Start agent plugin ${PLUGIN_NAME} successfully ]" >> ${LOG_FILE}
        return 0
    fi
    echo "[$(date "+%Y-%m-%d %H:%M:%S")][ERROR][ Failed to start agent plugin ${PLUGIN_NAME}. ]" >> ${LOG_FILE}
    return 1
}

main "$@"
