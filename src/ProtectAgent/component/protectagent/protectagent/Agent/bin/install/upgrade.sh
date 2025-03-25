#!/bin/sh

##############################################################
# The function of this script is to upgrade the client agent.
##############################################################
PRODUCT_NAME="DataBackup ProtectAgent"
AGENT_USER=rdadmin
AGENT_GROUP=rdadmin
SANCLIENT_USER=sanclient
SANCLIENT_GROUP=sanclient
BACKUP_ROLE_SANCLIENT_PLUGIN=5
SYS_NAME=`uname -s`
SYS_ARCH=""
SYS_BIT=""
CURRENT_PATH=`dirname $0` && cd $CURRENT_PATH && CURRENT_PATH=`pwd`
LOG_FILE_PATH=${CURRENT_PATH}
LOG_FILE_NAME="${CURRENT_PATH}/upgrade.log"
LOG_ERR_FILE="${CURRENT_PATH}/errormsg.log"
SHELL_TYPE_SH="/bin/sh"
RDAGENT_SERVICE_FILE=/lib/systemd/system/rdagent.service
export UPGRADE_FLAG_TEMPORARY="1"
export INSTALL_PACKAGE_PATH=${CURRENT_PATH}
export CRL_STATUS=0

##### upgrade errcode start #####
ERR_UPGRADE_NETWORK_ERROR=1577209877
ERR_UPGRADE_FAIL_SAVE_CONFIGURE=1577209879
ERR_UPGRADE_FAIL_BACKUP=1577209880
ERR_UPGRADE_FAIL_VERIFICATE_PARAMETER=1577209881
ERR_UPGRADE_FAIL_READ_CONFIGURE=1577209882
ERR_UPGRADE_FAIL_GET_SYSTEM_PARAMETERS=1577209883
ERR_UPGRADE_FAIL_START_PROTECT_AGENT=1577209884
ERR_UPGRADE_FAIL_ADD_USER=1577209885
ERR_UPGRADE_FAIL_ADD_USERGROUP=1577209886
ERR_UPGRADE_INSTALL_PACKAGE_INCOMPLETE=1577209876
ERR_UPGRADE_PACKAGE_NO_MATCH_HOST_SYSTEM=1577209878
ERR_UPGRADE_FAIL_STOP_PROTECT_AGENT=1577209887
ERR_UPGRADE_HOST_MISS_TOOL_PACKAGE=1577209888
ERR_UPGRADE_FAIL_REGISTER_TO_PM=1577209889
ERR_UPGRADE_FAIL_VERIFICATE_CERTIFICATE=1577209890
ERR_UPGRADE_FAIL_PORT_OCCUPY=1577209891
ERROR_OS_NOT_SUPPORT=1577266689

ERR_UPGRADE_PKG_VERSION_LOW=1577210139
ERR_IPADDR_SET_FAILED=1577210136
ERR_IPADDR_SET_FAILED_RETCODE=76
##### upgrade errcode end #####


###### SET UMASK ######
umask 0022

if [ "$SYS_NAME" = "SunOS" ]; then
    MYAWK=nawk
    AGENT_USER_PATH="/export/home"
    MYGREP=/usr/xpg4/bin/grep
else
    MYAWK=awk
    AGENT_USER_PATH="/home"
    MYGREP=grep
fi

if [ "`ls -al /bin/sh | ${MYAWK} '{print $NF}'`" = "bash" ]; then
    EXPORT_ENV="if [ -f ~/.profile ]; then . ~/.profile; fi;"
else
    EXPORT_ENV=""
fi

###### Custom installation directory ######
CHECK_DIR=/opt
TMP_DATA_BACKUP_AGENT_HOME=`cat /etc/profile | grep "DATA_BACKUP_AGENT_HOME=" |${MYAWK} -F "=" '{print $NF}'`
TMP_DATA_BACKUP_SANCLIENT_HOME=`cat /etc/profile | grep "export DATA_BACKUP_SANCLIENT_HOME=" |${MYAWK} -F "=" '{print $NF}'`
if [ -n "${TMP_DATA_BACKUP_AGENT_HOME}" ] || [ -n "${TMP_DATA_BACKUP_SANCLIENT_HOME}" ] ; then
    . /etc/profile
    if [ -n "${DATA_BACKUP_AGENT_HOME}" ] || [ -n "${DATA_BACKUP_SANCLIENT_HOME}" ] ; then
        DATA_BACKUP_AGENT_HOME=${TMP_DATA_BACKUP_AGENT_HOME}
        DATA_BACKUP_SANCLIENT_HOME=${TMP_DATA_BACKUP_SANCLIENT_HOME}
        export DATA_BACKUP_AGENT_HOME DATA_BACKUP_SANCLIENT_HOME
    fi
else
    DATA_BACKUP_AGENT_HOME=/opt
    DATA_BACKUP_SANCLIENT_HOME=/opt
    ENV_IS_NOT_EXIST=1
    export DATA_BACKUP_AGENT_HOME DATA_BACKUP_SANCLIENT_HOME
fi

###### The lower level may need to ######
if [ -d "/opt/OceanProtect/ProtectClient/ProtectClient-E/bin" ]; then
    AGENT_ROOT_PATH=/opt/OceanProtect/ProtectClient
    BACKUP_COPY_DIR=/opt/OceanProtect/Bak
    NEW_INSTALL_PATH=/opt/DataBackup/ProtectClient
