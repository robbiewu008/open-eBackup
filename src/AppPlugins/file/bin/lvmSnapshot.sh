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
readonly SNAP_MOUNT_PATH="/opt/lvm_snapshots"
readonly SNAP_VOL_PREFIX="snap"
readonly LOST_AND_FOUND_DIR="/lost+found"

function LOG()
{
    if [ ! -z "$LOG_FILE" ]
    then
        local dirName=$(dirname "$LOG_FILE")
        mkdir -p $dirName

        local newLogFile=0
        if [ -f "$LOG_FILE" ]
        then
            local logSize=$(stat -c "%s" "$LOG_FILE")
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

function createSnapshot()
{
    ## ----- head -----
	##
	## DESCRIPTION:
	##   creates a snapshot of a file.
	##
	## ARGUMENTS:
	##   1: filePath (req): eg:/opt/install
	##
	##
	local filePath=${1}
	LOG "INFO" "createSnapshot of file:${filePath}."
	if [[ -z "${filePath}" ]]; then
		LOG "ERROR" "Failed to createSnapshot: argument 1(filePath) is empty."
		return 2
	fi
	checkFilePath "${filePath}"
	dfSys=$(df --output=source,fstype,target "$filePath" 2>/dev/null)
	ret=$?
	if [ $ret -ne 0 ];then
        LOG "ERROR" "Failed to createSnapshot: ${filePath} is not exist."
        return $ret
    fi
	dfInfo=$(echo "$dfSys" | tail -n +2)
	fsName=$(echo $dfInfo | awk '{ print $1 }')
	#fsType=$(echo $dfInfo | awk '{ print $2 }')
	#原始文件挂载点
	mountPoint=$(echo $dfInfo | awk '{ print $3 }')
	pathSufix=${filePath#"${mountPoint}"}
	#

	checkvolume ${fsName}
	lvPath=$(lvdisplay "${fsName}" 2>/dev/null | grep "LV Path" | awk '{print $3}')
	vol_group=$(echo $lvPath | awk -F '/' '{ print $3 }')
	logical_vol=$(echo $lvPath | awk -F '/' '{ print $4 }')
	LV_SIZE=$(lvs $lvPath --units "${SNAP_SIZE_UNIT}" 2>/dev/null | awk 'FNR==2 {print $4}' | cut -d. -f1 )
	SNAP_SIZE=$((LV_SIZE*5/100))
	currentTime=$(date "+%Y%m%d%H%M%S")
	snap_vol_name="${SNAP_VOL_PREFIX}_${logical_vol}_${currentTime}"
	created_snap_vol=${vol_group}/${snap_vol_name}
	mount_path="${SNAP_MOUNT_PATH}/$(get_mount_path "${created_snap_vol}")"
	if [[ ! -e "/dev/${created_snap_vol}" ]]; then
		#判断快照是否存在,若存在直接返回组装后的
	   lvcreate -s -n ${snap_vol_name} --addtag "plugin" -L ${SNAP_SIZE}${SNAP_SIZE_UNIT} ${vol_group}/${logical_vol} >> $LOG_FILE 2>&1
	   if [ $? -ne 0 ];then
		   LOG "ERROR" "Failed to createSnapshot,$created_snap_vol"
		   return $?
	   fi

	   LOG "INFO" "mounting /dev/${created_snap_vol} to ${mount_path}"
	   mkdir -p "${mount_path}"
	   mount -o ro "/dev/${created_snap_vol}" "${mount_path}"
	fi
	echo "${created_snap_vol}:${mount_path}:${mount_path}${pathSufix}"
	LOG "INFO" "${created_snap_vol}:${mount_path}:${mount_path}${pathSufix}"


}

function createSnapshotByVolume() {
	##
	## DESCRIPTION:
	##   creates a snapshot of a logical volume.
	##
	## ARGUMENTS:
	##   1: volumeGroupName/logicalVolumeName (req): vg/lv
	##   2: snapMountPrePath (req): /opt/lvm-snapshots/jobid
	##   3: snapTag (req): eg:jobid
	##   4: originDeviceMountPath (req): eg:/home
	## RETURN
	## SnapVolumeName:SnapmountPath
	##
	##

	local deviceVolumeName=${1}
	if [[ -z "${deviceVolumeName}" ]]; then
		LOG "ERROR" "Failed to createSnapshotByVolume: argument 1(deviceVolumeName) is empty."
		return 2 # error
	fi
	local mountPrePath=${2:-"${SNAP_MOUNT_PATH}"}

	local snapTag=${3}
	if [[ -z "${snapTag}" ]]; then
		LOG "ERROR" "Failed to createSnapshotByVolume: argument 3(snapTag) is empty."
		return 2 # error
	fi
	local originDeviceMountPath="${4}"

	if [[ -z "${originDeviceMountPath}" ]]; then
		LOG "ERROR" "Failed to createSnapshotByVolume: argument 4(originDeviceMountPath) is empty."
		return 2 # error
	fi
    # 快照百分比大小，默认为5
	local percent=${5:-5}

	## generate logical volume device (e.g. /dev/vg.sys/lv.home)
	local logicalVolumeDevice="/dev/${deviceVolumeName}"
    if [[ ! -b "${logicalVolumeDevice}" ]]; then
		LOG "ERROR" "Failed to createSnapshot: ${logicalVolumeDevice} is not block special file."
		return 2
	fi
	logical_vol=$(echo $logicalVolumeDevice | awk -F '/' '{ print $4 }')
	vol_group=$(echo $logicalVolumeDevice | awk -F '/' '{ print $3 }')
	snap_vol_name="${SNAP_VOL_PREFIX}_${logical_vol}_${snapTag}"
	created_snap_vol=${vol_group}/${snap_vol_name}
	mount_path="${mountPrePath}${originDeviceMountPath}"
	if [[ -e "/dev/${created_snap_vol}" ]]; then
		echo "${created_snap_vol}:${mount_path}"
		return 0
	fi

    LV_SIZE=$(lvs $logicalVolumeDevice --units "${SNAP_SIZE_UNIT}" 2>/dev/null | awk 'FNR==2 {print $4}' | cut -d. -f1 )
	SNAP_SIZE=$((LV_SIZE*${percent}/100))
	if [ $SNAP_SIZE -eq 0 ] ; then
	    LOG "ERROR" "Failed to createSnapshotByVolume:lv has no free space left."
		return 3 # error - space not enough
	fi
	vg_free_size=$(vgs $vol_group --units "${SNAP_SIZE_UNIT}" 2>/dev/null | awk 'NR >1 {print int($7)}')
	if [ $vg_free_size -lt $SNAP_SIZE ] ; then
	    LOG "ERROR" "Failed to createSnapshotByVolume:vg has no free space left."
		return 3 # error - space not enough
	fi

	lvcreate -s -n ${snap_vol_name} --addtag "${snapTag}" -L ${SNAP_SIZE}${SNAP_SIZE_UNIT} ${vol_group}/${logical_vol} >> $LOG_FILE 2>&1
	if [ $? -ne 0 ];then
		LOG "ERROR" "Failed to createSnapshot,$created_snap_vol"
		return 4 # error
	fi
	echo "${created_snap_vol}:${mount_path}"
	LOG "INFO" "Success create snaphot for volume: ${deviceVolumeName}, snapshot volume: ${created_snap_vol}, mount path: ${mount_path}"
	return 0
}

function changeSnapshotVolumeID() {
	local snapshotVolumeMapper="${1}"
	local fsType=$(blkid -c /dev/null -s TYPE -o value ${snapshotVolumeMapper} 2>>$LOG_FILE)
    if [[ $? -ne 0 ]]; then
	    LOG "ERROR" "failed running blkid to determine filesystem type of snapshot volume device '${snapshotVolumeMapper}'"
		return 2 # error
	fi
	local ret=0
    LOG "INFO" "changeSnapshotVolumeID fsType: ${fsType}"
	case "${fsType}" in
		xfs)
			xfs_repair -L ${snapshotVolumeMapper} > /dev/null 2>&1
			xfs_admin -U generate ${snapshotVolumeMapper} > /dev/null 2>&1
			;;
		btrfs)
			btrfstune -f -u ${snapshotVolumeMapper}
			;;
		ext[1-9])
            e2fsck -f -p ${snapshotVolumeMapper}
            local fsckRet=$?
			if [ ${fsckRet} -ne 0 ]; then
				ret=${ADD_LOST_FOUND}
				LOG "WARNNING" "e2fsck ret is ${fsckRet}, should delete lost+found dir later!"
			fi
			echo y | tune2fs -f -U random ${snapshotVolumeMapper}
			;;
		*)
			LOG "ERR" "Can not change snapshot volume id, fstype is ${fsType}"
			;;
	esac
	return ${ret};
}

