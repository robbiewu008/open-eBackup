#!/bin/bash

########################################
#  This file is part of the open-eBackup project.
# Copyright (c) 2024 Huawei Technologies Co.,Ltd.
#
# open-eBackup is licensed under MPL v2.
# You can use this software according to the terms and conditions of the MPL v2.
# You may obtain a copy of MPL v2 at:
#
#          https://www.mozilla.org/en-US/MPL/2.0
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the MPL v2 for more details.
########################################

tokenFile=$(cat /var/run/secrets/kubernetes.io/serviceaccount/token)
rootCAFile="/var/run/secrets/kubernetes.io/serviceaccount/ca.crt"

G_SUDO_SCRIPT_PATH=/opt/script

backup_flag=/tmp/backup_flag
function check_application_and_port() {
  if [ -f "${backup_flag}" ]; then
    # 升级备份gaussdb过程中，不检查gaussdb服务状态
    return 0
  fi

  # Lun异常状态时gs_ctl返回时间长，防止gs_ctl进程启动太多，添加数量限制
  local gs_ctl_num=$(pidof gs_ctl | wc -w)
  if [ $gs_ctl_num -gt 5 ]; then
    # gs_ctl 数量为3时，说明lun状态异常，返回异常
    return 1
  fi

  config_maps=$(LD_PRELOAD=/usr/lib64/libSecurityStarter.so curl --max-time 10 --cacert ${rootCAFile} -X GET -H "Authorization: Bearer $tokenFile" https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/dpa/configmaps/common-conf follow@-H)
  if [ $? -ne 0 ]; then
    echo "failed to get configmap common-conf"
    exit 1
  fi
  is_exist=$(echo "${config_maps} " | python3 -c "import sys, json;print(json.load(sys.stdin)['data'].get('rollbacking'))")
  if [ ! -z "${is_exist}" ] && [ "${is_exist}" != "None" ]; then
    # 回滚gaussdb数据过程中不检查gaussdb服务状态
    return 0
  fi

  if [[ ${DEPLOY_TYPE} == "d0" || ${DEPLOY_TYPE} == "d1" || ${DEPLOY_TYPE} == "d2" || ${DEPLOY_TYPE} == "d6" || ${DEPLOY_TYPE} == "d8" ]]; then
    # V1 Completed为大写C，V5小写
    build_result=$(sudo ${G_SUDO_SCRIPT_PATH}/ha_sudo.sh db_querybuild 2>&1 | grep -E "Build Completed|Build completed")
    if [[ "${build_result}" == "" ]]; then
      # 多集群HA场景判断如果数据库正在同步，不检查gaussdb服务状态
      return 0
    fi
  fi

  curl -kv --connect-timeout 3 --max-time 3 $POD_IP:$GAUSSDB_SERVICE_PORT 2>&1 | grep 'Connected'
  if [[ "$?" == 0 ]]; then
    echo "Detect connection to gaussdb successfully"
  else
    echo "Detect connection to gaussdb failed"
    exit 1
  fi

  unset GAUSSLOG # 屏蔽gs_ctl日志，避免日志写不下去导致gaussdb健康检查探针失败
  LD_LIBRARY_PATH=/usr/local/gaussdb/app/lib /usr/local/gaussdb/app/bin/gs_ctl -D /usr/local/gaussdb/data query -U GaussDB
  if [[ "$?" -ne 0 ]]; then
    echo "wrong when use gs_ctl"
    exit 1
  fi
  exit 0
}

function main() {
  check_application_and_port
}

main
