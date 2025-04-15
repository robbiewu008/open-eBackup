#!/usr/bin/bash

network_dir="/etc/sysconfig/network-scripts"

# 管理平面IP
self_manage_ip="192.168.128.101"
self_manage_netmask="255.255.0.0"

# 容器平面IP
self_container_ip="172.16.128.101"
self_container_netmask="255.255.0.0"

# 管理网口总线信息
manage_bus_info0="0000:7d:00.0"
manage_bus_info1="0000:7d:00.1"
manage_bus_info2="0000:7d:00.2"

# 25GE业务网口总线信息
bus_info0="0000:bd:00.0"
bus_info1="0000:bd:00.1"

# 100GE业务网口总线信息
GE_bus_info0="0000:81:00.0"
GE_bus_info1="0000:81:00.1"

manage_bond="eth0"
container_new_name="eth1"
service_bond="eth2"
service_bond1="eth3"

# 根据总线信息查询网口名称
function get_network_name_by_bus_info() {
  bus_info=$1
  bus_path=$(find "/sys/devices" -name "${bus_info}")
  if [ ! -n "${bus_path}" ]; then
    echo "1"
    return
  fi
  network_name=$(ls "${bus_path}/net")
  echo ${network_name}
}

# 端口组bond
function config_network_bond() {
  inner_nic1=$1
  inner_nic2=$2
  bondname=$3

  if [ ! -f "${network_dir}/ifcfg-${inner_nic1}" ] || [ ! -f "${network_dir}/ifcfg-${inner_nic2}" ]; then
    echo "The network configuration file of ${inner_nic1} or ${inner_nic2} does not exist."
    return
  fi

  file_list=("${inner_nic1}" "${inner_nic2}")
  for file in ${file_list[*]}; do
    network_file="${network_dir}/ifcfg-${file}"
    sed -i '1,$d' ${network_file}
    echo "DEVICE=${file}" >>${network_file}
    echo "ONBOOT=yes" >>${network_file}
    echo "BOOTPROTO=none" >>${network_file}
    echo "STARTMODE=auto" >>${network_file}
    echo "USERCTL=no" >>${network_file}
    echo "SLAVE=yes" >>${network_file}
    echo "MASTER=${bondname}" >>${network_file}
  done

  network_file_bond="${network_dir}/ifcfg-${bondname}"
  if [ -f ${network_file_bond} ]; then
    rm ${network_file_bond}
  fi
  touch ${network_file_bond}
  echo "DEVICE=${bondname}" >>${network_file_bond}
  echo "BONDING_MASTER=yes" >>${network_file_bond}
  echo "BOOTPROTO=static" >>${network_file_bond}
  echo "STARTMODE=auto" >>${network_file_bond}
  echo "BONDING_OPTS='mode=1 miimon=200'" >>${network_file_bond}
}

# 配置IP
function config_network_ip() {
  bondname=$1
  ip=$2
  netmask=$3

  network_file_bond="${network_dir}/ifcfg-${bondname}"
  sed -i /BOOTPROTO/d ${network_file_bond}
  sed -i /ONBOOT/d ${network_file_bond}
  echo "BOOTPROTO=static" >>${network_file_bond}
  echo "ONBOOT=yes" >>${network_file_bond}

  echo "IPADDR=$ip" >>${network_file_bond}
  echo "NETMASK=$netmask" >>${network_file_bond}
  sed -i /GATEWAY/d ${network_file_bond}
}

