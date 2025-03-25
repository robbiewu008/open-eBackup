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
if [ -z "${DATA_BACKUP_AGENT_HOME}" ]; then
    DATA_BACKUP_AGENT_HOME="/opt"
fi
AGENT_INSTALL_PATH=${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient
SANCLIENT_USER=sanclient
SHELL_TYPE_SH="/bin/sh"
MODIFY_PACKAGE_PATH=${DATA_BACKUP_AGENT_HOME}/modify
PRODUCT_NAME=DataProtect
AGENT_ROOT_PATH=${AGENT_INSTALL_PATH}/ProtectClient-E
LOG_FILE_NAME=${AGENT_ROOT_PATH}/slog/modify_pre.log
umask 0022
. "${AGENT_ROOT_PATH}/sbin/agent_sbin_func.sh"

########################################################################################
# Function Definition
LogError()
{
    #$1=errorInfo,$2=errorCode, $3=errorDetailParam
    LogErr "$1" "$2" "job_log_agent_storage_update_prepare_fail_label" "$3"
}

# Check whether the modify script exists
CheckUpgradeScript()
{
    cd ${MODIFY_PACKAGE_PATH}
    MODIFY_PACKAGE_NAME=`ls -l | grep ${PRODUCT_NAME} | grep '^d' | $MYAWK '{print $NF}'`
    cd ${MODIFY_PACKAGE_NAME}
    if [ -f "modify_plugins.sh" ]; then
        Log "The modify script [${MODIFY_PACKAGE_PATH}/${MODIFY_PACKAGE_NAME}/modify_plugins.sh] exists."
        chmod +x modify_plugins.sh
        return 0
    else
        Log "The modify script does not exist."
        return 1
    fi

}

#################################################################################
##  main process
#################################################################################
Log "Modify caller begin."

CheckUpgradeScript
if [ $? -ne 0 ]; then
    LogError "Upgrade caller execute unsuccessfully." ${ERR_MODIFY_FAIL_VERIFICATE_PARAMETER}
    exit 1
fi

# Move modify logs to the temporary directory.
cp -r ${LOG_FILE_NAME} ${MODIFY_PACKAGE_PATH}

# exec-mode to execute modify.sh to avoid the thread to be killed
Log "Begin to call modify script via exec mode."
exec ${MODIFY_PACKAGE_PATH}/${MODIFY_PACKAGE_NAME}/modify_plugins.sh -mode push

exit 0
