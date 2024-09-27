#!/bin/sh
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
set +x
#@function: mount nas share path

AGENT_ROOT_PATH=$1
PID=$2
PARAM_NUM=$3
. "${AGENT_ROOT_PATH}/sbin/agent_sbin_func.sh"

#for log
LOG_FILE_NAME="${LOG_PATH}/umountnasfilesystem.log"

BLOCK_LIST="^/$\|^/bin$\|^/bin/.*\|^/boot$\|^/boot/.*\|^/dev$\|^/dev/.*\|^/etc$\|^/etc/.*\|^/lib$\|^/lib/.*\|^/lost+found$\|^/lost+found/.*\|^/media$\|^/media/.*\|^/mnt$\|^/opt$\|^/proc$\|^/proc/.*\|^/root$\|^/root/.*\|^/sbin$\|^/sbin/.*\|^/selinux$\|^/selinux/.*\|^/srv$\|^/srv/.*\|^/sys$\|^/sys/.*\|^/usr$\|^/usr/.*\|^/var$\|^/var/.*\|^/run$\|^/run/.*"

CheckMountPath()
{
    umountPath=$1
    realMountPath=`TimeoutWithOutput realpath ${umountPath} $$`
    expr match ${realMountPath} ${BLOCK_LIST} > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        Log "The umount path is in the blocklist, path is ${umountPath}."
        return 1
    fi
    return 0
}

