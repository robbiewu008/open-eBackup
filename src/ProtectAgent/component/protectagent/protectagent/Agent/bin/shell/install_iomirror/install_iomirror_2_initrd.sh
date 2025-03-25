#!/bin/sh
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
set +x
AGENT_ROOT_PATH=$1
LOG_FILE_NAME=${AGENT_ROOT_PATH}/log/install_iomirror_2_initrd.log
. "${AGENT_ROOT_PATH}/sbin/agent_sbin_func.sh"

getOSVersion >> /dev/null
MODULE_FILE_DIR=${AGENT_ROOT_PATH}/bin/driver/dr/${DRIVER_OS_VERSION}/
MODULE_INSTALL_DIR="/lib/modules/`uname -r`/kernel/drivers/char/"

install_redhat6_5()
{
    if [ ! -f "/etc/rc.d/rc.sysinit" ]
    then
        Log "rc.sysinit not exist, install to init failed"
        return 1
    fi
    
    SYSINIT_NUM=`cat /etc/rc.d/rc.sysinit | grep -n "set -m" | awk -F ":" '{print $1}'`
    grep "modprobe iomirror" /etc/rc.d/rc.sysinit >> ${LOG_FILE_NAME} 2>& 1
    if [ $? -ne 0 ]
    then
        sed -i "${SYSINIT_NUM}a modprobe iomirror" /etc/rc.d/rc.sysinit
    fi

    if [ ! -d "${MODULE_INSTALL_DIR}" ]
    then
        echo ${MODULE_INSTALL_DIR} " not exist, install module failed"
        return 1
    fi

    cp -f "${MODULE_FILE_DIR}"/iomirror.ko ${MODULE_INSTALL_DIR}
    depmod -a

    grep "add_drivers+=\" iomirror \"" /etc/dracut.conf >> ${LOG_FILE_NAME} 2>& 1
    if [ $? -ne 0 ]
    then
        sed -i '$a add_drivers+=" iomirror "' /etc/dracut.conf
    fi

    KERNEL_VERSION=`uname -r`
    ### backup initramfs ###
    if [ ! -f /boot/bak-initramfs-${KERNEL_VERSION}-noiomirror.img ]
    then
        cp /boot/initramfs-${KERNEL_VERSION}.img /boot/bak-initramfs-${KERNEL_VERSION}-noiomirror.img
    fi
    
    mkinitrd -f /boot/initramfs-${KERNEL_VERSION}.img ${KERNEL_VERSION} >> ${LOG_FILE_NAME} 2>& 1
    if [ $? -ne 0 ]
    then
        Log "mkinitrd failed"
        return 1
    fi
    
    Log "Complete making redhat6.5 initrd."
    return 0
}

install_centos_7_2()
{
    IOMIRROR_INIT="/etc/sysconfig/modules/iomirror.modules"
    if [ -f "${IOMIRROR_INIT}" ]
    then
        rm -f ${IOMIRROR_INIT}
    fi
    touch ${IOMIRROR_INIT}
    echo -e '#!/bin/bash' > ${IOMIRROR_INIT}
    sed -i '$a modprobe iomirror' ${IOMIRROR_INIT}
    CHMOD +x ${IOMIRROR_INIT}

    if [ ! -d "${MODULE_INSTALL_DIR}" ]
    then
        Log ${MODULE_INSTALL_DIR} " not exist, install module failed"
        return 1
    fi

    cp -f ${MODULE_FILE_DIR}/iomirror.ko ${MODULE_INSTALL_DIR}
    depmod -a

    grep "add_drivers+=\" iomirror \"" /etc/dracut.conf >> ${LOG_FILE_NAME} 2>& 1
    if [ $? -ne 0 ]
    then
        sed -i '$a add_drivers+=" iomirror "' /etc/dracut.conf
    fi
    
    KERNEL_VERSION=`uname -r`
    ### backup initramfs ###
    if [ ! -f /boot/bak-initramfs-${KERNEL_VERSION}-noiomirror.img ]
    then
        cp /boot/initramfs-${KERNEL_VERSION}.img /boot/bak-initramfs-${KERNEL_VERSION}-noiomirror.img
    fi
    
    mkinitrd -f /boot/initramfs-${KERNEL_VERSION}.img ${KERNEL_VERSION} >> ${LOG_FILE_NAME} 2>& 1
    if [ $? -ne 0 ]
    then
        Log "mkinitrd failed"
        return 1
    fi
    
    Log "Complete making centos7.2 initrd."
    return 0
}

main()
{
    SYS_VERSION=$(getOSVersion)
    if [ "${SYS_VERSION}" = "REDHAT_6.5" ]
    then
        Log "install_redhat6_5 initrd"
        install_redhat6_5
    elif [ "${SYS_VERSION}" = "CENTOS_7.2.1511" ]
    then
        Log "install_centos_7_2 initrd"
        install_centos_7_2
    else
        Log "Unsupported OS ${SYS_VERSION}."
        return 1
    fi
    
    return $?
}

Log "Begin to initial initrd for iomirror."
main $*

if [ $? -eq 0 ]
then
    Log "Complete initialing initrd for iomirror."
    exit 0
else
    Log "Initial initrd for iomirror failed."
    exit 1
fi
