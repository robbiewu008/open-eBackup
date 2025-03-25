#!/bin/sh
set +x

AGENT_ROOT_PATH=$1
PID=$2
PARAM_NUM=$3
umask 0022
. "${AGENT_ROOT_PATH}/sbin/agent_sbin_func.sh"
LOG_FILE_NAME=${AGENT_ROOT_PATH}/slog/push_update_cert.log

# get param
PARAM_CONTENT=`ReadInputParam`
test -z "$PARAM_CONTENT"              && ExitWithError "PARAM_FILE"

#for GetValve
JOB_ID=`GetValue "${PARAM_CONTENT}" jobID`
FALLBACK_TYPE=`GetValue "${PARAM_CONTENT}" fallBackType`

Log "Job ID is ${JOB_ID}"

AGENT_NGINX_PATH=${AGENT_ROOT_PATH}/nginx
AGENT_NGINX_CONF_PATH=${AGENT_NGINX_PATH}/conf

AGENT_TPM_CART_UPDATE_DIR=${AGENT_ROOT_PATH}/tmp/cert_updating_${JOB_ID}/
AGENT_TMP_NEW_CERT_PATH=${AGENT_TPM_CART_UPDATE_DIR}/newCert
AGENT_TMP_OLD_CERT_PATH=${AGENT_TPM_CART_UPDATE_DIR}/oldCert

THRIFT_CERT_PATH="${AGENT_ROOT_PATH}/conf/thrift"

# 1: update Agent cert chain (file: agentca.pem, server.pem, server.key)
UPDATE_AGENT_CHAIN=1
# 2: update PM CAfile (file: pmca.pem)
UPDATE_PMCA=2
UPDATE_TYPE=${UPDATE_AGENT_CHAIN}

# update cert or rollback cert
UPDATE_CERT=10
ROLLBACK_CERT=11
CLEAN_TMP_FILE=12

BACKUP_ROLE_SANCLIENT_PLUGIN=5

# err code
ERR_ROLL_BACK_SUCCESS=5
ERR_ROLL_BACK_FAILED=6

AGENT_USER="rdadmin"
SANCLIENT_USER="sanclient"

