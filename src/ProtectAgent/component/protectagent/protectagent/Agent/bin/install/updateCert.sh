#!/bin/sh
set +x
SYS_NAME=`uname -s`
if [ "$SYS_NAME" = "SunOS" ]; then
    MYAWK=nawk
else
    MYAWK=awk
fi
###### Custom installation directory ######
TMP_DATA_BACKUP_SANCLIENT_HOME=`cat /etc/profile | grep "export DATA_BACKUP_SANCLIENT_HOME=" |${MYAWK} -F "=" '{print $NF}'`
TMP_DATA_BACKUP_AGENT_HOME=`cat /etc/profile | grep "export DATA_BACKUP_AGENT_HOME=" |${MYAWK} -F "=" '{print $NF}'`
if [ -n "${TMP_DATA_BACKUP_AGENT_HOME}" ] || [ -n "${TMP_DATA_BACKUP_SANCLIENT_HOME}" ] ; then
    . /etc/profile
else
    DATA_BACKUP_AGENT_HOME=/opt
    DATA_BACKUP_SANCLIENT_HOME=/opt
    export DATA_BACKUP_AGENT_HOME DATA_BACKUP_SANCLIENT_HOME
fi

####################################################################
# The function of this script is to upgrade the client certificate.
####################################################################

# define variables
INPUT_CERT_PATH=""
PKEY_PASSWORD=""
AGENT_USER=rdadmin
SANCLIENT_USER=sanclient
CURRENT_PATH=`dirname $0` && cd $CURRENT_PATH && CURRENT_PATH=`pwd`
INSTALL_DIR="${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient/"
SANCLIENT_INSTALL_DIR="${DATA_BACKUP_SANCLIENT_HOME}/DataBackup/SanClient/"
AGENT_ROOT_PATH="${INSTALL_DIR}/ProtectClient-E/"
SANCLIENT_AGENT_ROOT_PATH="${SANCLIENT_INSTALL_DIR}/ProtectClient-E/"
BACKUP_ROLE_SANCLIENT_PLUGIN=5
OLD_PKG_PASSWORD=""
SHELL_TYPE_SH="/bin/sh"
LO_IP_IPV4="127.0.0.1"
LO_IP_IPV6="::1"
OLD_HOST_NAME=""
NEW_HOST_NAME=""

#cert update type
UPDATE_TYPE=
#1 : update Agent cert chain (file: agentca.pem, server.pem, server.key)
UPDATE_AGENT_CHAIN=1
#2 : update PM CAfile (file: pmca.pem)
UPDATE_PMCA=2

