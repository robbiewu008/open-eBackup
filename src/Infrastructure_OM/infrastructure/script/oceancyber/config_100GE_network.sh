#!/usr/bin/bash

bond_100GE_network="/opt/huawei/oceancyber/bond_100GE_network.sh"
network_dir="/etc/sysconfig/network-scripts"
service_bond_name="eth3"
bond_log_path="/opt/huawei/oceancyber/bond_100GE_network_temp.log"

# 100GE业务网口总线信息
GE_bus_info0="0000:83:00.0"
GE_bus_info1="0000:84:00.0"

LOG_PATH='/var/log/config_100GE_network.log'

# 打印日志
log_info() {
  time=$(date "+%Y-%m-%d %H:%M:%S")
  user=$(whoami)
  echo -e "\033[32m[${time}][USER:${user}][INFO] ${FUNCNAME[1]}(): $*\033[0m" | tee -a $LOG_PATH
}

# 打印错误日志
log_error() {
  time=$(date "+%Y-%m-%d %H:%M:%S")
  user=$(whoami)
  echo -e "\033[31m[${time}][USER:${user}][ERROR] ${FUNCNAME[1]}(), line ${BASH_LINENO[0]}: $*\033[0m" | tee -a $LOG_PATH
  echo -e "\033[31mYou can get details in: ${LOG_PATH}\033[0m"
}

# 根据总线信息查询网口名称
function get_network_name_by_bus_info() {
    bus_info=$1
    bus_path=$(find "/sys/devices" -name "${bus_info}")
    if [ -z "${bus_path}" ];then
        echo "1"
        return
    fi
    network_name=$(ls "${bus_path}/net")
    echo "$network_name"
}

function check() {
    # 100GE网卡是否存在
    num=$(lspci | grep -c 0222)
    if [ "$num" -eq 0 ]; then
        log_error "100GE network card is not exist."
        exit 1
    fi
    # cpu及欧拉操作系统是否支持100GE网卡
    cpu_type=$(uname -r | grep "aarch64" | awk -F'.' '{print $NF}')
    if [ "$cpu_type" != 'aarch64' ]; then
        log_error "The CPU does not support 100GE network cards."
        exit 1
    fi
    # 操作系统是否支持100GE网卡，取版本号，支持12及以上版本
    euleros_version=$(uname -r | grep "euleros" | awk -F'.' '{print $(NF-1)}' | awk -F'r' '{print $NF}')
    if [ "$((euleros_version))" -lt 12 ]; then
        log_error "The os version does not support 100GE network cards."
        exit 1
    fi
    GE_port1=$(get_network_name_by_bus_info "${GE_bus_info0}")
    GE_port2=$(get_network_name_by_bus_info "${GE_bus_info1}")
    if [ "${GE_port1}" == "1" ] || [ "${GE_port2}" == "1" ];then
        log_error "100GE service port not exist. Please check whether the network card is inserted into the correct slot or whether the network card driver is installed correctly."
        exit 1
    fi
}

function config() {
    if [ "$(echo "$1" | tr '[:lower:]' '[:upper:]')"  == 'VLAN' ]; then
        config_eth3_vlan "$2" "$3" "$4" "$5"
    else
        config_eth3 "$1" "$2" "$3"
    fi
}

function start_network() {
    network_name=$1
    ifup_res=$(ifup "$network_name")
    res_status=$(echo "$ifup_res" | awk '{print $2}')
    if [ $? -ne 0 ] || [ "${res_status}" != "successfully" ]; then
        log_error "Failed to start ifup bond: ${network_name}"
        exit 1
    fi
}

function config_eth3() {
    # 1、参数校验
    IP_ADDRESS=$1
    NETMASK=$2
    GATEWAY=$3
    if [ -n "$GATEWAY" ]; then
        params=("$IP_ADDRESS" "$NETMASK" "$GATEWAY")
    else
        params=("$IP_ADDRESS" "$NETMASK")
    fi
    check_ip "${params[@]}"
    # 2、IP冲突检测
    check_ip_conflict "$IP_ADDRESS"
    # 3、bond_100GE_network.sh脚本是否存在
    if [ ! -f "$bond_100GE_network" ]; then
        log_error "Can not bond 100GE network card."
        exit 1
    fi
    # 4、执行bond_100GE_network.sh脚本
    execute_band
    # 5、修改IP
    network_file_bond="${network_dir}/ifcfg-${service_bond_name}"
    sed -i /IPADDR/d $network_file_bond
    sed -i /NETMASK/d $network_file_bond
    echo "IPADDR=$IP_ADDRESS" >> $network_file_bond
    echo "NETMASK=$NETMASK" >> $network_file_bond
    if [ -n "$GATEWAY" ]; then
        sed -i /GATEWAY/d $network_file_bond
        echo "GATEWAY=$GATEWAY" >> $network_file_bond
    fi
    # 6、激活eth3
    start_network $service_bond_name
    sleep 3
    # 7、查看网卡工作状态
    result=$(ip a | grep "$IP_ADDRESS" | awk '{print $NF}')
    if [ "$result" != $service_bond_name ]; then
        log_error "Config IP failed."
        exit 1
    fi
}

