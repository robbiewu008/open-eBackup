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
    echo "$(date +"%Y-%m-%d %H:%M:%S.%N") $0:${BASH_LINENO} mount_oper.sh.info. $1"
}

function log_error() {
    echo "$(date +"%Y-%m-%d %H:%M:%S.%N") $0:${BASH_LINENO} mount_oper.sh.error. $1"
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

    if [[ "${path}" =~ '..' ]]; then
        log_error "The path(${path}) cannot contain special characters(..)."
        return 1
    fi
    if [[ "${path}" =~ '.*.' ]]; then
        log_error "The path(${path}) cannot contain special characters(.*.)."
        return 1
    fi
    return 0
}

function check_path_mount() {
    local path="$1"

    check_path_common "${path}"
    if [ $? -ne 0 ]; then
        return 1
    fi

    if [[ ! "${path}" =~ ^/mnt/ ]]; then
        log_error "The mount point(${path}) path must start with '/mnt/'."
        return 1
    fi
    return 0
}

function mount_nfs() {
    #params: mount_point, src
    log_info "Begin mount."
    local mount_point="$1"
    local mount_src="$2"

    check_path_common "${mount_src}"
    if [ $? -ne 0 ]; then
        return 1
    fi

    check_path_mount "${mount_point}"
    if [ $? -ne 0 ]; then
        return 1
    fi

    # Modify this log carefully!!! We catch mount_point from this log in python process.
    log_info "mount_point is: ${mount_point}. Line:$LINENO"

    mkdir -p "${mount_point}"

    mount | grep -q "${mount_src} on ${mount_point}"
    if [ $? -eq 0 ]; then
        log_error "Mount point: ${mount_point} has been mounted."
        return 1
    fi

    log_info "Mount start. mount "${mount_src}" to "${mount_point}"."
    local L_RESULT
    local C_RESULT
    L_RESULT=$(mount -v -t nfs -o noexec -o nosuid -o retry=1,retrans=5,soft,nolock,timeo=30 "${mount_src}" "${mount_point}" 2>&1)
    if [ $? -ne 0 ]; then
        C_RESULT=${L_RESULT//[$'\t\r\n']/ } && C_RESULT=${C_RESULT%%*( )}
        log_error "Mount error: ${C_RESULT}"
        return 1
    fi
    C_RESULT=${L_RESULT//[$'\t\r\n']/ } && C_RESULT=${C_RESULT%%*( )}
    log_info "Mount success: ${C_RESULT}"
    return 0
}

function mount_fuse() {
    #params: mount_point, source_id, osad_ip_list, osad_auth_port, osad_server_port
    log_info "Begin mount."
    local mount_point="$1"
    local source_id="$2"
    local osad_ip_list="$3"
    local osad_auth_port="$4"
    local osad_server_port="$5"


    check_path_common "${mount_point}"
    if [ $? -ne 0 ]; then
        return 1
    fi

    # Modify this log carefully!!! We catch mount_point from this log in python process.
    log_info "mount_point is: ${mount_point}. Line:$LINENO"

    mkdir -p "${mount_point}"

    mount | grep -q "on ${mount_point}"
    if [ $? -eq 0 ]; then
        log_error "Mount point: ${mount_point} has been mounted."
        return 1
    fi

    log_info "Mount start. mount point "${mount_point}"."
    local L_RESULT
    local C_RESULT
    export LD_LIBRARY_PATH=/opt/FileClient/lib:/opt/FileClient/lib/3rd
    L_RESULT=$(/opt/FileClient/bin/file_admin_client --add --mount_point="${mount_point}"\
        --source_id="${source_id}" --osad_ip_list="${osad_ip_list}" --osad_auth_port="${osad_auth_port}"\
        --osad_server_port="${osad_server_port}" 2>&1)
    if [ $? -ne 0 ]; then
        C_RESULT=${L_RESULT//[$'\t\r\n']/ } && C_RESULT=${C_RESULT%%*( )}
        log_error "Mount error: ${C_RESULT}"
        return 1
    fi
    C_RESULT=${L_RESULT//[$'\t\r\n']/ } && C_RESULT=${C_RESULT%%*( )}
    log_info "Mount success: ${C_RESULT}"
    return 0
}

function umount_fuse() {
    local path="$1"
    log_info "Begin umount fuse point $1."

    check_path_mount "${path}"
    if [ $? -ne 0 ]; then
        return 1
    fi

    local L_RESULT
    export LD_LIBRARY_PATH=/opt/FileClient/lib:/opt/FileClient/lib/3rd
    L_RESULT=$(/opt/FileClient/bin/file_admin_client --remove --mount_point="${path}" 2>&1)
    if [ $? -ne 0 ]; then
        if [[ "${L_RESULT}" =~ "not mounted" ]]; then
            log_info "Mount point: $1 not mounted."
            return 0
        fi
        local C_RESULT
        C_RESULT=${L_RESULT//[$'\t\r\n']/ } && C_RESULT=${C_RESULT%%*( )}
        log_error "Umount $1 error: ${C_RESULT}"
        return 1
    fi
    fcnt=`ls -A $1 | wc -l`
    [ -d $1 ] && [ $fcnt -eq 0 ] && rm -rf $1
    log_info "Umount $1 success."
    return 0
}

function umount_volume() {
    local path="$1"
    log_info "Begin umount nfs point $1."

    check_path_mount "${path}"
    if [ $? -ne 0 ]; then
        return 1
    fi

    local L_RESULT
    L_RESULT=$(umount -f "${path}" 2>&1)
    if [ $? -ne 0 ]; then
        if [[ "${L_RESULT}" =~ "not mounted" ]]; then
            log_info "Mount point: $1 not mounted."
            return 0
        fi
        local C_RESULT
        C_RESULT=${L_RESULT//[$'\t\r\n']/ } && C_RESULT=${C_RESULT%%*( )}
        log_error "Umount $1 error: ${C_RESULT}"
        return 1
    fi
    fcnt=`ls -A $1 | wc -l`
    [ -d $1 ] && [ $fcnt -eq 0 ] && rm -rf $1
    log_info "Umount $1 success."
    return 0
}

function rm_dir() {
    local path="$1"
    log_info "Begin delete dir $1."

    check_path_mount "${path}"
    if [ $? -ne 0 ]; then
        log_error "Delete path error, ${path}."
        return 1
    fi
    rm -rf "${path}"
}

function clear_path() {
    local path="$1"
    log_info "Begin clear path $1."

    check_path_mount "${path}"
    if [ $? -ne 0 ]; then
        log_error "Clear path error, ${path}."
        return 1
    fi
    rm -rf "${path}"/*
}

function read_data() {
    local path="$1"
    check_path_mount "$path"
    if [ $? -ne 0 ]; then
        log_error "File path error, ${path}."
        return 1
    fi
    if [ ! -f $path ]; then
      log_error "File path not exists, ${path}."
      return 1
    fi
    data=$(cat $path)
    echo "${data}"
}

function over_write_data() {
    local path="$1"
    check_path_mount "$path"
    if [ $? -ne 0 ]; then
        log_error "File path error, ${path}."
        return 1
    fi
    if [[ "$3" == "no_next" ]]; then
        echo -n "$2">$path
    else
        echo "$2">$path
    fi

}

function append_write_data() {
    local path="$1"
    check_path_mount "$path"
    if [ $? -ne 0 ]; then
        log_error "File path error, ${path}."
        return 1
    fi
    if [[ "$3" == "no_next" ]]; then
        echo -n "$2">>$path
    else
        echo "$2">>$path
    fi
}

function create_path() {
    local path="$1"
    check_path_mount "$path"
    if [ $? -ne 0 ]; then
        log_error "Dir path error, ${path}."
        return 1
    fi
    if [ ! -d "$path" ]; then
        mkdir -p $path
    fi
}

function check_file_exist() {
    local path="$1"
    if [ ! -f $path ]; then
      log_error "File path not exists, ${path}."
      return 1
    fi
}

function is_directory_sudo() {
    local path="$1"
    if [ -d "$path" ]; then
        log_info "${path} is a directory"
    else
        log_error "${path} is not a directory"
        return 1
    fi
}

function is_mount_sudo() {
    local path="$1"
    if mountpoint -q "$path"; then
        log_info "${path} is a mount point"
    else
        log_error "${path} is not a mount point"
        return 1
fi
}

function main() {
    if [ $(whoami) != "root" ]; then
        log_error "Error:Current operation must perform by root account."
        return 1
    fi
    local L_OPERATION="$1"
    case "${L_OPERATION}" in
    mount_nfs)
        #params: mount_point, mount_src
        mount_nfs "$2" "$3"
        return $?
        ;;
    mount_fuse)
        #params: mount_point, mount_src
        mount_fuse "$2" "$3" "$4" "$5" "$6"
        return $?
        ;;
    umount_fuse)
        umount_fuse "$2"
        return $?
        ;;
    umount)
        umount_volume "$2"
        return $?
        ;;
    rm)
        rm_dir "$2"
        return $?
        ;;
    create_path)
        create_path "$2"
        return $?
        ;;
    clear_path)
        clear_path "$2"
        return $?
        ;;
    read)
        read_data "$2"
        return $?
        ;;
    over_write)
        over_write_data "$2" "$3" "$4"
        return $?
        ;;
    append_write)
        append_write_data "$2" "$3" "$4"
        return $?
        ;;
    check_file)
        check_file_exist "$2"
        return $?
        ;;
    is_directory)
        is_directory_sudo "$2"
        return $?
        ;;
    is_mount)
        is_mount_sudo "$2"
        return $?
        ;;
    *)
        log_error "Error:Invalid parameter."
        return 1
        ;;
    esac
    return 0

}

main "$@"
exit $?
