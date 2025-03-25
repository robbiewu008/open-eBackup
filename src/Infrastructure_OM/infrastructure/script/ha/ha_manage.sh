#!/bin/bash

# 常量定义
if [ "$__HA_MANAGE_SH__" == "" ]; then

  # 返回描述
  declare -r CHECK_NETWORK_SUCCESS="check network success"
  declare -r ADD_HA_SUCCESS="add ha success"
  declare -r MODIFY_HA_SUCCESS="modify ha success"
  declare -r REMOVE_HA_SUCCESS="remove ha success"
  declare -r ROLLBACK_HA_SUCCESS="rollback ha success"
  declare -r FORBID_SWITCHOVER_SUCCESS="forbid switchover success"
  declare -r ALLOW_SWITCHOVER_SUCCESS="allow switchover success"
  declare -r UPDATE_HA_CERT_SUCCESS="update ha cert success"
  declare -r REMOVE_HA_CERT_SUCCESS="remove ha cert success"
  declare -r ROLLBACK_HA_CERT_SUCCESS="rollback ha cert success"
  declare -r CONNECTION_IP_ERROR="connect ip can not be connected"
  declare -r FLOAT_IP_ERROR="float ip can not be used"
  declare -r GATEWAY_IPS_ERROR="gateway ip can not be used"
  declare -r DATABASE_CONFIG_ERROR="config database failed"
  declare -r HA_CONFIG_ERROR="config ha failed"
  declare -r HA_START_ERROR="start ha failed"
  declare -r HA_STOP_ERROR="stop ha failed"
  declare -r DATABASE_RESTART_NORMAL_ERROR="restart database to normal failed"
  declare -r FORBID_SWITCHOVER_ERROR="forbid switchover failed"
  declare -r ALLOW_SWITCHOVER_ERROR="allow switchover failed"
  declare -r MODIFY_HA_ERROR="modify ha failed"

  # ha资源状态
  declare -r HA_RES_NORMAL="Normal"
  declare -r HA_RES_STOPPED="Stopped"
  declare -r HA_RES_ACTIVE_NORMAL="Active_normal"
  declare -r HA_RES_STANDBY_NORMAL="Standby_normal"
  declare -r DB_RES_NORMAL="Normal"
  declare -r DB_ROLE_PRIMARY="Primary"
  declare -r DB_ROLE_STANDBY="Standby"

  # 脚本相关
  SCRIPT_PATH=/usr/local/ha/script
  source ${SCRIPT_PATH}/log.sh
  source ${SCRIPT_PATH}/dbfunc.sh
  source ${SCRIPT_PATH}/event_lib.sh

  # ha相关
  HAInstallPath=/usr/local/ha
  HA_SUDO_SCRIPT_PATH=/opt/script
  HA_PROCESS_NAME="HAProcess"
  HA_CONNECT_PORT=30171
  HA_FILE_SYNC_PORT=30172
  PRIMARY_HA_NAME=""
  STANDBY_HA_NAME=""
  HA_STATUS_NORMAL="normal"
  HA_LOG_PATH=/opt/OceanProtect/logs/${NODE_NAME}/infrastructure
  HA_MARK_FILE="/usr/local/gaussdb/data/ha_started"
  HA_RES_GAUSSDB="gaussdb"
  HA_RES_FLOATIP="floatIp"
  G_IFCONFIG="$(which ifconfig)"
  G_IP="$(which ip)"

  IPPattern='^(\<([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\>\.){3}\<([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\>$'

  __HA_MANAGE_SH__='initialized'
fi

######################################################################
#   FUNCTION   : check_ip_valid
#   DESCRIPTION: 检查ip格式是否合法
#   输入参数为IP地址
######################################################################
function check_ip_valid() {
  local ip="$1"
  for special_ip in ${special_ips[@]}; do
    local ret=$(echo $ip | grep ${special_ip})
    if [ -n "$ret" ]; then
      return 1
    fi
  done
  if [[ "${ip}" =~ ${IPPattern} ]]; then
    return 0
  else
    return 1
  fi
}

######################################################################
#   FUNCTION   : ip和掩码做与计算
#   DESCRIPTION: 可用于检查两个ip是否在同一网段
#   输入参数为IP地址
######################################################################
function calc_ip_net() {
  local sip="$1"
  local snetmask="$2"

  check_ip_valid "$sip"
  if [ $? -ne 0 ]; then
    echo ""
    return 1
  fi

  local ipFIELD1=$(echo "$sip" | cut -d. -f1)
  local ipFIELD2=$(echo "$sip" | cut -d. -f2)
  local ipFIELD3=$(echo "$sip" | cut -d. -f3)
  local ipFIELD4=$(echo "$sip" | cut -d. -f4)

  local netmaskFIELD1=$(echo "$snetmask" | cut -d. -f1)
  local netmaskFIELD2=$(echo "$snetmask" | cut -d. -f2)
  local netmaskFIELD3=$(echo "$snetmask" | cut -d. -f3)
  local netmaskFIELD4=$(echo "$snetmask" | cut -d. -f4)

  local tmpret1=$((ipFIELD1 & netmaskFIELD1))
  local tmpret2=$((ipFIELD2 & netmaskFIELD2))
  local tmpret3=$((ipFIELD3 & netmaskFIELD3))
  local tmpret4=$((ipFIELD4 & netmaskFIELD4))

  echo "$tmpret1.$tmpret2.$tmpret3.$tmpret4"
}

