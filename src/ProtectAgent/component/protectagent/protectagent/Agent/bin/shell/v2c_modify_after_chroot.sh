
SYS_TYPE=$1
SYS_VERSION=$2
INITRD_FILE_LIST=$3
export PATH=/bin:/sbin:/usr/bin:/usr/sbin:$PATH
mkinitrd_path=$(which mkinitrd)

cd /boot

if [ "${SYS_TYPE}" = "suse" ]
then
    echo "mkinitrd_path is " ${mkinitrd_path}
    echo "SYS_TYPE " ${SYS_TYPE}
    echo "SYS_VERSION " ${SYS_VERSION}

    echo "INITRD_FILE_LIST is " ${INITRD_FILE_LIST}
    ${mkinitrd_path}
else
    echo "mkinitrd_path is " ${mkinitrd_path}
    echo "SYS_TYPE " ${SYS_TYPE}
    echo "SYS_VERSION " ${SYS_VERSION}

    echo "INITRD_FILE_LIST is " ${INITRD_FILE_LIST}
    for file in ${INITRD_FILE_LIST[@]}
    do
        KERNEL_VERSION=$(echo ${file} | awk -F '-' '{OFS="-";$1=null;print $0}' | awk -F ".img" '{print $1}' | sed 's/-//')
        echo "mkinitrd -f ${file} ${KERNEL_VERSION}"
        mkinitrd -f ${file} ${KERNEL_VERSION}
    done
fi
exit


