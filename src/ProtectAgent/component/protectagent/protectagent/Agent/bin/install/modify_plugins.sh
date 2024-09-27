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

##############################################################
# The function of this script is to MODIFY the client agent.
##############################################################
SYS_NAME=`uname -s`
if [ "$SYS_NAME" = "SunOS" ]; then
    MYAWK=nawk
    AGENT_USER_PATH="/export/home"
else
    MYAWK=awk
    AGENT_USER_PATH="/home"
fi

TMP_DATA_BACKUP_AGENT_HOME=`cat /etc/profile | grep "export DATA_BACKUP_AGENT_HOME=" |${MYAWK} -F "=" '{print $NF}'`
if [ -n "${TMP_DATA_BACKUP_AGENT_HOME}" ]; then
    . /etc/profile
else
    DATA_BACKUP_AGENT_HOME=/opt
    export DATA_BACKUP_AGENT_HOME
fi

PRODUCT_NAME="DataBackup ProtectAgent"
AGENT_USER=rdadmin
AGENT_GROUP=rdadmin

# modify dir
CURRENT_PATH=`dirname $0` && cd $CURRENT_PATH && CURRENT_PATH=`pwd`
BACKUP_LOG="/var/log/ProtectAgent/log"
AGENT_ROOT_PATH="${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient"
PLUGIN_DIR="${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient/Plugins"
AGENT_BIN_PATH="${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient/ProtectClient-E"
LOG_FILE_PATH=${CURRENT_PATH}
LOG_FILE_NAME="${CURRENT_PATH}/modify.log"
PUSH_MODIFY_PACKAGE_PATH="${DATA_BACKUP_AGENT_HOME}/modify"
MODIFY_BACKUP_PATH="${DATA_BACKUP_AGENT_HOME}/AgentModify"
LOG_ERR_FILE_STUB="${AGENT_ROOT_PATH}/ProtectClient-E/tmp/errormsg.log"
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${AGENT_BIN_PATH}/bin/
export LD_LIBRARY_PATH

SYS_ARCH=""
SYS_BIT=""
SHELL_TYPE_SH="/bin/sh"

export INSTALL_PACKAGE_PATH=${CURRENT_PATH}

##### MODIFY errcode start #####

ERR_MODIFY_FAIL_BACKUP=1577209880
ERR_NO_SUPPORT_PLUGIN=2
ERR_INSTALL_PLUGIN_RETCODE=71

##### MODIFY errcode end #####

if [ "`ls -al /bin/sh | ${MYAWK} '{print $NF}'`" = "bash" ]; then
    EXPORT_ENV="if [ -f ~/.profile ]; then . ~/.profile; fi;"
else
    EXPORT_ENV=""
fi

########################################################################################
# Function Definition
########################################################################################
Log()
{
    if [ ! -f "${LOG_FILE_NAME}" ]; then
        touch "${LOG_FILE_NAME}"
        CHMOD 600 "${LOG_FILE_NAME}"
    fi

    DATE=`date +%y-%m-%d--%H:%M:%S`
    if [ "SunOS" = "$SYS_NAME" ]; then
        command -v whoami >/dev/null
        if [ $? -eq 0 ]; then
            USER_NAME=`whoami`
        else
            USER_NAME=`/usr/ucb/whoami`
        fi
    else
        USER_NAME=`whoami`
    fi

    echo "[${DATE}][$$][${USER_NAME}] $1" >> "${LOG_FILE_NAME}"
}

PrintHelp()
{
    printf "\033[31mValid parameter, params:\033[0m\n"
    printf "mode: push installation\n"
    printf "eg:\n"
    printf "\033[31msh modify.sh\033[0m\n"
    printf "\033[31msh modify.sh -mode push\033[0m\n"
}

