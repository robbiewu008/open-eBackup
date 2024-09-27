#!/bin/sh
set +x
sysName=`uname -s`
AGENT_INSTALL_PATH=${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient
UPGRADE_PACKAGE_PATH=${DATA_BACKUP_AGENT_HOME}/upgrade
PRODUCT_NAME=DataProtect
SHELL_TYPE_SH="/bin/sh"
BACKUP_ROLE_SANCLIENT_PLUGIN=5
SANCLIENT_USER=sanclient
CURRENT_PATH=`dirname $0` && cd $CURRENT_PATH && CURRENT_PATH=`pwd`
TESTCFG_BACK_ROLE=`su - sanclient 2>/dev/null -s ${SHELL_TYPE_SH} -c "sed '/^BACKUP_ROLE=/!d;s/.*=//' ${CURRENT_PATH}/../conf/testcfg.tmp "`
if [ "${TESTCFG_BACK_ROLE}" = "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ] ; then
    AGENT_INSTALL_PATH=${DATA_BACKUP_SANCLIENT_HOME}/DataBackup/SanClient
    UPGRADE_PACKAGE_PATH=${DATA_BACKUP_SANCLIENT_HOME}/upgradeSanclient
fi
AGENT_UPGRADE_INFOFILE_PATH=${AGENT_INSTALL_PATH}/ProtectClient-E/tmp
AGENT_ROOT_PATH=${AGENT_INSTALL_PATH}/ProtectClient-E
LOG_FILE_NAME=${AGENT_ROOT_PATH}/slog/upgrade_pre.log
PUBLIC_KEY_FILE=${AGENT_ROOT_PATH}/upgrade/upgrade_public_key.pem
SIGNATURE_FILE=${AGENT_ROOT_PATH}/stmp/upgrade_signature.sign
AGENT_BIN_PATH=${AGENT_ROOT_PATH}/bin

. "${AGENT_ROOT_PATH}/sbin/agent_sbin_func.sh"

##### upgrade errcode start #####
ERR_PKG_NOT_COMPLETE=84
ERR_UNZIP_PKG=85
##### upgrade errcode end #####

read SHA256_VALUE
########################################################################################
# Function Definition
LogError()
{
    #$1=errorInfo,$2=errorCode, $3=errorDetailParam
    LogErr "$1" "$2" "job_log_agent_storage_update_prepare_fail_label" "$3"
}

# Check the uniqueness of the upgrade package
CheckPacUnique()
{
    cd ${AGENT_UPGRADE_INFOFILE_PATH}
    UPGRADE_PACKAGE_COUNT=`ls -l | grep ${PRODUCT_NAME} | wc -l`
    if [ ${UPGRADE_PACKAGE_COUNT} -ne 1 ]; then
        Log "Check package uniqueness unsuccessfully, UPGRADE_PACKAGE_COUNT=[$UPGRADE_PACKAGE_COUNT]."
        return 1
    else
        UPGRADE_PACKAGE_NAME=`ls -l | grep ${PRODUCT_NAME} | $MYAWK '{print $NF}'`
        Log "Check package uniqueness successfully."
        return 0
    fi
}

# Verify the value of the sha256
CheckShaValue()
{
    AGENT_UPGRADE_INFOFILE=${AGENT_UPGRADE_INFOFILE_PATH}/tmpUpgradeInfo
    AGENT_UPGRADE_ZIPFILE=${AGENT_UPGRADE_INFOFILE_PATH}/${UPGRADE_PACKAGE_NAME}
    if [ ! -f "${AGENT_UPGRADE_INFOFILE}" ]; then
        Log "Not find the upgrade infomation file [${AGENT_UPGRADE_INFOFILE_PATH}/tmpUpgradeInfo]."
        return 1
    fi
    # unix dont support sha256sum a file
    if [ ${sysName} = "AIX" ] || [ ${sysName} = "HP-UX" ] || [ ${sysName} = "SunOS" ]; then
        return 0
    fi

    # return if sha256 command not exists
    command -v sha256sum >/dev/null 2>&1
    if [ $? -ne 0 ]; then
        Log "Command sha256sum is not supported."
        return 0
    fi

    # verify the consistence of the sha256 value
    sha256sum ${AGENT_UPGRADE_ZIPFILE} | $MYAWK -F " " '{print $1}' | grep ${SHA256_VALUE} >/dev/null 2>&1
    if [ $? -eq 0 ]; then
        Log "Check sha256 successfully, original sha256 value [${SHA256_VALUE}]."
        return 0
    else
        Log "Check sha256 unsuccessfully, original sha256 value [${SHA256_VALUE}]."
        return 1
    fi
}

UnzipUpgradeFile()
{
    cd ${UPGRADE_PACKAGE_PATH}
    mv ${AGENT_UPGRADE_ZIPFILE} ${UPGRADE_PACKAGE_PATH}/
    unzip -q ${UPGRADE_PACKAGE_PATH}/${UPGRADE_PACKAGE_NAME} >/dev/null 2>&1
    return $?
}

VerifySignature()
{
    Log "Start verify upgrade pkg signature."
    # printf <sha256值> | openssl dgst -sha256 -verify <公钥文件> -signature <签名文件>
    printf ${SHA256_VALUE} | ${AGENT_BIN_PATH}/openssl dgst -sha256 -verify ${PUBLIC_KEY_FILE} -signature ${SIGNATURE_FILE} >> $LOG_FILE_NAME 2>&1
    if [ $? -eq 0 ]; then
        Log "Verify upgrade file signature successfully."
        CleanSignatureFile
        return 0
    else
        Log "Verify upgrade file signature unsuccessfully."
        CleanSignatureFile
        return 1
    fi
}
 
CleanSignatureFile()
{
    # stmp目录rdadmin用户无法创建文件，所以先删除，再创建空签名文件，下次升级rdadmin直接写入签名内容
    rm -rf ${SIGNATURE_FILE}
    touch ${SIGNATURE_FILE}
    chmod 660 ${SIGNATURE_FILE}
    chown root:${AGENT_USER} ${SIGNATURE_FILE}
}

#################################################################################
##  main process
#################################################################################
Log "Upgrade prepare begin."
CheckPacUnique
if [ $? -ne 0 ]; then
    LogError "Check pac unique unsuccessfully." ${ERR_UPGRADE_FAIL_VERIFICATE_PARAMETER}
    exit 1
fi

CheckShaValue
if [ $? -ne 0 ]; then
    LogError "Check sha256 value unsuccessfully." ${ERR_UPGRADE_FAIL_VERIFICATE_PARAMETER}
    exit 1
fi

VerifySignature
if [ $? -ne 0 ]; then
    LogError "Verify upgrade file signature unsuccessfully." ${ERR_UPGRADE_FAIL_VERIFICATE_PARAMETER}
    exit 1
fi

UnzipUpgradeFile
if [ $? -ne 0 ]; then
    LogError "Unzip upgrade file unsuccessfully." ${ERR_UPGRADE_FAIL_VERIFICATE_PARAMETER}
    exit 1
fi

Log "Upgrade prepare successfully."
exit 0
