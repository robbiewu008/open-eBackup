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

SYS_NAME=`uname -s`
if [ "${SYS_NAME}" == "SunOS" ]; then
    MYAWK=nawk
else
    MYAWK=awk
fi

###### Custom installation directory ######
TMP_DATA_BACKUP_AGENT_HOME=`cat /etc/profile | grep "export DATA_BACKUP_AGENT_HOME=" |${MYAWK} -F "=" '{print $NF}'`
if [ -n "${TMP_DATA_BACKUP_AGENT_HOME}" ]; then
    . /etc/profile
    if [ -n "${DATA_BACKUP_AGENT_HOME}" ]; then
        DATA_BACKUP_AGENT_HOME=${TMP_DATA_BACKUP_AGENT_HOME}
        export DATA_BACKUP_AGENT_HOME
    fi
else
    DATA_BACKUP_AGENT_HOME=/opt
    export DATA_BACKUP_AGENT_HOME
fi

##############################################################
# The function of this script is to upgrade the client agent.
##############################################################
INSTALL_PATH="${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient/"
AGENT_ROOT_PATH="${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient/ProtectClient-E/"
AGENT_USER=rdadmin

# sanclient 
SANCLIENT_USER=sanclient
SANCLIENT_INSTALL_PATH="/opt/DataBackup/SanClient/"
SANCLIENT_AGENT_ROOT_PATH="/opt/DataBackup/SanClient/ProtectClient-E/"
BACKUP_ROLE_SANCLIENT_PLUGIN=5
SHELL_TYPE_SH="/bin/sh"

TESTCFG_BACK_ROLE=""
TESTCFG_BACK_ROLE=`su - ${SANCLIENT_USER} 2>/dev/null -s ${SHELL_TYPE_SH} -c "sed '/^BACKUP_ROLE=/!d;s/.*=//' /opt/DataBackup/SanClient/ProtectClient-E/conf/testcfg.tmp "`
if [ "${BACKUP_ROLE_SANCLIENT_PLUGIN}" = "${TESTCFG_BACK_ROLE}" ]; then
    INSTALL_PATH=${SANCLIENT_INSTALL_PATH}
    AGENT_ROOT_PATH=${SANCLIENT_AGENT_ROOT_PATH}
    AGENT_USER=${SANCLIENT_USER}
fi

AGENT_BIN_PATH=${AGENT_ROOT_PATH}/bin/
AGENT_LOG_PATH=${AGENT_ROOT_PATH}/log/
AGENT_CONF_PATH=${AGENT_ROOT_PATH}/conf/
AGENT_TMP_PATH=${AGENT_ROOT_PATH}/tmp/
NGINX_CONF_PATH=${AGENT_ROOT_PATH}/nginx/conf/

START_AGENT_SCRPT="${INSTALL_PATH}/start.sh"
STOP_AGENT_SCRPT="${INSTALL_PATH}/stop.sh"
LOG_FILE_NAME="${AGENT_ROOT_PATH}/slog/crl_update.log"
OPENSSL_FILE="${AGENT_BIN_PATH}/openssl"
NGINX_CONF_FILE="${NGINX_CONF_PATH}/nginx.conf"
NGINX_CONF_FILE_BACK="${NGINX_CONF_PATH}/nginx.conf.back"
CA_FILE="${NGINX_CONF_PATH}/pmca.pem"
CERT_FILE="${NGINX_CONF_PATH}/server.pem"
CRL_FILE="${NGINX_CONF_PATH}/server.crl"
COMMAND_RETURN_FILE="${AGENT_TMP_PATH}/command_return.tmp"
TESTCFG_TMP_FILE="${AGENT_CONF_PATH}/testcfg.tmp"
TMP_TESTCFG_TMP_FILE="${AGENT_CONF_PATH}/tmptestcfg.tmp"
IMPORT_CRL_FILE=
ISIMPORT_FLAG=

#CRL size limit is 5kb
CRL_MAX_SIZE_5KB=5120

# MODE_TYPE
# 1: Import CRL
# 2: Restore certificate
MODE_TYPE=0
IS_UPGRADE=0
NEED_CLEAN=0