# 卸载文件系统，失败重试三次
UmountFs()
{
    umountPoint=$1
    repositoryType=$2
    repositoryTempDir=$3
    CheckMountPath "${umountPoint}"
    if [ $? -ne 0 ]; then
        Log "ERROR: umountPoint[${umountPoint}] is in the blocklist."
        return 1 # 检查路径不合法
    fi
    Timeout rm -f ${umountPoint}/*mounted.lock &
    retryTime=1
    while [ $retryTime -le 3 ]; do
        if [ ${SYS_NAME} = "AIX" ]; then
            TimeoutWithoutOutput umount -f ${umountPoint} >> $LOG_FILE_NAME 2>&1
        else
            Timeout umount -f ${umountPoint} >> $LOG_FILE_NAME 2>&1
        fi
        if [ $? -ne 0 ]; then
            Log "Force umount nas mountpoint: ${umountPoint} failed, retry ${retryTime} times, suggest to delete it manually."
            retryTime=`expr $retryTime + 1`
            sleep 5
            continue
        fi
        sourceFS=`mount | grep "${umountPoint}"`
        if [ $? -eq 0 -o -n "${sourceFS}" ]; then
            Log "Mountpoint: ${umountPoint} still exist, umount failed."
            return 2  # 卸载后挂载点依然存在
        else
            Log "Umount nas mountpoint: ${umountPoint} success."
            return 0 # 卸载成功
        fi
    done
    return 3 # 超过重试次数
}

# 卸载文件系统，失败重试三次
UmountFileClientFs()
{
    umountPoint=$1
    repositoryType=$2
    repositoryTempDir=$3
    CheckMountPath "${umountPoint}"
    if [ $? -ne 0 ]; then
        Log "ERROR: umountPoint[${umountPoint}] is in the blocklist."
        return 1 # 检查路径不合法
    fi

    retryTime=1
    while [ $retryTime -le 3 ]; do
        if [ -d "${AGENT_ROOT_PATH}/../../FileClient/bin" ]; then
            cd "${AGENT_ROOT_PATH}/../../FileClient/bin"
        else
            return 1
        fi
        Timeout ./file_admin_client --remove --mount_point=${umountPoint} >> $LOG_FILE_NAME 2>&1
        if [ $? -ne 0 ]; then
            Log "Force umount Fileclient mountpoint: ${umountPoint} failed, retry ${retryTime} times, suggest to delete it manually."
            retryTime=`expr $retryTime + 1`
            sleep 5
            continue
        fi
        sourceFS=`mount | grep "${umountPoint}"`
        if [ $? -eq 0 -o -n "${sourceFS}" ]; then
            Log "Mountpoint: ${umountPoint} still exist, umount failed."
            return 2  # 卸载后挂载点依然存在
        else
            Log "Umount nas mountpoint: ${umountPoint} success."
            return 0 # 卸载成功
        fi
    done
    return 3 # 超过重试次数
}

VaryOff()
{
    protocolId=$1
    adapterType=`echo $1 | grep iqn`
    if [ "${adapterType}" = "" ]; then
        adapterType="fc"
    else
        protocolId=${protocolId%_*}
        adapterType="iscsi"
    fi
    Log "${adapterType}"
    lunId=`printf '%x' $2`
    for hdisk in `lsdev -Cc disk |  $MYAWK '{print $1}'`; do
        if [ "${adapterType}" = "fc" ]; then
            ww_name=`lsattr -El ${hdisk} -F value -a ww_name | sed -e s/0x//`
            if [ "${ww_name}" != "${protocolId}" ]; then
                continue
            fi
        else
            target_name=`lsattr -El ${hdisk} -F value -a target_name`
            if [ "${target_name}" != "${protocolId}" ]; then
                continue
            fi
        fi
        lun_id=`lsattr -El ${hdisk} -F value -a lun_id | sed -e s/0x// | sed -e s/${lunId}/Z/ | sed -e s/0//g`
        Log "searched disk ${hdisk} lun id sed is ${lun_id}, orgin is:  ${lunId}."
        if [ "${lun_id}" != 'Z' ]; then
            continue
        fi
        isActive=`lspv | grep -w "${hdisk}" | ${MYAWK} '{print $NF}'`
        tmpVgName=`lspv | grep -w "${hdisk}" | ${MYAWK} '{print $3}'`
        Log "Find vg ${tmpVgName}, active is ${isActive}, ready to release."
        if [ -n "${tmpVgName}" ] && [ "${tmpVgName}" != "None" ]; then
            Log "Vary off vg ${tmpVgName}"
            TimeoutWithoutOutput varyoffvg ${tmpVgName} >> $LOG_FILE_NAME 2>&1
            Log "Export vg ${tmpVgName}"
            TimeoutWithoutOutput exportvg ${tmpVgName} >> $LOG_FILE_NAME 2>&1
        fi
        Log "Begin to remove ${hdisk}."
        rmdev -dl ${hdisk} >> $LOG_FILE_NAME 2>&1
        lspv | grep -w ${hdisk}
        if [ $? -eq 0 ]; then
            Log "Remove ${hdisk} failed, ${hdisk} exists."
            return 1
        fi
        return 0
    done
    return 0
}

RemoveSanclientLvm()
{
    LunInfo=$1
    for info in `echo ${LunInfo#*:} | $MYAWK -F '//' '{print $1,$NF}'`; do
        wwpntmp=${info%/*}
        protocolTmp=`echo ${wwpntmp} | $MYAWK -F ',' '{print $1}'`
        protocolId=`echo ${protocolTmp} | $MYAWK -F '_' '{print $1}'`
        adapterType=`echo $protocolId | grep iqn`
        if [ "${adapterType}" != "" ]; then
            Log "Start destroying chap certification."
            sanclientIqn=${info%%_*}
            grep -v ${sanclientIqn} /etc/iscsi/targets > /etc/iscsi/targets_tmp
            mv /etc/iscsi/targets_tmp /etc/iscsi/targets
        fi
        lunTmp=${info#*/}
        lunId=`echo ${lunTmp}|$MYAWK -F ',' '{print $1}'`
        Log "Begin to clear vg info with ${lunId}"
        VaryOff ${protocolId} ${lunId}
        if [ $? -ne 0 ]; then
            Log "Clear ${lunId} with err, ignore."
        fi
    done
}

