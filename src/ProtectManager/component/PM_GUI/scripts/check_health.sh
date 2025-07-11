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
# Copyright (c) 2022-2022 Huawei .
# All rights reserved.
#
# Please send feedback to http://www.huawei.com
#
# Function 容器的健康检查
# revise note
########################################

. /app/gui/init_cluster_role.sh
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
  curl -kv --connect-timeout 3 --max-time 3 ${IPA}:30080 2>&1 | grep 'Bad Request'
  if [ $? == 0 ]; then
    exit 0
  else
    exit 1
  fi
}

function main()
{
    # 获取集群角色
    getClusterRole

    if [[ ${CLUSTER_ROLE} == ${MEMBER_ROLE} ]]; then
      # 1. 成员节点正常运行
      return 0
    else
      # 2. 主、从节点监听端口正常运行
      check_health
    fi
}

main