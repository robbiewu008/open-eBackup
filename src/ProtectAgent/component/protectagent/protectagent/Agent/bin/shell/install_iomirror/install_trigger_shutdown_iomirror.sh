#!/bin/sh
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
set +x
AGENT_ROOT_PATH=$1
LOG_FILE_NAME=${AGENT_ROOT_PATH}/log/install_trigger_shutdown_iomirror.log
. "${AGENT_ROOT_PATH}/sbin/agent_sbin_func.sh"

SYS_VERSION=$(getOSVersion)

Log "Begin to register shutdown iomirror."

if [ "${SYS_VERSION}" = "REDHAT_6.5" ]
then
    Log "make redhat6.5 link file"
    touch /etc/rc.d/init.d/shutdown2iomirror
    
    # touch new lockfile for next reboot
    echo "touch /var/lock/subsys/shutdown2iomirror" > /etc/rc.d/init.d/shutdown2iomirror
    echo "${AGENT_ROOT_PATH}/bin/shutdown2iomirror > /boot/shutdown2iomirror.log" >> /etc/rc.d/init.d/shutdown2iomirror
    CHMOD +x /etc/rc.d/init.d/shutdown2iomirror
    
    #shutdowm
    ln -s /etc/init.d/shutdown2iomirror /etc/rc0.d/K20shutdown2iomirror
    #reboot
    ln -s /etc/init.d/shutdown2iomirror /etc/rc6.d/K20shutdown2iomirror
    
    mkdir -p /var/lock/subsys
    touch /var/lock/subsys/shutdown2iomirror
elif [ "${SYS_VERSION}" = "CENTOS_7.2.1511" ]
then
    Log "make centos7.2 service"
    systemctl disable shutdown_save_bitmap 2>/dev/null
    touch /etc/rc.d/init.d/shutdown2iomirror.sh
    echo "#!/bin/bash" >> /etc/rc.d/init.d/shutdown2iomirror.sh
    echo "${AGENT_ROOT_PATH}/bin/shutdown2iomirror > /boot/shutdown2iomirror.log" >> /etc/rc.d/init.d/shutdown2iomirror.sh
    echo "exit 0" >> /etc/rc.d/init.d/shutdown2iomirror.sh
    CHMOD +x /etc/rc.d/init.d/shutdown2iomirror.sh
    
    cp -f ${AGENT_ROOT_PATH}/conf/driver/shutdown_save_bitmap.service /etc/systemd/system/
    CHMOD +x /etc/systemd/system/shutdown_save_bitmap.service
    systemctl enable shutdown_save_bitmap
else
    Log "Unsupported OS ${SYS_VERSION}."
    exit 1
fi

Log "Finish registering shutdown iomirror."
exit 0
