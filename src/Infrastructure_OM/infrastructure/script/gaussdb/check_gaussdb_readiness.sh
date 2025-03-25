#!/bin/bash

#########################################
# Copyright (c) 2012-2015 Huawei .
# All rights reserved.
#
# Function 容器的健康检查
# revise note
########################################

backup_flag=/usr/local/gaussdb/backup_flag
G_SUDO_SCRIPT_PATH=/opt/script

function check_db_state() {
  local dbinfo=$(
    export LD_LIBRARY_PATH=/usr/local/gaussdb/app/lib
    /usr/local/gaussdb/app/bin/gs_ctl -D /usr/local/gaussdb/data query -U GaussDB
  )

  local local_role=$(echo "$dbinfo" | grep -Ew "local_role" | awk '{print $3}')
  if [ "${local_role}" == "Standby" ]; then
    # 备节点gaussdb不需要检查gaussdb状态
    return 0
  fi

  local db_state=""
  eval $(echo "$dbinfo" | grep -Ew "db_state" | sed -e 's/:/=/g' | sed -e 's/ //g' | sed -n '1p')
  # 检查gaussdb状态是否为Normal
  if [ "${db_state}" == "Normal" ]; then
    return 0
  fi
  return 1
}

function main() {
  if [ -f "${backup_flag}" ]; then
    # 升级过程中，备份和恢复gaussdb数据过程中不监听gaussdb端口
    return 0
  fi

  # Lun异常状态时gs_ctl返回时间长，防止gs_ctl进程启动太多，添加数量限制
  local gs_ctl_num=$(pidof gs_ctl | wc -w)
  if [ $gs_ctl_num -gt 5 ]; then
    # gs_ctl 数量为3时，说明lun状态异常，返回异常
    return 1
  fi
  check_db_state
}

main
