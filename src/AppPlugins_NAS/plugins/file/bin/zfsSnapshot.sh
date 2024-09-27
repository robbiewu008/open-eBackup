#!/bin/bash
#
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
#
SCRIPT_PATH=$(cd $(dirname $0); pwd)
SCRIPT_NAME="${0##*/}"
PLUGIN_NAME="FilePlugin"
LOG_FILE=${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient/ProtectClient-E/slog/Plugins/${PLUGIN_NAME}/file_plugin.log

if [ "${OS_TYPE}" = "SunOS" ]; then
    AWK=nawk
else
    AWK=awk
fi

#定义一些变量
readonly SNAP_MOUNT_DEFAULT_PREFIX="/opt/zfs_snapshots"
readonly ERR_SNAPSHOT_OUT_OF_SPACE="out of space"
readonly SUCCESS=0;
readonly ERR_NUM_INVALID_OPERATION=1
readonly ERR_NUM_PARAM_INCORRECT=2;
readonly ERR_NUM_SNAPSHOT_OUT_OF_SPACE=3;
readonly ERR_NUM_OTHERS=4;

function LOG()
{
    if [ ! -z "$LOG_FILE" ]
    then
        local dirName=$(dirname "$LOG_FILE")
        mkdir -p $dirName

        local newLogFile=0
        if [ -f "$LOG_FILE" ]
        then
            local logSize=$(/bin/ls -l "$LOG_FILE" | awk '{print $5}')
            if [ $logSize -gt 10485760 ]
            then
                mv ${LOG_FILE} ${LOG_FILE}.old
                newLogFile=1
            fi
        fi

        if [ $newLogFile == "1" ]
        then
            touch $LOG_FILE
        fi
    fi

    local log_level=$1
    local log_content=$2
    local log_time=$(date +"%Y-%m-%d %H:%M:%S")
    echo -e "[${log_time}][${log_level}][ ${log_content} ][$$][${SCRIPT_NAME}][${USER}]" | tee -a "$LOG_FILE"
    return ${SUCCESS}
}

function checkFilePath()
{
	filePath=$1
	realPath=`realpath ${filePath}`
	if [ $? -ne 0 ]
	then
		LOG "ERROR" "${realPath} is not the effective path"
		exit 1
	fi
    if [ ! -e "${realPath}" ] && [ ! -d "${realPath}" ]
	then
		LOG "ERROR" "Filepath is not existed"
		exit 1
	fi
}

function createSnapshotByVolume() {
	##
	## DESCRIPTION:
	##   creates a snapshot of a zfs file system or volume.
	##
	## ARGUMENTS:
	##   1: zpool/zfs file system(or volume) (req): zfs/zfsDev
	##   2: snapshotMountPointPrePath (req): (default)/opt/zfs_snapshots/jobdId
	##   3: snapshotName (req): snapshot name, take jodid as eg:jobId
	##   4: originDeviceMountPoint (req): eg:/home
	## RETURN
	## SnapVolumeName:SnapmountPath 
	## 
	##
	local srcVolumeName=${1}
	if [[ -z "${srcVolumeName}" ]]; then
		LOG "ERROR" "Failed to createSnapshotByVolume: argument 1(srcVolumeName) is empty."
		return ${ERR_NUM_PARAM_INCORRECT} # error
	fi

	local snapshotName=${2}
	if [[ -z "${snapshotName}" ]]; then
		LOG "ERROR" "Failed to createSnapshotByVolume: argument 3(snapshot name) is empty."
		return ${ERR_NUM_PARAM_INCORRECT} # error
	fi

    ## snapshotVolumeName is srcVolumeName@snapshotVolumeName
	local snapshotVolumeName="${srcVolumeName}@${snapshotName}"
    ## Check whether the snapshot already exists
    zfs list -H -o name ${snapshotVolumeName} >/dev/null 2>&1
    if [ $? -eq 0 ]; then
        echo "${snapshotVolumeName} already exists"
        return ${SUCCESS};
    fi

    #Check whether the zpool has 5% free capacity.
    local zpoolName=${srcVolumeName%%/*}        # srcVolumeName is zpool/prefs1/.../fs
    local zpoolCapacity=`zpool list -H -o capacity ${zpoolName} | sed 's/%//g'`
	if [  ${zpoolCapacity} -ge 95 ]; then
        LOG "ERROR" "Failed to createSnapshotByVolume: zpool does not have nough space, ${zpoolName}:${zpoolCapacity}"
		return ${ERR_NUM_SNAPSHOT_OUT_OF_SPACE}
	fi

	## generate zfs volume/fs snapshot (e.g. srcDevice@snapshotName)
    local snapshotOutput=`zfs snapshot ${snapshotVolumeName} >/dev/null 2>&1 > /dev/null`
    local snapshotRet=$?
	if [ ${snapshotRet} -ne 0 ]; then
        LOG "ERROR" "Failed to create snapshot, name: ${snapshotVolumeName}, errno: $snapshotRet, message: ${snapshotOutput}."
        if [[ ${snapshotRet} == *${ERR_SNAPSHOT_OUT_OF_SPACE} ]]; then
            return ${ERR_NUM_SNAPSHOT_OUT_OF_SPACE}
        fi
		return ${ERR_NUM_OTHERS} # other error
    fi
    LOG "INFO" "Success create snaphot, snapshot name: ${snapshotVolumeName}."
    echo "snapshotName:${snapshotVolumeName}"

    ## Make Snapshot Directory Visible
    local snapdirStatus=`zfs get -H -o value snapdir ${srcVolumeName}`
    if [[ ${snapdirStatus} == "visible" ]]; then
        LOG "DEGUG" "Snapdir status of ${srcVolumeName} is already setted to visible!"
        return ${SUCCESS}
    fi
    LOG "INFO" "Snapdir status of ${srcVolumeName} is hidden, make it visible!"
    zfs set snapdir=visible ${srcVolumeName}
    if [[ $? -ne 0 ]]; then
        LOG "ERROR" "Failed to set snapdir to visible for volume: ${srcVolumeName}."
        return ${ERR_NUM_OTHERS}
    fi
	return ${SUCCESS}
}

function deleteSnapshotByName()
{
	##
	## DESCRIPTION:
	##   deletes a snapshot volume/file_system
	##
	## ARGUMENTS:
	##   1: snapshotName (req): snapshot name, volume/file_system@snapshotName
	##   
	##
	local snapshotName=${1}
	LOG "INFO" "Begin to delete snapshot by name: $snapshotName."
	if [[ -z "${snapshotName}" ]]; then
		LOG "ERROR" "Failed to delete snapshot: argument 1(snapshot name) is empty."
		return ${ERR_NUM_PARAM_INCORRECT}
	fi

    #检查快照是否存在，若不存在直接返回成功
    zfs list -H -t snapshot ${snapshotName} > /dev/null 2>&1
    if [ $? -ne 0 ]; then
        LOG "INFO" "Snapshot: ${snapshotName} does not exist, return success directly."
        return ${SUCCESS}
    fi

	#按快照名删除快照
    local destroyOutput=`zfs destroy ${snapshotName} >/dev/null 2>&1 > /dev/null`
    local destroyRet=$?
    if [ $? -ne 0 ]; then
        LOG "ERROR" "Failed to delete snapshot, name: ${snapshotName}, errno: $destroyRet, message: ${snapshotOutput}."
        return ${ERR_NUM_OTHERS}
    fi
	LOG "INFO" "Success delete snapshot by name: $snapshotName."

    #确认快照已经删除
    zfs list -H -t snapshot ${snapshotName} > /dev/null 2>&1
    if [ $? -ne 0 ]; then
        LOG "INFO" "Confirmed that the snapshot: ${snapshotName} has been deleted correctly"
        return ${SUCCESS}
    fi
    return ${ERR_NUM_OTHERS}
}

function deleteSnapshotByTag()
{
	##
	## DESCRIPTION:
	##   deletes a snapshot volume/file_system
	##
	## ARGUMENTS:
	##   1: snapshotName (req): snapshot name, @snapshotName
	##   
	##
	local snapshotName=${1}
	LOG "INFO" "Begin to delete snapshot by tag: $snapshotName"
	if [[ -z "${snapshotName}" ]]; then
		LOG "ERROR" "Failed to delete snapshot, argument 1(snapshot name) is empty."
		return ${ERR_NUM_PARAM_INCORRECT}
	fi

    #检索快照名称，并删除快照卷
    local delRet=${SUCCESS}
    local delFailedSnapshot=()
    for delSnapshot in `zfs list -H -o name -t snapshot | grep "@${snapshotName}"`
    do
        zfs destroy ${delSnapshot} >/dev/null 2>&1
        if [ ${delRet} -ne 0 ]; then
            delFailedSnapshot+=${delSnapshot}
            LOG "ERROR" "Failed to delete snapshot, ret: ${delRet}, delete failed snapshot: ${delSnapshot}"
        fi
	    LOG "INFO" "Success delete snapshot: $delSnapshot"
    done
    if [[ ${#array[@]} -ne 0 ]]; then
        echo "${delFailedSnapshot[@]}"
        LOG "INFO" "Delete failed to snapshot: ${delFailedSnapshot[@]}"
    fi
	LOG "INFO" "Success delete snapshot by tag: $snapshotName"
	return ${SUCCESS}
}

# fail if user is not root
validate_root_user() {
  id | grep "uid=0" >/dev/null 2>&1
  if [ $? -ne 0 ]; then
    LOG "ERROR" "this script requires root privileges"
    exit 1
  fi
}

function main()
{
    local L_OPERATION=$1
    case "${L_OPERATION}" in
	-cv)
        createSnapshotByVolume "$2" "$3"
        return $?
        ;;
    -dv)
	    deleteSnapshotByName "$2"
        return $?
        ;;
    -dtag)
	    deleteSnapshotByTag "$2"
        return $?
        ;;
    *)
        LOG "ERROR" "Invalid operation."
        return ${ERR_NUM_INVALID_OPERATION}
        ;;
    esac
    return ${SUCCESS}
}

validate_root_user
main "$@"