. "${AGENT_ROOT_PATH}/sbin/agent_sbin_func.sh"

#################### Function ##########################

CheckParameter()
{
    if [ $# -lt 1 ] || [ $# -ge 3 ]; then 
        echo "Invalid argument, use help to get usage."
        return 1
    fi

    if [ "$1" == "help" ]; then
        PrintHelp
        exit 1
    fi 

    option=$1
    ISIMPORT_FLAG=`cat ${TESTCFG_TMP_FILE} | grep CERTIFICATE_REVOCATION | ${MYAWK} -F= '{print $2}'`

    if [ "${option}" == "-i" ]; then
        IMPORT_CRL_FILE=$2
        if [ ! -f "${IMPORT_CRL_FILE}" ]; then
            echo "Not found CRL, use help to get usage."
            return 1
        else
            MODE_TYPE=1
            echo "Curren option is import CRL."
            return 0
        fi
    elif [ "${option}" == "-r" ]; then
        if [ "${ISIMPORT_FLAG}" == "0" ]; then
            echo "There is no imported CRL, restore operation is illegal"
            return 1
        fi
        MODE_TYPE=2
        echo "Curren option is restore certificate."
        return 0
    elif [ "${option}" == "-u" ]; then
        IMPORT_CRL_FILE=$2
        if [ ! -f "${IMPORT_CRL_FILE}" ]; then
            Log "Not found CRL, in upgrade agent."
            return 1
        fi
        FILE_REAL_DIR=`readlink -e ${IMPORT_CRL_FILE}`
        if [ "`dirname ${FILE_REAL_DIR}`" != "${DATA_BACKUP_AGENT_HOME}/AgentUpgrade/ProtectClient-E/nginx/conf" ]; then
            Log "Crl file must be in the ${DATA_BACKUP_AGENT_HOME}/AgentUpgrade/ProtectClient-E/nginx/conf."
            return 1
        fi
        MODE_TYPE=1
        IS_UPGRADE=1
        return 0
    else
        echo "Invalid argument, use help to get usage."
        return 1
    fi
    return 1
}

CheckInstalled()
{
    if [ -d "${AGENT_ROOT_PATH}" ]; then
        return 0
    else
        echo "DataBackup ProtectAgent is not detected in the current environment, failed to update CRL."
        Log "Check DataBackup ProtectAgent failed."
        return 1
    fi
}

CheckOpenssl()
{
    if [ ! -f "${OPENSSL_FILE}" ]; then
        echo "Can't find openssl."
        Log "Can't find openssl in dir[bin]."
        return 1
    fi
    Log "Check openssl successfully."
    return 0
}

CheckCRLIsImported()
{
    if [ "${ISIMPORT_FLAG}" == "1" ]; then    
        #upgrade Agent execute script
        if [ "${IS_UPGRADE}" == "1" ]; then
            return 0;
        fi
        #manual execute script
        for i in 0 1 2
        do
            echo "The certificate is currently imported, whether to overwrite it?[y/n]"
            read choice
            if [ "${choice}" == "y" ] || [ "${choice}" == "Y" ]; then
                echo "Prepare to overwrite CRL."
                Log "User choose overwrite CRL."
                return 0
            elif [ "${choice}" == "n" ] || [ "${choice}" == "N" ]; then
                echo "Current choice cancel import CRL."
                Log "User choose cancel import CRL."
                return 1
            else
                echo "Please enter y/Y (YES) or n/N (NO), and default is n."
            fi
        done
        echo "The number of incorrect inputs exceeds 3. default is cancel import."
        Log "The number of incorrect inputs exceeds 3. default is cancel import."
        return 1
    fi
    echo "There is currently no imported CRL file."
    Log "There is no imported CRL file."
    return 0
}

ParseCRL()
{
    #Check if it is a CRL file
    "${OPENSSL_FILE}" crl -in "${IMPORT_CRL_FILE}" -noout -text &>"${COMMAND_RETURN_FILE}"
    return_check_crl_file=`cat "${COMMAND_RETURN_FILE}" | grep "unable to load CRL"`
    if [ "${return_check_crl_file}" ]; then
        echo "Input file is not a CRL."
        printf "\\033[1;31mParse CRL failed.\\033[0m \n"
        Log "Load CRL file failed."
        return 1
    fi 
    rm -f "${COMMAND_RETURN_FILE}"

    #No certificate have been revoked in CRL
    "${OPENSSL_FILE}" crl -in "${IMPORT_CRL_FILE}" -noout -text &>"${COMMAND_RETURN_FILE}"
    return_check_crl_file=`cat "${COMMAND_RETURN_FILE}" | grep "No Revoked Certificates"`
    if [ "${return_check_crl_file}" ]; then
        echo "No certificate have been revoked in CRL."
        printf "\\033[1;31mParse CRL failed.\\033[0m \n"
        Log "Input CRL is NULL."
        return 1
    fi 
    rm -f "${COMMAND_RETURN_FILE}"

    #Check CRL file size
    return_file_size=`wc -c "${IMPORT_CRL_FILE}" | ${MYAWK} '{print $1}'`
    if [ ${return_file_size} -gt ${CRL_MAX_SIZE_5KB} ]; then
        echo "File size missed limit."
        printf "\\033[1;31mParse CRL failed.\\033[0m \n"
        Log "CRL file size exceeded."
        return 1
    fi

    #Check CA chain
    "${OPENSSL_FILE}" crl -in "${IMPORT_CRL_FILE}" -CAfile "${CA_FILE}" -noout -verify &>"${COMMAND_RETURN_FILE}"
    return_crl_ca_chain=`cat "${COMMAND_RETURN_FILE}" | grep "verify OK"`
    if [ -z "${return_crl_ca_chain}" ]; then
        echo "The imported CRL does not match CA certificate."
        printf "\\033[1;31mParse CRL failed.\\033[0m \n"
        Log "CRL does not match CA certificate."
        return 1
    fi
    rm -f "${COMMAND_RETURN_FILE}"

    #Check CRL update
    "${OPENSSL_FILE}" verify -crl_check -CRLfile "${IMPORT_CRL_FILE}" -CAfile "${CA_FILE}" "${CERT_FILE}" &>"${COMMAND_RETURN_FILE}"
    return_crl_expired=`cat "${COMMAND_RETURN_FILE}" | grep "CRL has expired"`
    if [ "${return_crl_expired}" ]; then
        echo "The valid time in the input CRL is incorrect."
        printf "\\033[1;31mParse CRL failed.\\033[0m \n"
        Log "Check CRL expiration date failed."
        return 1
    fi
    rm -f "${COMMAND_RETURN_FILE}"

    cp -f "${IMPORT_CRL_FILE}" "${CRL_FILE}"
    chown ${AGENT_USER}:${AGENT_USER} "${CRL_FILE}"
    CHMOD 600 "${CRL_FILE}"
    Log "Parse inport CRLfile succeed."
    return 0
}

#back file(nginx.conf testcfg.tmp server.crl)
BackupFile()
{
    cp -rfp "${TESTCFG_TMP_FILE}" "${TESTCFG_TMP_FILE}.bak"
    cp -rfp "${NGINX_CONF_FILE}" "${NGINX_CONF_FILE}.bak"
    if [ ! -f "${TESTCFG_TMP_FILE}.bak" ] || [ ! -f "${NGINX_CONF_FILE}.bak" ]; then
        echo "Backup related configuration files failed."
        Log "Backup related configuration files failed."
        return 1
    fi
    if [ "${ISIMPORT_FLAG}" == "1" ]; then
        cp -rfp "${CRL_FILE}" "${CRL_FILE}.bak"
        if [ ! -f "${CRL_FILE}.bak" ]; then
            echo "Backup previous CRLfile failed."
            Log "Backup previous CRLfile failed."
            return 1
        fi
    fi
    Log "Backup related configuration files succeed."
    return 0
}

UpdateNginxConf()
{
    if [ ! -f "${NGINX_CONF_FILE}" ]; then
        echo "Can't find nginx.conf."
        Log "Can't find nginx.conf, DataBackup ProtectAgent is damaged."
        return 1
    fi
    
    return_cat_ssl_crl=`cat "${NGINX_CONF_FILE}" | grep "ssl_crl"`
    if [ "${return_cat_ssl_crl}" ]; then
        ${MYAWK} '!/ssl_crl/ {print}' "${NGINX_CONF_FILE}" > "${NGINX_CONF_FILE_BACK}"
        CHMOD --reference=${NGINX_CONF_FILE} ${NGINX_CONF_FILE_BACK}
        chown --reference=${NGINX_CONF_FILE} ${NGINX_CONF_FILE_BACK}
        mv "${NGINX_CONF_FILE_BACK}" "${NGINX_CONF_FILE}"
        if [ "${MODE_TYPE}" == "2" ]; then
            echo "Update nginx.conf successfully."
            Log "Update nginx.conf successfully, in restore certificate."
            return 0
        fi
    else
        if [ "${MODE_TYPE}" == "2" ]; then
            echo "Update nginx.conf failed."
            Log "Update nginx.conf failed, can not find ssl_crl in nginx.conf."
            return 1
        fi
    fi
    
    if [ "${MODE_TYPE}" == "1" ]; then
        ${MYAWK} '1; /ssl_certificate_key/ {print "        ssl_crl server.crl;"}' "${NGINX_CONF_FILE}" > "${NGINX_CONF_FILE_BACK}"
        CHMOD --reference=${NGINX_CONF_FILE} ${NGINX_CONF_FILE_BACK}
        chown --reference=${NGINX_CONF_FILE} ${NGINX_CONF_FILE_BACK}
        mv "${NGINX_CONF_FILE_BACK}" "${NGINX_CONF_FILE}"
        return_cat_ssl_crl=`cat "${NGINX_CONF_FILE}" | grep "ssl_crl"`
        if [ -f "${CRL_FILE}" ] && [ "${return_cat_ssl_crl}" ]; then
            echo "Update nginx.conf successfully."
            Log "Update nginx.conf successfully, in import CRL."
            return 0
        else
            echo "Update Nginx.Conf failed."
            Log "Update Nginx.Conf failed."
            return 1
        fi
    fi
}

UpdateTestcfg()
{
    Log "Update testcfg.tmp started."
    if [ "${MODE_TYPE}" == "1" ]; then
        ${MYAWK} -F= '{
            if(/CERTIFICATE_REVOCATION/) {
                sub(/.*/,1,$2)
                print $1"="$2
            } else {
                print
            }
        }' ${TESTCFG_TMP_FILE} > ${TMP_TESTCFG_TMP_FILE}
        CHMOD --reference=${TESTCFG_TMP_FILE} ${TMP_TESTCFG_TMP_FILE}
        chown --reference=${TESTCFG_TMP_FILE} ${TMP_TESTCFG_TMP_FILE}
        mv ${TMP_TESTCFG_TMP_FILE} ${TESTCFG_TMP_FILE}
    elif [ "${MODE_TYPE}" == "2" ]; then
        ${MYAWK} -F= '{
            if(/CERTIFICATE_REVOCATION/) {
                sub(/.*/,0,$2)
                print $1"="$2
            } else {
                print
            }
        }' ${TESTCFG_TMP_FILE} > ${TMP_TESTCFG_TMP_FILE}
        CHMOD --reference=${TESTCFG_TMP_FILE} ${TMP_TESTCFG_TMP_FILE}
        chown --reference=${TESTCFG_TMP_FILE} ${TMP_TESTCFG_TMP_FILE}
        mv ${TMP_TESTCFG_TMP_FILE} ${TESTCFG_TMP_FILE}
    fi
    echo "Update testcfg.tmp completed."
    Log "Update testcfg.tmp completed."
    return 0
}

