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
AGENT_USER=rdadmin
SHELL_TYPE_SH="/bin/sh"
SYS_NAME=`uname -s`

# get agent install path
if [ -z "${DATA_BACKUP_AGENT_HOME}" ];then
    DATA_BACKUP_AGENT_HOME="/opt"
fi

if [ "${SYS_NAME}" = "SunOS" ]; then
    MYAWK=nawk
else
    MYAWK=awk
fi

if [ "`ls -al /bin/sh | ${MYAWK} '{print $NF}'`" = "bash" ]; then
    EXPORT_ENV="if [ -f ~/.profile ]; then source ~/.profile; fi;"
else
    EXPORT_ENV=""
fi

PLUGIN_INSTALL_PATH="${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient/Plugins"
MOUNT_SCRIPT_PATH=${DATA_BACKUP_AGENT_HOME}/script/mount_oper.sh

AGENT_FOLDER="${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient/ProtectClient-E"
IP_RULE='^(2[0-4][0-9]|25[0-5]|1[0-9][0-9]|[1-9]?[0-9])(\.(2[0-4][0-9]|25[0-5]|1[0-9][0-9]|[1-9]?[0-9])){3}$'
AGENT_START_PORT=59570
AGENT_END_PORT=59600

SUExecCmd()
{
    cmd=$1
    result=""
    if [ "${SYS_NAME}" = "SunOS" ] || [ "${SYS_NAME}" = "AIX" ]; then
        result=`su - ${AGENT_USER} -c "${EXPORT_ENV}${cmd}"`
    else
        result=`su -m ${AGENT_USER} -s ${SHELL_TYPE_SH} -c "${EXPORT_ENV}${cmd}"`
    fi

    echo $result
}

