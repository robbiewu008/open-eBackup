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
set +x

source "/etc/profile"

AGENT_INSTALL_PATH=${DATA_BACKUP_AGENT_HOME}
 
if [ -z "${AGENT_INSTALL_PATH}" ]; then
    AGENT_INSTALL_PATH="/opt"
fi

if [ ! -d "${AGENT_INSTALL_PATH}" ]; then
    echo "ERROR" "Agent Install Path [${AGENT_INSTALL_PATH}] : No such file or directory."
    return 1
fi

SHELL_TYPE_SH="/bin/sh"
G_HOME_PATH="${AGENT_INSTALL_PATH}/DataBackup/ProtectClient"
VIRT_ROOT_PATH="${G_HOME_PATH}/Plugins/VirtualizationPlugin"
LOGFILE_SUFFIX="gz"
BACKLOGSIZE=`cat ${VIRT_ROOT_PATH}/conf/hcpconf.ini | grep "LogMaxSize" | awk -F "=" '{print $NF}'`  # MB
BACKLOGCOUNT=`cat ${VIRT_ROOT_PATH}/conf/hcpconf.ini | grep "LogCount" | awk -F "=" '{print $NF}'`
LOG_SCRIPT_FILE="${G_HOME_PATH}/ProtectClient-E/slog/Plugins/VirtualizationPlugin/script.log"  # root privilege

function char_to_num()
{
    echo "$1" | awk '{print int($0)}'
}

function log()
{
    local scriptFile="$1"
    local logLevel="$2"
    local logMsg="$3"
    local currentUser=$(whoami | awk '{print $1}')
    local logDateTime=$(date +%Y/%m/%d-%H:%M:%S)
    local logFormat="[${logDateTime}][${logLevel}][${currentUser}][${scriptFile}] ${logMsg}"

    if [ -L "${LOG_SCRIPT_FILE}" ]; then
        echo "The file[${LOG_SCRIPT_FILE}] is a symbol link."
        return
    fi

    echo "${logFormat}" >> "${LOG_SCRIPT_FILE}" 2>&1

    # 字符转换成整数
    BACKLOGSIZE=`char_to_num ${BACKLOGSIZE}`
    BACKLOGCOUNT=`char_to_num ${BACKLOGCOUNT}`
    LOGFILESIZE=`ls -l "${LOG_SCRIPT_FILE}" --block-size=m | awk -F " " '{print $5}' | awk -F "M" '{print $1}'` # MB unit
    LOGFILESIZE=`char_to_num ${LOGFILESIZE}`

    BACKLOGNAME="${LOG_SCRIPT_FILE}.${BACKLOGCOUNT}.${LOGFILE_SUFFIX}"
    NUMBER=`expr ${BACKLOGCOUNT} - 1` 
    if [ ${LOGFILESIZE} -gt $BACKLOGSIZE ]; then
        if [ -f "${BACKLOGNAME}" ]; then
            rm -f "${BACKLOGNAME}"
        fi

        while [ $NUMBER -ge 0 ]
        do
            if [ $NUMBER -eq 0 ]; then
                gzip -f -q -9 "${LOG_SCRIPT_FILE}"
                BACKLOGNAME="${LOG_SCRIPT_FILE}.${LOGFILE_SUFFIX}"
            else
                BACKLOGNAME="${LOG_SCRIPT_FILE}.${NUMBER}.${LOGFILE_SUFFIX}"                 
            fi

            if [  -f "${BACKLOGNAME}" ]; then
                DestNum=`expr $NUMBER + 1`
                mv -f "${BACKLOGNAME}" "${LOG_SCRIPT_FILE}.${DestNum}.${LOGFILE_SUFFIX}" 
                chmod 440 "${LOG_SCRIPT_FILE}.${DestNum}.${LOGFILE_SUFFIX}" 
            fi

            NUMBER=`expr $NUMBER - 1`
        done
    fi
}

function log_error()
{
    log "${0##*/}" "ERROR" "$*"
}

function log_info()
{
    log "${0##*/}" "INFO" "$*"
}

function log_debug()
{
     log "${0##*/}" "DEBUG" "$*"
}

function log_warn()
{
    log "${0##*/}" "WARN" "$*"
}

function verify_special_char()
{
    local special_chars="[\`$;|&<>\!]"
    for arg in $*
    do
        echo "$arg" | grep ${special_chars} 1>/dev/null 2>&1
        if [ $? -eq 0 ]; then
            log "The variable[$arg] cannot contain special characters."
            exit 1
        fi
    done
}

if [ -L "${LOG_SCRIPT_FILE}" ]; then
    echo "The file[${LOG_SCRIPT_FILE}] is a symbol link."
    exit 1
fi

chmod 600 ${LOG_SCRIPT_FILE} >/dev/null 2>&1