######################################################################
#   FUNCTION   : check_network
#   DESCRIPTION: 检查内部通信网络连通性、浮动ip是否可用、仲裁网关是否可用
#   不需要输入
######################################################################
function check_network() {
  # ping connection_ip
  if [ -n "$CONNECTION_IP" ]; then
    ping -I vrf-srv -c 1 -I "$INTERNAL_IP" "$CONNECTION_IP" > /dev/null
    if [ $? -ne 0 ]; then
      log_error "[${FUNCNAME[0]}(),$LINENO] connect ip ${CONNECTION_IP} can not be connected."
      echo $CONNECTION_IP_ERROR
      return 1
    fi
  fi

  # ping float_ip
  if [ -n "$FLOAT_IP" ]; then
    # 检查浮动ip和内部通信网络ip是否在同一网段
    local netmask_bits="$(${G_IP} addr 2>/dev/null | grep $INTERNAL_IP | awk '{print $2}' | awk -F '/' '{print $2}')"
    local mask=$((0xffffffff << (32 - $netmask_bits)))
    local netmask=$((($mask >> 24) & 0xff)).$((($mask >> 16) & 0xff)).$((($mask >> 8) & 0xff)).$(($mask & 0xff))
    local tmpip1=$(calc_ip_net "$INTERNAL_IP" "$netmask")
    local tmpip2=$(calc_ip_net "$FLOAT_IP" "$netmask")
    echo "tmpip1=$tmpip1, tmpip2=$tmpip2"
    if [ "$tmpip1" != "$tmpip2" ]; then
      log_error "[${FUNCNAME[0]}(),$LINENO] float ip ${FLOAT_IP} and internal ip ${INTERNAL_IP} not in the same network segment."
      echo $FLOAT_IP_ERROR
      return 1
    fi
    # 判断浮动ip和本机网络ip是否重复
    # 如果浮动ip不存在，则校验连通性，ip不通，则可配置, 否则，浮动ip不可配置。
    # 如果浮动ip存在，且是同一个网卡， 则可以配置浮动ip。
    # 如果浮动ip存在，且不是同一个网卡， 说明浮动ip重复，不可配置。
    ${G_IP} addr | grep "$FLOAT_IP/" 2>/dev/null
    # 浮动ip不存在
    if [ $? -ne 0 ]; then
      # 判断浮动ip和vrf-srv的连通性，通则报错
      ping -I vrf-srv -I $INTERNAL_IP -c 1 $FLOAT_IP >/dev/null
      if [ $? -eq 0 ]; then
        log_error "[${FUNCNAME[0]}(),$LINENO] float ip ${FLOAT_IP} is connected and can not be used."
        echo $FLOAT_IP_ERROR
        return 1
      fi
      log_info "[${FUNCNAME[0]}(),$LINENO] float ip ${FLOAT_IP} is unconnected and can be used."
    else
      # 浮动ip能查到
      local float_ip_network=$(${G_IP} addr | grep $FLOAT_IP | awk '{print $NF}')
      local internal_ip_network=$(${G_IP} addr | grep $INTERNAL_IP | awk '{print $NF}')
      if [ "$internal_ip_network" == "$float_ip_network" ]; then
        log_info "[${FUNCNAME[0]}(),$LINENO] float ip ${FLOAT_IP},${internal_ip_network} is exist in the same eth ${float_ip_network}
              and needn't to config."
      else
        log_error "[${FUNCNAME[0]}(),$LINENO] float ip ${FLOAT_IP},${internal_ip_network} is exist in other eth ${float_ip_network}
               and can not be used."
        echo $FLOAT_IP_ERROR
        return 1
      fi
    fi
  fi

  # ping gateway_ips
  GATEWAY_IPS_ARR=(${GATEWAY_IPS//,/ })
  for gateway_ip in "${GATEWAY_IPS_ARR[@]}"; do
    # 判断仲裁网关和本机网络ip是否重复，重复则报错
    ${G_IP} addr | grep "$gateway_ip/" 2>/dev/null
    if [ $? -eq 0 ]; then
      log_error "[${FUNCNAME[0]}(),$LINENO] gateway ip ${gateway_ip} is the same as host ip and can not be used."
      echo $GATEWAY_IPS_ERROR
      return 1
    fi

    # 判断仲裁网关和vrf-srv的连通性，不通，则报错。
    ping -I vrf-srv -I $INTERNAL_IP -c 1 $gateway_ip >/dev/null
    if [ $? -ne 0 ]; then
      log_error "[${FUNCNAME[0]}(),$LINENO] gateway ip ${gateway_ip} is unconnected and can not be used."
      echo $GATEWAY_IPS_ERROR
      return 1
    fi
  done
  log_info "[${FUNCNAME[0]}(),$LINENO] check network success."
  echo $CHECK_NETWORK_SUCCESS
  return 0
}

######################################################################
#   FUNCTION   : add_ha
#   DESCRIPTION: 添加HA成员，配置数据库主从链路，配置HA主从链路，启动HA
#   不需要输入
######################################################################
function add_ha() {
  if [ "${ROLE}" == "primary" ]; then
    PRIMARY_HA_NAME=$LOCAL_HA_NAME
    STANDBY_HA_NAME=$PEER_HA_NAME
    add_ha_for_primary
    retVal=$?
  else
    PRIMARY_HA_NAME=$PEER_HA_NAME
    STANDBY_HA_NAME=$LOCAL_HA_NAME
    add_ha_for_standby
    retVal=$?
  fi
  return $retVal
}

######################################################################
#   FUNCTION   : check_ha_status
#   DESCRIPTION: 检查ha是否正常启动
#   需要输入ha_name
######################################################################
function check_ha_status() {
  local ha_name=$1
  local check_status=$2
  log_info "[${FUNCNAME[0]}(),$LINENO] check ${ha_name} status, expect status: ${check_status}."
  local status=$(sudo ${HA_SUDO_SCRIPT_PATH}/ha_sudo.sh ha_status | grep "^NodeName.*HostName.*HAVersion" -A 2 | grep -v "^NodeName.*HostName.*HAVersion" | grep -w "^${ha_name}[[:space:]]*" | awk '{print $7}')
  if [ "${status}" == "${check_status}" ]; then
    return 0
  fi
  log_error "[${FUNCNAME[0]}(),$LINENO] check ${ha_name} status not match, expect status: ${check_status}, actual status: ${status}"
  return 1
}

######################################################################
#   FUNCTION   : check_res_status
#   DESCRIPTION: 检查资源是否正常启动
#   需要输入本地ha名称，资源明显
######################################################################
function check_res_status() {
  local ha_name=$1
  local res_name=$2
  local check_status=$3
  log_info "[${FUNCNAME[0]}(),$LINENO] check ${ha_name} resource ${res_name} status, expect status: ${check_status}."
  # 等待1min，保证备节点同步主节点数据
  local num=0
  local res=0
  while [ $num -le 10 ]; do
    local status=$(sudo ${HA_SUDO_SCRIPT_PATH}/ha_sudo.sh ha_status | grep "^NodeName.*ResName.*ResStatus" -A 2 | grep -v "^NodeName.*ResName.*ResStatus" | grep -w "^${ha_name}[[:space:]]*" | grep -w "${res_name}[[:space:]]*" | awk '{print $3}')
    if [ "${status}" == "${check_status}" ]; then
      return 0
    fi
    let num+=1
    sleep 6
  done
  log_error "[${FUNCNAME[0]}(),$LINENO] check resource ${res_name} status not match, expect status: ${check_status}, actual status: ${status}."
  return 1
}

######################################################################
#   FUNCTION   : check_db_status
#   DESCRIPTION: 检查数据库状态是否正常
#   需要输入预期角色和状态
######################################################################
function check_db_status() {
  local role=$1
  local check_status=$2
  log_info "[${FUNCNAME[0]}(),$LINENO] check db resource status, expect role: ${role}, expect status: ${check_status}."
  # 等待1min，保证备节点同步主节点数据
  local num=0
  local res=0
  while [ $num -le 10 ]; do
    # 获取数据库状态
    dbinfo=$(get_db_status_info)
    get_db_state "$dbinfo"
    log_info "[${FUNCNAME[0]}(),$LINENO] get db status, role: ${local_role}, status: ${db_state}"
    if [ "${local_role}" == "${role}" ] && [ "${db_state}" == "${check_status}" ]; then
      return 0
    fi
    let num+=1
    sleep 6
  done
  log_error "[${FUNCNAME[0]}(),$LINENO] check db resource status not match, expect status: ${check_status}, actual status: ${db_state}."
  return 1
}

######################################################################
#   FUNCTION   : config_ha
#   DESCRIPTION: 配置HA信息
#   需要输入主端链路ip，对端链路ip，本端ha名，对端ha名，角色（active/standby）
######################################################################
function config_ha() {
  if [ $# -ne 5 ]; then
    log_error "[${FUNCNAME[0]}(),$LINENO] input params invalid, config ha failed."
    return 1
  fi
  local primary_ip=$1
  local standby_ip=$2
  local local_ha_name=$3
  local peer_ha_name=$4
  local role=$5
  log_info "[${FUNCNAME[0]}(),$LINENO] config ha, params: ${primary_ip}, ${standby_ip}, ${local_ha_name}, ${peer_ha_name}, ${role}."
  local heartbeat_link="${PRIMARY_HA_NAME}:${primary_ip}:${HA_CONNECT_PORT},${STANDBY_HA_NAME}:${standby_ip}:${HA_CONNECT_PORT}"
  local file_sync_link="${PRIMARY_HA_NAME}:${primary_ip}:${HA_FILE_SYNC_PORT},${STANDBY_HA_NAME}:${standby_ip}:${HA_FILE_SYNC_PORT}"
  sudo ${HA_SUDO_SCRIPT_PATH}/ha_sudo.sh ha_config link double $local_ha_name $peer_ha_name $heartbeat_link $file_sync_link $FLOAT_IP $GATEWAY_IPS $role
  if [ $? -ne 0 ]; then
    log_error "[${FUNCNAME[0]}(),$LINENO] config ha param failed."
    return 1
  fi
  #配置ha日志位置
  sudo ${HA_SUDO_SCRIPT_PATH}/ha_sudo.sh ha_config log $HA_LOG_PATH
  if [ $? -ne 0 ]; then
    log_error "[${FUNCNAME[0]}(),$LINENO] config ha log path failed."
    return 1
  fi

  if [[ ${DEPLOY_TYPE} == "d0" || ${DEPLOY_TYPE} == "d1" || ${DEPLOY_TYPE} == "d2" || ${DEPLOY_TYPE} == "d6" ]]; then
    #配置证书
    sudo ${HA_SUDO_SCRIPT_PATH}/ha_sudo.sh ha_config cert $role $INTERNAL_IP $CONNECTION_IP
    if [ $? -ne 0 ]; then
      log_error "[${FUNCNAME[0]}(),$LINENO] config ha ssl failed."
      return 1
    fi
  fi
}

######################################################################
#   FUNCTION   : start_ha
#   DESCRIPTION: 启动HA进程
#   输入本地HA名
######################################################################
function start_ha() {
  log_info "[${FUNCNAME[0]}(),$LINENO] begin start ha $1."
  local ha_name=$1
  sudo ${HA_SUDO_SCRIPT_PATH}/ha_sudo.sh ha_start
  if [ $? -ne 0 ]; then
    log_error "[${FUNCNAME[0]}(),$LINENO] start ha failed."
    return 1
  fi
  return 0
}

######################################################################
#   FUNCTION   : stop_ha
#   DESCRIPTION: 停止HA进程
#   不需要输入
######################################################################
function stop_ha() {
  log_info "[${FUNCNAME[0]}(),$LINENO] begin stop ha."
  sudo ${HA_SUDO_SCRIPT_PATH}/ha_sudo.sh ha_stop
  if [ $? -ne 0 ]; then
    log_error "[${FUNCNAME[0]}(),$LINENO] stop ha failed."
    return 1
  fi
  log_info "[${FUNCNAME[0]}(),$LINENO] stop ha success."
  return 0
}

######################################################################
#   FUNCTION   : restart_ha
#   DESCRIPTION: 重启HA进程
#   不需要输入
######################################################################
function restart_ha() {
  log_info "[${FUNCNAME[0]}(),$LINENO] begin restart ha."
  local ha_name=$1
  stop_ha
  if [ $? -ne 0 ]; then
    return 1
  fi
  start_ha $ha_name
  if [ $? -ne 0 ]; then
    return 1
  fi
  log_info "[${FUNCNAME[0]}(),$LINENO] restart ha success."
  return 0
}

######################################################################
#   FUNCTION   : add_ha_for_primary
#   DESCRIPTION: 为主端配置HA，启动HA
#   不需要输入
######################################################################
function add_ha_for_primary() {
  log_info "[${FUNCNAME[0]}(),$LINENO] begin add ha for primary."
  #移除ha，预防残留，先停掉
  if [[ ${DEPLOY_TYPE} == "d0" || ${DEPLOY_TYPE} == "d1" || ${DEPLOY_TYPE} == "d2" || ${DEPLOY_TYPE} == "d6" ]]; then
    remove_ha
  else
    remove_ha keep_cert
  fi
  if [ $? -ne 0 ]; then
    log_error "[${FUNCNAME[0]}(),$LINENO] stop ha failed."
    return 1
  fi
  #配置数据库链路信息
  config_db_link $INTERNAL_IP $CONNECTION_IP
  if [ $? -ne 0 ]; then
    echo ${DATABASE_CONFIG_ERROR}
    return 1
  fi
  #配置ha信息
  config_ha $INTERNAL_IP $CONNECTION_IP $PRIMARY_HA_NAME $STANDBY_HA_NAME active
  if [ $? -ne 0 ]; then
    echo ${HA_CONFIG_ERROR}
    return 1
  fi
  #启动ha
  start_ha $PRIMARY_HA_NAME
  if [ $? -ne 0 ]; then
    echo ${HA_START_ERROR}
    return 1
  fi
  #检查资源状态是否正常
  check_db_status $DB_ROLE_PRIMARY $DB_RES_NORMAL
  if [ $? -ne 0 ]; then
    echo ${HA_START_ERROR}
    return 1
  fi
  #生成标记文件
  mkdir -p ${HAInstallPath}/local/tmp
  touch $HA_MARK_FILE
  if [ $? -ne 0 ]; then
    log_error "[${FUNCNAME[0]}(),$LINENO] generate ha mark file failed."
    echo ${HA_START_ERROR}
    return 1
  fi
  log_info "[${FUNCNAME[0]}(),$LINENO] add ha for primary success."
  echo ${ADD_HA_SUCCESS}
  return 0
}

######################################################################
#   FUNCTION   : add_ha_for_standby
#   DESCRIPTION: 为从端配置HA，启动HA
#   不需要输入
######################################################################
function add_ha_for_standby() {
  log_info "[${FUNCNAME[0]}(),$LINENO] begin add ha for standby."
  #移除ha，预防残留，先停掉
  remove_ha keep_cert
  if [ $? -ne 0 ]; then
    log_error "[${FUNCNAME[0]}(),$LINENO] stop ha failed."
    return 1
  fi
  #配置数据库链路信息
  config_db_link $INTERNAL_IP $CONNECTION_IP
  if [ $? -ne 0 ]; then
    echo ${DATABASE_CONFIG_ERROR}
    return 1
  fi
  #配置ha信息
  config_ha $CONNECTION_IP $INTERNAL_IP $STANDBY_HA_NAME $PRIMARY_HA_NAME standby
  if [ $? -ne 0 ]; then
    echo ${HA_CONFIG_ERROR}
    return 1
  fi
  #启动ha
  start_ha $STANDBY_HA_NAME
  if [ $? -ne 0 ]; then
    echo ${HA_START_ERROR}
    return 1
  fi
  #检查资源状态是否正常
  check_db_status $DB_ROLE_STANDBY $DB_RES_NORMAL
  if [ $? -ne 0 ]; then
    echo ${HA_START_ERROR}
    return 1
  fi
  #生成标记文件
  mkdir -p ${HAInstallPath}/local/tmp
  touch $HA_MARK_FILE
  if [ $? -ne 0 ]; then
    log_error "[${FUNCNAME[0]}(),$LINENO] generate ha mark file failed."
    echo ${HA_START_ERROR}
    return 1
  fi
  log_info "[${FUNCNAME[0]}(),$LINENO] add ha for standby success."
  echo ${ADD_HA_SUCCESS}
  return 0
}

######################################################################
#   FUNCTION   : allow_switchover
#   DESCRIPTION: 允许主备倒换函数
#   不需要输入
######################################################################
function allow_switchover() {
  log_info "[${FUNCNAME[0]}(),$LINENO] begin allow switchover."
  local query_res=$(sudo ${HA_SUDO_SCRIPT_PATH}/ha_sudo.sh ha_query_fsw)
  if [ "${query_res}" == "swap forbid cancel" ]; then
    log_info "[${FUNCNAME[0]}(),$LINENO] allow switchover success."
    return 0
  fi
  sudo ${HA_SUDO_SCRIPT_PATH}/ha_sudo.sh ha_allow_switchover $HA_PROCESS_NAME
  local query_res=$(sudo ${HA_SUDO_SCRIPT_PATH}/ha_sudo.sh ha_query_fsw)
  if [ "${query_res}" != "swap forbid cancel" ]; then
    log_info "[${FUNCNAME[0]}(),$LINENO] allow switchover failed."
    echo ${ALLOW_SWITCHOVER_ERROR}
    return 1
  fi
  log_info "[${FUNCNAME[0]}(),$LINENO] allow switchover success."
  echo ${ALLOW_SWITCHOVER_SUCCESS}
  return 0
}

######################################################################
#   FUNCTION   : forbid_switchover
#   DESCRIPTION: 禁止主备倒换函数
#   不需要输入
######################################################################
function forbid_switchover() {
  log_info "[${FUNCNAME[0]}(),$LINENO] begin forbid switchover."
  local query_res=$(sudo ${HA_SUDO_SCRIPT_PATH}/ha_sudo.sh ha_query_fsw)
  if [ "${query_res}" == "swap forbid" ]; then
    log_info "[${FUNCNAME[0]}(),$LINENO] forbid switchover success."
    return 0
  fi
  sudo ${HA_SUDO_SCRIPT_PATH}/ha_sudo.sh ha_forbid_switchover $HA_PROCESS_NAME
  local query_res=$(sudo ${HA_SUDO_SCRIPT_PATH}/ha_sudo.sh ha_query_fsw)
  if [ "${query_res}" != "swap forbid" ]; then
    log_info "[${FUNCNAME[0]}(),$LINENO] forbid switchover failed."
    echo ${FORBID_SWITCHOVER_ERROR}
    return 1
  fi
  log_info "[${FUNCNAME[0]}(),$LINENO] forbid switchover success."
  echo ${FORBID_SWITCHOVER_SUCCESS}
  return 0
}

######################################################################
#   FUNCTION   : modify_ha
#   DESCRIPTION: 修改HA操作入口函数
#   不需要输入
######################################################################
function modify_ha() {
  log_info "[${FUNCNAME[0]}(),$LINENO] begin modify ha."

  stop_ha
  if [ $? -ne 0 ]; then
    return 1
  fi

  modify_float_ip_and_gateway
  if [ $? -ne 0 ]; then
    return 1
  fi

  #重启HA进程
  if [ "${ROLE}" == "primary" ]; then
    start_ha $PRIMARY_HA_NAME
  else
    start_ha $STANDBY_HA_NAME
  fi
  if [ $? -ne 0 ]; then
    echo ${MODIFY_HA_ERROR}
    log_error "[${FUNCNAME[0]}(),$LINENO] restart ha failed."
    return 1
  fi
  log_info "[${FUNCNAME[0]}(),$LINENO] modify ha success."
  echo ${MODIFY_HA_SUCCESS}
  return 0
}

######################################################################
#   FUNCTION   : modify_float_ip_and_gateway
#   DESCRIPTION: 修改配置文件中的float_ip和gateway
#   不需要输入
######################################################################
function modify_float_ip_and_gateway() {
  log_info "[${FUNCNAME[0]}(),$LINENO] begin modify float ip and gateway."
  pre_float_ip=$(get_float_ip)
  #修改浮动ip
  if [ -n "$FLOAT_IP" ]; then
    sudo ${HA_SUDO_SCRIPT_PATH}/ha_sudo.sh ha_config floatIp $FLOAT_IP
    if [ $? -ne 0 ]; then
      echo ${MODIFY_HA_ERROR}
      log_error "[${FUNCNAME[0]}(),$LINENO] modify float ip ${FLOAT_IP} failed."
      return 1
    fi
  fi
  sudo ip rule del from all to $pre_float_ip

  pre_gw_ip=$(get_gateway_ip)
  #修改网关
  if [ -n "$GATEWAY_IPS" ]; then
    sudo ${HA_SUDO_SCRIPT_PATH}/ha_sudo.sh ha_config gatewayIp $GATEWAY_IPS
    if [ $? -ne 0 ]; then
      echo ${MODIFY_HA_ERROR}
      log_error "[${FUNCNAME[0]}(),$LINENO] modify gateway ips ${GATEWAY_IPS} failed."
      return 1
    fi
  fi
  sudo ip rule del from all to $pre_gw_ip
  return 0
}

######################################################################
#   FUNCTION   : remove_ha
#   DESCRIPTION: 移除HA操作入口函数
#   不需要输入
######################################################################
function remove_ha() {
  local keep_cert=$1
  log_info "[${FUNCNAME[0]}(),$LINENO] begin remove ha."
  gw_ip=$(get_gateway_ip)
  float_ip=$(get_float_ip)
  local peer_name=$(get_peer_name)
  local peer_ip=$(get_ip_by_ha_name ${peer_name})
  log_info "[${FUNCNAME[0]}(),$LINENO] get ha ip  ${gw_ip} ${float_ip}, ${peer_ip}."
  if [ "$gw_ip" != "undefined" ]; then
    sudo ip rule del from all to $gw_ip
    sudo ip rule del from all to $float_ip
    sudo ip rule del from all to $peer_ip
  fi
  #删除标记文件
  rm -f $HA_MARK_FILE
  #停止HA
  stop_ha
  if [ $? -ne 0 ]; then
    log_error "[${FUNCNAME[0]}(),$LINENO] stop ha failed."
    echo ${HA_STOP_ERROR}
    return 1
  fi
  #清除证书
  if [ "$keep_cert" != "keep_cert" ]; then
    clear_cert
  fi
  #清除数据库链路配置信息
  clear_db_link
  #重启数据库为Normal状态
  restart_to_normal
  if [ $? -ne 0 ]; then
    log_error "[${FUNCNAME[0]}(),$LINENO] restart database to normal failed."
    echo ${DATABASE_RESTART_NORMAL_ERROR}
    return 1
  fi

  log_info "[${FUNCNAME[0]}(),$LINENO] remove ha success."
  echo ${REMOVE_HA_SUCCESS}
  return 0
}

######################################################################
#   FUNCTION   : rollback
#   DESCRIPTION: HA操作回滚入口函数
#   不需要输入
######################################################################
function rollback() {
  case "$ROLLBACK_ACTION" in
  add)
    rollback_for_add_ha
    ret=$?
    return $ret
    ;;
  modify)
    rollback_for_modify_ha
    ret=$?
    return $ret
    ;;
  remove)
    rollback_for_remove_ha
    ret=$?
    return $ret
    ;;
  update_ha_cert)
    rollback_for_ha_cert
    ret=$?
    return $ret
    ;;
  *)
    return 1
    ;;
  esac
}

