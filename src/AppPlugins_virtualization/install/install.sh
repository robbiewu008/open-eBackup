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
# /
set +x

VIRT_PLUGIN_PATH="$(
    cd "$(dirname "$BASH_SOURCE")/../"
    pwd
)"
PLUGIN_NAME="VirtualizationPlugin"
AGENT_INSTALL_PATH=${DATA_BACKUP_AGENT_HOME}
AGENT_USER=rdadmin
AGENT_GROUP=rdadmin
EXAGENT_USER=exrdadmin
USER_NOLOGIN_SHELL="/sbin/nologin"
PLUGIN_USER=${AGENT_USER}
SYS_ARCH=`uname -m`
STATUS_FILE="${AGENT_INSTALL_PATH}/DataBackup/ProtectClient/ProtectClient-E/slog/Plugins/${PLUGIN_NAME}/status.log"

if [ -z "${AGENT_INSTALL_PATH}" ]; then
    AGENT_INSTALL_PATH="/opt"
fi
 
if [ ! -d "${AGENT_INSTALL_PATH}" ]; then
    echo "ERROR" "Agent Install Path [${AGENT_INSTALL_PATH}] : No such file or directory."
    return 1
fi
 
LOG_FILE=${AGENT_INSTALL_PATH}/DataBackup/ProtectClient/ProtectClient-E/slog/Plugins/${PLUGIN_NAME}/install.log

function create_file()
{
    # 1. create clean.ini
    touch "${VIRT_PLUGIN_PATH}"/conf/clean.ini
    chown root:rdadmin "${VIRT_PLUGIN_PATH}"/conf/clean.ini
    chmod 660 "${VIRT_PLUGIN_PATH}"/conf/clean.ini
}

function change_permission()
{
    if [ ! -f "/etc/sudoers" ]; then
        echo "[Warning] The file [/etc/sudoers] is not exist, plugin can not start with rdadmin."
    fi

    # 1. internal agent use 22222 user to run virtualizationplugin.
    local backup_scene=`cat ${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient/ProtectClient-E/conf/testcfg.tmp | grep "BACKUP_SCENE" | cut -d'=' -f2`
    if [ $backup_scene == "1" ]; then
        useradd -m -s ${USER_NOLOGIN_SHELL} -u 22222 -g 99 -G ${AGENT_GROUP} ${EXAGENT_USER}
        PLUGIN_USER=${EXAGENT_USER}
        sed -i "s/rdadmin/"${PLUGIN_USER}"/g" ${VIRT_PLUGIN_PATH}/conf/plugin_attribute_1.0.0.json
      	sed -i "s/rdadmin/"${PLUGIN_USER}"/g" ${VIRT_PLUGIN_PATH}/plugin_attribute_1.0.0.json
        echo "${PLUGIN_USER}   ALL=NOPASSWD: NOPASSWD:${AGENT_INSTALL_PATH}/DataBackup/ProtectClient/ProtectClient-E/sbin/rootexec" >> /etc/sudoers
    fi

    # 2. set the sudo permission for user rdadmin.
    echo "${AGENT_USER}    ALL=NOPASSWD: NOPASSWD:${VIRT_PLUGIN_PATH}/install/sudo_set_caps.sh" >> /etc/sudoers
    echo "${PLUGIN_USER}   ALL=NOPASSWD: NOPASSWD:${VIRT_PLUGIN_PATH}/install/sudo_set_caps.sh" >> /etc/sudoers
    echo "${AGENT_USER}   ALL=NOPASSWD: NOPASSWD:${VIRT_PLUGIN_PATH}/install/python_env.sh" >> /etc/sudoers
    echo "${PLUGIN_USER}   ALL=NOPASSWD: NOPASSWD:${VIRT_PLUGIN_PATH}/install/python_env.sh" >> /etc/sudoers
    echo "${PLUGIN_USER}   ALL=NOPASSWD: NOPASSWD:${VIRT_PLUGIN_PATH}/bin/reg_fs.sh" >> /etc/sudoers
    echo "${PLUGIN_USER}   ALL=NOPASSWD: NOPASSWD:${VIRT_PLUGIN_PATH}/bin/security_sudo_disk.sh" >> /etc/sudoers
    echo "${PLUGIN_USER}   ALL=NOPASSWD: NOPASSWD:${VIRT_PLUGIN_PATH}/bin/security_sudo_vbs_cli.sh" >> /etc/sudoers
    echo "${PLUGIN_USER}   ALL=NOPASSWD: NOPASSWD:${VIRT_PLUGIN_PATH}/bin/superlog.sh" >> /etc/sudoers
    echo "${PLUGIN_USER}   ALL=NOPASSWD: NOPASSWD:${VIRT_PLUGIN_PATH}/vbstool/vrmVBSTool.sh" >> /etc/sudoers

    # 2. set script permission
    chmod 550 ${VIRT_PLUGIN_PATH}/vbstool/vrmVBSTool.sh
    chmod 550 ${VIRT_PLUGIN_PATH}/bin/reg_fs.sh
    chmod 550 ${VIRT_PLUGIN_PATH}/bin/security_sudo_disk.sh
    chmod 550 ${VIRT_PLUGIN_PATH}/bin/security_sudo_vbs_cli.sh
    chmod 550 ${VIRT_PLUGIN_PATH}/bin/superlog.sh
    chmod 550 ${VIRT_PLUGIN_PATH}/install/sudo_set_caps.sh
    chmod 550 ${VIRT_PLUGIN_PATH}/install/python_env.sh

    return 0
}

