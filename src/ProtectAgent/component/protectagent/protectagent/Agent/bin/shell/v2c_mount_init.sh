#!/bin/bash
# 
#  This file is a part of the open-eBackup project.
#  This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
#  If a copy of the MPL was not distributed with this file, You can obtain one at
#  http://mozilla.org/MPL/2.0/.
# 
#  Copyright (c) [2024] Huawei Technologies Co.,Ltd.
# 
#  THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
#  EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
#  MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
#

MOUNT_POINT="/mnt_target"
SYS_DISK=""
MOUNT_SECTION=""

mount_once()
{
    Log "mount once"
    #check lvm or not
    check_lvm
    if [ $? -eq 0 ]
    then
        # lvm mount
        mount_lvm
    else
        # direct mount
        mount_direct
    fi
}

mount_twice()
{
    Log "mount twice"
    #check lvm or not
    check_lvm
    if [ $? -eq 0 ]
    then
        # lvm mount
        mount_lvm
        mount_seporate_boot
    else
        # direct mount
        mount_direct
        mount_seporate_boot
    fi
}

check_lvm()
{
    Log "check if lvm or not ${MOUNT_SECTION}"
    
    fdisk -l 2> /dev/null | grep "${MOUNT_SECTION}" | grep -i "LVM"
    IS_LVM=$?
    return ${IS_LVM}
}

mount_lvm()
{
    Log "mount lvm"
    VG_NAME=$(lvm pvscan | grep "${SYS_DISK}" | awk '{print $4}')
    Log "VG_NAME ${VG_NAME}" 
    lvm vgchange -ay ${VG_NAME}
    LV_ROOT=$(lvdisplay "/dev/"${VG_NAME} | grep "LV Name" | grep -i "root" | awk '{print $3}')
    Log "LV_ROOT ${LV_ROOT}"

    mkdir -p ${MOUNT_POINT}
    mount "/dev/${VG_NAME}/${LV_ROOT}" ${MOUNT_POINT}
}

mount_direct()
{
    Log "mount direct"
    mkdir -p ${MOUNT_POINT}
    mount ${MOUNT_SECTION} ${MOUNT_POINT}
}

mount_seporate_boot()
{
    Log "mount seporate boot ${BOOT_SECTION}"
    mount ${BOOT_SECTION} ${MOUNT_POINT}"/boot"
}

#begin
Log "begin mount init"

BOOT_SECTION_LIST=`fdisk -l 2>/dev/null | grep "^\/dev.*\*" | awk '{print $1}'`

mount_time=0
mount_disk=""

for attached in ${BOOT_SECTION_LIST[@]}
do
    attach_disk=$(echo ${attached} | awk -F "/" '{print $3}' | sed 's/.$//')

    temp_time=$(dmesg | grep "${attach_disk}:.*${attach_disk}" | awk -F "[" '{print $2}' | awk -F "]" '{print $1}')
    Log "temp_time ${temp_time}"

    if { echo "${temp_time}" ; echo "${mount_time}" ; } | sort -n -c 2>/dev/null
    then
        Log "${temp_time} is smaller than ${mount_time}"
    else
        Log "${temp_time} is bigger than ${mount_time}"
        mount_time=$temp_time
        mount_disk=$attach_disk
    fi

done

SYS_DISK=$mount_disk
Log "SYS_DISK to be mounted ${SYS_DISK}"

SECTION_COUNT=$(fdisk -l 2>/dev/null | grep "${SYS_DISK}" | grep -v "Disk" | wc -l)
if [ "${SECTION_COUNT}" = "1" ]
then
    BOOT_SECTION=$(fdisk -l 2>/dev/null | grep "${SYS_DISK}.*\*" | awk '{print $1}')
    MOUNT_SECTION=${BOOT_SECTION}
    mount_once
elif [ "${SECTION_COUNT}" = "2" ]
then
    BOOT_SECTION=$(fdisk -l 2>/dev/null | grep "${SYS_DISK}.*\*" | awk '{print $1}')
    MOUNT_SECTION=$(fdisk -l 2>/dev/null | grep "${SYS_DISK}" | grep -v "Disk" | grep -v "${BOOT_SECTION}" | awk '{print $1}')
    mount_twice
else
    Log "boot section not found"
    exit 1
fi

Log "in mount BOOT_SECTION ${BOOT_SECTION}"
BOOT_UUID=$( blkid -s UUID | grep "${BOOT_SECTION}" | awk -F'UUID=' '{print $2}' | tr -d '"' )
Log "in mount BOOT_UUID ${BOOT_UUID}"


