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

######################################################################
# The function of this script is to install the ProtectAgent client.
######################################################################
PRODUCT_NAME="DataBackup ProtectAgent"
AGENT_USER=rdadmin
AGENT_GROUP=rdadmin
SANCLIENT_USER=sanclient
SANCLIENT_GROUP=sanclient
SYS_NAME=`uname -s`
SYS_ARCH=""
SYS_BIT=""
SYS_LEVEL=""
SEMICOLON=":"
DOT="."
IS_MANUAL=0
IS_REINSTALL=0
LOGFILE_SUFFIX="gz"
CURRENT_PATH=`dirname $0` && cd $CURRENT_PATH && CURRENT_PATH=`pwd`
INSTALL_PACKAGE_PATH=${CURRENT_PATH}
export INSTALL_PACKAGE_PATH
SHELL_TYPE=
LOG_LEVEL=""
IS_AUTO_SYNCHRONIZE_HOST_NAME="false"
ENABLE_HTTP_PROXY="0"
DISABLE_SAFE_RANDOM_NUMBER="0"

IS_DPC_COMPUTE_NODE=""
DPC_STORAGE_FRONT_IP=""
DPC_STORAGE_FRONT_GATEWAY=""

# ASAN_OPTIONS
ASAN_OPTIONS=0
# TSAN_OPTIONS
TSAN_OPTIONS=0

###### SET UMASK ######
umask 0022

##### The lower level may need #####
CERT_PATH=""
CERT_PATH_EXIST="nul"
PKG_CONF_PATH=${CURRENT_PATH}/conf
USER_ID=""
INSTALLATION_MODE=""
INSTALL_FAIL_LOG_DIR="/var/log/ProtectAgent"
SANCLIENT_INSTALL_FAIL_LOG_DIR="/var/log/ProtectSanClient"
AGENT_PUSH_INSTALL_PATH="/opt/register"
CURRENT_TIME=`date "+%Y-%m-%d_%H-%M-%S"`
SANCLIENT_UPGRADE_BACKUP_PATH="/opt/SanClientUpgrade"
BACKUP_MAX_NUM=4
SANCLIENT_SUPPORT_VERSION="12-SP4"

ARR_OF_LOG="agentcli.log agent_start.log agent_stop.log agent_upgrade_sqlite.log"
ARR_OF_SLOG="agent_firewall_tool.log agent_install.log agent_upgrade.log gethostos.log install.log mountnasfilesystem.log upgrade.log"

##### Initial value for upgrade #####
UPGRADE_INITIAL_VALUE=9
MODIFY_INITIAL_VALUE=9

##### install errcode start #####
ERR_COMMAND_NOT_FOUND=1577210129
ERR_WORKINGUSER_ADD_FAILED=1577209885
ERR_WORKINGUSER_EXISTS=1577209885
ERR_ENV_SET_FAILED=42
ERR_AGENT_PATH_IS_EXIST=1577210135
ERR_USER_EXIST=20
###### install errcode end ######

ERR_COMMAND_NOT_FOUND_RETCODE=70

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
ERR_MULTIPLE_AGENT_TYPE_NOT_SUPPORT=1577265921
ERROR_OS_NOT_SUPPORT=1577266689
##### upgrade errcode end #####

###### backup application type ######
BACKUP_ROLE=-1
BACKUP_ROLE_HOST=0
BACKUP_ROLE_VMWARE=2
BACKUP_ROLE_GENERAL_PLUGIN=4
BACKUP_ROLE_SANCLIENT_PLUGIN=5
PLUGIN_DIR=

###### BACKUP_SCENE ######
BACKUP_SCENE=-1
BACKUP_SCENE_EXTERNAL=0
BACKUP_SCENE_INTERNAL=1

###### CERTIFICATE_REVOCATION ######
CERTIFICATE_REVOCATION=0
AGENT_IP=
PRIVATE_IP=
FLOATING_IP=


###### Custom installation directory ######
DEFAULT_INSTALL_PATH=/opt
CUSTOM_INSTLL_PATH=
CUSTOM_INSTLL_PATH_INVALID=1
CUSTOM_INSTLL_PATH_NOT_EXIST=2
CUSTOM_INSTLL_PATH_PERMISSION_HIGH=3
CUSTOM_INSTLL_PATH_PERMISSION_LOW=4
CHECK_DIR=/opt
DATA_BACKUP_AGENT_HOME=

BLOCK_LIST="//|^/$|^/tmp$|^/tmp/.*|^/dev$|^/dev/.*|^/bin$|^/bin/.*|^/usr$|/usr/.*"

##################################################################

if [ "$SYS_NAME" = "SunOS" ]; then
    MYAWK=nawk
    AGENT_USER_PATH="/export/home"

    if [ "${UPGRADE_FLAG_TEMPORARY}" = "1" ]; then
        USER=root
        export USER
    fi
else
    MYAWK=awk
    AGENT_USER_PATH="/home"
fi

if [ "`ls -al /bin/sh | ${MYAWK} '{print $NF}'`" = "bash" ]; then
    EXPORT_ENV="if [ -f ~/.profile ]; then source ~/.profile; fi;"
else
    EXPORT_ENV=""
fi

LOG_ERR_INSTALL_FILE=${AGENT_PUSH_INSTALL_PATH}/errormsg.log
LOG_ERR_FILE=${CURRENT_PATH}/errormsg.log
LOG_FILE_NAME=${CURRENT_PATH}/install.log
. ${CURRENT_PATH}/func_util.sh