function mountSnapshot() {

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

	local volumeGroupName=$(echo "$snapshotVolumeDevice" | awk -F '/' '{ print $3 }')
	local snapshotVolumeName=$(echo "$snapshotVolumeDevice" | awk -F '/' '{ print $4 }')
	local snapshotVolumeMapper="/dev/${volumeGroupName}/${snapshotVolumeName}"

    local fsType=$(blkid -c /dev/null -s TYPE -o value ${snapshotVolumeMapper} 2>>$LOG_FILE)
	## mount snapshot volume
	local option=rw
	if [[ "${fsType}" == "xfs" ]]; then
        option=rw,nouuid  ## xfs文件系统相同uuid的卷不允许同时挂载
    fi
	LOG "INFO" "mount -o $option ${snapshotVolumeDevice} ${snapshotVolumeMountDirectory}"
	mount -o $option ${snapshotVolumeDevice} ${snapshotVolumeMountDirectory}
	if [ $? -ne 0 ]; then
		LOG "ERROR" "Failed to mountSnapshot: snapshot volume device '${snapshotVolumeDevice}'"
		return 2
	fi

	## delete added lost+found dir
    if [[ "${fsType}" =~ ext[1-9] ]]; then
		LOG "INFO" "Delete lost+found dir: ${snapshotVolumeMountDirectory}${LOST_AND_FOUND_DIR}"
		rm -rf "${snapshotVolumeMountDirectory}${LOST_AND_FOUND_DIR}"
    fi
    return 0
}

