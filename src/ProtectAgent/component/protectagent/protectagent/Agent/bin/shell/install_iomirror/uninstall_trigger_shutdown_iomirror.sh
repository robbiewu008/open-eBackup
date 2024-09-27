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
set +x
AGENT_ROOT_PATH=$1
LOG_FILE_NAME=${AGENT_ROOT_PATH}/log/uninstall_trigger_shutdown_iomirror.log
. "${AGENT_ROOT_PATH}/sbin/agent_sbin_func.sh"

SYS_VERSION=$(getOSVersion)

Log "Begin to unregister shutdown iomirror."

if [ "${SYS_VERSION}" = "REDHAT_6.5" ]
then
    Log "delete redhat6.5 link file"
    #shutdowm
    DeleteFile /etc/rc0.d/K20shutdown2iomirror /etc/rc6.d/K20shutdown2iomirror
    DeleteFile /var/lock/subsys/shutdown2iomirror
    
    DeleteFile /etc/rc.d/init.d/shutdown2iomirror
elif [ "${SYS_VERSION}" = "CENTOS_7.2.1511" ]
then
    Log "delete centos7.2 service"
    systemctl disable shutdown_save_bitmap
    DeleteFile /etc/systemd/system/shutdown_save_bitmap.service
    DeleteFile /etc/rc.d/init.d/shutdown2iomirror.sh
else
    Log "Unsupported OS type ${SYS_VERSION}."
    exit 1
fi

Log "Finish unregistering shutdown iomirror."
exit 0
