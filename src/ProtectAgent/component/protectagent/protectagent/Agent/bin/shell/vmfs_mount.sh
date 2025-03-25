#!/bin/sh
set +x

AGENT_ROOT_PATH=$1
PID=$2
PARAM_NUM=$3
. "${AGENT_ROOT_PATH}/sbin/agent_sbin_func.sh"

readonly MNT_CMD_V6="vmfs6-fuse"
readonly MNT_CMD_V5="vmfs5-fuse"
readonly MNT_CMD_V0="vmfs-fuse"
readonly DBG_CMD_V6="debugvmfs6"
readonly DBG_CMD_V5="debugvmfs5"
readonly DBG_CMD_V0="debugvmfs"
readonly WWN_PATH="/dev/disk/by-id/"

readonly FIND_FILE_MAX_RETRY_COUNT=30
readonly FIND_FILE_RETRY_INTERVAL=5
readonly OPERATION_MAX_RETRY_COUNT=3
readonly OPERATION_RETRY_INTERVAL=10

VMFS_VERSION=-1
VMFS_UUID=""
VMFS_FULLNAME=""

#for log
LOG_FILE_NAME="${LOG_PATH}/vmfs_mount.log"
TMP_DIR="/mnt"

BLOCK_LIST="^/$\|^/tmp$\|^/mnt/databackup$\|^/bin$\|^/bin/.*\|^/boot$\|^/boot/.*\|^/dev$\|^/dev/.*\|^/etc$\|^/etc/.*\|^/lib$\|^/lib/.*\|^/lost+found$\|^/lost+found/.*\|^/media$\|^/media/.*\|^/mnt$\|^/opt$\|^/proc$\|^/proc/.*\|^/root$\|^/root/.*\|^/sbin$\|^/sbin/.*\|^/selinux$\|^/selinux/.*\|^/srv$\|^/srv/.*\|^/sys$\|^/sys/.*\|^/usr$\|^/usr/.*\|^/var$\|^/var/.*\|^/run$\|^/run/.*"

CheckPath() 
{
    filepat='[|;&$><`\!.]+'
    if [[ "$1" =~ ${filepat} ]]; then
        return 1
    fi

    if [[ "$1" =~ ^/mnt ]]; then
        return 0
    fi
    return 1
}

CheckPathEx()
{
    realMountPath=""
    if [ ${SYS_NAME} = "AIX" ] || [ ${SYS_NAME} = "SunOS" ]; then
        cd $1
        realMountPath=`pwd`
        cd -
    else
        mountPath=$1
        realMountPath=`realpath ${mountPath}`
    fi
    expr match ${realMountPath} ${BLOCK_LIST} > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        Log "The mount path is in the blocklist, path is ${realMountPath}."
        exit 1
    fi
    CheckPath ${realMountPath}
    if [ $? -ne 0 ]; then
        Log "The mount path is against the rule, path is ${realMountPath}."
        exit 1
    fi
}

GetWwnFullPath()
{
    cd ${WWN_PATH}
    local reTryCnt=0
    local wwnNum=$1
    Log " wwn= '${wwnNum}'"
    while [ ${reTryCnt} -lt ${FIND_FILE_MAX_RETRY_COUNT} ]; do
        local file_num=$(ls | grep -c "${wwnNum}")
        if [ ${file_num} -gt 0 ]; then
            break
        fi
        Log "find wwn failed , retry in ${FIND_FILE_RETRY_INTERVAL} seconds"
        local reTryCnt=`expr $reTryCnt + 1`
        sleep ${FIND_FILE_RETRY_INTERVAL}
    done
    if [ ${file_num} -eq 0 ]; then
        Log "no wwn found, exit ! wwn= '${wwnNum}'"
        exit 1
    fi

    local file_name="wwn-0x${wwnNum}-part1"
    local reTryCnt=0
    while [ ${reTryCnt} -lt ${FIND_FILE_MAX_RETRY_COUNT} ]; do
        local file_num=$(ls | grep -c "${file_name}")
        if [ ${file_num} -gt 0 ]; then
            break
        fi
        Log "find wwn file failed , retry in ${FIND_FILE_RETRY_INTERVAL} seconds"
        local reTryCnt=`expr $reTryCnt + 1`
        sleep ${FIND_FILE_RETRY_INTERVAL}
    done
    if [ ${file_num} -eq 0 ]; then
        Log "no file found, exit ! file_name= '${file_name}'"
        exit 1
    fi

    local fullPath=${WWN_PATH}${file_name}
    Log "fullPath = '${fullPath}'"
    echo $(realpath ${fullPath})
}