else
    AGENT_ROOT_PATH=${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient
    BACKUP_COPY_DIR=${DATA_BACKUP_AGENT_HOME}/DataBackup/Bak
    NEW_INSTALL_PATH=${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient
fi

UPGRADE_BACKUP_PATH=${DATA_BACKUP_AGENT_HOME}/AgentUpgrade
PUSH_UPGRADE_PACKAGE_PATH=${DATA_BACKUP_AGENT_HOME}/upgrade

. ${CURRENT_PATH}/func_util.sh
CLIENT_BACK_ROLE=`cat ${CURRENT_PATH}/conf/client.conf | grep "backup_role" | ${MYAWK} -F '=' '{print $NF}'`
if [ "${CLIENT_BACK_ROLE}" = "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ] ; then
    AGENT_USER=${SANCLIENT_USER}
    AGENT_GROUP=${SANCLIENT_GROUP}
    PUSH_UPGRADE_PACKAGE_PATH=${DATA_BACKUP_SANCLIENT_HOME}/upgradeSanclient
    AGENT_ROOT_PATH="${DATA_BACKUP_SANCLIENT_HOME}/DataBackup/SanClient"
    UPGRADE_BACKUP_PATH="${DATA_BACKUP_SANCLIENT_HOME}/SanClientUpgrade"
    BACKUP_COPY_DIR="${DATA_BACKUP_SANCLIENT_HOME}/DataBackup/SanClientBak"
    NEW_INSTALL_PATH="${DATA_BACKUP_SANCLIENT_HOME}/DataBackup/SanClient"
fi
OLD_INSTALL_PATH=${AGENT_ROOT_PATH}
export OLD_INSTALL_PATH
LOG_ERR_FILE_STUB="${AGENT_ROOT_PATH}/ProtectClient-E/tmp/errormsg.log"
if [ -d "${BACKUP_COPY_DIR}" ]; then
    LATEST_DIR=`ls -t ${BACKUP_COPY_DIR} | head -n 1`
fi
LATEST_BAK_DIR=${BACKUP_COPY_DIR}/${LATEST_DIR}

AGENT_HAVE_BEEN_STOPED_FLAG=0

########################################################################################
# Function Definition
########################################################################################
GetOsVersion()
{
    if [ "${TESTCFG_BACK_ROLE}" = "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ]; then
        GetSanCientOsVersion
        return $?
    fi

    case ${SYS_NAME} in
    "Linux")
        SYS_ARCH=`arch`
        if [ "${SYS_ARCH}" = "x86_64" ] || [ "${SYS_ARCH}" = "aarch64" ]; then
            SYS_BIT="64"
        elif [ "${SYS_ARCH}" = "x86" ] || [ "${SYS_ARCH}" = "aarch32" ]; then
            SYS_BIT="32"
        fi
        return 0
    ;;
    "HP-UX")
        SYS_ARCH=`uname -m`
        if [ ${SYS_ARCH} = "ia64" ]; then
            SYS_BIT="64"
        else
            SYS_BIT="32"
        fi
        return 0
    ;;
    "AIX")
        SYS_BIT=`getconf HARDWARE_BITMODE`
        if [ ${SYS_BIT} = "64" ]; then
            SYS_ARCH="x86_64"
        else
            SYS_ARCH="x86"
        fi
        return 0
    ;;
    "SunOS")
        SYS_BIT=`isainfo -b`
        if [ ${SYS_BIT} = "64" ]; then
            SYS_ARCH="x86_64"
        else
            SYS_ARCH="x86"
        fi
        return 0
    ;;
    *)
        return 1
    ;;
    esac
}

PrintHelp()
{
    printf "\033[31mValid parameter, params:\033[0m\n"
    printf "mode: push installation\n"
    printf "eg:\n"
    printf "\033[31msh upgrade.sh\033[0m\n"
    printf "\033[31msh upgrade.sh -mode push\033[0m\n"
}

