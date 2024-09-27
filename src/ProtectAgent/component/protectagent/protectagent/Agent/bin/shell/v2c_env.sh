#!/bin/bash
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

export rootPath=''
export SYS_TYPE=''
export SYS_VERSION=''
export XEN_DRIVER='yes'
export IDE_DRIVER='yes'
export VIRTIO_DRIVER='yes'
export XEN_DISK='xvd'
export SCSI_DISK='sd'

export MOUNT_POINT=""
export SYS_DISK=""
export MOUNT_SECTION=""
export BOOT_SECTION_LIST=""
export BOOT_SECTION=""
export BOOT_UUID=""
export FSTAB_BOOT_DISK=""
export IS_LVM=1

export ERROR_SCRIPT_LINUX_V2C_MOUNT_FAILED=200
export ERROR_SCRIPT_LINUX_V2C_CHECK_FSTAB_FAILED=201
export ERROR_SCRIPT_LINUX_V2C_VERSION_UNSUPPORTED=202
export ERROR_SCRIPT_LINUX_V2C_GRUBFILE_NOT_FOUND=203
export ERROR_SCRIPT_LINUX_V2C_INITRD_NOT_FOUND=204
export ERROR_SCRIPT_LINUX_V2C_MODIFY_GRUB_FAILED=205
export ERROR_SCRIPT_LINUX_V2C_DELETE_OLD_DRIVER_FAILED=206
export ERROR_SCRIPT_LINUX_V2C_DELETE_VMWARE_TOOL_FAILED=207
export ERROR_SCRIPT_LINUX_V2C_ADD_KERNEL_PARAM_FAILED=208
export ERROR_SCRIPT_LINUX_V2C_MODIFY_MKINITRD_CONF_FAILED=209
export ERROR_SCRIPT_LINUX_V2C_REMAKE_INITRD_FAILED=210


### mount -o bind & umount
chroot_enter()
{
    if [ -z "${MOUNT_POINT}" ]
    then
        Log "mount bind failed"
        return 1	
    fi
    Log "MOUNT_POINT ${MOUNT_POINT}"

    Log "mount /dev /sys /proc"
    /usr/bin/mount -o bind /dev ${MOUNT_POINT}/dev
    /usr/bin/mount -o bind /sys ${MOUNT_POINT}/sys
    /usr/bin/mount -o bind /proc ${MOUNT_POINT}/proc

    return 0
}
chroot_exit()
{
    if [ -z "${MOUNT_POINT}" ]
    then
        Log "umount failed"
        return 1
    fi
    Log "MOUNT_POINT ${MOUNT_POINT}"

    Log "umount /dev /sys /proc"
    /usr/bin/umount ${MOUNT_POINT}/dev
    /usr/bin/umount ${MOUNT_POINT}/sys
    /usr/bin/umount ${MOUNT_POINT}/proc

    return 0
}

### backup file
MODIFIED_FILE_LIST=""

backupFile()
{
    file=$1
    backupFile="${file}.rbak"
    Log "backup file: ${file}"
    MODIFIED_FILE_LIST="${MODIFIED_FILE_LIST} ${backupFile}"
    
    if [ ! -e "${backupFile}" ]; then
        if ! cp ${file} ${backupFile}; then
            return 1
        fi
    fi

    return 0
}

backup_initrd_file()
{
    Log "backup all initrd file..."
    for file in ${INITRD_FILE_LIST[@]}
    do
        backupFile "${rootPath}/boot/${file}"
    done

    return 0
}

restore_initrd_file()
{
    Log "restore all initrd file..."
    for file in ${INITRD_FILE_LIST[@]}
    do
        backupFile="${file}.rbak"
        if [ -e "${backupFile}" ]; then
            cp ${backupFile} ${file}
        fi
    done

    return 0
}

