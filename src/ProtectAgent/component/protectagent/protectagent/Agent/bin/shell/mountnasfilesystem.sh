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
#@function: mount nas share path

AGENT_ROOT_PATH=$1
PID=$2
PARAM_NUM=$3
umask 0022
. "${AGENT_ROOT_PATH}/sbin/agent_sbin_func.sh"

#for log
LOG_FILE_NAME="${LOG_PATH}/mountnasfilesystem.log"
TMP_DIR="/mnt"

IS_IPV6=0

BLOCK_LIST="^/$\|^/tmp$\|^/mnt/databackup$\|^/bin$\|^/bin/.*\|^/boot$\|^/boot/.*\|^/dev$\|^/dev/.*\|^/etc$\|^/etc/.*\|^/lib$\|^/lib/.*\|^/lost+found$\|^/lost+found/.*\|^/media$\|^/media/.*\|^/mnt$\|^/opt$\|^/proc$\|^/proc/.*\|^/root$\|^/root/.*\|^/sbin$\|^/sbin/.*\|^/selinux$\|^/selinux/.*\|^/srv$\|^/srv/.*\|^/sys$\|^/sys/.*\|^/usr$\|^/usr/.*\|^/var$\|^/var/.*\|^/run$\|^/run/.*"

EX_AGENT_USER=exrdadmin
IS_DPC=`cat ${AGENT_ROOT_PATH}/conf/agent_cfg.xml |grep '<is_dpc_compute_node' | awk -F '"' '{print $2}'`

