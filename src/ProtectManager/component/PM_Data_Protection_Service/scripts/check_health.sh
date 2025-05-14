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
# Copyright (c) 2021-2021 Huawei .
# All rights reserved.
#
# Please send feedback to http://www.huawei.com
#
# Function 容器的健康检查
# revise note
########################################

function check_health()
{
  if [ "${DEPLOY_TYPE}" = "d5" ] || [ "${DEPLOY_TYPE}" = "d4" ]; then
    # 安全一体机d5场景使用eth1网桥
    IPA=${POD_IP}
  elif [[ ! -z "`ifconfig |grep "cbri1601"`" ]]; then
    IPA=`ifconfig cbri1601 | grep "inet " | awk '{print $2}'` &> /dev/null
  else
    IPA="127.0.0.1"
  fi
  status_code=$("timeout" 30 "/usr/bin/restclient" "/opt/OceanProtect/protectmanager/kmc/master.ks" "/kmc_conf/..data/backup.ks" "/opt/OceanProtect/infrastructure/cert/internal/internal_cert" -u "https://${IPA}:30092/v1/internal/health" "")
  if [ "${status_code}" == "200" ]; then
    exit 0
  else
    exit 1
  fi
}

function main()
{
    check_health
}

main