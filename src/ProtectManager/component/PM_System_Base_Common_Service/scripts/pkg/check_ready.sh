#!/bin/bash

#########################################
# Copyright (c) 2021-2021 Huawei .
# All rights reserved.
#
# Please send feedback to http://www.huawei.com
#
# Function 容器的健康检查
# 使用https调用自己的服务
# revise note
########################################
CLUSTER_ADDING_FLAG_PATH="/opt/cluster_config/CLUSTER_ADDING_FLAG"

function check_application()
{
  # 添加成员集群时停止探针
  if [ -f "${CLUSTER_ADDING_FLAG_PATH}" ]; then
    local check_res=`cat ${CLUSTER_ADDING_FLAG_PATH} | grep true`
      if [ -n "${check_res}" ]; then
          exit 0
      fi
  fi
  isHas=`ifconfig cbri1601 2>/dev/null`
  if [ "${DEPLOY_TYPE}" = "d5" ] || [ "${DEPLOY_TYPE}" = "d7" ] || [ "${DEPLOY_TYPE}" = "d8" ]; then
    # 安全一体机d5场景使用eth1网桥
    local IPA=${POD_IP}
    status_code=$("timeout" 30 "/usr/bin/restclient" "/opt/OceanProtect/protectmanager/kmc/master.ks" "/kmc_conf/..data/backup.ks" "/opt/OceanProtect/infrastructure/cert/internal/internal_cert" -u "https://${IPA}:30081/v1/internal/health" "")
  elif [[ ! -z "${isHas}" ]]; then
    local IPA=`ifconfig  cbri1601 | grep "inet " | awk '{print $2}'`
    status_code=$("timeout" 30 "/usr/bin/restclient" "/opt/OceanProtect/protectmanager/kmc/master.ks" "/kmc_conf/..data/backup.ks" "/opt/OceanProtect/infrastructure/cert/internal/internal_cert" -u "https://${IPA}:30081/v1/internal/health" "")
  else
    local IPA=`ifconfig  eth0 | head -n2 | grep inet | awk '{print $2}'`
    status_code=$("timeout" 30 "/usr/bin/restclient" "/opt/OceanProtect/protectmanager/kmc/master.ks" "/kmc_conf/..data/backup.ks" "/opt/OceanProtect/infrastructure/cert/internal/internal_cert" -u "https://${IPA}:30081/v1/internal/health" "")
  fi
  if [ "${status_code}" == "200" ]; then
    exit 0
  else
    exit 1
  fi
}

function main()
{
    check_application
}

main