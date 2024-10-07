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
# Uinstall nas plugin for agent
CUR_UNINSTALL_PATH=`dirname $0`
SCRIPT_PATH=`cd ${CUR_UNINSTALL_PATH} && pwd`
COMMON_PATH=${SCRIPT_PATH}
. ${COMMON_PATH}/common.sh
SCRIPT_NAME=`basename $0`
PLUGIN_INSTALL_PATH=${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient/Plugins

invoke_app_uninstall()
{
    PLUGIN_NAME=`get_plugin_name`
    if [ $? -ne 0 ]; then
        exit 1
    fi

    # uninstall.log 日志是root权限，输出在slog下
    LOG_FILE_PREFIX=${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient/ProtectClient-E/slog/Plugins/${PLUGIN_NAME}
    if [ ! -d $DATA_BACKUP_AGENT_HOME ];then
        echo "Agent home dir do not exist"
        exit 1
    fi
    mkdir -p ${LOG_FILE_PREFIX}
    LOG_FILE=${LOG_FILE_PREFIX}/uninstall.log

    appInstall=${PLUGIN_INSTALL_PATH}/${PLUGIN_NAME}/install
    if [ -f ${appInstall}/uninstall.sh ]; then
        if [ "${OS_TYPE}" = "AIX" ]; then
            chmod +x ${appInstall}/uninstall.sh
            ${appInstall}/uninstall.sh "$@"
        else
            source ${appInstall}/uninstall.sh "$@"
        fi
        ret=$?
        if [ ${ret} -ne 0 ]; then
            echo "[$(date "+%Y-%m-%d %H:%M:%S")][ERROR][ excute app uninstall.sh failed. ret is ${ret} ]" >> ${LOG_FILE}
            exit ${ret}
        fi
    fi
    return 0
}

main()
{
    invoke_app_uninstall

    PLUGIN_NAME=`get_plugin_name`
    if [ $? -ne 0 ]; then
        exit 1
    fi

    cd "${SCRIPT_PATH}"
     ./stop.sh

    rm -rf ${PLUGIN_INSTALL_PATH}/${PLUGIN_NAME}
    return 0
}

main "$@"