######################################################################
#   FUNCTION   : rollback_for_add_ha
#   DESCRIPTION: 为添加HA操作做回滚，需要删除标志文件, 停止HA，还原gaussdb为单机模式
#   不需要输入
######################################################################
function rollback_for_add_ha() {
  log_info "[${FUNCNAME[0]}(),$LINENO] begin rollback for add ha."
  remove_ha
  if [ $? -ne 0 ]; then
    log_error "[${FUNCNAME[0]}(),$LINENO] rollback ha failed."
    return 1
  fi
  log_info "[${FUNCNAME[0]}(),$LINENO] rollback ha success."
  echo ${ROLLBACK_HA_SUCCESS}
  return 0
}

######################################################################
#   FUNCTION   : rollback_for_modify_ha
#   DESCRIPTION: 为修改HA操作做回滚，需要恢复之前的浮动ip和网关配置
#   不需要输入
######################################################################
function rollback_for_modify_ha() {
  log_info "[${FUNCNAME[0]}(),$LINENO] begin rollback for modify ha."
  modify_ha
  if [ $? -ne 0 ]; then
    log_error "[${FUNCNAME[0]}(),$LINENO] rollback ha failed."
    return 1
  fi
  log_info "[${FUNCNAME[0]}(),$LINENO] rollback ha success."
  echo ${ROLLBACK_HA_SUCCESS}
  return 0
}

