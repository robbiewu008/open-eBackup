#!/bin/sh
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
set +x

source "/etc/profile"

AGENT_INSTALL_PATH=${DATA_BACKUP_AGENT_HOME}
 
if [ -z "${AGENT_INSTALL_PATH}" ]; then
    AGENT_INSTALL_PATH="/opt"
fi

if [ ! -d "${AGENT_INSTALL_PATH}" ]; then
    echo "ERROR" "Agent Install Path [${AGENT_INSTALL_PATH}] : No such file or directory."
    return 1
fi

# import log tool
VIRT_ROOT_PATH="${AGENT_INSTALL_PATH}/DataBackup/ProtectClient/Plugins/VirtualizationPlugin"
source "${VIRT_ROOT_PATH}/bin/superlog.sh"

G_EBACKUP_KEY=0xBB8
SUCCESS=0
NO_RESVERSE=1
NOT_SUPPORT_RESVERSE=2
REG_EBACKUP_KEY=3
REG_OTHER_KEY=4
ERROR=5

function print_log_error()
{
    log_error "$1"
    echo -e "\e[0;31;1mError: $1\e[0m"
}

function print_log_info()
{
    log_info "$1"
    echo "$1"
}

function print_usage()
{
    echo "Usage:"
    echo "$0 reg <diskDevicePath>"
    echo "$0 unreg <diskDevicePath>"
}

function reg_key_command()
{
    local l_rv=0
    log_info "running: sg_persist -n -o -I -S key -d"
    sg_persist -n -o -I -S ${G_EBACKUP_KEY} -d $1 >/dev/null 2>&1
    l_rv=$?
    log_info "end."
    return ${l_rv}
}

function unreg_key_command()
{
    local l_rv=0
    log_info "running: sg_persist -n -o -I -K key -S 0 -d"
    sg_persist -n -o -I -K ${G_EBACKUP_KEY} -S 0 -d $1 >/dev/null 2>&1
    l_rv=$?
    log_info "end."
    return ${l_rv}
}

function is_has_sg_persist()
{
    log_info "running: rpm -qi sg3_utils"
    local l_sg_persist_fullname=""
    local l_exist_pkg_name=$(rpm -qi sg3_utils 2>&1 | sed -n '1p' | awk -F":" '{print $2}' | awk '{print $1}')
    log_info "end."
    if [ "${l_exist_pkg_name}" == "sg3_utils" ]; then
        l_sg_persist_fullname=$(which sg_persist)
        if [ $? -ne ${SUCCESS} ]; then
            print_log_error "Cannot find sg_persist."
            return ${ERROR}
        fi
        
        if [ ! -x "${l_sg_persist_fullname}" ]; then
            print_log_error "Cannot execute sg_persist."
            return ${ERROR}
        fi
        return ${SUCCESS}
    fi
    print_log_error "sg3_utils package need to be installed."
    log_error "The package get now is ${l_exist_pkg_name}"
    return ${ERROR}
}

function is_support_key()
{
    local l_scsi_id="$1"
    echo "$l_scsi_id" | grep -E "[$]|[&]|[|]|[<]|[>]|[;]|[\`]|[!]|[\\]|(\.\.)" >/dev/null 2>&1
    if [ $? -eq 0 ]; then
        print_log_error "invalid params"
        return ${NO_RESVERSE}
    fi

    log_info "running: sg_persist -i -n -r -d"
    res=$(sg_persist -i -n -r -d $1 )
    l_no_reverse="there is NO reservation held"
    l_not_support_reverse="command not supported"
    l_has_registed_ebackup_key="Key=0xbb8"
    l_has_registed_other_key="scope: LU_SCOPE,  type: Write Exclusive, registrants only"
    if [[ ${res} = *${l_no_reverse}* ]]; then
        log_info "there is NO reservation held"
        return ${NO_RESVERSE}
    elif [[ ${res} = *${l_not_support_reverse}* ]]; then
        log_info "sg_persist command not supported registed key"
        return ${NOT_SUPPORT_RESVERSE}
    elif [[ ${res} = *${l_has_registed_ebackup_key}* ]]; then
        log_info "the key 0xbb8 has been registed"
        return ${REG_EBACKUP_KEY}
    elif [[ ${res} = *${l_has_registed_other_key}* ]]; then
        log_info "the other key has been registed"
        return ${REG_OTHER_KEY}
    else 
        log_error "There is an error occure in support key,result is ${res}"
        return ${ERROR}
    fi
    
}

function reg_key()
{
    log_info "Begin to run: reg_key"

    is_has_sg_persist
    l_rv=$?
    if [ ${l_rv} -ne ${SUCCESS} ]; then
        return ${l_rv} 
    fi
    
    is_support_key $1
    l_rv=$?
    if [ ${l_rv} -eq ${REG_EBACKUP_KEY} -o ${l_rv} -eq ${REG_OTHER_KEY} ]; then
        reg_key_command $1 
        l_res=$?
        return ${l_res}
    elif [ ${l_rv} -eq ${NO_RESVERSE} -o ${l_rv} -eq ${NOT_SUPPORT_RESVERSE} ]; then
        return ${SUCCESS}
    else
        return ${l_rv}
    fi
}

function unreg_key()
{
    log_info "Begin to run: unreg_key"

    if [ ! -e $1 ]; then
        return ${SUCCESS}
    fi
    is_has_sg_persist
    l_rv=$?
    if [ ${l_rv} -ne ${SUCCESS} ]; then
        return ${l_rv} 
    fi

    is_support_key $1
    l_rv=$?
    if [ ${l_rv} -eq ${REG_EBACKUP_KEY} ]; then
        unreg_key_command $1 
        l_res=$?
        return ${l_res}
    elif [ ${l_rv} -eq ${NO_RESVERSE} -o ${l_rv} -eq ${NOT_SUPPORT_RESVERSE} -o ${l_rv} -eq ${REG_OTHER_KEY} ]; then
        return ${SUCCESS}
    else
        return ${l_rv}
    fi
}

function main()
{
    verify_special_char "$@"

    if [ $# -ne 2 ]; then
        print_usage
        exit 1
    fi

    case "$1" in
        reg)
            reg_key $2
            return $?
            ;;
        unreg)
            unreg_key $2
            return $?
            ;;
        *)
            print_usage
            return 1
            ;;
    esac
}

main $@
exit $?
