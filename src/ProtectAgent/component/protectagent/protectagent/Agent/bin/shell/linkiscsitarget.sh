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
#@function: get DB capacity, Four capacities need to be returned.
USAGE="Usage: ./linkiscsitarget.sh AgentRoot PID"

AGENT_ROOT_PATH=$1
PID=$2
PARAM_NUM=$3
IN_ORACLE_HOME=""
. "${AGENT_ROOT_PATH}/sbin/agent_sbin_func.sh"

#********************************define these for the functions of agent_sbin_func.sh********************************
#for Log
LOG_FILE_NAME="${LOG_PATH}/linkiscsitarget.log"
#********************************define these for the functions of agent_sbin_func.sh********************************

#********************************define these for local script********************************
PARAM_FILE=`ReadInputParam`
test -z "$PARAM_FILE"              && ExitWithError "PARAM_FILE"
#############################################################
targetIp=`GetValue "${PARAM_FILE}" targetIp`
targetPort=`GetValue "${PARAM_FILE}" targetPort`
chapName=`GetValue "${PARAM_FILE}" chapName`
Log "targetIp=${targetIp};targetPort=${targetPort};chapName=${chapName}"

test -z "${targetIp}" && ExitWithErrorCode "target IP is invalid" 1
test -z "${targetPort}" && ExitWithErrorCode "target port is invalid" 1

ping -c 2 ${targetIp}
if [ $? -ne 0 ]; then
    Log "${targetIp} can't access."
    exit 1
fi

iscsiadm -m session | grep "${targetIp}:${targetPort}" >> "${LOG_FILE_NAME}" 2>&1
if [ $? -eq 0 ]; then
    Log "${targetIp}:${targetPort} have already linked."
    exit 0
fi

iscsiadm -m discovery -t st -p ${targetIp}:${targetPort} >> "${LOG_FILE_NAME}" 2>&1
if [ $? -ne 0 ]; then
    Log "discovery ${targetIp}:${targetPort} failed."
    exit 1
fi

iscsiadm -m node -l -p ${targetIp}:${targetPort} >> "${LOG_FILE_NAME}" 2>&1
if [ $? -ne 0 ]; then
    Log "link ${targetIp}:${targetPort} failed."
    exit 1
fi

Log "link ${targetIp}:${targetPort} successfully."
exit 0