RemoveSanclientWithMountPath()
{
    umountPath=$1
    devPath=`mount | grep "${umountPath}" |  $MYAWK '{print $1}'`
    if [ $? -ne 0 ]; then
        Log "Not find mount device. ignore"
        return 0
    fi
    lvm=${devPath##*/}
    hdisk=`lslv -l ${lvm} | grep -v "PV" | grep -v ${lvm} | $MYAWK '{print $1}'`
    UmountFs "${umountPath}"
    if [ $? -ne 0 ]; then
        Log "Umount ${umountPath} failed."
        return 1
    fi
    isActive=`lspv | grep -w "${hdisk}" | ${MYAWK} '{print $NF}'`
    tmpVgName=`lspv | grep -w "${hdisk}" | ${MYAWK} '{print $3}'`
    Log "Find vg ${tmpVgName}, active is ${isActive}, ready to release."
    if [ -n "${tmpVgName}" ] && [ "${tmpVgName}" != "None" ]; then
        Log "Vary off vg ${tmpVgName}"
        TimeoutWithoutOutput varyoffvg ${tmpVgName} >> $LOG_FILE_NAME 2>&1
        Log "Export vg ${tmpVgName}"
        TimeoutWithoutOutput exportvg ${tmpVgName} >> $LOG_FILE_NAME 2>&1
    fi
    Log "Begin to remove ${hdisk}."
    rmdev -dl ${hdisk} >> $LOG_FILE_NAME 2>&1
    lspv | grep -w ${hdisk}
    if [ $? -eq 0 ]; then
        Log "Remove ${hdisk} failed, ${hdisk} exists."
        return 1
    fi
    return 0
}

DelMountPath()
{
    PATH_TO_DELETE=$1
    JOB_ID=$2

    if [ -z "${JOB_ID}" ]; then
        Log "JobId is empty!"
        return 1
    fi

    PATH_TEMP=$(echo ${PATH_TO_DELETE} | grep ${JOB_ID})
    if [ "${PATH_TEMP}" = "${PATH_TO_DELETE}" ]; then
        PATH_TO_KEEP=$(echo "${PATH_TO_DELETE}" | sed "s/${JOB_ID}.*//")
    else
        PATH_TO_KEEP=/mnt/
    fi

    if [ -d ${PATH_TO_KEEP} ]; then
        Log "PATH_TO_KEEP is ${PATH_TO_KEEP}"
    else
        return 1
    fi
    # Add protection file
    TMP_FILE_PROTECTION=${PATH_TO_KEEP}/tmpprotectionfile
    if [ ! -f "${TMP_FILE_PROTECTION}" ]; then
        touch ${TMP_FILE_PROTECTION}
    fi
    Log "The path to delete is ${PATH_TO_DELETE}."
    rmdir -p ${PATH_TO_DELETE} >> $LOG_FILE_NAME 2>&1
    rm -f ${TMP_FILE_PROTECTION}
    return 0
}

Log "********************************Start to execute the nas umount script********************************"

PARAM_CONTENT=`ReadInputParam`
test -z "$PARAM_CONTENT"              && ExitWithError "PARAM_FILE"

#for GetValue
UmountPath=`GetValue "${PARAM_CONTENT}" umountPath`
RepositoryType=`GetValue "${PARAM_CONTENT}" repositoryType`
RepositoryTempDir=`GetValue "${PARAM_CONTENT}" repositoryTempDir`
JobId=`GetValue "${PARAM_CONTENT}" jobid`
IsFileClientMount=`GetValue "${PARAM_CONTENT}" isFileClientMount`
test "$UmountPath" = "${ERROR_PARAM_INVALID}"                           && ExitWithError "UmountPath"
Log "PID=${PID};umountPath=${UmountPath};repositoryType=${RepositoryType};repositoryTempDir=${RepositoryTempDir};jobId=${JobId}"

# 判断是否有sanClient挂载路径 mountpath="path#sanclientinfo"
sanClientInfo=`echo "${UmountPath}" | $MYAWK -F "#" '{print $2}'`
if [ -n "${sanClientInfo}" ]; then
    Log "Is sanclient mount, info:${sanClientInfo} need clear lvm."
    UmountPath=`echo "${UmountPath}" | $MYAWK -F "#" '{print $1}'`
    Log "umount path is ${UmountPath}."
    for info in `echo ${sanClientInfo#*:} | $MYAWK -F '//' '{for(i=1;i<NF;i++){print $i}}'`; do
        wwpntmp=${info%/*}
        protocolTmp=`echo ${wwpntmp} | $MYAWK -F ',' '{print $1}'`
        protocolId=`echo ${protocolTmp} | $MYAWK -F '_' '{print $1}'`
        adapterType=`echo $protocolId | grep iqn`
        if [ "${adapterType}" != "" ]; then
            Log "Start destroying chap certification."
            sanclientIqn=${info%%_*}
            grep -v ${sanclientIqn} /etc/iscsi/targets > /etc/iscsi/targets_tmp
            mv /etc/iscsi/targets_tmp /etc/iscsi/targets
        fi
        lunTmp=${info#*/}
        fileioName=`echo ${lunTmp} | $MYAWK -F ',' '{print $5}'`
        jobType=`echo ${lunTmp} | $MYAWK -F ',' '{print $6}'`
        if [ ${jobType} -eq 2 ]; then
            repoID=`echo ${fileioName} | $MYAWK -F '_' '{print $1}'`
            umountPoint="${UmountPath}/${repoID}"
            Log "Begin to umount mountpoint:${umountPoint}."
            RemoveSanclientWithMountPath ${umountPoint}
        fi
    done
fi

# 使用Dataturbo挂载时，挂载的是文件系统，但是传递的卸载路径是文件系统下的目录，这里主要适配这种情况
FsPath=`echo "${UmountPath}" | $MYAWK -F "@" '{print $2}'`
UmountPath=`echo "${UmountPath}" | $MYAWK -F "@" '{print $1}'`
Log "Input param, UmountPath: ${UmountPath}, FsPath: ${FsPath}."
#grep StorageIP and NasFileSystemName
StorageIP=${UmountPath##*/}

test -z "$UmountPath"              && ExitWithError "UmountPath"

echo "${sanClientInfo}" | grep -w "fileio" > /dev/null 2>&1
if [ $? -eq 0 ]; then
    Log "Remove sanClient mount should search lvm with mountpath first."
    RemoveSanclientWithMountPath ${UmountPath}
fi

#umount FileClient
if [ ${IsFileClientMount} = "true" ]; then
    sourceFS=`mount | grep "${UmountPath}"`
    if [ $? -eq 0 -o -n "${sourceFS}" ]; then
        UmountFileClientFs "${UmountPath}" "${RepositoryType}" "${RepositoryTempDir}"
        if [ $? -ne 0 ]; then
            exit ${ERROR_UMOUNT_FS}
        fi
        chattr -i ${UmountPath}
        DelMountPath ${UmountPath} ${JobId}
        exit 0
    fi
    Log "Mountpoint: ${umountPoint} has been umount."
    exit 0
fi


#umount nas
sourceFS=`mount | grep "${UmountPath}"`
if [ $? -eq 0 -o -n "${sourceFS}" ]; then
    UmountFs "${UmountPath}" "${RepositoryType}" "${RepositoryTempDir}"
    if [ $? -ne 0 ]; then
        exit ${ERROR_UMOUNT_FS}
    fi
    # 卸载sanclient vg
    if [ -n "${sanClientInfo}" ];then
        RemoveSanclientLvm ${sanClientInfo}
    fi
    # 卸载成功后，去掉保护属性，不管源文件是否有保护属性，都不会报错的
    chattr -i ${UmountPath}
    DelMountPath ${UmountPath} ${JobId}
else
    # 卸载sanclient vg
    if [ -n "${sanClientInfo}" ];then
        RemoveSanclientLvm ${sanClientInfo}
    fi
    Log "Mountpoint ${UmountPath} does not exist, will try to umount root fs!"
    if [ -z "${FsPath}" ]; then
        Log "Mountpoint ${UmountPath} does not exist, will skip umount operation!"
        exit 0
    fi
    # 判断主机路径字符串是否以要挂在的文件系统路径结尾
    patchStr=`echo ${UmountPath} | grep "${FsPath}$"`
    if [ -z "${patchStr}" ]; then
        exit ${ERROR_UMOUNT_FS}
    fi
    rootFs=`echo "${FsPath}" | $MYAWK -F "/" '{print $2}'`
    rootFs="/${rootFs}"
    prefixHost="${UmountPath%${rootFs}*}${rootFs}"
    Log "Root fs: ${rootFs}, prefixHost: ${prefixHost}."
    sourceFS=`mount | grep "${prefixHost}"`
    if [ $? -eq 0 -o -n "${sourceFS}" ]; then
        UmountFs "${prefixHost}"
        if [ $? -ne 0 ]; then
            exit ${ERROR_UMOUNT_FS}
        fi
        chattr -i ${prefixHost}
    fi
fi

exit 0