######################################################################
#   FUNCTION   : rollback_for_remove_ha
#   DESCRIPTION: 为移除HA操作做回滚，需要恢复HA进程
#   不需要输入
######################################################################
function rollback_for_remove_ha() {
  log_info "[${FUNCNAME[0]}(),$LINENO] begin rollback for remove ha."
  add_ha
  if [ $? -ne 0 ]; then
    log_error "[${FUNCNAME[0]}(),$LINENO] rollback ha failed."
    return 1
  fi
  log_info "[${FUNCNAME[0]}(),$LINENO] rollback ha success."
  echo ${ROLLBACK_HA_SUCCESS}
  return 0
}

######################################################################
#   FUNCTION   : get_role
#   DESCRIPTION: 获取ha的角色
#   不需要输入
######################################################################
function get_role() {
  log_info "[${FUNCNAME[0]}(),$LINENO] get ha role."
  local local_name=$(sed -n 's/.*local name=\"\(.*\)\".*/\1/p' ${G_HA_INSTALL_PATH}/local/hacom/conf/hacom_local.xml)
  local role=$(sudo ${HA_SUDO_SCRIPT_PATH}/ha_sudo.sh ha_status | grep "^NodeName.*HostName.*HAVersion" -A 2 | grep -v "^NodeName.*HostName.*HAVersion" | grep -w "^${local_name}[[:space:]]*" | awk '{print $6}')
  local run_phase=$(sudo ${HA_SUDO_SCRIPT_PATH}/ha_sudo.sh ha_status | grep "^NodeName.*HostName.*HAVersion" -A 2 | grep -v "^NodeName.*HostName.*HAVersion" | grep -w "^${local_name}[[:space:]]*" | awk '{print $8}')
  # 当ha运行处于稳态时，才返回相应角色
  if [[ "$run_phase" == "Actived" || "$run_phase" == "Deactived" ]]; then
    echo ${role}
  fi
  return 0
}