function checkvolume()
{
    LOG "INFO" "Checking availability of volume: $@"
    lvdisplay $@ > /dev/null 2>&1
    if [ $? -ne 0  ]; then
		LOG "ERROR" "Volume '$@' does not exist"
        exit 2
    else
        return $?
    fi
}

# replace '/' with '-' in $1
function get_mount_path()
{
   sed 's/\//-/' <<< "${1}";
}

function deleteSnapshot()
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

	if [[ "${created_snap_vol}" =~ "/dev/"* ]]; then
		local snapshotVolumeDevice="${created_snap_vol}"
	else
		local snapshotVolumeDevice="/dev/${created_snap_vol}"
	fi
	LOG "INFO" "snapshotVolumeDevice:${snapshotVolumeDevice}"
	if [[ -e "${snapshotVolumeDevice}" ]]; then
		#查找挂载点
		#校验是否为快照卷,防止误删
		if ! checkIsSnapshotVolume "${snapshotVolumeDevice}"; then
			LOG "ERROR" "device '${snapshotVolumeDevice}' is not a snapshot volume"
			return 2 # error
	    fi
		## check if mount point directory actually has something mounted on
		local mountPath=$(findmnt --noheadings --output=target "${snapshotVolumeDevice}" 2>>"${LOG_FILE}")
        LOG "INFO" "mountPath:${mountPath}"
		if [[ -n ${mountPath} ]]; then
			LOG "INFO" "umount snapshotVolumeDevice: ${snapshotVolumeDevice}"
            umount -l ${snapshotVolumeDevice}
			#删除挂载目录
			LOG "INFO" "remove mountPath:${mountPath}"
			rm -rf ${mountPath}
        fi

		#删除快照卷
		LOG "INFO" "Removing snapshot:${snapshotVolumeDevice}"
		local delRet=-1
		local lvsRet=-1
		local maxTime=3
		local retryInterval=3
		for ((deletTime=1; deletTime<=maxTime; deletTime++)); do
			# 执行快照卷删除
			lvremove -f ${snapshotVolumeDevice} 2>>$LOG_FILE
			delRet=$?
			# 检查快照卷是否删除成功
			lvs --noheadings ${snapshotVolumeDevice} 2>>$LOG_FILE
			lvsRet=$?
			if [ $lvsRet -ne 0 ]; then
				LOG "INFO" "Delete snapshot volume:${snapshotVolumeDevice} success and have Confirmed that the snapshot volume was not exist!"
				return 0
			else
				LOG "WARNNING" "Delete snapshot volume:${snapshotVolumeDevice} failed, try again in 3 seconds!"
				sleep retryInterval
			fi
		done
		LOG "ERROR" "Delete snapshot volume:${snapshotVolumeDevice} failed after all ${deletTime} times try!"
		return $delRet
	else
		LOG "WARNNING" "Failed to deleteSnapshot: snapshotVolume:'${created_snap_vol}' is not exist."
		return 0
	fi
}

