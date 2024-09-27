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
# Copyright (c) 2023-2023 Huawei .
# All rights reserved.
#
# Please send feedback to http://www.huawei.com
#
# Function 获取集群角色
# revise note
########################################

# 主节点
PRIMARY_ROLE="PRIMARY"
# 从节点
STANDBY_ROLE="STANDBY"
# 成员节点
MEMBER_ROLE="MEMBER"

# 默认为主节点
CLUSTER_ROLE=PRIMARY_ROLE
# 集群配置文件路径
CLUSTER_ROLE_CONF_PATH="/opt/cluster_config/CLUSTER_ROLE"

function getClusterRole() {
  if [ ! -f "${CLUSTER_ROLE_CONF_PATH}" ];then
    return 0
  fi

  cluster_role_tmp=`cat ${CLUSTER_ROLE_CONF_PATH}`
  if [[ ${cluster_role_tmp} ]]; then
    if [[ ${cluster_role_tmp} != ${CLUSTER_ROLE} ]]; then
      echo "CLUSTER_ROLE is ${CLUSTER_ROLE}. cluster_role_tmp is ${cluster_role_tmp}. "
    fi
    CLUSTER_ROLE=${cluster_role_tmp}
  fi
}

function main()
{
    getClusterRole
}

main