######################################################################
#   FUNCTION   : update_ha_cert
#   DESCRIPTION: 更新HA证书
#   不需要输入
######################################################################
function update_ha_cert() {
  log_info "[${FUNCNAME[0]}(),$LINENO] update ha cert."
  if [ "$ROLE" == "primary" ]; then
    local role="active"
  else
    local role="standby"
  fi
  #重新配置证书
  sudo ${HA_SUDO_SCRIPT_PATH}/ha_sudo.sh ha_config cert $role $INTERNAL_IP $CONNECTION_IP
  if [ $? -ne 0 ]; then
    log_error "[${FUNCNAME[0]}(),$LINENO] config ha cert failed."
    return 1
  fi
  #重启HA
  if [ "${ROLE}" == "primary" ]; then
    restart_ha $PRIMARY_HA_NAME
  else
    restart_ha $STANDBY_HA_NAME
  fi
  if [ $? -ne 0 ]; then
    log_error "[${FUNCNAME[0]}(),$LINENO] restart ha failed."
    return 1
  fi
  log_info "[${FUNCNAME[0]}(),$LINENO] update ha cert success."
  echo ${UPDATE_HA_CERT_SUCCESS}
  return 0
}

######################################################################
#   FUNCTION   : clear_cert
#   DESCRIPTION: 配置HA信息
#   需要输入主端链路ip，对端链路ip，本端ha名，对端ha名，角色（active/standby）
######################################################################
function clear_cert() {
  #清除证书
  sudo ${HA_SUDO_SCRIPT_PATH}/ha_sudo.sh ha_config remove_cert
  if [ $? -ne 0 ]; then
    log_error "[${FUNCNAME[0]}(),$LINENO] clear cert failed."
    return 1
  fi
  log_info "[${FUNCNAME[0]}(),$LINENO] clear cert success."
  echo ${REMOVE_HA_CERT_SUCCESS}
  return 0
}

