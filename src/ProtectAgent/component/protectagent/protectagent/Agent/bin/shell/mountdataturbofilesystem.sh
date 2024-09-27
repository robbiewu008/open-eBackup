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
LOG_FILE_NAME="${LOG_PATH}/mountdataturbofilesystem.log"
TMP_DIR="/tmp"

BLOCK_LIST="^/$\|^/bin$\|^/bin/.*\|^/boot$\|^/boot/.*\|^/dev$\|^/dev/.*\|^/etc$\|^/etc/.*\|^/lib$\|^/lib/.*\|^/lost+found$\|^/lost+found/.*\|^/media$\|^/media/.*\|^/mnt$\|^/opt$\|^/proc$\|^/proc/.*\|^/root$\|^/root/.*\|^/sbin$\|^/sbin/.*\|^/selinux$\|^/selinux/.*\|^/srv$\|^/srv/.*\|^/sys$\|^/sys/.*\|^/usr$\|^/usr/.*\|^/var$\|^/var/.*\|^/run$\|^/run/.*\|^/mnt/databackup$"
WHITE_LIST=("^/mnt/databackup/.*/.*$")

CheckWhiteList() {
    InputPath="$1"
    if [ ! -e "${InputPath}" ]; then
        Log "ERROR: Path: ${InputPath} is not exists."
        return 1
    fi
    AbsolutePath=`realpath ${InputPath}`
    Log "AbsolutePath:${AbsolutePath}"
    for wdir in "${WHITE_LIST[@]}"; do
        if [[ $AbsolutePath =~ $wdir ]]; then
            return 0
        fi
    done
    return 1
}

CheckMountPath()
{
    umountPath=$1
    realMountPath=`realpath ${umountPath}`
    expr match ${realMountPath} ${BLOCK_LIST} > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        Log "The umount path is in the blocklist, path is ${umountPath}."
        return 1
    fi
    return 0
}

CheckFileSystem()
{
    sourceFS=`mount | grep "$2" | $MYAWK '{print $1}' | grep "$1"`
    if [ $? -ne 0 -o -z "${sourceFS}" ]; then
        Log "Mount $1 on $2 check failed."
        otherFS=`mount | grep "$2"`
        mountPointsList=`mount | grep "${otherFS}" | $MYAWK '{print $3}'`
        for mountPoint in ${mountPointsList}; do
            CheckMountPath "${mountPoint}"
            if [ $? -ne 0 ]; then
                Log "ERROR: MountPoint[${mountPoint}] is in the blocklist."
                return 1
            fi
            umount -f ${mountPoint} >> $LOG_FILE_NAME 2>&1
        done
        return ${ERROR_CHECK_MOUNT_FS}
    else
        return 0
    fi
}

CheckFileSystemFileClient()
{
    sourceFS=`mount | grep "$1"`
    if [ $? -eq 0 -o -n "${sourceFS}" ]; then
        sourceDataturboFS=`mount | grep "$1" | $MYAWK '{print $1}' | grep -w "/dev/fuse"`
        if [ "${sourceDataturboFS}" = "/dev/fuse" ]; then
            Log "Mounted ${sourceDataturboFS}, will skip mount operation."
            return 0
        else
            Log "ERROR: Mounted other file system ${sourceFS}, mount failed."
            return ${ERROR_CHECK_MOUNT_FS}
        fi
    fi
}

