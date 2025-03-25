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
SYS_NAME=`uname -s`
if [ "${SYS_NAME}" = "SunOS" ]; then
    AWK=nawk
    HOME_DIR="/export/home/"
else
    AWK=awk
    HOME_DIR="/home/"
fi
###### Custom installation directory ######
TMP_DATA_BACKUP_AGENT_HOME=`cat /etc/profile | grep "export DATA_BACKUP_AGENT_HOME=" |${AWK} -F "=" '{print $NF}'`
if [ -n "${TMP_DATA_BACKUP_AGENT_HOME}" ]; then
    . /etc/profile
    if [ -n "${DATA_BACKUP_AGENT_HOME}" ]; then
        DATA_BACKUP_AGENT_HOME=${TMP_DATA_BACKUP_AGENT_HOME}
        export DATA_BACKUP_AGENT_HOME
    fi
else
    DATA_BACKUP_AGENT_HOME=/opt
    export DATA_BACKUP_AGENT_HOME
fi

#############################################
# this shell for agent migration
#############################################
CURRENT_PATH=`dirname $0` && cd $CURRENT_PATH && CURRENT_PATH=`pwd`
LOG_FILE_NAME=${CURRENT_PATH}/host_migration.log
source ${CURRENT_PATH}/func_util.sh
SHELL_TYPE_SH="/bin/sh"
INSTALL_PATH="${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient"
AGENT_BACKUP_PATH="${DATA_BACKUP_AGENT_HOME}/AgentUpgrade"
OLD_INSTALL_PATH="${DATA_BACKUP_AGENT_HOME}"

SANCLIENT_INSTALL_PATH="${DATA_BACKUP_AGENT_HOME}/DataBackup/SanClient"
AGENT_BACKUP_PATH="${DATA_BACKUP_AGENT_HOME}/AgentUpgrade"
SANCLIENT_AGENT_BACKUP_PATH="${DATA_BACKUP_AGENT_HOME}/SanclientUpgrade"
BACKUP_LOG="/var/log/ProtectAgent/log"
BACKUP_SANCLIENT_LOG="/var/log/SanClient/log"
SYS_NAME=`uname -s`
AGENT_USER=rdadmin
AGENT_GROUP=rdadmin