######################################################################
#   FUNCTION   : rollback_for_ha_cert
#   DESCRIPTION: 回退HA证书
#   不需要输入
######################################################################
function rollback_for_ha_cert() {
  #回退证书
  sudo ${HA_SUDO_SCRIPT_PATH}/ha_sudo.sh ha_config cert_rollback
  if [ $? -ne 0 ]; then
    log_error "[${FUNCNAME[0]}(),$LINENO] config ha cert rollback failed."
    return 1
  fi
  #重启HA
  if [ "${ROLE}" == "primary" ]; then
    restart_ha $PRIMARY_HA_NAME
  else
    restart_ha $STANDBY_HA_NAME
  fi
  if [ $? -ne 0 ]; then
    log_error "[${FUNCNAME[0]}(),$LINENO] restart ha failed."
    return 1
  fi
  log_info "[${FUNCNAME[0]}(),$LINENO] rollback ha cert success."
  echo ${ROLLBACK_HA_CERT_SUCCESS}
  return 0
}

function main() {
  case "$ACTION" in
  check_network)
    check_network
    exit $?
    ;;
  add)
    add_ha
    exit $?
    ;;
  modify)
    modify_ha
    exit $?
    ;;
  remove)
    remove_ha
    exit $?
    ;;
  rollback)
    rollback
    exit $?
    ;;
  switchover)
    allow_switchover
    exit $?
    ;;
  unswitchover)
    forbid_switchover
    exit $?
    ;;
  get_role)
    get_role
    exit $?
    ;;
  update_ha_cert)
    update_ha_cert
    exit $?
    ;;
  *)
    echo "ERROR: Invalid parameter: $*."
    exit 1
    ;;
  esac
}

