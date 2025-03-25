#!/bin/bash
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
# mkdir:: mkdir dir node-name
# chown:: chown option user dir node-name
# internal agent use only

DIR_WHITE_LIST=(
"^/opt/protectagent/.*/config/HostSN"
"^/opt/logpath/logs/.*/protectengine-e/agent$"
"^/opt/logpath/logs/.*/protectengine-e/agent/log$"
"^/opt/logpath/logs/.*/protectengine-e/agent/slog$"
"^/opt/logpath/logs/.*/protectengine-e/agent/log/Plugins/NasPlugin$"
"^/opt/logpath/logs/.*/protectengine-e/agent/log/Plugins/VirtualizationPlugin$"
"^/opt/logpath/logs/.*/protectengine-e/agent/slog/Plugins/NasPlugin$"
"^/opt/logpath/logs/.*/protectengine-e/agent/slog/Plugins/VirtualizationPlugin$"
"^/opt/logpath/logs/.*/protectengine-e/agent/slog/Plugins/GeneralDBPlugin$"
"^/opt/logpath/logs/.*/protectengine-e/agent/slog/Plugins/Block_Service$"
"^/opt/logpath/logs/.*/protectengine-e/agent/slog/Plugins/ObsPlugin$"
"^/opt/logpath/logs/.*/protectengine-e/agent/nginx$"
"^/opt/logpath/logs/.*/protectengine-e/agent/nginx/logs$"
"^/opt/DataBackup/ProtectClient/Plugins/VirtualizationPlugin$"
"^/opt/DataBackup/ProtectClient/Plugins/VirtualizationPlugin/conf$"
"^/opt/DataBackup/ProtectClient/ProtectClient-E/conf$"
"^/opt/DataBackup/ProtectClient/ProtectClient-E/nginx$"
"^/opt/DataBackup/ProtectClient/ProtectClient-E/log$"
"^/opt/protectagent/.*/config/nginx/conf$"
"^/opt/protectagent/.*/config/nginx$"
"^/opt/protectagent/.*/config/db$"
"^/opt/protectagent/.*/config/conf$"
"^/opt/protectagent/.*/config/log$"
"^/opt/protectagent/.*/config$"
"^/opt/protectagent/.*/Plugins$"
"^/opt/protectagent/.*"
"^/opt/protectagent"
"^/mnt/protectagent$"
"^/mnt/protectagent/tmp$"
"^/mnt/protectagent/stmp$"
"^/mnt/protectagent/nginx"
"^/mnt/protectagent/hosts$"
"^/mnt/protectagent/resolv.conf$"
"^/etc/iscsi/initiatorname.iscsi$"
"^/sys/module/scsi_mod/parameters/scan$"
)

function CheckDirWhiteList() {
    local name=$1
    for wdir in "${DIR_WHITE_LIST[@]}"
    do
        if [[ "$name" =~ $wdir ]]; then
            return 0
        fi
    done
    return 1
}

function check_command_injection()
{
    expression='[|;&$><`\!]+'
    if [[ "$1" =~ "${expression}" ]]; then
        echo "The param cannot contain special character(${expression})."
        return 1
    fi
    return 0
}

