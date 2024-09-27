#!/bin/sh
set +x
sysName=`uname -s`
if [ -z "${DATA_BACKUP_AGENT_HOME}" ]; then
    DATA_BACKUP_AGENT_HOME="/opt"
fi
AGENT_INSTALL_PATH=${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient
SANCLIENT_INSTALL_PATH="${DATA_BACKUP_AGENT_HOME}/DataBackup/SanClient"
MODIFY_PACKAGE_PATH=${DATA_BACKUP_AGENT_HOME}/modify
PRODUCT_NAME=DataProtect
SHELL_TYPE_SH="/bin/sh"

AGENT_MODIFY_INFOFILE_PATH=${AGENT_INSTALL_PATH}/ProtectClient-E/tmp
AGENT_ROOT_PATH=${AGENT_INSTALL_PATH}/ProtectClient-E
LOG_FILE_NAME=${AGENT_ROOT_PATH}/slog/modify_pre.log
PUBLIC_KEY_FILE=${AGENT_ROOT_PATH}/upgrade/upgrade_public_key.pem
SIGNATURE_FILE=${AGENT_ROOT_PATH}/stmp/upgrade_signature.sign
AGENT_BIN_PATH=${AGENT_ROOT_PATH}/bin

. "${AGENT_ROOT_PATH}/sbin/agent_sbin_func.sh"

##### modify errcode start #####
ERR_PKG_NOT_COMPLETE=84
ERR_UNZIP_PKG=85
##### modify errcode end #####

read SHA256_VALUE
########################################################################################
# Function Definition
LogError()
{
    #$1=errorInfo,$2=errorCode, $3=errorDetailParam
    LogErr "$1" "$2" "job_log_agent_storage_update_prepare_fail_label" "$3"
}

# Check the uniqueness of the modify package
CheckPacUnique()
{
    cd ${AGENT_MODIFY_INFOFILE_PATH}
    MODIFY_PACKAGE_COUNT=`ls -l | grep ${PRODUCT_NAME} | wc -l`
    if [ ${MODIFY_PACKAGE_COUNT} -ne 1 ]; then
        Log "Check package uniqueness unsuccessfully, MODIFY_PACKAGE_COUNT=[$MODIFY_PACKAGE_COUNT]."
        return 1
    else
        MODIFY_PACKAGE_NAME=`ls -l | grep ${PRODUCT_NAME} | $MYAWK '{print $NF}'`
        Log "Check package uniqueness successfully."
        return 0
    fi
}

# Verify the value of the sha256
CheckShaValue()
{
    AGENT_MODIFY_INFOFILE=${AGENT_MODIFY_INFOFILE_PATH}/tmpModifyInfo
    AGENT_MODIFY_ZIPFILE=${AGENT_MODIFY_INFOFILE_PATH}/${MODIFY_PACKAGE_NAME}
    if [ ! -f "${AGENT_MODIFY_INFOFILE}" ]; then
        Log "Not find the modify infomation file [${AGENT_MODIFY_INFOFILE_PATH}/tmpModifyInfo]."
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
    sha256sum ${AGENT_MODIFY_ZIPFILE} | $MYAWK '{print $1}' | grep ${SHA256_VALUE} >/dev/null 2>&1
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
    cd ${MODIFY_PACKAGE_PATH}
    mv ${AGENT_MODIFY_ZIPFILE} ${MODIFY_PACKAGE_PATH}/
    unzip -q ${MODIFY_PACKAGE_PATH}/${MODIFY_PACKAGE_NAME} >/dev/null 2>&1
    return $?
}

VerifySignature()
{
    Log "Start verify modify pkg signature."
    # printf <sha256值> | openssl dgst -sha256 -verify <公钥文件> -signature <签名文件>
    printf ${SHA256_VALUE} | ${AGENT_BIN_PATH}/openssl dgst -sha256 -verify ${PUBLIC_KEY_FILE} -signature ${SIGNATURE_FILE} >> $LOG_FILE_NAME 2>&1
    if [ $? -eq 0 ]; then
        Log "Verify modify file signature successfully."
        CleanSignatureFile
        return 0
    else
        Log "Verify modify file signature unsuccessfully."
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
    LogError "Check pac unique unsuccessfully." ${ERR_MODIFY_FAIL_VERIFICATE_PARAMETER}
    exit 1
fi

CheckShaValue
if [ $? -ne 0 ]; then
    LogError "Check sha256 value unsuccessfully." ${ERR_MODIFY_FAIL_VERIFICATE_PARAMETER}
    exit 1
fi

VerifySignature
if [ $? -ne 0 ]; then
    LogError "Verify modify file signature unsuccessfully." ${ERR_MODIFY_FAIL_VERIFICATE_PARAMETER}
    exit 1
fi

UnzipUpgradeFile
if [ $? -ne 0 ]; then
    LogError "Unzip modify file unsuccessfully." ${ERR_MODIFY_FAIL_VERIFICATE_PARAMETER}
    exit 1
fi

Log "Upgrade prepare successfully."
exit 0