# 解析参数
ACTION=""
ROLE=""
CONNECTION_IP=""
FLOAT_IP=""
GATEWAY_IPS=""
INTERNAL_IP=""
ROLLBACK_ACTION=""
LOCAL_HA_NAME=""
PEER_HA_NAME=""

in_param_num="$#"
declare script_name="$(basename $0)"
log_warn "[${FUNCNAME[0]}(),$LINENO] enter the script ${script_name}($in_param_num)"
umask 0007
while getopts ":a:r:c:f:g:i:m:l:p:" opt; do # 首位的冒号表示不打印错误信息
  case $opt in
  a)
    log_info "[${FUNCNAME[0]}(),$LINENO] set ACTION:${OPTARG}"
    ACTION="$OPTARG"
    ;;
  r)
    log_info "[${FUNCNAME[0]}(),$LINENO] set ROLE:${OPTARG}"
    ROLE="$OPTARG"
    ;;
  c)
    log_info "[${FUNCNAME[0]}(),$LINENO] set CONNECTION_IP:${OPTARG}"
    CONNECTION_IP="$OPTARG"
    ;;
  f)
    log_info "[${FUNCNAME[0]}(),$LINENO] set FLOAT_IP:${OPTARG}"
    FLOAT_IP="$OPTARG"
    ;;
  g)
    log_info "[${FUNCNAME[0]}(),$LINENO] set GATEWAY_IPS:${OPTARG}"
    GATEWAY_IPS="$OPTARG"
    ;;
  i)
    log_info "[${FUNCNAME[0]}(),$LINENO] set INTERNAL_IP:${OPTARG}"
    INTERNAL_IP="$OPTARG"
    ;;
  m)
    log_info "[${FUNCNAME[0]}(),$LINENO] set ROLLBACK_ACTION:${OPTARG}"
    ROLLBACK_ACTION="$OPTARG"
    ;;
  l)
    log_info "[${FUNCNAME[0]}(),$LINENO] set LOCAL_HA_NAME:${OPTARG}"
    LOCAL_HA_NAME="$OPTARG"
    ;;
  p)
    log_info "[${FUNCNAME[0]}(),$LINENO] set PEER_HA_NAME:${OPTARG}"
    PEER_HA_NAME="$OPTARG"
    ;;
  :)
    exit 1
    ;;
  ?) #当有不认识的选项的时候arg为?
    # echo "Invalid option: -$OPTARG index:$OPTIND"
    ;;
  esac
done

main "$@"
exit $?
