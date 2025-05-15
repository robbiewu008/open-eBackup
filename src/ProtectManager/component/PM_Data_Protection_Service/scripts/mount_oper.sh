#!/bin/bash
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
function log_info() {
    echo "$(date +"[%Y-%m-%d %H:%M:%S,%N]") [$0:${BASH_LINENO}] mount_oper.sh.info: $1"
}
function log_error() {
    echo "$(date +"[%Y-%m-%d %H:%M:%S,%N]") [$0:${BASH_LINENO}] mount_oper.sh.error $1"
}
whitelist=(
"^/opt/OceanProtect/logs/.*/protectmanager/$"
"^/context/src/logs$"
)

check_whitelist() {
  local name=$1
  # 校验文件是否存在软连接
  if [[ -L $name ]]; then
    log_error "ERROR: ${name} file or folder has a soft link."
    return 1
  fi
  for wdir in "${whitelist[@]}"; do
    if [[ $name =~ $wdir ]]; then
      return 0
    fi
  done
  return 1
}

function check_path_common() {
    local path="$1"
    if [ -z "${path}" ]; then
        log_error "No path(${path}) specified."
        return 1
    fi
    filepat='[|;&$><`\!]+'
    if [[ "${path}" =~ "${filepat}" ]]; then
        log_error "The path(${path}) cannot contain special characters(${filepat})."
        return 1
    fi
    if [[ "${path}" =~ '..' ]] || [[ "${path}" =~ '.*.' ]]; then
        log_error "The path(${path}) cannot contain special characters(.. or .*.)."
        return 1
    fi
    return 0
}
mount_bind() {
    log_info "Begin mount."
    local mount_src="$1"
    local mount_target="$2"
    check_path_common "${mount_src}"
    if [ $? -ne 0 ]; then
        echo "ERROR: check_path_common ${mount_src} failed!"
        return 1
    fi
    check_path_common "${mount_target}"
    if [ $? -ne 0 ]; then
        echo "ERROR: check_path_common ${mount_src} failed!"
        return 1
    fi
    # white list
    check_whitelist "${mount_src}"
    if [ $? -ne 0 ]; then
        echo "ERROR: ${mount_src} not in whitelist!"
        return 1
    fi
    check_whitelist "${mount_target}"
    if [ $? -ne 0 ]; then
        echo "ERROR: ${mount_target} not in whitelist!"
        return 1
    fi
    log_info "Mount start. mount bind "${mount_src}" to "${mount_target}"."
    mount -o nosuid,noexec --bind "$mount_src" "$mount_target"
    if [ $? -ne 0 ]; then
        echo "ERROR: mount --bind ${mount_src} ${mount_target} failed!"
        return 1
    fi
}
main() {
  #eg: mount_oper.sh mount_bind /nas/path/abc /usr/local/ef
    if [ $(whoami) != "root" ]; then
        log_error "Error:Current operation must perform by root account."
        return 1
    fi
    local L_OPERATION="$1"
    case "${L_OPERATION}" in
    mount_bind)
        #params: mount_point, mount_src
        mount_bind "${2}" "${3}"
        return $?
        ;;
    *)
        log_error "Error:Invalid parameter."
        return 1
        ;;
    esac
    return 0
}
main $@
exit $?
