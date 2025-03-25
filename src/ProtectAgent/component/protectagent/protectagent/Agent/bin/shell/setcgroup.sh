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
#@function: set cgroup info to limit plugin

AGENT_ROOT_PATH=$1
PID=$2
PARAM_NUM=$3
. "${AGENT_ROOT_PATH}/sbin/agent_sbin_func.sh"
#for log
LOG_FILE_NAME="${LOG_PATH}/setcgroup.log"
DIR_CGROUP_CPU="/sys/fs/cgroup/cpu"
DIR_CGROUP_MEMORY="/sys/fs/cgroup/memory"
DIR_CGROUP_BLKIO="/sys/fs/cgroup/blkio"

PARAM_CONTENT=`ReadInputParam`
test -z "$PARAM_CONTENT"              && ExitWithError "PARAM_FILE"
#for GetValue
PLUGIN_NAME=`GetValue "${PARAM_CONTENT}" PluginName`
PLUGIN_PID=`GetValue "${PARAM_CONTENT}" PluginPID`
CPU_LIMIT=`GetValue "${PARAM_CONTENT}" CpuLimit`
MEMORY_LIMIT=`GetValue "${PARAM_CONTENT}" MemoryLimit`
BLKIO_WEIGHT=`GetValue "${PARAM_CONTENT}" BlkioWeight`

test "$PLUGIN_NAME" = "${ERROR_PARAM_INVALID}"                && ExitWithError "PLUGIN_NAME"
test "$PLUGIN_PID" = "${ERROR_PARAM_INVALID}"            && ExitWithError "PLUGIN_PID"
test "$CPU_LIMIT" = "${ERROR_PARAM_INVALID}"      && ExitWithError "CPU_LIMIT"
test "$MEMORY_LIMIT" = "${ERROR_PARAM_INVALID}"              && ExitWithError "MEMORY_LIMIT"
test "$BLKIO_WEIGHT" = "${ERROR_PARAM_INVALID}"              && ExitWithError "BLKIO_WEIGHT"
test -z "$PLUGIN_NAME"           && ExitWithError "PLUGIN_NAME"
test -z "$PLUGIN_PID"     && ExitWithError "PLUGIN_PID"
test -z "$CPU_LIMIT"             && ExitWithError "CPU_LIMIT"
test -z "$MEMORY_LIMIT"               && ExitWithError "MEMORY_LIMIT"
test -z "$BLKIO_WEIGHT"               && ExitWithError "BLKIO_WEIGHT"

Log "PID=${PID};PLUGIN_NAME=${PLUGIN_NAME};PLUGIN_PID=${PLUGIN_PID};CPU_LIMIT=${CPU_LIMIT};MEMORY_LIMIT=${MEMORY_LIMIT};BLKIO_WEIGHT=${BLKIO_WEIGHT}."

mount | grep cgroup 1>/dev/null 2>&1
if [ $? -ne 0 ]; then
    Log "The system not support cgroup."
    exit 1
fi

if [ "${CPU_LIMIT}" != "-1" ]; then
    mkdir -p "${DIR_CGROUP_CPU}/${PLUGIN_NAME}"
    echo 100000 > "${DIR_CGROUP_CPU}/${PLUGIN_NAME}/cpu.cfs_period_us"
    ((CFS_QUOTA_US=100000 * ${CPU_LIMIT} / 100))
    echo ${CFS_QUOTA_US} > "${DIR_CGROUP_CPU}/${PLUGIN_NAME}/cpu.cfs_quota_us"
    echo ${PLUGIN_PID} > "${DIR_CGROUP_CPU}/${PLUGIN_NAME}/cgroup.procs"
fi

if [ "${MEMORY_LIMIT}" != "-1" ]; then
    mkdir -p "${DIR_CGROUP_MEMORY}/${PLUGIN_NAME}"
    echo 1 > "${DIR_CGROUP_MEMORY}/${PLUGIN_NAME}/memory.oom_control"
    ((LIMIT_IN_BYTES=1024 * 1024 * ${MEMORY_LIMIT}))
    echo ${LIMIT_IN_BYTES} > "${DIR_CGROUP_MEMORY}/${PLUGIN_NAME}/memory.limit_in_bytes"
    echo ${PLUGIN_PID} > "${DIR_CGROUP_MEMORY}/${PLUGIN_NAME}/cgroup.procs"
fi

if [ "${BLKIO_WEIGHT}" != "-1" ]; then
    if [ ${BLKIO_WEIGHT} -lt 10 ] || [ ${BLKIO_WEIGHT} -gt 1000 ]; then
        Log "Plugin ${PLUGIN_NAME} cgroup blkio weight value out of range."
        exit 1
    fi
    mkdir -p "${DIR_CGROUP_BLKIO}/${PLUGIN_NAME}"
    echo ${BLKIO_WEIGHT} > "${DIR_CGROUP_BLKIO}/${PLUGIN_NAME}/blkio.weight"
    if [ $? -ne 0 ]; then
        Log "Set plugin ${PLUGIN_NAME} cgroup blkio weight failed."
        exit 1
    fi
    echo ${PLUGIN_PID} > "${DIR_CGROUP_BLKIO}/${PLUGIN_NAME}/cgroup.procs"
fi

Log "Set plugin ${PLUGIN_NAME} cgroup info success."
exit 0