check_paramters()
{
    if [ $# -ne 5 ];then
        log_echo "ERROR" "The script is asked to be five parameters." >> ${LOG_FILE}
        exit 1
    fi
    local logPath="$1"
    local startPort="$2"
    local endPort="$3"
    local agentIp="$4"
    local agentPort="$5"

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

    VALID_CHECK=$(echo ${agentIp}|awk -F. '$1<=255&&$2<=255&&$3<=255&&$4<=255{print "yes"}')
    if echo ${agentIp}|grep -E "${IP_RULE}">/dev/null; then
        if [ ${VALID_CHECK:-no} != "yes" ]; then
            log_echo "IP ${agentIp} not available!"
            exit 1
        fi
    else
        log_echo "IP ${agentIp} format error!"
        exit 1
    fi

    if [ ${agentPort} -lt ${AGENT_START_PORT} -o ${agentPort} -gt ${AGENT_END_PORT} ];then
        log_echo "ERROR" "The agentPort is wrong, please check it." >> ${LOG_FILE}
        exit 1
    fi
    log_echo "INFO" "The parameter verification is successful.." >> ${LOG_FILE}
}

check_plugin_exist()
{
    local executable_file="$1"
    local curPid=$(ps -efww | grep -E "./bin/${executable_file}\>" | grep -v start.sh | grep -v grep | awk '{print $2}')
    # 若插件进程存在，则先杀掉插件进程
    if [ -n "${curPid}" ]; then
        log_echo "WARNING" "Nas process already exist. Process id is ${curPid}, stop it first." >> ${LOG_FILE}
        kill -9 ${curPid}
    fi
}

enter_virtual()
{
    if [ ${SYS_NAME} = "AIX" ]; then
        export PYTHONHASHSEED=1
        version=`oslevel | grep "^6.*"`
        if [ $? -eq 0 -o -n "${version}" ]; then
            log_echo "INFO" "The current operating system version is AIX6." >> ${LOG_FILE}
            return 0
        fi
    fi
    if [ "X${BACKUP_SCENE}" = "X1" ];then
        log_echo "INFO" "The current env is internal agent. Python does not need to be installed." >> ${LOG_FILE}
        return 0
    fi
    INSTALL_PATH="${SCRIPT_PATH}/install"
    PYTHON_PATH="${SCRIPT_PATH}/install/build_python"
    export LD_LIBRARY_PATH=${PYTHON_PATH}/lib:${INSTALL_PATH}:${LD_LIBRARY_PATH}
    export VIRTUALENVWRAPPER_PYTHON=${PYTHON_PATH}/bin/python3.10
    export VIRTUALENVWRAPPER_VIRTUALENV=${PYTHON_PATH}/bin/virtualenv
    export WORKON_HOME=${INSTALL_PATH}/.virtualenvs
    source ${PYTHON_PATH}/bin/virtualenvwrapper.sh
    workon plugin_env
    if [ $? -ne 0 ];then
        log_echo "ERROR" "Enter virtual env failed." >> ${LOG_FILE}
        return 1
    fi
    log_echo "INFO" "Enter virtual env success." >> ${LOG_FILE}
    return 0
}

set_ulimit()
{
    cur_ulimit=$(ulimit -n)
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
    ulimitSoft=$(ulimit -n)
    ulimitHard=$(ulimit -Hn)
    log_echo "INFO" "Set ulimit soft ${ulimitSoft}, ulimit hard ${ulimitHard}" >> ${LOG_FILE}
}

start_plugin()
{
    local logPath="$1"
    local startPort="$2"
    local endPort="$3"
    local agentIp="$4"
    local agentPort="$5"
    cd ${SCRIPT_PATH}/bin
    local executable_file=AgentPlugin
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

    local curPort=${startPort}
    for port in  $(seq ${startPort} ${endPort})
    do
        curPort=${port}
        netstat -an | grep -q "\<$port\>"
        if [ $? -ne 0 ];then
           break;
        fi
    done

    export LD_LIBRARY_PATH=${AGENT_FOLDER}/bin:${SCRIPT_PATH}/lib/3rd:${SCRIPT_PATH}/lib/platform:${SCRIPT_PATH}/lib:${SCRIPT_PATH}/lib/ext:${SCRIPT_PATH}/lib:${SCRIPT_PATH}/lib/agent_sdk:${SCRIPT_PATH}/lib/service/:${SCRIPT_PATH}/lib/dme:${SCRIPT_PATH}/lib/dme/3rd:${SCRIPT_PATH}/lib/dme/platform:${LD_LIBRARY_PATH}

    set_ulimit

    ${SCRIPT_PATH}/bin/${executable_file} "${logPath}" "${curPort}" "${endPort}" "${agentIp}" "${agentPort}"  &
    cd - 2> /dev/null
    i=0
    while ((i < 3))  # 查询进程号3次
    do
        sleep 1  # 等待进程启动
        if [ "${OS_TYPE}" = "AIX" ]; then  # AIX ps不支持-w选项
            local curPid=$(ps -ef | grep -E "./bin/${executable_file}" | grep -v start.sh | grep -v grep | awk '{print $2}')
        else
            local curPid=$(ps -efw | grep -E "./bin/${executable_file}\>" | grep -v start.sh | grep -v grep | awk '{print $2}')
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

# 如果是内置agent，通过挂载日志目录到插件conf目录，实现conf目录下文件可写
change_internal_plugin_path()
{
    CONF_MOUNT_FALGS=0
    TMP_MOUNT_FALGS=0
    STMP_MOUNT_FALGS=0
    log_echo "INFO" "Start to change internal plugin path." >> ${LOG_FILE}
    if [ "X${BACKUP_SCENE}" = "X0" ];then
        log_echo "INFO" "This is external agent mode, no need to mount conf dir." >> ${LOG_FILE}
        return 0
    fi

    pod_name=`SUExecCmd "cat ${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient/ProtectClient-E/conf/testcfg.tmp | grep PODE_NAME"`
    pod_name=`echo ${pod_name} | awk -F "=" '{print $2}'`

    mount | grep -q "${PLUGIN_INSTALL_PATH}/${PLUGIN_NAME}/conf"
    if [ $? -eq 0 ]; then
        log_echo "INFO" "The conf directory has been mounted." >> ${LOG_FILE}
        CONF_MOUNT_FALGS=1
    fi
    mount | grep -q "${PLUGIN_INSTALL_PATH}/${PLUGIN_NAME}/tmp"
    if [ $? -eq 0 ]; then
        log_echo "INFO" "The plugin tmp directory has been mounted." >> ${LOG_FILE}
        TMP_MOUNT_FALGS=1
    fi
    mount | grep -q "${PLUGIN_INSTALL_PATH}/${PLUGIN_NAME}/stmp"
    if [ $? -eq 0 ]; then
        log_echo "INFO" "The plugin stmp directory has been mounted." >> ${LOG_FILE}
        STMP_MOUNT_FALGS=1
    fi
    cd ${DATA_BACKUP_AGENT_HOME}/protectagent/${pod_name}/Plugins/
    if [ ${CONF_MOUNT_FALGS} -eq 0 ]; then
        mkdir -p ${PLUGIN_NAME}/conf
        cp -rf ${PLUGIN_INSTALL_PATH}/${PLUGIN_NAME}/conf/* ${PLUGIN_NAME}/conf/ >/dev/null 2>&1
        sudo ${MOUNT_SCRIPT_PATH} mount_bind "${DATA_BACKUP_AGENT_HOME}/protectagent/${pod_name}/Plugins/${PLUGIN_NAME}/conf" "${PLUGIN_INSTALL_PATH}/${PLUGIN_NAME}/conf"
        if [ $? -ne 0 ]; then
            log_echo "ERROR" "Mount conf directory failed, this will affect modifying log levels from PM." >> ${LOG_FILE}
        fi
    fi

    if [ ${TMP_MOUNT_FALGS} -eq 0 ]; then
        mkdir -p ${PLUGIN_NAME}/tmp
        sudo ${MOUNT_SCRIPT_PATH} mount_bind "${DATA_BACKUP_AGENT_HOME}/protectagent/${pod_name}/Plugins/${PLUGIN_NAME}/tmp" "${PLUGIN_INSTALL_PATH}/${PLUGIN_NAME}/tmp"
        if [ $? -ne 0 ]; then
            log_echo "ERROR" "Mount plugin tmp directory failed." >> ${LOG_FILE}
            exit 1
        fi
    fi

    if [ ${STMP_MOUNT_FALGS} -eq 0 ]; then
        mkdir -p ${PLUGIN_NAME}/stmp
        sudo ${MOUNT_SCRIPT_PATH} mount_bind "${DATA_BACKUP_AGENT_HOME}/protectagent/${pod_name}/Plugins/${PLUGIN_NAME}/stmp" "${PLUGIN_INSTALL_PATH}/${PLUGIN_NAME}/stmp"
        if [ $? -ne 0 ]; then
            log_echo "ERROR" "Mount plugin stmp directory failed." >> ${LOG_FILE}
            exit 1
        fi
    fi

    chown -R root:nobody ${PLUGIN_NAME}
    chmod -R 700 ${PLUGIN_NAME}
    chmod 700 ${PLUGIN_NAME}/tmp
    chmod 700 ${PLUGIN_NAME}/stmp
    log_echo "INFO" "Has finished to change internal plugin path." >> ${LOG_FILE}
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
    BACKUP_SCENE=`SUExecCmd "cat ${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient/ProtectClient-E/conf/testcfg.tmp | grep "BACKUP_SCENE" | cut -d'=' -f2"`

    LOG_FILE=${LOG_FILE_PREFIX}/start.log
    check_paramters "$@"
    if [ $? -ne 0 ];then
        return 1
    fi
    enter_virtual
    change_internal_plugin_path
    if [ $? -ne 0 ]; then
        exit 1
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
