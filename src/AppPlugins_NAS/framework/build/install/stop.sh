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
# Stop nas plugin process for agent
SCRIPT_PATH=$(cd $(dirname $0); pwd)
COMMON_PATH=${SCRIPT_PATH}
. ${COMMON_PATH}/common.sh
SCRIPT_NAME=$(basename $0)
SYS_NAME=`uname -s`

if [ "${OS_TYPE}" = "SunOS" ]; then
    AWK=nawk
else
    AWK=awk
fi

LOG_FILE_ARRAY="start.log stop.log Plugin_lle.log dme_nas.log"
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

logfile_switch()
{
    SWITCH_FILE_POSTFIX=".gz"
    ONE_DAY=1 # 1day
    ONE_DAY=`expr ${ONE_DAY} \* 24` # day to hour
    ONE_DAY=`expr ${ONE_DAY} \* 60` # hour to min
    ONE_DAY=`expr ${ONE_DAY} \* 60` # min to sec
    DUMP_INTERVAL=185
    DUMP_INTERVAL_M=6
    LOG_COUNT=20
    LOG_SWITCH_SIZE=10 # 10 MB
    # switch to Byte
    LOG_SWITCH_SIZE=`expr ${LOG_SWITCH_SIZE} \* 1024`
    LOG_SWITCH_SIZE=`expr ${LOG_SWITCH_SIZE} \* 1024`
    for file in ${LOG_FILE_ARRAY}; do
        filepath="${LOG_ROOT_PATH}/${file}"
        if [ ! -f ${filepath} ]; then
            continue
        fi
        if [ ${SYS_NAME} = "AIX" ]; then
            deleteLogNum=0
            currentNum=`ls ${LOG_ROOT_PATH} | grep ${file} | grep gz | wc -l | ${AWK} '{print $1}'`
            if [ ${currentNum} -gt ${LOG_COUNT} ]; then
                deleteLogNum=`expr ${currentNum} - ${LOG_COUNT}`
            fi
            gzFileList=`ls -tr ${LOG_ROOT_PATH} | grep ${file} | grep gz`
            for gzfile in $gzFileList; do
                if [ ${deleteLogNum} -eq 0 ]; then
                    break
                fi
                rm -rf ${LOG_ROOT_PATH}/${gzfile}
                deleteLogNum=`expr ${deleteLogNum} - 1`
            done
        else
            #Delete expired log file
            gzFileList=`ls ${LOG_ROOT_PATH} | grep ${file} | grep gz`
            for gzfile in ${gzFileList}; do
                cat /etc/release | grep "Solaris 10" >/dev/null 2>&1
                isSolaris10=$?
                log_echo "INFO" "isSolaris10: $isSolaris10." >> ${LOG_FILE}
                if [ "$isSolaris10" = "0" ]; then
                    fileDate=`ls -E ${LOG_ROOT_PATH}/${gzfile} | ${AWK} '{print $6}'`
                    curDate=`date "+%Y-%m"`
                    dateInterval=`expr $((10#${fileDate:0:4})) \* 12 + $((10#${fileDate:5:2})) - $((10#${curDate:0:4})) \* 12 - $((10#${curDate:5:2}))`
                    if [ ${dateInterval} -gt ${DUMP_INTERVAL_M} ]; then
                        echo "deleting expired (${DUMP_INTERVAL} days) file: ${gzfile}"
                        rm -rf ${LOG_ROOT_PATH}/${gzfile}
                    fi
                else
                    fileDate=`stat -c %Y ${LOG_ROOT_PATH}/${gzfile}`
                    curDate=`date +%s`
                    dateInterval=`expr ${curDate} - ${fileDate}`
                    dateInterval=`expr ${dateInterval} / ${ONE_DAY}`
                    if [ ${dateInterval} -gt ${DUMP_INTERVAL} ]; then
                        echo "deleting expired (${DUMP_INTERVAL} days) file: ${gzfile}"
                        rm -rf ${LOG_ROOT_PATH}/${gzfile}
                    fi
                fi
            done
        fi
        logSize=0
        nameIindex=0
        if [ ${SYS_NAME} = "AIX" ]; then
            logSize=`istat ${filepath} | grep "Length" | ${AWK} '{print $5}'`
            nameIindex=`ls ${LOG_ROOT_PATH} | grep ${file} | grep gz | wc -l | ${AWK} '{print $1}'`
        else
            logSize=`ls -l "${filepath}" | ${AWK} '{print $5}'`
            nameIindex=`ls ${LOG_ROOT_PATH} | grep ${file} | grep gz | wc -l`
        fi
        if [ $logSize -gt ${LOG_SWITCH_SIZE} ]; then
            gzFileList=`ls -tr ${LOG_ROOT_PATH} | grep ${file} | grep gz`
            for gzfile in $gzFileList; do
                mv ${LOG_ROOT_PATH}/${gzfile} ${LOG_ROOT_PATH}/${file}.${nameIindex}${SWITCH_FILE_POSTFIX}
                nameIindex=`expr $nameIindex - 1`
            done
            log_echo "INFO" "$file size is too big ,$file has been switched." >> ${LOG_FILE}
            gzip -f "$filepath"
            mv ${LOG_ROOT_PATH}/${file}${SWITCH_FILE_POSTFIX} ${LOG_ROOT_PATH}/${file}.0${SWITCH_FILE_POSTFIX}
        fi
    done
}

