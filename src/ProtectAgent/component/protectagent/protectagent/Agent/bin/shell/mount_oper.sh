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
# internal agent use only
set +x

function LogInfo() {
    echo "$(date +"[%Y-%m-%d %H:%M:%S,%N]") mount_oper.sh.info: $1"
}

function LogError() {
    echo "$(date +"[%Y-%m-%d %H:%M:%S,%N]") mount_oper.sh.error $1"
}

MOUNT_SRC_WHITE_LIST=(
    "^/opt/protectagent/.*/config/HostSN/HostSN$"
    "^/opt/logpath/logs/.*/protectengine-e/agent/log$"
    "^/opt/logpath/logs/.*/protectengine-e/agent/slog$"
    "^/mnt/protectagent/nginx$"
    "^/opt/logpath/logs/.*/protectengine-e/agent/nginx/logs$"
    "^/opt/protectagent/.*/config/nginx/conf$"
    "^/opt/protectagent/.*/config/db$"
    "^/opt/protectagent/.*/config/conf$"
    "^/mnt/protectagent/tmp$"
    "^/mnt/protectagent/stmp$"
    "^/mnt/protectagent/hosts$"
    "^/mnt/protectagent/resolv.conf$"
    "^/opt/protectagent/.*/Plugins/NasPlugin/conf$"
    "^/opt/protectagent/.*/Plugins/NasPlugin/plugin_attribute_1.0.0.json$"
    "^/opt/protectagent/.*/Plugins/VirtualizationPlugin/conf$"
    "^/opt/protectagent/.*/Plugins/GeneralDBPlugin/conf$"
    "^/opt/protectagent/.*/Plugins/GeneralDBPlugin/tmp$"
    "^/opt/protectagent/.*/Plugins/GeneralDBPlugin/stmp$"
    "^/opt/protectagent/.*/Plugins/Block_Service/conf$"
    "^/opt/protectagent/.*/Plugins/Block_Service/tmp$"
    "^/opt/protectagent/.*/Plugins/Block_Service/stmp$"
    "^/opt/protectagent/.*/Plugins/ObsPlugin/conf$"
    "^/opt/protectagent/.*/Plugins/ObsPlugin/tmp$"
    "^/opt/protectagent/.*/Plugins/ObsPlugin/stmp$"
)

MOUNT_TARGET_WHITE_LIST=(
    "^/etc/HostSN/HostSN$"
    "^/opt/DataBackup/ProtectClient/ProtectClient-E/log$"
    "^/opt/DataBackup/ProtectClient/ProtectClient-E/slog$"
    "^/opt/DataBackup/ProtectClient/ProtectClient-E/nginx$"
    "^/opt/DataBackup/ProtectClient/ProtectClient-E/nginx/logs$"
    "^/opt/DataBackup/ProtectClient/ProtectClient-E/nginx/conf$"
    "^/opt/DataBackup/ProtectClient/ProtectClient-E/db$"
    "^/opt/DataBackup/ProtectClient/ProtectClient-E/conf$"
    "^/opt/DataBackup/ProtectClient/ProtectClient-E/tmp$"
    "^/opt/DataBackup/ProtectClient/ProtectClient-E/stmp$"
    "^/etc/hosts$"
    "^/etc/resolv.conf$"
    "^/opt/DataBackup/ProtectClient/Plugins/NasPlugin/conf$"
    "^/opt/DataBackup/ProtectClient/Plugins/NasPlugin/plugin_attribute_1.0.0.json$"
    "^/opt/DataBackup/ProtectClient/Plugins/VirtualizationPlugin/conf$"
    "^/opt/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/conf$"
    "^/opt/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/tmp$"
    "^/opt/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/stmp$"
    "^/opt/DataBackup/ProtectClient/Plugins/Block_Service/conf$"
    "^/opt/DataBackup/ProtectClient/Plugins/Block_Service/tmp$"
    "^/opt/DataBackup/ProtectClient/Plugins/Block_Service/stmp$"
    "^/opt/DataBackup/ProtectClient/Plugins/ObsPlugin/conf$"
    "^/opt/DataBackup/ProtectClient/Plugins/ObsPlugin/tmp$"
    "^/opt/DataBackup/ProtectClient/Plugins/ObsPlugin/stmp$"
)

function GetPathInWhiteListIndex() {
    local path="$1"
    local realPath=`realpath ${path}`
    local flag="$2"
    local whiteList
    local index=0
    if [ "${flag}" = "mount_src" ]; then
        whiteList=${MOUNT_SRC_WHITE_LIST[*]}
    else
        whiteList=${MOUNT_TARGET_WHITE_LIST[*]}
    fi

    for whitePath in ${whiteList}; do
        if [[ "${realPath}" =~ $whitePath ]]; then
            return ${index}
        fi
        index=$((index+1))
    done
    return -1
}