# 容器网口改名并配置IP
function config_container_network() {
  old_nic=$1
  new_nic=$2
  ip=$3
  netmask=$4
  mac_address=$(cat /sys/class/net/${old_nic}/address)

  rules_file="/etc/udev/rules.d/50-persistent-net.rules"
  echo 'SUBSYSTEM=="net", ACTION=="add", DRIVERS=="?*", ATTR{address}=="'${mac_address}'", NAME="'${new_nic}'"' >${rules_file}

  network_file="${network_dir}/ifcfg-${new_nic}"
  cp "${network_dir}/ifcfg-${old_nic}" ${network_file}

  if [ "${old_nic}" == "${new_nic}" ]; then
    echo "$(date) same nicname o: ${old_nic}  n: ${new_nic}"
  else
    echo "$(date) diff nicname o: ${old_nic}  n: ${new_nic}"
    rm -rf "$network_dir/ifcfg-${old_nic}"
  fi

  sed -i /NAME/d ${network_file}
  sed -i /DEVICE/d ${network_file}
  sed -i /IPADDR/d ${network_file}
  sed -i /NETMASK/d ${network_file}
  sed -i /GATEWAY/d ${network_file}
  echo "NAME=${new_nic}" >>${network_file}
  echo "DEVICE=${new_nic}" >>${network_file}
  echo "IPADDR=${ip}" >>${network_file}
  echo "NETMASK=${netmask}" >>${network_file}

  sed -i /ONBOOT/d ${network_file}
  sed -i /BOOTPROTO/d ${network_file}
  echo "ONBOOT=yes" >>${network_file}
  echo "BOOTPROTO=static" >>${network_file}
}

function config_disk() {
  disk_info=$(lsblk -nl)
  for info in ${disk_info[*]}; do
    echo ${info} | grep "T"
    if [[ "$?" -ne "0" ]]; then
      continue
    fi
    info_size=$(echo ${info} | awk -F'T' '{print $1}')
    if python -c "exit(0 if ${info_size} > 8 else 1)"; then
      disk_size=${info}
      break
    fi
  done
  if python -c "exit(0 if ${info_size} > 8 else 1)"; then
    disk_name=$(lsblk | grep "${info}" | awk -F' ' '{print $1}')
    disk_name=$(echo ${disk_name} | awk -F' ' '{print $1}')
    pvcreate /dev/${disk_name}
    vgcreate ce-vg /dev/${disk_name}
    lvcreate -y -L 8T -i1 -I64 -n d-data-lv ce-vg
    mkfs.ext4 /dev/mapper/ce--vg-d--data--lv
    mkdir -p /opt/cyberengine/d-data
    uuid=$(blkid /dev/mapper/ce--vg-d--data--lv | awk -F "\"" '{print $2}')
    echo "UUID=${uuid} /opt/cyberengine/d-data ext4 defaults 0 0" >>/etc/fstab
  else
    echo "fail"
    exit 1
  fi
}

function config_disk_soft_raid() {
  # 等待 系统盘md2：k8s-vg 同步状态完成 实测2小时以内
  COUNTER=0
  while [ ${COUNTER} -lt 720 ]; do
    is_ready=$(su - root -c "cat /proc/mdstat  |grep -A 2 md2 |grep resync")
    if [[ ${is_ready} == '' ]]; then
      echo "Succeed to sync system disk"
      break
    fi
    let COUNTER++
    echo "waiting md resync"
    sleep 10
  done
  disk_info=$(lsblk -nl)
  for info in ${disk_info[*]}; do
    echo "${info}" | grep "T"
    if [[ "$?" -ne "0" ]]; then
      continue
    fi
    info_size=$(echo "${info}" | awk -F'T' '{print $1}')
    # 比较时候考虑小数 如 expr 10.1 \> 8 返回0而不是1
    if [ $(echo "${info_size}" 8 | awk '{if($1>$2){print 1;}else if($1<$2){print 0;}}') -eq 1 ]; then
      break
    fi
  done
  if [ $(echo "${info_size}" 8 | awk '{if($1>$2){print 1;}else if($1<$2){print 0;}}') -eq 1 ]; then
    # 软raid下返回2个盘
    disk_name=($(lsblk | grep "${info}" | grep "sd" | awk -F' ' '{print $1}'))
    # 数据盘重装前清理
    old_md=$(lsblk -nl /dev/"${disk_name[0]}" | grep md | awk -F ' ' '{print $1}')
    if [ ! -z "${old_md}" ]; then
      # 磁盘已有软raid， vgscan防止vg识别不到数据盘中已有vg
      vgscan --mknode
      vgremove -f ce-vg
      mdadm -S /dev/"${old_md}"
      mdadm --zero-superblock /dev/"${disk_name[0]}"
      mdadm --zero-superblock /dev/"${disk_name[1]}"
    fi
    # 软raid 数据盘组件md5
    mdadm --create --verbose --metadata=1.2 /dev/md5 --level=1 --raid-devices=2 /dev/"${disk_name[0]}" /dev/"${disk_name[1]}"
    pvcreate /dev/md5
    vgcreate ce-vg /dev/md5
    lvcreate -y -L 8T -i1 -I64 -n d-data-lv ce-vg
    mkfs.ext4 /dev/mapper/ce--vg-d--data--lv
    mkdir -p /opt/cyberengine/d-data
    uuid=$(blkid /dev/mapper/ce--vg-d--data--lv | awk -F "\"" '{print $2}')
    echo "UUID=${uuid} /opt/cyberengine/d-data ext4 defaults 0 0" >>/etc/fstab
    # 软raid 写入数据盘到mdadm.conf, 解决软raid分区显示乱序问题
    res=$(mdadm -E /dev/sdb | grep 'Array UUID' | awk -F " : " '{print $2}')
    config_cmd="ARRAY /dev/md/5 level=raid1 num-devices=2 UUID=${res}"
    echo ${config_cmd} >>/etc/mdadm.conf
  else
    echo "fail"
    exit 1
  fi
}