add_kernel_param_suse()
{
    for file in ${grub_file}
    do
        if [ "${SYS_TYPE}" = "suse" ]; then
            backupFile ${file}
            Log "add xen_platform_pci.dev_unplug=all in ${file}"
            sed -i "s/xen_platform_pci.dev_unplug=\S*//g" ${file}
            sed -i "s/xen_emul_unplug=never//g" ${file}
            if [ "${SYS_TYPE}${SYS_VERSION}" = "suse11" ]; then
                sed -i "s/\(^[[:space:]]*[\$]\?\(kernel\|linux\).*\)/\1 xen_platform_pci.dev_unplug=all/g" ${file}
            fi
            if [ "${XEN_DRIVER}" = 'no' ]; then
              sed -i "s/\(^[[:space:]]*[\$]\?\(kernel\|linux\).*\)/\1 xen_emul_unplug=never/g" ${file}
            fi
        fi
    done

    return 0
}

###support os list:rhel6.x rhel7.x
add_kernel_param_rhel()
{
    #grub file
    for file in ${grub_file}
    do
        if [ "${SYS_TYPE}${SYS_VERSION}" = "redhat6" ] || [ "${SYS_TYPE}${SYS_VERSION}" = "redhat7" ]; then
            Log "remove kernel params: rhgb quiet console=ttyS*"
            sed -i "s/\(^[[:space:]]*[\$]\?\(kernel\|linux\).*\) rhgb\(.*\)/\1\3/g" ${file}
            sed -i "s/\(^[[:space:]]*[\$]\?\(kernel\|linux\).*\) quiet\(.*\)/\1\3/g" ${file}
            ###grub1
            if [ -n "`echo ${file} | grep -E \"grub.conf|menu.lst\"`" ]; then
                add_kernel_param_for_grub1 ${file}
            ###grub2
            elif [ -n "`echo ${grub_file} | grep 'grub.cfg'`" ]; then
                add_kernel_param_for_grub2 ${file}
            fi
        fi
    done
    
    #syslinux file
    for file in ${syslinux_file}
    do
        if [ "${SYS_TYPE}${SYS_VERSION}" = "redhat6" ] || [ "${SYS_TYPE}${SYS_VERSION}" = "redhat7" ]; then
            Log "remove kernel params: rhgb quiet console=ttyS*"
            sed -i "s/\(^[[:space:]]*[\$]\?\(kernel\|linux\).*\)rhgb\(.*\)/\1 \3/g" ${file}
            sed -i "s/\(^[[:space:]]*[\$]\?\(kernel\|linux\).*\)quiet\(.*\)/\1 \3/g" ${file}
            sed -i "s/console=ttyS0//g" ${file}
            Log "add kernel param: console=ttyS0"
            sed -i "s/\(^[[:space:]]*[\$]\?\(kernel\|linux\).*\)/\1 console=ttyS0/g" ${file}
        fi
    done
    
    return 0
}

### for /boot/grub/grub.conf /boot/grub/menu.lst
add_kernel_param_for_grub1()
{
    file=$1
    Log "add console=ttyS0 for grub1"

    boot_num=`cat ${file} | grep "^[[:space:]]*default=" | awk -F '=' '{print $2}' | tr -d ' '`
    Log "boot_num=$boot_num"

    kernel_line=`cat ${file} | grep -n "^[[:space:]]*[\$]\?\(kernel\|linux\)" | sed -n $((${boot_num}+1))p | awk -F ':' '{print $1}'`
    Log "kernel_line=${kernel_line}"

    Log "add kernel param: console=ttyS0"
    if [ -z "`sed -n ${kernel_line}p ${file} | grep "console=ttyS0"`" ]; then
        sed -i "${kernel_line}s/\(^[[:space:]]*[\$]\?\(kernel\|linux\).*\)/\1 console=ttyS0/" ${file}
    fi

    return 0
}

