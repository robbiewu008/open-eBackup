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

AGENT_USER=rdadmin
SANCLIENT_USER=sanclient
CURRENT_PATH=`dirname $0` && cd $CURRENT_PATH && CURRENT_PATH=`pwd`

SYS_NAME=`uname -s`
if [ "${SYS_NAME}" = "SunOS" ]; then
    AWK=nawk
else
    AWK=awk
fi

###### Custom installation directory ######
TMP_DATA_BACKUP_AGENT_HOME=`cat /etc/profile | grep "DATA_BACKUP_AGENT_HOME=" |${AWK} -F "=" '{print $NF}'`
TMP_DATA_BACKUP_SANCLIENT_HOME=`cat /etc/profile | grep "export DATA_BACKUP_SANCLIENT_HOME=" |${AWK} -F "=" '{print $NF}'`
if [ -n "${TMP_DATA_BACKUP_AGENT_HOME}" ] || [ -n "${TMP_DATA_BACKUP_SANCLIENT_HOME}" ] ; then
    . /etc/profile
    if [ -n "${DATA_BACKUP_AGENT_HOME}" ] || [ -n "${DATA_BACKUP_SANCLIENT_HOME}" ] ; then
        DATA_BACKUP_AGENT_HOME=${TMP_DATA_BACKUP_AGENT_HOME}
        DATA_BACKUP_SANCLIENT_HOME=${TMP_DATA_BACKUP_SANCLIENT_HOME}
        export DATA_BACKUP_AGENT_HOME DATA_BACKUP_SANCLIENT_HOME
    fi
else
    DATA_BACKUP_AGENT_HOME=/opt
    DATA_BACKUP_SANCLIENT_HOME=/opt
    export DATA_BACKUP_AGENT_HOME DATA_BACKUP_SANCLIENT_HOME
fi

AGENT_PATH=${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient

SANCLIENT_AGENT_PATH="${DATA_BACKUP_SANCLIENT_HOME}/DataBackup/SanClient"
SHELL_TYPE_SH="/bin/sh"
BACKUP_ROLE_SANCLIENT_PLUGIN=5
TESTCFG_BACK_ROLE=`su - ${SANCLIENT_USER} 2>/dev/null -s ${SHELL_TYPE_SH} -c "sed '/^BACKUP_ROLE=/!d;s/.*=//' ${CURRENT_PATH}/ProtectClient-E/conf/testcfg.tmp "`
CLIENT_BACK_ROLE=`cat ${CURRENT_PATH}/conf/client.conf 2>/dev/null | grep "backup_role" | ${AWK} -F '=' '{print $NF}'`
if [ "${TESTCFG_BACK_ROLE}" = "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ] || [ "${CLIENT_BACK_ROLE}" = "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ]; then
    AGENT_USER=${SANCLIENT_USER}
    AGENT_PATH=${SANCLIENT_AGENT_PATH}
    INSTALL_PATH=${AGENT_PATH}
else
    if [ -d "${AGENT_PATH}/ProtectClient-E/bin" ]; then
        INSTALL_PATH=${AGENT_PATH}
    else
        INSTALL_PATH="/opt/OceanProtect/ProtectClient"
    fi
fi
UNIX_CMD=
ROCKY4_FLAG=0

if [ "`ls -al /bin/sh | ${AWK} '{print $NF}'`" = "bash" ]; then
    EXPORT_ENV="if [ -f ~/.profile ]; then . ~/.profile; fi;"
else
    EXPORT_ENV=""
fi

###############################################################
CollectLog()
{
    if [ "${SYS_NAME}" = "Linux" ]; then
        rocky4=`cat /etc/issue | grep 'Rocky'`
        if [ -n "${rocky4}" ]; then
            ROCKY4_FLAG=1
        fi
    fi

    if [ "${SYS_NAME}" = "AIX" ] || [ "${SYS_NAME}" = "HP-UX" ] || [ "${SYS_NAME}" = "SunOS" ] || [ 1 -eq ${ROCKY4_FLAG} ]; then
        ECHO_E=""
        if [ "${SHELL_TYPE}" = "bash" ]; then
            UNIX_CMD=-l
        fi
    fi

    if [ "${SYS_NAME}" = "SunOS" ] || [ "${SYS_NAME}" = "AIX" ]; then
        su - ${AGENT_USER} -c "${EXPORT_ENV}echo y | ${INSTALL_PATH}/ProtectClient-E/bin/agentcli collectlog"
    else
        su - ${AGENT_USER} -s ${SHELL_TYPE_SH} ${UNIX_CMD} -c "${EXPORT_ENV}echo y | ${INSTALL_PATH}/ProtectClient-E/bin/agentcli collectlog"
    fi

    if [ 0 != $? ]; then
        echo "Collect DataBackup ProtectAgent Log failed."
        return 1
    else
        echo "Collect DataBackup ProtectAgent Log successfully. Log is stored in[${INSTALL_PATH}/ProtectClient-E/stmp]."
        return 0
    fi
}

CollectLog
exit $?