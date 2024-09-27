#!/bin/sh
set +x

AGENT_ROOT_PATH=$1
LOG_FILE_NAME=${AGENT_ROOT_PATH}/log/uninstall_iomirror_2_initrd.log
. "${AGENT_ROOT_PATH}/sbin/agent_sbin_func.sh"

SYS_VERSION=$(getOSVersion)
MODULE_INSTALL_DIR="/lib/modules/`uname -r`/kernel/drivers/char/"

uninstall_redhat6_5()
{
    if [ ! -f "/etc/rc.d/rc.sysinit" ]
    then
        Log "rc.sysinit not exist, uninstall failed"
        return 1
    fi
    
    Log "restore /etc/rc.d/rc.sysinit"
    cp -p /etc/rc.d/rc.sysinit /etc/rc.d/rc.sysinit.tmp
    sed -i "/modprobe iomirror/d" /etc/rc.d/rc.sysinit.tmp
    mv -f /etc/rc.d/rc.sysinit.tmp /etc/rc.d/rc.sysinit

    if [ ! -d "${MODULE_INSTALL_DIR}" ]
    then
        Log "${MODULE_INSTALL_DIR} not exist, iomirror file no more exist"
    else
        Log "restore iomirror file"
        DeleteFile ${MODULE_INSTALL_DIR}iomirror.ko 
    fi
    depmod -a

    Log "restore /etc/dracut.conf"
    cp -p /etc/dracut.conf /etc/dracut.conf.tmp
    sed -i '/add_drivers+=" iomirror "/d' /etc/dracut.conf.tmp
    mv -f /etc/dracut.conf.tmp /etc/dracut.conf

    Log "backup initramfs"
    KERNEL_VERSION=`uname -r`
    if [ -f "/boot/bak-initramfs-${KERNEL_VERSION}-noiomirror.img" ]
    then
        mv -f /boot/bak-initramfs-${KERNEL_VERSION}-noiomirror.img /boot/initramfs-${KERNEL_VERSION}.img
    else
        Log "[Warning] There are no bak-initramfs-${KERNEL_VERSION}-noiomirror.img file."
    fi

    Log "Complete delete redhat6.5 initrd."
    return 0
}

uninstall_centos_7_2()
{
    Log "delete iomirror module"
    IOMIRROR_INIT="/etc/sysconfig/modules/iomirror.modules"
    DeleteFile ${IOMIRROR_INIT}
    
    if [ ! -d "${MODULE_INSTALL_DIR}" ]
    then
        Log "${MODULE_INSTALL_DIR} not exist, iomirror file no more exist"
    else
        Log "delete iomirror file"
        DeleteFile ${MODULE_INSTALL_DIR}iomirror.ko 
    fi
    depmod -a

    Log "restore /etc/dracut.conf"
    cp -p /etc/dracut.conf /etc/dracut.conf.tmp
    sed -i '/add_drivers+=" iomirror "/d' /etc/dracut.conf.tmp
    mv -f /etc/dracut.conf.tmp /etc/dracut.conf
    
    Log "restore initramfs"
    KERNEL_VERSION=`uname -r`
    if [ -f "/boot/bak-initramfs-${KERNEL_VERSION}-noiomirror.img" ]
    then
        mv -f /boot/bak-initramfs-${KERNEL_VERSION}-noiomirror.img /boot/initramfs-${KERNEL_VERSION}.img
    else
        Log "[Warning] There are no bak-initramfs-${KERNEL_VERSION}-noiomirror.img file."
    fi
    
    Log "Complete delete centos7.2 initrd."
    return 0
}

main()
{
    if [ "${SYS_VERSION}" = "REDHAT_6.5" ]
    then
        Log "uninstall_redhat6_5 initrd"
        uninstall_redhat6_5
    elif [ "${SYS_VERSION}" = "CENTOS_7.2.1511" ]
    then
        Log "uninstall_centos_7_2 initrd"
        uninstall_centos_7_2
    else
        Log "Unsupported OS type ${SYS_VERSION}."
        exit 1
    fi
    
    return $?
}

Log "Begin to uninitial initrd for iomirror."
main $*

if [ $? -eq 0 ]
then
    Log "Complete uninitialing initrd for iomirror."
    exit 0
else
    Log "Initial uninitrd for iomirror failed."
    exit 1
fi