function CreateDir()
{
    # dirName node-name
    # 1. Parameter Verification
    if [ $# -ne 2 ]; then
        echo "ERROR(mkdir):The number of parameters is incorrect."
        exit 1
    fi

    # 2. mkdir dir
    local dirName="$2"
    check_command_injection "${dirName}"
    if [ $? -ne 0 ]; then
        echo "ERROR(chown): Modifying User fail, dirName[${dirName}] invalid."
        return 1
    fi

    CheckDirWhiteList "${dirName}"
    if [ $? -ne 0 ]; then
        echo "ERROR(mkdir): Create dir[${dirName}] not in whitelist."
        return 1
    fi
    mkdir -p ${dirName}
}

function ChangeOwner()
{
    # option user dir node-name
    local dirOption=
    local dirOwner=
    local dirUser=
    local dirGroup=
    local dirName=

    # 1. Parameter Verification
    # 1.1 number of parameters
    if [ $# -eq 3 ]; then
        dirOwner="$2"
        dirName="$3"
    elif [ $# -eq 4 ] && [ "$2" = "-R" ]; then
        dirOption="$2"
        dirOwner="$3"
        dirName="$4"
    else
        echo "ERROR(chown):The number of parameters is incorrect."
        exit 1
    fi

    # 1.2 user information contains special characters
    check_command_injection "${dirOwner}"
    if [ $? -ne 0 ]; then
        echo "ERROR(chown): Modifying User fail, dirOwner[${dirOwner}] invalid."
        return 1
    fi

    dirUser=${dirOwner%:*}
    dirGroup=${dirOwner#*:}
    # 1.3  user parameters is valid
    id ${dirUser} >/dev/null 2>&1
    if [ $? -ne 0 ]; then
        echo "ERROR(chown): Modifying User fail, dirUser[${dirUser}] invalid."
        return 1
    fi
    # 1.4  group parameters is valid
    groups  ${dirGroup} >/dev/null 2>&1
    if [ $? -ne 0 ]; then
        echo "ERROR(chown): Modifying User fail, dirGroup[${dirGroup}] invalid."
        return 1
    fi
    # 1.5 path is in the trustlist
    check_command_injection "${dirName}"
    if [ $? -ne 0 ]; then
        echo "ERROR(chown): Modifying User fail, dirName[${dirName}] invalid."
        return 1
    fi

    realPath=`realpath ${dirName}`
    CheckDirWhiteList "${realPath}"
    if [ $? -ne 0 ]; then
        echo "ERROR(chown): Modifying User fail, path[${realPath}] not in whitelist."
        return 1
    fi
    # 1.5 whether the path exists
    if [ ! -d "${realPath}" ] && [ ! -f "${realPath}" ]; then
        echo "ERROR(chown): Modifying User fail, path[${realPath}] does not exist."
        return 1
    fi
    # 2. chown dir
    chown -h ${dirOption} ${dirOwner} "${realPath}"
}

function ChangeMode()
{
    # option user dir node-name
    local modeOption=
    local dirMode=
    local dirName=

    # 1. Parameter Verification
    # 1.1 number of parameters
    if [ $# -eq 3 ]; then
        dirMode="$2"
        dirName="$3"
    elif [ $# -eq 4 ] && [ "$2" = "-R" ]; then
        modeOption="$2"
        dirMode="$3"
        dirName="$4"
    else
        echo "ERROR(chmod):The number of parameters is incorrect."
        exit 1
    fi

    # 1.2 permission is valid
    echo "${dirMode}" | grep -E '^[1-7][0-5]{2}$|^\+t$' >/dev/null 2>&1
    if [ $? -ne 0 ]; then
        echo "ERROR(chmod): The parameters of mode is incorrect"
        exit 1
    fi
    # 1.3 path is in the trustlist
    check_command_injection "${dirName}"
    if [ $? -ne 0 ]; then
        echo "ERROR(chown): Modifying User fail, dirName[${dirName}] invalid."
        return 1
    fi

    realPath=`realpath ${dirName}`
    CheckDirWhiteList "${realPath}"
    if [ $? -ne 0 ]; then
        echo "ERROR(chmod): Modifying User fail, dir[${realPath}] not in whitelist."
        return 1
    fi
    # 1.4 whether the path exists
    if [ ! -d "${realPath}" ] && [ ! -f "${realPath}" ]; then
        echo "ERROR(chown): Modifying User fail, path[${realPath}] does not exist."
        return 1
    fi
    # 2. chmod dir
    chmod ${modeOption} ${dirMode} "${realPath}"
}

type="$1"
if [ -z "${type}" ]; then
    echo "ERROR:Type error."
    exit 1
fi

if [ "${type}" = "mkdir" ]; then
    CreateDir $*
elif [ "${type}" = "chown" ]; then
    ChangeOwner $*
elif [ "${type}" = "chmod" ]; then
    ChangeMode $*
else
    echo "ERROR:Param is error."
    exit 1
fi