function checkDirHasMount() {

	## ----- head -----
	##
	## DESCRIPTION:
	##    检查指定目录是否已挂载 0：已挂载 1：未挂载.
	##
	## ARGUMENTS:
	##   1: directory (req)
	#
	##

	local directory=${1}
	if [[ -z "${directory}" ]]; then
		LOG "ERROR" "Failed to checkDirHasMount: argument 1(directory) is empty."
		return 2 # error
	fi
	if [[ ! -e /proc/mounts ]]; then
	    LOG "ERROR" "/proc/mounts does not exist"
		return 2 # error
	fi

	## split /proc/mounts into array by newline
	IFS=$'\n'
	local -a procMountArray=( $(sort -k 2,2 < /proc/mounts) )
	local -i sortExitCode=${?}
	IFS=$' \t\n'
	if [[ ${sortExitCode} -ne 0 ]]; then
	    LOG "ERROR" "failed sorting /proc/mounts"
		return 2
	fi

	## loop through array of mounts
	local -i i
	for ((i = 0; i < ${#procMountArray[@]}; i++)); do

		IFS=' '
		set -- ${procMountArray[i]}
		IFS=$' \t\n'
		local procMountPoint=${2}
		local procMountOpts=${4}
		set --
		if [[ "${procMountPoint}" == "${directory%/}" ]]; then
			return 0
		fi
	done
	return 1
}


function deleteSnapshotByTag()
{
	##
	## DESCRIPTION:
	##   根据快照标签删除所有快照
	##
	## ARGUMENTS:
	##   1: snapTag (req): snapTag
	##
	##
	##
	local snapTag=${1}
	LOG "INFO" "deleteSnapshotByTag of snapTag:$snapTag"
	if [[ -z "${snapTag}" ]]; then
		LOG "ERROR" "Failed to deleteSnapshot: argument 1(snapTag) is empty."
		return 2
	fi

	#按标签查找快照
	local -a snapshotVolumeArray=( $(lvs --separator / --noheadings --nosuffix  -o vg_name,lv_name --select="role=snapshot" "@${snapTag}" | tr -d " " 2>>"${LOG_FILE}") )
    local -i lvsExitCode=${?}
	if [[ ${lvsExitCode} -ne 0 ]]; then
		LOG "ERROR" "Failed to deleteSnapshot: snapTag:${snapTag}"
		return 2 # error
	fi
	local -i i
	for ((i = 0; i < ${#snapshotVolumeArray[@]}; i++)); do
		local snapshotVolume=${snapshotVolumeArray[i]}
		if [[ -z ${snapshotVolume} ]]; then
			continue
	    fi
	    deleteSnapshot "${snapshotVolume}"
		if [[ $? -ne 0 ]]; then
			LOG "ERROR" "Failed to deleteSnapshotByTag: snapTag:${snapTag}"
	    fi
	done
	return $?
}

function checkmount {
    LINES=`mount | grep $@ | wc -l`
    if [ $LINES -gt 0 ]; then
        return 0
    else
        return 1
    fi
}

function checkIsSnapshotVolume()
{
	##
	## DESCRIPTION:
	##   checks if the given device is a snapshot volume.
	##
	## ARGUMENTS:
	##   1: device (req) /dev/group/lv
	##
	##

	local device=${1}
	if [[ -z "${device}" ]]; then
		LOG "ERROR" "Failed to checkIsSnapshotVolume: argument 1(device) is empty."
		return 2 # error
	fi

	## get snapshot volume origin
	local snapshotVolumeOrigin=$(lvs --separator / --noheadings --nosuffix --units m -o origin "${device}" 2>>"${LOG_FILE}" | xargs -n 1 2>>"${LOG_FILE}")
	local -i lvsExitCode=${?}
	if [[ ${lvsExitCode} -ne 0 ]]; then
	    LOG "ERROR" "Failed to running 'lvs' to check if device '${device}' is a snapshot volume"
		return 2 # error
	fi

	## 为空代表不是snapshot volume
	if [[ -z ${snapshotVolumeOrigin} ]]; then
		return 1
	fi
	return 0
}

function getMountPathByVolume() {
	local deviceArgs=${1}
	if [[ -z "${deviceArgs}" ]]; then
		LOG "ERROR" "Failed to getMountPathByVolume: argument 1(deviceArgs) is empty."
		return 2
	fi
	local logicalMountPath=$(findmnt --noheadings --output=target "${deviceArgs}" | tr -d " " 2>>"${LOG_FILE}")
	echo "${logicalMountPath}"
}

function getMountedVolumes() {

	##
	## DESCRIPTION:获取指定设备的挂载点或根据目录获取所有的挂载设备。
	##
	##
	## ARGUMENTS:
	##   1. printType: 'device' or 'directory'
	##   2:  device:传设备卷名   /dev/mapper/v
    ##   	 directory: /opt or /mnt/snapshot
	##   3: isCrossVolume  'true'  or 'false'
	##
	##


    local printType=${1}
	local deviceArgs=${2}
	local isCrossVolume=${3:-"true"}
	if [[ -z "${printType}" ]]; then
		LOG "ERROR" "Failed to getMountedVolumes: argument 1(printType) is empty."
		return 2
	fi
	if [[ "${printType}" != "device" ]] && [[ "${printType}" != "directory" ]]; then
	     LOG "ERROR" "Failed to getMountedVolumes: argument 1(printType) is wrong(only device or directory is right)."
		return 2
	fi

	if [[ -z "${deviceArgs}" ]]; then
		LOG "ERROR" "Failed to getMountedVolumes: argument 2(deviceArgs) is empty."
		return 2
	fi


	if [[ ! -e /proc/mounts ]]; then
	    LOG "ERROR" "/proc/mounts does not exist"
		return 2 # error
	fi
	## split /proc/mounts into array by newline
	IFS=$'\n'
	local -a procMountArray=( $(sort -k 2,2 < /proc/mounts) )
	local -i sortExitCode=${?}
	IFS=$' \t\n'
	if [[ ${sortExitCode} -ne 0 ]]; then
	    LOG "ERROR" "failed sorting /proc/mounts"
		return 2 # error
	fi

	local -i i
	for ((i = 0; i < ${#procMountArray[@]}; i++)); do
		## split mount line into fields
		IFS=' '
		set -- ${procMountArray[i]}
		IFS=$' \t\n'
		local procMountDevice=${1}
		local procMountPoint=${2}
		local procMountFilesystemType=${3}
		set --
		case "${procMountFilesystemType}" in
			debugfs|devpts|nfsd|proc|rootfs|securityfs|sysfs|tmpfs|usbfs|nfs|btrfs)
				continue
				;;
			ext[234]|xfs) #supported fstype
				;;
			*)
				continue
				;;
		esac
		if [ ${printType} = "device" ]; then
		   if [ ${procMountDevice} = "${deviceArgs}" ]; then
			   echo "${procMountPoint}"
			   return 0
		   fi
		else
		   if [ ${isCrossVolume} = "true" ]; then
		         case "${procMountPoint}" in
					${deviceArgs%/}) ;;
					${deviceArgs%/}/*) ;;
			       *) continue ;;
		         esac
			else
			    case "${procMountPoint}" in
					${deviceArgs%/}) ;;
			       *) continue ;;
		        esac
		   fi
		   local logicalVolumeInfo=$(printLogicalVolumeInfo "${procMountDevice}")
		   echo "${logicalVolumeInfo}:${procMountPoint}"
		fi

	done
	return 0

} #


function printLogicalVolumeInfo() {

	## ----- head -----
	##
	## DESCRIPTION:
	##   prints the volume group name and logical volume name of a logical volume device.
	##
	## ARGUMENTS:
	##   1: device (req): /dev/mapper/vg-lv_name
	##
	##
	##s

	local device=${1}
	if [[ -z "${device}" ]]; then
	    LOG "ERROR" "Failed to printLogicalVolumeInfo: argument 1(device) is empty."
		return 2 # error
	fi

	local logicalVolumeInfo=$(lvs --separator / --noheadings --nosuffix \
		--units m -o vg_name,lv_name "${device}" 2>> "${LOG_FILE}" | tr -d " " 2>>"${LOG_FILE}")
	local -i lvsExitCode=$?
	if [[ ${lvsExitCode} -ne 0 ]]; then
		LOG "ERROR" "failed to get logical volume info of device '${device}'"
		return 2
	else
		if checkIsSnapshotVolume "${device}"; then
			LOG "ERROR" "snapshot logical device is not support: '${device}'"
			return 2
		fi
		echo "${logicalVolumeInfo}"
	fi

	return 0 # success

}



function unmount() {

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

function getLvmPath() {
    ##
	## DESCRIPTION:
	##   Get LvmPath(vg-name/lvm-name) by /dev/mapper/vg-lvm
	##
	## ARGUMENTS:
	##   1: volumename: /dev/mapper/vg-lvm or vg/lvm or /dev/vg/lvm
	##
	##

    local deviceVolumeName=${1}
	if [[ -z "${deviceVolumeName}" ]]; then
		LOG "ERROR" "Failed to createSnapshotByVolume: argument 1(deviceVolumeName) is empty."
		return 2 # error
	fi

    local vgName=$(lvdisplay $deviceVolumeName | grep "VG Name" | awk '{print $3}')
	if [[ -z "${vgName}" ]]; then
		LOG "ERROR" "Failed to get vgName for device: ${deviceVolumeName}."
		return 3
	fi

    local lvName=$(lvdisplay $deviceVolumeName | grep "LV Name" | awk '{print $3}')
	if [[ -z "${vgName}" ]]; then
		LOG "ERROR" "Failed to get lvName for device: ${deviceVolumeName}."
		return 3
	fi

	# SUSE 11.4(kernel 3.1) compatible issue
	local prefix="/dev/${vgName}/"
	if [[ $lvName == $prefix* ]]; then
		lvName=${lvName#$prefix}
	fi

    local lvmPath=${vgName}/${lvName}
    echo ${lvmPath}
    return 0
}

function main()
{
    local L_OPERATION=$1
    case "${L_OPERATION}" in
    -c)
        createSnapshot "$2"
        return $?
        ;;
	-cv)
        createSnapshotByVolume "$2" "$3" "$4" "$5" "$6"
        return $?
        ;;
    -dv)
	    deleteSnapshot "$2"
        return $?
        ;;
	-dtag)
	    deleteSnapshotByTag "$2"
        return $?
        ;;
	-lv)
	    #返回有效的LV逻辑卷，若为空代表没有
	    getMountedVolumes "directory" "$2" "$3"
        return $?
        ;;
	-ld)
	    #根据设备名返回LV逻辑卷
	    printLogicalVolumeInfo "$2"
        return $?
        ;;
	-mount)
	    #挂载LV卷
	    mountSnapshot "$2" "$3"
        return $?
        ;;
    -lvpath)
        #根据volumeName获取lvPath: vg/lv
        getLvmPath "$2"
        return $?
        ;;
	-csnapid)
        #改变snapshot uuid
        changeSnapshotVolumeID "$2"
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