SANCLIENT_USER=sanclient
SANCLIENT_GROUP=sanclient
TESTCFG_BACK_ROLE=`su - ${SANCLIENT_USER} 2>/dev/null -s ${SHELL_TYPE_SH} -c "sed '/^BACKUP_ROLE=/!d;s/.*=//' /opt/DataBackup/SanClient/ProtectClient-E/conf/testcfg.tmp "`
if [ "${TESTCFG_BACK_ROLE}" = "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ]; then
    AGENT_GROUP=${SANCLIENT_GROUP}
    AGENT_USER=${SANCLIENT_USER}
    INSTALL_PATH=${SANCLIENT_INSTALL_PATH}
    BACKUP_LOG=${BACKUP_SANCLIENT_LOG}
    AGENT_BACKUP_PATH=${SANCLIENT_AGENT_BACKUP_PATH}
fi
export AGENT_ROOT="${INSTALL_PATH}/ProtectClient-E/"
LOG_FILE_NAME=${CURRENT_PATH}/host_migration.log

if [ "`ls -al /bin/sh | ${AWK} '{print $NF}'`" = "bash" ]; then
    EXPORT_ENV="if [ -f ~/.profile ]; then . ~/.profile; fi;"
else
    EXPORT_ENV=""
fi

BackupRadminUserId()
{
    cat /etc/passwd | grep rdadmin | $AWK -F ":" '{print $3,$4}' > $AGENT_BACKUP_PATH/rdadminInfo
}

BackupRadminUserEnv()
{
    CP -a -r -f -p -P "${HOME_DIR}"/"${AGENT_USER}" "$AGENT_BACKUP_PATH"
}

RecoverRdadminUserEnv()
{
    if [ -d "${HOME_DIR}/${AGENT_USER}" ]; then
        SUExecCmd "rm -rf ${HOME_DIR}/${AGENT_USER}/* ${HOME_DIR}/${AGENT_USER}/.*"
        rm -rf ${HOME_DIR}/${AGENT_USER}
    fi
    mv $AGENT_BACKUP_PATH/${AGENT_USER} ${HOME_DIR}/
}

BackupAgentPkg()
{
    CP -a -r -f -p -P "${INSTALL_PATH}" "${AGENT_BACKUP_PATH}"
}

BackUpCurAgentInfo()
{
    echo "Begin to backup DataBackup ProtectAgent."
    Log "MIGRATE: Begin to back up DataBackup ProtectAgent."
    if [ -d "${AGENT_BACKUP_PATH}" ]; then
        rm -rf "${AGENT_BACKUP_PATH}"
    fi
    mkdir "$AGENT_BACKUP_PATH"
    CHMOD 755 "$AGENT_BACKUP_PATH"
    BackupAgentPkg
    BackupRadminUserId
    BackupRadminUserEnv
    Log "MIGRATE: The installed DataBackup ProtectAgent are backed up successfully."
    echo "The installed DataBackup ProtectAgent are backed up successfully."
}

UninstallOldAgent()
{
    Log "MIGRATE: Begin to uninstall old ProtectAgent."
    echo "Begin to uninstall old ProtectAgent."
    echo y | "${INSTALL_PATH}"/uninstall.sh
    if [ $? -ne 0 ]; then
        Log "MIGRATE: Uninstall old ProtectAgent failed."
        echo "Uninstall old ProtectAgent failed."
        exit 1
    fi
}

InstallPath()
{
    echo "custom_install_path=${OLD_INSTALL_PATH}" >> ${CURRENT_PATH}/conf/client.conf
}

InstallNewAgent()
{
    Log "MIGRATE: Begin to install new ProtectAgent."
    echo "Begin to install new ProtectAgent."
    chmod +x install.sh
    ${CURRENT_PATH}/install.sh $@
    installRet=$?
    if [ "$installRet" -ne "0" ];then
        echo  "Failed to install the new agent, err code [${installRet}]."
        Log "MIGRATE: Failed to install the new agent, err code [${installRet}]."
    fi
    return $installRet
}

StartAgent()
{
    cd ${INSTALL_PATH}/
    chmod +x start.sh
    ./start.sh >/dev/null 2>&1
    if [ $? -eq 0 ]; then
        Log "MIGRATE: The DataBackup ProtectAgent service is successfully started."
    else
        echo "The DataBackup ProtectAgent service fails to be started."
        Log "MIGRATE: The DataBackup ProtectAgent service fails to be started."
        exit 1
    fi
}

RollBackAgent()
{
    Log "MIGRATE: Start to rollback agent."
    echo "Start to rollback agent."
    AddSpecifyIdUserAndGroup `cat ${AGENT_BACKUP_PATH}/rdadminInfo` ${AGENT_BACKUP_PATH}/ProtectClient/ProtectClient-E/conf/testcfg.tmp
    rm -f "${AGENT_BACKUP_PATH}"/rdadminInfo
    RecoverRdadminUserEnv
    mkdir -p "${INSTALL_PATH}"
    mv -f "${AGENT_BACKUP_PATH}"/ProtectClient/* "${INSTALL_PATH}/"
    StartAgent
    RegisterHost
    echo "Rollback agent success."
    Log "MIGRATE: Rollback agent success."
    CleanPkg
    return
}

RegisterHost()
{
    if [ "${SYS_NAME}" = "SunOS" ] || [ "${SYS_NAME}" = "AIX" ]; then
        su - ${AGENT_USER} -c "${EXPORT_ENV}${INSTALL_PATH}/ProtectClient-E/bin/agentcli registerHost RegisterHost" >/dev/null
    else
        su - ${AGENT_USER} -s ${SHELL_TYPE_SH} -c "${EXPORT_ENV}${INSTALL_PATH}/ProtectClient-E/bin/agentcli registerHost RegisterHost"
    fi
    if [ "$?" != 0 ]; then
        Log "MIGRATE: Register host to ProtectManager failed."
        echo "The host cannot be registered to the ProtectManager."
        exit 1
    fi
    echo "Register host to ProtectManager success."
    Log "MIGRATE: Register host to ProtectManager success."
}

CleanPkg()
{
    rm -rf "${AGENT_BACKUP_PATH}"/*
}

BackUpLog()
{
    if [ ! -d "${BACKUP_LOG}" ]; then
        mkdir -p "${BACKUP_LOG}"
    fi
    CP -a -f -p $LOG_FILE_NAME /var/log/ProtectAgent/log/
}

BackUpCurAgentInfo

UninstallOldAgent

InstallPath

InstallNewAgent $@
if [ $? -ne 0 ]; then
    RollBackAgent
    BackUpLog
    exit 1
fi

CleanPkg
echo "Migrate ProtectAgent success."
Log "MIGRATE: Migrate ProtectAgent success."
BackUpLog
exit 0