CheckInputParams()
{
    UPGRADE_MODE=common
    if [ $# -eq 0 ]; then
        return 0
    elif [ $# -eq 1 ] && [ $1 = "-h" ]; then
        PrintHelp
        return 1
    elif [ $# -eq 2 ] && [ $1 = "-mode" ] && [ $2 = "push" ]; then
        UPGRADE_MODE=push
        return 0
    else
        PrintHelp
        return 1
    fi
}

CHMOD()
{
    source=
    arg2=
    arg=
    if [ $# -eq 3 ]; then
        arg=$1
        arg2=$2
        source=$3
    elif [ $# -eq 2 ]; then
        source=$2
        arg2=$1
    fi
    if [ -L "$source" ]; then
        Log "source file  is a link file can not copy."
        return
    fi
    chmod $arg $arg2 $source  >>${LOG_FILE_NAME} 2>&1
}

CP()
{
    # High to Low, Determining Linked Files
    eval LAST_PARAM=\$$#
    TARGET_FILE=${LAST_PARAM}
    if [ -L "${TARGET_FILE}" ]; then
        echo "The target file is a linked file. Exit the current process."
        Log "The target file is a linked file. Exit the current process."
        exit 1
    fi

    if [ "$SYS_NAME" = "SunOS" ]; then
        cmd="cp -R -P $*"
    else
        cmd="cp -d $*"
    fi
    ${cmd}
}

LogError()
{
    #$1=errorInfo,$2=errorCode, $3=errorDetailParam
    LogErr "$1" "$2" "job_log_agent_storage_update_install_fail_label" "$3"
}

LogErr() 
{
    #$1=errorInfo,$2=errorCode,$3=errorLable,$4=errorDetailParam
    Log "[ERR] $1"
    if [ $# -ge 3 ]; then
        LogErrDetail "$2" "$3" "$4"
    fi
}

LogErrDetail()
{
    if [ ! -f "${LOG_ERR_FILE}" ]; then
        touch "${LOG_ERR_FILE}"
        chmod 604 "${LOG_ERR_FILE}" 
    fi
    echo "logDetail=$1" > ${LOG_ERR_FILE}
    echo "logInfo=$2" >> ${LOG_ERR_FILE}
    echo "logDetailParam=$3" >> ${LOG_ERR_FILE}
}

Log()
{
    if [ ! -f "${LOG_FILE_NAME}" ]; then
        touch "${LOG_FILE_NAME}"
        CHMOD 600 "${LOG_FILE_NAME}"
    fi

    DATE=`date +%y-%m-%d--%H:%M:%S`
    if [ "SunOS" = "$SYS_NAME" ]; then
        command -v whoami >/dev/null
        if [ $? -eq 0 ]; then
            USER_NAME=`whoami`
        else
            USER_NAME=`/usr/ucb/whoami`
        fi
    else
        USER_NAME=`whoami`
    fi

    echo "[${DATE}][$$][${USER_NAME}] $1" >> "${LOG_FILE_NAME}"
}

AdaptClientPackage()
{ 
    if [ -d "${CURRENT_PATH}/ProtectClient-e" ]; then
        cd "${CURRENT_PATH}/ProtectClient-e"
    else
        echo "Not found ProtectClient-e directory."
        return 1
    fi
    system_name=""
    if [ $SYS_NAME = "Linux" ]; then
        cat /etc/system-release 2>/dev/null | grep CentOS >>/dev/null 2>&1
        if [ $? -eq 0 ]; then
            system_name="Centos"
        fi
    fi

    if [ "${BACKUP_ROLE}" == "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ];then
        FILE_PATH="sanclient-Linux-x86_64.tar.xz"
    fi

    # Level-by-level filtering by priority
    for cparm in SYS_NAME SYS_ARCH SYS_BIT
    do
        case "$cparm" in
            "SYS_NAME")
                FILE_COUNT=`ls $pwd | grep "${SYS_NAME}" | wc -l`
                if [ $FILE_COUNT -eq 1 ]; then
                    FILE_PATH=`ls $pwd | grep "${SYS_NAME}"`
                    Log "The current system is ${SYS_NAME},adaptation succeeded."
                    break
                elif [ $FILE_COUNT -eq 0 ]; then
                    echo "The current system is ${SYS_NAME},no proper installation package is found."
                    Log "The current system is ${SYS_NAME},no proper installation package is found."
                    return 1
                else
                    continue
                fi
                ;;
            "SYS_ARCH")
                if [ $SYS_ARCH = "aarch64" ]; then
                    if [ "$system_name" = "Centos" ]; then
                        FILE_COUNT=`ls $pwd | grep "${SYS_NAME}" | grep $SYS_ARCH | grep "Centos" | wc -l`
                    else
                        FILE_COUNT=`ls $pwd | grep "${SYS_NAME}" | grep $SYS_ARCH | grep -v "Centos" | wc -l`
                    fi
                else
                    FILE_COUNT=`ls $pwd | grep "${SYS_NAME}" | grep $SYS_ARCH | wc -l`
                fi
                if [ $FILE_COUNT -eq 1 ]; then
                    if [ $SYS_ARCH = "aarch64" ]; then
                        if [ "$system_name" = "Centos" ]; then
                            FILE_PATH=`ls $pwd | grep "${SYS_NAME}" | grep $SYS_ARCH | grep "Centos"`
                        else
                            FILE_PATH=`ls $pwd | grep "${SYS_NAME}" | grep $SYS_ARCH | grep -v "Centos"`
                        fi
                    else
                        FILE_PATH=`ls $pwd | grep "${SYS_NAME}" | grep $SYS_ARCH`
                    fi
                    Log "The current system is ${SYS_NAME},adaptation succeeded."
                    break
                elif [ $FILE_COUNT -eq 0 ]; then
                    echo "Check whether the name of the installation package is correct."
                    Log "Check whether the name of the installation package is correct."
                    return 1
                else
                    continue
                fi
                ;;
            "SYS_BIT")
                FILE_COUNT=`ls $pwd | grep "${SYS_NAME}" | grep $SYS_ARCH | grep $SYS_BIT | wc -l`
                if [ $FILE_COUNT -eq 1 ]; then
                    FILE_PATH=`ls $pwd | grep "${SYS_NAME}" | grep $SYS_ARCH | grep $SYS_BIT`
                    Log "The current system is ${SYS_NAME},adaptation succeeded."
                    break
                elif [ $FILE_COUNT -eq 0 ]; then
                    echo "Check whether the name of the installation package is correct."
                    Log "Check whether the name of the installation package is correct."
                    return 1
                else
                    continue
                fi
                ;;
            *)
                continue
                ;;
        esac
    done
    Log "The current system is ${SYS_NAME},adaptation succeeded."
    return 0
}

ModifyServiceFile()
{
    # obtain the service config from service file
    if [ -f "/etc/os-release" ]; then
        cat /etc/os-release | grep "EulerOS 2.0"
        if [ $? = 0 ] && [ -f "${RDAGENT_SERVICE_FILE}" ]; then
            cat ${RDAGENT_SERVICE_FILE} | grep 'KillMode=' >/dev/null 2>&1
            if [ $? -ne 0 ]; then
                sed -i 's/\[Service\]/\[Service\]\nKillMode=process/g' ${RDAGENT_SERVICE_FILE}
                systemctl daemon-reload
            fi
        fi
    fi
    return
}

CompareUpdateVersion()
{
    # obtain old version of agent
    OLD_UPDATE_VERSION=`cd "${OLD_INSTALL_PATH}/ProtectClient-E/conf"; find ./ -name version|xargs sed -n "3,3p"`
    if [ -d "${CURRENT_PATH}/ProtectClient-e" ]; then
        cd "${CURRENT_PATH}/ProtectClient-e"
    else
        echo "Not found ProtectClient-e directory."
        return 1
    fi

    # obtain new version of agent
    mkdir -p ${CURRENT_PATH}/ProtectClient-e/DIR_TMP
    CHMOD 750 ${CURRENT_PATH}/ProtectClient-e/DIR_TMP

    if [ "${SYS_NAME}" = "Linux" ]; then
        SYS_ARCH=`arch`
    else
        SYS_ARCH=""
    fi
    system_name=""
    if [ $SYS_NAME = "Linux" ]; then
        cat /etc/system-release 2>/dev/null | grep CentOS >>/dev/null 2>&1
        if [ $? -eq 0 ]; then
            system_name="Centos"
        fi
    fi
    if [ "${SYS_ARCH}" = "aarch32" ] || [ "${SYS_ARCH}" = "aarch64" ] || [ "${SYS_ARCH}" = "x86_64" ] || [ "${SYS_ARCH}" = "x86" ]; then
        if [ "$system_name" = "Centos" ] && [ "${SYS_ARCH}" = "aarch64" ]; then
            PAC_NAME_TMP=`ls $pwd | grep "tar.xz" | grep "$SYS_NAME" | grep "${SYS_ARCH}" | grep Centos`
        else
            PAC_NAME_TMP=`ls $pwd | grep "tar.xz" | grep "$SYS_NAME" | grep "${SYS_ARCH}" | grep -v Centos`
        fi
    else
        PAC_NAME_TMP=`ls $pwd | grep "tar.xz" | grep "$SYS_NAME"`
    fi

    if [ "$SYS_NAME" = "Linux" ]; then
        tar -xf $PAC_NAME_TMP -C ${CURRENT_PATH}/ProtectClient-e/DIR_TMP
    elif [ "$SYS_NAME" = "SunOS" ]; then
        Temporary_Path=`pwd`
        PAC_NAME_TMP=`ls $pwd | grep "tar.gz" | grep "$SYS_NAME"`
        cp $PAC_NAME_TMP ${CURRENT_PATH}/ProtectClient-e/DIR_TMP
        cd ${CURRENT_PATH}/ProtectClient-e/DIR_TMP
        gzip -d ${PAC_NAME_TMP}
        packName=`ls | grep protect*.tar`
        tar -xf ${packName}
        cd ${Temporary_Path}
    else
        Temporary_Path=`pwd`
        cp $PAC_NAME_TMP ${CURRENT_PATH}/ProtectClient-e/DIR_TMP
        cd ${CURRENT_PATH}/ProtectClient-e/DIR_TMP
        chmod 550 ${INSTALL_PACKAGE_PATH}/third_party_software/XZ/xz
        ${INSTALL_PACKAGE_PATH}/third_party_software/XZ/xz -d ${PAC_NAME_TMP}
        packName=${PAC_NAME_TMP%%.xz}
        tar -xf ${packName}
        cd $Temporary_Path
    fi

    NEW_UPDATE_VERSION=`find ${CURRENT_PATH}/ProtectClient-e/DIR_TMP -name version |xargs sed -n "3,3p"`
    if [ -d "${CURRENT_PATH}/ProtectClient-e/DIR_TMP" ]; then
        rm -rf "${CURRENT_PATH}/ProtectClient-e/DIR_TMP"
    fi

    cd $CURRENT_PATH
    # compare the version
    if [ $NEW_UPDATE_VERSION -gt $OLD_UPDATE_VERSION ]; then
        Log "Old version[$OLD_UPDATE_VERSION] < new version[$NEW_UPDATE_VERSION], continue to upgrade."
        return 0
    else
        Log "Old version is higher than the new version, failed to upgrade."
        return 1
    fi
}

IpAddrVerify()
{
    ipAddr=$1
    echo "$ipAddr" | ${MYGREP} -E "^([a-fA-F0-9]{1,4}(:|\.)){0,7}(:|::)?([a-fA-F0-9]{1,4}(:|\.)){0,7}([a-fA-F0-9]{1,4})?$" >/dev/null 2>&1
    if [ "$?" = "0" ];then
        Log "$ipAddr verify success."
        return 0
    else
        Log "$ipAddr verify failed."
        return 1
    fi
}

VerifyAndPing()
{
    ipAddr=$1
    IpAddrVerify $ipAddr
    if [ $? -ne 0 ]; then
        Log "$ipAddr verify failed."
        return 1
    fi
    echo ${ipAddr} | grep "\\:" >/dev/null 2>&1
    if [ 0 -eq $? ]; then
        if [ "${SYS_NAME}" = "SunOS" ]; then
            ping6 -s ${ipAddr} 56 1 > /dev/null 2>&1
        else
            ping6 -c 1 -w 3 ${ipAddr} > /dev/null 2>&1
        fi
        return $?
    fi
    echo ${ipAddr} | grep "\\." >/dev/null 2>&1
    if [ 0 -eq $? ]; then
        if [ "${SYS_NAME}" = "SunOS" ]; then
            ping -s ${ipAddr} 56 1 > /dev/null 2>&1
        else
            ping -c 1 -w 3 ${ipAddr} > /dev/null 2>&1
        fi
        return $?
    fi
    return 1
}

# Ping ip1,ip2,ip3
Ping()
{
    ipList="$1"
    if [ "$ipList" = "" ]; then
        echo "The input ip address is empty."
        return 1
    fi
    index=1      # the index of awk -v
    for ipAddr in ${ipList}; do
        if [ "${ipAddr}" = "" ]; then
            break
        fi
        VerifyAndPing "$ipAddr"
        if [ $? -eq 0 ]; then
            return 0
        fi
        index=`expr $index + 1`
    done
    ShowWarning "Local ip can not connect remote ips ${ipList}."
    Log "Check connection to dst ${ipList} fail."
    return 1
}

# $1: dst_ips  $2: port
CheckIpsWithCurl()
{
    dstIps=$1
    dstPort=$2
    Log "will check dstIps $dstIps dstport $dstPort"

    for dstIp in ${dstIps}; do
        protocolFamily="--ipv4"
        echo ${dstIp} | grep "\\." >/dev/null 2>&1
        if [ $? -eq 0 ]; then
            G_NETWORK_TYPE="ipv4"
        else
            G_NETWORK_TYPE="ipv6"
            protocolFamily="--ipv6"
        fi

        curl -kv ${dstIp}:${dstPort} --connect-timeout 3 ${protocolFamily} >/dev/null 2>&1
        if [ $? -eq 0 ]; then
            Log "Check connection dst ${dstIp} port $dstPort success."
            return 0
        fi
        Log "Check connection to dst ${dstIp} port $dstPort fail."
    done
    ShowWarning "Local ip can not connect remote ips ${dstIps} port $dstPort."
    Log "Local ip can not connect remote ips ${dstIps} port $dstPort."
    return 1
}

# $1: dst_ips  $2: port
TestHosts()
{
    dstIps=$1
    dstPort=$2
    Log "will test dstIps $dstIps dstport $dstPort"

    for dstIp in ${dstIps}; do
        SUExecCmd "${AGENT_ROOT_PATH}/ProtectClient-E/bin/agentcli testhost ${dstIp} $dstPort 5000"
        if [ $? -eq 0 ]; then
            Log "Check connection dst ${dstIp} port $dstPort success."
            return 0
        fi
        Log "Check connection to dst ${dstIp} port $dstPort fail."
    done
    ShowWarning "Local ip can not connect remote ips ${dstIps} port $dstPort."
    Log "Local ip can not connect remote ips ${dstIps} port $dstPort."
    return 1
}

# $1 dstIps, $2 dstPort
CheckIpsConnectivity()
{
    which curl >/dev/null 2>&1
    if [ $? -eq 0 ]; then
        CheckIpsWithCurl "$1" "$2"
        return $?
    else
        if [ "$2" = "" ] || [ "$2" = "null" ]; then
            Ping "$1"
        else
            TestHosts "$1" "$2"
        fi
        return $?
    fi
}

# Check connectivity to avoid the installation failure of the new client
CheckConnectivity()
{
    PM_IP_LIST=`sed '/^PM_IP=/!d;s/.*=//' ${OLD_INSTALL_PATH}/ProtectClient-E/conf/testcfg.tmp`
    PM_PORT=`sed '/^PM_PORT=/!d;s/.*=//' ${OLD_INSTALL_PATH}/ProtectClient-E/conf/testcfg.tmp`
    CheckIpsConnectivity "`echo ${PM_IP_LIST} | sed 's/,/ /g'`"  "${PM_PORT}"
    if [ $? -eq 0 ]; then
        return 0
    fi

    PM_MANAGER_IP_LIST=`sed '/^PM_MANAGER_IP=/!d;s/.*=//' ${OLD_INSTALL_PATH}/ProtectClient-E/conf/testcfg.tmp`
    PM_MANAGER_PORT=`sed '/^PM_MANAGER_PORT=/!d;s/.*=//' ${OLD_INSTALL_PATH}/ProtectClient-E/conf/testcfg.tmp`
    CheckIpsConnectivity "`echo ${PM_MANAGER_IP_LIST} | sed 's/,/ /g'`"  "${PM_MANAGER_PORT}"
    if [ $? -eq 0 ]; then
        return 0
    fi

    return 1
}

CheckAgentIP()
{
    tmp_agent_ip=`cat ${CURRENT_PATH}/conf/client.conf | grep "AGENT_IP"`
    if [ -z "${tmp_agent_ip}" ]; then
        ISIPV6=`cat ${OLD_INSTALL_PATH}/ProtectClient-E/nginx/conf/nginx.conf | grep "listen" | grep "]"`
        if [ "${ISIPV6}" != "" ]; then
            AGENT_IP=`cat ${OLD_INSTALL_PATH}/ProtectClient-E/nginx/conf/nginx.conf |grep listen |${MYAWK} '{print $2}' | cut -d ']' -f 1 | cut -d '[' -f 2`
        else
            AGENT_IP=`cat ${OLD_INSTALL_PATH}/ProtectClient-E/nginx/conf/nginx.conf |grep listen |${MYAWK} '{print $2}' |${MYAWK} -F ":" '{print $1}'`
        fi
        if [ -z "${AGENT_IP}" ]; then
            Log "Failed to obtain the IP address of the agent."
            return 1
        fi

        echo AGENT_IP=${AGENT_IP} | tr -d '\r' >> ${CURRENT_PATH}/conf/client.conf
        Log "The ip address of the agent is configured successfully.AGENT_IP is:${AGENT_IP}."
    else
        Log "The ip address of the agent already exists."
    fi
    return 0
}

CertificateSystemUpdateAdaptation()
{   
    if [ ! -f "${OLD_INSTALL_PATH}/ProtectClient-E/nginx/conf/pmca.pem" ]; then
        if [ -f "${OLD_INSTALL_PATH}/ProtectClient-E/nginx/conf/bcmagentca.pem" ]; then
            cp -p -f "${OLD_INSTALL_PATH}/ProtectClient-E/nginx/conf/bcmagentca.pem" "${OLD_INSTALL_PATH}/ProtectClient-E/nginx/conf/pmca.pem"
            cp -p -f "${OLD_INSTALL_PATH}/ProtectClient-E/nginx/conf/bcmagentca.pem" "${OLD_INSTALL_PATH}/ProtectClient-E/nginx/conf/agentca.pem"
        elif [ -f "${OLD_INSTALL_PATH}/ProtectClient-E/bin/nginx/conf/bcmagentca.pem" ]; then
            cp -p -f "${OLD_INSTALL_PATH}/ProtectClient-E/bin/nginx/conf/bcmagentca.pem" "${OLD_INSTALL_PATH}/ProtectClient-E/bin/nginx/conf/pmca.pem"
            cp -p -f "${OLD_INSTALL_PATH}/ProtectClient-E/bin/nginx/conf/bcmagentca.pem" "${OLD_INSTALL_PATH}/ProtectClient-E/bin/nginx/conf/agentca.pem"
        fi
    fi
    if [ ! -f "${OLD_INSTALL_PATH}/ProtectClient-E/nginx/conf/pmca.pem" ] && [ ! -f "${OLD_INSTALL_PATH}/ProtectClient-E/bin/nginx/conf/pmca.pem" ]; then
        echo "Compatible certificate system upgrade failed."
        Log "Compatible certificate system upgrade failed."
        return 1
    else
        echo "Compatible certificate system upgrade succeed."
        Log "Compatible certificate system upgrade succeed."
        return 0
    fi
}

BackupInstalledPac()
{
    if [ -d "$UPGRADE_BACKUP_PATH" ]; then
        rm -rf "$UPGRADE_BACKUP_PATH"
    fi
    mkdir $UPGRADE_BACKUP_PATH
    #欧拉系统升级适配
    chmod 755 $UPGRADE_BACKUP_PATH
    
    # 证书独立需求后版本兼容之前版本升级
    CertificateSystemUpdateAdaptation
    if [ $? -ne 0 ]; then
        return 1
    fi

    # save CRL status
    return_get_CRL_status=`cat "${OLD_INSTALL_PATH}/ProtectClient-E/conf/testcfg.tmp" | grep "CERTIFICATE_REVOCATION" | ${MYAWK} -F '=' '{print $2}'`
    if [ "${return_get_CRL_status}" == "1" ]; then
        CRL_STATUS=1;
    fi
    Log "CRL status is ${CRL_STATUS}."

    if [ ! -d "$OLD_INSTALL_PATH" ]; then
        Log "Not install the DataBackup ProtectAgent, failed to upgrade."
        return 1
    else
        return 0
    fi
}

ClearUselessPackage()
{
    if [ $# -eq 0 ]; then
        rm -rf "$UPGRADE_BACKUP_PATH"
        # clear push pkg
        if [ "${UPGRADE_MODE}" = "push" ]; then
            if [ "${CLIENT_BACK_ROLE}" = "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ] && [ -d "${DATA_BACKUP_SANCLIENT_HOME}/upgradeSanclient" ]; then
                rm -rf "${DATA_BACKUP_SANCLIENT_HOME}/upgradeSanclient"
            elif [ -d "${PUSH_UPGRADE_PACKAGE_PATH}" ]; then
                rm -rf "${PUSH_UPGRADE_PACKAGE_PATH}"
            fi
        fi
    elif [ $# -eq 1 ]; then
        # AIX child directory script cannot delete parent
        cd ${CURRENT_PATH}
        CLEAR_PATH=$1
        Log "Clear useless package [$CLEAR_PATH]."
        rm -rf "$CLEAR_PATH"
    else
        Log "Clear package wrong param."
    fi

    return 0
}

CopyLogFile()
{
    DST_DIR=$1
    # Directory-based judgment is used to ensure interface universality.
    if [ -d "${DST_DIR}/ProtectClient-E/slog" ]; then
        [ -f "${LOG_FILE_PATH}/upgrade.log" ] && mv "${LOG_FILE_PATH}/upgrade.log" "${DST_DIR}/ProtectClient-E/slog"
        [ -f "${LOG_FILE_PATH}/agent_upgrade.log" ] && mv "${LOG_FILE_PATH}/agent_upgrade.log" "${DST_DIR}/ProtectClient-E/slog"
        [ -f "${LOG_FILE_PATH}/agent_install_in_upgrade.log" ] && mv "${LOG_FILE_PATH}/agent_install_in_upgrade.log" "${DST_DIR}/ProtectClient-E/slog"
        [ -f "${LOG_FILE_PATH}/agent_upgrade_sqlite.log" ] && mv "${LOG_FILE_PATH}/agent_upgrade_sqlite.log" "${DST_DIR}/ProtectClient-E/slog"
        # copy push upgrade logs
        if [ "${UPGRADE_MODE}" = "push" ]; then
            [ -f "${PUSH_UPGRADE_PACKAGE_PATH}/upgrade_pre.log" ] && mv "${PUSH_UPGRADE_PACKAGE_PATH}/upgrade_pre.log" "${DST_DIR}/ProtectClient-E/slog"
        fi
    elif [ -d "${DST_DIR}/ProtectClient-E" ]; then
        mkdir -p "${DST_DIR}/ProtectClient-E/slog"
        [ -f "${LOG_FILE_PATH}/upgrade.log" ] && mv "${LOG_FILE_PATH}/upgrade.log" "${DST_DIR}/ProtectClient-E/slog"
        [ -f "${LOG_FILE_PATH}/agent_upgrade.log" ] && mv "${LOG_FILE_PATH}/agent_upgrade.log" "${DST_DIR}/ProtectClient-E/slog"
        [ -f "${LOG_FILE_PATH}/agent_install_in_upgrade.log" ] && mv "${LOG_FILE_PATH}/agent_install_in_upgrade.log" "${DST_DIR}/ProtectClient-E/slog"
        [ -f "${LOG_FILE_PATH}/agent_upgrade_sqlite.log" ] && mv "${LOG_FILE_PATH}/agent_upgrade_sqlite.log" "${DST_DIR}/ProtectClient-E/slog"
        # copy push upgrade logs
        if [ "${UPGRADE_MODE}" = "push" ]; then
            [ -f "${PUSH_UPGRADE_PACKAGE_PATH}/upgrade_pre.log" ] && mv "${PUSH_UPGRADE_PACKAGE_PATH}/upgrade_pre.log" "${DST_DIR}/ProtectClient-E/slog"
        fi
    fi

    if [ -d "${DST_DIR}/ProtectClient-E/stmp" ]; then
        [ -f "${LOG_FILE_PATH}/errormsg.log" ] && mv "${LOG_FILE_PATH}/errormsg.log" "${DST_DIR}/ProtectClient-E/stmp"
        [ -f "${DST_DIR}/ProtectClient-E/stmp/errormsg.log" ] && CHMOD 640 "${DST_DIR}/ProtectClient-E/stmp/errormsg.log"
        [ -f "${DST_DIR}/ProtectClient-E/stmp/errormsg.log" ] && chown root:${AGENT_GROUP} "${DST_DIR}/ProtectClient-E/stmp/errormsg.log"
    else
        mkdir -p "${DST_DIR}/ProtectClient-E/stmp"
        [ -f "${LOG_FILE_PATH}/errormsg.log" ] && mv "${LOG_FILE_PATH}/errormsg.log" "${DST_DIR}/ProtectClient-E/stmp"
        [ -f "${DST_DIR}/ProtectClient-E/stmp/errormsg.log" ] && CHMOD 640 "${DST_DIR}/ProtectClient-E/stmp/errormsg.log"
        [ -f "${DST_DIR}/ProtectClient-E/stmp/errormsg.log" ] && chown root:${AGENT_GROUP} "${DST_DIR}/ProtectClient-E/stmp/errormsg.log"
    fi
}

CheckAndDeletEnv()
{
    # 升级失败，判断是否需要清理环境变量
    UPGRADE_EXIT_CODE=$1
    if [ "${UPGRADE_EXIT_CODE}" != "0" ] && [ "${ENV_IS_NOT_EXIST}" = "1" ]; then
        envName=DATA_BACKUP_AGENT_HOME
        if [ "${SYS_NAME}" = "AIX" ]; then
            sed "/export ${envName}=\//d" /etc/profile > /etc/profile.bak
            mv -f /etc/profile.bak /etc/profile
        elif [ "${SYS_NAME}" = "SunOS" ]; then
            sed "/${envName}=/d" /etc/profile > /etc/profile.bak1
            sed "/export ${envName}/d" /etc/profile.bak1 > /etc/profile.bak
            mv -f /etc/profile.bak /etc/profile
            rm -rf /etc/profile.bak1
        else
            sed -i "/export ${envName}=\//d" /etc/profile
        fi
    fi
}

ExitHandle()
{
    EXIT_CODE=$1

    # add log to upgrade.log
    if [ ${EXIT_CODE} -ne 0 ]; then
        echo "Failed to upgrade the Agent on the host"
        Log "Failed to upgrade the Agent on the host."
    else
        Log "DataBackup ProtectAgent is successfully upgraded on the host."

        if [ -d "${LATEST_BAK_DIR}/log" ]; then
          [ -f "${LATEST_BAK_DIR}/log/agentcli.log" ] && cat "${LATEST_BAK_DIR}/log/agentcli.log" >> "${NEW_INSTALL_PATH}/ProtectClient-E/log/agentcli.log"
          [ -f "${LATEST_BAK_DIR}/log/agent_start.log" ] && cat "${LATEST_BAK_DIR}/log/agent_start.log" >> "${NEW_INSTALL_PATH}/ProtectClient-E/log/agent_start.log"
          [ -f "${LATEST_BAK_DIR}/log/agent_stop.log" ] && cat "${LATEST_BAK_DIR}/log/agent_stop.log" >> "${NEW_INSTALL_PATH}/ProtectClient-E/log/agent_stop.log"
          [ -f "${LATEST_BAK_DIR}/log/agent_upgrade_sqlite.log" ] && cat "${LATEST_BAK_DIR}/log/agent_upgrade_sqlite.log" >> "${NEW_INSTALL_PATH}/ProtectClient-E/log/agent_upgrade_sqlite.log"
        fi

        if [ -d "${LATEST_BAK_DIR}/slog" ]; then
          [ -f "${LATEST_BAK_DIR}/slog/upgrade.log" ] && cat "${LATEST_BAK_DIR}/slog/upgrade.log" >> ${LOG_FILE_NAME}
          [ -f "${LATEST_BAK_DIR}/slog/agent_firewall_tool.log" ] && cat "${LATEST_BAK_DIR}/slog/agent_firewall_tool.log" >> "${NEW_INSTALL_PATH}/ProtectClient-E/slog/agent_firewall_tool.log"
          [ -f "${LATEST_BAK_DIR}/slog/gethostos.log" ] && cat "${LATEST_BAK_DIR}/slog/gethostos.log" >> "${NEW_INSTALL_PATH}/ProtectClient-E/slog/gethostos.log"
          [ -f "${LATEST_BAK_DIR}/slog/mountnasfilesystem.log" ] && cat "${LATEST_BAK_DIR}/slog/mountnasfilesystem.log" >> "${NEW_INSTALL_PATH}/ProtectClient-E/slog/mountnasfilesystem.log"
        fi

    fi

    # need restart agent if upgrade fail
    if [ ${EXIT_CODE} -ne 0 ] && [ "${AGENT_HAVE_BEEN_STOPED_FLAG}" = "1" ]; then
        Log "Need restart agent."
        $OLD_INSTALL_PATH/start.sh
        if [ $? = 0 ]; then
            Log "Restart agent success."
        else
            Log "Restart agent fail."
        fi
    else
        Log "No need restart agent."
    fi

    # copy logs to upgrade dir
    if [ "${EXIT_CODE}" = "0" ]; then
        CopyLogFile "${NEW_INSTALL_PATH}"
    else
        CopyLogFile "${OLD_INSTALL_PATH}"
    fi

    # clear upgrade package
    ClearUselessPackage

    CheckAndDeletEnv ${EXIT_CODE}

    # upgrade mode
    if [ "${UPGRADE_MODE}" = "common" ]; then
        exit ${EXIT_CODE}
    elif [ "${UPGRADE_MODE}" = "push" ]; then  # UPGRADE_STATUS: 0-failure 1-success 3-intermediate 8-abnormal 9-initial
        if [ "${EXIT_CODE}" = "0" ]; then
            # success
            if [ "${SYS_NAME}" = "SunOS" ] || [ "${SYS_NAME}" = "AIX" ]; then
                su - ${AGENT_USER} -c "sed \"/UPGRADE_STATUS/s/.*/UPGRADE_STATUS=1/g\" ${NEW_INSTALL_PATH}/ProtectClient-E/conf/testcfg.tmp > ${NEW_INSTALL_PATH}/ProtectClient-E/conf/testcfg.tmp.bak"
                su - ${AGENT_USER} -c "mv -f "${NEW_INSTALL_PATH}/ProtectClient-E/conf/testcfg.tmp.bak" "${NEW_INSTALL_PATH}/ProtectClient-E/conf/testcfg.tmp""
            else
                su - ${AGENT_USER} -s ${SHELL_TYPE_SH} -c "sed \"/UPGRADE_STATUS/s/.*/UPGRADE_STATUS=1/g\" ${NEW_INSTALL_PATH}/ProtectClient-E/conf/testcfg.tmp > ${NEW_INSTALL_PATH}/ProtectClient-E/conf/testcfg.tmp.bak"
                su - ${AGENT_USER} -s ${SHELL_TYPE_SH} -c "mv -f "${NEW_INSTALL_PATH}/ProtectClient-E/conf/testcfg.tmp.bak" "${NEW_INSTALL_PATH}/ProtectClient-E/conf/testcfg.tmp""
            fi
            CHMOD 600 "${NEW_INSTALL_PATH}/ProtectClient-E/conf/testcfg.tmp"
        else
            # failure
            if [ "${SYS_NAME}" = "SunOS" ] || [ "${SYS_NAME}" = "AIX" ]; then
                su - ${AGENT_USER} -c "sed \"/UPGRADE_STATUS/s/.*/UPGRADE_STATUS=0/g\" ${OLD_INSTALL_PATH}/ProtectClient-E/conf/testcfg.tmp > ${OLD_INSTALL_PATH}/ProtectClient-E/conf/testcfg.tmp.bak"
                su - ${AGENT_USER} -c "mv -f "${OLD_INSTALL_PATH}/ProtectClient-E/conf/testcfg.tmp.bak" "${OLD_INSTALL_PATH}/ProtectClient-E/conf/testcfg.tmp""
            else
                su - ${AGENT_USER} -s ${SHELL_TYPE_SH} -c "sed \"/UPGRADE_STATUS/s/.*/UPGRADE_STATUS=0/g\" ${OLD_INSTALL_PATH}/ProtectClient-E/conf/testcfg.tmp > ${OLD_INSTALL_PATH}/ProtectClient-E/conf/testcfg.tmp.bak"
                su - ${AGENT_USER} -s ${SHELL_TYPE_SH} -c "mv -f "${OLD_INSTALL_PATH}/ProtectClient-E/conf/testcfg.tmp.bak" "${OLD_INSTALL_PATH}/ProtectClient-E/conf/testcfg.tmp""
            fi
            CHMOD 600 "${OLD_INSTALL_PATH}/ProtectClient-E/conf/testcfg.tmp"
        fi
        exit ${EXIT_CODE}
    fi
}

ExecUpgrade()
{
    CLIENT_NAME=$FILE_PATH
    Log "Begin upgrade the DataBackup ProtectAgent."
    cd ${CURRENT_PATH}/ProtectClient-e
    TAR_PATH=`pwd` && TAR_PATH="${TAR_PATH}/ProtectClient-E"
    mkdir -p ${TAR_PATH}

    if [ "$SYS_NAME" = "Linux" ]; then
        tar -xf ${CLIENT_NAME} -C ${TAR_PATH}
    elif [ "$SYS_NAME" = "SunOS" ]; then
        Temporary_Path=`pwd`
        cp -r "${CLIENT_NAME}" "${TAR_PATH}"
        cd "${TAR_PATH}"
        gzip -d ${CLIENT_NAME}
        packName=`ls | grep protect*.tar`
        tar -xf ${packName}
        rm -rf "${TAR_PATH}/${packName}"
        cd ${Temporary_Path}
    else
        Temporary_Path=`pwd`
        cp -r "${CLIENT_NAME}" "${TAR_PATH}"

        cd "${TAR_PATH}"
        ${INSTALL_PACKAGE_PATH}/third_party_software/XZ/xz -d ${CLIENT_NAME}
        packName=${CLIENT_NAME%%.xz}
        tar -xf ${packName}
        rm -rf "${TAR_PATH}/${packName}"
        cd ${Temporary_Path}
    fi

    if [ -f "${TAR_PATH}/sbin/agent_upgrade.sh" ]; then
        cd "${TAR_PATH}/sbin"
        CHMOD +x ${TAR_PATH}/sbin/*.sh
        sh agent_upgrade.sh ${CURRENT_PATH}
        return $?
    else
        Log "Not found the DataBackup ProtectAgent upgrade script ${TAR_PATH}/sbin/agent_upgrade.sh, failed to upgrade."
        LogError "Not found the DataBackup ProtectAgent upgrade script, failed to upgrade." ${ERR_UPGRADE_INSTALL_PACKAGE_INCOMPLETE}
        return 1
    fi
}

GetSanCientOsVersion()
{
    if [ "${TESTCFG_BACK_ROLE}" != "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ]; then
        Log "Not sanclient type do not need to check system version."
        return 0
    fi

    index=1      # the index of awk -v
    while [ 1 ]
    do
        SUPPORT_VERSION=`echo "${SANCLIENT_SUPPORT_VERSION}" | ${MYAWK} -F ',' -v i="$index" '{print $i}'`
        VERIFY_VERSION=`cat /etc/os-release | grep  "VERSION=" | ${MYAWK} -F "=" '{print $NF}'| tr -d '"'`
        if [ "${SUPPORT_VERSION}" = "${VERIFY_VERSION}" ]; then
            Log "The support vssion is ${SUPPORT_VERSION}."
            return 0
        fi

        if [ "${SUPPORT_VERSION}" != "${VERIFY_VERSION}" ] && [ "${SUPPORT_VERSION}" != "" ]; then
            index=`expr $index + 1`
            continue
        fi

        if [ "${SUPPORT_VERSION}" = "" ]; then
            echo "This system do not support upgrade sanclient."
            LogError "Failed to upgrade sanclient." ${ERROR_OS_NOT_SUPPORT}
            return 1
        fi
    done   
}

BackupProfile()
{
    if [ "${OLD_INSTALL_PATH}" = "/opt/OceanProtect/ProtectClient" ]; then
        su - ${AGENT_USER} -s ${SHELL_TYPE_SH} -c "cp -r -P /home/rdadmin/.profile  /home/.profile.bak"
    fi
}


########################################################################################
# Main Process
########################################################################################
printf "\\033[1;32m********************************************************\\033[0m \n"
printf "\\033[1;32m     Start the upgration of ${PRODUCT_NAME}     \\033[0m \n"
printf "\\033[1;32m********************************************************\\033[0m \n"

# 1.Check Whether Input Params Are Valid
CheckInputParams $*
if [ $? -eq 1 ]; then
    ExitHandle 1
fi

# 2.Obtaining OS Information
GetOsVersion
if [ $? -eq 1 ]; then
    echo "Failed to obtain the operating system information."
    LogError "Failed to obtain the operating system information." ${ERR_UPGRADE_FAIL_GET_SYSTEM_PARAMETERS}
    ExitHandle 1
fi

# 3.Compare upgrade version and check resource
CompareUpdateVersion
if [ $? -eq 1 ]; then
    echo "Failed to update, Old_Update_Version[$OLD_UPDATE_VERSION]>=New_Update_Version[$NEW_UPDATE_VERSION]"
    LogError "Failed to update, Old_Update_Version[$OLD_UPDATE_VERSION]>=New_Update_Version[$NEW_UPDATE_VERSION]"
    ExitHandle 1
else
    Log "The upgrade version is detected successfully."
fi
GetBackupRole || ExitHandle 1
CheckHostResource

# 4.Check connectivity between ProtectAgent and ProtectManager
CheckConnectivity
if [ $? -eq 1 ]; then
    Log "Can not access to the ProtectManager, the upgrade process exits."
    echo "Can not access to the ProtectManager, the upgrade process exits."
    ExitHandle 1
else
    Log "The connectivity is checked successfully."
fi

# 5.Obtaining the Current IP Address Configuration of the Agent
CheckAgentIP
if [ $? -eq 1 ]; then
    Log "Failed to check the ip address of the agent."
    echo "Failed to check the ip address of the Agent."
    ExitHandle 1
else
    Log "The IP address of the Agent is checked successfully.."
fi

# 6.Adapt installation package selection
AdaptClientPackage
if [ $? -eq 1 ]; then
    Log "Not found any suitable package,check whether the name of the installation package is correct."
    LogError "Not found any suitable package,check whether the name of the installation package is correct." ${ERR_UPGRADE_PACKAGE_NO_MATCH_HOST_SYSTEM} ${SYS_NAME}
    ExitHandle 1
else
    Log "Adapt install package successfully,continue to upgrade."
fi
# Modify Euler 2.0 service file
ModifyServiceFile
# stop agent
echo "OLD_INSTALL_PATH=$OLD_INSTALL_PATH"
sh $OLD_INSTALL_PATH/stop.sh
if [ $? -ne 0 ]; then
    echo "The DataBackup ProtectAgent service fails to be stoped."
    Log "The DataBackup ProtectAgent service fails to be stoped."
    ExitHandle 1
else
    Log "The DataBackup ProtectAgent service is successfully stoped."
    AGENT_HAVE_BEEN_STOPED_FLAG=1
fi

# Backup installed package
BackupInstalledPac
if [ $? -eq 1 ]; then
    echo "The installed DataBackup ProtectAgent application is not found."
    LogError "The installed DataBackup ProtectAgent application is not found." ${ERR_UPGRADE_PACKAGE_NO_MATCH_HOST_SYSTEM} ${SYS_NAME}
    ExitHandle 1
else
    Log "The installed DataBackup ProtectAgent are backed up successfully."
fi

# backup log
sh $CURRENT_PATH/collectlog.sh >/dev/null 2>&1
if [ $? -ne 0 ]; then
    echo "Collect old log failed."
    LogError "Collect old log failed." ${ERR_UPGRADE_FAIL_BACKUP}
    ExitHandle 1
else
    CP -r -p $OLD_INSTALL_PATH/* $UPGRADE_BACKUP_PATH
    # 适配1.2升级1.5场景
    BackupProfile
fi

# Implement upgrade
ExecUpgrade
UPGRADE_RESULT=$?
if [ ${UPGRADE_RESULT} -ne 0 ]; then
    echo "DataBackup ProtectAgent upgrade failed, UPGRADE_RESULT is ${UPGRADE_RESULT} ."
    Log "DataBackup ProtectAgent upgrade UPGRADE_RESULT is ${UPGRADE_RESULT} ."
else
    Log "DataBackup ProtectAgent upgrade successfully."
fi

ExitHandle ${UPGRADE_RESULT}