stop_plugin()
{
    cd ${SCRIPT_PATH}/bin
    executable_file=AgentPlugin
    if [ ! -f ${executable_file} ];then
        if [ ! -f "${PLUGIN_NAME}" ]; then
            log_echo "ERROR" "Not exist executable file." >> ${LOG_FILE}
            exit 1
        else
           executable_file=${PLUGIN_NAME}
        fi
    fi
    if [ "${OS_TYPE}" = "AIX" ] || [ "${OS_TYPE}" = "SunOS" ]; then  # AIX/SunOS ps不支持-w选项
        curPid=$(ps -ef | grep -E "./bin/${executable_file}" | grep -v grep | ${AWK} '{print $2}')
    else
        curPid=$(ps -efww | grep -E "./bin/${executable_file}\>" | grep -v grep | ${AWK} '{print $2}')
    fi
    if [ -z "${curPid}" ];then
        log_echo "WARNING" "${PLUGIN_NAME} process not exist, no need to stop it. return success." >> ${LOG_FILE}
        exit 0
    fi
    log_echo "INFO" "Plugin process id is ${curPid}" >> ${LOG_FILE}
    typeset backup_scene=`cat ${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient/ProtectClient-E/conf/testcfg.tmp | grep "BACKUP_SCENE" | cut -d'=' -f2`
    if [ "${backup_scene}" == "1" ] && [ "${PLUGIN_NAME}" == "VirtualizationPlugin" ]; then
        # internal agent use exrdadmin user to run and stop virtualizationplugin
        su - exrdadmin -s /bin/bash -c "kill -9 "${curPid}""
    else
        kill -9 ${curPid}
    fi
    if [ $? -eq 0 ]; then
        log_echo "INFO" "Stop nas process sucessfully" >> ${LOG_FILE}
        return 0
    fi
    log_echo "ERROR" "Failed to stop the nas process." >> ${LOG_FILE}
    return 1
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

    LOG_FILE=${LOG_FILE_PREFIX}/stop.log
    LOG_ROOT_PATH=${LOG_FILE_PREFIX}

    stop_plugin "$@"
    if [ $? -ne 0 ]; then
        echo "[$(date "+%Y-%m-%d %H:%M:%S")][ERROR][ Failed to stop agent plugin ${PLUGIN_NAME}. ]" >> ${LOG_FILE}
        return 1
    fi
    echo "[$(date "+%Y-%m-%d %H:%M:%S")][INFO][ Stop agent plugin ${PLUGIN_NAME} successfully. ]" >> ${LOG_FILE}
    logfile_switch
    return 0
}

main "$@"
