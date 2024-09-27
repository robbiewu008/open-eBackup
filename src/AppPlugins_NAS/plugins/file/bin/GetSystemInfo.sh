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
SCRIPT_NAME="${0##*/}"
SCRIPT_PATH=$(cd $(dirname $0); pwd)
if [ -z "$DATA_BACKUP_AGENT_HOME" ]; then
    echo "The environment variable: DATA_BACKUP_AGENT_HOME is empty."
    DATA_BACKUP_AGENT_HOME=${SCRIPT_PATH%/DataBackup/ProtectClient*};
    export DATA_BACKUP_AGENT_HOME
    echo "Set DATA_BACKUP_AGENT_HOME: ${DATA_BACKUP_AGENT_HOME}"
fi
if [ ! -d $DATA_BACKUP_AGENT_HOME ];then
    echo "Agent home dir do not exist"
    exit 1
fi
LOG_FILE=${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient/ProtectClient-E/slog/Plugins/FilePlugin/file_plugin.log

function LOG()
{
    if [ ! -z "$LOG_FILE" ]; then
        local dirName=$(dirname "$LOG_FILE")
        mkdir -p $dirName

        if [ -f "$LOG_FILE" ]; then
            local logSize=$(stat -c "%s" "$LOG_FILE")
            if [ $logSize -gt 10485760 ]; then
                mv ${LOG_FILE} ${LOG_FILE}.old
                touch $LOG_FILE
            fi
        fi
    fi

    local log_level=$1
    local log_content=$2
    local log_time=$(date +"%Y-%m-%d %H:%M:%S")
    echo -e "[${log_time}][${log_level}][ ${log_content} ][$$][${SCRIPT_NAME}][$(whoami)]" | tee -a "$LOG_FILE"
    return 0
}

sysInfoPath=$1
if [ -z ${sysInfoPath} ]; then
    LOG "ERROR" "Need set system info path."
    exit 1;
fi

sysInfoPath=`readlink -f ${sysInfoPath}`
LOG "INFO" "System related information will be saved in ${sysInfoPath}"

parted -l -s > ${sysInfoPath}/partitions_parted
if [ $? == 0 ]; then
	LOG "INFO" "Execute the parted -l -s command successfully, save to file partitions_parted"
else
	LOG "ERROR" "Failed to execute parted -l command"
    exit 1;
fi

fdisk -l > ${sysInfoPath}/partitions_fdisk
if [ $? == 0 ]; then
	LOG "INFO" "Execute the fdisk -l command successfully, save to file partitions_fdisk"
else
	LOG "ERROR" "Failed to execute fdisk -l command"
    exit 1;
fi

if [[ $(uname -r | cut -d. -f1) == "2" && $(uname -r | cut -d. -f2) == "6" ]]; then
    blkid -o list | grep /dev/mapper/ | awk '{print $1,$2}' > ${sysInfoPath}/lv_fsType
else
    lsblk -p -l -o  NAME,TYPE,FSTYPE | awk '$2=="lvm" {print $1,$3}' > ${sysInfoPath}/lv_fsType
fi
if [ $? == 0 ]; then
	LOG "INFO" "Get fsTypes of lvs successfully, save to file lv_fsType"
else
	LOG "ERROR" "Failed to get fsTypes of lvs"
    exit 1;
fi


lsblk -l -b -o NAME,TYPE,SIZE | awk '$2=="disk"' | awk '{print "/dev/"$1,$3}' > ${sysInfoPath}/blk_disk
if [ $? == 0 ]; then
	LOG "INFO" "Execute the lsblk command successfully, saved to file blk_disk"
else
	LOG "ERROR" "Failed to execute lsblk -l -b -o NAME,TYPE,SIZ command"
    exit 1;
fi

# only support backup partition type in (gpt, dos)
while read line; do
    devname=$(echo $line | awk '{print $1}')
    if [ -n "$devname" ]; then
        disk_type=`fdisk -l ${devname} | grep "type:" | awk -F ':' '{print $2}' | awk '$1=$1'`
        partition_file="partition_"`echo ${devname} | sed 's/\//_/g'`
        if [ ${disk_type} == "dos" ]; then
            sfdisk -d ${devname} > ${sysInfoPath}/"dos_"${partition_file}
            LOG "INFO" "Save ${devname} partition to file dos_${partition_file}"
        elif [ ${disk_type} == "gpt" ]; then
            sgdisk --backup=${sysInfoPath}/"gpt_"${partition_file} ${devname}
            LOG "INFO" "Save ${devname} partition to file gpt_${partition_file}"
        else
            LOG "WARNING" "${devname} partition type is ${disk_type}, but only support (gpt, dos)"
        fi
    fi
done < ${sysInfoPath}/blk_disk

lsblk -b -o NAME,TYPE,FSTYPE,MOUNTPOINT,SIZE -l | awk '$2=="part"' | grep /boot | awk '$1="/dev/"$1' > ${sysInfoPath}/boot_part
if [ $? == 0 ]; then
	LOG "INFO" "Execute the lsblk command successfully, saved to file boot_part"
else
	LOG "ERROR" "Failed to execute lsblk -b -o NAME,TYPE,FSTYPE,MOUNTPOINT,SIZE -l command"
    exit 1;
fi

# fstab 可能不存在初始挂载的文件系统
if [ -e "/etc/fstab" ]; then
    cat /etc/fstab > ${sysInfoPath}/fstab
    if [ $? == 0 ]; then
        LOG "INFO" "Save /etc/fstab to file fstab"
    else
        LOG "ERROR" "Failed to execute command: cat /etc/fstab"
        exit 1;
    fi
fi

cat /proc/mounts > ${sysInfoPath}/mounts
if [ $? == 0 ]; then
	LOG "INFO" "Save mounted information to file mounted_info"
else
	LOG "ERROR" "Failed to obtain the mounted information"
    exit 1;
fi

# LVM卷备份
mkdir -p ${sysInfoPath}/lvm_info
vgs --noheadings | awk '{print $1}' | xargs -i vgcfgbackup -f ${sysInfoPath}/lvm_info/{} {}
if [ $? == 0 ]; then
	LOG "INFO" "The LVM information has been saved to a file named lvm_info"
else
	LOG "ERROR" "Failed to obtain the LVM information"
    exit 1;
fi

vgs --noheadings -o vg_name | awk '{print $1}' > ${sysInfoPath}/lvm_info/vglist
pvs --noheadings -o pv_name,vg_name,pv_uuid > ${sysInfoPath}/lvm_info/pvlist
LOG "INFO" "Save volume groups to file vglist and physical volumes to file pvlist"

# 获取sysPathList所在的系统卷路径，这些可能在part卷，也可能在LVM卷
sysPathList=("/" "/usr" "/usr/local" "/var" "/boot" "/etc")
if [ -e "/opt" ]; then # /opt 不一定存在
    sysPathList=("${sysPathList[@]}" "/opt")
fi
df -hT ${sysPathList[*]} | sed -n '2,$p' | awk '{print $1}' | sort -u > ${sysInfoPath}/sysvol
if [ $? == 0 ]; then
	LOG "INFO" "Get disk info successfully, saved to file sysvol"
else
	LOG "ERROR" "Failed to execute df -hT command"
    exit 1;
fi

# 获取ip&route信息
ip -o addr show | awk '{printf "name=%s %s=%s brd=%s\n", $2,$3,$4,$6}' | egrep -v "lo" > ${sysInfoPath}/network
ip route show > ${sysInfoPath}/route
LOG "INFO" "Save ip and route info to file network & route"

# CPU platform
uname -m > ${sysInfoPath}/cpu_arch