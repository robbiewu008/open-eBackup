#!/bin/sh
set +x

AGENT_INSTALL_PATH=${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient
SANCLIENT_USER=sanclient
SHELL_TYPE_SH="/bin/sh"
BACKUP_ROLE_SANCLIENT_PLUGIN=5
UPGRADE_PACKAGE_PATH=${DATA_BACKUP_AGENT_HOME}/upgrade
PRODUCT_NAME=DataProtect
CURRENT_PATH=`dirname $0` && cd $CURRENT_PATH && CURRENT_PATH=`pwd`
TESTCFG_BACK_ROLE=`su - sanclient 2>/dev/null -s ${SHELL_TYPE_SH} -c "sed '/^BACKUP_ROLE=/!d;s/.*=//' ${CURRENT_PATH}/../conf/testcfg.tmp "`
if [ "${TESTCFG_BACK_ROLE}" = "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ] ; then
    AGENT_INSTALL_PATH=${DATA_BACKUP_SANCLIENT_HOME}/DataBackup/SanClient
    UPGRADE_PACKAGE_PATH=${DATA_BACKUP_SANCLIENT_HOME}/upgradeSanclient
fi
AGENT_ROOT_PATH=${AGENT_INSTALL_PATH}/ProtectClient-E
LOG_FILE_NAME=${AGENT_ROOT_PATH}/slog/upgrade_pre.log

. "${AGENT_ROOT_PATH}/sbin/agent_sbin_func.sh"

########################################################################################
# Function Definition
LogError()
{
    #$1=errorInfo,$2=errorCode, $3=errorDetailParam
    LogErr "$1" "$2" "job_log_agent_storage_update_prepare_fail_label" "$3"
}

# Check whether the upgrade script exists
CheckUpgradeScript()
{
    cd ${UPGRADE_PACKAGE_PATH}
    UPGRADE_PACKAGE_NAME=`ls -l | grep ${PRODUCT_NAME} | grep '^d' | $MYAWK '{print $NF}'`
    cd ${UPGRADE_PACKAGE_NAME}
    if [ -f "upgrade.sh" ]; then
        Log "The upgrade script [${UPGRADE_PACKAGE_PATH}/${UPGRADE_PACKAGE_NAME}/upgrade.sh] exists."
        chmod +x upgrade.sh
        return 0
    else
        Log "The upgrade script does not exist."
        return 1
    fi

}

#################################################################################
##  main process
#################################################################################
Log "Upgrade caller begin."

CheckUpgradeScript
if [ $? -ne 0 ]; then
    LogError "Upgrade caller execute unsuccessfully." ${ERR_UPGRADE_FAIL_VERIFICATE_PARAMETER}
    exit 1
fi

# Move upgrade logs to the temporary directory.
cp -r ${LOG_FILE_NAME} ${UPGRADE_PACKAGE_PATH}

# exec-mode to execute upgrade.sh to avoid the thread to be killed
Log "Begin to call upgrade script via exec mode."
exec ${UPGRADE_PACKAGE_PATH}/${UPGRADE_PACKAGE_NAME}/upgrade.sh -mode push

exit 0