ChangeUser()
{
    if [ $# -ne 2 ]; then
        return 0
    fi
    path=$1
    uid=$2
    if [ -n "${uid}" ]; then
        chown ${uid} ${path}
    fi
    return 0
}

# 使用Dataturbo挂载文件系统，失败重试三次
# 返回值0：成功
# 返回值1：失败
MountDataturboFs()
{
    tmpStorageName=$1
    tmpFsMountPath=$2
    tmpHostMountPath=$3
    retryTime=1
    while [ $retryTime -le 3 ]; do
        dataturbo mount storage_object storage_name="${tmpStorageName}" filesystem_name="${tmpFsMountPath}" mount_dir="${tmpHostMountPath}" >> $LOG_FILE_NAME 2>&1
        if [ $? -ne 0 ]; then
            Log "Mount ${tmpStorageName} ${tmpFsMountPath} ${tmpHostMountPath} failed retry $retryTime time"
            retryTime=`expr $retryTime + 1`
            sleep 5
            continue
        fi
        CheckFileSystem ${tmpFsMountPath} ${tmpHostMountPath}
        if [ $? -eq 0 ]; then
            return 0
        fi
        Log "Mount ${tmpStorageName} ${tmpFsMountPath} ${tmpHostMountPath} check failed retry $retryTime time"
        retryTime=`expr $retryTime + 1`
        sleep 5
    done
    return 1
}

# 使用Dataturbo挂载文件系统，失败重试三次
# 返回值0：成功
# 返回值1：失败
MountFileClientFs()
{
    retryTime=1
    while [ $retryTime -le 3 ]; do
        if [ -d "${AGENT_ROOT_PATH}/../../FileClient/bin" ]; then
            cd "${AGENT_ROOT_PATH}/../../FileClient/bin"
        else
            return 1
        fi
        if [ $IsLinkEncryption -eq 1 ]; then 
            mountRes=`TimeoutMountWithOutput ./file_admin_client --add --mount_point=${HostMountPath} --source_id=${JobID} --osad_ip_list=${IpList} --osad_auth_port=${AuthPort} --osad_server_port=${ServerPort} --tls`
        else
            mountRes=`TimeoutMountWithOutput ./file_admin_client --add --mount_point=${HostMountPath} --source_id=${JobID} --osad_ip_list=${IpList} --osad_auth_port=${AuthPort} --osad_server_port=${ServerPort}`
        fi
        echo ${mountRes} >> $LOG_FILE_NAME
        CheckFileSystemFileClient ${HostMountPath}
        if [ $? -ne 0 ]; then
            Log "Mount jobId:${JobID} ${HostMountPath} failed retry $retryTime time"
            retryTime=`expr $retryTime + 1`
            sleep 5
            continue
        fi
        Log "Mount FileClient success"
        return 0
    done
    return 1
}

# 修改挂载点权限
# 返回值0：成功
# 返回值1：失败
ModifyMountPointPermissions()
{
    tmpHostMountPath=$1
    if [ ${RepositoryType} = "log" ]; then
        chmod 755 ${tmpHostMountPath}
        chown root:root ${tmpHostMountPath}
        if [ ! -f ${tmpHostMountPath}/.agentlastlogbackup.meta ]; then
            touch ${tmpHostMountPath}/.agentlastlogbackup.meta
            chmod 640 ${tmpHostMountPath}/.agentlastlogbackup.meta
            chown rdadmin:rdadmin ${tmpHostMountPath}/.agentlastlogbackup.meta
        fi
        if [ ! -f ${tmpHostMountPath}/.dmelastlogbackup.meta ]; then
            touch ${tmpHostMountPath}/.dmelastlogbackup.meta
            chmod 640 ${tmpHostMountPath}/.dmelastlogbackup.meta
            chown rdadmin:rdadmin ${tmpHostMountPath}/.dmelastlogbackup.meta
        fi
        subPathList=`echo ${SubPath} | sed 's/:/ /g'`
        for path in ${subPathList}; do
            tmpPath="${tmpHostMountPath}/${path}"
            [ ! -d "${tmpPath}" ] && mkdir -p "${tmpPath}"
            if [ "${RunAccount}" = "rdadmin" ]; then
                chmod ${appMode} ${tmpPath}
                chown ${AGENT_USER}:${appGroupName} ${tmpPath}
            fi
            if [ -n "${GroupID}" ]; then
                chgrp ${appGroupName} ${tmpPath}
            fi
            if [ -n "${Mode}" ]; then
                chmod ${appMode} ${tmpPath}
            fi
            ChangeUser ${tmpPath} ${UserID}
        done
    fi

    if [ ${RepositoryType} = "cache" ]; then
        chmod 755 ${tmpHostMountPath}
        chown root:root ${tmpHostMountPath}

        subPathList=`echo ${SubPath} | sed 's/:/ /g'`
        for path in ${subPathList}; do
            tmpPath="${tmpHostMountPath}/${path}"
            [ ! -d "${tmpPath}" ] && mkdir -p "${tmpPath}"
            if [ "${RunAccount}" = "rdadmin" ]; then
                chmod ${appMode} ${tmpPath}
                chown ${AGENT_USER}:${appGroupName} ${tmpPath}
            fi
            if [ -n "${GroupID}" ]; then
                chgrp ${appGroupName} ${tmpPath}
            fi
            if [ -n "${Mode}" ]; then
                chmod ${appMode} ${tmpPath}
            fi
            ChangeUser ${tmpPath} ${UserID}
        done
    fi

    if [ "${RunAccount}" = "rdadmin" ]; then
        if [ ${RepositoryType} = "meta" ] || [ ${RepositoryType} = "data" ] || [ ${RepositoryType} = "index" ]; then
            chmod ${appMode} ${tmpHostMountPath}
            chown ${AGENT_USER}:${appGroupName} ${tmpHostMountPath}
            ChangeUser ${tmpHostMountPath} ${UserID}
        fi
    else
        if [ ${RepositoryType} = "meta" ] || [ ${RepositoryType} = "data" ] || [ ${RepositoryType} = "index" ]; then
            if [ -n "${GroupID}" ]; then
                chgrp ${appGroupName} ${tmpHostMountPath}
            fi
            if [ -n "${Mode}" ]; then
                chmod ${appMode} ${tmpHostMountPath}
            fi
            ChangeUser ${tmpHostMountPath} ${UserID}
        fi
    fi
}

Log "********************************Start to execute the dataturbo mount script********************************"

# global var, for kill monitor
PARAM_CONTENT=`ReadInputParam`
test -z "$PARAM_CONTENT"              && ExitWithError "PARAM_FILE"

#for GetValue
StorageName=`GetValue "${PARAM_CONTENT}" storageName`
HostMountPath=`GetValue "${PARAM_CONTENT}" mountPath`
FileSystemMountPath=`GetValue "${PARAM_CONTENT}" nasFileSystemName`
RepositoryType=`GetValue "${PARAM_CONTENT}" repositoryType`
RunAccount=`GetValue "${PARAM_CONTENT}" runAccount`
UserID=`GetValue "${PARAM_CONTENT}" uid`
GroupID=`GetValue "${PARAM_CONTENT}" gid`
Mode=`GetValue "${PARAM_CONTENT}" mode`
SubPath=`GetValue "${PARAM_CONTENT}" subPath`
IpList=`GetValue "${PARAM_CONTENT}" ipList`
JobID=`GetValue "${PARAM_CONTENT}" jobID`
AuthPort=`GetValue "${PARAM_CONTENT}" authPort`
ServerPort=`GetValue "${PARAM_CONTENT}" serverPort`
IsLinkEncryption=`GetValue "${PARAM_CONTENT}" isLinkEncryption`

test "$StorageName" = "${ERROR_PARAM_INVALID}"              && ExitWithError "StorageName"
test "$HostMountPath" = "${ERROR_PARAM_INVALID}"            && ExitWithError "HostMountPath"
test "$FileSystemMountPath" = "${ERROR_PARAM_INVALID}"      && ExitWithError "FileSystemMountPath"
test "$RepositoryType" = "${ERROR_PARAM_INVALID}"           && ExitWithError "RepositoryType"
test "$RunAccount" = "${ERROR_PARAM_INVALID}"               && ExitWithError "RunAccount"

test -z "$HostMountPath"           && ExitWithError "HostMountPath"
test -z "$FileSystemMountPath"     && ExitWithError "FileSystemMountPath"
test -z "$StorageName"             && ExitWithError "StorageName"
test -z "$RepositoryType"          && ExitWithError "RepositoryType"

Log "PID=${PID};HostMountPath=${HostMountPath};FileSystemMountPath=${FileSystemMountPath};StorageName=${StorageName};RepositoryType=${RepositoryType}; \
    UserID=${UserID};GroupID=${GroupID};Mode=${Mode};SubPath=${SubPath}."

# FileClient挂载分支
if [ "${StorageName}" = "FileClient" ]; then

    if [ ! -d "${HostMountPath}" ]; then
        mkdir -p "${HostMountPath}" >> $LOG_FILE_NAME 2>&1
        if [ $? -ne 0 ]; then
            Log "Ceate mount point failed."
            exit ${ERROR_MOUNT_FS}
        fi
    fi

    sourceFS=`mount | grep "${HostMountPath}"`
    if [ $? -eq 0 -o -n "${sourceFS}" ]; then
        sourceDataturboFS=`mount | grep "${HostMountPath}" | $MYAWK '{print $1}' | grep "/dev/fuse"`
        if [ "${sourceDataturboFS}" = "/dev/fuse" ]; then
            Log "Mounted ${sourceDataturboFS}, will skip mount operation."
            RealMountPath="${HostMountPath}${FileSystemMountPath}"
            ModifyMountPointPermissions ${RealMountPath}
            exit 0;
        else
            Log "ERROR: Mounted other file system ${sourceFS}, mount failed."
            exit ${ERROR_MOUNTED_OTHER_FS}
        fi
    fi

    CheckMountPath "${HostMountPath}"
    if [ $? -ne 0 ]; then
        Log "ERROR: MountPoint[${HostMountPath}] is in the blocklist."
        exit ${ERROR_MOUNT_FS}
    fi
    
    appGroupName=${AGENT_GROUP}
    if [ -n "${GroupID}" ]; then
        appGroupName=${GroupID}
    fi

    mode=755
    appMode=${mode}
    if [ -n "${Mode}" ]; then
        appMode=${Mode}
    fi
    MountFileClientFs
    if [ $? -ne 0 ]; then
        exit ${ERROR_MOUNT_FS}
    fi
    RealMountPath="${HostMountPath}${FileSystemMountPath}"
    ModifyMountPointPermissions ${RealMountPath}
    Log "Mount ${StorageName} ${FileSystemMountPath} ${HostMountPath} success."
    exit 0
fi



needReExec=0  # 用于判断是否需要重新挂载根文件系统，或检查是否挂载了根文件系统
rootFs=
prefixHost=
patchStr=`echo ${HostMountPath} | grep "${FileSystemMountPath}$"` # 挂载点末尾必须与文件系统共享点对应
if [ -n "${patchStr}" ]; then
    if [ "${RepositoryType}" = "meta" ] || [ "${RepositoryType}" = "cache" ] || [ "${RepositoryType}" = "data" ] || [ "${RepositoryType}" = "log" ]; then
        needReExec=1
        rootFs="/"`echo "${FileSystemMountPath}" | $MYAWK -F "/" '{print $2}'`
        prefixHost="${HostMountPath%${rootFs}*}${rootFs}"
        Log "Parse param, rootFs: ${rootFs}, prefixHost: ${prefixHost}."
    fi
fi

[ ! -d "${HostMountPath}" ] && mkdir -p "${HostMountPath}"
sourceFS=`mount | grep "${HostMountPath}"`
if [ $? -eq 0 -o -n "${sourceFS}" ]; then
    sourceDataturboFS=`mount | grep "${HostMountPath}" | $MYAWK '{print $1}' | grep "${FileSystemMountPath}"`
    if [ "${sourceDataturboFS}" = "${FileSystemMountPath}" ]; then
        Log "Mounted ${sourceDataturboFS}, will skip mount operation."
        exit 0;
    else
        Log "ERROR: Mounted other file system ${sourceFS}, mount failed."
        exit ${ERROR_MOUNTED_OTHER_FS}
    fi
else # 没有找到挂载点的话，要继续检查一下是否挂载了根文件系统，对于meta/cache/data/log做这个检查
    if [ $needReExec -eq 1 ]; then
        sourceFS=`mount | grep "${prefixHost}"`
        if [ $? -eq 0 -o -n "${sourceFS}" ]; then
            sourceDataturboFS=`mount | grep "${prefixHost}" | $MYAWK '{print $1}' | grep "${rootFs}"`
            if [ "${sourceDataturboFS}" = "${rootFs}" ]; then
                Log "Mounted root filesystem ${sourceDataturboFS}, will skip mount operation."
                exit 0;
            else
                Log "ERROR: Mounted other file system ${sourceFS}, mount failed."
                exit ${ERROR_MOUNTED_OTHER_FS}
            fi
        fi
    fi
fi

CheckMountPath "${HostMountPath}"
if [ $? -ne 0 ]; then
    Log "ERROR: MountPoint[${HostMountPath}] is in the blocklist."
    exit ${ERROR_MOUNT_FS}
fi

chattr +i ${HostMountPath}

appGroupName=${AGENT_GROUP}
if [ -n "${GroupID}" ]; then
    appGroupName=${GroupID}
fi

mode=755
appMode=${mode}
if [ -n "${Mode}" ]; then
    appMode=${Mode}
fi

MountDataturboFs "${StorageName}" "${FileSystemMountPath}" "${HostMountPath}"
if [ $? -ne 0 ]; then
    if [ $needReExec -eq 1 ]; then
        Log "Now try to mount root fs."
        chattr -i ${HostMountPath}
        CheckWhiteList "${prefixHost}"
        if [ $? -ne 0 ]; then
            Log "ERROR: MountPoint[${prefixHost}] is not in the whiteList."
            exit ${ERROR_MOUNT_FS}
        fi
        rm -rf ${prefixHost}/*
        chattr +i ${prefixHost}
        MountDataturboFs "${StorageName}" "${rootFs}" "${prefixHost}"
        if [ $? -ne 0 ]; then
            chattr -i ${prefixHost}
            exit ${ERROR_MOUNT_FS}
        fi
        suffixHostList=`echo "${HostMountPath#*${rootFs}}" | sed 's#/# #g'`
        # meta仓 和 cache仓更改权限不同
        if [ "${RepositoryType}" = "meta" ]; then
            for tmpDir in ${suffixHostList}; do
                tmpPath="${prefixHost}/${tmpDir}"
                Log "Meta path: ${tmpPath}."
                if [ "${RunAccount}" = "rdadmin" ]; then
                    chmod ${appMode} ${tmpPath}
                    chown ${AGENT_USER}:${appGroupName} ${tmpPath}
                    ChangeUser ${tmpPath} ${UserID}
                else
                    if [ -n "${GroupID}" ]; then
                        chgrp ${appGroupName} ${tmpPath}
                    fi
                    if [ -n "${Mode}" ]; then
                        chmod ${appMode} ${tmpPath}
                    fi
                    ChangeUser ${tmpPath} ${UserID}
                fi
            done
        fi 
        if [ "${RepositoryType}" = "cache" ]; then
            for tmpDir in ${suffixHostList}; do
                tmpPath="${prefixHost}/${tmpDir}"
                Log "Cache path: ${tmpPath}."
                chmod ${mode} ${tmpPath}
                chgrp ${groupName} ${tmpPath}
                ChangeUser ${tmpPath} ${UserID}
            done
        fi
        Log "Change dir property success."
    else
        chattr -i ${HostMountPath}
        chattr -i ${prefixHost}
        exit ${ERROR_MOUNT_FS}
    fi
fi

if [ ${RepositoryType} = "log" ]; then
    chmod 755 ${HostMountPath}
    chown root:root ${HostMountPath}
    if [ ! -f ${HostMountPath}/.agentlastlogbackup.meta ]; then
        touch ${HostMountPath}/.agentlastlogbackup.meta
        chmod 640 ${HostMountPath}/.agentlastlogbackup.meta
        chown rdadmin:rdadmin ${HostMountPath}/.agentlastlogbackup.meta
    fi
    if [ ! -f ${HostMountPath}/.dmelastlogbackup.meta ]; then
        touch ${HostMountPath}/.dmelastlogbackup.meta
        chmod 640 ${HostMountPath}/.dmelastlogbackup.meta
        chown rdadmin:rdadmin ${HostMountPath}/.dmelastlogbackup.meta
    fi
    subPathList=`echo ${SubPath} | sed 's/:/ /g'`
    for path in ${subPathList}; do
        tmpPath="${HostMountPath}/${path}"
        [ ! -d "${tmpPath}" ] && mkdir -p "${tmpPath}"
        if [ "${RunAccount}" = "rdadmin" ]; then
            chmod ${appMode} ${tmpPath}
            chown ${AGENT_USER}:${appGroupName} ${tmpPath}
        fi
        if [ -n "${GroupID}" ]; then
            chgrp ${appGroupName} ${tmpPath}
        fi
        if [ -n "${Mode}" ]; then
            chmod ${appMode} ${tmpPath}
        fi
        ChangeUser ${tmpPath} ${UserID}
    done
fi

if [ ${RepositoryType} = "cache" ]; then
    chmod 755 ${HostMountPath}
    chown root:root ${HostMountPath}

    subPathList=`echo ${SubPath} | sed 's/:/ /g'`
    for path in ${subPathList}; do
        tmpPath="${HostMountPath}/${path}"
        [ ! -d "${tmpPath}" ] && mkdir -p "${tmpPath}"
        if [ "${RunAccount}" = "rdadmin" ]; then
            chmod ${appMode} ${tmpPath}
            chown ${AGENT_USER}:${appGroupName} ${tmpPath}
        fi
        if [ -n "${GroupID}" ]; then
            chgrp ${appGroupName} ${tmpPath}
        fi
        if [ -n "${Mode}" ]; then
            chmod ${appMode} ${tmpPath}
        fi
        ChangeUser ${tmpPath} ${UserID}
    done
fi

if [ "${RunAccount}" = "rdadmin" ]; then
    if [ ${RepositoryType} = "meta" ] || [ ${RepositoryType} = "data" ] || [ ${RepositoryType} = "index" ]; then
        chmod ${appMode} ${HostMountPath}
        chown ${AGENT_USER}:${appGroupName} ${HostMountPath}
        ChangeUser ${HostMountPath} ${UserID}
    fi
else
    if [ ${RepositoryType} = "meta" ] || [ ${RepositoryType} = "data" ] || [ ${RepositoryType} = "index" ]; then
        if [ -n "${GroupID}" ]; then
            chgrp ${appGroupName} ${HostMountPath}
        fi
        if [ -n "${Mode}" ]; then
            chmod ${appMode} ${HostMountPath}
        fi
        ChangeUser ${HostMountPath} ${UserID}
    fi
fi

Log "Mount ${StorageName} ${FileSystemMountPath} ${HostMountPath} success."
exit 0