CheckVmfsVersion()
{
    local vmfsPath=${1}
    local paramStr="${vmfsPath} show 2>&1| grep "UUID:" | awk '{print $2}'"
    VMFS_UUID=$(${DBG_CMD_V6} ${paramStr})
    if [ ${#VMFS_UUID} -ne 0 ]; then
        VMFS_VERSION=6
        return 0
    fi
    VMFS_UUID=$(${DBG_CMD_V5} ${paramStr})
    if [ ${#VMFS_UUID} -ne 0 ]; then
        VMFS_VERSION=5
        return 0
    fi
    VMFS_UUID=$(${DBG_CMD_V0} ${paramStr})
    if [ ${#VMFS_UUID} -ne 0 ]; then
        VMFS_VERSION=0
        return 0
    fi
    Log "vmfs version not found, exit ! vmfsPath=${vmfsPath} "
    exit 1
}

CheckVmfs()
{
    cd ${WWN_PATH}
    file_num=$(ls | grep -c "$1") 
    if [ ${file_num} -eq 0 ]; then
        Log "no file found, exit !"
        exit 1
    else
        file_name=$(ls | grep --line-regexp "wwn-$1-part1")
    fi
    VMFS_FULLNAME=${WWN_PATH}${file_name} 
    REAL_VMFS_PATH=$(realpath ${VMFS_FULLNAME})
    param_str="${REAL_VMFS_PATH} show 2>&1| grep "UUID:" | awk '{print $2}'"
    VMFS_UUID=$(${DBG_CMD_V6} ${param_str})
    if [ ${#VMFS_UUID} -ne 0 ]; then
        VMFS_VERSION=6
        return 0
    fi
    VMFS_UUID=$(${DBG_CMD_V5} ${param_str})
    if [ ${#VMFS_UUID} -ne 0 ]; then
        VMFS_VERSION=5
        return 0
    fi
    VMFS_UUID=$(${DBG_CMD_V0} ${param_str})
    if [ ${#VMFS_UUID} -ne 0 ]; then
        VMFS_VERSION=0
        return 0
    fi
    Log "vmfs version not found, exit !"
    exit 1
}

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

VerifyMnt()
{
    #mnt_file=$1
    mnt_path=$1
    if [ $(mount | grep "${mnt_path} " | awk '{print $1}') != "/dev/fuse" ]; then
        Log "target directory is occupied by sth else,  exit with fail!"
        exit 1
    fi
}

Log "********************************Start to execute the vmfs mount script********************************"

# global var, for kill monitor
PARAM_CONTENT=`ReadInputParam`
test -z "$PARAM_CONTENT"              && ExitWithError "PARAM_FILE"

Log ${PARAM_CONTENT}

REAL_MNT_PATH=$(GetValue "${PARAM_CONTENT}" mnt_path)

mkdir -p ${REAL_MNT_PATH}

CheckPathEx ${REAL_MNT_PATH}
CheckMntPoint ${REAL_MNT_PATH}
if [ $? -eq 0 ]; then
    VerifyMnt ${REAL_MNT_PATH}
    Log "target directory is occupied by other fuse block divece, exit!"
    exit 1
fi

wwnArrCnt=0
for item in ${PARAM_CONTENT}; do
    key=`echo ${item} | $MYAWK -F "=" '{print $1}'`
    value=`echo ${item} | $MYAWK -F "=" '{print $2}'`
    if [ ${key} = "wwn" ]; then
        wwnArr[${wwnArrCnt}]=${value}
        wwnArrCnt=${wwnArrCnt}+1
    fi
done
wwnRealPathCnt=0
for item in ${wwnArr}; do
    wwnRealPath[${wwnRealPathCnt}]=$(GetWwnFullPath "${item}")
    wwnRealPathCnt=$(expr ${wwnRealPathCnt} + 1)
done

CheckVmfsVersion ${wwnRealPath[0]}
Log "vmfs uuid is: ${VMFS_UUID}"
Log "vmfs version is: ${VMFS_VERSION}"

exec_cmd_str=""
if [ ${VMFS_VERSION} -eq 6 ]; then
    exec_cmd_str="${MNT_CMD_V6}"
elif [ ${VMFS_VERSION} -eq 5 ]; then
    exec_cmd_str="${MNT_CMD_V5}"
elif [ ${VMFS_VERSION} -eq 0 ]; then
    exec_cmd_str="${MNT_CMD_V0}"
else
    Log "no valid version. exit !"
    exit 1
fi

for item in ${wwnRealPath}; do
    exec_cmd_str="${exec_cmd_str} ${item}"
done
exec_cmd_str="${exec_cmd_str} ${REAL_MNT_PATH}"
Log "cmd string is : ${exec_cmd_str}"

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
else
    Log "vmfs_fuse success ! "
    exit 0
fi
if [ ${ret} -ne 0 ]; then
    Log "failed to execute vmfs_fuse, exit !"
    exit 1
else
    Log "vmfs_fuse success ! "
    exit 0
fi
Log "********************************end of execute the vmfs mount script********************************"