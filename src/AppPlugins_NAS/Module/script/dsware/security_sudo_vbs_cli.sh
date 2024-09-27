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

G_VBS_CLI="/bin/vbs_cli"
VIRT_ROOT_PATH="${AGENT_INSTALL_PATH}/DataBackup/ProtectClient/Plugins/VirtualizationPlugin"
source "${VIRT_ROOT_PATH}/bin/superlog.sh"

function main()
{
    verify_special_char "$@"

    if [ ! -f "${G_VBS_CLI}" ]; then
        echo "vbs_cli is not found."
        exit 1
    fi

    if [ -L "${G_VBS_CLI}" ]; then
        echo "The file[${G_VBS_CLI}] is a symbol link."
        exit 1
    fi

    "$G_VBS_CLI" "$@"
}

main "$@"
exit $?
