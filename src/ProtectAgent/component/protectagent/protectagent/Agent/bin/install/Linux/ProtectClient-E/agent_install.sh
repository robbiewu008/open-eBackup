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
AGENT_USER=rdadmin
AGENT_GROUP=rdadmin
SANCLIENT_USER=sanclient
SANCLIENT_GROUP=sanclient
DEFAULT_GROUP_INTERNAL=nobody
UNIX_CMD=
SHELL_TYPE=""
AGENT_PORT=""
IP=0
ECHO_E=""
ROCKY4_FLAG=0
G_NETWORK_TYPE=""
DPA_ROLE="DPA"
NIC="null"
TmpIndex=""
SHELL_TYPE_SH="/bin/sh"
SYS_NAME=`uname -s`

# other params
PKEY_PASSWORD=""
PKEY_VERIFY=""
PM_IP=""
PM_IP_LIST=""
PM_PORT=""
PM_MANAGER_IP_LIST=""
PM_MANAGER_PORT=""
RDAGENT_PORT=""
NGINX_PORT=""
USER_ID=""
IP_TYPE=""
DEPLOY_TYPE=""
IS_ENABLE_DATATURBO=""

IS_DPC_COMPUTE_NODE=""
DPC_STORAGE_FRONT_IP=""
DPC_STORAGE_FRONT_GATEWAY=""
IS_FILECLIENT_INSTALL=""

REGIST_DIR=${DATA_BACKUP_AGENT_HOME}/register
LO_IP_IPV4="127.0.0.1"
LO_IP_IPV6="::1"
LOCALNICARRAY=""
BACKUP_COPY_DIR=${INSTALL_DIR_UPPER}/Bak
LATEST_DIR=""
if [ -d "${BACKUP_COPY_DIR}" ]; then
    LATEST_DIR=`ls -t ${BACKUP_COPY_DIR} | head -n 1`
fi
LATEST_BAK_DIR=${BACKUP_COPY_DIR}/${LATEST_DIR}
SCAN_DIR_RECORD="scan_dir_record.txt"

############# error code ##############
ERR_INPUT_PARAM_ERR=10
ERR_WORKINGUSER_ADD_FAILED=14
ERR_CHECK_INSTALLATION_PATH_FAILED=75
ERR_USERNAME_SET_FAILED=16
ERR_PASSWORD_SET_FAILED=17
ERR_IPADDR_SET_FAILED=1577210136
ERR_PORT_SET_FAILED=1577210137
ERR_SAMEIP_INSTALL_PUSH=22
ERR_AGENT_START_FAILED=1577210140
ERR_ENV_SET_FAILED=28
ERR_PROCESS_IS_EXIST=29
ERR_SET_DIR_ATTR_FAILED=34
ERR_DRIVER_HAS_INSTALLED=35
ERR_DRIVER_INSTALL_FAILED=36
ERR_NIC_SET_FAILED=1577210136
ERR_ADD_DPA_USER_ERROR=38
ERR_REGISTER_HOST_ERROR=39
ERR_VERIFY_PRIVATE_KEY_FAILED=1577209890
ERR_WORKINGUSER_EXISTS=1577209885
ERR_NO_SUPPORT_PLUGIN=2
ERR_IPADDR_SET_FAILED_RETCODE=76
ERR_DATATURBO_INSTALL_FAILED=21
ERR_INSTALL_PLUGINS=1577210130
ERR_PORT_SET_FAILED_RETCODE=77
ERR_AGENT_START_FAILED_RETCODE=80
ERR_CONFIG_POLICY_ROUTE_FAILED=1577209892
WARING_UNSSPORT_APPS=250
ERR_FILECLIENT_INSTALL_FAILED=1
############# error code ##############

ERR_INSTALL_PLUGIN_RETCODE=71

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
ERR_UPGRADE_NO_PAM_ROOTOK=1677873153
##### upgrade errcode end #####

if [ "${SYS_NAME}" = "SunOS" ]; then
    TMP_DATA_BACKUP_AGENT_HOME=`cat /etc/profile | grep "DATA_BACKUP_AGENT_HOME=" | nawk -F "=" '{print $NF}'`
    TMP_DATA_BACKUP_SANCLIENT_HOME=`cat /etc/profile | grep "DATA_BACKUP_SANCLIENT_HOME=" | nawk -F "=" '{print $NF}'`
else
    TMP_DATA_BACKUP_AGENT_HOME=`cat /etc/profile | grep "export DATA_BACKUP_AGENT_HOME=" | awk -F "=" '{print $NF}'`
    TMP_DATA_BACKUP_SANCLIENT_HOME=`cat /etc/profile | grep "export DATA_BACKUP_SANCLIENT_HOME=" | awk -F "=" '{print $NF}'`
fi

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
    export DATA_BACKUP_AGENT_HOME DATA_BACKUP_SANCLIENT_HOME
fi

