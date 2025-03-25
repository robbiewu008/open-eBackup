#!/bin/sh
set +x

AGENT_ROOT_PATH=$1
PID=$2
PARAM_NUM=$3
. "${AGENT_ROOT_PATH}/sbin/agent_sbin_func.sh"

#for log
LOG_FILE_NAME="${LOG_PATH}/vmfs_umount.log"
TMP_DIR="/mnt"

BLOCK_LIST="^/$\|^/tmp$\|^/mnt/databackup$\|^/bin$\|^/bin/.*\|^/boot$\|^/boot/.*\|^/dev$\|^/dev/.*\|^/etc$\|^/etc/.*\|^/lib$\|^/lib/.*\|^/lost+found$\|^/lost+found/.*\|^/media$\|^/media/.*\|^/mnt$\|^/opt$\|^/proc$\|^/proc/.*\|^/root$\|^/root/.*\|^/sbin$\|^/sbin/.*\|^/selinux$\|^/selinux/.*\|^/srv$\|^/srv/.*\|^/sys$\|^/sys/.*\|^/usr$\|^/usr/.*\|^/var$\|^/var/.*\|^/run$\|^/run/.*"

readonly OPERATION_MAX_RETRY_COUNT=3
readonly OPERATION_RETRY_INTERVAL=10

CheckMntPoint()
{
    mnt_path=$1
    mountpoint ${mnt_path}
    if [ $? -eq 0 ]; then
        Log "mnt path is a mount point!"
        return 0
    fi
    return 1;
}

CheckPath() 
{
    filepat='[|;&$><`\!.]+'
    if [[ "$1" =~ $filepat ]]; then
        return 1
    fi

    if [[ "$1" =~ ^/mnt ]]; then
        return 0
    fi
    return 1
}

Log "********************************Start to execute the vmfs umount script********************************"
# global var, for kill monitor
PARAM_CONTENT=`ReadInputParam`
test -z "$PARAM_CONTENT"              && ExitWithError "PARAM_FILE"

MNT_PATH=$(GetValue "${PARAM_CONTENT}" mnt_path)
#REAL_MNT_PATH=$(realpath ${MNT_PATH})
REAL_MNT_PATH=${MNT_PATH}

ret=$(echo $(cd ${REAL_MNT_PATH}) | grep -c "No such file or directory")
if [ ${ret} -eq 1 ]; then
    Log "The mount path does not exist, path is ${REAL_MNT_PATH}."
    exit 1
fi
CheckPath ${REAL_MNT_PATH}
if [ $? -ne 0 ]; then
        Log "The mount path is against the rule, path is ${realMountPath}."
        exit 1
fi
CheckMntPoint ${REAL_MNT_PATH}
if [ $? -ne 0 ]; then
    Log "${REAL_MNT_PATH} is not a mount point"
    exit 0
else
    Log "${REAL_MNT_PATH} is a mount point, start to umount it."
fi
exec_cmd_str="umount ${REAL_MNT_PATH}"
$(${exec_cmd_str})
if [ $? -ne 0 ]; then
    retry_time=0
    while [ ${retry_time} -lt ${OPERATION_MAX_RETRY_COUNT} ]; do
        Log "exec cmd ${exec_cmd_str} failed , retry in ${OPERATION_RETRY_INTERVAL} seconds"
        retry_time=`expr $retry_time + 1`
        sleep ${OPERATION_RETRY_INTERVAL}
        $(${exec_cmd_str})
        ret=$?
        if [ ${ret} -eq 0 ]; then
            break
        fi
    done
fi
if [ ${ret} -ne 0 ]; then
    Log "failed to execute umount, exit !"
    rm -rf ${REAL_MNT_PATH}
    exit 1
fi
CheckMntPoint ${REAL_MNT_PATH}
if [ $? -ne 0 ]; then
    Log "${REAL_MNT_PATH} is nolonger a mount point"
    rm -rf ${REAL_MNT_PATH}
    exit 0
else
    Log "${REAL_MNT_PATH} is still a mount point, execution failed!"
    rm -rf ${REAL_MNT_PATH}
    exit 1
fi