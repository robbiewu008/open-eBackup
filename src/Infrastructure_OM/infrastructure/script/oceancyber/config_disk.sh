#!/usr/bin/bash

function config_disk() {
  disk_info=$(lsblk -nl)
  for info in ${disk_info[*]}; do
    echo ${info} | grep "T"
    if [[ "$?" -ne "0" ]]; then
      continue
    fi
    info_size=$(echo ${info} | awk -F'T' '{print $1}')
    if [[ $(expr ${info_size} \> 8) -eq 1 ]]; then
      disk_zise=${info}
      break
    fi
  done
  if [[ $(expr ${info_size} \> 8) -eq 1 ]]; then
    disk_name=$(lsblk | grep "${info}" | awk -F' ' '{print $1}')
    disk_name=$(echo ${disk_name} | awk -F' ' '{print $1}')
    pvcreate /dev/${disk_name}
    vgcreate ce-vg /dev/${disk_name}
    lvcreate -y -L 8T -i1 -I64 -n d-data-lv ce-vg
    mkfs.ext4 /dev/mapper/ce--vg-d--data--lv
    mkdir -p /opt/cyberengine/d-data
    uuid=$(blkid /dev/mapper/ce--vg-d--data--lv | awk -F "\"" '{print $2}')
    echo "UUID=${uuid} /opt/cyberengine/d-data ext4 defaults 0 0" >>/etc/fstab
    mount -a
  else
    echo "fail"
    exit 1
  fi
}

config_disk