###### BACKUP ROLE ######
RegTypeName=""
RegType=-1
BACKUP_ROLE=-1
BACKUP_ROLE_HOST=0
BACKUP_ROLE_VMWARE=2
BACKUP_ROLE_GENERAL_PLUGIN=4
BACKUP_ROLE_SANCLIENT_PLUGIN=5
INSTALL_PATH=${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient
SANCLIENT_INSTALL_PATH=${DATA_BACKUP_SANCLIENT_HOME}/DataBackup/SanClient
AGENT_BACKUP_PATH="${DATA_BACKUP_AGENT_HOME}/AgentUpgrade"
SANCLIENT_AGENT_BACKUP_PATH="${DATA_BACKUP_SANCLIENT_HOME}/SanClientUpgrade"
CPU_LIMIT_PLUGIN="FilePlugin GeneralDBPlugin ElasticSearchPlugin"
MEM_LIMIT_PLUGIN="FilePlugin ElasticSearchPlugin"

XBSA_INSTALL_DIR="${INSTALL_PATH}/interfaces/xbsa"
XBSA_INSTALL_LIB_DIR="${INSTALL_PATH}/interfaces/xbsa/lib/"
XBSA_INSTALL_LOG_DIR="${INSTALL_PATH}/interfaces/xbsa/log/"
XBSA_INSTALL_CERT_DIR="${INSTALL_PATH}/interfaces/xbsa/conf/thrift/client"
XBSA_INSTALL_CONF_DIR="${INSTALL_PATH}/interfaces/xbsa/conf/"
XBSA_INSTALL_KMC_DIR="${INSTALL_PATH}/interfaces/xbsa/conf/kmc"
XBSA_INSTALL_NGINX_CONF_DIR="${INSTALL_PATH}/interfaces/xbsa/nginx/conf"
###### BACKUP SCENE ######
BACKUP_SCENE=-1
BACKUP_SCENE_EXTERNAL=0
BACKUP_SCENE_INTERNAL=1


###### SET UMASK ######
umask 0022

###### CONFIG PARAM ######
AGENT_IP=
CURRENT_PATH=`dirname $0` && cd $CURRENT_PATH && CURRENT_PATH=`pwd`
TESTCFG_BACK_ROLE=`su - ${SANCLIENT_USER} 2>/dev/null -s ${SHELL_TYPE_SH} -c "sed '/^BACKUP_ROLE=/!d;s/.*=//' ${CURRENT_PATH}/../conf/testcfg.tmp "`
if [ "${TESTCFG_BACK_ROLE}" = "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ]; then
    AGENT_GROUP=${SANCLIENT_GROUP}
    AGENT_USER=${SANCLIENT_USER}
    INSTALL_PATH=${SANCLIENT_INSTALL_PATH}
    AGENT_BACKUP_PATH=${SANCLIENT_AGENT_BACKUP_PATH}
fi
PLUGIN_DIR=${INSTALL_PATH}/Plugins
NIGINX_BACKUP_CONFIG="${AGENT_BACKUP_PATH}/ProtectClient-E/nginx/conf/nginx.conf"
# confirm default user home dir
if [ "${SYS_NAME}" = "SunOS" ]; then
    AWK=nawk
    DEFAULT_HOME_DIR="/export/home/${AGENT_USER}"
else
    AWK=awk
    DEFAULT_HOME_DIR="/home/${AGENT_USER}"
fi

if [ "`ls -al /bin/sh | ${AWK} '{print $NF}'`" = "bash" ]; then
    EXPORT_ENV="if [ -f ~/.profile ]; then . ~/.profile; fi;"
else
    EXPORT_ENV=""
fi

# Function Configuration
LOG_FILE_NAME=${AGENT_ROOT_PATH}/slog/agent_install.log
. "${AGENT_ROOT_PATH}/sbin/agent_sbin_func.sh"

# Function Definition
PreCheck()
{
    if [ "${SYS_NAME}" = "Linux" ]; then
        SysRockyFlag=`cat /etc/issue | grep 'Linx'`
        rocky4=`cat /etc/issue | grep 'Rocky'`
        if [ -n "${rocky4}" ]; then
            ROCKY4_FLAG=1
        fi
    fi

    if [ "${SysRockyFlag}" = "" ]; then
        ECHO_E=-e
    fi

    if [ "${SYS_NAME}" = "AIX" ] || [ "${SYS_NAME}" = "HP-UX" ] || [ "${SYS_NAME}" = "SunOS" ] || [ 1 -eq ${ROCKY4_FLAG} ]; then
        ECHO_E=""
        if [ "${SHELL_TYPE}" = "bash" ]; then
            UNIX_CMD=-l
        fi
    fi
}

LogError()
{
    #$1=errorInfo,$2=errorCode, $3=errorDetailParam
    LogErr "$1" "$2" "job_log_agent_storage_update_install_fail_label" "$3"
}

# create install info file, may be used on the next step
CreateInstallInfoFile()
{
    INSTALL_INFO_FILE=${INSTALL_PATH}/ProtectClient-E/slog/install.info
    if [ -f "${INSTALL_INFO_FILE}" ]; then
        rm -f "${INSTALL_INFO_FILE}"
    fi
    touch ${INSTALL_INFO_FILE}
    CHMOD 600 ${INSTALL_INFO_FILE}
}

# check the network connectivity, to find accessable pm ip
AdaptListenIp()
{
    Log "Adapting listening ip address."
    if [ "${BACKUP_SCENE}" = "${BACKUP_SCENE_INTERNAL}" ]; then
        Log "Internal install."
        return
    fi
    index=1      # the index of awk -v
    if [ "${PM_IP_LIST}" = "" ] || [ "${PM_PORT}" = "" ]; then
        echo "PING_RESULT=1" >> ${INSTALL_INFO_FILE}
        echo "The PM address or port in the configuration file is empty."
        LogError "The PM address or port in the configuration file is empty." ${ERR_UPGRADE_FAIL_VERIFICATE_PARAMETER}
        exit 1
    fi
    while [ 1 ]
    do
        tmpPmIpAddr=`echo "${PM_IP_LIST}" | $AWK -F ',' -v i="$index" '{print $i}'`
        if [ "${tmpPmIpAddr}" = "" ]; then
            break
        fi
        CheckIpsConnectivity "${AGENT_IP}" "$tmpPmIpAddr" "${PM_PORT}"
        if [ $? -eq 0 ]; then
            PM_IP="${tmpPmIpAddr}"
            Log "PM ip address1:${PM_IP}."
            break
        fi
        index=`expr $index + 1`
        Log "Can not access PM ip address:${tmpPmIpAddr}."
    done
 
    if [ -n "$PM_IP" ]; then
        echo "PING_RESULT=0" >> ${INSTALL_INFO_FILE}
        echo "PM ip($PM_IP) addresse can be accessed."
    else
        echo "PING_RESULT=1" >> ${INSTALL_INFO_FILE}
        echo "All PM IP addresses cannot be accessed."
        LogError "All IP addresses cannot be accessed." ${ERR_UPGRADE_NETWORK_ERROR}
        exit 1
    fi
}

CheckIpsType()
{
    srcIp=$1
    dstIp=$2
    echo ${srcIp} | grep "\\." >/dev/null 2>&1
    if [ $? -eq 0 ]; then
        SRC_IP_TYPE="ipv4"
    else
        SRC_IP_TYPE="ipv6"
    fi
    echo ${dstIp} | grep "\\." >/dev/null 2>&1
    if [ $? -eq 0 ]; then
        DST_IP_TYPE="ipv4"
    else
        DST_IP_TYPE="ipv6"
    fi
    if [ ${SRC_IP_TYPE} = ${DST_IP_TYPE} ]; then
        return 0
    fi
    return 1
}

#获取临时配置文件信息
GetConfigInfo()
{
    USER_ID=`SUExecCmdWithOutput "sed '/^USER_ID=/!d;s/.*=//' ${INSTALL_PATH}/ProtectClient-E/conf/testcfg.tmp"`
    PKG_CONF_PATH=`SUExecCmdWithOutput "sed '/^PKG_CONF_PATH=/!d;s/.*=//' ${INSTALL_PATH}/ProtectClient-E/conf/testcfg.tmp"`
    DOMAIN_NAME=`SUExecCmdWithOutput "sed '/^DOMAIN_NAME=/!d;s/.*=//' ${INSTALL_PATH}/ProtectClient-E/conf/testcfg.tmp"`
    INSTALLATION_MODE=`SUExecCmdWithOutput "sed '/^INSTALLATION_MODE=/!d;s/.*=//' ${INSTALL_PATH}/ProtectClient-E/conf/testcfg.tmp"`
    PKG_VERSION=`SUExecCmdWithOutput "sed '/^PKG_VERSION=/!d;s/.*=//' ${INSTALL_PATH}/ProtectClient-E/conf/testcfg.tmp"`
    VERSION_TIME_STAMP=`SUExecCmdWithOutput "sed '/^VERSION_TIME_STAMP=/!d;s/.*=//' ${INSTALL_PATH}/ProtectClient-E/conf/testcfg.tmp"`
    BACKUP_ROLE=`SUExecCmdWithOutput "sed '/^BACKUP_ROLE=/!d;s/.*=//' ${INSTALL_PATH}/ProtectClient-E/conf/testcfg.tmp"`
    BACKUP_SCENE=`SUExecCmdWithOutput "sed '/^BACKUP_SCENE=/!d;s/.*=//' ${INSTALL_PATH}/ProtectClient-E/conf/testcfg.tmp"`
    AGENT_IP=`SUExecCmdWithOutput "sed '/^AGENT_IP=/!d;s/.*=//' ${INSTALL_PATH}/ProtectClient-E/conf/testcfg.tmp"`
    Log "Agent ip is $AGENT_IP."

    PM_IP_LIST=`SUExecCmdWithOutput "sed '/^PM_IP=/!d;s/.*=//' ${INSTALL_PATH}/ProtectClient-E/conf/testcfg.tmp"`
    PM_PORT=`SUExecCmdWithOutput "sed '/^PM_PORT=/!d;s/.*=//' ${INSTALL_PATH}/ProtectClient-E/conf/testcfg.tmp"`
    PM_MANAGER_IP_LIST=`SUExecCmdWithOutput "sed '/^PM_MANAGER_IP=/!d;s/.*=//' ${INSTALL_PATH}/ProtectClient-E/conf/testcfg.tmp"`
    PM_MANAGER_PORT=`SUExecCmdWithOutput "sed '/^PM_MANAGER_PORT=/!d;s/.*=//' ${INSTALL_PATH}/ProtectClient-E/conf/testcfg.tmp"`

    IS_DPC_COMPUTE_NODE=`SUExecCmdWithOutput "sed '/^IS_DPC_COMPUTE_NODE=/!d;s/.*=//' ${INSTALL_PATH}/ProtectClient-E/conf/testcfg.tmp"`
    DPC_STORAGE_FRONT_IP=`SUExecCmdWithOutput "sed '/^DPC_STORAGE_FRONT_IP=/!d;s/.*=//' ${INSTALL_PATH}/ProtectClient-E/conf/testcfg.tmp"`
    DPC_STORAGE_FRONT_GATEWAY=`SUExecCmdWithOutput "sed '/^DPC_STORAGE_FRONT_GATEWAY=/!d;s/.*=//' ${INSTALL_PATH}/ProtectClient-E/conf/testcfg.tmp"`
    IS_FILECLIENT_INSTALL=`SUExecCmdWithOutput "sed '/^IS_FILECLIENT_INSTALL=/!d;s/.*=//' ${INSTALL_PATH}/ProtectClient-E/conf/testcfg.tmp"`

    if [ "${BACKUP_ROLE}" = "" ]; then
        echo "Get BACKUP_ROLE from config failed."
        Log "Get BACKUP_ROLE from config failed."
        LogError "Get BACKUP_ROLE from config failed." ${ERR_UPGRADE_FAIL_READ_CONFIGURE}
        exit 1
    fi
    #内置参数写入
    SUExecCmd "${AGENT_ROOT_PATH}/bin/xmlcfg write Backup backup_scene ${BACKUP_SCENE}"

    # check if can access pm 
    CheckIpsConnectivity "${AGENT_IP}" "`echo ${PM_IP_LIST} | sed 's/,/ /g'`" "${PM_PORT}"
    if [ $? -ne 0 ] && [ ${BACKUP_SCENE} -ne ${BACKUP_SCENE_INTERNAL} ]; then
        CheckIsEip ${AGENT_IP}
        RET_CODE=$?
        if [ ${BACKUP_ROLE} -ne ${BACKUP_ROLE_GENERAL_PLUGIN} ] && [ ${RET_CODE} -ne 0 ]; then
            echo "Business ip is not valid, use manage ip."
            Log "Business ip is not valid, use manage ip."
            business_is_usable=0
            for i in 1 2 3; do
                sleep 5
                CheckIpsConnectivity "${AGENT_IP}" "`echo ${PM_IP_LIST} | sed 's/,/ /g'`" "${PM_PORT}"
                if [ $? -eq 0 ]; then
                    business_is_usable=1
                    echo "The business IP has been restored, use the business ip."
                    Log "The business IP has been restored, use the business ip."
                    break
                fi
                echo "Retrying to connect to the business ip failed time(${i}/3)."
                Log "Retrying to connect to the business ip failed time(${i}/3)."
            done
            if [ ${business_is_usable} -eq 0 ]; then
                echo "No general_plugin scenarios must use business ip."
                LogError "No general_plugin scenarios must use business ip." ${ERR_UPGRADE_NETWORK_ERROR}
                exit 1
            fi
        elif [ ${RET_CODE} -ne 0 ]; then
            PM_IP_LIST=$PM_MANAGER_IP_LIST
            PM_PORT=$PM_MANAGER_PORT
        fi
    fi
}

# Create a unique ID
AgentUniqueID()
{
    if [ ! -d "/etc/HostSN" ]; then
        mkdir "/etc/HostSN"
    fi
    chown -h -R ${AGENT_USER}:${AGENT_USER} /etc/HostSN
}

# Check if Agent exists
CheckAgentExist()
{
    tempRdadminUid=`${AGENT_ROOT_PATH}/sbin/xmlcfg read Backup rdadmin_gid`
    tempSanclientUid=`${AGENT_ROOT_PATH}/sbin/xmlcfg read Backup sanclient_gid`
    if [ ${SYS_NAME} = "AIX" ]; then
        ps -u ${AGENT_USER} | grep -v grep | grep $1 1>/dev/null 2>&1
    elif [ "${BACKUP_ROLE_SANCLIENT_PLUGIN}" = "${BACKUP_ROLE}" ]; then
        ps -lu ${AGENT_USER} | grep -v grep | grep $1 | grep ${tempSanclientUid} 1>/dev/null 2>&1
    else
        ps -lu ${AGENT_USER} | grep -v grep | grep $1 | grep ${tempRdadminUid} 1>/dev/null 2>&1
    fi
    if [ $? -ne 0 ]; then
        Log "Process $1 is not exist, then install DataBackup ProtectAgent."
        return 0
    fi

    echo "Process of $1 already exists. The ${AGENT_USER} of installation process exits."
    LogError "Process $1 already exists. The ${AGENT_USER} of installation process exits." ${ERR_PROCESS_IS_EXIST}
    exit $ERR_PROCESS_IS_EXIST
}

SetUserNameAndPasswd()
{
    NAMENUM=0
    COMPLEX=0
    USERNAME=
    PASSWORD=
    PASSWORDTEMP=
    USERREX="^[a-zA-Z]\\w\\{3,15\\}$"

    if [ "AIX" = "$SYS_NAME" ]; then
        USERREX="^[a-zA-Z][a-zA-Z0-9_]\{3,15\}$"
    elif [ "HP-UX" = "$SYS_NAME" ] || [ "SunOS" = "$SYS_NAME" ]; then
        USERREX="^[a-zA-Z]"
    fi 

    while [ ${NAMENUM} -lt 3 ]
    do
        USERNAME=${USER}        
        USERCHECK=`echo "${USERNAME}" | grep -n ${USERREX}`
        if [ "${USERCHECK}" = "" ]; then
            echo "The name should contain 4 to 16 characters,  including case-sensitive letters, digits or underscores (_),and must start with a letter."
            NAMENUM=3
            break
        fi
        
        if [ "HP-UX" = "$SYS_NAME" ] || [ "SunOS" = "$SYS_NAME" ]; then
            LENGTH=`echo $USERNAME | $AWK '{print length()}'`
            if [ ${LENGTH} -gt 16 ] || [ ${LENGTH} -lt 4 ]; then
                echo "The name sholud contain 4 to 16 characters." 
                NAMENUM=3
                break
            fi

            TMP=1
            SWITCH=0
            while [ $TMP -le $LENGTH ]
            do
                CHECK=`echo $USERNAME | $AWK -v i="$USERNAME" -v j="$TMP" '{print substr(i,j,1)}' | grep -n [a-zA-Z0-9_]`
                if [ "$CHECK" = "" ]; then
                    SWITCH=1
                fi
                TMP=`expr $TMP + 1`
            done

            if [ $SWITCH -eq 1 ]; then
                echo "The name should contains only letters(a-zA-Z) digits(0-9) and underscores(_)." 
                NAMENUM=3
                break
            fi
        fi
        break
    done

    if [ ${NAMENUM} = 3 ]; then
        echo ${ECHO_E} "\nThe installation of DataBackup ProtectAgent will be stopped."
        LogError "The name that you entered($USERNAME) is error, so installation was stopped." ${ERR_UPGRADE_FAIL_VERIFICATE_PARAMETER}
        exit $ERR_USERNAME_SET_FAILED
    fi

    SUExecCmd "${AGENT_ROOT_PATH}/bin/xmlcfg write System name ${USERNAME}"
}

AjustIpType()
{
    PM_IP_ADDRESS=$1
    echo ${PM_IP_ADDRESS} | grep "\\${SEMICOLON}" >/dev/null 2>&1
    if [ 0 -eq $? ]; then
        IP_TYPE="ipv6"
    fi
    
    echo ${PM_IP_ADDRESS} | grep "\\${DOT}" >/dev/null 2>&1
    if [ 0 -eq $? ]; then
        IP_TYPE="ipv4"
    fi

    Log "Pm ip address [${PM_IP_ADDRESS}] ip type [${IP_TYPE}]"
}

#******************************************************************#
# Function: get_network_type
# Description: get user input for network type
# Input Parameters:
# Returns:
#******************************************************************#
GetNetworkType()
{
    while true
    do
        L_RET=""
        AjustIpType ${PM_IP}
        if [ "${IP_TYPE}" = "ipv4" ]; then
            L_RET=1
        elif [ "${IP_TYPE}" = "ipv6" ]; then
            L_RET=2
        fi

        G_NETWORK_TYPE=${L_RET}
        if [ "${L_RET}" = "" ]; then
            Log "Default network type is ipv4."
            G_NETWORK_TYPE="ipv4"
        elif [ "${L_RET}" = "1" ]; then
            G_NETWORK_TYPE="ipv4"
        elif [ "${L_RET}" = "2" ]; then
            G_NETWORK_TYPE="ipv6"
        else
            printf "\e[0;33;1mWarning: illegal type provided, please input again. \e[0m \n"
            continue
        fi     
        Log "Network type is ${G_NETWORK_TYPE}." 
        break
    done
}

ChooseEIP()
{
    if [ "$UPGRADE_FLAG_TEMPORARY" = "1" ]; then
        return 0
    fi

    if [ "${INSTALLATION_MODE}" = "push" ]; then
        return 0
    fi

    choice_number=0
    while [ $choice_number -lt 3 ]
    do
        printf "\\033[1;32mCheck whether the agent ip will be redirected?(Such as EIP or the ip transformed by NAT):(y|n), default(n)\\033[0m \n"
        printf "Your choice:"
        read eip_choice
        if [ "$eip_choice" = "" ] || [ "$eip_choice" = "n" ]; then
            return 0
        elif [ "$eip_choice" = "y" ]; then
                printf "Please enter the redirect ip: "
                read eip
                chkResult=`CheckIpLegality ${eip}`
                if [ $chkResult -ne 0 ]; then
                    AGENT_IP=${eip}
                    return 0
                fi
            break
        else
            echo "Please enter y or n."
        fi
        choice_number=`expr $choice_number + 1`
    done
    if [ $choice_number -ge 3 ]
    then
        echo "The number of incorrect inputs exceeds 3. The installation process exits abnormally."
        LogError "The number of incorrect inputs exceeds 3. The installation process exits abnormally." ${ERR_IPADDR_SET_FAILED_RETCODE}
        exit ${ERR_IPADDR_SET_FAILED_RETCODE}
    fi

}

CheckIsEip()
{
    if [ "${SYS_NAME}" = "Linux" ] && [ "$1" != "" ]; then
        enter_eip=`ip addr | grep $1 | $AWK '{print $2}' | $AWK -F "/" '{print $1}'`
        if [ "${enter_eip}" != "$1" ]; then
            Log "Current IP is EIP."
            return 0
        fi
    fi
    Log "Current IP is not EIP."
    return 1
}

GetLocalConnectableNic()
{
    Log "Start to obtain network adapter information."
    if [ "AIX" = "${SYS_NAME}" ] || [ "SunOS" = "${SYS_NAME}" ]; then
        localNic=`ifconfig -a | grep "flags" | $AWK '{print $1}' | $AWK -F ":" '{print$1}'`
    elif [ "HP-UX" = "${SYS_NAME}" ]; then
        # Keep the IP detection mode used.
        startline=`netstat -ni | sed -n -e '/IPv4:/='`
        endline=`netstat -ni | sed -n -e '/IPv6:/='`
        if [ "${startline}" != "" ] && [ "${endline}" != "" ]; then     # ipv4 and ipv6 both
            LOCALIPARRAY_BUFF=`netstat -ni| sed -n ${startline}','${endline}'p' |grep -v 'Address'|$AWK -F " " '{print $4}'`
            localNic="${localNic} ${LOCALIPARRAY_BUFF}"
        elif [ "${startline}" = "" ] && [ "${endline}" = "" ]; then  # ipv4 only
            localNic=`netstat -ni|grep -v 'Address'|$AWK '{print $4}'`
        else
            # no scenario
            return 0
        fi
    else
        localNic=`ip addr | grep "mtu" | $AWK '{print $2}' | $AWK -F ":" '{print $1}'`
    fi

    if [ "${localNic}" = "" ]; then
        Log "Failed to obtain the network adapter information. error exit."
        return 1
    fi

    if [ "ipv4" = "${G_NETWORK_TYPE}" ]; then
        pingCMD="ping"
    elif [ "ipv6" = "${G_NETWORK_TYPE}" ]; then
        pingCMD="ping6"
    else
        echo "IP type judgment error, G_NETWORK_TYPE=${G_NETWORK_TYPE}. error exit."
        LogError "IP type judgment error, G_NETWORK_TYPE=${G_NETWORK_TYPE}. error exit." ${ERR_IPADDR_SET_FAILED}
        exit $ERR_IPADDR_SET_FAILED_RETCODE
    fi

    mkdir ${AGENT_ROOT_PATH}/stmp/NicTmp
    for nic in ${localNic}
    do
    {
        # handle this type: ens1f0.30:1@ens1f0: (@ ip addr)
        nic=`echo ${nic} | $AWK -F "@" '{print $1}'`
        if [ "${nic}" = "lo" ]; then
            continue
        fi
        for retryTime in 0 1 2
        do
            CheckIpsConnectivity "$nic" "$PM_IP" "$PM_PORT"
            if [ $? -eq 0 ]; then
                echo ${nic} > ${AGENT_ROOT_PATH}/stmp/NicTmp/${nic}
                break
            fi
            Log "${pingCMD} -I ${nic} ${ip} failed."
        done
    }&
    done
    wait

    LOCALNICARRAY=`ls -l ${AGENT_ROOT_PATH}/stmp/NicTmp/* | ${AWK} -F "/"  '{print $NF}' | ${AWK} '{print $NF}'`
    rm -rf "${AGENT_ROOT_PATH}/stmp/NicTmp"
    Log "LOCALNICARRAY=${LOCALNICARRAY}"
}

ObtainIpUponNic()
{
    if [ "AIX" = "${SYS_NAME}" ] || [ "SunOS" = "${SYS_NAME}" ]; then
        if [ "ipv4" = "${G_NETWORK_TYPE}" ]; then
            echo "`ifconfig $1 | grep -w "inet" | $AWK '{print $2}'`"
        elif [ "ipv6" = "${G_NETWORK_TYPE}" ]; then
            echo "`ifconfig $1 | grep -w "inet6" | $AWK '{print $2}'`"
        else
            Log "Failed to obtain the IP address type."
        fi
    elif [ "HP-UX" = "${SYS_NAME}" ]; then
        # hpux no change for now
        echo $1
    else
        if [ "ipv4" = "${G_NETWORK_TYPE}" ]; then
            echo "`ip addr show $1 | grep -w "inet" | $AWK -F " " '{print $2}' | $AWK -F "/" '{print $1}'`"
        elif [ "ipv6" = "${G_NETWORK_TYPE}" ]; then
            echo "`ip addr show $1 | grep -w "inet6" | $AWK -F " " '{print $2}' | $AWK -F "/" '{print $1}'`"
        else
            Log "Failed to obtain the IP address type."
        fi
    fi
}

#$1-- user chooice, $2 LOCALIPARRAY count;
#return :0 -- error, 1 -- right
IsIpIndexRight()
{
    input=$1
    limit=$2
    check=`echo $input | grep "^[0-9]\{1,\}$"`
    if [[ "$check" = "" ]] || [[ $input -gt $limit ]]; then
        return 0
    fi
    return 1
}

SetIp()
{
    Log "Start set ip."
    # no need to abtain network card's nic-name on solaris hp-ux aix
    if [ "$1" = "${DPA_ROLE}" ] && [ "${SYS_NAME}" != "Linux" ]; then
        return 0
    elif [ ${BACKUP_SCENE} -eq ${BACKUP_SCENE_INTERNAL} ]; then
        Log "Internal install."
        return 0
    elif [ ! -z "${AGENT_IP}" ]; then
        IP=${AGENT_IP}
        Log "Use the configured ip address:${AGENT_IP}."
        return 0
    fi

    GetLocalNicName

    # HP-UX use ip, others use nic
    if [ "HP-UX" = "${SYS_NAME}" ]; then
        [ "$1" != "${DPA_ROLE}" ] && IP=`echo $LOCALNICARRAY | $AWK -v Index="$ipIndex" '{print $Index}'`
        [ "$1" = "${DPA_ROLE}" ] && NIC=`echo $LOCALNICARRAY | $AWK -v Index="$ipIndex" '{print $Index}'`
    else
        CheckIp $1 ${ipIndex}
        if [ $? -ne 0 ]; then 
            echo "Check ip choice failed."
            Log "Check ip choice failed."
            LogError "Check ip choice failed."$ERR_IPADDR_SET_FAILED
            exit $ERR_IPADDR_SET_FAILED_RETCODE
        fi
    fi
  
    if [ "$1" = "${DPA_ROLE}" ]; then
        Log "The DPA listening NIC ($NIC) is set successfully."
    else
        Log "The Nginx listening IP address ($IP) is set successfully."
        printf "The Nginx listening IP address (\\033[1;32m$IP\\033[0m) is set successfully. \n"
    fi
}

GetLocalNicName()
{
    if [ "${LOCALNICARRAY}" = "" ]; then
        echo "Obtaining ProtectAgent network adapter information. Please wait..."
        Log "Obtaining ProtectAgent network adapter information. Please wait..."
        GetLocalConnectableNic
        if [ "${LOCALNICARRAY}" = "" ]; then
            echo "The local IP address that can connect to the ProtectManager is not found. error exit."
            LogError "The local IP address that can connect to the ProtectManager is not found. error exit." ${ERR_IPADDR_SET_FAILED}
            exit $ERR_IPADDR_SET_FAILED_RETCODE
        fi
    fi

    LOOP_NUMBER=0
    localIpArraySize=`echo $LOCALNICARRAY | $AWK '{print NF}'`
    while [ $LOOP_NUMBER -lt 3 ]
    do
        index=1
        if [ "$1" != "${DPA_ROLE}" ]; then
            # connectable ip count > 1, user choose.
            if [ ${localIpArraySize} -gt 1 ]; then
                echo "Please select the network adapter for communication with the ProtectAgent."
                Log "Please select the network adapter for communication with the ProtectAgent."
                for nic in ${LOCALNICARRAY}; do
                    if [ "${INSTALLATION_MODE}" = "push" ]; then
                        # Select the first IP address that can be connected.
                        ipIndex=${index}
                        TmpIndex=${ipIndex}
                        break
                    fi
                    echo "   $index  $nic"
                    Log "   $index  $nic"
                    index=`expr $index + 1`
                done;
                echo "Please select the serial number is greater than 0 numbers."
                Log "Please select the serial number is greater than 0 numbers."
                PrintPrompt
                if [ "${INSTALLATION_MODE}" != "push" ]; then
                    read ipIndex
                fi
                TmpIndex=${ipIndex}
            elif [ ${localIpArraySize} -eq 1 ]; then
                ipIndex=${index}
                TmpIndex=${ipIndex}
            else
                echo "The number of IP addresses connected to the network is incorrect, error exit."
                Log "The number of IP addresses connected to the network is incorrect, error exit."
                LogError "The number of IP addresses connected to the network is incorrect, error exit."$ERR_IPADDR_SET_FAILED
                exit $ERR_IPADDR_SET_FAILED_RETCODE
            fi
        else
            ipIndex=${TmpIndex}
        fi

        IsIpIndexRight $ipIndex $localIpArraySize
        IpIndexResult=$?
        if [ "$ipIndex" = "" ] || [ "0" = "$ipIndex" ] || [ ${IpIndexResult} -eq 0 ]; then 
            echo "Not detect any local ip address that can access to the pm address."
            Log "Not detect any local ip address that can access to the pm address."
            LOOP_NUMBER=`expr $LOOP_NUMBER + 1`
            continue
        fi
        break
    done

    if [ $LOOP_NUMBER = 3 ]; then
        echo ${ECHO_E} "The installation of DataBackup ProtectAgent will be stopped."
        LogError "The ip you entered($IP) is error, so installation was stopped." ${ERR_IPADDR_SET_FAILED}
        exit $ERR_IPADDR_SET_FAILED_RETCODE
    fi
    
    Log "Get local nic name success."
    return 0
}

CheckIp()
{
    if [ "$2" = "" ]; then
       ipIndex=$1
    else
       ipIndex=$2 
    fi

    if [ "$1" != "${DPA_ROLE}" ]; then
        nicName=`echo $LOCALNICARRAY | $AWK -v Index="$ipIndex" '{print $Index}'`
        nicLocalIp=`ObtainIpUponNic ${nicName}`
        nicIpSize=`echo $nicLocalIp | $AWK '{print NF}'`
        nicIpIndex=0
        LocalConnectIPList=""
        if [ ${nicIpSize} -gt 1 ]; then
            for ip in ${nicLocalIp}; do
                CheckIpsConnectivity  "${ip}" "${PM_IP}" "${PM_PORT}"
                if [ $? -eq 0 ]; then
                    echo "The list of available IP addresses is as follows:"
                    Log "The list of available IP addresses is as follows:"
                    nicIpIndex=`expr $nicIpIndex + 1`
                    echo "   $nicIpIndex  $ip"
                    Log "   $nicIpIndex  $ip"
                    LocalConnectIPList="$LocalConnectIPList $ip"
                fi
            done
        
            if [ $nicIpIndex -gt 1 ]; then
                echo "Please select the serial number is greater than 0 numbers."
                Log "Please select the serial number is greater than 0 numbers."
                if [ "${INSTALLATION_MODE}" = "push" ] || [ "${UPGRADE_FLAG_TEMPORARY}" = "1" ]; then
                    nicLocalIpIndex=1
                else
                    CheckIpChoice
                    if [ $? -ne 0 ]; then
                       echo "The IP address you selected is invalid."
                       Log "The IP address you selected is invalid."
                       return 1
                    fi
                fi
            elif [ $nicIpIndex -eq 1 ]; then
                nicLocalIpIndex=1
            else
                echo "No available local IP address found."
                LogError "No available local IP address found." ${ERR_UPGRADE_NETWORK_ERROR}
                return 1
            fi
            IP=`echo $LocalConnectIPList | $AWK -v Index="$nicLocalIpIndex" '{print $Index}'`
        elif [ ${nicIpSize} -eq 1 ]; then
            IP=$nicLocalIp
        else
            echo "No available local IP address found."
            LogError "No available local IP address found." ${ERR_UPGRADE_NETWORK_ERROR}
            return 1
        fi
    else
        NIC=`echo $LOCALNICARRAY | $AWK -v Index="$ipIndex" '{print $Index}'`
    fi
    return 0
}

CheckIpChoice()
{
    for i in 0 1 2; do
        PrintPrompt
        read nicLocalIpIndex
        nicLocalIpIndexRex="^[1-9][0-9]\{0,1\}$" 
        nicLocalIpIndexCheck=`echo "${nicLocalIpIndex}" | grep -n "${nicLocalIpIndexRex}"`
        IpValidity=`echo $LocalConnectIPList | $AWK -v Index="$nicLocalIpIndex" '{print $Index}'`
        if [ "${nicLocalIpIndex}" = "" ] || [ "${nicLocalIpIndexCheck}" = "" ] || [ "${IpValidity}" = "" ]; then
            echo "Please enter a valid number to select."
            Log "Please enter a valid number to select." 
        else
            echo "The IP address you selected is valid."
            Log "The IP address you selected is valid."
            return 0
        fi 
    done
    
    echo "You have tried 3 times, failed to choice the nginx ip."
    Log "You have tried 3 times, failed to choice the nginx ip."
    return 1
}

SetPMIp()
{
    Log "Set the ip address of the PM."
    
    if [ ${BACKUP_SCENE} -eq ${BACKUP_SCENE_INTERNAL} ]; then
        Log "Internal install."
        return 0
    fi

    IPREX4="^[0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}$"
    IPREX6="^\s*(((([0-9A-Fa-f]{1,4}:){7}(([0-9A-Fa-f]{1,4})|:))|(([0-9A-Fa-f]{1,4}:){6}(:|((25[0-5]|2[0-4]\d|[01]?\d{1,2})(\.(25[0-5]|2[0-4]\d|[01]?\d{1,2})){3})|(:[0-9A-Fa-f]{1,4})))|(([0-9A-Fa-f]{1,4}:){5}((:((25[0-5]|2[0-4]\d|[01]?\d{1,2})(\.(25[0-5]|2[0-4]\d|[01]?\d{1,2})){3})?)|((:[0-9A-Fa-f]{1,4}){1,2})))|(([0-9A-Fa-f]{1,4}:){4}(:[0-9A-Fa-f]{1,4}){0,1}((:((25[0-5]|2[0-4]\d|[01]?\d{1,2})(\.(25[0-5]|2[0-4]\d|[01]?\d{1,2})){3})?)|((:[0-9A-Fa-f]{1,4}){1,2})))|(([0-9A-Fa-f]{1,4}:){3}(:[0-9A-Fa-f]{1,4}){0,2}((:((25[0-5]|2[0-4]\d|[01]?\d{1,2})(\.(25[0-5]|2[0-4]\d|[01]?\d{1,2})){3})?)|((:[0-9A-Fa-f]{1,4}){1,2})))|(([0-9A-Fa-f]{1,4}:){2}(:[0-9A-Fa-f]{1,4}){0,3}((:((25[0-5]|2[0-4]\d|[01]?\d{1,2})(\.(25[0-5]|2[0-4]\d|[01]?\d{1,2})){3})?)|((:[0-9A-Fa-f]{1,4}){1,2})))|(([0-9A-Fa-f]{1,4}:)(:[0-9A-Fa-f]{1,4}){0,4}((:((25[0-5]|2[0-4]\d|[01]?\d{1,2})(\.(25[0-5]|2[0-4]\d|[01]?\d{1,2})){3})?)|((:[0-9A-Fa-f]{1,4}){1,2})))|(:(:[0-9A-Fa-f]{1,4}){0,5}((:((25[0-5]|2[0-4]\d|[01]?\d{1,2})(\.(25[0-5]|2[0-4]\d|[01]?\d{1,2})){3})?)|((:[0-9A-Fa-f]{1,4}){1,2})))|(((25[0-5]|2[0-4]\d|[01]?\d{1,2})(\.(25[0-5]|2[0-4]\d|[01]?\d{1,2})){3})))\;?\s*)*$"

    HDRS_NUMBER=0
    FIRST_IP_SEG=0
    while [ $HDRS_NUMBER -lt 3 ]
    do
        FLAG=0
        ipInput=${PM_IP}
        if [ "$IP_TYPE" = "ipv4" ]; then
            IPCHECK=`echo "${ipInput}" | grep -n "${IPREX4}"`
            if [ "$IPCHECK" = "" ]; then
                echo "The format of the entered IP address is incorrect."
                Log "IP address format is error."
                HDRS_NUMBER=3
                break
            fi
        fi

        if [ "$IP_TYPE" = "ipv6" ]; then
            IPCHECK=`echo "${ipInput}" | grep -E "${IPREX6}"`
            if [ "$IPCHECK" = "" ]; then
                echo "The format of the entered IP address is incorrect."
                Log "IP address format is error."
                HDRS_NUMBER=3
                break
            fi
        fi

        if [ "$IP_TYPE" = "ipv4" ]; then
            IP_NUM=`echo "$ipInput" | $AWK -F "." '{for(x=1;x<=NF;x++) print $x}'`
            for loop in ${IP_NUM}
            do
                IP_START=`RDsubstr $loop 1 1`
                IP_NEXT=`RDsubstr $loop 2 1`
                
                if [ $FIRST_IP_SEG -eq 0 ] && [ $loop -eq 0 ]; then
                    Log "First ip address number[$loop] can not 0."
                    FLAG=1
                    break
                fi

                FIRST_IP_SEG=1 

                if [ $IP_START -eq 0 ]; then
                    if [ "$IP_NEXT" != "" ]; then
                        Log "Ip address number[$loop] can not start with 0."
                        FLAG=1
                        break
                    fi 
                fi

                if [ $loop -lt 0 ]||[ $loop -gt 255 ]; then
                    Log "The ip address number should be more than 0 and less than 255."
                    FLAG=1
                    break
                fi
            done
        fi
        
        if [ $FLAG -eq 1 ]; then
            echo "Ip address set failed."
            Log "Ip address was set failed."
            HDRS_NUMBER=`expr $HDRS_NUMBER + 1`
            sleep 1
            continue 
        fi
        SUExecCmd "${AGENT_ROOT_PATH}/bin/xmlcfg write Backup ebk_server_ip ${PM_IP_LIST}"
        break
    done

    if [ $HDRS_NUMBER = 3 ]; then
        echo ${ECHO_E} "The installation of DataBackup ProtectAgent will be stopped."
        LogError "The ip you entered($ipInput) is error, so installation was stopped." ${ERR_IPADDR_SET_FAILED}
        exit $ERR_IPADDR_SET_FAILED_RETCODE
    fi
  
    Log "Set pm_ip($PM_IP_LIST) successfully."
}

# get random port number, and check whether can be used
# rdagent: 59540~59559  nginx: 59520~59539
GetRandomPort()
{
    BeginSeq=$1
    FinalSeq=$2
    PotrUser=$3
    PortTemp=0

    Diff=`expr ${FinalSeq} - ${BeginSeq}`
    while true
    do
        # generate random port
        NumRandom=`cat /dev/urandom | head -n 10 | cksum | $AWK '{print $1}'`
        PortTemp=`expr ${NumRandom} % ${Diff} + ${BeginSeq}`
       
        # check whether the port is free 
        if [ "SunOS" = "${SYS_NAME}" ]; then
            Listeningnum=`netstat -an | grep "${PortTemp}" | $AWK '$NF == "ESTABLISHED"     {print $0}' | wc -l`
        else
            TCPListeningnum=`netstat -an | grep ":${PortTemp}" | $AWK '$1 == "tcp" && $NF == "LISTEN"     {print $0}' | wc -l`
            UDPListeningnum=`netstat -an | grep ":${PortTemp}" | $AWK '$1 == "udp" && $NF == "0.0.0.0:*"  {print $0}' | wc -l`
            Listeningnum=`expr ${TCPListeningnum} + ${UDPListeningnum}`
        fi
        
        if [ ${Listeningnum} -eq 0 ]; then
            if [ "${PotrUser}" = "rdagent" ]; then
                RDAGENT_PORT=${PortTemp}
            elif [ "${PotrUser}" = "nginx" ]; then
                NGINX_PORT=${PortTemp}
            fi
            return 0
        fi
    done
}

# $1 agent and nginx port number
SetPort()
{
    PORTREX="^[1-9][0-9]\\{0,4\\}$"
    PORT=0
    PORT_NUMBER=0
    while [ ${PORT_NUMBER} -lt 3 ]
    do
        #read PORT
        if [ "rdagent" = "$1" ]; then
            GetRandomPort 59540 59559 "rdagent"
            PORT=${RDAGENT_PORT}
        elif [ "nginx" = "$1" ]; then
            # VMWare Host Use a fixed port(59526),other use random port.
            if [ ${BACKUP_ROLE} -eq ${BACKUP_ROLE_VMWARE} ]; then
                PORT=59526
            else
                if [ -f "${NIGINX_BACKUP_CONFIG}" ]; then
                    PORT=`SUExecCmdWithOutput "cat ${NIGINX_BACKUP_CONFIG} | grep \"listen\" | $AWK '{print \\$2}' | $AWK -F \":\" '{print \\$NF}'"`
                    netstat -an | grep ${PORT} 1>/dev/null 2>&1
                    if [ $? -eq 0 ]; then
                        echo "The nginx listening port[${PORT}] is occupied, and a new port will be allocated randomly."
                        echo "Please refer to the ProtectAgent deployment chapter [Step 5: Enabling the Input and Output Functions of the Nginx Listening Port] for configuration."
                        Log "The nginx listening port[${PORT}] is occupied, and a new port will be allocated randomly."
                        Log "Please refer to the ProtectAgent deployment chapter [Step 5: Enabling the Input and Output Functions of the Nginx Listening Port] for configuration."
                        GetRandomPort 59520 59539 "nginx"
                        PORT=${NGINX_PORT}
                    fi
                elif [ "${BACKUP_ROLE}" = "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ]; then
                    GetRandomPort 59570 59580 "nginx"
                    PORT=${NGINX_PORT}
                else
                    GetRandomPort 59520 59539 "nginx"
                    PORT=${NGINX_PORT}
                fi
            fi
        fi

        if [ "${PORT}" = "" ]; then
            if [ "rdagent" = "$1" ]; then
                echo "You choose $1 default port number[8091]."
                PORT=8091
            elif [ "nginx" = "$1" ]; then 
                echo "You choose $1 default port number[59526]."
                PORT=59526
            fi
        fi

        PORTCHECK=`echo "${PORT}" | grep -n "${PORTREX}"`
        if [ "${PORTCHECK}" = "" ]; then
            echo "The port number should be contain 1 to 5 digits and start with 1-9."
            PORT_NUMBER=`expr ${PORT_NUMBER} + 1`
            sleep 1
            continue
        elif [ ${PORT} -gt 65535 -o ${PORT} -le 1024 ]; then
            echo "The port number should be more than 1024 and less than or equal to 65535."
            PORT_NUMBER=`expr ${PORT_NUMBER} + 1`
            sleep 1
            continue
        fi

        if [ "Linux" = "${SYS_NAME}" ]; then
            PORTS=`netstat -an | grep ${PORT} | grep -v TIME_WAIT | $AWK '{print $4}' | $AWK -F ':' '{print $NF}'`
        elif [ "HP-UX" = "${SYS_NAME}" ]; then
            PORTS=`netstat -an | grep ${PORT} | grep -v TIME_WAIT | $AWK -F " " '{print $4}' | $AWK -F "." '{print $NF}'`
        elif [ "AIX" = "${SYS_NAME}" ]; then
            PORTS=`netstat -an | grep ${PORT} | grep -v TIME_WAIT| $AWK -F '.' '{print $2}' | $AWK -F ' ' '{print $1}'`
        elif [ "SunOS" = "${SYS_NAME}" ]; then
            PORTS=`netstat -an | grep ${PORT} | grep -v TIME_WAIT | $AWK '{print $1}' | $AWK -F '.' '{print $NF}'`
        fi

        PORTSTATUS=""
        for PORTEACH in ${PORTS}
        do
            if [ "${PORTEACH}" = "${PORT}" ]; then
                PORTSTATUS=${PORTEACH}
                break
            fi
        done

        if [ "${PORTSTATUS}" != "" ]; then 
            echo "The port number(${PORT}) is used by other process."
            Log "The port number(${PORT}) is used by other process."
            PORT_NUMBER=`expr ${PORT_NUMBER} + 1`
            sleep 1
            continue
        fi
        
        if [ "${AGENT_PORT}" = "${PORT}" ]; then
            Log "Nginx port number is the same with agent port number(${AGENT_PORT})."
            echo "Nginx port number ${PORT} is the same with rdagent port number ${AGENT_PORT}."
            PORT_NUMBER=`expr ${PORT_NUMBER} + 1`
            sleep 1
            continue
        fi

        if [ "rdagent" = "$1" ]; then
            AGENT_PORT=${PORT}
        fi

        break
    done

    if [ ${PORT_NUMBER} = 3 ]; then
        echo "The installation of DataBackup ProtectAgent will be stopped."
        LogError "The port you entered($PORT) is error, so installation was stopped." ${ERR_PORT_SET_FAILED}
        exit $ERR_PORT_SET_FAILED_RETCODE
    fi
    
    if [ "rdagent" = "$1" ]; then
        SUExecCmd "${AGENT_ROOT_PATH}/bin/xmlcfg write System port ${PORT}"
        SUExecCmd "sed  \"/fastcgi_pass/s/.*/            fastcgi_pass   127.0.0.1:${PORT};/g\" \"${AGENT_ROOT_PATH}/nginx/conf/nginx.conf\" > ${AGENT_ROOT_PATH}/nginx/conf/nginx.conf.bak"
        SUExecCmd "mv -f ${AGENT_ROOT_PATH}/nginx/conf/nginx.conf.bak ${AGENT_ROOT_PATH}/nginx/conf/nginx.conf"
    elif [ "nginx" = "$1" ]; then
        if [ "ipv4" = "${G_NETWORK_TYPE}" ]; then 
            SUExecCmd "sed  \"/listen/s/.*/        listen       ${IP}:${PORT} ssl;/g\" ${AGENT_ROOT_PATH}/nginx/conf/nginx.conf > ${AGENT_ROOT_PATH}/nginx/conf/nginx.conf.bak"
        elif [ "ipv6" = "${G_NETWORK_TYPE}" ]; then
            ####IPV6 write to config end [xxx::xxx]
            SUExecCmd "sed  \"/listen/s/.*/        listen       [${IP}]:${PORT} ssl;/g\" ${AGENT_ROOT_PATH}/nginx/conf/nginx.conf > ${AGENT_ROOT_PATH}/nginx/conf/nginx.conf.bak"
        else
            Log "Set port failed, the network type = ${G_NETWORK_TYPE} is invailed."        
        fi
        if [ "${SYS_NAME}" = "SunOS" ] || [ "${SYS_NAME}" = "AIX" ]; then
            su - ${AGENT_USER} -c "mv -f ${AGENT_ROOT_PATH}/nginx/conf/nginx.conf.bak ${AGENT_ROOT_PATH}/nginx/conf/nginx.conf" >/dev/null
        else
            su - ${AGENT_USER} -s ${SHELL_TYPE_SH} -c "mv -f \"${AGENT_ROOT_PATH}/nginx/conf/nginx.conf.bak\" \"${AGENT_ROOT_PATH}/nginx/conf/nginx.conf\""
            #change nginx.conf 664 to 640
            chmod 640 ${AGENT_ROOT_PATH}/nginx/conf/nginx.conf
        fi
    fi
    echo "Set $1 listening port($PORT) successfully."
    Log "Set $1 listening port($PORT) successfully."
}

SetPMPort()
{
    SUExecCmd "${AGENT_ROOT_PATH}/bin/xmlcfg write Backup ebk_server_auth_port ${PM_PORT}"
}

StartAgent()
{
    Log "Start DataBackup ProtectAgent services."
    echo "Start DataBackup ProtectAgent services."

    if [ ${BACKUP_SCENE} -eq ${BACKUP_SCENE_INTERNAL} ]; then
        Log "Internal install."
        return
    fi

    if [ "${SYS_NAME}" = "SunOS" ] || [ "${SYS_NAME}" = "AIX" ]; then
        su - ${AGENT_USER} -c "${EXPORT_ENV}${AGENT_ROOT_PATH}/bin/agent_start.sh" >/dev/null
    else
        su - ${AGENT_USER} -s ${SHELL_TYPE_SH} 1>/dev/null ${UNIX_CMD} -c "${EXPORT_ENV}\"${AGENT_ROOT_PATH}/bin/agent_start.sh\""
    fi
    if [ 0 != $? ]; then
        echo "DataBackup ProtectAgent was installed failed."
        LogError "DataBackup ProtectAgent was installed failed." ${ERR_AGENT_START_FAILED}
        exit $ERR_AGENT_START_FAILED_RETCODE
    else
        Log "Start DataBackup ProtectAgent successfully."
    fi
}

SwitchOffReportHeartbeatToPm()
{
    touch "${AGENT_ROOT_PATH}/tmp/disable_heartbeat_to_pm"
    if [ ! -f "${AGENT_ROOT_PATH}/tmp/disable_heartbeat_to_pm" ]; then
        Log "Switch off report heart beat to pm fail."
        return 1
    fi
    chmod 755 "${AGENT_ROOT_PATH}/tmp/disable_heartbeat_to_pm"
}

SwitchOnReportHeartbeatToPm()
{
    rm -f "${AGENT_ROOT_PATH}/tmp/disable_heartbeat_to_pm"
    if [ -f "${AGENT_ROOT_PATH}/tmp/disable_heartbeat_to_pm" ]; then
        Log "Switch on report heart beat to pm fail."
    fi
}

RegisterHost()
{
    Log "Start register host."
    JudgeRandomNumType

    if [ ${BACKUP_SCENE} -eq ${BACKUP_SCENE_INTERNAL} ]; then
        Log "Internal install."
        return
    fi
    if [ "${SYS_NAME}" = "SunOS" ] || [ "${SYS_NAME}" = "AIX" ]; then
        if [ "${UPGRADE_FLAG_TEMPORARY}" = "1" ]; then
            su - ${AGENT_USER} -c "${AGENT_ROOT_PATH}/bin/agentcli registerHost RegisterHost Upgrade"
        else
            su - ${AGENT_USER} -c "${AGENT_ROOT_PATH}/bin/agentcli registerHost RegisterHost"
        fi
    else
        if [ "${UPGRADE_FLAG_TEMPORARY}" = "1" ]; then
            su - ${AGENT_USER} -s ${SHELL_TYPE_SH} -c "${EXPORT_ENV}${AGENT_ROOT_PATH}/bin/agentcli registerHost RegisterHost Upgrade"
        else
            su - ${AGENT_USER} -s ${SHELL_TYPE_SH} -c "${EXPORT_ENV}${AGENT_ROOT_PATH}/bin/agentcli registerHost RegisterHost"
        fi
    fi
    if [ "$?" = 0 ]; then
        Log "Register host to ProtectManager successfully."
        return 0
    else
        if [ -f "${AGENT_ROOT_PATH}log/AgentInstallMode.log" ]; then
            Log "Duplicate host IP address exists in the ProtectManager,Register host to ProtectManager failed." 
            LogError "Duplicate host IP address exists in the ProtectManager,Register host to ProtectManager failed." ${ERR_SAMEIP_INSTALL_PUSH}
            exit ${ERR_SAMEIP_INSTALL_PUSH}
        fi
        LogError "Register host to ProtectManager failed." ${ERR_UPGRADE_FAIL_REGISTER_TO_PM}
        exit 1
    fi
}

SetRegisterType()
{
    if [ "${BACKUP_ROLE}" = "${BACKUP_ROLE_HOST}" ]; then
        RegType="0"
        RegTypeName="oracle"
    elif [ "${BACKUP_ROLE}" = "${BACKUP_ROLE_VMWARE}" ]; then
        RegType="2"
        RegTypeName="vmware"
    elif [ "${BACKUP_ROLE}" = "${BACKUP_ROLE_GENERAL_PLUGIN}" ]; then
        RegType=4
        RegTypeName="GeneralPlugin"
    elif [ "${BACKUP_ROLE}" = "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ]; then
        RegType="5"
        RegTypeName="SanClient"  # 设置SanClient代理类型
    else
        RegType="0"
        RegTypeName="oracle"
    fi
    printf "Set ProtectAgent registration type: \\033[1;32m${RegTypeName}\\033[0m. \n"
    Log "Set ProtectAgent registration type: ${RegTypeName}"
    SUExecCmd "${AGENT_ROOT_PATH}/bin/xmlcfg write Backup backup_role ${RegType}"
}

SetHostType()
{
    if [ "${BACKUP_ROLE}" = "${BACKUP_ROLE_VMWARE}" ] && [ "${SYS_NAME}" = "Linux" ]; then
        IS_PHYSICAL_VIRTUAL=`cat /proc/cpuinfo | grep 'hypervisor'`
        Log "The host type of the current system is [${IS_PHYSICAL_VIRTUAL}]."
        if [ "${IS_PHYSICAL_VIRTUAL}" = "" ]; then
            SUExecCmd "${AGENT_ROOT_PATH}/bin/xmlcfg write DataProcess system_virt 0"
        else
            SUExecCmd "${AGENT_ROOT_PATH}/bin/xmlcfg write DataProcess system_virt 1"
        fi
    fi
}

XmlParseViaShell()
{
    XML_FILE=$1
    PARENT_FILED=$2
    CHILD_FILED=$3
    TEMP_FILE=${DEFAULT_HOME_DIR}/tempFile
    FILED_VALUE=

    if [ ! -f ${XML_FILE} ]; then
        echo "${XML_FILE} not exist."
        Log "xmlfile(${XML_FILE}) not exist.""${XML_FILE} not exist."
        return 1
    fi
    
    if [ "${SYS_NAME}" = "Linux" ]; then
        FILED_VALUE=`SUExecCmdWithOutput "cat ${XML_FILE} | grep -A 100 \"<${PARENT_FILED}>\" | grep -B 100 \"</${PARENT_FILED}>\" | grep \"${CHILD_FILED}\" | $AWK -F '=' '{print \\$2}' | $AWK -F '\"' '{print \\$2}'"` >/dev/null 2>&1
    elif [ "${SYS_NAME}" = "AIX" ]; then
        SUExecCmd "grep -n \"<${PARENT_FILED}>\" ${XML_FILE} | cut -d':' -f1 | xargs -n1 -I % $AWK 'NR>=% && NR<=%+100' ${XML_FILE} > ${TEMP_FILE}_1"
        SUExecCmd "grep -n \"</${PARENT_FILED}>\" ${TEMP_FILE}_1 | cut -d':' -f1 | xargs -n1 -I % $AWK 'NR>=%-100 && NR<=%' ${TEMP_FILE}_1 > ${TEMP_FILE}_2"
        FILED_VALUE=`SUExecCmdWithOutput "cat ${TEMP_FILE}_2 | grep \"${CHILD_FILED}\" | $AWK -F '=' '{print \\$2}' | $AWK -F '\"' '{print \\$2}'"` >/dev/null 2>&1
        SUExecCmd "rm -rf \"${TEMP_FILE}_\"*"
    else
        FILED_VALUE=`SUExecCmdWithOutput "cat ${XML_FILE} | grep \"${CHILD_FILED}\" | $AWK -F '=' '{print \\$2}' | $AWK -F '\"' '{print \\$2}'"` >/dev/null 2>&1
    fi

    if [ "${FILED_VALUE}" != "" ]; then
        echo ${FILED_VALUE}
        return 0
    else
        echo "FILED_VALUE is empty, file(${XML_FILE}), id(${PARENT_FILED}:${CHILD_FILED})."
        Log "FILED_VALUE is empty, file(${XML_FILE}), id(${PARENT_FILED}:${CHILD_FILED})."
        return 1
    fi
}

VerifyPasswd()
{
    JudgeRandomNumType
    if [ "${SYS_NAME}" = "SunOS" ] || [ "${SYS_NAME}" = "AIX" ]; then
        su - ${AGENT_USER} -c "${AGENT_ROOT_PATH}/bin/agentcli verifykey > ${DEFAULT_HOME_DIR}/verify.xml" >/dev/null
    else
        su - ${AGENT_USER} -s ${SHELL_TYPE_SH} -c "${EXPORT_ENV}${AGENT_ROOT_PATH}/bin/agentcli verifykey > ${DEFAULT_HOME_DIR}/verify.xml"
    fi
    PKEY_VERIFY=`SUExecCmdWithOutput "cat ${DEFAULT_HOME_DIR}/verify.xml| grep '^[^Enter]'"`
    SUExecCmd "rm -rf \"${DEFAULT_HOME_DIR}/verify.xml\""
    if [ "${PKEY_VERIFY}" = "failed" ]; then
        echo "Failed to verify the private key password."
        Log "Failed to verify the private key password."
        return ${ERR_VERIFY_PRIVATE_KEY_FAILED}
    else
        echo "The private key password is verified successfully."
        Log "The private key password is verified successfully."
        return 0
    fi
}

#该功能暂不使用
ConfigDpcNetworkInfo()
{
    if [ "${SYS_NAME}" != "Linux" ]
    then
        return 0
    fi
    if [ "${IS_DPC_COMPUTE_NODE}" = "null" ] || [ "${IS_DPC_COMPUTE_NODE}" = "" ]
    then
        choice_number=0
        while [ $choice_number -lt 3 ]
        do
            printf "\\033[1;32mCheck whether the host is an E6000 DPC compute node:(y|n), default(n)\\033[0m \n"
            printf "Your choice:"
            read node_choice
            if [ "$node_choice" = "" ] || [ "$node_choice" = "n" ]; then
                IS_DPC_COMPUTE_NODE="false"
                return 0
            elif [ "$node_choice" = "y" ]; then
                IS_DPC_COMPUTE_NODE="true"
                break
            else
                echo "Please enter y or n."
            fi
            choice_number=`expr $choice_number + 1`
        done
        if [ $choice_number -ge 3 ]
        then
            echo "The number of incorrect inputs exceeds 3. The installation process exits abnormally."
            LogError "The number of incorrect inputs exceeds 3. The installation process exits abnormally." ${ERR_CONFIG_POLICY_ROUTE_FAILED}
            exit ${ERR_CONFIG_POLICY_ROUTE_FAILED}
        fi
        dpc_storage_ip_number=0
        while [ true ]
        do
            printf "\\033[1;32mPlease input ip address and gateway address of the dpc compute node storage front network plane, use whitespace ( ) to separate the IP address and gateway address.\\033[0m \n"
            read address_str
            dpc_storage_ip=`echo "$address_str" | $AWK '{print $1}'`
            dpc_storage_gateway=`echo "$address_str" | $AWK '{print $2}'`
            sh ${AGENT_ROOT_PATH}/sbin/config_dpc_policy_route.sh $dpc_storage_ip $dpc_storage_gateway 1>/dev/null
            if [ $? -ne 0 ]
            then
                echo "Config storage front network policy route failed, ip:${dpc_storage_ip}, gateway:${dpc_storage_gateway}."
                LogError "Config storage front network policy route failed, ip:${dpc_storage_ip}, gateway:${dpc_storage_gateway}." ${ERR_CONFIG_POLICY_ROUTE_FAILED}
                exit ${ERR_CONFIG_POLICY_ROUTE_FAILED}
            fi
            dpc_storage_ip_number=`expr $dpc_storage_ip_number + 1`
            printf "\\033[1;32mDo you want to continue entering the IP address and gateway address?(y|n), default(n)\\033[0m \n"
            read continue_choice
            if [ "$continue_choice"  != "y" ]
            then
                break
            fi
            if [ $dpc_storage_ip_number -eq 4 ]
            then
                break
            fi
        done
    elif [ "${IS_DPC_COMPUTE_NODE}" = "true" ]
    then
        echo "The host is a dpc compute node of E6000."
        if [ "${DPC_STORAGE_FRONT_IP}" != "" ]
        then
            cir_number=1
            for dpc_ip in `echo "$DPC_STORAGE_FRONT_IP" | $MYAWK -F ',' '{for(x=1;x<=NF;x++) print $x}'`
            do
                dpc_gateway=`echo "$DPC_STORAGE_FRONT_GATEWAY" | $MYAWK -F ',' -v i=$cir_number '{print $i}'`
                ${AGENT_ROOT_PATH}/sbin/config_dpc_policy_route.sh "${dpc_ip}" "${dpc_gateway}" 1>/dev/null
                if [ $? -ne 0 ]
                then
                    echo "Config storage front network policy route failed, ip:${dpc_ip}, gateway:${dpc_gateway}."
                    LogError "Config storage front network policy route failed, ip:${dpc_ip}, gateway:${dpc_gateway}." ${ERR_CONFIG_POLICY_ROUTE_FAILED}
                    exit ${ERR_CONFIG_POLICY_ROUTE_FAILED}
                fi
                cir_number=`expr $cir_number + 1`
            done
        fi
    fi
}

ReadAndVerifyPasswd()
{
    Log "Start read and verify password."
    JudgeRandomNumType
    if [ "${BACKUP_SCENE}" = "${BACKUP_SCENE_INTERNAL}" ]; then
        Log "Internal install."
        return
    fi

    if [ "${UPGRADE_FLAG_TEMPORARY}" != "1" ]; then    # first install, not upgrade
        ATTEMPT_COUNT=0
        ATTEMPT_LEFT=3
        
        while [ $ATTEMPT_COUNT -lt 3 ]
        do
            printf "\\033[1;32mPlease enter the private key password set on ProtectManager, you still have $ATTEMPT_LEFT chances:\\033[0m \n"
            Log "Please enter the private key password set on ProtectManager, you still have $ATTEMPT_LEFT chances:"
            if [ "${SYS_NAME}" = "SunOS" ]; then
                su - ${AGENT_USER} -c "${EXPORT_ENV}${AGENT_ROOT_PATH}/bin/agentcli enckey cipherFile"
            elif [ "${SYS_NAME}" = "AIX" ]; then
                su - ${AGENT_USER} -c "${EXPORT_ENV}${AGENT_ROOT_PATH}/bin/agentcli enckey cipherFile"
            else
                su - ${AGENT_USER} -s ${SHELL_TYPE_SH} -c "${EXPORT_ENV}${AGENT_ROOT_PATH}/bin/agentcli enckey cipherFile"
            fi

            # check whether the encrypted file exists
            if [ -f "${AGENT_ROOT_PATH}/tmp/input_tmpcipherFile" ]; then
                PKEY_PASSWORD=`SUExecCmdWithOutput "cat ${AGENT_ROOT_PATH}/tmp/input_tmpcipherFile"`
                SUExecCmd "rm -rf \"${AGENT_ROOT_PATH}/tmp/input_tmpcipherFile\""
                if [ "${PKEY_PASSWORD}" != "" ]; then
                    SUExecCmd "${AGENT_ROOT_PATH}/bin/xmlcfg write Monitor nginx ssl_key_password ${PKEY_PASSWORD}"
                    VerifyPasswd
                    if [ 0 -eq $? ]; then
                        Log "Succeed to read and verify the private key password."
                        return 0
                    fi
                else 
                    echo "Not input any valid private key password."
                    Log "Not input any valid private key password."
                fi
            else
                echo "Not generate the encrypted file, please check the resource of the host."
                Log "Can not generate temporary file, please check the resource of the host(disk space, cpu and etc.)."
            fi

            ATTEMPT_LEFT=`expr $ATTEMPT_LEFT - 1`
            ATTEMPT_COUNT=`expr $ATTEMPT_COUNT + 1`
        done
        if [ $ATTEMPT_COUNT -ge 3 ]; then
            echo "You have tried 3 times, failed to verify the passwd, exit installation."
            echo "Please uninstall the client before reinstall it."
            Log "You have tried 3 times, failed to verify the passwd, exit installation."
            LogError "Please uninstall the client before reinstall it." ${ERR_VERIFY_PRIVATE_KEY_FAILED}
            exit ${ERR_VERIFY_PRIVATE_KEY_FAILED}
        fi
    else # upgrade process
        if [ -f "${AGENT_BACKUP_PATH}/ProtectClient-E/nginx/conf/pmca.pem" ]; then 
            PKEY_PASSWORD=`XmlParseViaShell ${AGENT_BACKUP_PATH}/ProtectClient-E/conf/agent_cfg.xml Monitor ssl_key_password`
            SUExecCmd "${AGENT_ROOT_PATH}/bin/xmlcfg write Monitor nginx ssl_key_password ${PKEY_PASSWORD}"
        else
            SUExecCmd "${AGENT_ROOT_PATH}/bin/xmlcfg write Monitor nginx ssl_key_password ${UPGRADE_PASSWORD}"
            unset UPGRADE_PASSWORD
        fi
        VerifyPasswd
        if [ 0 -eq $? ]; then
            Log "Succeed to read and verify the private key password in upgrade process."
            return 0 
        else
            LogError "Failed to read and verify the private key password in upgrade process." ${ERR_VERIFY_PRIVATE_KEY_FAILED}
            ShowError "Failed to read and verify the private key password in upgrade process."
            exit ${ERR_VERIFY_PRIVATE_KEY_FAILED}
        fi
    fi
}

EnableSSLCertVerify()
{
    NGINX_CONF_PATH="${AGENT_ROOT_PATH}/nginx/conf/nginx.conf"
    if [ ! -f "${NGINX_CONF_PATH}" ]; then
        echo "Failed to modify the authentication type."
        LogError "The nginx file(${NGINX_CONF_PATH}) does not exist." ${ERR_UPGRADE_INSTALL_PACKAGE_INCOMPLETE}
        exit 1
    fi
    SUExecCmd "cat \"${NGINX_CONF_PATH}\" | grep -w \"ssl_verify_client\" >/dev/null"
    if [ $? -ne 0 ]; then
        echo "Failed to modify the authentication type."
        LogError "The nginx conf file no item ssl_verify_client." ${ERR_UPGRADE_FAIL_READ_CONFIGURE}
        exit 1
    fi
    SUExecCmd "sed  '/ssl_verify_client/s/.*/        ssl_verify_client on;/g' ${NGINX_CONF_PATH} > ${NGINX_CONF_PATH}.bak"
    if [ $? -ne 0 ]; then
        echo "Failed to modify the authentication type."
        LogError "Enable certification verify failed." ${ERR_UPGRADE_FAIL_SAVE_CONFIGURE}
        exit 1
    fi
    
    if [ "${SYS_NAME}" = "SunOS" ] || [ "${SYS_NAME}" = "AIX" ]; then
        su - ${AGENT_USER} -c "mv -f ${NGINX_CONF_PATH}.bak ${NGINX_CONF_PATH}" >/dev/null
    else
        su - ${AGENT_USER} -s ${SHELL_TYPE_SH} -c "mv -f \"${NGINX_CONF_PATH}.bak\" \"${NGINX_CONF_PATH}\""
    fi

    if [ $? -ne 0 ]; then
        echo "Failed to modify the authentication type."
        LogError "Moving ${NGINX_CONF_PATH}.bak to ${NGINX_CONF_PATH} failed." ${ERR_UPGRADE_FAIL_BACKUP}
        exit 1
    fi
    
    #modify agent config, 1: password auth, 2: cert auth
    AUTH_TYPE=2
    SUExecCmd "${AGENT_ROOT_PATH}/bin/xmlcfg write System auth_type ${AUTH_TYPE}"
    if [ $? -ne 0 ]; then
        echo "Failed to modify the authentication type."
        LogError "Configuration auth type failed." ${ERR_UPGRADE_FAIL_SAVE_CONFIGURE}
        exit 1
    fi
}

HandleSslHostname()
{
    echo "Start to configure hostname."
    Log "Start to configure hostname."
    SUExecCmdWithOutput "${EXPORT_ENV}${AGENT_ROOT_PATH}/bin/agentcli gethostname > ${DEFAULT_HOME_DIR}/verify.xml"
    tempHostname=`SUExecCmdWithOutput "cat ${DEFAULT_HOME_DIR}/verify.xml"`
    SUExecCmd "rm -rf ${DEFAULT_HOME_DIR}/verify.xml"
    if [ "${tempHostname}" = "" ]; then
        echo "Get hostname from certificate failed."
        LogError "Get hostname from certificate failed." ${ERR_UPGRADE_FAIL_READ_CONFIGURE}
        exit 1
    fi
    VerifySpecialChar "${tempHostname}"
    ret=`grep ${tempHostname} /etc/hosts`
    if [ "${ret}" = "" ]; then
        echo "${LO_IP_IPV4} ${tempHostname}" >> /etc/hosts
        echo "${LO_IP_IPV6} ${tempHostname}" >> /etc/hosts
    fi
    if [ "ipv6" = "${G_NETWORK_TYPE}" ]; then
        retv6=`grep ${tempHostname} /etc/hosts | grep ${LO_IP_IPV6}`
        if [ "${retv6}" = "" ]; then
            echo "${LO_IP_IPV6} ${tempHostname}" >> /etc/hosts
        fi
    fi
}

ConfigHostInfo()
{
    SUExecCmd "${AGENT_ROOT_PATH}/bin/xmlcfg write System client_version \"${PKG_VERSION}\""
    SUExecCmd "${AGENT_ROOT_PATH}/bin/xmlcfg write System version_time_stamp \"${VERSION_TIME_STAMP}\""
    SUExecCmd "${AGENT_ROOT_PATH}/bin/xmlcfg write System domain_name ${DOMAIN_NAME}"
}

ConfigRunDependency()
{
    ConfigDwsHost
    DisableHttpProxy
}

ConfigDwsHost()
{
    if [ ${BACKUP_SCENE} -eq ${BACKUP_SCENE_INTERNAL} ] || [ ${BACKUP_ROLE} -eq ${BACKUP_ROLE_SANCLIENT_PLUGIN}  ]; then
        return 0
    fi

    if [ "SunOS" = "$SYS_NAME" ]; then
        return 0
    fi


    Log "Configuring the dws host."
    if [ ${BACKUP_ROLE} -eq ${BACKUP_ROLE_GENERAL_PLUGIN} ]; then
        # handle hostname
        HandleSslHostname
        # install xbsa
        mkdir -p ${XBSA_INSTALL_LIB_DIR} > /dev/null 2>&1
        mkdir -p ${XBSA_INSTALL_LOG_DIR} > /dev/null 2>&1
        mkdir -p ${XBSA_INSTALL_CERT_DIR} > /dev/null 2>&1
        mkdir -p ${XBSA_INSTALL_KMC_DIR} > /dev/null 2>&1
        mkdir -p ${XBSA_INSTALL_NGINX_CONF_DIR} > /dev/null 2>&1
        chmod -R 555 ${XBSA_INSTALL_DIR}
        # install xbsa lib
        cp ${INSTALL_PATH}/ProtectClient-E/bin/plugins/libxbsa64* ${XBSA_INSTALL_LIB_DIR}
        if [ ${SYS_NAME} = "AIX" ]; then
            cp "${INSTALL_PATH}/ProtectClient-E/bin/libstdc++.a" "${XBSA_INSTALL_LIB_DIR}"
            cp "${INSTALL_PATH}/ProtectClient-E/bin/libgcc_s.a" "${XBSA_INSTALL_LIB_DIR}"
            cp "${INSTALL_PATH}/ProtectClient-E/bin/libcommon.a" "${XBSA_INSTALL_LIB_DIR}"
            cp "${INSTALL_PATH}/ProtectClient-E/bin/libsecurecom.a" "${XBSA_INSTALL_LIB_DIR}"
            cp "${INSTALL_PATH}/ProtectClient-E/bin/libcommon.a" /lib64/
            cp "${INSTALL_PATH}/ProtectClient-E/bin/libsecurecom.a" /lib64/
            chmod 555 /lib64/libcommon.a
            chmod 555 /lib64/libsecurecom.a
        else
            cp ${INSTALL_PATH}/ProtectClient-E/bin/libstdc++.so.6.* "${XBSA_INSTALL_LIB_DIR}"
            ln -s ${XBSA_INSTALL_LIB_DIR}/libstdc++.so.6.* "${XBSA_INSTALL_LIB_DIR}/libstdc++.so.6"
            cp "${INSTALL_PATH}/ProtectClient-E/bin/libcommon.so" "${XBSA_INSTALL_LIB_DIR}"
            cp "${INSTALL_PATH}/ProtectClient-E/bin/libsecurecom.so" "${XBSA_INSTALL_LIB_DIR}"
            cp "${INSTALL_PATH}/ProtectClient-E/bin/libsecurecomv1.so" "${XBSA_INSTALL_LIB_DIR}"   
            cp "${INSTALL_PATH}/ProtectClient-E/bin/libsecurecomforgauss.so" "${XBSA_INSTALL_LIB_DIR}"
            rm -rf ${INSTALL_PATH}/ProtectClient-E/bin/libsecurecomforgauss.so
            rm -rf ${INSTALL_PATH}/ProtectClient-E/bin/plugins/libxbsa64forgauss.so
            cp "${INSTALL_PATH}/ProtectClient-E/bin/libcommon.so" /lib64/
            cp "${INSTALL_PATH}/ProtectClient-E/bin/libsecurecom.so" /lib64/
            chmod 555 /lib64/libcommon.so
            chmod 555 /lib64/libsecurecom.so
        fi
        chmod 550 ${XBSA_INSTALL_LIB_DIR}/*
        chown -h root:${AGENT_GROUP} ${XBSA_INSTALL_LIB_DIR}/*
        # install xbsa cert
        cp ${INSTALL_PATH}/ProtectClient-E/conf/thrift/client/* ${XBSA_INSTALL_CERT_DIR}
        chmod 660 ${XBSA_INSTALL_CERT_DIR}/*
        chown root:${AGENT_GROUP} ${XBSA_INSTALL_CERT_DIR}/*
        # install xbsa conf, used to connect thrift
        cp ${INSTALL_PATH}/ProtectClient-E/conf/agent_cfg.xml ${XBSA_INSTALL_CONF_DIR}
        cp ${INSTALL_PATH}/ProtectClient-E/conf/kmc_config.txt ${XBSA_INSTALL_CONF_DIR}
        cp ${INSTALL_PATH}/ProtectClient-E/conf/kmc_store.txt ${XBSA_INSTALL_CONF_DIR}
        cp ${INSTALL_PATH}/ProtectClient-E/conf/kmc/* ${XBSA_INSTALL_KMC_DIR}
        chmod 440 ${XBSA_INSTALL_KMC_DIR}/*
        chmod 550 ${XBSA_INSTALL_CONF_DIR}/*
        chmod 440 ${XBSA_INSTALL_CONF_DIR}/*.txt
        chown root:${AGENT_GROUP} ${XBSA_INSTALL_KMC_DIR}/*
        chown root:${AGENT_GROUP} ${XBSA_INSTALL_CONF_DIR}/*
        # install xbsa nginx conf and cert, used to connect DEM
        cp ${INSTALL_PATH}/ProtectClient-E/nginx/conf/kmc_store_bak.txt ${XBSA_INSTALL_NGINX_CONF_DIR}
        cp ${INSTALL_PATH}/ProtectClient-E/nginx/conf/pmca.pem ${XBSA_INSTALL_NGINX_CONF_DIR}
        cp ${INSTALL_PATH}/ProtectClient-E/nginx/conf/server.key ${XBSA_INSTALL_NGINX_CONF_DIR}
        cp ${INSTALL_PATH}/ProtectClient-E/nginx/conf/server.pem ${XBSA_INSTALL_NGINX_CONF_DIR}
        chmod 600 ${XBSA_INSTALL_NGINX_CONF_DIR}/kmc_store_bak.txt
        chmod 600 ${XBSA_INSTALL_NGINX_CONF_DIR}/*.pem
        chmod 400 ${XBSA_INSTALL_NGINX_CONF_DIR}/*.key
        chown ${AGENT_USER}:${AGENT_GROUP} ${XBSA_INSTALL_NGINX_CONF_DIR}/*
        # set xbsa log mode
        chmod 700 ${XBSA_INSTALL_LOG_DIR}
        chown -h ${AGENT_USER}:${AGENT_GROUP} ${XBSA_INSTALL_LOG_DIR}
    else
        SUExecCmd "sed \"/libdws/d\" ${INSTALL_PATH}/ProtectClient-E/conf/pluginmgr.xml > ${INSTALL_PATH}/ProtectClient-E/conf/pluginmgr.xml.bak"
        SUExecCmd "mv -f ${INSTALL_PATH}/ProtectClient-E/conf/pluginmgr.xml.bak ${INSTALL_PATH}/ProtectClient-E/conf/pluginmgr.xml"
        SUExecCmd "chmod 600 ${INSTALL_PATH}/ProtectClient-E/conf/pluginmgr.xml"
    fi
}

# determin execute user by different ways
CheckExecUser()
{
    LOG_USER=${LOGNAME}
    ENV_USER=${USER}
    ENV_UID=${UID}

    if [ "SunOS" = "$SYS_NAME" ]; then
        command -v whoami >/dev/null
        if [ $? -eq 0 ]; then
            WHO_AM_I=`whoami`
        else
            WHO_AM_I=`/usr/ucb/whoami`
        fi    
    else
        WHO_AM_I=`whoami`
    fi

    if [ "root" = "${LOG_USER}" ] || [ "root" = "${ENV_USER}" ] || [ "0" = "${ENV_UID}" ] || [ "root" = "${WHO_AM_I}" ]; then
        return 0
    else
        return 1
    fi
}

WriteNetworkInfo()
{
    SUExecCmd "${AGENT_ROOT_PATH}/bin/xmlcfg write Backup link_ebk_server_nic ${NIC}"
    return 0
}

SetUserId()
{
    Log "Setting the userid value of ProtectAgent."

    if [ ${BACKUP_SCENE} -eq ${BACKUP_SCENE_INTERNAL} ]; then
        Log "Internal install."
        return
    fi

    if [ "${USER_ID}" != "" ]; then
        SUExecCmd "${AGENT_ROOT_PATH}/bin/xmlcfg write System userid ${USER_ID}"
    else 
        echo "No valid userid."
        Log "No valid userid."
        exit 1
    fi

    return 0
}

InstallFileClient()
{
    # Fileclient support linux OS only.
    if [ "${SYS_NAME}" != "Linux" ]; then
        Log "Fileclient do not surport this system!"
        return 0
    fi

    if [ "${IS_FILECLIENT_INSTALL}" != "" ] && [ "${IS_FILECLIENT_INSTALL}" != "true" ]; then
        Log "Fileclient do not need install!"
        return 0
    fi
    
    SYS_ARCH=`arch`
    if [ "${SYS_ARCH}" = "x86_64" ]; then
        PATH_FILECLIENT_PACKAGE=${INSTALL_PACKAGE_PATH}/third_party_software/fileClient_x86_64.tar.gz
    else
        PATH_FILECLIENT_PACKAGE=${INSTALL_PACKAGE_PATH}/third_party_software/fileClient_aarch64.tar.gz
    fi

    if [ ! -f ${PATH_FILECLIENT_PACKAGE} ]; then
        Log "Do not need to install FileClient!"
        return 0
    fi
    FILECLIENT_INSTALL_PATH=${DATA_BACKUP_AGENT_HOME}/DataBackup/FileClient
    if [ -d ${FILECLIENT_INSTALL_PATH} ]; then
        echo "Failed to install fileclient, the path: ${FILECLIENT_INSTALL_PATH} already existed!"
        Log "Failed to install fileclient, the path: ${FILECLIENT_INSTALL_PATH} already existed!"
        return 1
    fi

    if [ -f ${AGENT_ROOT_PATH}/nginx/conf/encry_pwd.txt ];then
        rm -f ${AGENT_ROOT_PATH}/nginx/conf/encry_pwd.txt
    fi
    
    ENCRY_PWD_STR=${PKEY_PASSWORD}
    echo ${ENCRY_PWD_STR} > ${AGENT_ROOT_PATH}/nginx/conf/encry_pwd.txt
    chown ${AGENT_USER}:${AGENT_USER} ${AGENT_ROOT_PATH}/nginx/conf/encry_pwd.txt
    chmod 400 ${AGENT_ROOT_PATH}/nginx/conf/encry_pwd.txt

    AGENTCA_PEM_PATH=${AGENT_ROOT_PATH}/nginx/conf/pmca.pem
    SERVER_PEM_PATH=${AGENT_ROOT_PATH}/nginx/conf/server.pem
    SERVER_KEY_PATH=${AGENT_ROOT_PATH}/nginx/conf/server.key
    KMC_STORE_PATH=${AGENT_ROOT_PATH}/conf/kmc_store.txt
    KMC_BACKUP_PATH=${AGENT_ROOT_PATH}/nginx/conf/kmc_store_bak.txt
    ENCRY_PWD_PATH=${AGENT_ROOT_PATH}/nginx/conf/encry_pwd.txt

    Log "Start to install fileclient!"
    echo "Start to install fileclient!"
    mkdir ${FILECLIENT_INSTALL_PATH}
    tar -xf ${PATH_FILECLIENT_PACKAGE} -C ${FILECLIENT_INSTALL_PATH}
    cd ${FILECLIENT_INSTALL_PATH}/install
    ./install.sh ${AGENTCA_PEM_PATH} ${SERVER_PEM_PATH} ${SERVER_KEY_PATH} ${KMC_STORE_PATH} ${KMC_BACKUP_PATH} ${ENCRY_PWD_PATH} >> ${LOG_FILE_NAME} 2>&1
    RET_CODE=$?
    if [ ${RET_CODE} -ne 0 ]; then
        echo "Install fileclient failed."
        log "Install fileclient failed."
        return 1
    fi
    
    echo "Install fileclient successfully."
    return 0
}

ManualDealDataturbo()
{
    if [ "${INSTALLATION_MODE}" = "push" ]; then
        return 0
    fi
    if [ "${IS_ENABLE_DATATURBO}" = "false" ]  || [ "${IS_ENABLE_DATATURBO}" = "" ]; then
        choice_number=0
        while [ $choice_number -lt 3 ]
        do
            printf "\\033[1;32mWhether Open Dataturbo Service:(y|n), default(n)\\033[0m \n"
            printf "Your choice:"
            read node_choice
            if [ "$node_choice" = "" ] || [ "$node_choice" = "n" ]; then
                is_enable_dataturbo="false"
                Log "user choose close dataturbo service."
                sed -i "s/^is_enable_dataturbo=.*/is_enable_dataturbo=$is_enable_dataturbo/g" "${INSTALL_PACKAGE_PATH}/conf/client.conf"
                return 0
            elif [ "$node_choice" = "y" ]; then
                is_enable_dataturbo="true"
                Log "user choose open dataturbo service."
                sed -i "s/^is_enable_dataturbo=.*/is_enable_dataturbo=$is_enable_dataturbo/g" "${INSTALL_PACKAGE_PATH}/conf/client.conf"
                return 0
            else
                echo "Please enter y or n."
            fi
            choice_number=`expr $choice_number + 1`
        done
        if [ $choice_number -ge 3 ]; then
            echo "The number of incorrect inputs exceeds 3. The installation process exits abnormally."
            LogError "The number of incorrect inputs exceeds 3. The installation process exits abnormally." ${ERR_CONFIG_POLICY_ROUTE_FAILED}
            sed -i "s/^is_enable_dataturbo=.*/is_enable_dataturbo=false/g" "${INSTALL_PACKAGE_PATH}/conf/client.conf"
            return ${ERR_CONFIG_POLICY_ROUTE_FAILED}
        fi
    fi
}

HandleInstallDataTurbo()
{
    if [ "$UPGRADE_FLAG_TEMPORARY" = "1" ]; then
        return
    fi

    # 支持sanclient与通用代理注册同一台主机
    if [ "${BACKUP_ROLE}" = "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ]; then
         # 默认使用rpm查询
        rpm -q dataturbo >/dev/null 2>&1
        if [ $? -eq 0 ]; then
            Log "Sanclient dataturbo have been installed."
            return 0
        fi
    fi

    # 支持sanclient与通用代理注册同一台主机
    TMP_BACK_ROLE=`su - ${SANCLIENT_USER} 2>/dev/null -s ${SHELL_TYPE_SH} -c "sed '/^BACKUP_ROLE=/!d;s/.*=//' ${DATA_BACKUP_SANCLIENT_HOME}/DataBackup/SanClient/ProtectClient-E/conf/testcfg.tmp "`
    if [ "${BACKUP_ROLE}" = "${BACKUP_ROLE_GENERAL_PLUGIN}" -a "${TMP_BACK_ROLE}" = "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ]; then
         # 默认使用rpm查询
        rpm -q dataturbo >/dev/null 2>&1
        if [ $? -eq 0 ]; then
            Log "Agent dataturbo have been installed."
            return 0
        fi
    fi
    #分布式场景不支持安装dataturbo
    CheckDistributedScenes
    if [ $? -ne 0 ]; then
        ShowWarning "The Distributed Scenes do not support dataturbo."
        Log "The Distributed Scenes do not support dataturbo."
        return
    fi
    
    #代理界面选择dataturbo安装
    ManualDealDataturbo

    #服务关闭不会安装dataturbo
    SkipDataturboInstall
    if [ $? -ne 0 ]; then
        ShowWarning "Service close and not install dataturbo."
        Log "Service close and not install dataturbo."
        return
    fi

    # 不允许环境预先安装 dataturbo
    IsDataTurboInstalled
    if [ $? -eq 0 ]; then
        ShowError "The dataturbo already installed, need uninstall it."
        Log "The dataturbo already installed, need uninstall it."
        LogError "The dataturbo already installed, need uninstall it."${ERR_DATATURBO_INSTALL_FAILED}
        exit $ERR_DATATURBO_INSTALL_FAILED
    fi

    ShowInfo "Start Install dataturbo ..."
    unzipdir=`which unzip 2>/dev/null | awk '{print $1}'`
    if [ "" = "${unzipdir}" ]; then
        InstallDataTurbo "${INSTALL_PACKAGE_PATH}/third_party_software/dataturbo.tar.gz" "tar"
    else 
        InstallDataTurbo "${INSTALL_PACKAGE_PATH}/third_party_software/dataturbo.zip" "zip"
    fi
    # 安装dataturbo失败
    if [ $? -ne 0 ]; then
        ShowError "Install dataturbo failed."
        LogError "Install dataturbo failed."${ERR_DATATURBO_INSTALL_FAILED}
        exit $ERR_DATATURBO_INSTALL_FAILED
    fi
    ShowInfo "Install dataturbo success or skip dataturbo install."
}

RecordPluginSelfFiles()
{
    baseDir=$1
    relativeDir=$2
    outFile="${baseDir}/${SCAN_DIR_RECORD}"
    if [ ! -d "${baseDir}" ]; then
        Log "Record Plugin Self Files: dir ${baseDir} not exist."
        return
    else
        Log "Record Plugins self files in ${baseDir}/${relativeDir}."
    fi
    
    if [ -z "${relativeDir}" ]; then
        if [ -f "${outFile}" ]; then
            echo "file ${outFile} exist"
            mv -f ${outFile} "${outFile}_bak"
            touch ${outFile}
        else
            touch ${outFile}
        fi
    fi

    currentDir="${baseDir}/${relativeDir}"
    for file in `ls -a ${currentDir}`
    do
        if [ -d "${currentDir}/${file}" ]; then
            dir=${file}
            if [ "${dir}" != "." ] && [ "${dir}" != ".." ] && [ "${dir}" != "./" ] && [ "${dir}" != "../" ]; then
                RecordPluginSelfFiles "${baseDir}" "${relativeDir}/${dir}"
            fi
        else
            rPath="${relativeDir}/${file}"
            rPath=`RDsubstr ${rPath} 2`
            echo "${rPath}" >> "${outFile}"
        fi
    done
}

ClearPluginSelfFiles()
{
    baseDir=$1
    recordFile="${baseDir}/${SCAN_DIR_RECORD}"
    while read line
    do
        file="${baseDir}/${line}"
        if [ ${file} != ${recordFile} ]; then
            rm -f ${file}
        fi
    done < ${recordFile}
}

RestoringPluginBak()
{
    srcDir=$1
    dstDir=$2
    if [ -d "${srcDir}" ]; then
        if [ ! -d "$dstDir" ]; then
            Log "The target directory[${dstDir}] does not exist."
            return
        fi
        if [ -f "${srcDir}/${SCAN_DIR_RECORD}" ]; then
            Log "Plugin dir's record file ${srcDir}/${SCAN_DIR_RECORD} exist."
            ClearPluginSelfFiles "${srcDir}"       # 清除原插件能自带的文件，其余新增文件保留
        else
            Log "Plugin dir's record file ${srcDir}/${SCAN_DIR_RECORD} not exist."
        fi
        cp -r -p -n "${srcDir}" "${dstDir}"
    else
        Log "The source directory[${srcDir}] does not exist."
    fi
}

RestoreFile()
{
    tmpSrcFile=$1
    tmpDstFile=$2

    if [ -f "${tmpSrcFile}" ]; then
        Log "Restore plugin file ${tmpSrcFile} to ${tmpDstFile}."
        cat "${tmpSrcFile}" > ${tmpDstFile}
    else
        Log "The source file[${tmpSrcFile}] does not exist."
    fi
}

RestoringPluginBakCoverFiles()
{
    srcFile=$1
    dstDir=$2
    if [ -f "${srcFile}" ]; then
        Log "Plugin dir's record file ${srcFile} exist."
        cp -p -f "${srcFile}" "${dstDir}"
    fi
}

replaceContent()
{
    rowNum=$1
    keyMatched=$2
    contentToReplace=$3
    if [[ ${contentToReplace} == */* ]]; then
        echo "${rowNum}s!${keyMatched}!${contentToReplace}!g" >>"$PATH_SED_SCRIPT"
    else
        echo "${rowNum}s/${keyMatched}/${contentToReplace}/g" >>"$PATH_SED_SCRIPT"
    fi
}

MatchKeyWords()
{
    keyWord=$1
    fileToMatch=$2
    minrow=$3
    maxrow=$4

    IFS_OLD=$IFS
    rowsMatch=`$AWK -F '=' '{if ($1 == key) print NR}' key="$keyWord" "$fileToMatch"` # 此处 "==" 不要改！！！除非能够合理重构
    IFS=$IFS_OLD
    if [ -z "$rowsMatch" ]; then
        echo ""
        return
    fi
    # 多行相同内容匹配，取范围内的
    IFS=$'\n'
    tempRowsMatch=`echo "$rowsMatch" | ${AWK} -F "/n" '{for(x=1;x<=NF;x++) print $x}'`
    for i in ${tempRowsMatch}; do
        if (( $i > $minrow && $i < $maxrow )); then
        echo $i
        break
        fi
    done
    IFS=$IFS_OLD
}

MergeConfFile()
{
    confFileOld=$1
    confFileNew=$2
    pluginName=$3
    fileOfSedScript="./temp_script.sed"
    if [ ! -f "$confFileOld" ]; then
        LogError "Old config file:${confFileOld} not exist"
        return
    fi
    if [ ! -f "$confFileNew" ]; then
        LogError "New config file:${confFileNew} not exist"
        return
    fi

    Log "Merge config files of ${pluginName}"

    if [ ! -f "$fileOfSedScript" ]; then
        touch temp_script.sed
        Log "Build temp_script.sed"
    fi

    PATH_SED_SCRIPT=$fileOfSedScript

    cat /dev/null >"$fileOfSedScript"   # 清空sed脚本文件

    # linux环境中去除Windows中的\r符号
    sed -i 's/\r//g' "$confFileOld"
    sed -i 's/\r//g' "$confFileNew"

    # 旧配置文件拆分
    IFS_OLD=$IFS
    oldFileRows=`$AWK -F '=' '{print $0}' "$confFileOld"`
    IFS=$'\n'

    jump=1
    nextSectionRow=`sed -n '1,$ {/^\[[^]]*\]$/ {=; q}}' $confFileNew`
    maxRowOfNewFile=`(wc -l < ${confFileNew})`
    ((maxRowOfNewFile ++))
    sectionMatch=1

    # 匹配
    for tempRow in $oldFileRows; do
        if [ -z "$tempRow" ]; then
            continue
        fi

        if [[ $tempRow == [* ]];then
            sectionMatch=`MatchKeyWords "$tempRow" "$confFileNew" "0" "${maxRowOfNewFile}"`
            if [ -z "$sectionMatch" ];then
                jump=0
                continue
            else
                jump=1
                TMP=`expr $sectionMatch + 1`
                nextSectionRow=`sed -n "${TMP},$ {/^\[[^]]*\]$/ {=; q}}" $confFileNew`
                if [ -z "$nextSectionRow" ];then
                    nextSectionRow=$maxRowOfNewFile
                fi
            fi
        fi
        if [ $jump = 0 ];then
            continue
        fi

        IFS=$'='    # 每行按等号分割
        keyWordOfOldFile=`echo "$tempRow" | ${AWK} -F "=" '{print $1}'`

        # 配置项匹配替换
        rowMatchNewFile=`MatchKeyWords "$keyWordOfOldFile" "$confFileNew" "$sectionMatch" "$nextSectionRow"`
        if [ -z "$rowMatchNewFile" ]; then
            continue
        else
            replaceContent "$rowMatchNewFile" "$keyWordOfOldFile.*" "$tempRow"
        fi
    done
    IFS=${IFS_OLD}

    # 运行sed script修改配置文件
    if [ ! -s "$confFileOld" ]; then
        echo -n "#head" >>"$confFileOld" # 如果文件为空，则需要添加一个头部内容才能用sed命令,否则sed命令无效
    fi
    Log "Run temp_script.sed"
    sed -i -f "$fileOfSedScript" "$confFileNew"
    rm -f temp_script.sed
    Log "Finish merge config files"
}

RecordPluginsSelfFiles()
{
    RecordPluginSelfFiles "${INSTALL_PATH}/Plugins/tmp" ""

    RecordPluginSelfFiles "${INSTALL_PATH}/Plugins/FilePlugin/conf" ""

    RecordPluginSelfFiles "${INSTALL_PATH}/Plugins/FusionComputePlugin/bin/vbstool" ""
    RecordPluginSelfFiles "${INSTALL_PATH}/Plugins/FusionComputePlugin/tmp" ""

    RecordPluginSelfFiles "${INSTALL_PATH}/Plugins/VirtualizationPlugin/vbstool" ""
    RecordPluginSelfFiles "${INSTALL_PATH}/Plugins/VirtualizationPlugin/cert" ""
    RecordPluginSelfFiles "${INSTALL_PATH}/Plugins/VirtualizationPlugin/conf/hcpconf.ini" ""

    RecordPluginSelfFiles "${INSTALL_PATH}/Plugins/GeneralDBPlugin/bin/applications/dws" ""
}

RestoringPluginsBak()
{
    if [ -d "${LATEST_BAK_DIR}/plugins" ]; then
        if [ "${SYS_NAME}" = "Linux" ]; then
            # 配置文件 合并
            MergeConfFile "${LATEST_BAK_DIR}/plugins/VirtualizationPlugin/conf/hcpconf.ini" "${INSTALL_PATH}/Plugins/VirtualizationPlugin/conf/hcpconf.ini" "VirtualizationPlugin"
            MergeConfFile "${LATEST_BAK_DIR}/plugins/FusionComputePlugin/conf/hcpconf.ini" "${INSTALL_PATH}/Plugins/FusionComputePlugin/conf/hcpconf.ini" "FusionComputePlugin"
        fi

        Log "Restoring Plugins Back from ${LATEST_BAK_DIR}......"

        RestoringPluginBak "${LATEST_BAK_DIR}/plugins/tmp" "${INSTALL_PATH}/Plugins/"

        RestoringPluginBak "${LATEST_BAK_DIR}/plugins/FilePlugin/conf" "${INSTALL_PATH}/Plugins/FilePlugin/"

        RestoringPluginBak "${LATEST_BAK_DIR}/plugins/FusionComputePlugin/vbstool" "${INSTALL_PATH}/Plugins/FusionComputePlugin/bin/"
        RestoringPluginBak "${LATEST_BAK_DIR}/plugins/FusionComputePlugin/tmp" "${INSTALL_PATH}/Plugins/FusionComputePlugin/"

        RestoringPluginBak "${LATEST_BAK_DIR}/plugins/VirtualizationPlugin/vbstool" "${INSTALL_PATH}/Plugins/VirtualizationPlugin/"
        RestoringPluginBak "${LATEST_BAK_DIR}/plugins/VirtualizationPlugin/cert" "${INSTALL_PATH}/Plugins/VirtualizationPlugin/"
    fi
}

MergeConfFileByAgentcli()
{
    PLUGIN_NAME=$1
    OLD_FILE_PATH=${AGENT_BACKUP_PATH}/Plugins/${PLUGIN_NAME}/conf/hcpconf.ini
    NEW_FILE_PATH=${INSTALL_PATH}/Plugins/${PLUGIN_NAME}/conf/hcpconf.ini
    if [ ! -f "$OLD_FILE_PATH" ]; then
        Log "The old config file:${OLD_FILE_PATH} not exist."
        return
    fi
    if [ ! -f "$NEW_FILE_PATH" ]; then
        Log "The new config file:${NEW_FILE_PATH} not exist."
        return
    fi

    if [ "GeneralDBPlugin" = "$PLUGIN_NAME" ]; then
        chmod 555 ${AGENT_BACKUP_PATH}/Plugins/${PLUGIN_NAME}/conf
        chmod 555 ${INSTALL_PATH}/Plugins/${PLUGIN_NAME}/conf
    fi

    sed -i 's/\r//g' "${OLD_FILE_PATH}"
    sed -i 's/\r//g' "${NEW_FILE_PATH}"

    SUExecCmd "${AGENT_ROOT_PATH}/bin/agentcli mergefile ${OLD_FILE_PATH} ${NEW_FILE_PATH}"
    MERGE_RES_FILE_PATH=${AGENT_ROOT_PATH}/tmp/merging_res.ini
    if [ ! -f "${MERGE_RES_FILE_PATH}" ]; then
        LogError "Merging file failed, the res file ${MERGE_RES_FILE_PATH} is not exist!"
        return
    fi

    Log "Merge the ${OLD_FILE_PATH} to ${NEW_FILE_PATH}"
    CP -f ${MERGE_RES_FILE_PATH} ${NEW_FILE_PATH};
    chown root:${AGENT_GROUP} ${NEW_FILE_PATH}
    chmod 640 ${NEW_FILE_PATH}
    rm -f ${MERGE_RES_FILE_PATH}

    if [ "GeneralDBPlugin" = "$PLUGIN_NAME" ]; then
        chmod 500 ${AGENT_BACKUP_PATH}/Plugins/${PLUGIN_NAME}/conf
        chmod 500 ${INSTALL_PATH}/Plugins/${PLUGIN_NAME}/conf
    fi
}

UpgradeJsonConfByAgentcli()
{
    PLUGIN_NAME=$1
    Log "Upgrade the ${PLUGIN_NAME} conf file."
    file=$(basename ./${PLUGIN_NAME}/plugin_attribute_*.json)

    OLD_PLUGIN_FILE=${AGENT_BACKUP_PATH}/Plugins/${PLUGIN_NAME}/${file}
    NEW_PLUGIN_FILE=${PLUGIN_DIR}/${PLUGIN_NAME}/${file}

    chmod 660 ${NEW_PLUGIN_FILE}
    SUExecCmd "${AGENT_ROOT_PATH}/bin/agentcli upgradeJsonConf ${OLD_PLUGIN_FILE} ${NEW_PLUGIN_FILE}"
    chmod 440 ${NEW_PLUGIN_FILE}
}

UpgradeConfByAgentcli()
{
    if [ "${UPGRADE_FLAG_TEMPORARY}" != "1" ]; then
        return
    fi
    Log "Upgrade json conf by agentcli start."
    tmpPath=`pwd`
    cd "${AGENT_BACKUP_PATH}/Plugins"
    oldPluginNames=`ls -l . | grep -E "Plugin|Block_Service" | ${MYAWK} '/^d/ {print $NF}'`
    for pluginName in $oldPluginNames; do
        oldPluginExistNow=`ls -l ${PLUGIN_DIR} | grep ${pluginName} | ${MYAWK} '/^d/ {print $NF}'`
        if [ -z "${oldPluginExistNow}" ];then
            Log "${pluginName} not used now."
            continue
        fi
        UpgradeJsonConfByAgentcli ${pluginName}
        MergeConfFileByAgentcli ${pluginName}
    done
    cd ${tmpPath}
    Log "Upgrade: The backup DataBackup ProtectAgent Plugin conf has been replaced."
    echo "The backup DataBackup ProtectAgent Plugin conf has been upgraded successfully."
    
}

ConfigPluginCpuLimit()
{
    Log "Start config plugin cpu_limit."
    if [ "${SYS_NAME}" = "Linux" ]; then
        cpu_limit=400
        cpu_num=`cat /proc/cpuinfo |grep processor|wc -l`
        if [ $? -eq 0 ] && [ ${cpu_num} -ne 0 ]; then
            Log "Start calc cpu limit,cpu_num: ${cpu_num}."
            tmp_limit=`expr ${cpu_num} \* 20`
            if [ ${tmp_limit} -le ${cpu_limit} ]; then
                cpu_limit=${tmp_limit}
            fi
        fi
        Log "cpu_num=${cpu_num}, cpu_limit=${cpu_limit}"
        index=1
        while [ 1 ]
        do
            plugin=`echo "${CPU_LIMIT_PLUGIN}" | ${AWK} -v i="${index}" '{print $i}'`
            if [ "${plugin}" = "" ]; then
                break
            fi
            index=`expr $index + 1`
            Log "Start config plugin(${plugin}) cpu_limit."
            if [ ! -d "${PLUGIN_DIR}/${plugin}" ]; then
                Log "The plugin[${plugin}] is not installed."
                continue
            fi
            config_files=`ls "${PLUGIN_DIR}/${plugin}" |grep plugin_attribute`
            for config_file in ${config_files}; do
                tmp_result=`cat "${PLUGIN_DIR}/${plugin}/${config_file}"| grep \"cpu_limit\"`
                if [ -z "${tmp_result}" ]; then
                    Log "Add config."
                    sed -i "1 a\  \"cpu_limit\":400," "${PLUGIN_DIR}/${plugin}/${config_file}" 
                else
                    Log "Change config."
                fi
                sed -i "/cpu_limit/s/.*/    \"cpu_limit\":${cpu_limit},/g" "${PLUGIN_DIR}/${plugin}/${config_file}"
            done
        done
    fi
    Log "Start config plugin cpu_limit succ."
}

ConfigPluginMemLimit()
{
    Log "Start config plugin memory_limit."
    if [ "${SYS_NAME}" = "Linux" ]; then
        memory_limit=4096 # 默认值4G
        memory_limit_file_plugin=16384 # 文件集默认值16G
        memory_size=`free -m | grep Mem | ${AWK} '{print $2}'`
        if [ $? -eq 0 ] && [ ${memory_size} -ne 0 ]; then
            Log "Start calc memory limit,memory_size: ${memory_size}."
            tmp_limit=`expr ${memory_size} / 5`
            if [ ${tmp_limit} -le ${memory_limit} ]; then
                memory_limit=${tmp_limit}
            fi
            if [ ${tmp_limit} -le ${memory_limit_file_plugin} ]; then
                memory_limit_file_plugin=${tmp_limit}
            fi
        fi
        Log "Max_mem_size=${memory_size}M, memory_limit=${memory_limit}M, memory_limit_file_plugin=${memory_limit_file_plugin}."
        index=1
        while [ 1 ]
        do
            plugin=`echo "${MEM_LIMIT_PLUGIN}" | ${AWK} -v i="${index}" '{print $i}'`
            if [ "${plugin}" = "" ]; then
                break
            fi
            index=`expr $index + 1`
            Log "Start config plugin(${plugin}) memory_limit."
            if [ ! -d "${PLUGIN_DIR}/${plugin}" ]; then
                Log "The plugin[${plugin}] is not installed."
                continue
            fi
            config_files=`ls "${PLUGIN_DIR}/${plugin}" |grep plugin_attribute`
            for config_file in ${config_files}; do
                tmp_result=`cat "${PLUGIN_DIR}/${plugin}/${config_file}"| grep \"memory_limit\"`
                if [ -z "${tmp_result}" ]; then
                    Log "Add config."
                    if [ ${plugin} = "FilePlugin" ]; then
                        sed -i "1 a\  \"memory_limit\":16384," "${PLUGIN_DIR}/${plugin}/${config_file}"
                    else
                        sed -i "1 a\  \"memory_limit\":4096," "${PLUGIN_DIR}/${plugin}/${config_file}"
                    fi
                else
                    Log "Change config."
                fi
                if [ ${plugin} = "FilePlugin" ]; then
                    sed -i "/memory_limit/s/.*/    \"memory_limit\":${memory_limit_file_plugin},/g" "${PLUGIN_DIR}/${plugin}/${config_file}"
                else
                    sed -i "/memory_limit/s/.*/    \"memory_limit\":${memory_limit},/g" "${PLUGIN_DIR}/${plugin}/${config_file}"
                fi
            done
        done
    fi
    Log "Start config plugin memory_limit succ."
}

GetPluginDir()
{
    for i in 0 1 2 3
    do
        PLUGIN_LIST=`ls . | grep ".tar*" | grep -v "cppframework"`
        if [ -n "${PLUGIN_LIST}" ]; then
            echo "The plugins need to be installed: ${PLUGIN_LIST}."
            Log "The plugins need to be installed: ${PLUGIN_LIST}."
            return 0
        else
            if [ ${i} = "3" ]; then
                echo "Get plugin list failed, please check package!"
                Log "Get plugin list failed, please check package!"
                return 1
            fi
            echo "The plugin list is empty, retry!"
            Log "The plugin list is empty, retry!"
        fi
        sleep 1
    done
    return 1
}

InstallPlugins()
{
    if [ "${TESTCFG_BACK_ROLE}" = "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ]; then
        return 0   # SanClient不需要安装Plugins
    fi

    if [ ! -d "${PLUGIN_DIR}" ]; then
        Log "Plugins path is not exist."
        return 0
    fi
    Log  "InstallPlugins..."
    #1. Get plugins
    tmpPath=`pwd`
    cd ${PLUGIN_DIR}
    GetPluginDir
    #2. install plugins
    for plugin in ${PLUGIN_LIST}; do
        # 2.1 prepare plugin
    {
        tmpPluginPath=`pwd`

        if [ "${plugin}" = "Block_Service_aarch64.tar.xz" ]; then
            pluginName="Block_Service"
        elif [ "${BACKUP_SCENE}" = "${BACKUP_SCENE_EXTERNAL}" ] && [ "${SYS_NAME}" = "Linux" ]; then
            # linux中Block_Service中包含_
            pluginName=`echo ${plugin} |${MYAWK} -F '.' '{print $1}'`
        else
            # AIX、Solaris中FilePlugin_ppc中用_隔开
            pluginName=`echo ${plugin} |${MYAWK} -F "[_.]" '{print $1}'`
        fi
        mkdir -p ${pluginName}
        SUExecCmd "mkdir -p ${AGENT_ROOT_PATH}/log/Plugins/${pluginName}"
        SUExecCmd "chmod -R 700 ${AGENT_ROOT_PATH}/log/Plugins/${pluginName}"
        SUExecCmd "chown -R ${AGENT_USER}:${AGENT_GROUP} ${AGENT_ROOT_PATH}/log/Plugins/${pluginName}"
        mkdir -p ${AGENT_ROOT_PATH}/slog/Plugins/${pluginName}
        CHMOD 700  ${AGENT_ROOT_PATH}/slog/Plugins/${pluginName}

        mv "${PLUGIN_DIR}/${plugin}" "${PLUGIN_DIR}/${pluginName}"
        cd "${PLUGIN_DIR}/${pluginName}"
        pluginFile=${plugin}
        if [ "xz" = `echo ${plugin} | ${AWK} -F '.' '{print $NF}'` ]; then
            if [ "${SYS_NAME}" = "AIX" ]; then
                ${INSTALL_PACKAGE_PATH}/third_party_software/XZ/xz -d $plugin
            else
                xz -d $plugin
            fi
            if [ $? -ne 0 ]; then
                echo "${INSTALL_PACKAGE_PATH}/third_party_software/XZ/xz $pluginName fail." >> $AGENT_ROOT_PATH/stmp/InstallPlugins.info
                LogError "Install $pluginName fail." $ERR_INSTALL_PLUGINS
                exit $ERR_INSTALL_PLUGIN_RETCODE
            fi
            pluginFile=${plugin%%.xz}
        fi
        if [ "${SYS_NAME}" = "SunOS" ]; then
            gzip -d $pluginFile
            pluginFile=`ls | grep ".tar"`
        fi
        tar -xf "${PLUGIN_DIR}/${pluginName}/${pluginFile}"
        if [ $? -ne 0 ]; then
            echo "tar $pluginName fail." >> $AGENT_ROOT_PATH/stmp/InstallPlugins.info
            LogError "Install $pluginName fail." $ERR_INSTALL_PLUGINS
            exit $ERR_INSTALL_PLUGIN_RETCODE
        fi
        rm -rf ${PLUGIN_DIR}/${pluginName}/${pluginFile}
        chmod 755 "${PLUGIN_DIR}"/"${pluginName}"/*.sh

        # 2.2 install plugin
        ./install.sh >> ${LOG_FILE_NAME} 2>&1
        RET_CODE=$?
        if [ "${RET_CODE}" = "${ERR_NO_SUPPORT_PLUGIN}" ]; then
            ShowWarning "Warning: The plug-in [${pluginName}] cannot be installed. If the plug-in needs to be supported, please resolve the problem and reinstall the ProtectAgent."
            Log "Warning: The plug-in [${pluginName}] cannot be installed. If the plug-in needs to be supported, please resolve the problem and reinstall the ProtectAgent."
            echo "${pluginName}" >> ${AGENT_ROOT_PATH}/tmp/unssport_plugins.info
            rm -f "${PLUGIN_DIR}"/"$plugin"
            rm -rf "${PLUGIN_DIR}"/"${pluginName}"
            cd ${tmpPluginPath}
        else
            if [ ${RET_CODE} -ne 0 ]; then
                Log "Install $pluginName fail."
                echo "Install $pluginName fail."
                rm -rf ${PLUGIN_DIR}/cppframework*
                echo "Install $pluginName fail." >> $AGENT_ROOT_PATH/stmp/InstallPlugins.info
                LogError "Install $pluginName fail." $ERR_INSTALL_PLUGINS
                exit $ERR_INSTALL_PLUGIN_RETCODE
            fi
            Log "Install $pluginName succ."
            echo "Install $pluginName succ."
            echo "${pluginName}" >> ${AGENT_ROOT_PATH}/tmp/installed_plugins.info
            cd ${tmpPluginPath}
            if [ -f "$plugin" ]; then
                rm -rf "$plugin"
            fi
        fi
    }
    done
    wait

    SavePluginsLogs

    if [ -f "$AGENT_ROOT_PATH/stmp/InstallPlugins.info" ]; then
        InsPlureasult=`cat $AGENT_ROOT_PATH/stmp/InstallPlugins.info | grep fail`
        if [ "$InsPlureasult" != "" ];then
            echo "Install plugins failed."
            LogError "Install plugins failed." $ERR_INSTALL_PLUGINS
            exit $ERR_INSTALL_PLUGIN_RETCODE
        fi
    fi

    if [ -f "${AGENT_ROOT_PATH}/tmp/unssport_plugins.info" ]; then
        chmod o+r "${AGENT_ROOT_PATH}/tmp/unssport_plugins.info"
    fi

    # 通用备份代理至少要安装一个插件
    CheckPluginsEmpty

    ConfigPluginCpuLimit

    ConfigPluginMemLimit

    rm -rf "$AGENT_ROOT_PATH"/stmp/InstallPlugins.info
    rm -rf ${PLUGIN_DIR}/cppframework*
 
    cd $tmpPath
    CHMOD 550 ${PLUGIN_DIR}
    SUExecCmd "mkdir ${AGENT_ROOT_PATH}/conf/PluginPid"
    chown -h root:${AGENT_GROUP} ${PLUGIN_DIR}
    CHMOD -R 700 ${AGENT_ROOT_PATH}/slog/Plugins/
    SUExecCmd "chmod 700 ${AGENT_ROOT_PATH}/conf/PluginPid"
    # 2. set host ssl domain name
    if [ "${BACKUP_SCENE}" = "${BACKUP_SCENE_EXTERNAL}" ]; then
        DomainName=`${AGENT_ROOT_PATH}/bin/openssl x509 -subject -in ${AGENT_ROOT_PATH}/nginx/conf/server.pem -noout | ${AWK}  -F '=' '{print $NF}'`
        if [ -z "${DomainName}" ];then
            echo "Get ssl domain failed."
            exit 1
        fi
        tmpDomainName=`cat /etc/hosts | grep ${DomainName}`
        if [ -z "${tmpDomainName}" ]; then
            echo "" >> "/etc/hosts"
            echo "127.0.0.1 ${DomainName}" >> "/etc/hosts"
        fi
    elif [ "${BACKUP_SCENE}" = "${BACKUP_SCENE_INTERNAL}" ]; then
        chown -h root:${DEFAULT_GROUP_INTERNAL} ${PLUGIN_DIR}
    fi

    RecordPluginsSelfFiles
    if [ "$UPGRADE_FLAG_TEMPORARY" = "1" ]; then
        RestoringPluginsBak
    fi
    ShowInfo "The plugins are successfully installed."
}

# 升级时，保留旧的插件日志
SavePluginsLogs()
{
    if [ "${UPGRADE_FLAG_TEMPORARY}" != "1" ]; then
        return 0
    fi
    Log "Start to save old plugin logs."
    BACKUP_PLUGINS_LOG_PATH=${DATA_BACKUP_AGENT_HOME}/AgentUpgrade/ProtectClient-E/slog/Plugins
    CURRENT_PLUGINS_LOG_PATH=${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient/ProtectClient-E/slog/Plugins

    if [ ! -d ${CURRENT_PLUGINS_LOG_PATH} ]; then
        Log "The path ${CURRENT_PLUGINS_LOG_PATH} not exists."
        return 0
    fi
    if [ ! -d ${BACKUP_PLUGINS_LOG_PATH} ]; then
        Log "The path ${BACKUP_PLUGINS_LOG_PATH} not exists."
        return 0
    fi
    # 不覆盖复制，新日志目录中保留旧的业务日志，以及最新安装日志
    CP -rf -n -p ${BACKUP_PLUGINS_LOG_PATH}/* ${CURRENT_PLUGINS_LOG_PATH}
}

CheckPluginsEmpty()
{
    if [ ! -d "${PLUGIN_DIR}" ]; then
        return 0
    fi

    if [ ! -f "${AGENT_ROOT_PATH}/tmp/installed_plugins.info" ]; then
        Log "There is no installed plugin."
        rm -f "${AGENT_ROOT_PATH}/tmp/installed_plugins.info"
        exit $ERR_INSTALL_PLUGIN_RETCODE
    fi
}

ConfigurePluginList()
{
    if [ "${SYS_NAME}" = "Linux" ]; then
        if [ "${TESTCFG_BACK_ROLE}" = "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ]; then
            SUExecCmd "sed -i '/<PluginList>/a\        <Plugin name=\"libappprotect\" version=\"1.9.0\" service=\"tasks\" lazyload=\"0\">        </Plugin>' ${AGENT_ROOT_PATH}/conf/pluginmgr.xml"
        elif [ "${BACKUP_ROLE}" = "${BACKUP_ROLE_VMWARE}" ]; then
            # add vmware plugin
            SUExecCmd "sed -i '/<PluginList>/a\        <Plugin name=\"libvmwarenative\" version=\"1.9.0\" service=\"vmwarenative\" lazyload=\"0\">        </Plugin>' ${AGENT_ROOT_PATH}/conf/pluginmgr.xml"
        elif [ "${BACKUP_ROLE}" = "${BACKUP_ROLE_GENERAL_PLUGIN}" ]; then
            # add appprotect plugin and dws plugin
            SUExecCmd "sed -i '/<PluginList>/a\        <Plugin name=\"libappprotect\" version=\"1.9.0\" service=\"tasks\" lazyload=\"0\">        </Plugin>' ${AGENT_ROOT_PATH}/conf/pluginmgr.xml"
            if [ ${BACKUP_SCENE} -ne ${BACKUP_SCENE_INTERNAL} ];then
                SUExecCmd "sed -i '/<PluginList>/a\        <Plugin name=\"libdws\" version=\"1.9.0\" service=\"dws\" lazyload=\"0\">        </Plugin>' ${AGENT_ROOT_PATH}/conf/pluginmgr.xml"
            fi
        else
            SUExecCmd "sed -i '/<PluginList>/a\        <Plugin name=\"liboraclenative\" version=\"1.9.0\" service=\"oraclenative\" lazyload=\"0\">        </Plugin>' ${AGENT_ROOT_PATH}/conf/pluginmgr.xml"
            SUExecCmd "sed -i '/<PluginList>/a\        <Plugin name=\"liboracle\" version=\"1.9.0\" service=\"oracle\" lazyload=\"0\">        </Plugin>' ${AGENT_ROOT_PATH}/conf/pluginmgr.xml"
        fi
    elif [ "${SYS_NAME}" = "AIX" ] || [ "${SYS_NAME}" = "SunOS" ]; then
        SUExecCmd "${AWK} '1;/<PluginList>/{print \"        <Plugin name=\\\"libappprotect\\\" version=\\\"1.9.0\\\" service=\\\"tasks\\\" lazyload=\\\"0\\\">        </Plugin>\"}' ${AGENT_ROOT_PATH}/conf/pluginmgr.xml > ${AGENT_ROOT_PATH}/conf/pluginmgr.xml.backup"
        SUExecCmd "mv ${AGENT_ROOT_PATH}/conf/pluginmgr.xml.backup ${AGENT_ROOT_PATH}/conf/pluginmgr.xml"
        if [ "${SYS_NAME}" = "AIX" ]; then
            SUExecCmd "${AWK} '1;/<PluginList>/{print \"        <Plugin name=\\\"libdws\\\" version=\\\"1.9.0\\\" service=\\\"dws\\\" lazyload=\\\"0\\\">        </Plugin>\"}' ${AGENT_ROOT_PATH}/conf/pluginmgr.xml > ${AGENT_ROOT_PATH}/conf/pluginmgr.xml.backup"
            SUExecCmd "mv ${AGENT_ROOT_PATH}/conf/pluginmgr.xml.backup ${AGENT_ROOT_PATH}/conf/pluginmgr.xml"
        fi
    fi
}

# built-in agent and microservice authentication
ContainerAuth()
{
    if [ ${BACKUP_SCENE} -ne ${BACKUP_SCENE_INTERNAL} ]; then
        return
    fi
 
    python3 -m py_compile ${AGENT_ROOT_PATH}/bin/decrypt.py
    python3 -m py_compile ${AGENT_ROOT_PATH}/bin/getpmip.py
    python3 -m py_compile ${AGENT_ROOT_PATH}/bin/nodeinfo.py
    python3 -m py_compile ${AGENT_ROOT_PATH}/bin/update_cluster.py
    python3 -m py_compile ${AGENT_ROOT_PATH}/bin/kmc_util.py
    python3 -m py_compile ${AGENT_ROOT_PATH}/bin/get_net_plane_ip.py
    cp -rf ${AGENT_ROOT_PATH}/bin/__pycache__/decrypt.*.pyc ${AGENT_ROOT_PATH}/bin/decrypt.pyc
    cp -rf ${AGENT_ROOT_PATH}/bin/__pycache__/getpmip.*.pyc ${AGENT_ROOT_PATH}/bin/getpmip.pyc
    cp -rf ${AGENT_ROOT_PATH}/bin/__pycache__/nodeinfo.*.pyc ${AGENT_ROOT_PATH}/bin/nodeinfo.pyc
    cp -rf ${AGENT_ROOT_PATH}/bin/__pycache__/kmc_util.*.pyc ${AGENT_ROOT_PATH}/bin/kmc_util.pyc
    cp -rf ${AGENT_ROOT_PATH}/bin/__pycache__/get_net_plane_ip.*.pyc ${AGENT_ROOT_PATH}/bin/get_net_plane_ip.pyc
    cp -rf ${AGENT_ROOT_PATH}/bin/__pycache__/update_cluster.*.pyc ${AGENT_ROOT_PATH}/bin/update_cluster.pyc
    rm -rf ${AGENT_ROOT_PATH}/bin/__pycache__
    rm -rf ${AGENT_ROOT_PATH}/bin/*.py
    chown ${AGENT_USER}:${AGENT_GROUP} ${AGENT_ROOT_PATH}/bin/*.pyc
    chmod 500 ${AGENT_ROOT_PATH}/bin/*.pyc
}

WriteUpgradeParam()
{
    echo "PM_IP=${PM_IP_LIST}" | tr -d '\r' > ${INSTALL_PATH}/ProtectClient-E/stmp/ServerInfo.tmp
    echo "PM_PORT=${PM_PORT}" | tr -d '\r' >> ${INSTALL_PATH}/ProtectClient-E/stmp/ServerInfo.tmp

    # change upgrade config privilege
    CHMOD 600 "${AGENT_ROOT_PATH}/stmp/ServerInfo.tmp"
}

CheckUnssportPlugins()
{
    if [ -f ${AGENT_ROOT_PATH}/tmp/unssport_plugins.info ]; then
        rm -f "${AGENT_ROOT_PATH}/tmp/unssport_plugins.info"
    fi

    if [ -f ${AGENT_ROOT_PATH}/tmp/unssport_apps.info ]; then
        chmod o+r "${AGENT_ROOT_PATH}/tmp/unssport_apps.info"
        if [ -s "${AGENT_ROOT_PATH}/tmp/unssport_apps.info" ]; then  # 判断文件内容是否为空
            ShowWarning "Warning: The following applications will not be available: "
            cat ${AGENT_ROOT_PATH}/tmp/unssport_apps.info
            if [ "${INSTALLATION_MODE}" = "push" ]; then
                cp -p -f ${AGENT_ROOT_PATH}/tmp/unssport_apps.info ${REGIST_DIR}
            fi
            rm -f "${AGENT_ROOT_PATH}/tmp/unssport_apps.info"
            exitCode=${WARING_UNSSPORT_APPS}
        fi
    fi

    if [ "${UPGRADE_FLAG_TEMPORARY}" = "1" ]; then
        exitCode=0
    fi
}
########################################################################################
# Main Process
########################################################################################

PreCheck

GetConfigInfo

WriteUpgradeParam

# Creating an installation information file
CreateInstallInfoFile

# check execute user
CheckExecUser
if [ $? -ne 0 ]; then
    echo "The login/execute user must be user root. However, the actual login user is ${LOG_USER}."
    echo "The installation of DataBackup ProtectAgent will be stopped."
    Log "The login/execute user must be user root. However, the actual login user is ${LOG_USER}."
    LogError "The installation of DataBackup ProtectAgent will be stopped." ${ERR_UPGRADE_FAIL_VERIFICATE_PARAMETER}
    exit 1
fi

# Ip connectivity check
AdaptListenIp

# Creating the agent uuid folder
AgentUniqueID

# add plugin lists
ConfigurePluginList

# Configuring Host Information
ConfigHostInfo

# Certificate two-way authentication
Secure_Authentication=`SUExecCmdWithOutput "${AGENT_ROOT_PATH}/bin/xmlcfg read System secure_channel"`
if [ "${Secure_Authentication}" = "1" ]; then
    EnableSSLCertVerify
fi

# Check whether the process exists.
Log "Check proccess of DataBackup ProtectAgent."
CheckAgentExist rdagent
CheckAgentExist rdnginx
CheckAgentExist monitor

# set register type.
SetRegisterType

# set host type
SetHostType

ReadAndVerifyPasswd

#install fileclient
InstallFileClient
if [ $? -ne 0 ]; then
    exit ${ERR_FILECLIENT_INSTALL_FAILED}
fi

# handle install dataturbo
HandleInstallDataTurbo

# install external plugin
InstallPlugins

# Set the user name and password.
Log "Set env of DataBackup ProtectAgent."
SetUserNameAndPasswd

# Obtaining the NIC Type
Log "Check IP4 or IPV6 of DataBackup ProtectAgent."
GetNetworkType

# Choose is EIP or not
ChooseEIP

CheckIsEip ${AGENT_IP}
if [ $? -eq 0 ]; then    
    echo EIP=${AGENT_IP} | tr -d '\r' >> ${INSTALL_PATH}/ProtectClient-E/conf/testcfg.tmp
    SUExecCmd "${AGENT_ROOT_PATH}/bin/xmlcfg write Mount eip ${AGENT_IP}"
    Log "set eip:${AGENT_IP}."
    AGENT_IP=
fi

# Setting the Listening IP Address
Log "The nginx listening ip address is to be set."
SetIp
if [ "$?" != "0" ]; then
    echo "Failed to obtain the network card, the installation process stops."
    LogError "The local IP address that can connect to the ProtectManager is not found. error exit." ${ERR_NIC_SET_FAILED}
    exit $ERR_IPADDR_SET_FAILED_RETCODE
fi

# Setting the Listening Port
Log "Set port of DataBackup ProtectAgent."
SetPort rdagent
SetPort nginx

# Write network adapter information.
WriteNetworkInfo

# set userid
SetUserId

# input PM Ip
SetPMIp

# set PM port
SetPMPort

# Setting Automatic Startup
Log "Set the ProtectAgent service to automatically start upon system startup."
echo "Set the ProtectAgent service to automatically start upon system startup."
SetAutoStart

# Configuring the run process dependence
ConfigRunDependency

# switch off report heatbeat to pm
SwitchOffReportHeartbeatToPm

# upgrade plugin confs for plugins
UpgradeConfByAgentcli

# Start the agent process.
StartAgent

# VMware Host Enable the firewall port(59526).
# Oracle Host Check the activity of the firewall.
if [ ${BACKUP_ROLE} -eq ${BACKUP_ROLE_VMWARE} ]; then 
    if [ -f "/etc/redhat-release" ]; then
        SYS_VERSION=`cat /etc/redhat-release | ${AWK} -F '(' '{print $1}' | ${AWK} '{print $NF}'`
        if [ `expr $SYS_VERSION \< 7` -eq 1 ]; then
            AddPortFilter_rh6 59526
        else
            AddPortFilter 59526
        fi
    elif [ -f "/etc/SuSE-release" ]; then
        AddPortFilter_SUSE 59526
    else
        AddPortFilter 59526
    fi
else
    CheckFirewallActivity
fi

# register to PM
RegisterHost

# switch on report heatbeat to pm
SwitchOnReportHeartbeatToPm

# microservice authentication
ContainerAuth

#判断是否存在不支持的插件
exitCode=0
CheckUnssportPlugins

Log "DataBackup ProtectAgent is successfully installed on the host."

if [ "${UPGRADE_FLAG_TEMPORARY}" = "1" ] && [ -f "${LATEST_BAK_DIR}/slog/agent_install.log" ]; then
    cat "${LATEST_BAK_DIR}/slog/agent_install.log" > ${LOG_FILE_NAME}.tmp
    cat "${LOG_FILE_NAME}" >> ${LOG_FILE_NAME}.tmp
    mv -f ${LOG_FILE_NAME}.tmp ${LOG_FILE_NAME}
fi

exit ${exitCode}