CheckFileSystem()
{
    if [ ${SYS_NAME} = "AIX" ]; then
        ip=`mount | grep "$3" | $MYAWK '{print $1}' | grep "$1"`
        if [ $? -ne 0 -o -z "${ip}" ]; then
            return ${ERROR_CHECK_MOUNT_FS}
        fi
        fs=`mount | grep "$3" | $MYAWK '{print $2}' | grep "$2"`
        if [ $? -ne 0 -o -z "${fs}" ]; then
            return ${ERROR_CHECK_MOUNT_FS}
        fi
        return 0
    elif [ ${SYS_NAME} = "SunOS" ]; then
        if [ ${IS_IPV6} -ne 1 ]; then
            Log "mount | grep "^$3 on $1:$2""
            sourceFS=`mount | grep "^$3 on $1:$2"`
        else
            sourceFS=`mount | grep "^$3 on \\[$1]\\]:$2"`
        fi
        if [ $? -ne 0 ] || [ -z "${sourceFS}" ]; then
            Log "check fail.sourceFS=${sourceFS}"
            return ${ERROR_CHECK_MOUNT_FS}
        fi
        Log "sourceFS=${sourceFS}"
        return 0
    else
        storageServer="$1"
        if [ "$IS_DPC" = "true" ]
        then
            storageServer="system"
        fi
        if [ ${IS_IPV6} -ne 1 ] || [ ${IS_DPC} = "true" ]; then
            sourceFS=`mount | grep "$3" | $MYAWK '{print $1}' | grep "$storageServer:$2"`
        else
            sourceFS=`mount | grep "$3" | $MYAWK '{print $1}' | grep "\["$storageServer"\]:""$2"`
        fi
        if [ $? -ne 0 -o -z "${sourceFS}" ]; then
            Log "Mount $storageServer:$2 on $3 check failed."
            if [ ${IS_IPV6} -ne 1 ] || [ ${IS_DPC} = "true" ]; then
                otherFS=`mount | grep "$3" | grep "${storageServer}:"`
            else
                otherFS=`mount | grep "$3" | grep "\["$storageServer"\]:"`
            fi
            if [ ! -z "${otherFS}" ]; then
                mountPointsList=`mount | grep "${otherFS}" | $MYAWK '{print $3}'`
                for mountPoint in ${mountPointsList}; do
                    CheckMountPath ${mountPoint}
                    umount -f ${mountPoint} >> $LOG_FILE_NAME 2>&1
                done
            fi
            return ${ERROR_CHECK_MOUNT_FS}
        else
            return 0
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

CheckMountPath()
{
    realMountPath=""
    if [ ${SYS_NAME} = "AIX" ] || [ ${SYS_NAME} = "SunOS" ]; then
        cd $1
        realMountPath=`pwd`
        Log "realpath=${realMountPath}"
        cd -
    else
        mountPath=$1
        realMountPath=`TimeoutWithOutput realpath ${mountPath} $$`
    fi
    expr match ${realMountPath} ${BLOCK_LIST} > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        Log "The mount path is in the blocklist, path is ${mountPath}."
        exit ${ERROR_MOUNTPATH_BLOCk}
    fi
}

# 日志仓根目录固定给755 root:root, 任务ID目录根据应用返回权限设置
ModifyMountPointPermissions()
{
    tmpHostMountPath=$1
    if [ ${RepositoryType} = "log" ]; then
        chmod 755 ${tmpHostMountPath}
        chown -h root:root ${tmpHostMountPath}
        if [ ! -f ${tmpHostMountPath}/.agentlastlogbackup.meta ]; then
            touch ${tmpHostMountPath}/.agentlastlogbackup.meta
            chmod 640 ${tmpHostMountPath}/.agentlastlogbackup.meta
            chown -h ${AGENT_USER}:${AGENT_GROUP} ${tmpHostMountPath}/.agentlastlogbackup.meta
        fi
        if [ ! -f ${tmpHostMountPath}/.dmelastlogbackup.meta ]; then
            touch ${tmpHostMountPath}/.dmelastlogbackup.meta
            chmod 640 ${tmpHostMountPath}/.dmelastlogbackup.meta
            chown -h ${AGENT_USER}:${AGENT_GROUP} ${tmpHostMountPath}/.dmelastlogbackup.meta
        fi
        subPathList=`echo ${SubPath} | sed 's/:/ /g'`
        for path in ${subPathList}; do
            tmpPath="${tmpHostMountPath}/${path}"
            [ ! -d "${tmpPath}" ] && mkdir -p "${tmpPath}"
            if [ "${RunAccount}" = "rdadmin" ]; then
                chmod ${appMode} ${tmpPath}
                chown -h ${AGENT_USER}:${appGroupName} ${tmpPath}
            elif [ "${RunAccount}" = "exrdadmin" ] && [ "${AGENT_BACKUP_SCENE}" == "1" ]; then
                chmod ${appMode} ${tmpPath}
                chown -h ${EX_AGENT_USER}:${appGroupName} ${tmpPath}
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
        chown -h root:root ${tmpHostMountPath}
        if [ "${RunAccount}" = "rdadmin" ]; then
            chmod ${appMode} ${tmpHostMountPath}
            chown -h ${AGENT_USER}:${appGroupName} ${tmpHostMountPath}
        elif [ "${RunAccount}" = "exrdadmin" ] && [ "${AGENT_BACKUP_SCENE}" == "1" ]; then
            chmod ${appMode} ${tmpHostMountPath}
            chown -h ${EX_AGENT_USER}:${appGroupName} ${tmpHostMountPath}
        fi
        subPathList=`echo ${SubPath} | sed 's/:/ /g'`
        for path in ${subPathList}; do
            tmpPath="${tmpHostMountPath}/${path}"
            [ ! -d "${tmpPath}" ] && mkdir -p "${tmpPath}"
            if [ "${RunAccount}" = "rdadmin" ]; then
                chmod ${appMode} ${tmpPath}
                chown -h ${AGENT_USER}:${appGroupName} ${tmpPath}
            elif [ "${RunAccount}" = "exrdadmin" ] && [ "${AGENT_BACKUP_SCENE}" == "1" ]; then
                chmod ${appMode} ${tmpPath}
                chown -h ${EX_AGENT_USER}:${appGroupName} ${tmpPath}
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
            if [ "${PluginName}" = "VirtualizationPlugin" ]; then
                chmod -R ${appMode} ${tmpHostMountPath}
                chown -h -R ${AGENT_USER}:${appGroupName} ${tmpHostMountPath}
                ChangeUser ${tmpHostMountPath} ${UserID}
            else
                chmod ${appMode} ${tmpHostMountPath}
                chown -h ${AGENT_USER}:${appGroupName} ${tmpHostMountPath}
                ChangeUser ${tmpHostMountPath} ${UserID}
            fi
        fi
    elif [ "${RunAccount}" = "exrdadmin" ] && [ "${AGENT_BACKUP_SCENE}" == "1" ]; then
        if [ ${RepositoryType} = "meta" ] || [ ${RepositoryType} = "data" ] || [ ${RepositoryType} = "index" ]; then
            if [ "${PluginName}" = "VirtualizationPlugin" ]; then
                chmod -R ${appMode} ${tmpHostMountPath}
                chown -h -R ${EX_AGENT_USER}:${appGroupName} ${tmpHostMountPath}
                ChangeUser ${tmpHostMountPath} ${UserID}
            else
                chmod ${appMode} ${tmpHostMountPath}
                chown -h ${EX_AGENT_USER}:${appGroupName} ${tmpHostMountPath}
                ChangeUser ${tmpHostMountPath} ${UserID}
            fi
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

CheckModifyMountPointPermissions()
{
    tmpRealMountPath=$1
    Timeout ModifyMountPointPermissions ${tmpRealMountPath}
    if [ $? -ne 0 ]; then
        Log "Fail to modify mount point permissions for ${tmpRealMountPath}."
        exit ${ERROR_MOUNT_FAILED}
    fi
}

Log "********************************Start to execute the nas mount script********************************"

# global var, for kill monitor
PARAM_CONTENT=`ReadInputParam`
test -z "$PARAM_CONTENT"              && ExitWithError "PARAM_FILE"


#for GetValue
StorageIP=`GetValue "${PARAM_CONTENT}" storageIp`
HostMountPath=`GetValue "${PARAM_CONTENT}" mountPath`
FileSystemMountPath=`GetValue "${PARAM_CONTENT}" nasFileSystemName`
MountOption=`GetMountParamValue "${PARAM_CONTENT}" mountOption`
MountProtocol=`GetValue "${PARAM_CONTENT}" mountProtocol`
RepositoryType=`GetValue "${PARAM_CONTENT}" repositoryType`
RunAccount=`GetValue "${PARAM_CONTENT}" runAccount`
UserID=`GetValue "${PARAM_CONTENT}" uid`
GroupID=`GetValue "${PARAM_CONTENT}" gid`
Mode=`GetValue "${PARAM_CONTENT}" mode`
SubPath=`GetValue "${PARAM_CONTENT}" subPath`
PluginName=`GetValue "${PARAM_CONTENT}" pluginName`
JobID=`GetValue "${PARAM_CONTENT}" jobID`

Log "StorageIP=$StorageIP"
Log "HostMountPath=$HostMountPath"
test "$StorageIP" = "${ERROR_PARAM_INVALID}"                && ExitWithError "StorageIP"
test "$HostMountPath" = "${ERROR_PARAM_INVALID}"            && ExitWithError "HostMountPath"
test "$FileSystemMountPath" = "${ERROR_PARAM_INVALID}"      && ExitWithError "FileSystemMountPath"
test "$MountOption" = "${ERROR_PARAM_INVALID}"              && ExitWithError "MountOption"
test "$MountProtocol" = "${ERROR_PARAM_INVALID}"            && ExitWithError "MountProtocol"
test "$RepositoryType" = "${ERROR_PARAM_INVALID}"           && ExitWithError "RepositoryType"
test "$RunAccount" = "${ERROR_PARAM_INVALID}"               && ExitWithError "RunAccount"

test -z "$HostMountPath"           && ExitWithError "HostMountPath"
test -z "$FileSystemMountPath"     && ExitWithError "FileSystemMountPath"
test -z "$MountOption"             && ExitWithError "MountOption"
test -z "$StorageIP"               && ExitWithError "StorageIP"
test -z "$MountProtocol"           && ExitWithError "MountProtocol"
test -z "$RepositoryType"          && ExitWithError "RepositoryType"


if [ `CheckIsIpv6 ${StorageIP}` -eq 0 ]; then
    MountOption=${MountOption/tcp,}
    IS_IPV6=1
fi
Log "PID=${PID};HostMountPath=${HostMountPath};FileSystemMountPath=${FileSystemMountPath};MountProtocol=${MountProtocol}; \
    MountOption=${MountOption};StorageIP=${StorageIP};RepositoryType=${RepositoryType};UserID=${UserID};GroupID=${GroupID}; \
    Mode=${Mode};SubPath=${SubPath};PluginName=${PluginName};JobID=${JobID}."

grepStr=""
if [ ${SYS_NAME} = "AIX" ]; then
    grepStr=" ${HostMountPath} nfs"
elif [ ${SYS_NAME} = "SunOS" ]; then
    grepStr="^${HostMountPath} "
else
    grepStr=" ${HostMountPath} type"
fi
sourceFS=`TimeoutMountWithOutput mount | grep "${grepStr}"`
if [ $? -eq 0 -o -n "${sourceFS}" ]; then
    if [ ${SYS_NAME} = "AIX" ]; then
        grepStr="^${StorageIP} ${FileSystemMountPath} ${HostMountPath} nfs"
    elif [ ${SYS_NAME} = "SunOS" ]; then
        if [ ${IS_IPV6} -ne 1 ]; then
            grepStr="^${HostMountPath} on ${StorageIP}:${FileSystemMountPath} "
        else
            grepStr="^${HostMountPath} on \\[${StorageIP}\\]:${FileSystemMountPath} "
        fi
    else
        storageServer="$StorageIP"
        if [ "$IS_DPC" = "true" ]
        then
            storageServer="system"
        fi
        if [ ${IS_IPV6} -ne 1 ] || [ "$IS_DPC" = "true" ]; then
            grepStr="^${storageServer}:${FileSystemMountPath} on ${HostMountPath} type"
        else
            grepStr="^\\[${storageServer}\\]:${FileSystemMountPath} on ${HostMountPath} type"
        fi
    fi
    sourceNasFS=`TimeoutMountWithOutput mount | grep "${grepStr}"`
    if [ $? -eq 0 -o -n "${sourceNasFS}" ]; then
        Log "Mounted ${sourceNasFS}, will skip mount operation."
        exit 0;
    else
        Log "Mounted other file system ${sourceFS}, mount failed."
        exit ${ERROR_POINT_MOUNTED}
    fi
fi

CheckMountPath ${HostMountPath}

if [ ! -d "${HostMountPath}" ]; then
    Timeout mkdir -p "${HostMountPath}" >> $LOG_FILE_NAME 2>&1
    if [ $? -ne 0 ]; then
        Log "Ceate mount point failed."
        exit ${ERROR_MOUNTPATH}
    fi
fi
Timeout chattr +i ${HostMountPath}

subHostMountPath=${HostMountPath#*/}
subHostMountPath=${subHostMountPath#*/}
dirList=`echo ${subHostMountPath} | sed 's/\// /g'`
cd ${TMP_DIR}
for dirName in ${dirList}; do
    chmod 555 ${dirName}
    cd ${dirName}
done
cd /

retryTime=1
while [ $retryTime -le 3 ]; do
    if [ ${SYS_NAME} = "AIX" ]; then
        mountRes=`TimeoutMountWithOutput mount -o ${MountOption} ${StorageIP}":"${FileSystemMountPath} ${HostMountPath}`
    elif [ ${SYS_NAME} = "SunOS" ]; then
        WAIT_SEC=60
        mountRes=`TimeoutMountWithOutput mount -F ${MountProtocol} -o ${MountOption} ${StorageIP}":"${FileSystemMountPath} ${HostMountPath}`
    else
        if [ ${IS_IPV6} -ne 1 ]; then
            if [ "$IS_DPC" = "true" ]
            then
                Log "Exec command: mount -t dpc -o cnflush system:${FileSystemMountPath} ${HostMountPath}"
                mountRes=`TimeoutMountWithOutput mount -t dpc -o cnflush system":"${FileSystemMountPath} ${HostMountPath}`
            else
                Log "Exec command: mount -t ${MountProtocol} -o ${MountOption} -o noexec -o nosuid ${StorageIP}:${FileSystemMountPath} ${HostMountPath}"
                mountRes=`TimeoutMountWithOutput mount -t ${MountProtocol} -o ${MountOption} -o noexec -o nosuid ${StorageIP}":"${FileSystemMountPath} ${HostMountPath}`
            fi
        else
            if [ "$IS_DPC" = "true" ]
            then
                Log "Exec command: mount -t dpc -o cnflush system:${FileSystemMountPath} ${HostMountPath}"
                mountRes=`TimeoutMountWithOutput mount -t dpc -o cnflush system":"${FileSystemMountPath} ${HostMountPath}`
            else
                mountRes=`TimeoutMountWithOutput mount -t ${MountProtocol} -o ${MountOption} -o noexec -o nosuid "["${StorageIP}"]:"${FileSystemMountPath} ${HostMountPath}`
            fi
        fi
    fi
    echo ${mountRes} >> $LOG_FILE_NAME
    CheckFileSystem ${StorageIP} ${FileSystemMountPath} ${HostMountPath}
    if [ $? -eq 0 ]; then
        break
    fi
    Log "Mount ${StorageIP}:${FileSystemMountPath} ${HostMountPath} check failed retry $retryTime time"
    retryTime=`expr $retryTime + 1`
    if [ $retryTime -ge 4 ]; then
        chattr -i ${HostMountPath}
        DeleteFile ${RESULT_FILE}
        echo ${mountRes} > "${RESULT_FILE}"
        exit ${ERROR_MOUNT_FAILED}
        break
    fi
    sleep 5
done

appGroupName=${AGENT_GROUP}
if [ -n "${GroupID}" ]; then
    appGroupName=${GroupID}
fi

mode=755
appMode=${mode}
if [ -n "${Mode}" ]; then
    appMode=${Mode}
fi

AGENT_BACKUP_SCENE=`cat /opt/DataBackup/ProtectClient/ProtectClient-E/conf/testcfg.tmp | grep "BACKUP_SCENE" | cut -d'=' -f2`
Log "The scene is ${AGENT_BACKUP_SCENE}"
CheckModifyMountPointPermissions ${HostMountPath}
Log "Mount ${StorageIP}:${FileSystemMountPath} ${HostMountPath} success."
exit 0