function check_ip_conflict() {
    IP_ADDRESS=$1
    net_card=$(ip a | grep "$IP_ADDRESS" | awk '{print $NF}')
    if [ -n "$net_card" ]; then
      log_error "IP address $IP_ADDRESS is already occupied by network card $net_card, and there may be an IP conflict."
      exit 1
    fi
    if ping -c 1 "$IP_ADDRESS" > /dev/null 2>&1; then
      log_error "IP address $IP_ADDRESS is already occupied, and there may be an IP conflict."
      exit 1
    fi
}

function config_eth3_vlan() {
    # 1、参数校验
    check_id "$1"
    VLAN_ID=${service_bond_name}.$1
    IP_ADDRESS=$2
    NETMASK=$3
    GATEWAY=$4
    if [ -n "$GATEWAY" ]; then
        params=("$IP_ADDRESS" "$NETMASK" "$GATEWAY")
    else
        params=("$IP_ADDRESS" "$NETMASK")
    fi
    check_ip "${params[@]}"
    # 2、IP冲突检测
    check_ip_conflict "$IP_ADDRESS"
    # 3、bond_100GE_network.sh脚本是否存在
    if [ ! -f "$bond_100GE_network" ]; then
        log_error "Can not bond 100GE network card."
        exit 1
    fi
    # 4、执行bond_100GE_network.sh脚本
    execute_band
    # 5、修改IP
    network_file_bond="${network_dir}/ifcfg-${service_bond_name}"
    vlan_network_file_bond="${network_dir}/ifcfg-$VLAN_ID"
    config_vlan_ip "$network_file_bond" "$vlan_network_file_bond" "$IP_ADDRESS" "$NETMASK" "$GATEWAY"

    # 6、激活eth3-vlan
    start_network "$VLAN_ID"
    sleep 3
    start_network $service_bond_name
    sleep 3
    # 7、查看vlan网卡工作状态
    result=$(ip a | grep "$IP_ADDRESS" | awk '{print $NF}')
    if [ "$result" != "$VLAN_ID" ]; then
        log_error "Config IP failed."
        exit 1
    fi
}

function config_vlan_ip() {
    network_file_bond=$1
    vlan_network_file_bond=$2
    IP_ADDRESS=$3
    NETMASK=$4
    GATEWAY=$5
    sed -i /IPADDR/d "$network_file_bond"
    sed -i /NETMASK/d "$network_file_bond"
    if [ -f "$vlan_network_file_bond" ]; then
        sed -i /IPADDR/d "$vlan_network_file_bond"
        sed -i /NETMASK/d "$vlan_network_file_bond"
    else
        touch "$vlan_network_file_bond"
        echo "DEVICE=${VLAN_ID}" >> "$vlan_network_file_bond"
        echo "ONBOOT=yes" >> "$vlan_network_file_bond"
        echo "VLAN=yes" >> "$vlan_network_file_bond"
        echo "BOOTPROTO=static" >> "$vlan_network_file_bond"
        echo "STARTMODE=auto" >> "$vlan_network_file_bond"
    fi
    echo "IPADDR=$IP_ADDRESS" >> "$vlan_network_file_bond"
    echo "NETMASK=$NETMASK" >> "$vlan_network_file_bond"
    if [ -n "$GATEWAY" ]; then
        sed -i /GATEWAY/d "$vlan_network_file_bond"
        echo "GATEWAY=$GATEWAY" >> "$vlan_network_file_bond"
    fi
}

# 检查eth3是否绑定
function check_service_bond() {
    res=$(ls /proc/net/bonding/)
    if [[ ${res} =~ ${service_bond_name} ]]; then
        log_error "${service_bond_name} is exist, please shut it down first."
        exit 1
    fi
}

function execute_band() {
    check_service_bond
    # 执行bond_100GE_network.sh脚本
    if [ -f "$bond_log_path" ]; then
        rm $bond_log_path
    else
        touch $bond_log_path
    fi
    sh $bond_100GE_network > $bond_log_path 2>&1
    if [ $? -eq 1 ]; then
        log_error "$(tail -n 1 $bond_log_path)"
        rm "$bond_log_path"
        exit 1
    else
        result=$(tail -n 1 "$bond_log_path")
        bond_status=$(echo "$result" | awk -F' ' '{print $2}')
        if [ "$bond_status" == 'failed' ]; then
            log_error "$result"
            rm "$bond_log_path"
            exit 1
        fi
    fi
    rm "$bond_log_path"
}

function check_id() {
  # VLAN ID必须为介于1-4094之间的整数
    if ! [[ $1 -ge 1 && $1 -le 4094 ]]; then
        log_error "'$1' is not a valid vlan id. It must be an integer between 1 and 4094."
        exit 1
    fi
}

function check_ip() {
    for address in "$@" ; do
        if ! check_ipv4 "$address"; then
            log_error "The address ${address} is invalid!"
            exit 1
        fi
    done
}

# 检查ipv4地址合法性
function check_ipv4() {
  ipStr="$1"
  isValid=$(echo "${ipStr}"|grep -E "^([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\.([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\.([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\.([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])$" | wc -l);
  if [ "${isValid}" -gt 0 ];then
    return 0
  fi
  return 1
}

function main() {
    log_info "Start checking whether the 100GE network card is available."
    check
    log_info "The 100GE network card is available."

    log_info "Start configuring the 100GE network card."
    config "$@"
    log_info "The 100GE network card has been configured successfully."
}

main "$@"