CheckInputParams()
{
    if [ $# -eq 0 ]; then
        return 0
    elif [ $# -eq 1 ] && [ $1 = "-h" ]; then
        PrintHelp
        return 1
    elif [ $# -eq 2 ] && [ $1 = "-mode" ] && [ $2 = "push" ]; then
        MODIFY_MODE=push
        return 0
    else
        PrintHelp
        return 1
    fi
}

CHMOD()
{
    filePath=
    arg2=
    arg=
    if [ $# -eq 3 ]; then
        arg=$1
        arg2=$2
        filePath=$3
    elif [ $# -eq 2 ]; then
        filePath=$2
        arg2=$1
    fi
    if [ -L "$filePath" ]; then
        Log "source file  is a link file can not copy."
        return
    fi
    chmod $arg $arg2 $filePath  >>${LOG_FILE_NAME} 2>&1
}

# echo warning message in purple color
ShowWarning()
{
    printf "\\033[1;35m$1\\033[0m\n"
}

CP()
{
    # High to Low, Determining Linked Files
    eval LAST_PARAM=\$$#
    TARGET_FILE=${LAST_PARAM}
    if [ -L "${TARGET_FILE}" ]; then
        echo "The target file is a linked file. Exit the current process."
        Log "The target file is a linked file. Exit the current process."
        ExitHandle 1
    fi

    if [ "$SYS_NAME" = "SunOS" ]; then
        cmd="cp -R -P $*"
    else
        cmd="cp -d $*"
    fi
    ${cmd} 
}

LogError()
{
    #$1=errorInfo,$2=errorCode, $3=errorDetailParam
    LogErr "$1" "$2" "job_log_agent_storage_update_install_fail_label" "$3"
}

LogErr() 
{
    #$1=errorInfo,$2=errorCode,$3=errorLable,$4=errorDetailParam
    Log "[ERR] $1"
    if [ $# -ge 3 ]; then
        LogErrDetail "$2" "$3" "$4"
    fi
}

LogErrDetail()
{
    if [ ! -f "${LOG_ERR_FILE_STUB}" ]; then
        touch "${LOG_ERR_FILE_STUB}"
        chmod 604 "${LOG_ERR_FILE_STUB}" 
    fi
    echo "logDetail=$1" > ${LOG_ERR_FILE_STUB}
    echo "logInfo=$2" >> ${LOG_ERR_FILE_STUB}
    echo "logDetailParam=$3" >> ${LOG_ERR_FILE_STUB}
}

BackupAgentPluginPkg()
{
    CP -a -r -f -p -P "${PLUGIN_DIR}" "${MODIFY_BACKUP_PATH}" >/dev/null 2>&1
}

BackupLog()
{
    if [ ! -d "${BACKUP_LOG}" ]; then
        mkdir -p "${BACKUP_LOG}"
    fi
    CP -a -f -p $LOG_FILE_NAME /var/log/ProtectAgent/log/
}

CleanPkg()
{
    if [ -d "${PUSH_MODIFY_PACKAGE_PATH}" ]; then
        rm -rf "${PUSH_MODIFY_PACKAGE_PATH}"
    fi 
    if [ -d "${MODIFY_BACKUP_PATH}" ]; then
        rm -rf "${MODIFY_BACKUP_PATH}"
    fi 
}

StartAgent()
{
    cd ${AGENT_ROOT_PATH}/
    chmod +x start.sh
    ./start.sh >/dev/null 2>&1
    if [ $? -eq 0 ]; then
        Log "MIGRATE: The DataBackup ProtectAgent service is successfully started."
    else
        echo "The DataBackup ProtectAgent service fails to be started."
        Log "MIGRATE: The DataBackup ProtectAgent service fails to be started."
        return 1
    fi
}

RegisterHost()
{
    if [ "${SYS_NAME}" = "SunOS" ] || [ "${SYS_NAME}" = "AIX" ]; then
        su - ${AGENT_USER} -c "${EXPORT_ENV}${AGENT_ROOT_PATH}/ProtectClient-E/bin/agentcli registerHost RegisterHost"
    else
        su - ${AGENT_USER} -s ${SHELL_TYPE_SH} -c "${EXPORT_ENV}${AGENT_ROOT_PATH}/ProtectClient-E/bin/agentcli registerHost RegisterHost"
    fi
    if [ "$?" != 0 ]; then
        Log "MIGRATE: Register host to ProtectManager failed."
        echo "The host cannot be registered to the ProtectManager."
        return 1
    fi
    Log "Modify: Register host to ProtectManager success."
}

RollBackAgentPlugin()
{
    Log "Modify: Start to rollback agent plugin."
    echo "Start to rollback agent plugin."
    mv -f "${MODIFY_BACKUP_PATH}"/* "${PLUGIN_DIR}/"
    StartAgent
    RegisterHost
    echo "Rollback agent success."
    Log "MIGRATE: Rollback agent success."
    CleanPkg
    return
}

SUExecCmd()
{
    cmd=$1
    if [ "${SYS_NAME}" = "SunOS" ] || [ "${SYS_NAME}" = "AIX" ]; then
        su - ${AGENT_USER} -c "${EXPORT_ENV}${cmd}" >>${LOG_FILE_NAME} 2>&1
    else
        su - ${AGENT_USER} -s ${SHELL_TYPE_SH} -c "${EXPORT_ENV}${cmd}" >>${LOG_FILE_NAME} 2>&1
    fi

    return $?
}

VerifySpecialChar()
{
    SPECIAL_CHARS="[\`$;|&<>\!]"
    for arg in $*
    do
        echo "$arg" | grep ${SPECIAL_CHARS} 1>/dev/null 2>&1
        if [ $? -eq 0 ]; then
            Log "The variable[$arg] cannot contain special characters."
            ExitHandle 1
        fi
    done
}

SUExecCmdWithOutput()
{
    cmd=$1
    if [ "${SYS_NAME}" = "SunOS" ] || [ "${SYS_NAME}" = "AIX" ]; then
        output=`su - ${AGENT_USER} -c "${EXPORT_ENV}${cmd}"` 
    else
        output=`su -m ${AGENT_USER} -s ${SHELL_TYPE_SH} -c "${EXPORT_ENV}${cmd}"`
    fi
    ret=$?
    VerifySpecialChar "$output"
    echo $output
    return $ret
}

CompareUpdateVersion()
{
    # obtain old version of agent
    OLD_UPDATE_VERSION=`cd "${AGENT_ROOT_PATH}/ProtectClient-E/conf"; find ./ -name version|xargs sed -n "3,3p"`
    if [ -d "${CURRENT_PATH}/ProtectClient-e" ]; then
        cd "${CURRENT_PATH}/ProtectClient-e"
    else
        echo "Not found ProtectClient-e directory."
        return 1
    fi

    # obtain new version of agent
    mkdir -p ${CURRENT_PATH}/ProtectClient-e/DIR_TMP
    CHMOD 750 ${CURRENT_PATH}/ProtectClient-e/DIR_TMP

    if [ $SYS_NAME = "Linux" ]; then
        SYS_ARCH=`arch`
    else
        SYS_ARCH=""
    fi

    system_name=""
    if [ $SYS_NAME = "Linux" ]; then
        cat /etc/system-release | grep CentOS >>/dev/null 2>&1
        if [ $? -eq 0 ]; then
            system_name="Centos"
        fi
    fi

    if [ "${SYS_ARCH}" = "aarch32" ] || [ "${SYS_ARCH}" = "aarch64" ] || [ "${SYS_ARCH}" = "x86_64" ] || [ "${SYS_ARCH}" = "x86" ]; then
        if [ "$system_name" = "Centos" ] && [ "${SYS_ARCH}" = "aarch64" ]; then
            PAC_NAME_TMP=`ls $pwd | grep "tar.xz" | grep "$SYS_NAME" | grep "${SYS_ARCH}" | grep Centos`
        else
            PAC_NAME_TMP=`ls $pwd | grep "tar.xz" | grep "$SYS_NAME" | grep "${SYS_ARCH}" | grep -v Centos-aarch64`
        fi
    else
        PAC_NAME_TMP=`ls $pwd | grep "tar.xz" | grep "$SYS_NAME"`
    fi

    if [ "$SYS_NAME" = "Linux" ]; then
        tar -xf $PAC_NAME_TMP -C ${CURRENT_PATH}/ProtectClient-e/DIR_TMP
    elif [ "$SYS_NAME" = "SunOS" ]; then
    # 对SunOS进行单独适配
        TEM_PATH=`pwd`
        PAC_NAME_TMP=`ls $pwd | grep "tar.gz" | grep "$SYS_NAME"`
        # 判断包是否获取到安装包
        if [ -z "${PAC_NAME_TMP}" ]; then
            Log "Not find package ${PAC_NAME_TMP}."
            return 1
        fi
        cp $PAC_NAME_TMP ${CURRENT_PATH}/ProtectClient-e/DIR_TMP
        cd ${CURRENT_PATH}/ProtectClient-e/DIR_TMP
        gzip -d ${PAC_NAME_TMP}
        PACK_NAME=`ls | grep protect*.tar`
        tar -xf ${PACK_NAME}
        cd ${TEM_PATH}
    else    
        TEM_PATH=`pwd`
        cp $PAC_NAME_TMP ${CURRENT_PATH}/ProtectClient-e/DIR_TMP
        cd ${CURRENT_PATH}/ProtectClient-e/DIR_TMP
        xz -d ${PAC_NAME_TMP}
        PACK_NAME=${PAC_NAME_TMP%%.xz}
        tar -xf ${PACK_NAME}
        cd $TEM_PATH
    fi

    NEW_UPDATE_VERSION=`find ${CURRENT_PATH}/ProtectClient-e/DIR_TMP -name version |xargs sed -n "3,3p"`
    if [ -d "${CURRENT_PATH}/ProtectClient-e/DIR_TMP" ]; then
        rm -rf "${CURRENT_PATH}/ProtectClient-e/DIR_TMP"
    fi

    cd $CURRENT_PATH
    # compare the version
    if [ $NEW_UPDATE_VERSION -ge $OLD_UPDATE_VERSION ]; then
        Log "Old version[$OLD_UPDATE_VERSION] <= new version[$NEW_UPDATE_VERSION], continue to upgrade."
        return 0
    else
        Log "Old version is not equal the new version, failed to modify."
        return 1
    fi
}

BackupInstalledPac()
{
    if [ -d "$MODIFY_BACKUP_PATH" ]; then
        rm -rf "$MODIFY_BACKUP_PATH"
    fi
    mkdir $MODIFY_BACKUP_PATH
    #欧拉系统升级适配
    chmod 755 $MODIFY_BACKUP_PATH

    # save CRL status
    return_get_CRL_status=`cat "${AGENT_ROOT_PATH}/ProtectClient-E/conf/testcfg.tmp" | grep "CERTIFICATE_REVOCATION" | ${MYAWK} -F '=' '{print $2}'`
    if [ "${return_get_CRL_status}" == "1" ]; then
        CRL_STATUS=1;
    fi
    Log "CRL status is ${CRL_STATUS}."

    if [ ! -d "$AGENT_ROOT_PATH" ]; then
        Log "Not install the DataBackup ProtectAgent, failed to upgrade."
        return 1
    else
        return 0
    fi
}

ClearUselessPackage()
{
    if [ $# -eq 0 ]; then
        rm -rf "$MODIFY_BACKUP_PATH"
        # clear push pkg
        if [ "${MODIFY_MODE}" = "push" ]; then
            [ -d "${PUSH_MODIFY_PACKAGE_PATH}" ] && rm -rf "${PUSH_MODIFY_PACKAGE_PATH}"
        fi
    else
        Log "Clear package wrong param."
    fi

    return 0
}

CopyLogFile()
{
    DST_DIR=$1
    # Directory-based judgment is used to ensure interface universality.
    if [ -d "${DST_DIR}/ProtectClient-E/slog" ]; then
        [ -f "${LOG_FILE_PATH}/modify.log" ] && mv "${LOG_FILE_PATH}/modify.log" "${DST_DIR}/ProtectClient-E/slog"
        # copy push MODIFY logs
        if [ "${MODIFY_MODE}" = "push" ]; then
            [ -f "${PUSH_MODIFY_PACKAGE_PATH}/modify_pre.log" ] && mv "${PUSH_MODIFY_PACKAGE_PATH}/modify_pre.log" "${DST_DIR}/ProtectClient-E/slog"
        fi
    elif [ -d "${DST_DIR}/ProtectClient-E" ]; then
        mkdir -p "${DST_DIR}/ProtectClient-E/slog"
        [ -f "${LOG_FILE_PATH}/modify.log" ] && mv "${LOG_FILE_PATH}/modify.log" "${DST_DIR}/ProtectClient-E/slog"
        # copy push MODIFY logs
        if [ "${MODIFY_MODE}" = "push" ]; then
            [ -f "${PUSH_MODIFY_PACKAGE_PATH}/modify_pre.log" ] && mv "${PUSH_MODIFY_PACKAGE_PATH}/modify_pre.log" "${DST_DIR}/ProtectClient-E/slog"
        fi
    fi
}

ExitHandle()
{
    EXIT_CODE=$1

    # add log to MODIFY.log
    if [ ${EXIT_CODE} -ne 0 ]; then
        echo "Failed to modify the Agent on the host"
        Log "Failed to modify the Agent on the host."
    else
        Log "DataBackup ProtectAgent is successfully modify on the host."
    fi

    if [ -f "${CURRENT_PATH}/modify.log" ]; then
        cat "${CURRENT_PATH}/slog/modify.log" >> ${LOG_FILE_NAME}
    fi

    if [ "${EXIT_CODE}" != "0" ]; then # UPGRADE_STATUS: 0-failure 1-success 3-intermediate 8-abnormal 9-initial
        if [ "${SYS_NAME}" = "SunOS" ] || [ "${SYS_NAME}" = "AIX" ]; then
            su - ${AGENT_USER} -c "sed \"/MODIFY_STATUS/s/.*/MODIFY_STATUS=0/g\" ${AGENT_ROOT_PATH}/ProtectClient-E/conf/testcfg.tmp > ${AGENT_ROOT_PATH}/ProtectClient-E/conf/testcfg.tmp.bak"
            su - ${AGENT_USER} -c "mv -f "${AGENT_ROOT_PATH}/ProtectClient-E/conf/testcfg.tmp.bak" "${AGENT_ROOT_PATH}/ProtectClient-E/conf/testcfg.tmp""
        else
            su - ${AGENT_USER} -s ${SHELL_TYPE_SH} -c "sed -i \"/MODIFY_STATUS/s/.*/MODIFY_STATUS=0/g\" ${AGENT_ROOT_PATH}/ProtectClient-E/conf/testcfg.tmp"
        fi
        RollBackAgentPlugin
        CHMOD 600 "${OLD_INSTALL_PATH}/ProtectClient-E/conf/testcfg.tmp"
    fi
    # clear upgrade package
    ClearUselessPackage
    exit ${EXIT_CODE}

}

CheckIsInstallAgent()
{
    id ${AGENT_USER} >/dev/null 2>&1
    if [ $? -eq 0 ] && [ -d "${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient" ] ; then
        echo "The host is install agent"
        Log "The host is install agent"
    else
        Log "The host is not install agent"
        ExitHandle 1
    fi
}

BackUpCurAgentInfo()
{
    echo "Begin to backup DataBackup ProtectAgent Plugin."
    Log "Modify: Begin to back up DataBackup ProtectAgent."
    if [ -d "${MODIFY_BACKUP_PATH}" ]; then
        rm -rf "${MODIFY_BACKUP_PATH}"
    fi
    mkdir "$MODIFY_BACKUP_PATH"
    CHMOD 755 "$MODIFY_BACKUP_PATH"
    BackupAgentPluginPkg
    Log "MIGRATE: The installed DataBackup ProtectAgent are backed up successfully."
    echo "The installed DataBackup ProtectAgent are backed up successfully."
}

UninstallPlugins()
{
    if [ ! -d "${PLUGIN_DIR}" ]; then
        Log "Plugins path is not exist."
        return 0
    fi

    Log "UninstallPlugins start."
    echo "UninstallPlugins start."
    tmpPath=`pwd`
    #1. Get all plugins
    cd ${PLUGIN_DIR}
    pluginNames=`ls -l . | grep "Plugin" | ${MYAWK} '/^d/ {print $NF}'`

    if [ -z "${pluginNames}" ];then
        Log "Plugins is not install."
        return 0
    fi
    #2. Uninstalling plugins
    for pluginName in $pluginNames; do
        Log "Uninstalling $pluginName."
        tmpPluginPath=`pwd`
        # uninstall plugin
        cd ${pluginName}
        ./uninstall.sh >> ${LOG_FILE_NAME} 2>&1
        RET_CODE=$?
        if [ ${RET_CODE} -ne 0 ]; then
            Log "Uninstall $pluginName fail."
            return 1
        fi
        cd ${tmpPluginPath}
    done
    # 确保插件卸载之后插件目录删除
    if [ -d "${PLUGIN_DIR}" ]; then
        rm -rf ${PLUGIN_DIR}/*
    fi
    cd ${tmpPath}
}


ReplacePack()
{
    CP -r -p ${CURRENT_PATH}/ProtectClient-e/Plugins/* ${PLUGIN_DIR}
}

ModifyParamAndRegister()
{
    Log "Start to modify plugin conf."
    OLD_APPLICATION_INFO=`SUExecCmdWithOutput "cat ${AGENT_BIN_PATH}/conf/testcfg.tmp | grep "APPLICATION_INFO" | ${MYAWK} -F '=' '{print $NF}'"`
    if [ "${SYS_NAME}" = "SunOS" ] || [ "${SYS_NAME}" = "AIX" ]; then
        su - ${AGENT_USER} -c "sed '/^APPLICATION_INFO/d' ${AGENT_BIN_PATH}/conf/testcfg.tmp > ${AGENT_ROOT_PATH}/ProtectClient-E/conf/testcfg.tmp.bak"
        su - ${AGENT_USER} -c "mv -f "${AGENT_BIN_PATH}/conf/testcfg.tmp.bak" "${AGENT_ROOT_PATH}/ProtectClient-E/conf/testcfg.tmp""
    else
        su - ${AGENT_USER} -s ${SHELL_TYPE_SH} -c "sed -i '/^APPLICATION_INFO/d' ${AGENT_BIN_PATH}/conf/testcfg.tmp"
    fi
    NEW_APPLICATION_INFO=`cat ${CURRENT_PATH}/conf/client.conf | grep "application_info" | ${MYAWK} -F '=' '{print $NF}'`
    echo APPLICATION_INFO=${NEW_APPLICATION_INFO} | tr -d '\r' >> ${AGENT_BIN_PATH}/conf/testcfg.tmp
    # success
    if [ "${SYS_NAME}" = "SunOS" ] || [ "${SYS_NAME}" = "AIX" ]; then
        su - ${AGENT_USER} -c "sed \"/MODIFY_STATUS/s/.*/MODIFY_STATUS=1/g\" ${AGENT_ROOT_PATH}/ProtectClient-E/conf/testcfg.tmp > ${AGENT_ROOT_PATH}/ProtectClient-E/conf/testcfg.tmp.bak"
        su - ${AGENT_USER} -c "mv -f "${AGENT_ROOT_PATH}/ProtectClient-E/conf/testcfg.tmp.bak" "${AGENT_ROOT_PATH}/ProtectClient-E/conf/testcfg.tmp""
    else
        su - ${AGENT_USER} -s ${SHELL_TYPE_SH} -c "sed -i \"/MODIFY_STATUS/s/.*/MODIFY_STATUS=1/g\" ${AGENT_ROOT_PATH}/ProtectClient-E/conf/testcfg.tmp"
    fi
    CHMOD 600 "${AGENT_ROOT_PATH}/ProtectClient-E/conf/testcfg.tmp"
    # 修改成功反向注册修改插件
    RegisterHost
}

StopAgent()
{
    cd "${AGENT_ROOT_PATH}"
    ./stop.sh >/dev/null 2>&1
    if [ $? -eq 0 ]; then
        Log "The DataBackup ProtectAgent service has been successfully stopped."
    else
        echo "The DataBackup ProtectAgent service fails to be stopped."
        Log "The DataBackup ProtectAgent service fails to be stopped."
        return 1
    fi
    return 0
}

StartAgent()
{
    cd "${AGENT_ROOT_PATH}"
    ./start.sh >/dev/null 2>&1
    if [ $? -eq 0 ]; then
        Log "The DataBackup ProtectAgent service is successfully started."
    else
        echo "The DataBackup ProtectAgent service fails to be started."
        Log "The DataBackup ProtectAgent service fails to be started."
        return 1
    fi
    return 0
}

RestartAgent()
{
    StopAgent
    if [ $? -eq 0 ]; then
        Log "Succeed to stop the service."
    else
        Log "Failed to stop the service."
        return 1
    fi

    StartAgent
    if [ $? -eq 0 ]; then
        Log "Succeed to start the service."
    else
        Log "Failed to start the service."
        return 1
    fi

    return 0
}

ConfigPluginCpuLimit()
{
    Log "Start config plugin cpu_limit."
    if [ "${SYS_NAME}" = "Linux" ]; then
        cpu_limit=400
        cpu_num=`cat /proc/cpuinfo |grep processor|wc -l`
        if [ $? -eq 0 ] && [ ${cpu_num} -ne 0 ]; then
            Log "Start calc cpu limit,cpu_num: ${cpu_num}."
            tmp_limit=`expr ${cpu_num} \* 20`
            if [ ${tmp_limit} -le ${cpu_limit} ]; then
                cpu_limit=${tmp_limit}
            fi
        fi
        Log "cpu_num=${cpu_num}, cpu_limit=${cpu_limit}"
        index=1
        while [ 1 ]
        do
            plugin=`echo "${CPU_LIMIT_PLUGIN}" | ${MYAWK} -v i="${index}" '{print $i}'`
            if [ "${plugin}" = "" ]; then
                break
            fi
            Log "Start config plugin(${plugin}) cpu_limit."
            config_files=`ls "${PLUGIN_DIR}/${plugin}" |grep plugin_attribute`
            for config_file in ${config_files}; do
                tmp_result=`cat "${PLUGIN_DIR}/${plugin}/${config_file}"| grep \"cpu_limit\"`
                if [ -z "${tmp_result}" ]; then
                    Log "Add config."
                    if [ "${SYS_NAME}" = "SunOS" ] || [ "${SYS_NAME}" = "AIX" ]; then
                        sed "1 a\  \"cpu_limit\":400," "${PLUGIN_DIR}/${plugin}/${config_file}" > "${PLUGIN_DIR}/${plugin}/${config_file}.tmp"
                        mv ${PLUGIN_DIR}/${plugin}/${config_file}.tmp ${PLUGIN_DIR}/${plugin}/${config_file}
                    else
                        sed -i "1 a\  \"cpu_limit\":400," "${PLUGIN_DIR}/${plugin}/${config_file}" 
                    fi
                else
                    Log "Change config."
                fi
                
                if [ "${SYS_NAME}" = "SunOS" ] || [ "${SYS_NAME}" = "AIX" ]; then
                    sed "/cpu_limit/s/.*/    \"cpu_limit\":${cpu_limit},/g" "${PLUGIN_DIR}/${plugin}/${config_file}" > "${PLUGIN_DIR}/${plugin}/${config_file}.tmp"
                    mv ${PLUGIN_DIR}/${plugin}/${config_file}.tmp ${PLUGIN_DIR}/${plugin}/${config_file}
                else
                    sed -i "/cpu_limit/s/.*/    \"cpu_limit\":${cpu_limit},/g" "${PLUGIN_DIR}/${plugin}/${config_file}"
                fi
            done
            index=`expr $index + 1`
        done
    fi
    Log "Start config plugin cpu_limit succ."
}

ConfigPluginMemLimit()
{
    Log "Start config plugin memory_limit."
    if [ "${SYS_NAME}" = "Linux" ]; then
        memory_limit=4096 # 默认值4G
        memory_limit_file_plugin=16384 # 文件集默认值16G
        memory_size=`free -m | grep Mem | ${MYAWK} '{print $2}'`
        if [ $? -eq 0 ] && [ ${memory_size} -ne 0 ]; then
            Log "Start calc memory limit,memory_size: ${memory_size}."
            tmp_limit=`expr ${memory_size} / 5`
            if [ ${tmp_limit} -le ${memory_limit} ]; then
                memory_limit=${tmp_limit}
            fi
            if [ ${tmp_limit} -le ${memory_limit_file_plugin} ]; then
                memory_limit_file_plugin=${tmp_limit}
            fi
        fi
        Log "Max_mem_size=${memory_size}M, memory_limit=${memory_limit}M, memory_limit_file_plugin=${memory_limit_file_plugin}."
        index=1
        while [ 1 ]
        do
            plugin=`echo "${MEM_LIMIT_PLUGIN}" | ${MYAWK} -v i="${index}" '{print $i}'`
            if [ "${plugin}" = "" ]; then
                break
            fi
            Log "Start config plugin(${plugin}) memory_limit."
            config_files=`ls "${PLUGIN_DIR}/${plugin}" |grep plugin_attribute`
            for config_file in ${config_files}; do
                tmp_result=`cat "${PLUGIN_DIR}/${plugin}/${config_file}"| grep \"memory_limit\"`
                if [ -z "${tmp_result}" ]; then
                    Log "Add config."
                    if [ ${plugin} = "FilePlugin" ]; then
                        if [ "${SYS_NAME}" = "SunOS" ] || [ "${SYS_NAME}" = "AIX" ]; then
                            sed "1 a\  \"memory_limit\":16384," "${PLUGIN_DIR}/${plugin}/${config_file}" > "${PLUGIN_DIR}/${plugin}/${config_file}.tmp"
                            mv ${PLUGIN_DIR}/${plugin}/${config_file}.tmp ${PLUGIN_DIR}/${plugin}/${config_file}
                        else
                            sed -i "1 a\  \"memory_limit\":16384," "${PLUGIN_DIR}/${plugin}/${config_file}"
                        fi
                    els
                        if [ "${SYS_NAME}" = "SunOS" ] || [ "${SYS_NAME}" = "AIX" ]; then
                            sed "/memory_limit/s/.*/    'memory_limit':\${memory_limit_file_plugin},/g" "${PLUGIN_DIR}/${plugin}/${config_file}" > "${PLUGIN_DIR}/${plugin}/${config_file}.tmp"
                            mv -f "${PLUGIN_DIR}/${plugin}/${config_file}.tmp" "${PLUGIN_DIR}/${plugin}/${config_file}"
                        else
                            sed -i "1 a\  \"memory_limit\":4096," "${PLUGIN_DIR}/${plugin}/${config_file}"
                        fi
                    fi
                else
                    Log "Change config."
                fi
                if [ ${plugin} = "FilePlugin" ]; then
                    if [ "${SYS_NAME}" = "SunOS" ] || [ "${SYS_NAME}" = "AIX" ]; then
                        sed "/memory_limit/s/.*/    'memory_limit':\${memory_limit_file_plugin},/g" "${PLUGIN_DIR}/${plugin}/${config_file}" > "${PLUGIN_DIR}/${plugin}/${config_file}.tmp"
                        mv -f "${PLUGIN_DIR}/${plugin}/${config_file}.tmp" "${PLUGIN_DIR}/${plugin}/${config_file}"
                    else
                        sed -i "/memory_limit/s/.*/    \"memory_limit\":${memory_limit_file_plugin},/g" "${PLUGIN_DIR}/${plugin}/${config_file}"
                    fi
                else
                    if [ "${SYS_NAME}" = "SunOS" ] || [ "${SYS_NAME}" = "AIX" ]; then
                        sed "/memory_limit/s/.*/    'memory_limit':\${memory_limit},/g" "${PLUGIN_DIR}/${plugin}/${config_file}" > "${PLUGIN_DIR}/${plugin}/${config_file}.tmp"
                        mv -f "${PLUGIN_DIR}/${plugin}/${config_file}.tmp" "${PLUGIN_DIR}/${plugin}/${config_file}"
                    else
                        sed -i "/memory_limit/s/.*/    \"memory_limit\":${memory_limit},/g" "${PLUGIN_DIR}/${plugin}/${config_file}"
                    fi
                fi
            done
            index=`expr $index + 1`
        done
    fi
    Log "Start config plugin memory_limit succ."
}

InstallPlugins()
{
    if [ ! -d "${PLUGIN_DIR}" ]; then
        Log "Plugins path is not exist."
        return 0
    fi
    echo "Begin to install plugins"
    Log  "InstallPlugins..."
    #1. Get plugins
    tmpPath=`pwd`
    cd ${PLUGIN_DIR}
    pluginList=`ls . | grep ".tar*" | grep -v "cppframework"`
    if [ -z "${pluginList}" ];then
        Log "Not find plugin to install."
        return 0
    fi
    #2. install plugins
    for plugin in ${pluginList}; do
        # 2.1 prepare plugin
    {
        tmpPluginPath=`pwd`
        if [ "${plugin}" = "Block_Service_aarch64.tar.xz" ]; then
            pluginName="Block_Service"
        elif [ "${BACKUP_SCENE}" = "${BACKUP_SCENE_EXTERNAL}" ] && [ "${SYS_NAME}" = "Linux" ]; then
            # linux中Block_Service中包含_
            pluginName=`echo ${plugin} |${MYAWK} -F '.' '{print $1}'`
        else
            # AIX、Solaris中FilePlugin_ppc中用_隔开
            pluginName=`echo ${plugin} |${MYAWK} -F "[_.]" '{print $1}'`
        fi
        mkdir -p ${pluginName}
        SUExecCmd "mkdir -p ${AGENT_BIN_PATH}/log/Plugins/${pluginName}"
        SUExecCmd "chmod -R 700 ${AGENT_BIN_PATH}/log/Plugins/${pluginName}"
        SUExecCmd "chown -R ${AGENT_USER}:${AGENT_GROUP} ${AGENT_BIN_PATH}/log/Plugins/${pluginName}"
        mkdir -p ${AGENT_BIN_PATH}/slog/Plugins/${pluginName}
        CHMOD 700  ${AGENT_BIN_PATH}/slog/Plugins/${pluginName}

        mv "${PLUGIN_DIR}/${plugin}" "${PLUGIN_DIR}/${pluginName}"
        cd "${PLUGIN_DIR}/${pluginName}"
        pluginFile=${plugin}
        if [ "xz" = `echo ${plugin} | ${MYAWK} -F '.' '{print $NF}'` ]; then
            xz -d $plugin
            if [ $? -ne 0 ]; then
                echo "xz $pluginName fail." >> $AGENT_BIN_PATH/stmp/InstallPlugins.info
                LogError "Install $pluginName fail." $ERR_INSTALL_PLUGINS
                return $ERR_INSTALL_PLUGIN_RETCODE
            fi
            pluginFile=${plugin%%.xz}
        fi
        if [ "${SYS_NAME}" = "SunOS" ]; then
            gzip -d $pluginFile
            pluginFile=`ls | grep ".tar"`
        fi
        tar -xf "${PLUGIN_DIR}/${pluginName}/${pluginFile}"
        if [ $? -ne 0 ]; then
            echo "tar $pluginName fail." >> $AGENT_BIN_PATH/stmp/InstallPlugins.info
            LogError "Install $pluginName fail." $ERR_INSTALL_PLUGINS
            return $ERR_INSTALL_PLUGIN_RETCODE
        fi
        rm -rf ${PLUGIN_DIR}/${pluginName}/${pluginFile}
        chmod 755 "${PLUGIN_DIR}"/"${pluginName}"/*.sh

        # 2.2 install plugin
        ./install.sh >> ${LOG_FILE_NAME} 2>&1
        RET_CODE=$?
        if [ "${RET_CODE}" = "${ERR_NO_SUPPORT_PLUGIN}" ]; then
            ShowWarning "Warning: The plug-in [${pluginName}] cannot be installed. If the plug-in needs to be supported, please resolve the problem and reinstall the ProtectAgent."
            Log "Warning: The plug-in [${pluginName}] cannot be installed. If the plug-in needs to be supported, please resolve the problem and reinstall the ProtectAgent."
            rm -f "${PLUGIN_DIR}"/"$plugin"
            rm -rf "${PLUGIN_DIR}"/"${pluginName}"
        else
            if [ ${RET_CODE} -ne 0 ]; then
                Log "Install $pluginName fail."
                rm -rf ${PLUGIN_DIR}/cppframework*
                echo "Install $pluginName fail." >> $AGENT_BIN_PATH/stmp/InstallPlugins.info
                LogError "Install $pluginName fail." $ERR_INSTALL_PLUGINS
                return $ERR_INSTALL_PLUGIN_RETCODE
            fi
            Log "Install $pluginName succ."
            cd ${tmpPluginPath}
            if [ -f "$plugin" ]; then
                rm -rf "$plugin"
            fi
        fi
    }&
    done
    wait

    if [ -f "$AGENT_BIN_PATH/stmp/InstallPlugins.info" ]; then
        InsPlureasult=`cat $AGENT_BIN_PATH/stmp/InstallPlugins.info | grep fail`
        if [ "$InsPlureasult" != "" ];then
            echo "Install plugins failed."
            LogError "Install plugins failed." $ERR_INSTALL_PLUGINS
            return $ERR_INSTALL_PLUGIN_RETCODE
        fi
    fi

    ConfigPluginCpuLimit

    ConfigPluginMemLimit

    rm -rf "$AGENT_BIN_PATH"/stmp/InstallPlugins.info
    rm -rf ${PLUGIN_DIR}/cppframework*
 
    cd $tmpPath
    CHMOD 550 ${PLUGIN_DIR}
    SUExecCmd "mkdir ${AGENT_BIN_PATH}/conf/PluginPid"
    chown -h root:${AGENT_GROUP} ${PLUGIN_DIR}
    CHMOD -R 700 ${AGENT_BIN_PATH}/slog/Plugins/
    SUExecCmd "chmod 700 ${AGENT_BIN_PATH}/conf/PluginPid"
    # 2. set host ssl domain name
    if [ "${BACKUP_SCENE}" = "${BACKUP_SCENE_EXTERNAL}" ]; then
        DomainName=`${AGENT_BIN_PATH}/bin/openssl x509 -subject -in ${AGENT_BIN_PATH}/nginx/conf/server.pem -noout | ${MYAWK}  -F '=' '{print $NF}'`
        if [ -z "${DomainName}" ];then
            echo "Get ssl domain failed."
            return 1
        fi
        tmpDomainName=`cat /etc/hosts | grep ${DomainName}`
        if [ -z "${tmpDomainName}" ]; then
            echo "127.0.0.1 ${DomainName}" >> "/etc/hosts"
        fi
    elif [ "${BACKUP_SCENE}" = "${BACKUP_SCENE_INTERNAL}" ]; then
        chown -h root:${DEFAULT_GROUP_INTERNAL} ${PLUGIN_DIR}
    fi
}

########################################################################################
# Main Process
########################################################################################
printf "\\033[1;32m********************************************************\\033[0m \n"
printf "\\033[1;32m     Start the modify of ${PRODUCT_NAME}     \\033[0m \n"
printf "\\033[1;32m********************************************************\\033[0m \n"

# Check Whether Input Params Are Valid
CheckInputParams $*
if [ $? -eq 1 ]; then
    ExitHandle 1
fi

# Check Whether Input Params Are Valid
CheckIsInstallAgent
# Compare modify version
CompareUpdateVersion
if [ $? -eq 1 ]; then
    echo "Failed to update, Old_Update_Version[$OLD_UPDATE_VERSION]!=New_Update_Version[$NEW_UPDATE_VERSION]"
    LogError "Failed to update, Old_Update_Version[$OLD_UPDATE_VERSION]!=New_Update_Version[$NEW_UPDATE_VERSION]"
    ExitHandle 1
else
    Log "The modify version is detected successfully."
fi
BackUpCurAgentInfo
# backup log

sh $CURRENT_PATH/collectlog.sh >/dev/null 2>&1
if [ $? -ne 0 ]; then
    echo "Collect old log failed."
    LogError "Collect old log failed." ${ERR_MODIFY_FAIL_BACKUP}
    ExitHandle 1
else
    CP -r -p $PLUGIN_DIR/* $MODIFY_BACKUP_PATH
fi

UninstallPlugins
if [ $? -ne 0 ]; then
    Log "Uninstall plugin failed"
    ExitHandle 1
fi

ReplacePack

InstallPlugins
if [ $? -ne 0 ]; then
    Log "Install plugin failed"
    ExitHandle 1
fi

RestartAgent

ModifyParamAndRegister

CopyLogFile ${AGENT_ROOT_PATH}

CleanPkg
printf "\\033[1;32mModify ProtectAgent success.\\033[0m \n"
Log "Modify: Modify ProtectAgent success."
exit 0