AGENT_BACK_ROLE=`su - ${SANCLIENT_USER} 2>/dev/null -s ${SHELL_TYPE_SH} -c "sed '/^BACKUP_ROLE=/!d;s/.*=//' ${CURRENT_PATH}/ProtectClient-E/conf/testcfg.tmp "`
if [ "${AGENT_BACK_ROLE}" = "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ]; then
    AGENT_USER=${SANCLIENT_USER}
    INSTALL_DIR=${SANCLIENT_INSTALL_DIR}
    AGENT_ROOT_PATH=${SANCLIENT_AGENT_ROOT_PATH}
fi

if [ "$SYS_NAME" = "SunOS" ]; then
    DEFAULT_HOME_DIR="/export/home/${AGENT_USER}"
else
    DEFAULT_HOME_DIR="/home/${AGENT_USER}"
fi
#Agent certification path
NGINX_CERT_BACK_PATH="${INSTALL_DIR}/tmpPems/ProtectClient-E/"
THRIFT_CERT_PATH="${AGENT_ROOT_PATH}/conf/thrift/"
THRIFT_CERT_BACK_PATH="${INSTALL_DIR}/tmpPems/thrift/"
DWS_XBSA_CERT_PATH="/usr/openv/conf/thrift/client/"
DWS_XBSA_CERT_BACK_PATH="${INSTALL_DIR}/tmpPems/dws/XBSA"
AGENT_CFG_XML_PATH="${AGENT_ROOT_PATH}/conf/agent_cfg.xml"
DWS_XBSA_CFG_PATH="/usr/openv/conf/agent_cfg.xml"
DWS_ARCHIVE_CERT_PATH="/usr/openv/nginx/conf/"
DWS_ARCHIVE_CERT_BACK_PATH="${INSTALL_DIR}/tmpPems/dws/ARCHIVE/"
PM_CA_PATH="${AGENT_ROOT_PATH}/nginx/conf/pmca.pem"
AGENT_CA_PATH="${AGENT_ROOT_PATH}/nginx/conf/agentca.pem"
SERVER_CRT_PATH="${AGENT_ROOT_PATH}/nginx/conf/server.pem"
SERVER_KEY_PATH="${AGENT_ROOT_PATH}/nginx/conf/server.key"

LOG_FILE_NAME=${AGENT_ROOT_PATH}/slog/upgradeCert.log
. "${AGENT_ROOT_PATH}/sbin/agent_sbin_func.sh"

CheckIsSanclientType()
{
    if [ "${AGENT_BACK_ROLE}" = "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ]; then
        AGENT_USER=${SANCLIENT_USER}
        AGENT_ROOT_PATH=${SANCLIENT_AGENT_ROOT_PATH}
    fi
}
IsUpdatePems()
{
    echo "[Info]This script replaces only the client certificate and does not verify the certificate. \n"
    echo "[Info]Please ensure that the new certificate and the OpenApi component certificate of ProtectManager are issued by the same CA.\n"
    echo "Updating certificates would restart DataBackup ProtectAgent, are you sure you want to start updating the certificates now?[y|n]:"
    read choice
    if [ "${choice}" = "y" ]; then
        Log "Start update the certificates."
    else 
        echo "Exit update the certificates."
        exit 1
    fi
    for i in 1 2 3
    do
        echo "Choice update type[try ${i}/3]:\n"
        echo "    1: Update Agent CAfile and server certificate.\n"
        echo "    2: Only update PM CAfile.\n"
        echo "input your choice?[1|2]:"
        read UPDATE_TYPE
        if [ "${UPDATE_TYPE}" == "${UPDATE_AGENT_CHAIN}" ]; then
            echo "user input is ${UPDATE_TYPE}: Update Agent CAfile and server certificate."
            Log "user input is ${UPDATE_TYPE}: Update Agent CAfile and server certificate."
            return 0
        fi
        if [ "${UPDATE_TYPE}" == "${UPDATE_PMCA}" ]; then
            echo "user input is ${UPDATE_TYPE}: Only update PM CAfile."
            Log "user input is ${UPDATE_TYPE}: Only update PM CAfile."
            return 0
        fi
        echo "pleas input update type."
    done
    echo "user input type incorrcet, exit the program."
    Log "user input type incorrcet, exit the program."
    exit 1
}

BackupPemsAndKey()
{
    if [ ! -d "${NGINX_CERT_BACK_PATH}" ]; then
        mkdir -p "${NGINX_CERT_BACK_PATH}"
        Log "Create certificates backup for the DataBackup ProtectAgent."
    fi

    if [ "${UPDATE_TYPE}" == "${UPDATE_PMCA}" ]; then
        CP -pf "${PM_CA_PATH}" "${NGINX_CERT_BACK_PATH}/pmca.pem"
        # backup Archive nginx
        if [ -d "${DWS_ARCHIVE_CERT_PATH}" ]; then
            if [ ! -d "${DWS_ARCHIVE_CERT_BACK_PATH}" ]; then
                mkdir -p "${DWS_ARCHIVE_CERT_BACK_PATH}"
                Log "Create certificates backup for the archive."
            fi
            CP -prf "${DWS_ARCHIVE_CERT_PATH}"/* "${DWS_ARCHIVE_CERT_BACK_PATH}"
            Log "Backup archive certificates successfully."
        fi
        Log "Backup PMCAfile successfully."
        return 0
    fi

    if [ "${UPDATE_TYPE}" == "${UPDATE_AGENT_CHAIN}" ]; then
        CP -pf "${AGENT_CA_PATH}" "${NGINX_CERT_BACK_PATH}/agentca.pem"
        CP -pf "${SERVER_CRT_PATH}" "${NGINX_CERT_BACK_PATH}/server.pem"
        CP -pf "${SERVER_KEY_PATH}" "${NGINX_CERT_BACK_PATH}/server.key"
        OLD_PKG_PASSWORD=`SUExecCmdWithOutput "${AGENT_ROOT_PATH}/bin/xmlcfg read Monitor nginx ssl_key_password"`
        # backup thrift cert
        if [ -d "${THRIFT_CERT_PATH}" ]; then
            if [ ! -d "${THRIFT_CERT_BACK_PATH}" ]; then
                mkdir -p "${THRIFT_CERT_BACK_PATH}"
                Log "Create certificates backup for the thrift."
            fi
            CP -prf "${THRIFT_CERT_PATH}"/* "${THRIFT_CERT_BACK_PATH}"
            Log "Backup thrift certificates successfully."
        fi
        # backup DWS cert
        if [ -d "${DWS_XBSA_CERT_PATH}" ]; then
            if [ ! -d "${DWS_XBSA_CERT_BACK_PATH}" ]; then
                mkdir -p "${DWS_XBSA_CERT_BACK_PATH}"
                Log "Create certificates backup for the dws."
            fi
            CP -prf "${DWS_XBSA_CERT_PATH}"/* "${DWS_XBSA_CERT_BACK_PATH}"
            Log "Backup DWS certificates successfully."
        fi
        # backup Archive nginx
        if [ -d "${DWS_ARCHIVE_CERT_PATH}" ]; then
            if [ ! -d "${DWS_ARCHIVE_CERT_BACK_PATH}" ]; then
                mkdir -p "${DWS_ARCHIVE_CERT_BACK_PATH}"
                Log "Create certificates backup for the archive."
            fi
            CP -prf "${DWS_ARCHIVE_CERT_PATH}"/* "${DWS_ARCHIVE_CERT_BACK_PATH}"
            Log "Backup archive certificates successfully."
        fi
        Log "Backup Agent CAfile relevant certificate successfully."
        return 0
    fi
}

DeleteBackupPems()
{
    if [ -d "${INSTALL_DIR}/tmpPems" ]; then
        rm -rf "${INSTALL_DIR}/tmpPems"
    fi
}

RollBackPems()
{
    echo "Start to rollback certificates."
    Log "Start to rollback certificates."
    #rollback certificates
    if [ "${UPDATE_TYPE}" == "${UPDATE_PMCA}" ]; then
        CP -pf "${NGINX_CERT_BACK_PATH}/pmca.pem" "${PM_CA_PATH}"
        # rollback Archive nginx
        if [ -d "${DWS_ARCHIVE_CERT_BACK_PATH}" ]; then
            CP -rpf "${DWS_ARCHIVE_CERT_BACK_PATH}"/* "${DWS_ARCHIVE_CERT_PATH}"
            Log "Rollback archive certificates successfully."
        fi
    fi

    if [ "${UPDATE_TYPE}" == "${UPDATE_AGENT_CHAIN}" ]; then
        CP -pf "${NGINX_CERT_BACK_PATH}/agentca.pem" "${AGENT_CA_PATH}"
        CP -pf "${NGINX_CERT_BACK_PATH}/server.pem" "${SERVER_CRT_PATH}"
        CP -pf "${NGINX_CERT_BACK_PATH}/server.key" "${SERVER_KEY_PATH}"
        SUExecCmd "${AGENT_ROOT_PATH}/bin/xmlcfg write Monitor nginx ssl_key_password ${OLD_PKG_PASSWORD}"
        # rollback thrift cert
        if [ -d "${THRIFT_CERT_BACK_PATH}" ]; then
            CP -prf "${THRIFT_CERT_BACK_PATH}"/* "${THRIFT_CERT_PATH}"
            #rollback hostname
            if [ -n "${NEW_HOST_NAME}" ] && [ -n "${OLD_HOST_NAME}" ]; then
                # verify special chars
                VerifySpecialChar ${LO_IP_IPV4} ${LO_IP_IPV6} ${NEW_HOST_NAME} ${OLD_HOST_NAME}
                sed -i "/\<${LO_IP_IPV4}\>/,//s+${NEW_HOST_NAME}+${OLD_HOST_NAME}+g" /etc/hosts
                sed -i "/\<${LO_IP_IPV6}\>/,//s+${NEW_HOST_NAME}+${OLD_HOST_NAME}+g" /etc/hosts
            fi
            Log "Rollback thrift certificates successfully."
        fi
        # rollback DWS cert
        if [ -d "${DWS_XBSA_CERT_BACK_PATH}" ]; then
            CP -rpf "${DWS_XBSA_CERT_BACK_PATH}"/* "${DWS_XBSA_CERT_PATH}"
            CP -f "${AGENT_CFG_XML_PATH}" "${DWS_XBSA_CFG_PATH}"
            Log "Rollback DWS certificates successfully."
        fi
        # rollback Archive nginx
        if [ -d "${DWS_ARCHIVE_CERT_BACK_PATH}" ]; then
            CP -rpf "${DWS_ARCHIVE_CERT_BACK_PATH}"/* "${DWS_ARCHIVE_CERT_PATH}"
            Log "Rollback archive certificates successfully."
        fi
    fi

    #register to protectmanager
    RegisterHost
    if [ $? -eq 1 ]; then
        echo "Register to ProtectManager failed."
        Log "Register to ProtectManager failed."
        return 1
    fi

    # restart agent process
    RestartAgent
    if [ $? -eq 0 ]; then
        # Deleting Backup Certificate Files
        DeleteBackupPems
        return 0
    elif [ $? -eq 1 ]; then
        echo "Failed to restart the ProtectAgent process. Please manually restart the process."
        return 1
    fi

    return 0
}

GetPemPathAndCopyPems()
{
    echo "Please enter the path of the certificate to be upgraded:"
    read INPUT_CERT_PATH

    if [ ! -d ${INPUT_CERT_PATH} ]; then
        echo "The entered path is incorrect. The path does not exist."
        return 1
    fi

    if [ "${UPDATE_TYPE}" == "${UPDATE_PMCA}" ]; then
        if [ ! -f "${INPUT_CERT_PATH}/ca.crt.pem" ]; then
            echo "Can not find the specified certificate file in the path.[ca.crt.pem]"
            Log "Can not find the specified certificate file in the path.[ca.crt.pem]"
            return 1
        fi
        CP "${INPUT_CERT_PATH}/ca.crt.pem" "${PM_CA_PATH}"
        # update archive ca certificates
        if [ -d "${DWS_ARCHIVE_CERT_PATH}" ]; then
            CP -f "${PM_CA_PATH}" "${DWS_ARCHIVE_CERT_PATH}"
        fi
        return 0
    fi

    if [ "${UPDATE_TYPE}" == "${UPDATE_AGENT_CHAIN}" ]; then
        if [ ! -f "${INPUT_CERT_PATH}/agentca.crt.pem" ] || [ ! -f "${INPUT_CERT_PATH}/client.pem" ] || [ ! -f "${INPUT_CERT_PATH}/client.crt.pem" ]; then
            echo "Can not find the specified certificate file in the path.[client.pem, client.crt.pem, agentca.crt.pem]"
            Log "Can not find the specified certificate file in the path.[client.pem, client.crt.pem, agentca.crt.pem]"
            return 1
        fi
        # update thrift certificates
        if [ -d "${THRIFT_CERT_PATH}" ]; then
            #get old host name
            SUExecCmd "${AGENT_ROOT_PATH}/bin/agentcli gethostname > ${DEFAULT_HOME_DIR}/verify.xml"
            OLD_HOST_NAME=`SUExecCmdWithOutput "cat ${DEFAULT_HOME_DIR}/verify.xml"`
            SUExecCmd "rm -rf ${DEFAULT_HOME_DIR}/verify.xml"
            if [ "${OLD_HOST_NAME}" = "" ]; then 
                echo "Get old hostname from certificate failed."
                Log "Get old hostname from certificate failed."
                return 1
            fi
            CP -f "${INPUT_CERT_PATH}/agentca.crt.pem" "${THRIFT_CERT_PATH}/server/ca.crt.pem"
            CP -f "${INPUT_CERT_PATH}/agentca.crt.pem" "${THRIFT_CERT_PATH}/client/ca.crt.pem"
            CP -f "${INPUT_CERT_PATH}/client.crt.pem" "${THRIFT_CERT_PATH}/server"
            CP -f "${INPUT_CERT_PATH}/client.crt.pem" "${THRIFT_CERT_PATH}/client"
            CP -f "${INPUT_CERT_PATH}/client.pem" "${THRIFT_CERT_PATH}/server"
            CP -f "${INPUT_CERT_PATH}/client.pem" "${THRIFT_CERT_PATH}/client"
        fi
        # update Agent nginx certificates
        CP -f "${INPUT_CERT_PATH}/agentca.crt.pem" "${AGENT_CA_PATH}"
        CP -f "${INPUT_CERT_PATH}/client.crt.pem" "${SERVER_CRT_PATH}"
        CP -f "${INPUT_CERT_PATH}/client.pem" "${SERVER_KEY_PATH}"
        # update dws certificates
        if [ -d "${DWS_XBSA_CERT_PATH}" ]; then
            CP -rf "${THRIFT_CERT_PATH}/client/"/* "${DWS_XBSA_CERT_PATH}" 2>/dev/null
            Log "Backup DWS certificates successfully."
        fi
        # update archive server certificates
        if [ -d "${DWS_ARCHIVE_CERT_PATH}" ]; then
            CP -f "${SERVER_CRT_PATH}" "${DWS_ARCHIVE_CERT_PATH}"
            CP -f "${SERVER_KEY_PATH}" "${DWS_ARCHIVE_CERT_PATH}"
        fi
        return 0
    fi

    return 1
}

VerifyPasswd()
{
    JudgeRandomNumType
    SUExecCmd "${AGENT_ROOT_PATH}/bin/agentcli verifykey > ${DEFAULT_HOME_DIR}/verify.xml"
    PKEY_VERIFY=`SUExecCmdWithOutput "cat ${DEFAULT_HOME_DIR}/verify.xml"`
    SUExecCmd "rm -rf ${DEFAULT_HOME_DIR}/verify.xml"

    if [ "${PKEY_VERIFY}" = "failed" ]; then
        echo "Failed to verify the private key password."
        Log "Failed to verify the private key password."
        return 1
    fi
    #change passwd in dws xbsa config file
    CP -f "${AGENT_CFG_XML_PATH}" "${DWS_XBSA_CFG_PATH}"
    return 0
}

GetSslKeyAndWriteXml()
{
    JudgeRandomNumType
    echo "Please enter the private key password of the certificate:"
    SUExecCmdWithOutput "${AGENT_ROOT_PATH}/bin/agentcli enckey cipherFile >/dev/null 2>&1"
    [ -f "${AGENT_ROOT_PATH}/tmp/input_tmpcipherFile" ] && PKEY_PASSWORD=`cat ${AGENT_ROOT_PATH}/tmp/input_tmpcipherFile`
    SUExecCmd "rm -rf \"${AGENT_ROOT_PATH}/tmp/input_tmpcipherFile\""

    if [ "${PKEY_PASSWORD}" != "" ]; then
        SUExecCmd "${AGENT_ROOT_PATH}/bin/xmlcfg write Monitor nginx ssl_key_password ${PKEY_PASSWORD}"
    else
        echo "No valid pkey password."
        Log "No valid pkey password."
        return 1
    fi

    VerifyPasswd
    return $?
}

RegisterHost()
{
    JudgeRandomNumType
    SUExecCmd "${AGENT_ROOT_PATH}/bin/agentcli registerHost RegisterHost >/dev/null 2>&1"
    if [ "$?" != 0 ]; then
        echo "The host fails to be registered to the ProtectManager."
        Log "The host fails to be registered to the ProtectManager."
        return 1
    fi
    echo "The host is successfully registered to the ProtectManager."
    Log "The host is successfully registered to the ProtectManager."
}

StopAgent()
{
    cd "${INSTALL_DIR}"
    ./stop.sh >/dev/null 2>&1
    if [ $? -eq 0 ]; then
        Log "The DataBackup ProtectAgent service has been successfully stopped."
    else
        echo "The DataBackup ProtectAgent service fails to be stopped."
        Log "The DataBackup ProtectAgent service fails to be stopped."
        return 1
    fi
    return 0
}

StartAgent()
{
    cd "${INSTALL_DIR}"
    ./start.sh >/dev/null 2>&1
    if [ $? -eq 0 ]; then
        Log "The DataBackup ProtectAgent service is successfully started."
    else
        echo "The DataBackup ProtectAgent service fails to be started."
        Log "The DataBackup ProtectAgent service fails to be started."
        return 1
    fi
    return 0
}

RestartAgent()
{
    StopAgent
    if [ $? -eq 0 ]; then
        Log "Succeed to stop the service."
    else
        Log "Failed to stop the service."
        return 1
    fi

    StartAgent
    if [ $? -eq 0 ]; then
        Log "Succeed to start the service."
    else
        Log "Failed to start the service."
        return 1
    fi

    return 0
}

UpdateSslHostName()
{
    if [ ! -d "${AGENT_ROOT_PATH}/conf/thrift" ]; then
        return 0
    fi

    echo "Start to update hostname."
    NEW_HOST_NAME=`SUExecCmdWithOutput "${AGENT_ROOT_PATH}/bin/agentcli gethostname"`

    if [ "${NEW_HOST_NAME}" = "" ]; then 
        echo "Get hostname from certificate failed."
        Log "Get hostname from certificate failed."
        return 1
    fi
    ret=`grep ${NEW_HOST_NAME} /etc/hosts`
    if [ "${ret}" = "" ]; then
        VerifySpecialChar ${LO_IP_IPV4} ${LO_IP_IPV6} ${OLD_HOST_NAME} ${NEW_HOST_NAME}
        sed -i "/\<${LO_IP_IPV4}\>/,//s+${OLD_HOST_NAME}+${NEW_HOST_NAME}+g" /etc/hosts
        sed -i "/\<${LO_IP_IPV6}\>/,//s+${OLD_HOST_NAME}+${NEW_HOST_NAME}+g" /etc/hosts
    fi
    return 0
}

UpdateCert()
{
    if [ -d "${AGENT_ROOT_PATH}/bin" ]; then
        Log "Begin to update the certificate."
        if [ "${UPDATE_TYPE}" == "${UPDATE_AGENT_CHAIN}" ]; then
            #get new package password
            GetSslKeyAndWriteXml
            if [ $? -eq 1 ]; then
                echo "Failed to obtain the private key password."
                Log "Failed to obtain the private key password."
                return 1
            fi
            #update ssl host name (dws)
            UpdateSslHostName
            if [ $? -eq 1 ]; then
                echo "Failed to update ssl host name."
                Log "Failed to update ssl host name."
                return 1
            fi
        fi
        
        #register to pm
        RegisterHost
        if [ $? -eq 1 ]; then
            echo "Register to ProtectManager failed."
            Log "Register to ProtectManager failed."
            return 1
        fi
    else 
        echo "Not found the DataBackup ProtectAgent path."
	    Log "Not found the DataBackup ProtectAgent path."
    fi
}

###################################################################################
# update pems
###################################################################################

printf "\\033[1;32m********************************************************\\033[0m \n"
printf "\\033[1;32m     Start updating the certificates of ${PRODUCT_NAME}     \\033[0m \n"
printf "\\033[1;32m********************************************************\\033[0m \n"

# 判断代理类型
CheckIsSanclientType

# 1.make sure is update pems
IsUpdatePems

# 2.backup pems
BackupPemsAndKey

# 3. get pem path and copt pems into install path
GetPemPathAndCopyPems
if [ $? -ne 0 ]; then
    DeleteBackupPems
    exit 1
fi

# 4.update pems
UpdateCert
if [ $? -ne 0 ]; then
    RollBackPems
    if [ $? -eq 0 ]; then
        echo "Certificate rolled back successfully. Failed to upgrade the certificate."
    else
        echo "Certificate rolled back failed. Please manually roll back the certificate."
    fi
    exit 1
fi

# 5.restart agent process
RestartAgent
if [ $? -ne 0 ]; then
    echo "Failed to restart the ProtectAgent process. Please manually restart the process."
fi

# 6.delete backup cert
DeleteBackupPems

echo "update the certificate in Agent succeed."
exit 0
