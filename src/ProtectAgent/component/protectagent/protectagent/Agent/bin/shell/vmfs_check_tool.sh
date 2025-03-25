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

Log "********************************Start to execute the vmfs_check_tool script********************************"
# global var, for kill monitor
PARAM_CONTENT=`ReadInputParam`
test -z "$PARAM_CONTENT"              && ExitWithError "PARAM_FILE"

ret=-1
ret=$(echo $(rpm -q vmfs-tools) | grep -c "vmfs-tools")
if [ ${ret} -ne 1 ]; then
    exit 1
fi
exit 0
Log "********************************end of execute the vmfs_check_tool script********************************"