create_python_env()
{
    local backup_scene=`cat ${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient/ProtectClient-E/conf/testcfg.tmp | grep "BACKUP_SCENE" | cut -d'=' -f2`
    if [ "X${backup_scene}" = "X1" ];then
        log_echo "INFO" "The current env is internal agent. Python does not need to be installed." >> ${LOG_FILE}
        return 0
    fi
    cd ${VIRT_PLUGIN_PATH}/install/
    python3_file=python3.pluginFrame.${SYS_ARCH}.tar.gz
    tar xf ${python3_file}
    rm -f python3.pluginFrame.*.tar.gz
    l_user_name=`whoami`
    if [ $l_user_name == "root" ] || [ $l_user_name == "" ]; then
        ${VIRT_PLUGIN_PATH}/install/python_env.sh >> ${LOG_FILE} 2>&1
    else
        sudo ${VIRT_PLUGIN_PATH}/install/python_env.sh >> ${LOG_FILE} 2>&1
    fi
    if [ $? -ne 0 ]; then
        echo "Failed to execute the python_env script !"
        exit 1
    fi
    cd -
}

function set_caps()
{
    local backup_scene=`cat ${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient/ProtectClient-E/conf/testcfg.tmp | grep "BACKUP_SCENE" | cut -d'=' -f2`
    if [ $backup_scene == "1" ]; then
        # internal agent set caps when internal_run.sh start
        return 0
    fi
    # set caps for virtual plugin
    l_user_name=`whoami`
    echo "[`date`] [INFO] current user:$l_user_name" >> ${LOG_FILE}
    if [ $l_user_name == "root" ] || [ $l_user_name == "" ]; then
        ${VIRT_PLUGIN_PATH}/install/sudo_set_caps.sh >> ${LOG_FILE} 2>&1
    else
        sudo ${VIRT_PLUGIN_PATH}/install/sudo_set_caps.sh >> ${LOG_FILE} 2>&1
    fi
}

function disable_lvm_auto_detect()
{
    sed -i 's/use_lvmetad.*=.*1/use_lvmetad = 0/g' /etc/lvm/lvm.conf 2>> ${LOG_FILE}
    systemctl stop lvm2-lvmetad.socket 2>> ${LOG_FILE}
    systemctl stop lvm2-lvmetad.service 2>> ${LOG_FILE}
    systemctl disable lvm2-lvmetad.socket 2>> ${LOG_FILE}
    systemctl disable lvm2-lvmetad.service 2>> ${LOG_FILE}
}

function tag_current_file_status()
{
    ls -l ${VIRT_PLUGIN_PATH}/bin >> ${STATUS_FILE}
    ls -l ${VIRT_PLUGIN_PATH}/lib >> ${STATUS_FILE}
    ls -l ${VIRT_PLUGIN_PATH}/lib/service >> ${STATUS_FILE}
    echo "======================" >> ${STATUS_FILE}
}

main()
{
    tag_current_file_status

    create_file

    tag_current_file_status

    change_permission

    tag_current_file_status

    create_python_env

    tag_current_file_status

    set_caps

    tag_current_file_status

    disable_lvm_auto_detect
    
    tag_current_file_status

    return 0
}

main
return $?