ActivateChange()
{
    if [ "${IS_UPGRADE}" == "1" ]; then
        return 0;
    fi
    for i in 1 2 3
    do
        echo "Restart the DataBackup ProtectAgent to make the changes take effect , whether restart DataBackup ProtectAgent immediately?[y/n]"
        echo "  Please enter y/Y (YES) or n/N (NO), default is n."
        read choice
        if [ "${choice}" == "y" ] || [ "${choice}" == "Y" ]; then
            echo "Restart DataBackup ProtectAgent."
            Log "Restart DataBackup ProtectAgent."
            RestartAgent
            if [ $? -ne 0 ]; then
                echo "Restart DataBackup ProtectAgent failed."
                Log "Restart DataBackup ProtectAgent failed."
                break
            fi
            echo "Restart DataBackup ProtectAgent successfully."
            Log "Restart DataBackup ProtectAgent successfully." 
            return 0
        elif [ "${choice}" == "n" ] || [ "${choice}" == "N" ]; then
            break
        else
        echo "  Try $i/3."
        fi
    done
    echo "Execute rollback function."
    Log "Execute rollback function."
    RollBack
    return 1
}

RestartAgent()
{
    Log "Restart ProtectAgent."

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

StopAgent()
{
    ${STOP_AGENT_SCRPT} >/dev/null 2>&1
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
    ${START_AGENT_SCRPT} >/dev/null 2>&1
    if [ $? -eq 0 ]; then
        Log "The DataBackup ProtectAgent service is successfully started."
    else
        echo "The DataBackup ProtectAgent service fails to be started."
        Log "The DataBackup ProtectAgent service fails to be started."
        return 1
    fi
    return 0
}

#restore file(nginx.conf testcfg.tmp server.crl)
RollBack()
{
    cp -rfp "${TESTCFG_TMP_FILE}.bak" "${TESTCFG_TMP_FILE}"
    cp -rfp "${NGINX_CONF_FILE}.bak" "${NGINX_CONF_FILE}"
    if [ "${ISIMPORT_FLAG}" == "1" ]; then
        cp -rfp "${CRL_FILE}.bak" "${CRL_FILE}"
    fi
    return 0
}

PrintHelp()
{
    printf "Valid parameter, params:"
    printf "    option [file path]\n"
    printf "option:\n"
    printf "    -i <CRL file>       import CRL file\n"
    printf "    -u <CRL file>       import CRL file, only used in upgrade Agent\n"
    printf "    -r                  restore certificate\n"
    printf "Usage:\n"
    printf "    sh crl_update.sh -i [CRL path]\n"
    printf "    sh crl_update.sh -r\n"
    return 0
}

ExitHandle()
{
    if [ -f "${COMMAND_RETURN_FILE}" ]; then
        rm -f "${COMMAND_RETURN_FILE}"
    fi

    if [ "${NEED_CLEAN}" == "1" ]; then
        rm -f "${TESTCFG_TMP_FILE}.bak" "${NGINX_CONF_FILE}.bak"
        if [ "${ISIMPORT_FLAG}" == "1" ]; then
            rm -f "${CRL_FILE}.bak"
        fi 
    fi
    
    if [ "$1" == "0" ]; then
        printf "\\033[1;32mScript executed successfully.\\033[0m \n"
        Log "Script executed successfully."
        exit 0
    else
        printf "\\033[1;31mScript execution failed.\\033[0m \n"
        Log "Script execution failed."
        exit 1
    fi
}

#################### Main Process ##########################
printf "\\033[1;32m********************************************************\\033[0m \n"
printf "\\033[1;32m     Start the update of CRL     \\033[0m \n"
printf "\\033[1;32m********************************************************\\033[0m \n"

CheckInstalled
if [ $? -ne 0 ]; then
    ExitHandle 1
fi 

CheckParameter $*
if [ $? -ne 0 ]; then
    ExitHandle 1
fi 

Log "Begin execute script."
Log "The current mode is ${MODE_TYPE}"
Log "Curren OS is ${SYS_NAME}."

CheckOpenssl
if [ $? -ne 0 ]; then
    ExitHandle 1
fi 

BackupFile
if [ $? -ne 0 ]; then
    ExitHandle 1
fi
NEED_CLEAN=1

if [ "${MODE_TYPE}" == "1" ]; then
    CheckCRLIsImported
    if [ $? -ne 0 ]; then
        ExitHandle 1
    fi
    ParseCRL
    if [ $? -ne 0 ]; then
        ExitHandle 1
    fi
fi

UpdateNginxConf
if [ $? -ne 0 ]; then
    ExitHandle 1
fi 

UpdateTestcfg

ActivateChange
if [ $? -ne 0 ]; then
    ExitHandle 1
fi 

ExitHandle 0