CLIENT_BACK_ROLE=`cat ${INSTALL_PACKAGE_PATH}/conf/client.conf | grep "backup_role" | ${MYAWK} -F '=' '{print $NF}'`
if [ "${CLIENT_BACK_ROLE}" = "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ]; then
    AGENT_ROOT_PATH=${SANCLIENT_AGENT_ROOT_PATH}
    INSTALL_DIR=${SANCLIENT_INSTALL_DIR}
    AGENT_GROUP=${SANCLIENT_GROUP}
    AGENT_USER=${SANCLIENT_USER}
    INSTALL_FAIL_LOG_DIR=${SANCLIENT_INSTALL_FAIL_LOG_DIR}
    AGENT_PUSH_INSTALL_PATH=${SANCLIENT_PUSH_INSTALL_PATH}
fi

ClearEnv()
{
    backRole="$1"
    envName=
    if [ "$1" = "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ]; then
        envName=DATA_BACKUP_SANCLIENT_HOME
    else
        envName=DATA_BACKUP_AGENT_HOME
    fi

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
}

SwitchLogs()
{
    SWITCH_LOG=AgentSwitchLog_`date +%Y%m%M``date +%s`.tar.gz
    pushd ${INSTALL_FAIL_LOG_DIR}
    tar -zcvf ${SWITCH_LOG} .
    rm -rf ../ProtectAgentSwitchLog && mkdir -p ../ProtectAgentSwitchLog
    mv ${SWITCH_LOG} ../ProtectAgentSwitchLog
    popd
}

ClearResource()
{
    printf "\\033[1;31mFailed to install the ProtectAgent. Clearing installation resources. Please wait...\\033[0m \n"
    #1. back log
    if [ ! -d "${INSTALL_FAIL_LOG_DIR}" ]; then
        mkdir -p ${INSTALL_FAIL_LOG_DIR}
    else
        SwitchLogs
        rm -rf "${INSTALL_FAIL_LOG_DIR}"/*
    fi

    sh ${INSTALL_DIR}/collectlog.sh >/dev/null 2>&1
    if [ $? -ne 0 ]; then
        cp -r "${INSTALL_DIR}"/ProtectClient-E/slog ${INSTALL_FAIL_LOG_DIR}/ 1>/dev/null 2>&1
        cp -r "${INSTALL_DIR}"/ProtectClient-E/log ${INSTALL_FAIL_LOG_DIR}/ 1>/dev/null 2>&1
    else
        SYSINFO_PATH="${INSTALL_DIR}"/ProtectClient-E/stmp/
        cp "${SYSINFO_PATH}"/AGENTLOG* ${INSTALL_FAIL_LOG_DIR}/ 1>/dev/null 2>&1
    fi
    
    if [ "$UPGRADE_FLAG_TEMPORARY" = "1" ]; then
        echo "The upgrade process does not require users and groups to be reset."
        return 0
    fi

    #2. uninstall agent
    if [ -f "${INSTALL_DIR}/uninstall.sh" ]; then
        echo y | sh "${INSTALL_DIR}/uninstall.sh" 1>/dev/null 2>&1
    fi

    IsKylinOS
    if [ $? -eq 0 ]; then
        Log "Kylin system does not require users and groups to be reset."
    else
        #3. delete user
        if id -u ${AGENT_USER} >/dev/null 2>&1; then
            userdel -r ${AGENT_USER} >/dev/null 2>&1
        fi

        #4. delete user dir
        if [ -d "/${AGENT_USER_PATH}/${AGENT_USER}" ]; then
            rm -rf "/${AGENT_USER_PATH}/${AGENT_USER}" >/dev/null 2>&1
        fi

        #5. delete group
        if id -g ${AGENT_GROUP} >/dev/null 2>&1; then
            groupdel ${AGENT_GROUP} >/dev/null 2>&1
        fi 
    fi
    #6. delete dir
    if [ -d "${INSTALL_DIR}" ]; then
        cd ${DEPLOY_TOP_DIR}
        rm -rf "${INSTALL_DIR}" >/dev/null 2>&1
    fi

    ClearEnv ${CLIENT_BACK_ROLE}
    echo "Note: The failure logs are stored in the directory[${INSTALL_FAIL_LOG_DIR}]."
}

ExitHandle()
{
    EXIT_CODE=$1
    Log "Install script will exit. The EXIT_CODE is : ${EXIT_CODE}"
    # 将可能存在的日志移动到安装目录
    if [ -d "${INSTALL_DIR}/ProtectClient-E/slog" ]; then
        mv "${LOG_FILE_NAME}" "${INSTALL_DIR}/ProtectClient-E/slog" 1>/dev/null 2>&1
    elif [ -d "${INSTALL_DIR}/ProtectClient-E" ]; then
        mkdir -p "${INSTALL_DIR}/ProtectClient-E/slog"
        mv "${LOG_FILE_NAME}" "${INSTALL_DIR}/ProtectClient-E/slog" 1>/dev/null 2>&1
    fi

    # 某些插件依赖java环境没有安装成功，需要给出警告的场景
    if [ ${EXIT_CODE} -eq 250 ]; then
        printf "\\033[1;32mDataBackup ProtectAgent is successfully installed on the host.\\033[0m \n"
        Log "DataBackup ProtectAgent is successfully installed on the host."
        [ -f "${BACKUP_COPY_DIR}/slog/install.log" ] && cat "${BACKUP_COPY_DIR}/slog/install.log" >> ${LOG_FILE_NAME}
        exit ${EXIT_CODE}
    fi

    # 安装失败处理
    if [ ${EXIT_CODE} -ne 0 ] && [ ${IS_REINSTALL} -eq 0 ]; then
        ClearResource
    else
        Log "No need clear resource."
    fi

    # 最后安装成功打印消息
    if [ ${EXIT_CODE} -eq 0 ]; then
        printf "\\033[1;32mDataBackup ProtectAgent is successfully installed on the host.\\033[0m \n"
        Log "DataBackup ProtectAgent is successfully installed on the host."
    fi

    [ -f "${BACKUP_COPY_DIR}/slog/install.log" ] && cat "${BACKUP_COPY_DIR}/slog/install.log" >> ${LOG_FILE_NAME}
    exit ${EXIT_CODE}
}

PrintHelp()
{
    printf "\033[31m    Invalid parameter, params:\033[0m
    type: Client Installation Type.
    mode: Push installation or common installation
    eg:
    \033[31m./install.sh -manual\033[0m ----> (Manual Installation)
    \033[31m./install.sh\033[0m ----> (oracle/vmware/plugin backup, Common installation)
    \033[31m./install.sh -mode push "*"\033[0m ----> (oracle/vmware/plugin backup, Push installation)\n"
}

CheckParams()
{
    if [ $# -eq 0 ]; then
        return 0
    fi
    # Manual Installation
    if [ $1 = "-manual" ] && [ $# -eq 1 ]; then
        IS_MANUAL=1
        return 0
    fi

    if [ $1 = "-h" ] && [ $# -eq 1 ]; then
        PrintHelp
        LogError "Parameter error." ${ERR_UPGRADE_FAIL_VERIFICATE_PARAMETER}
        ExitHandle 1
    fi

    if [ $# -eq 1 ]; then
        if [ $1 = "ASAN"  ]; then
            ASAN_OPTIONS=1
            return 0
        fi
        if [ $1 = "TSAN"  ]; then
            TSAN_OPTIONS=1
            return 0
        fi
    fi

    if [ $# -gt 4 ]; then
        printf "\\033[1;31mIncorrect installation parameters. too many input parameters. \\033[0m \n"
        PrintHelp
        LogError "Parameter error." ${ERR_UPGRADE_FAIL_VERIFICATE_PARAMETER}
        ExitHandle 1
    elif [ $# -ne 0 ] && [ $# -ne 2 ] && [ $# -ne 3 ] && [ $# -ne 4 ]; then
        PrintHelp
        LogError "Parameter error." ${ERR_UPGRADE_FAIL_VERIFICATE_PARAMETER}
        ExitHandle 1
    fi

    if [ $# -eq 2 ] && [ $1 = "-mode" ] && [ $2 = "push" ]; then
        INSTALLATION_MODE=$2
    elif [ $# -eq 3 ] && [ $1 = "-mode" ] && [ $2 = "push" ]; then
        INSTALLATION_MODE=$2
        AGENT_IP=$3
        echo "Set push install use agent ip ${AGENT_IP}"
        Log "Set push install use agent ip ${AGENT_IP}"
    else
        PrintHelp
        LogError "Parameter error." ${ERR_UPGRADE_FAIL_VERIFICATE_PARAMETER}
        ExitHandle 1
    fi
}

CheckAgentUser()
{
    IsKylinOS
    if [ $? -eq 0 ]; then
        Log "Kylin system does not require users and groups to be reset. Therefore, it is not necessary to detect whether the user already exists."
        return 0;
    fi

    if [ "$UPGRADE_FLAG_TEMPORARY" = "1" ]; then
        return 0;
    fi

    id ${AGENT_USER} >/dev/null 2>&1
    if [ $? -eq 0 ]; then
        return 1
    fi
    return 0
}

CheckInstallPath()
{
    if [ $# -ne 1 ]; then
        LogError "The customized installation directory invaild." ${CUSTOM_INSTLL_PATH_INVALID}
        return 1
    fi

    inputIstallPath="$1"
    #1. 删除历史结果文件
    if [ -f "${RESULT_FILE}" ]; then
        rm -f ${RESULT_FILE}
    fi

    #2. 校验特殊字符
    CheckCommandInjection "${inputIstallPath}"
    if [ $? -ne 0 ]; then
        echo "The customized installation directory contains special characters, Enter another directory:"
        LogError "The customized installation directory contains special characters." ${CUSTOM_INSTLL_PATH_INVALID}
        return 1
    fi

    #3. realpath校验
    command -v realpath > /dev/null
    if [ $? -eq 0 ]; then
        realInstallPath=`realpath ${inputIstallPath}`
    else
        if [ ! -d "${inputIstallPath}" ]; then
            echo "The directory does not exist. Enter another directory:"
            LogError "The customized installation directory does not exist." ${CUSTOM_INSTLL_PATH_NOT_EXIST}
            return 1
        fi
        realInstallPath=`cd ${inputIstallPath} && pwd`
    fi
    echo ${realInstallPath} | egrep ${BLOCK_LIST} > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        echo "The customized installation directory invaild."
        LogError "The customized installation directory invaild." ${CUSTOM_INSTLL_PATH_INVALID}
        return 1
    fi

    #4.校验路径是否存在
    if [ ! -d "${realInstallPath}" ]; then
        echo "The directory does not exist. Enter another directory:"
        LogError "The customized installation directory does not exist." ${CUSTOM_INSTLL_PATH_NOT_EXIST}
        return 1
    fi

    #5. 校验路径权限
    flagHigh=0

    #5.1. 校验目录和父目录，对其他other用户是否有读和执行权限
    tmpDir=""
    index=2
    while [ 1 ]
    do
        subPath=`echo "${realInstallPath}" | $AWK -F "/" -v i="${index}" '{print $i}'`
        if [ "${subPath}" = "" ]; then
            break
        fi
        index=`expr $index + 1`

        tmpDir="${tmpDir}/${subPath}"
        command -v stat >/dev/null
        if [ $? -ne 0 ]; then
        #stat不可用使用perl
            accessRights=$(perl -e 'my @stat = stat($ARGV[0]); printf "%o\n", $stat[2] & 07777;' $tmpDir)
        else
            accessRights=`stat -c %a ${tmpDir}`
        fi
        tmpAccessRights=7
        if [ "$SYS_NAME" = "SunOS" ]; then
            tmpAccessRights=`echo ${accessRights:2:1}`
        else
            tmpAccessRights=`expr substr "$accessRights" 3 1`
        fi
        if [ -z "${tmpAccessRights}" ] || [ ${tmpAccessRights} -lt 5 ]; then
            echo "The directory ${tmpDir} permission ${accessRights} is too low. Enter a directory with the correct permission again:"
            LogError "Invalid low permission: ${accessRights} path: ${tmpDir}." ${CUSTOM_INSTLL_PATH_PERMISSION_LOW}
            return 1
        fi
    done

    currentDir=${realInstallPath}
    #5.2 校验用户、属组
    command -v stat >/dev/null
    if [ $? -ne 0 ]; then
        #stat不可用使用perl
        userName=$(perl -e 'my @stat = stat($ARGV[0]); my $uid = $stat[4]; my $username = getpwuid($uid); print "$username\n";' $currentDir)
        groupName=$(perl -e 'my @stat = stat($ARGV[0]); my $gid = $stat[5]; my $groupname = getgrgid($gid); print "$groupname\n";' $currentDir)
    else
        userName=`stat -c %U ${currentDir}`    
        groupName=`stat -c %G ${currentDir}`   
    fi
    if [ ${userName} != "root" ]; then
        echo "The directory ${tmpDir} owner is ${userName}:${groupName}, not root. Enter a directory with the correct owner again:"
        LogError "Invalid user: ${userName} or group: ${groupName}." ${CUSTOM_INSTLL_PATH_PERMISSION_LOW}
        return 1
    fi

    #5.3 校验权限
    command -v stat >/dev/null
    if [ $? -ne 0 ]; then
    #stat不可用使用perl
        accessRights=$(perl -e 'my @stat = stat($ARGV[0]); printf "%o\n", $stat[2] & 07777;' $currentDir)
    else
        accessRights=`stat -c %a ${currentDir}`
    fi
    result=$(find "${currentDir}" -type d \( ! -name . -prune \) \( -perm -g=w -o -perm -o=w \))
    if [ -n "$result" ]; then
        echo "Invalid permission: $accessRights."
        flagHigh=1
    fi
    if [ ${flagHigh} -eq 1 ]; then
        echo "The directory permission is too high and may be changed by other users. Are you sure you want to continue the installation? (y/n)"
        read tmpChoice
        if [ "${tmpChoice}" = "y" ]; then
            return 0
        else
            echo "Please enter the agent installation directory:"
            return 1
        fi
    fi

    return 0
}

AdaptingCustomDirectory()
{
    #1.
    
    if [ "${CLIENT_BACK_ROLE}" = "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ]; then
        INSTALL_DIR="${CUSTOM_INSTLL_PATH}/DataBackup/SanClient"
        UPGRADE_BACKUP_PATH=${CUSTOM_INSTLL_PATH}/SanClientUpgrade
        AGENT_GROUP=${SANCLIENT_GROUP}
        AGENT_USER=${SANCLIENT_USER}
        INSTALL_FAIL_LOG_DIR=${SANCLIENT_INSTALL_FAIL_LOG_DIR}
        DATA_BACKUP_SANCLIENT_HOME=${CUSTOM_INSTLL_PATH}
        export DATA_BACKUP_SANCLIENT_HOME 
        InsertEnvInProfile "DATA_BACKUP_SANCLIENT_HOME" ${DATA_BACKUP_SANCLIENT_HOME}
    else
        UPGRADE_BACKUP_PATH=${CUSTOM_INSTLL_PATH}/AgentUpgrade
        INSTALL_DIR="${CUSTOM_INSTLL_PATH}/DataBackup/ProtectClient"
        DATA_BACKUP_AGENT_HOME=${CUSTOM_INSTLL_PATH}
        export DATA_BACKUP_AGENT_HOME
        InsertEnvInProfile "DATA_BACKUP_AGENT_HOME" ${DATA_BACKUP_AGENT_HOME}  
    fi 
    INSTALL_DIR_UPPER="${CUSTOM_INSTLL_PATH}/DataBackup"
    AGENT_ROOT_PATH="${INSTALL_DIR}/ProtectClient-E/"
    AGENT_ROOT=$AGENT_ROOT_PATH
    BACKUP_COPY_DIR="${INSTALL_DIR_UPPER}/Bak/${CURRENT_TIME}"
    CHECK_DIR=${CUSTOM_INSTLL_PATH}
    if [ -z "$LD_LIBRARY_PATH" ]; then
        LD_LIBRARY_PATH=${AGENT_ROOT_PATH}/bin
    else
        LD_LIBRARY_PATH=${AGENT_ROOT_PATH}/bin:$LD_LIBRARY_PATH
    fi
    AGENT_USER_LD_LIBRARY_PATH=${AGENT_ROOT_PATH}/bin
    LIBPATH=${AGENT_ROOT}/bin
    PLUGIN_DIR=${INSTALL_DIR}/Plugins
    DEPLOY_TOP_DIR=${CUSTOM_INSTLL_PATH}
    CHECK_DIR=${CUSTOM_INSTLL_PATH}

    #2.
    export LD_LIBRARY_PATH
    export AGENT_ROOT_PATH
    export LIBPATH
    export AGENT_USER_LD_LIBRARY_PATH
    export INSTALL_DIR
    export INSTALL_DIR_UPPER
}

InsertEnvInProfile()
{
    if [ "$UPGRADE_FLAG_TEMPORARY" = "1" ]; then
        TMP_ENV=`cat /etc/profile |grep $1 |${MYAWK} -F "=" '{print $NF}'`
        if [ -n "${TMP_ENV}" ]; then
            echo "The environment variable already exists."
        else
            echo "export $1=$2" >> /etc/profile
        fi
    else
        TMP_ENV=`cat /etc/profile |grep $1 |${MYAWK} -F "=" '{print $NF}'`
        if [ -n "${TMP_ENV}" ]; then
            echo "The environment variable already exists."
            Log "The environment variable already exists"
            exit 1        ### 之前做了什么操作
        fi
        if [ "$SYS_NAME" = "SunOS" ]; then
            echo "$1=$2" >> /etc/profile
            echo "export $1" >> /etc/profile
        else
            echo "export $1=$2" >> /etc/profile
            if [ ${BACKUP_SCENE} -eq ${BACKUP_SCENE_INTERNAL} ]; then
                echo "export $1=$2" >> /etc//bashrc
            fi
        fi
    fi
}


GetInstallPathFromConf()
{
    tmpConfPath=""
    if [ "${CLIENT_BACK_ROLE}" = "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ]; then
        if [ -f "${INSTALL_PACKAGE_PATH}/conf/client.conf" ]; then
            tmpConfPath=`cat ${INSTALL_PACKAGE_PATH}/conf/client.conf | grep "sanclient_custom_install_path" | ${MYAWK} -F '=' '{print $NF}'`
            if [ -z "${tmpConfPath}" ]; then
                Log "No user-defined path is entered."
                tmpConfPath="/opt"
            fi
        fi
    else
        if [ -f "${INSTALL_PACKAGE_PATH}/conf/client.conf" ]; then
            tmpConfPath=`cat ${INSTALL_PACKAGE_PATH}/conf/client.conf | grep "custom_install_path" | ${MYAWK} -F '=' '{print $NF}'`
            if [ -z "${tmpConfPath}" ]; then
                Log "No user-defined path is entered."
                tmpConfPath="/opt"
            fi
        fi
    fi
    echo "${tmpConfPath}"
}

GetIsSharedChoice()
{
    if [ ${BACKUP_SCENE} -eq ${BACKUP_SCENE_INTERNAL} ]; then
        IS_SHARED=true
        return 0
    fi

    if [ "${INSTALLATION_MODE}" != "push" ] && [ "$UPGRADE_FLAG_TEMPORARY" != "1" ]; then
        shareStutas=`cat ${INSTALL_PACKAGE_PATH}/conf/client.conf | grep "is_shared" | ${MYAWK} -F '=' '{print $NF}'`
        if [ "${shareStutas}" = "true" ]; then
            IS_SHARED=true
        else
            IS_SHARED=false
        fi
    fi
    return 0
}

GetInputInstallPath()
{
    #1.
    if [ "${INSTALLATION_MODE}" = "push" ]; then
        Log "push install."
        CUSTOM_INSTLL_PATH=`GetInstallPathFromConf`
        Log "CUSTOM_INSTLL_PATH: ${CUSTOM_INSTLL_PATH}"
    elif [ ${BACKUP_SCENE} -eq ${BACKUP_SCENE_INTERNAL} ]; then
        Log "internal install."
        CUSTOM_INSTLL_PATH=${DEFAULT_INSTALL_PATH}
    elif [ "$UPGRADE_FLAG_TEMPORARY" = "1" ]; then
        Log "upgrade ${DATA_BACKUP_AGENT_HOME_BAK}."
        CUSTOM_INSTLL_PATH=${DATA_BACKUP_AGENT_HOME_BAK}
    else
        echo "Enter the installation directory of the agent. The agent cannot be installed in a directory such as /, /tmp, /dev, /bin, /usr. Special characters such as [|;&$><\`!]+ are not allowed. By default, the agent is installed in the /opt directory."
        for i in 0 1 2
        do
            #1.1
            echo "Please enter the agent installation directory:"
            read tmpInstallPath

            #1.2
            if [ -z "${tmpInstallPath}" ]; then
                CUSTOM_INSTLL_PATH=${DEFAULT_INSTALL_PATH}
                echo "Use the default installation path /opt."
                Log "Use the default installation path /opt."
                break
            else
                CheckInstallPath $tmpInstallPath
                if [ $? -eq 0 ]; then
                    CUSTOM_INSTLL_PATH=${tmpInstallPath}
                    Log "The customized installation directory is ${CUSTOM_INSTLL_PATH}"
                    break
                fi
            fi
        done
    fi

    #2.
    if [  -z "${CUSTOM_INSTLL_PATH}" ]; then
        echo "The install directory inputs exceeds 3. The installation process exits abnormally."
        return 1
    fi

    #3.
    CUSTOM_INSTLL_PATH=`cd ${CUSTOM_INSTLL_PATH} && pwd`
    Log "The installation path is ${CUSTOM_INSTLL_PATH}."
    AdaptingCustomDirectory
    return 0
}

GetBackupScene()
{
    # 1. Read the configuration file.
    BACKUP_SCENE_CONFIG=`cat ${INSTALL_PACKAGE_PATH}/conf/client.conf | grep "backup_scene"`
    if [ -z "${BACKUP_SCENE_CONFIG}" ]; then
        Log "Failed to read the installation scene."
        BACKUP_SCENE=${BACKUP_SCENE_EXTERNAL}
    else
        Log "Succ to read the installation scene."
        BACKUP_SCENE=`cat ${INSTALL_PACKAGE_PATH}/conf/client.conf | grep "backup_scene" | ${MYAWK} -F '=' '{print $NF}'`
    fi

    if [ "${BACKUP_SCENE}" = "" ]; then
        echo "Get backup_scene from config  failed."
        LogError "Get backup_scene from config failed."
        ExitHandle 1
    fi

    # 2. Set the umask
    if [ ${BACKUP_SCENE} -eq ${BACKUP_SCENE_INTERNAL} ]; then
        umask 0022
        export USER=root
    fi
}

# check param
CheckParams $*

#check user
CheckAgentUser
if [ $? -ne 0 ]; then
    echo "The installation user already exists, uninstall it before installing it."
    LogError "The installation user already exists." ${ERR_AGENT_PATH_IS_EXIST}
    exit ${ERR_USER_EXIST}
fi

GetBackupScene

# Check version 1.2 is installed
OLD_VERSION_INSTALL_DIR="/opt/OceanProtect/ProtectClient"
# 爱数代理路径，需并存
OTHER_INSTALL_DIR="/opt/OceanProtect/ProtectClient/ProtectClient-A"
if [ -d ${OLD_VERSION_INSTALL_DIR} ] || [ -L ${OLD_VERSION_INSTALL_DIR} ]; then
    if [ -d ${OTHER_INSTALL_DIR} ]; then
        echo "The path ${OTHER_INSTALL_DIR} is already on the host!"
        Log "The path ${OTHER_INSTALL_DIR} is on this host!"
    else
        echo "The installation directory already exists. please enter ${OLD_VERSION_INSTALL_DIR},uninstall it before installing it."
        LogError "The installation directory already exists. please enter ${OLD_VERSION_INSTALL_DIR},uninstall it before installing it" ${ERR_AGENT_PATH_IS_EXIST}
        exit 1
    fi
fi

# Check version 1.3 is installed
if [ "${CLIENT_BACK_ROLE}" != "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ]; then
    OLD_VERSION_INSTALL_DIR="/opt/DataBackup/ProtectClient"
    if [ -d ${OLD_VERSION_INSTALL_DIR} ] || [ -L ${OLD_VERSION_INSTALL_DIR} ]; then
        echo "The installation directory already exists. please enter ${OLD_VERSION_INSTALL_DIR},uninstall it before installing it."
        LogError "The installation directory already exists. please enter ${OLD_VERSION_INSTALL_DIR},uninstall it before installing it" ${ERR_AGENT_PATH_IS_EXIST}
        exit 1
    fi
fi

GetInputInstallPath
if [ $? -ne 0 ]; then
    echo "Failed to obtain the installation directory."
    Log "The installation user already exists."
    exit 1
fi

# Check whether is installed
if [ -d ${INSTALL_DIR} ] || [ -L ${INSTALL_DIR} ]; then
    echo "The installation directory already exists. please enter ${INSTALL_DIR},uninstall it before installing it."
    IS_REINSTALL=1
    LogError "The installation directory already exists." ${ERR_AGENT_PATH_IS_EXIST}
    ExitHandle 1
fi

GetChoice()
{
    DATE=`date +%Y/%m/%d-%H:%M:%S[%Z]`
    for i in 0 1 2
    do
        echo "The current host time and time zone are ${DATE}. Check whether the time and time zone are the same as those on DataBackup time:(y|n)"
        printf "Your choice:"
        if [ "${INSTALLATION_MODE}" = "push" ]; then
            HostDate="y"
            printf " y\n"
        else
            read HostDate
        fi

        if [ "$HostDate" = "n" ] || [ "$HostDate" = "no" ]; then
            echo "The current system time is not consistent with the DataBackup. As a result, the installation process exits abnormally."
            LogError "The current system time is not consistent with the DataBackup. As a result, the installation process exits abnormally." ${ERR_UPGRADE_FAIL_VERIFICATE_PARAMETER}
            ExitHandle 1
        elif [ "$HostDate" = "y" ] || [ "$HostDate" = "yes" ]; then
            return 1
        elif [ -z "$HostDate" ]; then
            echo "Default selection y."
            return 0
        fi
        echo "Please enter y or n."
    done
    echo "The number of incorrect inputs exceeds 3. The installation process exits abnormally."
    LogError "The number of incorrect inputs exceeds 3. The installation process exits abnormally." ${ERR_UPGRADE_FAIL_VERIFICATE_PARAMETER}
    ExitHandle 1
}

GetOsVersion()
{
    SYS_NAME=`uname -s`

    case ${SYS_NAME} in
    "Linux")    
        SYS_ARCH=`arch` 
        if [ "${SYS_ARCH}" = "x86_64" ] || [ "${SYS_ARCH}" = "aarch64" ]
        then
            SYS_BIT="64"
        elif [ "${SYS_ARCH}" = "x86" ] || [ "${SYS_ARCH}" = "aarch32" ]
        then 
            SYS_BIT="32"
        fi
        return 0
    ;;
    "HP-UX")
        SYS_ARCH=`uname -m`
        if [ ${SYS_ARCH} = "ia64" ]
        then 
            SYS_BIT="64"
        else
            SYS_BIT="32"
        fi
        SYS_LEVEL=`uname -a | ${MYAWK} '{print $3}'`
        return 0
    ;;
    "AIX")
        SYS_BIT=`getconf HARDWARE_BITMODE`
        if [ ${SYS_BIT} = "64" ]
        then 
            SYS_ARCH="x86_64"
        else
            SYS_ARCH="x86"
        fi
        return 0
    ;;
    "SunOS")
        SYS_BIT=`isainfo -b`
        if [ ${SYS_BIT} = "64" ]
        then 
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

GetSanCientOsVersion()
{
    if [ "${BACKUP_ROLE}" != "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ]; then
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
            return 1
        fi
    done   
}

CheckCommand()
{
    if [ "${SYS_NAME}" = "AIX" ]; then
        return 1
    fi

    command -v tar 1>/dev/null 2>&1
    if [ $? -eq 0 ]; then
        # 获取tar版本号
        tar_version=$(tar --version | head -n 1 | awk '{print $NF}')

        # 判断版本号是否小于1.22
        if [[ $(echo "$tar_version < 1.22" | bc -l) -eq 1 ]]; then
            echo "ERROR : tar version less than 1.22, please update tar version"
            LogError "tar version less than 1.22, please update tar version"
            ExitHandle 1
        fi
        return 0
    fi 

    Log "The command tar not found."
    LogError "The command tar not found." ${ERR_COMMAND_NOT_FOUND}
    if [ -d "${AGENT_PUSH_INSTALL_PATH}" ]; then
        rm -rf "${LOG_ERR_INSTALL_FILE}" 1>/dev/null 2>&1
        echo "tar" >> ${LOG_ERR_INSTALL_FILE}
        ExitHandle ${ERR_COMMAND_NOT_FOUND_RETCODE}
    fi
}

CheckPamRootOk()
{
    if [ "${SYS_NAME}" != "Linux" ]; then
        return 0
    fi
    if [ -f "/etc/SuSE-release" ]; then
        return 0
    fi
    cat /etc/pam.d/su | grep "pam_rootok.so" | grep "^auth" >/dev/null 2>&1
    if [ $? -eq 0 ]; then
        return 0
    fi 

    ShowError "The system does not have the permission to switch to another user using the su command."
    LogError "The system does not have the permission to switch to another user using the su command." ${ERR_UPGRADE_NO_PAM_ROOTOK}
    if [ -d "${AGENT_PUSH_INSTALL_PATH}" ]; then
        rm -rf "${LOG_ERR_INSTALL_FILE}"
        echo "pam_rootok.so" >> ${LOG_ERR_INSTALL_FILE}
    fi
    ExitHandle ${ERR_COMMAND_NOT_FOUND_RETCODE}
}

CheckNfsUtils()
{
    if [ "${SYS_NAME}" != "Linux" ]; then
        return 0
    fi

    if [ "`cat /etc/issue | grep 'Linx'`" != "" ]; then
        # ROCKY
        return 0
    elif [ -n "`cat /etc/os-release 2>/dev/null | grep -i 'uos'`" ]; then
        # UOS
    which dpkg >/dev/null 2>&1
    if [ $? = 0 ];then
        dpkg -l | grep nfs-common | grep ^ii >/dev/null 2>&1
        if [ $? -eq 0 ]; then
            return 0
        fi
    else 
        rpm -q nfs-utils >/dev/null 2>&1
        if [ $? -eq 0 ]; then
            return 0
        fi
    fi
        ShowError "nfs-common is not found, please install nfs-common first."
        LogError "nfs-common is not found, please install nfs-common first." ${ERR_UPGRADE_HOST_MISS_TOOL_PACKAGE} "nfs-common"
        if [ -d "${AGENT_PUSH_INSTALL_PATH}" ]; then
            rm -rf "${LOG_ERR_INSTALL_FILE}" 1>/dev/null 2>&1
            echo "nfs-common" >> ${LOG_ERR_INSTALL_FILE}
        fi
        ExitHandle ${ERR_COMMAND_NOT_FOUND_RETCODE}
    elif [ -f "/etc/SuSE-release" ]; then
        # SUSE
        rpm -q nfs-client >/dev/null 2>&1
        if [ $? -eq 0 ]; then
            return 0
        fi
        ShowError "nfs-client is not found, please install nfs-client first."
        LogError "nfs-client is not found, please install nfs-client first." ${ERR_UPGRADE_HOST_MISS_TOOL_PACKAGE} "nfs-client"
        if [ -d "${AGENT_PUSH_INSTALL_PATH}" ]; then
            rm -rf "${LOG_ERR_INSTALL_FILE}" 1>/dev/null 2>&1
            echo "nfs-client" >> ${LOG_ERR_INSTALL_FILE}
        fi
        ExitHandle ${ERR_COMMAND_NOT_FOUND_RETCODE}
    elif [ -f "/etc/debian_version" ]; then
        # Ubuntu & Debian & Astra
        dpkg -l | grep nfs-common | grep ^ii >/dev/null 2>&1
        if [ $? -eq 0 ]; then
            return 0
        fi
        ShowError "nfs-common is not found, please install nfs-common first."
        LogError "nfs-common is not found, please install nfs-common first." ${ERR_UPGRADE_HOST_MISS_TOOL_PACKAGE} "nfs-common"
        if [ -d "${AGENT_PUSH_INSTALL_PATH}" ]; then
            rm -rf "${LOG_ERR_INSTALL_FILE}" 1>/dev/null 2>&1
            echo "nfs-common" >> ${LOG_ERR_INSTALL_FILE}
        fi
        ExitHandle ${ERR_COMMAND_NOT_FOUND_RETCODE}
    else
        rpm -q nfs-utils >/dev/null 2>&1
        if [ $? -eq 0 ]; then
            return 0        
        fi

        rpm -q nfs-client >/dev/null 2>&1
        if [ $? -eq 0 ]; then
            return 0
        fi

        ShowError "nfs-utils or nfs-client is not found, please install nfs-utils or nfs-client first."
        LogError "nfs-utils or nfs-client is not found, please install nfs-utils or nfs-client first." ${ERR_UPGRADE_HOST_MISS_TOOL_PACKAGE} "nfs-utils or nfs-client"
        if [ -d "${AGENT_PUSH_INSTALL_PATH}" ]; then
            rm -rf "${LOG_ERR_INSTALL_FILE}" 1>/dev/null 2>&1
            echo "nfs-utils or nfs-client" >> ${LOG_ERR_INSTALL_FILE}
        fi
        ExitHandle ${ERR_COMMAND_NOT_FOUND_RETCODE}
    fi
}

GetConfigInfo()
{
    if [ -f "${INSTALL_PACKAGE_PATH}/conf/client.conf" ]; then
        PM_IP=`cat ${INSTALL_PACKAGE_PATH}/conf/client.conf | grep "pm_ip" | ${MYAWK} -F '=' '{print $NF}'`
        PM_PORT=`cat ${INSTALL_PACKAGE_PATH}/conf/client.conf | grep "pm_port" | ${MYAWK} -F '=' '{print $NF}'`
        PM_MANAGER_IP=`cat ${INSTALL_PACKAGE_PATH}/conf/client.conf | grep "pm_manager_ip" | ${MYAWK} -F '=' '{print $NF}'`
        PM_MANAGER_PORT=`cat ${INSTALL_PACKAGE_PATH}/conf/client.conf | grep "pm_manager_port" | ${MYAWK} -F '=' '{print $NF}'`
        USER_ID=`cat ${INSTALL_PACKAGE_PATH}/conf/client.conf | grep "user_id" | ${MYAWK} -F '=' '{print $NF}'`
        DOMAIN_NAME=`cat ${INSTALL_PACKAGE_PATH}/conf/client.conf | grep "domain" | ${MYAWK} -F '=' '{print $NF}'`
        APPLICATION_INFO=`cat ${INSTALL_PACKAGE_PATH}/conf/client.conf | grep "application_info" | ${MYAWK} -F '=' '{print $NF}'`
        if [ -z "${AGENT_IP}" ]; then
            AGENT_IP=`cat ${INSTALL_PACKAGE_PATH}/conf/client.conf | grep "AGENT_IP" | ${MYAWK} -F '=' '{print $NF}'`
            Log "The ip address of the agent is ${AGENT_IP} which get from AGENT_IP."
        fi
        # 适配hcs环境ecs主机agent安装
        PRIVATE_IP=`cat ${INSTALL_PACKAGE_PATH}/conf/client.conf | grep "private_ip" | ${MYAWK} -F '=' '{print $NF}'`
        FLOATING_IP=`cat ${INSTALL_PACKAGE_PATH}/conf/client.conf | grep "floating_ip" | ${MYAWK} -F '=' '{print $NF}'`
        IS_SHARED=`cat ${INSTALL_PACKAGE_PATH}/conf/client.conf | grep "is_shared" | ${MYAWK} -F '=' '{print $NF}'`
        if [ "$UPGRADE_FLAG_TEMPORARY" = "1" ]; then
            if [ -z "${PRIVATE_IP}" ]; then
                PRIVATE_IP=`cat ${UPGRADE_BACKUP_PATH}/ProtectClient-E/conf/testcfg.tmp | grep "PRIVATE_IP" | ${MYAWK} -F '=' '{print $NF}'`
            fi
            if [ -z "${FLOATING_IP}" ]; then
                FLOATING_IP=`cat ${UPGRADE_BACKUP_PATH}/ProtectClient-E/conf/testcfg.tmp | grep "FLOATING_IP" | ${MYAWK} -F '=' '{print $NF}'`
            fi
            if [ -z "${IS_SHARED}" ]; then
                IS_SHARED=`cat ${UPGRADE_BACKUP_PATH}/ProtectClient-E/conf/testcfg.tmp | grep "IS_SHARED" | ${MYAWK} -F '=' '{print $NF}'`
            fi
            if [ ! -z "${UPGRADE_BEFORE_LOGLEVEL}" ]; then
                LOG_LEVEL=${UPGRADE_BEFORE_LOGLEVEL}
            fi
        fi
        if [ ! -z "${PRIVATE_IP}" ]; then
            AGENT_IP=${PRIVATE_IP}
            Log "The ip address of the agent is ${AGENT_IP} which get from PRIVATE_IP."
        fi
        if [ -z "${LOG_LEVEL}" ]; then
            LOG_LEVEL=`cat ${INSTALL_PACKAGE_PATH}/conf/client.conf | grep "log_level" | ${MYAWK} -F '=' '{print $NF}'`
        fi
        AVAILABLE_ZONE=`cat ${INSTALL_PACKAGE_PATH}/conf/client.conf | grep "available_zone" | ${MYAWK} -F '=' '{print $NF}'`
        IS_AUTO_SYNCHRONIZE_HOST_NAME=`cat ${INSTALL_PACKAGE_PATH}/conf/client.conf | grep "is_auto_synchronize_host_name" | ${MYAWK} -F '=' '{print $NF}'`
        ENABLE_HTTP_PROXY=`cat ${INSTALL_PACKAGE_PATH}/conf/client.conf | grep "enable_http_proxy" | ${MYAWK} -F '=' '{print $NF}'`
        DISABLE_SAFE_RANDOM_NUMBER=`cat ${INSTALL_PACKAGE_PATH}/conf/client.conf | grep "disable_safe_random_number" | ${MYAWK} -F '=' '{print $NF}'`

        IS_DPC_COMPUTE_NODE=`cat ${INSTALL_PACKAGE_PATH}/conf/client.conf | grep "is_dpc_compute_node" | ${MYAWK} -F '=' '{print $NF}'`
        DPC_STORAGE_FRONT_IP=`cat ${INSTALL_PACKAGE_PATH}/conf/client.conf | grep "dpc_storage_front_ip" | ${MYAWK} -F '=' '{print $NF}'`
        DPC_STORAGE_FRONT_GATEWAY=`cat ${INSTALL_PACKAGE_PATH}/conf/client.conf | grep "dpc_storage_front_gateway" | ${MYAWK} -F '=' '{print $NF}'`
        IS_FILECLIENT_INSTALL=`cat ${INSTALL_PACKAGE_PATH}/conf/client.conf | grep "is_fileclient_install" | ${MYAWK} -F '=' '{print $NF}'`
    else
        echo "The configuration file client.conf cannot be found."
        LogError "The configuration file client.conf cannot be found." ${ERR_UPGRADE_INSTALL_PACKAGE_INCOMPLETE}
        ExitHandle 1
    fi

    if [ -f "${INSTALL_PACKAGE_PATH}/conf/package.json" ]; then
        PKG_VERSION=`cat ${INSTALL_PACKAGE_PATH}/conf/package.json | grep "releaseVersion" | ${MYAWK} '{print $2}' | ${MYAWK} -F "," '{print $1}' | ${MYAWK} -F "\"" '{print $2}'`
        VERSION_TIME_STAMP=`cat ${INSTALL_PACKAGE_PATH}/conf/package.json | grep "versionTimeStamp" | ${MYAWK} '{print $2}' | ${MYAWK} -F "," '{print $1}' | ${MYAWK} -F "\"" '{print $2}'`
    else
        echo "The configuration file package.json cannot be found under the dir[${INSTALL_PACKAGE_PATH}/conf]."
        LogError "The configuration file package.json cannot be found under the dir[${INSTALL_PACKAGE_PATH}/conf]." ${ERR_UPGRADE_INSTALL_PACKAGE_INCOMPLETE}
        ExitHandle 1
    fi

    GetBackupRole || ExitHandle 1
}

LegalitySelinuxStatus()
{
    Log "Start to check the SELinux status."
    command -v sestatus >/dev/null 2>&1
    if [ $? -eq 0 ]; then
        selinux_config_status=`sestatus | grep -w "config" | $MYAWK '{print $NF}'`
        host_selinux_status=`sestatus | grep -w "Current mode" | $MYAWK '{print $NF}'`
    else
        return 1
    fi
    
    if [ "${host_selinux_status}" != "${selinux_config_status}" ]; then
        Log "The SELinux configuration file is inconsistent with the actual status, the system may be restarted during the installation."
        Log "You can change the status of the SELinux configuration file to be consistent with the actual host status to avoid this problem."
        return 1
    fi

    return 0
}

# check whether the network tools exist
CheckNetToolExist()
{
    if [ "${SYS_NAME}" = "SunOS" ] || [ "${SYS_NAME}" = "HP-UX" ] || [ "${SYS_NAME}" = "AIX" ]; then
        command -v ifconfig >/dev/null 2>&1
        if [ $? -ne 0 ]; then
            echo "Command ifconfig is not found, please install net-tools first."
            LogError "Command ifconfig is not found, please install net-tools first." ${ERR_UPGRADE_HOST_MISS_TOOL_PACKAGE} "net-tools"
            ExitHandle 1
        fi
        command -v netstat  >/dev/null 2>&1
        if [ $? -ne 0 ]; then
            echo "Command netstat is not found, please install net-tools first."
            LogError "Command netstat is not found, please install net-tools first." ${ERR_UPGRADE_HOST_MISS_TOOL_PACKAGE} "net-tools"
            ExitHandle 1
        fi
    elif [ "${SYS_NAME}" = "Linux" ]; then
        command -v ip >/dev/null 2>&1
        if [ $? -ne 0 ]; then
            echo "Command ip is not found, please install iproute tools first."
            LogError "Command ip is not found, please install iproute tools first." ${ERR_UPGRADE_HOST_MISS_TOOL_PACKAGE} "iproute"
            ExitHandle 1
        fi
        ls /usr/sbin/ss >/dev/null 2>&1 || ls /sbin/ss >/dev/null 2>&1 || ls /bin/ss >/dev/null 2>&1
        if [ $? -ne 0 ]; then
            echo "Command ss is not found, please install iproute tools first."
            LogError "Command ss is not found, please install iproute tools first." ${ERR_UPGRADE_HOST_MISS_TOOL_PACKAGE} "iproute"
        fi
    fi
}

AdaptClientPackage()
{ 
    if [ -d "$1" ]; then
        cd $1
    else
        echo "The diretory[$1] is not exist, error exit."
        LogError "The diretory[$1] is not exist, error exit." ${ERR_UPGRADE_FAIL_VERIFICATE_PARAMETER}
        ExitHandle 1
    fi
    system_name=""
    if [ $SYS_NAME = "Linux" ]; then
        cat /etc/system-release 2>/dev/null | grep CentOS >>/dev/null 2>&1
        if [ $? -eq 0 ]; then
            system_name="Centos"
        fi
    fi

    for cparm in SYS_NAME SYS_ARCH SYS_BIT SYS_LEVEL
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
                    echo "The current system is ${SYS_NAME},no found any suitable pkg."
                    Log "The current system is ${SYS_NAME},no found any suitable pkg."
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
                    echo "The current system is ${SYS_NAME},no found any suitable pkg."
                    Log "The current system is ${SYS_NAME},no found any suitable pkg."
                    return 1
                else
                    continue
                fi
                ;;
            "SYS_LEVEL")
                if [ ${SYS_NAME} = "Linux" ]; then
                    FILE_COUNT=`ls $pwd | grep "${SYS_NAME}" | grep $SYS_ARCH | grep $SYS_BIT | wc -l`
                    if [ $FILE_COUNT -eq 1 ]; then
                        FILE_PATH=`ls $pwd | grep "${SYS_NAME}" | grep $SYS_ARCH | grep $SYS_BIT`
                        Log "The current system is ${SYS_NAME},adaptation succeeded."
                        break
                    else
                        echo "The current system is ${SYS_NAME},no found any suitable pkg."
                        Log "The current system is ${SYS_NAME},no found any suitable pkg."
                        return 1
                    fi
                else
                    echo "The current system is ${SYS_NAME},no found any suitable pkg."				
                    Log "The current system is ${SYS_NAME},no found any suitable pkg."
                    return 1
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



CopyCert()
{
    Log "Start to copy the certificate to the specified path."

    if [ ${BACKUP_SCENE} -eq ${BACKUP_SCENE_INTERNAL} ]; then
        return
    fi

    if [ ! -f "${INSTALL_PACKAGE_PATH}/conf/ca.crt.pem" ] || [ ! -f "${INSTALL_PACKAGE_PATH}/conf/client.crt.pem" ] \
        || [ ! -f "${INSTALL_PACKAGE_PATH}/conf/client.pem" ] || [ ! -f "${INSTALL_PACKAGE_PATH}/conf/agentca.crt.pem" ]; then
        Log "Not find the ca.crt.pem or client.crt.pem client.pem or agentca.crt.pem, please input the cert path:"
        echo "Not find the ca.crt.pem or client.crt.pem client.pem or agentca.crt.pem, please input the cert path:"
        read CERT_PATH
        echo "certpath:${CERT_PATH}"

        if [ ! -d "${CERT_PATH}" ]; then
            echo "The entered path is incorrect. The path does not exist."
            LogError "The entered path is incorrect. The path does not exist." ${ERR_UPGRADE_FAIL_VERIFICATE_CERTIFICATE}
            ExitHandle 1
        else
            echo "The client will use the certificates from the chosen path."
            CERT_PATH_EXIST="true"
        fi

        if [ ! -f "${CERT_PATH}/ca.crt.pem" ]; then
            echo "The path is incorrect, not find ca.crt.pem file."
            LogError "The path is incorrect, not find ca.crt.pem file." ${ERR_UPGRADE_FAIL_VERIFICATE_CERTIFICATE}
            ExitHandle 1
        else
            if [ -d "${INSTALL_DIR}/ProtectClient-E/nginx/conf" ]; then
                cp "${CERT_PATH}/ca.crt.pem" "${INSTALL_DIR}/ProtectClient-E/nginx/conf/pmca.pem"
            fi
        fi

        if [ ! -f "${CERT_PATH}/client.crt.pem" ]; then
            echo "The path is incorrect, not find client.crt.pem file."
            LogError "The path is incorrect, not find client.crt.pem file." ${ERR_UPGRADE_FAIL_VERIFICATE_CERTIFICATE}
            ExitHandle 1
        else
            if [ -d "${INSTALL_DIR}/ProtectClient-E/nginx/conf" ]; then
                cp "${CERT_PATH}/client.crt.pem" "${INSTALL_DIR}/ProtectClient-E/nginx/conf/server.pem"
            fi
        fi

        if [ ! -f "${CERT_PATH}/client.pem" ]; then
            echo "The path is incorrect, not find client.pem file."
            LogError "The path is incorrect, not find client.pem file." ${ERR_UPGRADE_FAIL_VERIFICATE_CERTIFICATE}
            ExitHandle 1
        else
            if [ -d "${INSTALL_DIR}/ProtectClient-E/nginx/conf" ]; then
                cp "${CERT_PATH}/client.pem" "${INSTALL_DIR}/ProtectClient-E/nginx/conf/server.key"
            fi
        fi

        if [ ! -f "${CERT_PATH}/agentca.crt.pem" ]; then
            echo "The path is incorrect, not find agentca.crt.pem file."
            LogError "The path is incorrect, not find agentca.crt.pem file." ${ERR_UPGRADE_FAIL_VERIFICATE_CERTIFICATE}
            ExitHandle 1
        else
            if [ -d "${INSTALL_DIR}/ProtectClient-E/nginx/conf" ]; then
                cp "${CERT_PATH}/agentca.crt.pem" "${INSTALL_DIR}/ProtectClient-E/nginx/conf/agentca.pem"
            fi
        fi
    else
        cp "${INSTALL_PACKAGE_PATH}/conf/ca.crt.pem" "${INSTALL_DIR}/ProtectClient-E/nginx/conf/pmca.pem"
        cp "${INSTALL_PACKAGE_PATH}/conf/client.crt.pem" "${INSTALL_DIR}/ProtectClient-E/nginx/conf/server.pem"
        cp "${INSTALL_PACKAGE_PATH}/conf/client.pem" "${INSTALL_DIR}/ProtectClient-E/nginx/conf/server.key"
        cp "${INSTALL_PACKAGE_PATH}/conf/agentca.crt.pem" "${INSTALL_DIR}/ProtectClient-E/nginx/conf/agentca.pem"
    fi

    if [ ${BACKUP_ROLE} -eq ${BACKUP_ROLE_GENERAL_PLUGIN} ]; then
        # copy dws thrift cert
        mkdir -p "${INSTALL_DIR}/ProtectClient-E/conf/thrift/server"
        mkdir -p "${INSTALL_DIR}/ProtectClient-E/conf/thrift/client"
        cp "${INSTALL_PACKAGE_PATH}/conf/agentca.crt.pem" "${INSTALL_DIR}/ProtectClient-E/conf/thrift/server/ca.crt.pem"
        cp "${INSTALL_PACKAGE_PATH}/conf/agentca.crt.pem" "${INSTALL_DIR}/ProtectClient-E/conf/thrift/client/ca.crt.pem"
        cp "${INSTALL_PACKAGE_PATH}/conf/client.crt.pem" "${INSTALL_DIR}/ProtectClient-E/conf/thrift/server"
        cp "${INSTALL_PACKAGE_PATH}/conf/client.crt.pem" "${INSTALL_DIR}/ProtectClient-E/conf/thrift/client"
        cp "${INSTALL_PACKAGE_PATH}/conf/client.pem" "${INSTALL_DIR}/ProtectClient-E/conf/thrift/server"
        cp "${INSTALL_PACKAGE_PATH}/conf/client.pem" "${INSTALL_DIR}/ProtectClient-E/conf/thrift/client"
    fi
}

WriteParamTmp()
{
    Log "Save parameters to a specified file.[${PARAMS_PATH}/testcfg.tmp]"
    PARAMS_PATH="${INSTALL_DIR}/ProtectClient-E/conf"

    echo SYS_NAME=${SYS_NAME} | tr -d '\r' > ${PARAMS_PATH}/testcfg.tmp
    echo SYS_ARCH=${SYS_ARCH} | tr -d '\r' >> ${PARAMS_PATH}/testcfg.tmp
    echo SYS_BIT=${SYS_BIT} | tr -d '\r' >> ${PARAMS_PATH}/testcfg.tmp
    echo USER_ID=${USER_ID} | tr -d '\r' >> ${PARAMS_PATH}/testcfg.tmp
    echo PKG_CONF_PATH=${PKG_CONF_PATH} | tr -d '\r' >> ${PARAMS_PATH}/testcfg.tmp
    echo PM_IP=${PM_IP} | tr -d '\r' >> ${PARAMS_PATH}/testcfg.tmp
    echo PM_PORT=${PM_PORT} | tr -d '\r' >> ${PARAMS_PATH}/testcfg.tmp
    echo PM_MANAGER_IP=${PM_MANAGER_IP} | tr -d '\r' >> ${PARAMS_PATH}/testcfg.tmp
    echo PM_MANAGER_PORT=${PM_MANAGER_PORT} | tr -d '\r' >> ${PARAMS_PATH}/testcfg.tmp
    echo DOMAIN_NAME=${DOMAIN_NAME} | tr -d '\r' >> ${PARAMS_PATH}/testcfg.tmp
    echo SHELL_TYPE=${SHELL_TYPE} | tr -d '\r' >> ${PARAMS_PATH}/testcfg.tmp

    echo INSTALLATION_MODE=${INSTALLATION_MODE} | tr -d '\r' >> ${PARAMS_PATH}/testcfg.tmp
    echo IS_MANUAL=${IS_MANUAL} | tr -d '\r' >> ${PARAMS_PATH}/testcfg.tmp
    echo PKG_VERSION=${PKG_VERSION} | tr -d '\r' >> ${PARAMS_PATH}/testcfg.tmp
    echo VERSION_TIME_STAMP=${VERSION_TIME_STAMP} | tr -d '\r' >> ${PARAMS_PATH}/testcfg.tmp
    echo UPGRADE_STATUS=${UPGRADE_INITIAL_VALUE} | tr -d '\r' >> ${PARAMS_PATH}/testcfg.tmp
    echo BACKUP_ROLE=${BACKUP_ROLE} | tr -d '\r' >> ${PARAMS_PATH}/testcfg.tmp
    echo BACKUP_SCENE=${BACKUP_SCENE} | tr -d '\r' >> ${PARAMS_PATH}/testcfg.tmp
    echo CERTIFICATE_REVOCATION=${CERTIFICATE_REVOCATION} | tr -d '\r' >> ${PARAMS_PATH}/testcfg.tmp
    echo AGENT_IP=${AGENT_IP} | tr -d '\r' >> ${PARAMS_PATH}/testcfg.tmp
    echo "APPLICATION_INFO=${APPLICATION_INFO}" | tr -d '\r' >> ${PARAMS_PATH}/testcfg.tmp
    echo MODIFY_STATUS=${MODIFY_INITIAL_VALUE} | tr -d '\r' >> ${PARAMS_PATH}/testcfg.tmp
    if [ ! -z "${PRIVATE_IP}" ]; then
        echo PRIVATE_IP=${PRIVATE_IP} | tr -d '\r' >> ${PARAMS_PATH}/testcfg.tmp
    fi
    if [ ! -z "${FLOATING_IP}" ]; then
        echo FLOATING_IP=${FLOATING_IP} | tr -d '\r' >> ${PARAMS_PATH}/testcfg.tmp
    fi
    echo AVAILABLE_ZONE=${AVAILABLE_ZONE} | tr -d '\r' >> ${PARAMS_PATH}/testcfg.tmp
    echo IS_SHARED=${IS_SHARED} | tr -d '\r' >> ${PARAMS_PATH}/testcfg.tmp
	echo IS_DPC_COMPUTE_NODE=${IS_DPC_COMPUTE_NODE} | tr -d '\r' >> ${PARAMS_PATH}/testcfg.tmp
	echo DPC_STORAGE_FRONT_IP=${DPC_STORAGE_FRONT_IP} | tr -d '\r' >> ${PARAMS_PATH}/testcfg.tmp
	echo DPC_STORAGE_FRONT_GATEWAY=${DPC_STORAGE_FRONT_GATEWAY} | tr -d '\r' >> ${PARAMS_PATH}/testcfg.tmp
    echo IS_FILECLIENT_INSTALL=${IS_FILECLIENT_INSTALL} | tr -d '\r' >> ${PARAMS_PATH}/testcfg.tmp
    # set permissions
    chmod 600 ${PARAMS_PATH}/testcfg.tmp
}

CheckShell()
{
    command -v "bash" >/dev/null 2>&1
    if [ 0 -eq $? ]; then
        Log "The current system supports bash, using bash install the DataBackup ProtectAgent."
        SHELL_TYPE="bash"
        return 0
    fi

    command -v "ksh" >/dev/null 2>&1
    if [ 0 -eq $? ]; then
        SHELL_TYPE="ksh"
        Log "The current system no supports bash, using ksh install the DataBackup ProtectAgent."
        return 0
    fi

    echo "No proper shell can be found. The installation process exits abnormally."
    LogError "No proper shell can be found. The installation process exits abnormally." ${ERR_UPGRADE_FAIL_VERIFICATE_PARAMETER}
    ExitHandle 1
}

ClearOldBak()
{
  if [ -d "${INSTALL_DIR_UPPER}/Bak" ]; then
      fileDir="${INSTALL_DIR_UPPER}/Bak"
      num=${BACKUP_MAX_NUM}
      f_size=`ls ${fileDir} | wc -l`

      while(( $f_size > $num ))
      do
      oldFile=`ls -rt $fileDir|head -1`
      echo "Delete file:"$oldFile
      rm -rf $fileDir/$oldFile
      let "f_size-=1"
      done
  fi
}

CopyLogBak()
{
    if [ -d "${UPGRADE_BACKUP_PATH}/ProtectClient-E/log" ]; then
        Log "CopyLogBak......"
        mkdir -p "${BACKUP_COPY_DIR}/log"

        index=1
        while [ 1 ]
        do
            log=`echo "${ARR_OF_LOG}" | $MYAWK -v i="${index}" '{print $i}'`
            if [ "${log}" = "" ]; then
                break
            fi

            if [ -f "${UPGRADE_BACKUP_PATH}/ProtectClient-E/log/${log}" ]; then
                cp -p "${UPGRADE_BACKUP_PATH}/ProtectClient-E/log/${log}" "${BACKUP_COPY_DIR}/log"
            fi

            index=`expr $index + 1`
        done
    fi
}

CopySlogBak()
{
    if [ -d "${UPGRADE_BACKUP_PATH}/ProtectClient-E/slog" ]; then
        Log "CopySlogBak......"
        mkdir -p "${BACKUP_COPY_DIR}/slog"

        index=1
        while [ 1 ]
        do
            slog=`echo "${ARR_OF_SLOG}" | $MYAWK -v i="${index}" '{print $i}'`
            if [ "${slog}" = "" ]; then
                break
            fi

            if [ -f "${UPGRADE_BACKUP_PATH}/ProtectClient-E/slog/${slog}" ]; then
                cp -p "${UPGRADE_BACKUP_PATH}/ProtectClient-E/slog/${slog}" "${BACKUP_COPY_DIR}/slog"
            fi

            index=`expr $index + 1`
        done
    fi
}

CopyConfBak()
{
    if [ -d "${UPGRADE_BACKUP_PATH}/ProtectClient-E/conf" ]; then
      Log "CopyConfBak......"
      mkdir -p "${BACKUP_COPY_DIR}/conf"
      [ -d "${UPGRADE_BACKUP_PATH}/ProtectClient-E/conf" ] && cp -r -p "${UPGRADE_BACKUP_PATH}/ProtectClient-E/conf/." "${BACKUP_COPY_DIR}/conf"
    fi
}

CopyNginxConfBak()
{
    if [ -d "${UPGRADE_BACKUP_PATH}/ProtectClient-E/nginx/conf" ]; then
      Log "CopyNginxConfBak......"
      mkdir -p "${BACKUP_COPY_DIR}/nginxConf"
      [ -d "${UPGRADE_BACKUP_PATH}/ProtectClient-E/nginx/conf" ] && cp -r -p "${UPGRADE_BACKUP_PATH}/ProtectClient-E/nginx/conf/." "${BACKUP_COPY_DIR}/nginxConf"
    fi
}

CopyPluginBak()
{
    typeset name=$1
    typeset srcBakDir=$2
    typeset dstBakDir=$3
    if [ -d "${srcBakDir}" ]; then
        Log "Copy ${name} Bak......"
        mkdir -p "${dstBakDir}"
        if [ -d "${dstBakDir}" ]; then
            cp -r -p "${srcBakDir}" "${dstBakDir}"
        fi
    fi
}

BackupFile()
{
    srcBakFile=$1
    dstBakDir=$2
    if [ -f "${srcBakFile}" ]; then
        Log "Back up file ${srcBakFile} to directory ${dstBakDir}."
        if [ ! -d "${dstBakDir}" ]; then
            mkdir -p "${dstBakDir}"
        fi
        cp -p "${srcBakFile}" "${dstBakDir}"
    fi
}

CopyPluginsBak()
{
    if [ ${BACKUP_ROLE} -eq ${BACKUP_ROLE_SANCLIENT_PLUGIN} ]; then
        return 0
    fi
    Log "Copy plugin dir to ${BACKUP_COPY_DIR}......"
    CopyPluginBak "PluginTmp" "${UPGRADE_BACKUP_PATH}/Plugins/tmp" "${BACKUP_COPY_DIR}/plugins/"

    CopyPluginBak "FilePlugin" "${UPGRADE_BACKUP_PATH}/Plugins/FilePlugin/conf" "${BACKUP_COPY_DIR}/plugins/FilePlugin"

    CopyPluginBak "FusionComputePluginVbstool" "${UPGRADE_BACKUP_PATH}/Plugins/FusionComputePlugin/bin/vbstool" "${BACKUP_COPY_DIR}/plugins/FusionComputePlugin"
    CopyPluginBak "FusionComputePluginTmp" "${UPGRADE_BACKUP_PATH}/Plugins/FusionComputePlugin/tmp" "${BACKUP_COPY_DIR}/plugins/FusionComputePlugin"

    CopyPluginBak "VirtualizationVbstool" "${UPGRADE_BACKUP_PATH}/Plugins/VirtualizationPlugin/vbstool" "${BACKUP_COPY_DIR}/plugins/VirtualizationPlugin"
    CopyPluginBak "VirtualizationVbstool" "${UPGRADE_BACKUP_PATH}/Plugins/VirtualizationPlugin/cert" "${BACKUP_COPY_DIR}/plugins/VirtualizationPlugin"
    
    # hcp文件合并保存
    BackupFile "${UPGRADE_BACKUP_PATH}/Plugins/FusionComputePlugin/conf/hcpconf.ini" "${BACKUP_COPY_DIR}/plugins/FusionComputePlugin/conf"
    BackupFile "${UPGRADE_BACKUP_PATH}/Plugins/VirtualizationPlugin/conf/hcpconf.ini" "${BACKUP_COPY_DIR}/plugins/VirtualizationPlugin/conf"

    BackupFile "${UPGRADE_BACKUP_PATH}/Plugins/GeneralDBPlugin/bin/applications/dws/dws.conf" "${BACKUP_COPY_DIR}/plugins/GeneralDBPlugin/bin/applications/dws"
}

CpAndDecompressPkg()
{
    if [ "$UPGRADE_FLAG_TEMPORARY" = "1" ] && [ -e "${UPGRADE_BACKUP_PATH}" ]; then
        ClearOldBak
        CopyLogBak
        CopySlogBak
        CopyConfBak
        CopyNginxConfBak
        CopyPluginsBak
    fi
    Log "Copying and decompressing the client installation package."

    AdaptClientPackage "${INSTALL_PACKAGE_PATH}/ProtectClient-e"
    if [ $? -eq 1 ]; then
        echo "The current system is ${SYS_NAME},no found any suitable package under the directory of ProtectClient-e."
        LogError "The current system is ${SYS_NAME},no found any suitable package under the directory of ProtectClient-e." ${ERR_UPGRADE_PACKAGE_NO_MATCH_HOST_SYSTEM} "${SYS_NAME}"
        ExitHandle 1
    fi
    if [ "${BACKUP_ROLE}" = "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ];then
        FILE_PATH="sanclient-Linux-x86_64.tar.xz"
    fi
    mkdir -p "${INSTALL_DIR}/ProtectClient-E"
    chmod 755 "${INSTALL_DIR_UPPER}"
    if [ ! -d "${INSTALL_DIR_UPPER}/thirdPart" ]; then
        mkdir -p "${INSTALL_DIR_UPPER}/thirdPart"
        chmod 755 "${INSTALL_DIR_UPPER}/thirdPart"
    fi

    chmod -R 750 "${INSTALL_DIR}/ProtectClient-E"
    cp -r "${INSTALL_PACKAGE_PATH}/ProtectClient-e/${FILE_PATH}" "${INSTALL_DIR}/ProtectClient-E"
    if [ "$SYS_NAME" = "Linux" ]; then
        tar -xf "${INSTALL_DIR}/ProtectClient-E/$FILE_PATH" -C "${INSTALL_DIR}/ProtectClient-E" > /dev/null
    elif [ "$SYS_NAME" = "SunOS" ]; then
        Temporary_Path=`pwd`
        cd "${INSTALL_DIR}/ProtectClient-E"
        gzip -d $FILE_PATH
        protectAgentFile=`ls | grep protect*.tar`
        tar -xf "${INSTALL_DIR}/ProtectClient-E/${protectAgentFile}"
        rm -rf ${protectAgentFile} >/dev/null 2>&1
        cd "$Temporary_Path"
    else
        Temporary_Path=`pwd`
        cd "${INSTALL_DIR}/ProtectClient-E"
        chmod 550 ${INSTALL_PACKAGE_PATH}/third_party_software/XZ/xz
        ${INSTALL_PACKAGE_PATH}/third_party_software/XZ/xz -d $FILE_PATH
        protectAgentFile=${FILE_PATH%%.xz}
        tar -xf "${INSTALL_DIR}/ProtectClient-E/${protectAgentFile}"
        cp ${INSTALL_PACKAGE_PATH}/third_party_software/XZ/xz ${INSTALL_DIR}/ProtectClient-E/bin
        rm -rf ${protectAgentFile} >/dev/null 2>&1
        cd "$Temporary_Path"
    fi
    if [ ${BACKUP_ROLE} -eq ${BACKUP_ROLE_GENERAL_PLUGIN} ]; then
        mkdir -p "${PLUGIN_DIR}"
        chmod -R 750 "${PLUGIN_DIR}"

        find "${INSTALL_PACKAGE_PATH}/ProtectClient-e/Plugins" -name "*.tar.gz" | xargs -I{} cp -rf {} "${PLUGIN_DIR}"
        find "${INSTALL_PACKAGE_PATH}/ProtectClient-e/Plugins" -name "*.tar.xz" | xargs -I{} cp -rf {} "${PLUGIN_DIR}"

    fi
    cp "${INSTALL_DIR}/ProtectClient-E/sbin/xmlcfg" "${INSTALL_DIR}/ProtectClient-E/bin"
    rm -rf "${INSTALL_DIR}/ProtectClient-E/$FILE_PATH"

    if [ -f "${INSTALL_PACKAGE_PATH}/conf/package.json" ]; then
        cp -r "${INSTALL_PACKAGE_PATH}/conf/package.json" "${INSTALL_DIR}/ProtectClient-E/conf"
    else
        echo "The configuration file package.json cannot be found under the dir[${INSTALL_PACKAGE_PATH}/conf]."
        LogError "The configuration file package.json cannot be found under the dir[${INSTALL_PACKAGE_PATH}/conf]." ${ERR_UPGRADE_INSTALL_PACKAGE_INCOMPLETE}
        ExitHandle 1
    fi

    mkdir -p ${INSTALL_DIR}/ProtectClient-E/upgrade
    chmod -R 400 ${INSTALL_DIR}/ProtectClient-E/upgrade
    cp -r "${INSTALL_PACKAGE_PATH}/conf/upgrade_public_key.pem" ${INSTALL_DIR}/ProtectClient-E/upgrade
    touch ${INSTALL_DIR}/ProtectClient-E/stmp/upgrade_signature.sign
    chmod 660 ${INSTALL_DIR}/ProtectClient-E/stmp/upgrade_signature.sign
    chown root:${AGENT_USER} ${INSTALL_DIR}/ProtectClient-E/stmp/upgrade_signature.sign >/dev/null 2>&1
}

ReplaceScript()
{
    Log "Replacing scripts to a specified directory."
    chmod +x ${INSTALL_PACKAGE_PATH}/*.sh
    cp -r "${INSTALL_PACKAGE_PATH}/start.sh" "${INSTALL_DIR}"
    cp -r "${INSTALL_PACKAGE_PATH}/stop.sh" "${INSTALL_DIR}"
    cp -r "${INSTALL_PACKAGE_PATH}/uninstall.sh" "${INSTALL_DIR}"
    cp -r "${INSTALL_PACKAGE_PATH}/collectlog.sh" "${INSTALL_DIR}"
    cp -r "${INSTALL_PACKAGE_PATH}/updateCert.sh" "${INSTALL_DIR}"
    cp -r "${INSTALL_PACKAGE_PATH}/func_util.sh" "${INSTALL_DIR}"
    cp -r "${INSTALL_PACKAGE_PATH}/crl_update.sh" "${INSTALL_DIR}"
    chmod 500 ${INSTALL_DIR}/*.sh
    CHMOD 550 "${INSTALL_DIR}/uninstall.sh"  #no write
    CHMOD 550 "${INSTALL_DIR}/updateCert.sh"
    CHMOD 550 "${INSTALL_DIR}/crl_update.sh"
}

WriteAgentConfig()
{
    if [ "${ASAN_OPTIONS}" != "0" ]; then
        SUExecCmd "${AGENT_ROOT_PATH}/bin/xmlcfg write Monitor rdagent vm_size 104857600"
    fi
    if [ ! -z "${LOG_LEVEL}" ]; then
        SUExecCmd "${AGENT_ROOT_PATH}/bin/xmlcfg write System log_level ${LOG_LEVEL}"
    fi
    if [ "${ENABLE_HTTP_PROXY}" != "" ]; then
        SUExecCmd "${AGENT_ROOT_PATH}/bin/xmlcfg write System enable_http_proxy ${ENABLE_HTTP_PROXY}"
    fi

    if [ "${DISABLE_SAFE_RANDOM_NUMBER}" != "0" ]; then
        SUExecCmd "${AGENT_ROOT_PATH}/bin/xmlcfg write Security disable_safe_random_number ${DISABLE_SAFE_RANDOM_NUMBER}"
    fi
}

ExecuteInstall()
{
    tmpResCode=0
    Log "Start install ${PRODUCT_NAME}."
    if [ "${BACKUP_SCENE}" != "${BACKUP_SCENE_INTERNAL}" ]; then
        Log "Add Defaults:rdadmin !requiretty to /etc/sudoers"
        echo "Defaults:rdadmin !requiretty" >> /etc/sudoers
    fi
    if [ -f "${INSTALL_DIR}/ProtectClient-E/sbin/agent_install.sh" ]; then
        cd "${INSTALL_DIR}/ProtectClient-E/sbin"
        ./agent_install.sh
        
        tmpResCode=$?
        if [ $tmpResCode -eq 250 ]; then
            Log "Some plugins do not support installation."
            return $tmpResCode
        fi
        if [ $tmpResCode -ne 0 ]; then
            Log "DataBackup ProtectAgent is installed on the host failed."
            ExitHandle $tmpResCode
        fi
    else
        echo "The sub-installation script is not found."
        LogError "The sub-installation script is not found." ${ERR_UPGRADE_INSTALL_PACKAGE_INCOMPLETE}
        ExitHandle 1
    fi

    return $tmpResCode
}

ManualInstallation()
{
    # Manual installation
    printf "\\033[1;32mPlease choose the backup type you want to support:\\033[0m \n"
    printf "   [ (1)Oracle   |   (2)Vmware   |   (3)Plugins]\n"
    
    for i in 1 2 3
    do
        printf "   Please input your select: "
        read choice
        if [ "$choice" = "1" ]; then
            BACKUP_ROLE=${BACKUP_ROLE_HOST}
            break
        elif [ "$choice" = "2" ]; then
            BACKUP_ROLE=${BACKUP_ROLE_VMWARE}
            break
        elif [ "$choice" = "3" ]; then
            BACKUP_ROLE=${BACKUP_ROLE_GENERAL_PLUGIN}
            break
        fi
        echo "Input error, please try again."
    done
}

ContainerSecure()
{
    if [ ${BACKUP_SCENE} -ne ${BACKUP_SCENE_INTERNAL} ]; then
        return
    fi

    # 1. set mount_oper.sh permissions
    mkdir "/opt/script"
    cp "${INSTALL_DIR}/ProtectClient-E/sbin/mount_oper.sh" "/opt/script"
    cp "${INSTALL_DIR}/ProtectClient-E/sbin/change_permission.sh" "/opt/script"
    chown root:nobody "/opt/script"
    chown root:root "/opt/script"/*.sh
    chmod -R 550 "/opt/script"

    # 2. set the sudo permission for user rdadmin.
    echo "rdadmin   ALL=NOPASSWD: NOPASSWD:/opt/script/mount_oper.sh" >> /etc/sudoers
    echo "rdadmin   ALL=NOPASSWD: NOPASSWD:/opt/script/change_permission.sh" >> /etc/sudoers
    echo "rdadmin   ALL=NOPASSWD: NOPASSWD:${INSTALL_DIR}/ProtectClient-E/sbin/rootexec" >> /etc/sudoers
    echo "rdadmin   ALL=NOPASSWD: NOPASSWD:/usr/sbin/iscsid" >> /etc/sudoers
    echo "rdadmin   ALL=NOPASSWD: NOPASSWD:/usr/sbin/iscsiadm" >> /etc/sudoers
    rm -rf /usr/bin/kmcdecrypt

    # 3. HostSN
    if [ ! -d "/etc/HostSN" ]; then
        mkdir "/etc/HostSN"
    fi
    touch "/etc/HostSN/HostSN"
    chmod -R 755 /etc/HostSN
    chown -R rdadmin:rdadmin "/etc/HostSN"

    # 4 env
    if [ "$SYS_NAME" = "SunOS" ]; then
        echo "AGENT_ROOT=${INSTALL_DIR}/ProtectClient-E/" >> /home/rdadmin/.bashrc
        echo "export AGENT_ROOT" >> /home/rdadmin/.bashrc
        echo "LD_LIBRARY_PATH=${INSTALL_DIR}/ProtectClient-E/bin" >> /home/rdadmin/.bashrc
        echo "export LD_LIBRARY_PATH" >> /home/rdadmin/.bashrc
        echo "LOGNAME=rdadmin" >> /home/rdadmin/.bashrc
        echo "export LOGNAME" >> /home/rdadmin/.bashrc
    else
        echo "export AGENT_ROOT=${INSTALL_DIR}/ProtectClient-E/" >> /home/rdadmin/.bashrc
        echo "export LD_LIBRARY_PATH=${INSTALL_DIR}/ProtectClient-E/bin" >> /home/rdadmin/.bashrc
        echo "export LOGNAME=rdadmin" >> /home/rdadmin/.bashrc
    fi
}

# set asan and tsan env
SetAsanAndTsanEnv()
{
    if [ "${ASAN_OPTIONS}" = "1" ]; then
        ASAN_OPTIONS="halt_on_error=0:use_sigaltstack=0:detect_leaks=1:malloc_context_size=15:log_path=/home/${AGENT_USER}/logs/asan_report:quarantine_size=4194304"
        UBSAN_OPTIONS="log_path=/home/${AGENT_USER}/logs/ubsan_report:verbosity=3:halt_on_error=0:exitcode=0"
    elif [ "${TSAN_OPTIONS}" = "1" ]; then
        TSAN_OPTIONS="detect_deadlocks=1:halt_on_error=0:suppress_equal_stacks=1:log_path=/home/${AGENT_USER}/logs/tsan_report:exitcode=0"
    fi

    return 0
}

# link to std cpp so
CreateSoftLinkToStdcpp()
{
    if [ "$SYS_NAME" = "Linux" ]; then
        COMMON_LIBSTDCPP="libstdc++.so.6"
        [ -f "${COMMON_LIBSTDCPP}" ] && return 0
        LIB_STD_CPP_SO=`find ${AGENT_ROOT_PATH}/bin -name "libstdc++.so.6.*"`
        ln -fs "${LIB_STD_CPP_SO}" "${AGENT_ROOT_PATH}/bin/${COMMON_LIBSTDCPP}"
    fi
}

CreateSoftLinkToLibcrypt()
{
    if [ "$SYS_NAME" = "Linux" ]; then
        LIB_CRYPT_SO=`find ${AGENT_ROOT_PATH}/bin -name "libcrypt.so*"`
        if [ "$LIB_CRYPT_SO" != "" ]; then
            [ -f "libcrypt.so.1" ] && return 0
            ln -fs "${LIB_CRYPT_SO}" "${AGENT_ROOT_PATH}/bin/libcrypt.so.1"
        fi
    fi
}

# 1.Selinux Exception judgments
LegalitySelinuxStatus

# 2.Select the shell type supported by the system
CheckShell

# 3.Obtaining OS Information
GetOsVersion
if [ $? -eq 1 ]; then
    echo "Failed to obtain the operating system information."
    LogError "Failed to obtain the operating system information." ${ERR_UPGRADE_FAIL_GET_SYSTEM_PARAMETERS}
    ExitHandle 1
fi

# 4.chek whether the command exists.
CheckCommand

# Check whether the permission to switch the user using the su command exists.
CheckPamRootOk

# 5.Check whether nfs-utils exists.
CheckNfsUtils

# 6.Obtaining Client.conf Information
GetConfigInfo

GetIsSharedChoice

if [ "$UPGRADE_FLAG_TEMPORARY" = "1" ]; then
    printf "\\033[1;32mStart the installation of ${PRODUCT_NAME}.\\033[0m \n"
else 
    printf "\\033[1;32m****************************************************************\\033[0m \n"
    printf "\\033[1;32m     Start the installation of ${PRODUCT_NAME}           \\033[0m \n"
    printf "\\033[1;32m****************************************************************\\033[0m \n"
fi

if [ "$UPGRADE_FLAG_TEMPORARY" != "1" ] && [ ${BACKUP_SCENE} -eq ${BACKUP_SCENE_EXTERNAL} ]; then
    GetChoice
fi

# SanCliet check OS 
GetSanCientOsVersion
if [ $? -ne 0 ]; then
    ShowWarning "Warning: The current system may not support sanclient."
    Log "The current system may not support sanclient."
fi

# 7.Check whether the needed network tools exist
CheckNetToolExist

# 8.Manual install
if [ $IS_MANUAL -eq 1 ]; then
    ManualInstallation
fi

# 9.copy and decompress package
CpAndDecompressPkg

# 10.replace script
ReplaceScript

# 11.check free space and Logic core
CheckHostResource

# 12.Copy certificates
CopyCert

# 13.Input parameters
WriteParamTmp
cp ${INSTALL_PACKAGE_PATH}/conf/client.conf ${INSTALL_DIR}/ProtectClient-E/conf

# 14.Set asan and tsan env
SetAsanAndTsanEnv

# 15.create soft link
CreateSoftLinkToStdcpp


CreateSoftLinkToLibcrypt

# 16.Add user and group
# 17.Add nobody to rdadmin
id ${AGENT_USER} >/dev/null 2>&1
if [  $? -eq 0 ] && [ "$UPGRADE_FLAG_TEMPORARY" = "1" ]; then
    # B版本特殊处理，防止升级失败
    Log "The upgrade process does not require users and groups to be reset."
else
    IsKylinOS
    if [ $? -eq 0 ]; then
        USER_CHECK=`cat /etc/passwd | grep "^${AGENT_USER}:"`
        if [ "$USER_CHECK" != "" ]; then
            Log "Kylin system does not require users and groups to be reset."
        else
            Log "Kylin add user and group."
            AddUserAndGroup ${BACKUP_ROLE} ${BACKUP_SCENE}
        fi
    else
        AddUserAndGroup ${BACKUP_ROLE} ${BACKUP_SCENE}
    fi
fi

# 18.Check user env
CheckRdadminUserEnv

# 19.Changing File Permissions
Log "Modify the ProtectAgent file permissions."
echo "Modify the ProtectAgent file permissions."
if [ $BACKUP_SCENE -eq $BACKUP_SCENE_INTERNAL ]; then
    ChangePrivilege "nobody"
else
    ChangePrivilege "${AGENT_USER}"
fi

WriteAgentConfig

if [ "${IS_AUTO_SYNCHRONIZE_HOST_NAME}" != "false" ]; then
    SUExecCmd "${AGENT_ROOT_PATH}/bin/xmlcfg write System is_auto_synchronize_host_name ${IS_AUTO_SYNCHRONIZE_HOST_NAME}"
fi

if [ "${IS_DPC_COMPUTE_NODE}" = "true" ]; then
	SUExecCmd "${AGENT_ROOT_PATH}/bin/xmlcfg write Backup is_dpc_compute_node  ${IS_DPC_COMPUTE_NODE}"
fi
# execute install
ExecuteInstall

# Configure the container
ContainerSecure

ExitHandle ${tmpResCode}