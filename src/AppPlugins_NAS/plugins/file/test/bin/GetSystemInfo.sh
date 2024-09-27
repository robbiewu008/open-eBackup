#!/bin/bash
#
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
#

sysInfoPath=$1
if [ -z ${sysInfoPath} ]; then
    echo "Need set system info path."
    exit 1;
fi

sysInfoPath=`realpath ${sysInfoPath}`
echo "System related information will be saved in ${sysInfoPath}"

parted -l -s > ${sysInfoPath}/partitions_parted.txt
if [ $? == 0 ]; then
	echo "Execute the parted -l -s command successfully, save to partitions_parted.txt"
else
	echo "Failed to execute parted -l command"
    exit 1;
fi

fdisk -l > ${sysInfoPath}/partitions_fdisk.txt
if [ $? == 0 ]; then
	echo "Execute the fdisk -l command successfully, save to partitions_fdisk.txt"
else
	echo "Failed to execute fdisk -l command"
    exit 1;
fi

lsblk -p -P -o NAME,TYPE,FSTYPE,MOUNTPOINT,SIZE | egrep 'TYPE="disk" | TYPE="part"' > ${sysInfoPath}/blk_disk.txt
if [ $? == 0 ]; then
	echo "Execute the lsblk command successfully, saved to blk_disk.txt"
else
	echo "Failed to execute lsblk -p -P -o NAME,TYPE,FSTYPE,MOUNTPOINT,SIZE command"
    exit 1;
fi

while read line; do
    devname=$(echo $line | awk '$2 ~ /disk/ {print $1}' | cut -d '"' -f 2)
    if [ -n "$devname" ]; then
        partition_file="partition_"`echo ${devname} | sed 's/\//_/g'`".txt"
        sfdisk -d ${devname} > ${sysInfoPath}/${partition_file}
        echo "Save ${devname} partition to file ${partition_file}"
    fi
done < ${sysInfoPath}/blk_disk.txt

# fstab 可能不存在初始挂载的文件系统
if [ -e "/etc/fstab" ]; then
    cat /etc/fstab > ${sysInfoPath}/fstab.txt
    if [ $? == 0 ]; then
        echo "Save /etc/fstab to file fstab.txt"
    else
        echo "Failed to execute command: cat /etc/fstab"
        exit 1;
    fi
fi

cat /proc/mounts > ${sysInfoPath}/mounts.txt
if [ $? == 0 ]; then
	echo "Save mounted information to file mounted_info.txt"
else
	echo "Failed to obtain the mounted information"
    exit 1;
fi

# LVM卷备份
mkdir -p ${sysInfoPath}/lvm_info
vgs --noheadings | awk '{print $1}' | xargs -i vgcfgbackup -f ${sysInfoPath}/lvm_info/{} {}
if [ $? == 0 ]; then
	echo "The LVM information has been saved to a file named lvm_info.txt"
else
	echo "Failed to obtain the LVM information"
    exit 1;
fi

# 获取sysPathList所在的系统卷路径，这些可能在part卷，也可能在LVM卷
sysPathList=("/" "/usr" "/usr/local" "/var" "/boot" "/etc")
if [ -e "/opt" ]; then # /opt 不一定存在
    sysPathList=("${sysPathList[@]}" "/opt")
fi
df -hT ${sysPathList[*]} | sed -n '2,$p' | awk '{print $1}' | sort -u > ${sysInfoPath}/sysvol.txt
if [ $? == 0 ]; then
	echo "Get disk info successfully, saved to sysvol.txt"
else
	echo "Failed to execute df -hT command"
    exit 1;
fi

# 获取ip&route信息
ip -o addr show | awk '{printf "name=%s %s=%s brd=%s\n", $2,$3,$4,$6}' | egrep -v "lo" > ${sysInfoPath}/network.txt
ip route show > ${sysInfoPath}/route.txt
echo "Save ip and route info to network.txt & route.txt"