function check_soft_raid() {
  res=$(lsscsi | grep -i 'avago' -c)
  if [ "$res" != 0 ]; then
    return 0
  else
    return 1
  fi
}

function check_bond() {
  res=$(cat "${network_dir}"/ifcfg-eth0 | grep BONDING_MASTER | awk -F '=' '{print $2}')
  if [ "$res" == "yes" ]; then
    exit 1
  fi
}

function main() {
  # 检查是否已组过BOND
  check_bond
  # 配置数据盘分区
  check_soft_raid
  if [ $? -eq 1 ]; then
    echo "config disk with soft raid"
    config_disk_soft_raid
  else
    echo "config disk with hard raid"
    config_disk
  fi
  echo "start to bond manage port"
  manage_port1=$(get_network_name_by_bus_info "${manage_bus_info0}")
  manage_port2=$(get_network_name_by_bus_info "${manage_bus_info1}")
  if [ "${manage_port1}" -eq "1" ] || [ "${manage_port2}" -eq "1" ]; then
    echo "manage port not exist"
    exit 1
  fi
  config_network_bond ${manage_port1} ${manage_port2} ${manage_bond}
  echo "end to bond manage port"

  echo "start to bond service port"
  # 25GE网卡组bond
  service_port1=$(get_network_name_by_bus_info "${bus_info0}")
  service_port2=$(get_network_name_by_bus_info "${bus_info1}")
  if [ "${service_port1}" -eq "1" ] || [ "${service_port2}" -eq "1" ]; then
    echo "25GE service port not exist"
    exit 1
  fi
  config_network_bond ${service_port1} ${service_port2} ${service_bond}
  # 100GE网卡组bond
  GE_port1=$(get_network_name_by_bus_info "${GE_bus_info0}")
  GE_port2=$(get_network_name_by_bus_info "${GE_bus_info1}")
  if [ "${GE_port1}" -eq "1" ] || [ "${GE_port2}" -eq "1" ]; then
    echo "100GE service port not exist"
  else
    config_network_bond ${GE_port1} ${GE_port2} ${service_bond1}
  fi
  echo "end to bond service port"

  echo "start to config manage ip"
  config_network_ip ${manage_bond} ${self_manage_ip} ${self_manage_netmask}
  echo "end to config manage ip"

  echo "start to config container ip"
  container_port=$(get_network_name_by_bus_info "${manage_bus_info2}")
  config_container_network ${container_port} ${container_new_name} ${self_container_ip} ${self_container_netmask}
  echo "end to config container ip"

  echo "start to change protocol of all networks to static"
  find ${network_dir} -type f -name "ifcfg*" -exec sed -i "s/BOOTPROTO=dhcp/BOOTPROTO=static/g" {} +
  echo "end to  change protocol of all networks to static"

  rm -rf /etc/resolv.conf
}

main
