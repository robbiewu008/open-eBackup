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

#--------------------------------------------
# $1: -r for push upgrade
#--------------------------------------------
SHELL_TYPE_SH="/bin/sh"

UNIX_CMD=
AGENT_USER_PATH="/home"
AGENT_USER=rdadmin
AGENT_GROUP=rdadmin
SANCLIENT_USER=sanclient
SANCLIENT_GROUP=sanclient
CURRENT_PATH=`dirname $0` && cd $CURRENT_PATH && CURRENT_PATH=`pwd`
AGENT_ROOT_PATH=${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient
DATA_BACKUP_AGENT_HOME_BAK=${DATA_BACKUP_AGENT_HOME}
SANCLIENT_AGENT_ROOT_PATH="${DATA_BACKUP_SANCLIENT_HOME}/DataBackup/SanClient"
AGENT_BACKUP_PATH=${DATA_BACKUP_AGENT_HOME}/AgentUpgrade
BACKUP_COPY_DIR=${DATA_BACKUP_AGENT_HOME}/DataBackup/Bak
BACKUP_ROLE_SANCLIENT_PLUGIN=5
NEW_PACKAGE_PATH=$1
BACKUP_ROLE=
PM_IP=
XBSA_INSTALL_DIR="/usr/openv/"
SYS_NAME=`uname -s`

if [ "$SYS_NAME" = "SunOS" ]; then
    AWK=nawk
else
    AWK=awk
fi

CLIENT_BACK_ROLE=`cat ${INSTALL_PACKAGE_PATH}/conf/client.conf | grep "backup_role" | ${AWK} -F '=' '{print $NF}'`
if [ "${CLIENT_BACK_ROLE}" = "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ]; then
    AGENT_USER=${SANCLIENT_USER}
    AGENT_GROUP=${SANCLIENT_GROUP} 
    DATA_BACKUP_AGENT_HOME_BAK=${DATA_BACKUP_SANCLIENT_HOME}
    AGENT_ROOT_PATH=${SANCLIENT_AGENT_ROOT_PATH}
    BACKUP_COPY_DIR="${DATA_BACKUP_SANCLIENT_HOME}/DataBackup/SanClientBak"
    AGENT_BACKUP_PATH="${DATA_BACKUP_SANCLIENT_HOME}/SanClientUpgrade"
fi

NEW_INSTALL_PATH=${AGENT_ROOT_PATH}
DWS_BACKUP_PATH=${NEW_PACKAGE_PATH}/DWS_BACKUP/
IIF_BACKUP_PATH=${NEW_PACKAGE_PATH}/IIF_BACKUP/


LATEST_DIR=
if [ -d "${BACKUP_COPY_DIR}" ]; then
    LATEST_DIR=`ls -t ${BACKUP_COPY_DIR} | head -n 1`
fi
LATEST_BAK_DIR=${BACKUP_COPY_DIR}/${LATEST_DIR}

# errorcode
ERR_UPGRADE_FAIL_DATATURBO_INSTALL=1577210105
ERR_UPGRADE_FAIL_DATATURBO_UPGRADE=1577210106

###### SET UMASK ######
umask 0022

################### SNMP Config ###################
ENGINE_ID=""
CONTEXT_NAME=""
SECURITY_NAME=""
SECURITY_MODEL=""
SECURITY_LEVEL=""
PRIVATE_PASSWORD=""
PRIVATE_PROTOCOL=""
AUTH_PASSWORD=""
AUTH_PROTOCOL=""
ARCHIVE_THRESHOLD=""
ARCHIVE_THREAD_TIMEOUT=""

########################################################################################
# Function Definition
########################################################################################
CheckShellType()
{
    ROCKY4_FLAG=""
    SHELL_TYPE=`cat /etc/passwd | grep "^${AGENT_USER}:" | $AWK -F "/" '{print $NF}'`

    if [ "${SYS_NAME}" = "Linux" ]; then
        rocky4=`cat /etc/issue | grep 'Rocky'`
        if [ -n "${rocky4}" ]; then
            ROCKY4_FLAG="1"
        fi
    fi

    if [ "${SYS_NAME}" = "AIX" ] || [ "${SYS_NAME}" = "HP-UX" ] || [ "${SYS_NAME}" = "SunOS" ] || [ "1" = "${ROCKY4_FLAG}" ]; then
        if [ "${SHELL_TYPE}" = "bash" ]; then
            UNIX_CMD=-l
        fi
    fi
}

LogError()
{
    #$1=errorInfo,$2=errorCode, $3=errorDetailParam
    Log "[ERR] $1"
    echo "logDetail=$2" > ${NEW_PACKAGE_PATH}/errormsg.log
    echo "logInfo=job_log_agent_storage_update_install_fail_label" >> ${NEW_PACKAGE_PATH}/errormsg.log
    echo "logDetailParam=$3" >> ${NEW_PACKAGE_PATH}/errormsg.log
    chmod 640 ${NEW_PACKAGE_PATH}/errormsg.log
}

############## Prepare For Upgration #################
# determin execute user by different ways
CheckExecUser()
{
    LOG_USER=${LOGNAME}
    ENV_USER=${USER}
    ENV_UID=${UID}

    if [ "SunOS" = "$SYS_NAME" ]; then
        command -v whoami >/dev/null
        if [ $? -eq 0 ]; then
            WHO_AM_I=`whoami`
        else
            WHO_AM_I=`/usr/ucb/whoami`
        fi
    else
        WHO_AM_I=`whoami`
    fi

    if [ "root" = "${LOG_USER}" ] || [ "root" = "${ENV_USER}" ] || [ "0" = "${ENV_UID}" ] || [ "root" = "${WHO_AM_I}" ]; then
        return 0
    else
        return 1
    fi
}

#stop agent
StopNewAgent()
{
    cd "${NEW_INSTALL_PATH}"
    chmod +x stop.sh
    ${NEW_INSTALL_PATH}/stop.sh >/dev/null 2>&1
    if [ $? -eq 0 ]; then
        Log "The DataBackup ProtectAgent service has been successfully stopped."
    else
        echo "The DataBackup ProtectAgent service fails to be stopped."
        Log "The DataBackup ProtectAgent service fails to be stopped."
        return 1
    fi
    return 0
}

#start agent
StartOldAgent()
{
    cd "${OLD_INSTALL_PATH}"
    chmod +x start.sh
    ${OLD_INSTALL_PATH}/start.sh >/dev/null 2>&1
    if [ $? -eq 0 ]; then
        Log "The DataBackup ProtectAgent service is successfully started."
    else
        echo "The DataBackup ProtectAgent service fails to be started."
        Log "The DataBackup ProtectAgent service fails to be started."
        return 1
    fi
    return 0
}

RestartNewAgent()
{
    Log "Step 6: Restart ProtectAgent."
    cd "${NEW_INSTALL_PATH}"
    
    chmod +x stop.sh
    ${NEW_INSTALL_PATH}/stop.sh >/dev/null 2>&1
    if [ $? -eq 0 ]; then
        Log "The DataBackup ProtectAgent service has been successfully stopped."
    else
        echo "The DataBackup ProtectAgent service fails to be stopped."
        Log "The DataBackup ProtectAgent service fails to be stopped."
        return 1
    fi

    chmod +x start.sh
    ${NEW_INSTALL_PATH}/start.sh >/dev/null 2>&1
    if [ $? -eq 0 ]; then
        Log "The DataBackup ProtectAgent service is successfully started."
    else
        echo "The DataBackup ProtectAgent service fails to be started."
        Log "The DataBackup ProtectAgent service fails to be started."
        return 1
    fi

    return 0
}

RegisterHost()
{
    JudgeRandomNumType
    INSTALL_PATH=$1
    if [ "$2" == "upgrade" ]; then
        SUExecCmd "${INSTALL_PATH}/ProtectClient-E/bin/agentcli registerHost RegisterHost Upgrade >/dev/null 2>&1"
    else
        SUExecCmd "${INSTALL_PATH}/ProtectClient-E/bin/agentcli registerHost RegisterHost >/dev/null 2>&1"
    fi 
    if [ "$?" != 0 ]; then
        Log "Register host to ProtectManager failed."
        printf "\033[31mThe host cannot be registered to the ProtectManager. \033[0m\n"
        return 1
    fi
    echo "Register host to ProtectManager success."
    Log "Register host to ProtectManager success."
    return 0
}

RestoreProfile()
{
    if [ "${OLD_INSTALL_PATH}" = "/opt/OceanProtect/ProtectClient" ]; then
        su - ${AGENT_USER} -s ${SHELL_TYPE_SH} -c "mv /home/.profile.bak  /home/rdadmin/.profile"
    fi
}

RollBackAgent()
{
    Log "Start to rollback agent."

    # 适配1.2升级1.5场景
    RestoreProfile

    # backup log files
    if [ -f "${NEW_INSTALL_PATH}/ProtectClient-E/slog/agent_install.log" ]; then
        CP ${NEW_INSTALL_PATH}/ProtectClient-E/slog/agent_install.log ${NEW_PACKAGE_PATH}/agent_install_in_upgrade.log
    fi

    if [ -f "${NEW_INSTALL_PATH}/ProtectClient-E/log/crl_update.log" ]; then
        CP ${NEW_INSTALL_PATH}/ProtectClient-E/log/crl_update.log ${NEW_PACKAGE_PATH}/crl_update_in_upgrade.log
    fi

    if [ -f "${NEW_INSTALL_PATH}/ProtectClient-E/stmp/errormsg.log" ]; then
        CP ${NEW_INSTALL_PATH}/ProtectClient-E/stmp/errormsg.log ${NEW_PACKAGE_PATH}/errormsg.log
    fi

    if [ -f "${NEW_INSTALL_PATH}/ProtectClient-E/log/agent_upgrade_sqlite.log" ]; then
        CP ${NEW_INSTALL_PATH}/ProtectClient-E/log/agent_upgrade_sqlite.log ${NEW_PACKAGE_PATH}/agent_upgrade_sqlite.log
    fi

    StopNewAgent
    if [ ! -d "${OLD_INSTALL_PATH}" ];then
        mkdir -p ${OLD_INSTALL_PATH}
        chmod -R 755 ${OLD_INSTALL_PATH}/..
    fi
    rm -rf "${OLD_INSTALL_PATH}"/*
    rm -rf "${NEW_INSTALL_PATH}"/*
    mv -f "${AGENT_BACKUP_PATH}"/* "${OLD_INSTALL_PATH}/"
    if [ -d "${XBSA_INSTALL_DIR}" ]; then
        cp -pR ${DWS_BACKUP_PATH}/* ${XBSA_INSTALL_DIR}
    fi
    if [ -d "${IIF_BACKUP_PATH}" ] && [ -f "${IIF_BACKUP_PATH}/libcommon.so" ] && [ -f "${IIF_BACKUP_PATH}/libsecurecom.so" ]; then
        IIF_LIB_COUNT=`ls -l ${IIF_BACKUP_PATH}/ |wc -l`
        if [ ${IIF_LIB_COUNT} -eq 2 ]; then
            cp -pR ${IIF_BACKUP_PATH}/* /lib64/
        fi
    fi
    StartOldAgent

    if [ -f ${OLD_INSTALL_PATH}/ProtectClient-E/nginx/conf/nginx.conf ]; then
        NGINX_PORT=`cat ${OLD_INSTALL_PATH}/ProtectClient-E/nginx/conf/nginx.conf | grep "listen*" | $AWK -F '= ' '{print $1}' | $AWK -F ':' '{print $2}' | $AWK -F ' ' '{print $1}'`
    else
        NGINX_PORT=`cat ${OLD_INSTALL_PATH}/ProtectClient-E/bin/nginx/conf/nginx.conf | grep "listen*" | $AWK -F '= ' '{print $1}' | $AWK -F ':' '{print $2}' | $AWK -F ' ' '{print $1}'`
    fi
    if [ 2 -eq $BACKUP_ROLE ]; then 
        AddPortFilter ${NGINX_PORT}
    fi

    DisableHttpProxy
    RegisterHost "${OLD_INSTALL_PATH}"
    if [ 0 != $? ]; then
        echo "Previous DataBackup ProtectAgent has been rolled back failed."
        Log "Previous DataBackup ProtectAgent has been rolled back failed."
        printf "\033[31mDataBackup ProtectAgent fails to be upgraded.\033[0m\n"
        return 1
    else
        echo "Previous DataBackup ProtectAgent has been rolled back successfully."
        Log "Previous DataBackup ProtectAgent has been rolled back successfully."
    fi

    WorkAfterUpgrade
    printf "\033[31mDataBackup ProtectAgent fails to be upgraded.\033[0m\n"
    return 1
}

IsParamValid()
{
    ParamInput=$1
    ParamType=$2
    
    IPV4_Regular_Expression="^(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])\.(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])\.(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])\.(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])$"
    IPV6_Regular_Expression="^((([0-9a-fA-F]{1,4}:){7}([0-9A-Fa-f]{1,4}|:))|(([0-9a-fA-F]{1,4}:){6}((([0-9]{1,3}\.){3}[0-9]{1,4})|([0-9a-fA-F]{1,4})|:))|(([0-9a-fA-F]{1,4}:){5}:((([0-9a-fA-F]{1,4})?)|([0-9a-fA-F]{1,4}:[0-9a-fA-F]{1,4})|(([0-9]{1,3}\.){3}[0-9]{1,4})))|(([0-9a-fA-F]{1,4}:){4}:((([0-9a-fA-F]{1,4})?)|(([0-9a-fA-F]{1,4}:){1,2}[0-9a-fA-F]{1,4})|(([0-9a-fA-F]{1,4}:)?([0-9]{1,3}\.){3}[0-9]{1,4})))|(([0-9a-fA-F]{1,4}:){3}:((([0-9a-fA-F]{1,4}:){0,2}([0-9]{1,3}\.){3}[0-9]{1,4})|(([0-9a-fA-F]{1,4}:){0,3}[0-9a-fA-F]{1,4})|$))|(([0-9a-fA-F]{1,4}:){2}:((([0-9a-fA-F]{1,4}:){0,3}([0-9]{1,3}\.){3}[0-9]{1,4})|(([0-9a-fA-F]{1,4}:){0,4}[0-9a-fA-F]{1,4})|$))|(([0-9a-fA-F]{1,4}:){1}:((([0-9a-fA-F]{1,4}:){0,4}([0-9]{1,3}\.){3}[0-9]{1,4})|(([0-9a-fA-F]{1,4}:){0,5}[0-9a-fA-F]{1,4})|$))|(::((([0-9a-fA-F]{1,4}:){0,5}([0-9]{1,3}\.){3}[0-9]{1,4})|(([0-9a-fA-F]{1,4}:){0,6}[0-9a-fA-F]{1,4})|$)))$"
    InvalidDelims="[\"\|\*\'\;\#]|\.(sh)"
    
    ExistCommandEgrep=`command -v egrep`
    if [ "${ExistCommandEgrep}" != "" ]; then
        MYGREP="egrep"
    else
        if [ "${SYS_NAME}" = "SunOS" ]; then
            Log "Command egrep not found, and grep -E not supported, exit judge."
            return 0
        fi
        MYGREP="grep -E"
    fi
    
    if [ "${ParamType}" = "IP" ]; then
        ContainInvalidDelim=`echo "${ParamInput}" | ${MYGREP} "${InvalidDelims}"`
        IsValidIPV4=`echo "${ParamInput}" | ${MYGREP} "${IPV4_Regular_Expression}"`
        if [ "${SYS_NAME}" = "Linux" ]; then
            IsValidIPV6=`echo "${ParamInput}" | ${MYGREP} "${IPV6_Regular_Expression}"`
        fi
        if [ "${ContainInvalidDelim}" != "" ]; then
            return 1
        fi
        if [ "${IsValidIPV4}" != "" ] || [ "${IsValidIPV6}" != "" ]; then
            return 0
        fi
    elif [ "${ParamType}" = "STRING" ]; then
        ContainInvalidDelim=`echo "${ParamInput}" | ${MYGREP} "${InvalidDelims}"`
        if [ "${ContainInvalidDelim}" = "" ]; then
            return 0
        fi
    elif [ "${ParamType}" = "PORT" ]; then
        #59520-59539
        IsValidPORT=`echo "$ParamInput" | ${MYGREP} "^595[2-3][0-9]$"`
        if [ -n "$IsValidPORT" ]; then
            return 0
        fi
    else
        Log "Invalid param type while invoke function IsParamValid."
        return 1
    fi
    return 1
}

XmlParseViaShell()
{
    XML_FILE=$1
    PARENT_FILED=$2
    CHILD_FILED=$3
    TEMP_FILE=${AGENT_USER_PATH}/${AGENT_USER}/tempFile
    FILED_VALUE=

    if [ ! -f ${XML_FILE} ]; then
        return 1
    fi
    
    if [ "${SYS_NAME}" = "Linux" ]; then
        FILED_VALUE=`SUExecCmdWithOutput "cat ${XML_FILE} | grep -A 100 \"<${PARENT_FILED}>\" | grep -B 100 \"</${PARENT_FILED}>\" | grep \"${CHILD_FILED}\" | $AWK -F '=' '{print \\$2}' | $AWK -F '\"' '{print \\$2}'"` >/dev/null 2>&1
    elif [ "${SYS_NAME}" = "AIX" ]; then
        SUExecCmd "grep -n \"<${PARENT_FILED}>\" ${XML_FILE} | cut -d':' -f1 | xargs -n1 -I % $AWK 'NR>=% && NR<=%+100' ${XML_FILE} > ${TEMP_FILE}_1"
        SUExecCmd "grep -n \"</${PARENT_FILED}>\" ${TEMP_FILE}_1 | cut -d':' -f1 | xargs -n1 -I % $AWK 'NR>=%-100 && NR<=%' ${TEMP_FILE}_1 > ${TEMP_FILE}_2"
        FILED_VALUE=`SUExecCmdWithOutput "cat ${TEMP_FILE}_2 | grep \"${CHILD_FILED}\" | $AWK -F '=' '{print \\$2}' | $AWK -F '\"' '{print \\$2}'"` >/dev/null 2>&1
        SUExecCmd "rm -rf \"${TEMP_FILE}_1\""
        SUExecCmd "rm -rf \"${TEMP_FILE}_2\""
    else
        FILED_VALUE=`SUExecCmdWithOutput "cat ${XML_FILE} | grep \"${CHILD_FILED}\" | $AWK -F '=' '{print \\$2}' | $AWK -F '\"' '{print \\$2}'"` >/dev/null 2>&1
    fi

    if [ "${FILED_VALUE}" != "" ]; then
        echo ${FILED_VALUE}
        return 0
    else
        return 1
    fi
}

PrepareBeforeInstallNewAgent()
{
    echo "Step 1: Back up the files and parameters required for the upgrade."
    Log "Step 1: Back up the files and parameters required for the upgrade."
    # backup role: 0:host; 2:vmware
    BACKUP_ROLE=`XmlParseViaShell ${AGENT_BACKUP_PATH}/ProtectClient-E/conf/agent_cfg.xml Backup backup_role`
    if [ 0 -ne $? ]; then
        printf "\033[31mFailed to get backup_role. \033[0m\n"
        return 1
    fi

    PM_IP=`XmlParseViaShell ${AGENT_BACKUP_PATH}/ProtectClient-E/conf/agent_cfg.xml Backup ebk_server_ip`
    PM_PORT=`SUExecCmdWithOutput "sed '/^PM_PORT=/!d;s/.*=//' ${AGENT_BACKUP_PATH}/ProtectClient-E/conf/testcfg.tmp"`
    PM_MANAGER_IP=`SUExecCmdWithOutput "sed '/^PM_MANAGER_IP=/!d;s/.*=//' ${AGENT_BACKUP_PATH}/ProtectClient-E/conf/testcfg.tmp"`
    PM_MANAGER_PORT=`SUExecCmdWithOutput "sed '/^PM_MANAGER_PORT=/!d;s/.*=//' ${AGENT_BACKUP_PATH}/ProtectClient-E/conf/testcfg.tmp"`
    ENABLE_HTTP_PROXY=`XmlParseViaShell ${AGENT_BACKUP_PATH}/ProtectClient-E/conf/agent_cfg.xml System enable_http_proxy`
    DISABLE_SAFE_RANDOM_NUMBER=`XmlParseViaShell ${AGENT_BACKUP_PATH}/ProtectClient-E/conf/agent_cfg.xml Security disable_safe_random_number`
    IS_ENABLE_DATATURBO=`XmlParseViaShell ${AGENT_BACKUP_PATH}/ProtectClient-E/conf/agent_cfg.xml Dataturbo is_enable_dataturbo`
    EIP=`SUExecCmdWithOutput "sed '/^EIP=/!d;s/.*=//' ${AGENT_BACKUP_PATH}/ProtectClient-E/conf/testcfg.tmp"`
    IS_FILECLIENT_INSTALL=`SUExecCmdWithOutput "sed '/^IS_FILECLIENT_INSTALL=/!d;s/.*=//' ${AGENT_BACKUP_PATH}/ProtectClient-E/conf/testcfg.tmp"`

    # current user id
    USER_ID=`XmlParseViaShell ${AGENT_BACKUP_PATH}/ProtectClient-E/conf/agent_cfg.xml System userid`
    if [ 0 -ne $? ]; then
        printf "\033[31mFailed to get userid. \033[0m\n"
        return 1
    fi

    # replace backup role in the new agent package client.conf
    if [ 0 -eq $BACKUP_ROLE ]; then 
        sed "s|vmCenter=.*|vmCenter=false|" ${NEW_PACKAGE_PATH}/conf/client.conf > ${NEW_PACKAGE_PATH}/conf/client.conf.bak
        mv -f "${NEW_PACKAGE_PATH}/conf/client.conf.bak" "${NEW_PACKAGE_PATH}/conf/client.conf"
    else
        sed "s|vmCenter=.*|vmCenter=true|" ${NEW_PACKAGE_PATH}/conf/client.conf > ${NEW_PACKAGE_PATH}/conf/client.conf.bak
        mv -f "${NEW_PACKAGE_PATH}/conf/client.conf.bak" "${NEW_PACKAGE_PATH}/conf/client.conf"
    fi

    # replace pm ip & port
    sed -i "s|pm_ip=.*|pm_ip=${PM_IP}|" ${NEW_PACKAGE_PATH}/conf/client.conf
    sed -i "s|pm_port=.*|pm_port=${PM_PORT}|" ${NEW_PACKAGE_PATH}/conf/client.conf
    sed -i "s|pm_manager_ip=.*|pm_manager_ip=${PM_MANAGER_IP}|" ${NEW_PACKAGE_PATH}/conf/client.conf
    sed -i "s|pm_manager_port=.*|pm_manager_port=${PM_MANAGER_PORT}|" ${NEW_PACKAGE_PATH}/conf/client.conf
    sed -i "s|enable_http_proxy=.*|enable_http_proxy=${ENABLE_HTTP_PROXY}|" ${NEW_PACKAGE_PATH}/conf/client.conf
    sed -i "s|is_enable_dataturbo=.*|is_enable_dataturbo=${IS_ENABLE_DATATURBO}|" ${NEW_PACKAGE_PATH}/conf/client.conf

    cat ${NEW_PACKAGE_PATH}/conf/client.conf | grep "disable_safe_random_number=" >/dev/null 2>&1
    if [ $? -eq 0 ]; then
        sed -i "s|disable_safe_random_number=.*|disable_safe_random_number=${DISABLE_SAFE_RANDOM_NUMBER}|" ${NEW_PACKAGE_PATH}/conf/client.conf
    else
        echo "disable_safe_random_number=${DISABLE_SAFE_RANDOM_NUMBER}" >> ${NEW_PACKAGE_PATH}/conf/client.conf
    fi

    # replace user id
    sed "s|user_id=.*|user_id=$USER_ID|" ${NEW_PACKAGE_PATH}/conf/client.conf > ${NEW_PACKAGE_PATH}/conf/client.conf.bak
    mv -f "${NEW_PACKAGE_PATH}/conf/client.conf.bak" "${NEW_PACKAGE_PATH}/conf/client.conf"

    # replace certification
    CP "${NEW_PACKAGE_PATH}/conf/ca.crt.pem" "${NEW_PACKAGE_PATH}/conf/ca.crt.pem.bk"
    CP "${NEW_PACKAGE_PATH}/conf/client.crt.pem" "${NEW_PACKAGE_PATH}/conf/client.crt.pem.bk"
    CP "${NEW_PACKAGE_PATH}/conf/client.pem" "${NEW_PACKAGE_PATH}/conf/client.pem.bk"
    CP "${NEW_PACKAGE_PATH}/conf/agentca.crt.pem" "${NEW_PACKAGE_PATH}/conf/agentca.crt.pem.bk"
    # 处理Agent证书独立后老版本向新版本升级问题, 该场景中使用新包中的证书文件，其余场景使用当前正在使用的证书文件
    if [ -f "${AGENT_BACKUP_PATH}/ProtectClient-E/nginx/conf/pmca.pem" ]; then 
        CP "${AGENT_BACKUP_PATH}/ProtectClient-E/nginx/conf/pmca.pem" "${NEW_PACKAGE_PATH}/conf/ca.crt.pem"
        CP "${AGENT_BACKUP_PATH}/ProtectClient-E/nginx/conf/server.pem" "${NEW_PACKAGE_PATH}/conf/client.crt.pem"
        CP "${AGENT_BACKUP_PATH}/ProtectClient-E/nginx/conf/server.key" "${NEW_PACKAGE_PATH}/conf/client.pem"
        CP "${AGENT_BACKUP_PATH}/ProtectClient-E/nginx/conf/agentca.pem" "${NEW_PACKAGE_PATH}/conf/agentca.crt.pem"
    elif [ -f "${AGENT_BACKUP_PATH}/ProtectClient-E/bin/nginx/conf/pmca.pem" ]; then 
        CP "${AGENT_BACKUP_PATH}/ProtectClient-E/bin/nginx/conf/pmca.pem" "${NEW_PACKAGE_PATH}/conf/ca.crt.pem"
        CP "${AGENT_BACKUP_PATH}/ProtectClient-E/bin/nginx/conf/server.pem" "${NEW_PACKAGE_PATH}/conf/client.crt.pem"
        CP "${AGENT_BACKUP_PATH}/ProtectClient-E/bin/nginx/conf/server.key" "${NEW_PACKAGE_PATH}/conf/client.pem"
        CP "${AGENT_BACKUP_PATH}/ProtectClient-E/bin/nginx/conf/agentca.pem" "${NEW_PACKAGE_PATH}/conf/agentca.crt.pem"
    else
        return 1
    fi
    # dws backup
    if [ -d "${XBSA_INSTALL_DIR}" ]; then
        mkdir -p ${DWS_BACKUP_PATH}
        cp -pR ${XBSA_INSTALL_DIR}/* ${DWS_BACKUP_PATH}
    fi

    if [ -f "/lib64/libcommon.so" ] && [ -f "/lib64/libsecurecom.so" ]; then
        mkdir -p ${IIF_BACKUP_PATH}
        cp -pR /lib64/libcommon.so ${IIF_BACKUP_PATH}
        cp -pR /lib64/libsecurecom.so ${IIF_BACKUP_PATH}
    fi
    return 0
}

WorkAfterUpgrade()
{
    CP "${NEW_PACKAGE_PATH}/conf/ca.crt.pem.bk" "${NEW_PACKAGE_PATH}/conf/ca.crt.pem"
    CP "${NEW_PACKAGE_PATH}/conf/client.crt.pem.bk" "${NEW_PACKAGE_PATH}/conf/client.crt.pem"
    CP "${NEW_PACKAGE_PATH}/conf/client.pem.bk" "${NEW_PACKAGE_PATH}/conf/client.pem"
    CP "${NEW_PACKAGE_PATH}/conf/agentca.crt.pem.bk" "${NEW_PACKAGE_PATH}/conf/agentca.crt.pem"

    rm "${NEW_PACKAGE_PATH}/conf/ca.crt.pem.bk"
    rm "${NEW_PACKAGE_PATH}/conf/client.crt.pem.bk"
    rm "${NEW_PACKAGE_PATH}/conf/client.pem.bk"
    rm "${NEW_PACKAGE_PATH}/conf/agentca.crt.pem.bk"

    if [ -d "${DWS_BACKUP_PATH}" ]; then
        rm -rf "${DWS_BACKUP_PATH}"
    fi
    rm -rf "${AGENT_BACKUP_PATH}"
}

HandleDataTurbo()
{
    # 判断是升级还是安装 dataturbo
    IsDataTurboInstalled
    if [ $? -eq 0 ]; then
        clearDataturboMountPath
        unzipdir=`which unzip 2>/dev/null | awk '{print $1}'`
        if [ "" = "${unzipdir}" ]; then
            UpgradeDataTurbo "${INSTALL_PACKAGE_PATH}/third_party_software/dataturbo.tar.gz" "tar"
        else
            UpgradeDataTurbo "${INSTALL_PACKAGE_PATH}/third_party_software/dataturbo.zip" "zip"
        fi
        if [ $? -ne 0 ]; then
            LogError "Faild to upgrade dataturbo." ${ERR_UPGRADE_FAIL_DATATURBO_UPGRADE}
            exit 1
        fi

    else
        #跟升级前保持一致 不安装 
         Log "The system support dataturbo but do not install.Maintain consistency with before upgrading."
    fi
}

UninstallOldAgent()
{
    echo "Step 2: Uninstall the old agent."
    Log "Step 2: Uninstall the old agent."
    # Get snmp configuration
    GetSNMPCfg

    ${OLD_INSTALL_PATH}/uninstall.sh
}

GetSNMPCfg()
{
    log_level=`SUExecCmdWithOutput "\"${OLD_INSTALL_PATH}/ProtectClient-E/sbin/xmlcfg\" read System \"log_level\""`
    log_count=`SUExecCmdWithOutput "\"${OLD_INSTALL_PATH}/ProtectClient-E/sbin/xmlcfg\" read System \"log_count\""`
    log_max_size=`SUExecCmdWithOutput "\"${OLD_INSTALL_PATH}/ProtectClient-E/sbin/xmlcfg\" read System \"log_max_size\""`
    log_keep_time=`SUExecCmdWithOutput "\"${OLD_INSTALL_PATH}/ProtectClient-E/sbin/xmlcfg\" read System \"log_keep_time\""`

    logfile_size=`SUExecCmdWithOutput "\"${OLD_INSTALL_PATH}/ProtectClient-E/sbin/xmlcfg\" read Monitor rdagent \"logfile_size\""`
    logfile_size_alarm_threshold=`SUExecCmdWithOutput "\"${OLD_INSTALL_PATH}/ProtectClient-E/sbin/xmlcfg\" read Monitor rdagent \"logfile_size_alarm_threshold\""`

    engine_id=`SUExecCmdWithOutput "\"${OLD_INSTALL_PATH}/ProtectClient-E/sbin/xmlcfg\" read SNMP \"engine_id\""`
    context_name=`SUExecCmdWithOutput "\"${OLD_INSTALL_PATH}/ProtectClient-E/sbin/xmlcfg\" read SNMP \"context_name\""`
    security_name=`SUExecCmdWithOutput "\"${OLD_INSTALL_PATH}/ProtectClient-E/sbin/xmlcfg\" read SNMP \"security_name\""`
    security_model=`SUExecCmdWithOutput "\"${OLD_INSTALL_PATH}/ProtectClient-E/sbin/xmlcfg\" read SNMP \"security_model\""`
    security_level=`SUExecCmdWithOutput "\"${OLD_INSTALL_PATH}/ProtectClient-E/sbin/xmlcfg\" read SNMP \"security_level\""`
    private_password=`SUExecCmdWithOutput "\"${OLD_INSTALL_PATH}/ProtectClient-E/sbin/xmlcfg\" read SNMP \"private_password\""`
    private_protocol=`SUExecCmdWithOutput "\"${OLD_INSTALL_PATH}/ProtectClient-E/sbin/xmlcfg\" read SNMP \"private_protocol\""`
    auth_password=`SUExecCmdWithOutput "\"${OLD_INSTALL_PATH}/ProtectClient-E/sbin/xmlcfg\" read SNMP \"auth_password\""`
    auth_protocol=`SUExecCmdWithOutput "\"${OLD_INSTALL_PATH}/ProtectClient-E/sbin/xmlcfg\" read SNMP \"auth_protocol\""`

    archive_threshold=`SUExecCmdWithOutput "\"${OLD_INSTALL_PATH}/ProtectClient-E/sbin/xmlcfg\" read Backup \"archive_threshold\""`
    archive_thread_timeout=`SUExecCmdWithOutput "\"${OLD_INSTALL_PATH}/ProtectClient-E/sbin/xmlcfg\" read Backup \"archive_thread_timeout\""`

    LOG_LEVEL=${log_level}
    LOG_COUNT=${log_count}
    LOG_MAX_SIZE=${log_max_size}
    LOG_KEEP_TIME=${log_keep_time}
    LOGFILE_SIZE=${logfile_size}
    LOGFILE_SIZE_ALARM_THRESHOLD=${logfile_size_alarm_threshold}
    ENGINE_ID=${engine_id}
    CONTEXT_NAME=${context_name}
    SECURITY_NAME=${security_name}
    SECURITY_MODEL=${security_model}
    SECURITY_LEVEL=${security_level}
    PRIVATE_PASSWORD=${private_password}
    PRIVATE_PROTOCOL=${private_protocol}
    AUTH_PASSWORD=${auth_password}
    AUTH_PROTOCOL=${auth_protocol}
    ARCHIVE_THRESHOLD=${archive_threshold}
    ARCHIVE_THREAD_TIMEOUT=${archive_thread_timeout}
    UPGRADE_BEFORE_LOGLEVEL=${LOG_LEVEL}
    export UPGRADE_BEFORE_LOGLEVEL
}

SetSNMPCfg()
{
    SUExecCmd "\"${NEW_INSTALL_PATH}/ProtectClient-E/sbin/xmlcfg\" write System log_level \"${LOG_LEVEL}\""
    SUExecCmd "\"${NEW_INSTALL_PATH}/ProtectClient-E/sbin/xmlcfg\" write System log_count \"${LOG_COUNT}\""
    SUExecCmd "\"${NEW_INSTALL_PATH}/ProtectClient-E/sbin/xmlcfg\" write System log_max_size \"${LOG_MAX_SIZE}\""
    SUExecCmd "\"${NEW_INSTALL_PATH}/ProtectClient-E/sbin/xmlcfg\" write System log_keep_time \"${LOG_KEEP_TIME}\""
    SUExecCmd "\"${NEW_INSTALL_PATH}/ProtectClient-E/sbin/xmlcfg\" write Monitor rdagent logfile_size \"${LOGFILE_SIZE}\""
    SUExecCmd "\"${NEW_INSTALL_PATH}/ProtectClient-E/sbin/xmlcfg\" write Monitor rdagent logfile_size_alarm_threshold \"${LOGFILE_SIZE_ALARM_THRESHOLD}\""
    SUExecCmd "\"${NEW_INSTALL_PATH}/ProtectClient-E/sbin/xmlcfg\" write SNMP engine_id \"${ENGINE_ID}\""
    SUExecCmd "\"${NEW_INSTALL_PATH}/ProtectClient-E/sbin/xmlcfg\" write SNMP context_name \"${CONTEXT_NAME}\""
    SUExecCmd "\"${NEW_INSTALL_PATH}/ProtectClient-E/sbin/xmlcfg\" write SNMP security_name \"${SECURITY_NAME}\""
    SUExecCmd "\"${NEW_INSTALL_PATH}/ProtectClient-E/sbin/xmlcfg\" write SNMP security_model \"${SECURITY_MODEL}\""
    SUExecCmd "\"${NEW_INSTALL_PATH}/ProtectClient-E/sbin/xmlcfg\" write SNMP security_level \"${SECURITY_LEVEL}\""
    SUExecCmd "\"${NEW_INSTALL_PATH}/ProtectClient-E/sbin/xmlcfg\" write SNMP private_protocol \"${PRIVATE_PROTOCOL}\""
    SUExecCmd "\"${NEW_INSTALL_PATH}/ProtectClient-E/sbin/xmlcfg\" write SNMP auth_protocol \"${AUTH_PROTOCOL}\""
    SUExecCmd "\"${NEW_INSTALL_PATH}/ProtectClient-E/sbin/xmlcfg\" write SNMP auth_password \"${AUTH_PASSWORD}\""
    SUExecCmd "\"${NEW_INSTALL_PATH}/ProtectClient-E/sbin/xmlcfg\" write SNMP private_password \"${PRIVATE_PASSWORD}\""
    SUExecCmd "\"${NEW_INSTALL_PATH}/ProtectClient-E/sbin/xmlcfg\" write Backup archive_threshold \"${ARCHIVE_THRESHOLD}\""
    SUExecCmd "\"${NEW_INSTALL_PATH}/ProtectClient-E/sbin/xmlcfg\" write Backup archive_thread_timeout \"${ARCHIVE_THREAD_TIMEOUT}\""
}

InstallUpgradeProxy()
{
    export DATA_BACKUP_AGENT_HOME_BAK
    echo "Step 3: Install the new agent."
    Log "Step 3: Install the new agent."
    cd ${NEW_PACKAGE_PATH}

    chmod +x ${NEW_PACKAGE_PATH}/install.sh
    ${NEW_PACKAGE_PATH}/install.sh

    INSTALL_RESULT=$?
    if [ 0 -ne ${INSTALL_RESULT} ]; then
        printf "\033[31mFailed to install the new agent. \033[0m\n"
        LogError "Failed to install the new agent, err code [${INSTALL_RESULT}]." ${ERR_UPGRADE_FAIL_VERIFICATE_PARAMETER}
        RollBackAgent
        exit 1
    fi

    #restore CRL in upgrade Agent
    if [ "${CRL_STATUS}" == "1" ]; then
        CRL_UPDATE_STATUS=1
        if [ -f "${NEW_INSTALL_PATH}/crl_update.sh" ] && [ -f "${AGENT_BACKUP_PATH}/ProtectClient-E/nginx/conf/server.crl" ]; then
            ${NEW_INSTALL_PATH}/crl_update.sh -u ${AGENT_BACKUP_PATH}/ProtectClient-E/nginx/conf/server.crl
            CRL_UPDATE_STATUS=$?
        else
            Log "Missing required documents in restore CRL."
        fi
        if [ 0 -ne ${CRL_UPDATE_STATUS} ]; then
            Log "Restore CRL status failed."
            printf "\033[31mFailed to restore CRL. \033[0m\n"
            LogError "Failed to restore CRL, err code [${CRL_UPDATE_STATUS}]." ${ERR_UPGRADE_FAIL_VERIFICATE_PARAMETER}
            RollBackAgent
            exit 1
        fi
    fi
}

RestoreHcsFile()
{
    if [ -f "$AGENT_BACKUP_PATH"/Plugins/VirtualizationPlugin/conf/clean.ini ]; then
        CP "$AGENT_BACKUP_PATH"/Plugins/VirtualizationPlugin/conf/clean.ini "${NEW_INSTALL_PATH}"/Plugins/VirtualizationPlugin/conf
    fi

    return 0
}

RestoreFusionOneFile()
{   
    echo "restore fusionone file"
    if [ -f "$AGENT_BACKUP_PATH"/Plugins/FusionComputePlugin/lib/service/librbd.so ]; then
        CP "$AGENT_BACKUP_PATH"/Plugins/FusionComputePlugin/lib/service/librbd.so "${NEW_INSTALL_PATH}"/Plugins/FusionComputePlugin/lib/service
        chmod 550 "${NEW_INSTALL_PATH}"/Plugins/FusionComputePlugin/lib/service/librbd.so
        chown root:rdadmin "${NEW_INSTALL_PATH}"/Plugins/FusionComputePlugin/lib/service/librbd.so
    fi

    return 0
}

RestoreThirdPartFile()
{
    Log "Backup third part file."
    if [ -d /opt/OceanProtect/thirdPart ]; then
        CP -r -p /opt/OceanProtect/thirdPart/* /opt/DataBackup/thirdPart
        rm -rf /opt/OceanProtect
    fi
    if [ -d "$AGENT_BACKUP_PATH"/ProtectClient-E/sbin/thirdparty ]; then
        CP -r -p "${AGENT_BACKUP_PATH}/ProtectClient-E/sbin/thirdparty" "${NEW_INSTALL_PATH}/ProtectClient-E/sbin/"
    fi
}

GetDWSConf()
{
    XML_FILE=$1
    PARENT_FILED=$2
    CHILD_FILED=$3
    if [ "${SYS_NAME}" = "SunOS" ] || [ "${SYS_NAME}" = "AIX" ]; then
        output=`su - ${AGENT_USER} -c "${EXPORT_ENV} cat ${XML_FILE} | grep -A 100 \"<${PARENT_FILED}>\" | grep -B 100 \"</${PARENT_FILED}>\" | grep \"${CHILD_FILED}\" | $AWK -F '=' '{print \\$2}' | $AWK -F '\"' '{print \\$2}'"` >/dev/null 2>&1
    else
        output=`su - ${AGENT_USER} -s ${SHELL_TYPE_SH} -c "${EXPORT_ENV} cat ${XML_FILE} | grep -A 100 \"<${PARENT_FILED}>\" | grep -B 100 \"</${PARENT_FILED}>\" | grep \"${CHILD_FILED}\" | $AWK -F '=' '{print \\$2}' | $AWK -F '\"' '{print \\$2}'"` >/dev/null 2>&1
    fi
    ret=$?
    echo $output
    return $ret
}

RestoreDWSconfFile()
{
    ${NEW_INSTALL_PATH}/ProtectClient-E/sbin/update_json_file "$AGENT_BACKUP_PATH/Plugins/GeneralDBPlugin/bin/applications/dws/dws.conf" "${NEW_INSTALL_PATH}/Plugins/GeneralDBPlugin/bin/applications/dws/dws.conf" >>${LOG_FILE_NAME} 2>&1
    ret=$?
    if [ $ret -ne 0 ]; then
        Log "Restore dws conf data failed"
    fi
    return $ret
}

RestoreProxyData()
{
    echo "Step 4: Restoring the ProtectAgent configuration before the upgrade."
    Log "Step 4: Restoring the ProtectAgent configuration before the upgrade."

    # restore snmp configuration
    SetSNMPCfg

    # if vmware, cp lib; if oracle copy nginx port
    if [ 2 -eq $BACKUP_ROLE ] && [ -d "${AGENT_BACKUP_PATH}/ProtectClient-E/lib" ]; then
        echo "Copy the VDDK library on which VMware backup depends to a specified path."
        CP -r -p "${AGENT_BACKUP_PATH}/ProtectClient-E/lib" "${NEW_INSTALL_PATH}/ProtectClient-E/"
        if [ -d "${NEW_INSTALL_PATH}/ProtectClient-E/lib/vddk" ]; then
            cd ${NEW_INSTALL_PATH}/ProtectClient-E/lib/vddk
            if [ -L 5.5 ]; then
                ln -snf 6.0 5.5
            fi
            if [ -L 5.1 ]; then
                ln -snf 6.0 5.1
            fi
            if [ -L 6.5 ]; then
                ln -snf 7.0 6.5
            fi
            if [ -L 6.7 ]; then
                ln -snf 7.0 6.7
            fi
            chown rdadmin:rdadmin * -R
            cd -
        fi
    fi

    # handle sqlite
    SUExecCmd "cp -r ${AGENT_BACKUP_PATH}/ProtectClient-E/db ${NEW_INSTALL_PATH}/ProtectClient-E/db/db_upgrade_pre"
    SUExecCmd "chown -R ${AGENT_USER}:${AGENT_GROUP} ${NEW_INSTALL_PATH}/ProtectClient-E/db/db_upgrade_pre"

    SUExecCmd "${NEW_INSTALL_PATH}/ProtectClient-E/bin/agent_upgrade_sqlite.sh ${NEW_INSTALL_PATH}/ProtectClient-E/db/db_upgrade_pre ${NEW_INSTALL_PATH}/ProtectClient-E/db"
    if [ 0 -ne $? ]; then
        printf "\033[31m Update sqlite db failed. \033[0m\n"
        LogError "Update sqlite db failed." ${ERR_UPGRADE_FAIL_VERIFICATE_PARAMETER}
        RollBackAgent
        exit 1
    fi

    # Delete pre-upgrade database files
    SUExecCmd "rm -rf \"${NEW_INSTALL_PATH}/ProtectClient-E/db/db_upgrade_pre\""
    # Restore DWS relation value
    dws_agent_relation=`GetDWSConf ${AGENT_BACKUP_PATH}/ProtectClient-E/conf/agent_cfg.xml Backup agent_storage_relation`
    if [ 0 -ne $? ]; then
        printf "\033[31mFailed to get dws agent relation. \033[0m\n"
        return 1
    fi
    SUExecCmd "\"${NEW_INSTALL_PATH}/ProtectClient-E/bin/xmlcfg\" write Backup agent_storage_relation \"${dws_agent_relation}\""

    # Save eip
    echo EIP=${EIP} | tr -d '\r' >> ${NEW_INSTALL_PATH}/ProtectClient-E/conf/testcfg.tmp
    SUExecCmd "\"${NEW_INSTALL_PATH}/ProtectClient-E/bin/xmlcfg\" write Mount eip \"${EIP}\""

    echo IS_FILECLIENT_INSTALL=${IS_FILECLIENT_INSTALL} | tr -d '\r' >> ${NEW_INSTALL_PATH}/ProtectClient-E/conf/testcfg.tmp

    # copy old logs
    CP ${AGENT_BACKUP_PATH}/ProtectClient-E/stmp/AGENTLOG* ${NEW_INSTALL_PATH}/ProtectClient-E/stmp

    # restore hcs file
    RestoreHcsFile
    # restore fusionone file
    RestoreFusionOneFile

    RestoreThirdPartFile

    # 判断是否DWS节点，是则执行
    APPLICATION_INFO=`cat ${INSTALL_PACKAGE_PATH}/conf/client.conf | grep "application_info" | ${AWK} -F '=' '{print $NF}'`
    echo $APPLICATION_INFO | grep "GaussDB(DWS)" >/dev/null 2>&1
    if [ $? -eq 0 ];then
        RestoreDWSconfFile
    fi
}

ReRegisterProxy()
{
    Log "Step 7: Re register the ProtectAgent to ProtectManager."
    RegisterHost "${NEW_INSTALL_PATH}" "upgrade" >/dev/null 2>&1
    if [ $? -ne 0 ]; then
        echo "[Note:]The ProtectAgent will connect to the ProtectManager in three minutes."
        Log "[Note:]The ProtectAgent will connect to the ProtectManager in three minutes."
    fi
}

# check exec user
CheckExecUser
if [ $? -ne 0 ]; then
    printf "\033[31m Please execute this script as user root. [0m\n"
    return 1
fi

# create softlink stdcpp
LOG_FILE_NAME="${NEW_PACKAGE_PATH}/agent_upgrade.log"
AGENT_ROOT_PATH="${NEW_PACKAGE_PATH}/ProtectClient-e/ProtectClient-E" # This parameter is provided by agent_sbin_func.sh
. "${NEW_PACKAGE_PATH}/ProtectClient-e/ProtectClient-E/sbin/agent_sbin_func.sh"

################## Begin Upgration #############################
Log "Begin to upgrade DataBackup ProtectAgent."
################################################################

# check shell type
CheckShellType

# do prepare work before installing new agent
PrepareBeforeInstallNewAgent
UPGRADE_RET=$?
if [ 0 -ne ${UPGRADE_RET} ]; then
    WorkAfterUpgrade
    LogError "Prepare params fail." ${ERR_UPGRADE_FAIL_VERIFICATE_PARAMETER}
    exit ${UPGRADE_RET}
fi

# handle dataturbo
HandleDataTurbo

# uninstall old agent
UninstallOldAgent

# install new agent
InstallUpgradeProxy

# restore data before upgrade
RestoreProxyData

# restart agent
RestartNewAgent

# Register the Agent with the ProtectManager again.
# Prevent the first ProtectManager access from being denied after the upgrade.
ReRegisterProxy

WorkAfterUpgrade
printf "\\033[1;32mDataBackup ProtectAgent is successfully upgraded on the host.\\033[0m \n"
Log "DataBackup ProtectAgent is successfully upgraded on the host."

[ -f "${LATEST_BAK_DIR}/slog/agent_upgrade.log" ] && cat "${LATEST_BAK_DIR}/slog/agent_upgrade.log" >> ${LOG_FILE_NAME}

exit 0