### for /boot/grub/grub.cfg /boot/grub2/grub.cfg
add_kernel_param_for_grub2()
{
    file=$1
    default_grub="${rootPath}/etc/default/grub"
    grubenv="${rootPath}/boot/grub2/grubenv"
    Log "add console=ttyS0 for grub2"

    default_type=`cat ${default_grub} | grep "GRUB_DEFAULT" | awk -F '=' '{print $2}' | tr -d ' '`
    search_rescue_title="FALSE"
    Log "default_type = ${default_type}"

    if [ "$default_type" -ge 0 ] 2> /dev/null; then
        Log "grub2 entry with default number"
        boot_num=`echo ${default_type} | tr -d ' '`
        kernel_line=`cat ${file} | grep -n "^[[:space:]]*[\$]\?\(kernel\|linux\)" | sed -n $((${boot_num}+1))p | awk -F ':' '{print $1}'`
    if [ -z "`sed -n ${kernel_line}p ${file} | grep "console=ttyS0"`" ]; then
        sed -i "${kernel_line}s/\(^[[:space:]]*[\$]\?\(kernel\|linux\).*\)/\1 console=ttyS0/" ${file}
    fi

    elif [ "$default_type" = "saved" ]; then
        Log "grub2 entry with saved entry"
        title=`cat ${grubenv} | grep "saved_entry" | awk -F '=' '{print $2}'`
        menuentry_num=`cat ${file} | grep -n "${title}" | grep "^[0-9]*:[[:space:]]*menuentry" | awk -F ':' '{print $1}'`
        if [ -n "${menuentry_num}" ];then
            kernel_offset_line=`cat ${file} | awk "NR>${menuentry_num}" | grep -n "^[[:space:]]*[\$]\?\(kernel\|linux\)" | awk -F ':' '{print $1}' | head -n 1`
            kernel_line=$((${menuentry_num}+${kernel_offset_line}))
            if [ -z "`sed -n ${kernel_line}p ${file} | grep "console=ttyS0"`" ]; then
                sed -i "${kernel_line}s/\(^[[:space:]]*[\$]\?\(kernel\|linux\).*\)/\1 console=ttyS0/" ${file}
            fi
        else
               search_rescue_title="TRUE"
        fi

    else
        Log "grub2 entry with default title"
        title=${default_type}
        menuentry_num=`cat ${file} | grep -n "${title}" | grep "^[0-9]*:[[:space:]]*menuentry" | awk -F ':' '{print $1}'`
        if [ -n "${menuentry_num}" ];then
            kernel_offset_line=`cat ${file} | awk "NR>${menuentry_num}" | grep -n "^[[:space:]]*[\$]\?\(kernel\|linux\)" | awk -F ':' '{print $1}' | head -n 1`
            kernel_line=$((${menuentry_num}+${kernel_offset_line}))
            if [ -z "`sed -n ${kernel_line}p ${file} | grep "console=ttyS0"`" ]; then
                sed -i "${kernel_line}s/\(^[[:space:]]*[\$]\?\(kernel\|linux\).*\)/\1 console=ttyS0/" ${file}
            fi
        else
            search_rescue_title="TRUE"
        fi
    fi

    ### search the title that inclue 'rescue' word
    if [ "${search_rescue_title}" = "TRUE" ]; then
        Log "search the rescue mode"
        menuentry_num=`cat ${file} | grep -n "^[[:space:]]*menuentry" | grep -iE "rescue|recovery mode" | awk -F ':' '{print $1}'`
        if [ -n "${menuentry_num}" ];then
            kernel_offset_line=`cat ${file} | awk "NR>${menuentry_num}" | grep -n "^[[:space:]]*[\$]\?\(kernel\|linux\)" | awk -F ':' '{print $1}' | head -n 1`
            kernel_line=$((${menuentry_num}+${kernel_offset_line}))
            #delete all console=ttyS0
            sed -i "s/console=ttyS0//g" ${file}
            #add console=ttyS0 for every kernel line
            sed -i "s/\(^[[:space:]]*[\$]\?\(kernel\|linux\).*\)/\1 console=ttyS0/g" ${file}
            #delete console=ttyS0 for rescue mode
            sed -i "${kernel_line}s/\(^[[:space:]]*[\$]\?\(kernel\|linux\).*\) console=tty\S*\(.*\)/\1\3/g" ${file}
        else
            Log "can not find rescue kernel line"
        fi
    fi

    return 0
}