CLIENT_BACK_ROLE=`SUExecCmdWithOutput "cat ${AGENT_ROOT_PATH}/conf/testcfg.tmp | grep \"BACKUP_ROLE\" | ${MYAWK} -F '=' '{print $NF}'"`
if [ "${CLIENT_BACK_ROLE}" = "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ]; then
    AGENT_USER=${SANCLIENT_USER}
fi

if [ "$SYS_NAME" = "SunOS" ]; then
    DEFAULT_HOME_DIR="/export/home/${AGENT_USER}"
else
    DEFAULT_HOME_DIR="/home/${AGENT_USER}"
fi

LO_IP_IPV4="127.0.0.1"
LO_IP_IPV6="::1"

########################################################################################
# Function Definition
SUExecCmdCP()
{
    eval LAST_PARAM=\$$#
    TARGET_FILE=${LAST_PARAM}
    if [ -L "${TARGET_FILE}" ]; then
        echo "The target file is a linked file. Exit the current process."
        Log "The target file is a linked file. Exit the current process."
        exit 1
    fi

    SYSNAME_JUDGE=`uname -s`
    if [ "$SYSNAME_JUDGE" = "SunOS" ]; then
        cmd="cp -R -P $*"
        SUExecCmd "$cmd"
    else
        cmd="cp -d $*"
        SUExecCmd "$cmd"
    fi
}

LogError()
{
    #$1=errorInfo,$2=errorCode, $3=errorDetailParam
    LogErr "$1" "$2" "job_log_agent_storage_update_prepare_fail_label" "$3"
}

BackupOldCerts()
{
    SUExecCmd "mkdir -p ${AGENT_TMP_OLD_CERT_PATH}"
    SUExecCmd "chmod 700 ${AGENT_TMP_OLD_CERT_PATH}"
    if [ "${UPDATE_TYPE}" = "${UPDATE_PMCA}" ]; then
        SUExecCmdCP -f -p ${AGENT_NGINX_CONF_PATH}/pmca.pem ${AGENT_TMP_OLD_CERT_PATH}
        Log "Backup pmca only."
        return 0
    fi
    Log "Backup agentca."
    SUExecCmdCP -f -p ${AGENT_NGINX_CONF_PATH}/agentca.pem ${AGENT_TMP_OLD_CERT_PATH}
    SUExecCmdCP -f -p ${AGENT_NGINX_CONF_PATH}/server.pem ${AGENT_TMP_OLD_CERT_PATH}
    SUExecCmdCP -f -p ${AGENT_NGINX_CONF_PATH}/server.key ${AGENT_TMP_OLD_CERT_PATH}

    OLD_CERT_PWD=`SUExecCmdWithOutput "${AGENT_ROOT_PATH}/bin/xmlcfg read Monitor nginx ssl_key_password"`
    if [ ${OLD_CERT_PWD} = "" ]; then
        Log "OLD_CERT_PWD is empty."
        echo "OLD_CERT_PWD is empty."
        return 1
    fi
    SUExecCmd "echo ${OLD_CERT_PWD} >> ${AGENT_TMP_OLD_CERT_PATH}/key.pwd"
    SUExecCmd "chmod 600 ${AGENT_TMP_OLD_CERT_PATH}/key.pwd"
    Log "Backup old certs success."
}

ReplaceWithNewCerts()
{
    targetPath=$1
    if [ "${UPDATE_TYPE}" = "${UPDATE_PMCA}" ]; then
        SUExecCmdCP -f -p ${targetPath}/pmca.pem ${AGENT_NGINX_CONF_PATH}
        SUExecCmd "chmod 600 ${AGENT_NGINX_CONF_PATH}/pmca.pem"
        Log "Update pmca only."
        return 0
    fi
    Log "Update agentca."
    # thrift certs updating
    if [ -d "${THRIFT_CERT_PATH}" ]; then
        #get old host name
        SUExecCmd "${AGENT_ROOT_PATH}/bin/agentcli gethostname > ${DEFAULT_HOME_DIR}/verify.xml"
        OLD_HOST_NAME=`SUExecCmdWithOutput "cat ${DEFAULT_HOME_DIR}/verify.xml"`
        SUExecCmd "rm -rf ${DEFAULT_HOME_DIR}/verify.xml"
        if [ "${OLD_HOST_NAME}" = "" ]; then 
            echo "Get hostname from certificate failed."
            Log "Get hostname from certificate failed."
            return 1
        fi
        SUExecCmdCP -f -p ${targetPath}/agentca.pem ${THRIFT_CERT_PATH}/server/ca.crt.pem
        SUExecCmd "chmod 600 ${THRIFT_CERT_PATH}/server/ca.crt.pem"
        SUExecCmdCP -f -p ${targetPath}/agentca.pem ${THRIFT_CERT_PATH}/client/ca.crt.pem
        SUExecCmd "chmod 600 ${THRIFT_CERT_PATH}/client/ca.crt.pem"
        SUExecCmdCP -f -p ${targetPath}/server.pem ${THRIFT_CERT_PATH}/server/client.crt.pem
        SUExecCmd "chmod 600 ${THRIFT_CERT_PATH}/server/client.crt.pem"
        SUExecCmdCP -f -p ${targetPath}/server.pem ${THRIFT_CERT_PATH}/client/client.crt.pem
        SUExecCmd "chmod 600 ${THRIFT_CERT_PATH}/client/client.crt.pem"
        SUExecCmdCP -f -p ${targetPath}/server.key ${THRIFT_CERT_PATH}/server/client.pem
        SUExecCmd "chmod 600 ${THRIFT_CERT_PATH}/server/client.pem"
        SUExecCmdCP -f -p ${targetPath}/server.key ${THRIFT_CERT_PATH}/client/client.pem
        SUExecCmd "chmod 600 ${THRIFT_CERT_PATH}/client/client.pem"
    else
        Log "THRIFT_CERT_PATH=${THRIFT_CERT_PATH} is not exist."
    fi

    # nginx certs updating
    SUExecCmdCP -f -p ${targetPath}/agentca.pem ${AGENT_NGINX_CONF_PATH}
    SUExecCmd "chmod 600 ${AGENT_NGINX_CONF_PATH}/agentca.pem"
    SUExecCmdCP -f -p ${targetPath}/server.pem ${AGENT_NGINX_CONF_PATH}
    SUExecCmd "chmod 600 ${AGENT_NGINX_CONF_PATH}/server.pem"
    SUExecCmdCP -f -p ${targetPath}/server.key ${AGENT_NGINX_CONF_PATH}
    SUExecCmd "chmod 400 ${AGENT_NGINX_CONF_PATH}/server.key"

    if [ ! -f "${targetPath}/key.pwd" ]; then
        Log "Cert password is not exist."
        echo "Cert password is not exist."
        return 1
    fi
    NEW_PKEY_PASSWORD=`SUExecCmdWithOutput "cat ${targetPath}/key.pwd"`
    if [ "${NEW_PKEY_PASSWORD}" = "" ]; then
        Log "New cert password is empty."
        echo "New cert password is empty."
        return 1
    fi
    SUExecCmd "${AGENT_ROOT_PATH}/bin/xmlcfg write Monitor nginx ssl_key_password ${NEW_PKEY_PASSWORD}"
    return 0
}

CheckCertReplaceCondition()
{
    CompareFileIdentity ${THRIFT_CERT_PATH}/server/ca.crt.pem ${THRIFT_CERT_PATH}/client/ca.crt.pem
    if [ $? -eq 1 ]; then
        Log "server.pem not same, replace failed."
        return 1;
    else
        Log "server.pem replace success."
    fi

    CompareFileIdentity ${THRIFT_CERT_PATH}/server/client.crt.pem ${THRIFT_CERT_PATH}/client/client.crt.pem
    if [ $? -eq 1 ]; then
        Log "agentca.pem not same, replace failed."
        return 1
    else
        Log "agentca.pem replace success."
    fi

    CompareFileIdentity ${THRIFT_CERT_PATH}/server/client.pem ${THRIFT_CERT_PATH}/client/client.pem
    if [ $? -eq 1 ]; then
        Log "server.key not same, replace failed."
        return 1
    else
        Log "server.key replace success."
    fi

    Log "All certs have been replaced successfully."
    return 0
}

CompareFileIdentity()
{
    FILE_NAME_1=$1
    FILE_NAME_2=$2
    md5sum1=$(md5sum ${FILE_NAME_1} | cut -d' ' -f1)
    md5sum2=$(md5sum ${FILE_NAME_2} | cut -d' ' -f1)
    if [ "${md5sum1}" = "${md5sum2}" ]; then
        return 0
    fi
    return 1
}


RestartAgent()
{
    SUExecCmd "${AGENT_ROOT_PATH}/bin/agent_stop.sh"
    if [ $? -eq 0 ]; then
        Log "The DataBackup ProtectAgent service has been successfully stopped."
    else
        echo "The DataBackup ProtectAgent service fails to be stopped."
        Log "The DataBackup ProtectAgent service fails to be stopped."
        return 1
    fi
    SUExecCmd "${AGENT_ROOT_PATH}/bin/agent_start.sh"
    if [ $? -eq 0 ]; then
        Log "The DataBackup ProtectAgent service is successfully started."
    else
        echo "The DataBackup ProtectAgent service fails to be started."
        Log "The DataBackup ProtectAgent service fails to be started."
        return 1
    fi
}

UpdateSslHostName()
{
    if [ ! -d "${AGENT_ROOT_PATH}/conf/thrift" ]; then
        return 0
    fi

    Log "Start to update hostname."
    NEW_HOST_NAME=`SUExecCmdWithOutput "${AGENT_ROOT_PATH}/bin/agentcli gethostname"`

    if [ "${NEW_HOST_NAME}" = "" ]; then 
        echo "Get hostname from certificate failed."
        Log "Get hostname from certificate failed."
        return 1
    fi
    retOld=`grep ${OLD_HOST_NAME} /etc/hosts`
    ret=`grep ${NEW_HOST_NAME} /etc/hosts`
    if [ "${retOld}" = "" ]; then
        VerifySpecialChar ${LO_IP_IPV4} ${LO_IP_IPV6} ${OLD_HOST_NAME} ${NEW_HOST_NAME}
        echo "${LO_IP_IPV4} ${NEW_HOST_NAME}" >> /etc/hosts
        echo "${LO_IP_IPV6} ${NEW_HOST_NAME}" >> /etc/hosts
    fi

    if [ "${ret}" = "" ]; then
        VerifySpecialChar ${LO_IP_IPV4} ${LO_IP_IPV6} ${OLD_HOST_NAME} ${NEW_HOST_NAME}
        sed -i "/\<${LO_IP_IPV4}\>/,//s+${OLD_HOST_NAME}+${NEW_HOST_NAME}+g" /etc/hosts
    fi
}

RollbackCert()
{
    # 证书文件回退
    Log "rollback certificate."
    if [ -f ${AGENT_ROOT_PATH}/tmp/cert_updating ]; then
        SUExecCmd "rm -f ${AGENT_ROOT_PATH}/tmp/cert_updating"
    fi

    if [ ! -d ${AGENT_TMP_OLD_CERT_PATH} ]; then
        LogError "The AGENT_TMP_NEW_CERT_PATH ${AGENT_TMP_OLD_CERT_PATH} does not exist." ${ERR_UPGRADE_FAIL_VERIFICATE_PARAMETER}
        echo "The AGENT_TMP_NEW_CERT_PATH ${AGENT_TMP_OLD_CERT_PATH} does not exist."
        exit 1
    fi
    ReplaceWithNewCerts ${AGENT_TMP_OLD_CERT_PATH}
    if [ $? -ne 0 ]; then
        Log "ReplaceWithNewCerts failed!."
        echo "ReplaceWithNewCerts failed!."
        exit 1
    fi

    if [ "${UPDATE_TYPE}" = "${UPDATE_AGENT_CHAIN}" ]; then
        UpdateSslHostName
        if [ $? -ne 0 ]; then
            echo "Failed to update ssl host name. Failed to rollback certs."
            Log "Failed to update ssl host name. Failed to rollback certs."
            exit 1
        fi
    fi

    RestartAgent
    if [ $? -ne 0 ]; then
        Log "RestartAgent fail. Failed to rollback certs."
        echo "RestartAgent fail. Failed to rollback certs."
        exit 1
    else
        CleanTmpFiles
        if [ $? -ne 0 ]; then
            Log "Clean temp files failed."
            echo "Clean temp files failed."
            exit 1
        fi
    fi

    Log "Rollback certificate files successfully!"
    echo "Rollback certificate files successfully!"
    exit ${ERR_ROLL_BACK_SUCCESS}
}

CleanTmpFiles()
{
    Log "Begin to clean files!"

    target="${AGENT_ROOT_PATH}/tmp/"
    key_word="cert_updating"

    find ${target} -name "*${key_word}*" | while read file
    do
        echo "delete ${file}"
        SUExecCmd "rm -rf ${file}"
    done

    if [ -f ${AGENT_ROOT_PATH}/tmp/pm_to_agent_status_ok ]; then
        SUExecCmd "rm -f ${AGENT_ROOT_PATH}/tmp/pm_to_agent_status_ok"
    fi

    Log "Clean temp files successfully!"
    return 0
}
#################################################################################
##  main process
#################################################################################
if [ "${FALLBACK_TYPE}" = "${CLEAN_TMP_FILE}" ]; then
    Log "************ Clean Temp cert files *************"
    CleanTmpFiles
    if [ $? -ne 0 ]; then
        Log "Clean temp files failed."
        echo "Clean temp files failed."
        exit 1
    fi
    exit 0
fi

# update cert
if [ -f "${AGENT_TMP_NEW_CERT_PATH}/pmca.pem" ]; then
    UPDATE_TYPE=${UPDATE_PMCA}
    echo "Update pmca only."
fi

# Check if rollback
if [ "${FALLBACK_TYPE}" = "${ROLLBACK_CERT}" ]; then
    Log "************ Rollback cert *************"
    RollbackCert
fi

Log "************ Update cert ************"

BackupOldCerts
if [ $? -ne 0 ]; then
    Log "BackupOldCerts fail."
    echo "BackupOldCerts fail."
    exit 1
fi

if [ ! -d ${AGENT_TMP_NEW_CERT_PATH} ]; then
    LogError "The AGENT_TMP_NEW_CERT_PATH ${AGENT_TMP_NEW_CERT_PATH} does not exist." ${ERR_UPGRADE_FAIL_VERIFICATE_PARAMETER}
    echo "The AGENT_TMP_NEW_CERT_PATH ${AGENT_TMP_NEW_CERT_PATH} does not exist."
    exit 1
fi

Log "Start copy new certificate files."
ReplaceWithNewCerts ${AGENT_TMP_NEW_CERT_PATH}
if [ $? -ne 0 ]; then
    Log "ReplaceWithNewCerts failed! Start rollback."
    echo "ReplaceWithNewCerts failed! Start rollback."
    RollbackCert
    exit 1
fi

CheckCertReplaceCondition

if [ "${UPDATE_TYPE}" = "${UPDATE_AGENT_CHAIN}" ]; then
    UpdateSslHostName
    if [ $? -ne 0 ]; then
        echo "Failed to update ssl host name. Start rollback."
        Log "Failed to update ssl host name. Start rollback."
        RollbackCert
        exit 1
    fi
fi

RestartAgent
if [ $? -ne 0 ]; then
    Log "RestartAgent fail."
    echo "RestartAgent fail."
    RollbackCert
    exit 1
fi

Log "Update certs successfully."
echo "Update certs successfully."
exit 0