function CheckSrcAndTargetIsMatch() {
    local mountSrc="$1"
    local mountTarget="$2"
    GetPathInWhiteListIndex "${mountSrc}" "mount_src"
    srcIndex=$?
    if [[ ${srcIndex} -eq -1 ]]; then
        LogError "${mountSrc} src path is not in white list!"
        return 1
    fi

    GetPathInWhiteListIndex "${mountTarget}" "mount_target"
    targetIndex=$?
    if [[ ${targetIndex} -eq -1 ]]; then
        LogError "${mountTarget} target path is not in white list!"
        return 1
    fi

    if [ ${targetIndex} -ne ${srcIndex} ]; then
        LogError "${mountTarget} and ${mountSrc} is not match in white list!"
        return 1
    fi
    return 0
}

function CheckPathCommon() {
    local path="$1"
    if [ -z "${path}" ]; then
        LogError "No path(${path}) specified."
        return 1
    fi

    filepat='[|;&$><`\!]+'
    if [[ "${path}" =~ "${filepat}" ]]; then
        LogError "The path(${path}) cannot contain special characters(${filepat})."
        return 1
    fi

    if [[ "${path}" =~ '..' ]]; then
        LogError "The path(${path}) cannot contain special characters(..)."
        return 1
    fi

    return 0
}

function MountBind() {
    LogInfo "Begin mount."
    local mountSrc="$1"
    local mountTarget="$2"

    # 1. Verification Path
    CheckPathCommon "${mountSrc}"
    if [ $? -ne 0 ]; then
        echo "ERROR: CheckPathCommon ${mountSrc} failed!"
        return 1
    fi

    CheckPathCommon "${mountTarget}"
    if [ $? -ne 0 ]; then
        LogError "ERROR: CheckPathCommon ${mountSrc} failed!"
        return 1
    fi

    # 2. white list
    local realPathSrc=`realpath ${mountSrc}`
    local realPathDst=`realpath ${mountTarget}`
    CheckSrcAndTargetIsMatch "${realPathSrc}" "${realPathDst}"
    if [ $? -ne 0 ]; then
        LogError "ERROR:  Check src and target is not match!"
        return 1
    fi

    # 3.mount --bind
    LogInfo "Mount start. mount bind ${realPathSrc} to ${realPathDst}."
    mount --bind $realPathSrc $realPathDst
    if [ $? -ne 0 ]; then
        LogError "ERROR: mount --bind ${realPathSrc} ${realPathDst} failed!"
        return 1
    fi
    LogInfo "Mount succ"
}

function MountIscsi() {
    LogInfo "Mount iscsi service dir start."
    mount -o remount,rw,nosuid,nodev,noexec,relatime -t sysfs sysfs /sys
    if [ $? -ne 0 ]; then
        LogError "ERROR: mount sys dir failed!"
        return 1
    fi
    mount -t devtmpfs devtmpfs /dev
    if [ $? -ne 0 ]; then
        LogError "ERROR: mount dev dir failed!"
        return 1
    fi
    mount -t devpts devpts /dev/pts
    if [ $? -ne 0 ]; then
        LogError "ERROR: mount devpts dir failed!"
        return 1
    fi
}

function MountCgroupDevice() {
    LogInfo "Mount cgourp device start."
    mkdir -p /mnt/inner_agent_cgroup
    if [ $? -ne 0 ]; then
        LogError "ERROR: create cgroup device dir(/mnt/inner_agent_cgroup) failed!"
        return 1
    fi
    mount -t cgroup -o devices devices /mnt/inner_agent_cgroup
    if [ $? -ne 0 ]; then
        LogError "ERROR: mount cgroup device to /mnt/inner_agent_cgroup failed!"
        return 1
    fi
    echo a > /mnt/inner_agent_cgroup/devices.allow
    if [ $? -ne 0 ]; then
        LogError "ERROR: add device allow list failed!"
        return 1
    fi
    umount /mnt/inner_agent_cgroup
    if [ $? -ne 0 ]; then
        LogError "WARN: umount cgroup device failed!"
    else
        rm -rf /mnt/inner_agent_cgroup
    fi
    LogInfo "Mount cgourp device success."
    return 0
}

function main() {
  #eg: mount_oper.sh mount_bind /nas/path/abc /usr/local/ef
    if [ $(whoami) != "root" ]; then
        LogError "Error:Current operation must perform by root account."
        return 1
    fi
    local L_OPERATION="$1"
    case "${L_OPERATION}" in
    mount_bind)
        #params: mount_point, mountSrc
        MountBind "${2}" "${3}"
        return $?
        ;;
    mount_iscsi)
        MountIscsi
        return $?
        ;;
    mount_cgroup_device)
        MountCgroupDevice
        return $?
        ;;
    *)
        LogError "Error:Invalid parameter."
        return 1
        ;;
    esac
    return 0
}

main $@
exit $?
