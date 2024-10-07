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
SCRIPT_PATH=$(dirname $0)
SCRIPT_NAME="${0##*/}"
PLUGIN_NAME="FilePlugin"
LOG_FILE=${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient/ProtectClient-E/slog/Plugins/${PLUGIN_NAME}/file_plugin.log

#定义一些变量
readonly SNAP_SIZE_UNIT="k"
readonly SNAP_MOUNT_PATH="/opt/jfs-snapshots"
readonly SNAP_VOL_PREFIX="snap"

LOG()
{
    if [ ! -z "$LOG_FILE" ]
    then
        local dirName=$(dirname "$LOG_FILE")
        mkdir -p $dirName

        local newLogFile=0
        if [ -f "$LOG_FILE" ]
        then
            local logSize=$(ls -l $LOG_FILE | awk '{printf $5}')
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
    echo -e "[${log_time}][${log_level}][ ${log_content} ][$$][${SCRIPT_NAME}][$(whoami)]" | tee -a "$LOG_FILE"
    return 0
}

GetVgNameByLvName() {
    ##
    ## DESCRIPTION:
    ##   Get the volume group name of a logical volume device.
    ##
    ## ARGUMENTS:
    ##   1: lvName (req): deviceName: /dev/hd4
    ##
    ##
    local deviceName=${1}
    local lvName=`echo "${deviceName}" | awk -F '/' '{print $3}'`
    local vgName=`lslv $lvName 2>/dev/null | grep "VOLUME GROUP" | awk -F ' ' '{print $6}'`
    if [[ -z "${vgName}" ]]; then
		LOG "ERROR" "Failed to get vgName for device: ${deviceName}."
        return 2 # error
    fi
    echo "$vgName"
    return 0
}

createSnapshotByVolume() {
	##
	## DESCRIPTION:
	##   creates a snapshot of a logical volume.
	##
	## ARGUMENTS:
	##   1: originDeviceMountPath (req): eg:/home
	## RETURN
	## SnapVolumeName:SnapmountPath 
	## 
	##
	local originDeviceMountPath="${1}"
	
	if [[ -z "${originDeviceMountPath}" ]]; then
		LOG "ERROR" "Failed to createSnapshotByVolume: argument 4(originDeviceMountPath) is empty."
		return 2 # error
	fi
    # Get lv name without /dev eg:/dev/hd1 -> hd1
    local lvName=`df -c ${originDeviceMountPath} | grep ${originDeviceMountPath} | awk -F ':' '{print $1}' | awk -F '/' '{print $3}'`
    # Get vg name
    local vgName=`lslv $lvName | grep "VOLUME GROUP" | awk -F ' ' '{print $6}'`
    # Get the free size of vg in megabytes
    local freeVgSize=`lsvg $vgName | grep -w "FREE PPs:" | awk -F '[()]' '{print $2}' | awk -F ' ' '{print $1}'`
    # Get lv size
    local lvSize=`df -m ${originDeviceMountPath} | grep ${originDeviceMountPath} | awk -F ' ' '{print $2}' | awk -F '.' '{print $1}'`
    # calculate the size of snapshot volume. eg: Take 5% of lvsize as the snapshot size
    local snapshotSize=$((${lvSize}*5/100))
    if [ ${snapshotSize} -eq 0 ]; then
        snapshotSize=1
    fi
    # Check whether the free size of the vg is greater than the size required by the snapshot volume.
    if [ ${freeVgSize} -lt ${snapshotSize} ]; then
        LOG "INFO" "Create snapshot failed, vg free size is not enought. volume: ${lvName}, lvsize: ${lvSize}, snapshot size: ${snapshotSize}, free vgsize: ${freeVgSize}"
        return 3
    fi
    snapshot -o snapfrom=${originDeviceMountPath} -o size=${snapshotSize}M

	if [ $? -ne 0 ];then
		local retValue= $?
		LOG "ERROR" "Failed to createSnapshot,${originDeviceMountPath} , returned value is ${retValue}"
		return 4 # error
	fi
	return 0 
}

mountSnapshot() {

	##
	## DESCRIPTION:
	##   mounts a snapshot volume on a directory.
	##
	## ARGUMENTS:
	##   1: snapshotVolumeDevice (/dev/vg/lv)
	##   2: snapshotVolumeMountDirectory : /opt/snapshots/home
	##
	##

	local snapshotVolumeDevice=${1}
	if [[ -z "${snapshotVolumeDevice}" ]]; then
		LOG "ERROR" "Failed to mountSnapshot: argument 1(snapshotVolumeDevice) is empty."
		return 2 # error
	fi
	
	local snapshotVolumeMountDirectory=${2}
	if [[ -z "${snapshotVolumeMountDirectory}" ]]; then
		LOG "ERROR" "Failed to mountSnapshot: argument 2(snapshotVolumeMountDirectory) is empty."
		return 2 
	fi
	

	## check if snapshot volume device exists
	if [[ ! -e "${snapshotVolumeDevice}" ]]; then
	    LOG "ERROR" "Failed to mountSnapshot: snapshot volume device '${snapshotVolumeDevice}' does not exist"
		return 2 
	fi

	## check if snapshotVolumeMountDirectory exists
	if [ ! -d "${snapshotVolumeMountDirectory}" ];then
    	mkdir -p "${snapshotVolumeMountDirectory}"
	fi

	## mount snapshot volume
	if ! mount -o snapshot "${snapshotVolumeDevice}" "${snapshotVolumeMountDirectory}"; then
		LOG "ERROR" "Failed to mountSnapshot: snapshot volume device '${snapshotVolumeDevice}'"
		return 2
	fi
	return 0 
}

umountSnapshot()
{
	##
	## DESCRIPTION:
	## umount all mountpoint under the specificed path
	##
	## ARGUMENT:
	## 1. the specificed path
	##
	##
	local mountPath=${1}
	LOG "INFO" "umount all path under ${mountPath}"
	if [[ -z ${mountPath} ]]; then
		LOG "ERROR" "${mountPath} do not exist"
	fi
	mount | grep ${mountPath} | awk -F' ' '{print $2}' | xargs -n 1 umount -f
	return $?
}

deleteSnapshot()
{
	##
	## DESCRIPTION:
	##   deletes a snapshot volume.
	##
	## ARGUMENTS:
	##   1: created_snap_vol (req):  volumeGroupName/snapshotVolumeName  vgpssa/test-snapshot
	##   2：mountParentPath (req): mountParentPath
	##   
	##
	##
	local created_snap_vol=${1}
	LOG "INFO" "deleteSnapshot of volume:$created_snap_vol"
	if [[ -z "${created_snap_vol}" ]]; then
		LOG "ERROR" "Failed to deleteSnapshot: argument 1(created_snap_vol) is empty."
		return 2
	fi

	LOG "INFO" "Removing snapshot:${created_snap_vol}"
	snapshot -d ${created_snap_vol}

	return $?
}

unmount() {

	##
	## DESCRIPTION:
	##   unmounts a filesystem from a directory.
	##
	## ARGUMENTS:
	##   1: mountPointDirectory
	##
	##

	local mountPointDirectory=${1}
	if [[ -z "${mountPointDirectory}" ]]; then
	    LOG "ERROR" "Failed to unmount directory: argument 1(mountPointDirectory) is empty."
		return 2 # error
	fi
	
	## unmount filesystem from mount point directory
	if ! umount "${mountPointDirectory}" >>"${LOG_FILE}" 2>&1; then
	    LOG "ERROR" "Failed to unmount filesystem from directory '${mountPointDirectory}'"
		return 2 # error
	fi
	return 0 
} 

# fail if user is not root
validate_root_user() {
  if [ "$(whoami)" != "root" ];then
    LOG "ERROR" "this script requires root privileges"
    exit 1
  fi
}

main()
{
    local L_OPERATION=$1
    case "${L_OPERATION}" in
	-cv)
        createSnapshotByVolume "$2"
        return $?
        ;;
    -dv)
	    deleteSnapshot "$2"
        return $?
        ;;
	-mount)
	    #挂载LV卷
	    mountSnapshot "$2" "$3"
        return $?
        ;;
	-umount)
	    #解挂载LV卷
	    umountSnapshot "$2"
        return $?
        ;;
    -ld)
        #根据逻辑卷名称lvName回卷组名称vgName
        GetVgNameByLvName "$2"
        return $?
        ;;
    *)
        LOG "ERROR" "Invalid operation."
        return 1
        ;;
    esac
    return 0
}

validate_root_user
main "$@"
