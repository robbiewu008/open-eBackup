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
# *注意*：若要兼容bash和ksh，必需使用. 的方式引入common，并事先定义COMMON_PATH!
if [ -z "${COMMON_PATH}" ]; then
    CUR_COMMON_PATH=`dirname ${BASH_SOURCE[0]}`
    COMMON_PATH="`cd ${CUR_COMMON_PATH} && pwd`"  # 该命令在bash中获取的才是common路径，ksh中为调用者路径
fi
SCRIPT_NAME=`basename $0`

PLUGIN_JSON=${COMMON_PATH}/plugin_*.json

# 系统类型
OS_TYPE=`uname -s`

if [ "${OS_TYPE}" = "SunOS" ]; then
    AWK=nawk
    PATH=/usr/xpg4/bin:$PATH
    export PATH
else
    AWK=awk
fi

log_echo()
{
    logType="$1"
    message="$2"
    echo "[`date \"+%Y-%m-%d %H:%M:%S\"`]["${logType}"][ "${message}" ]["${SCRIPT_NAME}"][${USER}]"
}

get_plugin_name()
{
    if [ ! -f ${PLUGIN_JSON} ]; then
        return 1
    fi

    PLUGIN_NAME=`cat ${PLUGIN_JSON} | grep name | ${AWK} -F '\"' '{print $4}'`
    if [ "${PLUGIN_NAME}X" = "X" ]; then
        return 1
    fi

    echo ${PLUGIN_NAME}
}

get_plugin_account()
{
    if [ ! -f ${PLUGIN_JSON} ]; then
        return 1
    fi

    PLUGIN_ACCOUNT=`cat ${PLUGIN_JSON} | grep run_account | ${AWK} -F '\"' '{print $4}'`
    if [ "${PLUGIN_ACCOUNT}X" = "X" ]; then
        return 1
    fi

    echo ${PLUGIN_ACCOUNT}
    return 0
}

get_plugin_log_path()
{
    # DATA_BACKUP_AGENT_HOME为agent安装目录，由agent设置的环境变量
    protectClientPath=${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient/ProtectClient-E
    PLUGIN_ACCOUNT=`get_plugin_account`
    if [ $? -ne 0 ]; then
        echo "get "
        return 1
    fi

    PLUGIN_NAME=`get_plugin_name`
    if [ $? -ne 0 ]; then
        echo ""
        return 1
    fi

    if [ "${PLUGIN_ACCOUNT}X" = "rootX" ]; then
        LOG_FILE_PREFIX=${protectClientPath}/slog/Plugins/${PLUGIN_NAME}
    else
        LOG_FILE_PREFIX=${protectClientPath}/log/Plugins/${PLUGIN_NAME}
    fi

    echo ${LOG_FILE_PREFIX}
    return 0
}

# AIX中sed不支持-i选项，使用下面方法替换
sed_local_modify()
{
    script=$1
    fileName=$2
    if [ "${OS_TYPE}" = "AIX" ] || [ "${OS_TYPE}" = "SunOS" ]; then
        sed "${script}" ${fileName} > ${fileName}.bak
        mv ${fileName}.bak ${fileName}
    else
        sed -i "${script}" ${fileName}
    fi
}