get_linux_type_version()
{
    Log "get linux system type and version"
    lsb_release="${rootPath}/etc/lsb-release"
    if [ -e "${lsb_release}" ] && [ -n "$(grep -i 'ubuntu' ${lsb_release})" ]
    then
        SYS_TYPE='ubuntu'
        if [ -n "$(grep '14\.[0-9]' ${lsb_release})" ]
        then
            SYS_VERSION=14
        fi
        if [ -n "$(grep '16\.[0-9]' ${lsb_release})" ]
        then
            SYS_VERSION=16
        fi
        if [ -n "$(grep '18\.[0-9]' ${lsb_release})" ]
        then
            SYS_VERSION=18
        fi

    elif [ -e "${rootPath}/etc/debian_version" ]
    then
        SYS_TYPE='debian'
        debian_version="${rootPath}/etc/debian_version"
        if [ -n "$(grep '8\.[0-9]' ${debian_version})" ]
        then
            SYS_VERSION=8
        elif [ -n "$(grep '9\.[0-9]' ${debian_version})" ]
        then
            SYS_VERSION=9
        fi
    
    elif [ -e "${rootPath}/etc/fedora-release" ]
    then
        SYS_TYPE='fedora'
        fedora_version="${rootPath}/etc/fedora-release"
        if [ -n "$(grep '22' ${fedora_version})" ]
        then
            SYS_VERSION=22
        elif [ -n "$(grep '23' ${fedora_version})" ]
        then
            SYS_VERSION=23
        elif [ -n "$(grep '24' ${fedora_version})" ]
        then
            SYS_VERSION=24
        elif [ -n "$(grep '25' ${fedora_version})" ]
        then
            SYS_VERSION=25
        elif [ -n "$(grep '26' ${fedora_version})" ]
        then
            SYS_VERSION=26
        elif [ -n "$(grep '28' ${fedora_version})" ]
        then
            SYS_VERSION=28
        elif [ -n "$(grep '29' ${fedora_version})" ]
        then
            SYS_VERSION=29
        fi

    elif [ -e "${rootPath}/etc/centos-release" ]
    then
        SYS_TYPE='centos'
        centos_version="${rootPath}/etc/centos-release"
        if [ -n "$(grep ' 6\.[0-9]' ${centos_version})" ]
        then
            SYS_VERSION=6
        elif [ -n "$(grep ' 7\.[0-9]' ${centos_version})" -o -n "$(grep 'EulerOS release 2.0' ${centos_version})" ]
        then
            SYS_VERSION=7
        fi

    elif [ -e "${rootPath}/etc/oracle-release" ]
    then
        SYS_TYPE='centos'
        oracle_version="${rootPath}/etc/oracle-release"
        if [ -n "$(grep ' 6\.[0-9]' ${oracle_version})" ]
        then
            SYS_VERSION=6
        elif [ -n "$(grep ' 7\.[0-9]' ${oracle_version})" -o -n "$(grep 'EulerOS release 2.0' ${oracle_version})" ]
        then
            SYS_VERSION=7
        fi
    
    elif [ -e "${rootPath}/etc/redhat-release" ]
    then
        SYS_TYPE='redhat'
        redhat_version="${rootPath}/etc/redhat-release"
        if [ -n "$(grep ' 6\.[0-9]' ${redhat_version})" ]
        then
            SYS_VERSION=6
        elif [ -n "$(grep ' 7\.[0-9]' ${redhat_version})" -o -n "$(grep 'EulerOS release 2.0' ${redhat_version})" ]
        then
            SYS_VERSION=7
        fi

    elif [ -e "${rootPath}/etc/SuSE-release" ] || [ -n "$(grep -i 'suse' ${rootPath}/etc/os-release)" ]
    then
        SYS_TYPE='suse'
        suse_version="${rootPath}/etc/SuSE-release"
        suse_version_15="${rootPath}/etc/os-release"
        if [ -n "$(grep '11' ${suse_version})" ]
        then
            SYS_VERSION=11
        elif [ -n "$(grep '12' ${suse_version})" ]
        then
            SYS_VERSION=12
        elif [ -n "$(grep '13' ${suse_version})" ]
        then
            SYS_VERSION=13
        elif [ -n "$(grep '15' ${suse_version_15})" ]
        then
            SYS_VERSION=15
        elif [ -n "$(grep '42' ${suse_version})" ]
        then
            SYS_VERSION=42
        fi

    else
        Log "cannot determine linux distribution"
        return 1
    fi

    if [ "${SYS_TYPE}" = "centos" ] || [ "${SYS_TYPE}" = "redhat" ]
    then
        Log "${SYS_TYPE} is supported"
        return 0
    else
        Log "${SYS_TYPE} is not supported"
        return 1
    fi
}
