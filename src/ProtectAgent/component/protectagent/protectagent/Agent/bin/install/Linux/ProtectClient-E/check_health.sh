#!/bin/bash
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
#########################################
# Copyright (c) 2012-2015 Huawei .
# All rights reserved.
#
# Function 容器的健康检查
# revise note
########################################

DEPLOY_TYPE_CYBER_ENGINE="d5"
AGENT_ROOT="/opt/DataBackup/ProtectClient"
CURL_ERR_CONNECT=7
OPERATION_TIMEOUT=28
LOG_FILE_NAME=$AGENT_ROOT/ProtectClient-E/log/check_health.log

Log()
{
    if [ ! -f "${LOG_FILE_NAME}" ]; then
        touch "${LOG_FILE_NAME}"
        chmod 600 "${LOG_FILE_NAME}"
    fi

    DATE=`date +%y-%m-%d--%H:%M:%S`
    USER_NAME=`whoami`

    echo "[${DATE}][$$][${USER_NAME}] $1" >> "${LOG_FILE_NAME}"
}

IpPing()
{
    ipAddr=$1
    echo ${ipAddr} | grep "\\:" >/dev/null 2>&1
    if [ 0 -eq $? ]; then
        ping6 -c 1 -W 1 ${ipAddr} > /dev/null 2>&1
        return $?
    fi
    echo ${ipAddr} | grep "\\." >/dev/null 2>&1
    if [ 0 -eq $? ]; then
        ping -c 1 -W 1 ${ipAddr} >/dev/null 2>&1
        return $?
    fi
    return 1
}

function check_application_and_port()
{
    ps aux | grep internal_run.sh | grep -v grep >> /dev/null
    if [ $? -ne 0 ]; then
        Log "Error internal run script not run "
        exit 1
    fi
}

function check_agent_conn()
{
    ps -ef | grep rdagent | grep -v grep >/dev/null
    if [ $? -ne 0 ]; then
        exit 0
    fi
    PM_IP=`grep -o '<ebk_server_ip value=".*"/>' ${AGENT_ROOT}/ProtectClient-E/conf/agent_cfg.xml | grep -o 'value=".*"'`
    if [ '${PM_IP}' = 'value="0.0.0.0"' ]; then
        Log "[INFO] pm ip not config, skip check agent health."
        exit 0
    fi
    rdnginxAddr=`grep 'listen' ${AGENT_ROOT}/ProtectClient-E/nginx/conf/nginx.conf|awk -F ' ' '{print $2}'`
    rdagentAddr=`grep 'fastcgi_pass' ${AGENT_ROOT}/ProtectClient-E/nginx/conf/nginx.conf|awk -F ' ' '{print $2}'`
    rdagentAddr=${rdagentAddr/;/}
    if [ ! -z "${rdnginxAddr}" ]; then
        $curl_cmd -kv https://$rdnginxAddr -m 120 >/dev/null 2>&1
        ret=$?
        if [ $ret -eq ${CURL_ERR_CONNECT} ] || [ $ret -eq ${OPERATION_TIMEOUT} ]; then
            Log "Error nginx status is invaild, ret:$ret "
            exit 1
        fi
    fi
    if [ ! -z "${rdagentAddr}" ]; then
        $curl_cmd -kv http://$rdagentAddr -m 120 >/dev/null 2>&1
        ret=$?
        if [ $ret -eq ${CURL_ERR_CONNECT} ] || [ $ret -eq ${OPERATION_TIMEOUT} ]; then
            Log "Error agent status is invaild, ret:$ret "
            exit 1
        fi
    fi
}

function modify_cmd()
{
    if [ "$DEPLOY_TYPE" = "hyperdetect" ] || [ "$DEPLOY_TYPE" = "d4" ] || [ "$DEPLOY_TYPE" = "d5" ] || [ "$DEPLOY_TYPE" = "d7" ] || [ "$DEPLOY_TYPE" = "d8" ] || [ "$DEPLOY_TYPE" = "" ]
    then
        curl_cmd="curl"
    else
        curl_cmd="ip vrf exec vrf-srv curl --interface vrf-srv"
    fi
}

function main()
{
    modify_cmd
    check_agent_conn
    check_application_and_port
}

main
exit $?