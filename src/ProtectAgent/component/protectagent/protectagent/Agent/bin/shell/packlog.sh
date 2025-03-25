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

###### Custom installation directory ######
AGENT_ROOT_PATH=${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient/ProtectClient-E/

##################################################################
CURRENT_PATH=$(cd $(dirname $0); pwd)
. ${CURRENT_PATH}/agent_sbin_func.sh

##################################################################

SANCLIENT_AGENT_ROOT_PATH="${DATA_BACKUP_SANCLIENT_HOME}/DataBackup/SanClient/ProtectClient-E/"
SANCLIENT_USER=sanclient
ID=$2

SHELL_TYPE_SH="/bin/sh"
BACKUP_ROLE_SANCLIENT_PLUGIN=5
TESTCFG_BACK_ROLE=""
TESTCFG_BACK_ROLE=`su - ${SANCLIENT_USER} 2>/dev/null -s ${SHELL_TYPE_SH} -c "sed '/^BACKUP_ROLE=/!d;s/.*=//' ${CURRENT_PATH}/../conf/testcfg.tmp "`
if [ "${BACKUP_ROLE_SANCLIENT_PLUGIN}" = "${TESTCFG_BACK_ROLE}" ]; then
    AGENT_ROOT_PATH=${SANCLIENT_AGENT_ROOT_PATH}
fi

SYSTEM_LOG_PATH="/var/log"
SYSTEM_LOG_FILE_NAME="messages"
AGENT_LOG_PATH="${AGENT_ROOT_PATH}/slog"
AGENT_LOG_PATH2="${AGENT_ROOT_PATH}/log"
AGENT_BIN_PATH="${AGENT_ROOT_PATH}/bin"
NGINX_LOG_PATH="${AGENT_ROOT_PATH}/nginx/logs"
AGENT_TMP_PATH="${AGENT_ROOT_PATH}/stmp"
PLUGIN_LOG_PATH="${AGENT_ROOT_PATH}/log/Plugins"
PLUGIN_SLOG_PATH="${AGENT_ROOT_PATH}/slog/Plugins"
FILECLIENT_SLOG_PATH="${AGENT_ROOT_PATH}/slog/FileClientLog"
LOG_FILE_NAME="${AGENT_LOG_PATH}/packlog.log"
SYS_LOG_MAX_100MB=102400
ARCHIVED_SYS_LOG_SIZE=10240
AGENT_TMP_PATH_USAGE=80

# read params(pack file name)
PARAM_NUM=$3
PARAM_CONTENT=`ReadInputParam`
test -z "$PARAM_CONTENT" && ExitWithError "Parameter"

PARAM_CONTENT=`echo ${PARAM_CONTENT} | xargs`
LOG_PACK_OPER=`GetValue "${PARAM_CONTENT}" operation`
LOG_PACK_NAME=`GetValue "${PARAM_CONTENT}" logPackName`
test -z "$LOG_PACK_OPER" && ExitWithError "Log collect operation invalid"
test -z "$LOG_PACK_NAME" && ExitWithError "Log pack name"
PACKAGE_LOG=${AGENT_TMP_PATH}/${LOG_PACK_NAME}

collectDataTurboLog()
{
    if [ ! -d "/var/log/dataturbo" ]; then
        return
    fi
    CP -r -p"/var/log/dataturbo/"/*  "${AGENT_TMP_PATH}/${LOG_FOLDER}/dataturbo_log"
}

collectSysLog()
{
    if [ ! -d "${SYSTEM_LOG_PATH}" ]; then
        Log "System log path is not exist."
        return 1
    fi

    dfAgentTmpPath=`df -k "${AGENT_TMP_PATH}" | tail -1`
    if [ "${SYS_NAME}" = "Linux" ]; then
        diskSpaceUsage=`echo ${dfAgentTmpPath} | ${MYAWK} '{print $5}' | ${MYAWK} -F "%" '{print $1}'`
        diskFreeSpace=`echo ${dfAgentTmpPath} | ${MYAWK} '{print $4}'`
        diskSumSpace=`echo ${dfAgentTmpPath} | ${MYAWK} '{print $2}'`
    elif [ "${SYS_NAME}" = "AIX" ]; then
        diskSpaceUsage=`echo ${dfAgentTmpPath} | ${MYAWK} '{print $4}' | ${MYAWK} -F "%" '{print $1}'`
        diskFreeSpace=`echo ${dfAgentTmpPath} | ${MYAWK} '{print $3}'`
        diskSumSpace=`echo ${dfAgentTmpPath} | ${MYAWK} '{print $2}'`
    fi
    diskAvailableSpace=`expr ${diskFreeSpace} - ${diskSumSpace} / 5`
    
    for i in `ls -lt "${SYSTEM_LOG_PATH}" | grep "${SYSTEM_LOG_FILE_NAME}" | ${MYAWK} '{print $NF}'`
    do
        Log "Check system error log size, filename[${i}]."
        sysCurErrLogSize=`du -k "${SYSTEM_LOG_PATH}/${i}" | ${MYAWK} '{print $1}'`
        if [ ${sysCurErrLogSize} -ge ${SYS_LOG_MAX_100MB} ]; then
            Log "System error log size exceeds limit[${sysCurErrLogSize}KB]."
            return 1
        fi
        
        Log "Check Agent space usage, filename[${i}]."
        if [ ${diskSpaceUsage} -ge ${AGENT_TMP_PATH_USAGE} ]; then
            Log "Agent space usage exceeds limit, SpaceUsage[${diskSpaceUsage}%]."
            return 1
        fi

        Log "Check system error log file size after compression, filename[${i}]."
        tar cvfp "${AGENT_TMP_PATH}/${LOG_FOLDER}/sys_log/${i}.tar"  "${SYSTEM_LOG_PATH}/${i}" > /dev/null
        gzip "${AGENT_TMP_PATH}/${LOG_FOLDER}/sys_log/${i}.tar" > /dev/null
        sysLogZipSize=`du -k "${AGENT_TMP_PATH}/${LOG_FOLDER}/sys_log/" | tail -1 | ${MYAWK} '{print $1}'`
        if [ ${diskAvailableSpace} -lt ${sysLogZipSize} ]; then
            Log "Agent space usage exceeds limit, Availablespace[${diskAvailableSpace}], SysLogZipSize[${sysLogZipSize}]."
            rm -f "${AGENT_TMP_PATH}/${LOG_FOLDER}/sys_log/${i}.tar"
            return 1
        fi

        if [ ${sysLogZipSize} -ge ${ARCHIVED_SYS_LOG_SIZE} ]; then
            Log "Archived syslog limit reached, sysLogZipSize[${sysLogZipSize}KB]."
            return 1
        fi

    done
    Log "System log collection completed."
    return 0
}

collectLogMain()
{
    Log "Begin to package log."
    Log "LOG_PACK_NAME is ${LOG_PACK_NAME}."
    AGENT_IP=`cat ${AGENT_ROOT_PATH}/nginx/conf/nginx.conf |grep listen |${MYAWK} '{print $2}' |${MYAWK} -F ":" '{print $1}'`
    if [ -z "${AGENT_IP}" ]; then
        Log "Failed to obtain the IP address of the agent."
    fi
    LOG_FOLDER=sysinfo_`date +%y-%m-%d-%H-%M-%S `_${AGENT_IP}

    rm -rf ${AGENT_TMP_PATH}/sysinfo_*.tar.gz > /dev/null
    rm -rf ${AGENT_TMP_PATH}/AGENTLOG_*.tar.gz > /dev/null
    mkdir "${AGENT_TMP_PATH}/${LOG_FOLDER}"
    mkdir "${AGENT_TMP_PATH}/${LOG_FOLDER}/nginx_log"
    mkdir "${AGENT_TMP_PATH}/${LOG_FOLDER}/agent_log"
    mkdir "${AGENT_TMP_PATH}/${LOG_FOLDER}/fileclient_log"
    mkdir "${AGENT_TMP_PATH}/${LOG_FOLDER}/plugin_log"
    mkdir "${AGENT_TMP_PATH}/${LOG_FOLDER}/plugin_log/log"
    mkdir "${AGENT_TMP_PATH}/${LOG_FOLDER}/plugin_log/slog"
    mkdir "${AGENT_TMP_PATH}/${LOG_FOLDER}/sys_log"
    mkdir "${AGENT_TMP_PATH}/${LOG_FOLDER}/dataturbo_log"

    if [ -d "${NGINX_LOG_PATH}" ]
    then
        CP -r -p "${NGINX_LOG_PATH}"/*.log*  "${AGENT_TMP_PATH}/${LOG_FOLDER}/nginx_log"
        CP -r -p "${NGINX_LOG_PATH}"/*.pid   "${AGENT_TMP_PATH}/${LOG_FOLDER}/nginx_log"
    fi

    if [ -d "${AGENT_LOG_PATH}" ]
    then
        CP -r -p "${AGENT_LOG_PATH}"/*.log*  "${AGENT_TMP_PATH}/${LOG_FOLDER}/agent_log"
        CP -r -p "${AGENT_LOG_PATH}"/*.pid  "${AGENT_TMP_PATH}/${LOG_FOLDER}/agent_log"
    fi

    if [ -d "${AGENT_LOG_PATH2}" ]
    then 
        CP -r -p "${AGENT_LOG_PATH2}"/*.log*  "${AGENT_TMP_PATH}/${LOG_FOLDER}/agent_log"
        CP -r -p "${AGENT_LOG_PATH2}"/*.pid  "${AGENT_TMP_PATH}/${LOG_FOLDER}/agent_log"
    fi

    if [ -d "${PLUGIN_LOG_PATH}" ]; then
        CP -rf "${AGENT_ROOT_PATH}/log/Plugins"/*Plugin "${AGENT_TMP_PATH}/${LOG_FOLDER}/plugin_log/log"
    fi

    if [ -d "${PLUGIN_SLOG_PATH}" ]; then
        CP -rf "${AGENT_ROOT_PATH}/slog/Plugins"/*Plugin "${AGENT_TMP_PATH}/${LOG_FOLDER}/plugin_log/slog"
    fi

    if [ -d "${FILECLIENT_SLOG_PATH}" ]; then
        CP -r -p "${FILECLIENT_SLOG_PATH}"/*.log*  "${AGENT_TMP_PATH}/${LOG_FOLDER}/fileclient_log"
    fi

    collectSysLog
    collectDataTurboLog

    Log "Compress agent log."
    cd "${AGENT_TMP_PATH}/${LOG_FOLDER}"
    tar cvfp "${PACKAGE_LOG}.tar" * > /dev/null
    cd "${AGENT_TMP_PATH}"
    gzip "${PACKAGE_LOG}.tar" > /dev/null

    rm -rf "${AGENT_TMP_PATH}/${LOG_FOLDER}" > /dev/null
    rm -rf "${PACKAGE_LOG}.tar" > /dev/null

    if [ ! -f "${PACKAGE_LOG}.tar.gz" ]
    then 
        Log "${PACKAGE_LOG}.tar.gz is not exists."
        exit 1
    fi
    chmod 644 ${PACKAGE_LOG}.tar.gz
    Log "Finish packaging log."
}

cleanLogPkg()
{
    Log "Log package to clean: ${PACKAGE_LOG}.tar.gz"
    rm -rf "${PACKAGE_LOG}.tar.gz"
}

if [ "${LOG_PACK_OPER}" = "collect" ]; then
    collectLogMain
elif [ "${LOG_PACK_OPER}" = "clean" ]; then
    cleanLogPkg
else
    Log "Invalid operation."
    exit 1
fi

exit 0
