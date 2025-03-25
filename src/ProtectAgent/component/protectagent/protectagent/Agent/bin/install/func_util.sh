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
#############################################
# this shell for basic function
#############################################
AGENT_USER=rdadmin
AGENT_GROUP=rdadmin
GROUP_NOBODY=nobody
DEE_GROUP=dataenableengine
SANCLIENT_USER=sanclient
SANCLIENT_GROUP=sanclient

SHELL_TYPE_SH="/bin/sh"
ERR_WORKINGUSER_ADD_FAILED=1577209885
ERR_WORKINGUSER_ADD_FAILED_RETCODE=14
USER_NOLOGIN_SHELL="/sbin/nologin"
SANCLIENT_AGENT_ROOT="/opt/DataBackup/SanClient/ProtectClient-E/"

CURRENT_PATH=`dirname $0` && cd $CURRENT_PATH && CURRENT_PATH=`pwd`
ENV_FILE=".bashrc"
ENV_EFFECT="export"
ENV_FLAG="="
SANCLIENT_INSTALL_PATH="/opt/DataBackup/SanCliet"

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
#####upgrade errcode end #####

###### BACKUP ROLE #######
BACKUP_ROLE_HOST=0
BACKUP_ROLE_VMWARE=2
BACKUP_ROLE_GENERAL_PLUGIN=4
BACKUP_ROLE_SANCLIENT_PLUGIN=5

###### BACKUP SCENE ######
BACKUP_SCENE_EXTERNAL=0
BACKUP_SCENE_INTERNAL=1

########################################################################################
SYS_NAME=`uname -s`
if [ "${SYS_NAME}" = "SunOS" ]; then
    AWK=nawk
else
    AWK=awk
fi

CLIENT_BACK_ROLE=""
if [ -f "${CURRENT_PATH}/conf/client.conf" ]; then
    CLIENT_BACK_ROLE=`cat ${CURRENT_PATH}/conf/client.conf | grep "backup_role" | ${AWK} -F '=' '{print $NF}'`
fi 
TESTCFG_BACK_ROLE=""
if [ "${CURRENT_PATH}" = "/opt/DataBackup/SanClient" ]; then
    TESTCFG_BACK_ROLE=`su - ${SANCLIENT_USER} -s ${SHELL_TYPE_SH} -c "sed '/^BACKUP_ROLE=/!d;s/.*=//' /opt/DataBackup/SanClient/ProtectClient-E/conf/testcfg.tmp "`
fi
if [ "${CLIENT_BACK_ROLE}" = "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ] || [ "${TESTCFG_BACK_ROLE}" = "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ] ; then
    AGENT_ROOT=${SANCLIENT_AGENT_ROOT}
    AGENT_USER=${SANCLIENT_USER}
    AGENT_GROUP=${SANCLIENT_GROUP}
    AGENT_ROOT_PATH=${SANCLIENT_AGENT_ROOT}
    LD_LIBRARY_PATH=${SANCLIENT_AGENT_ROOT}/bin
    LIBPATH=${SANCLIENT_AGENT_ROOT}/bin
    INSTALL_PATH=${SANCLIENT_INSTALL_PATH}
fi

if [ "${SYS_NAME}" = "SunOS" ]; then
    DEFAULT_HOME_DIR="/export/home/${AGENT_USER}"
else
    DEFAULT_HOME_DIR="/home/${AGENT_USER}"
fi

if [ "`ls -al /bin/sh | ${AWK} '{print $NF}'`" = "bash" ]; then
    EXPORT_ENV="if [ -f ~/.profile ]; then . ~/.profile; fi;"
else
    EXPORT_ENV=""
fi

if [ "${SYS_NAME}" = "Linux" ]; then
    SysRockyFlag=`cat /etc/issue | grep 'Linx'`
    rocky4=`cat /etc/issue | grep 'Rocky'`
    if [ -n "${rocky4}" ]; then
        ROCKY4_FLAG=1
    fi
fi

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
        Log "UTIL: source file  is a link file can not copy."
        return
    fi
    chmod $arg $arg2 "$source"  >>${LOG_FILE_NAME} 2>&1
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

    SYSNAME_JUDGE=`uname -s`
    if [ "$SYSNAME_JUDGE" = "SunOS" ]; then
        cmd="cp -R -P $*"
        $cmd
    else
        cmd="cp -d $*"
        $cmd
    fi
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
        chmod 640 "${LOG_ERR_FILE}"
    fi
    echo "logDetail=$1" > ${LOG_ERR_FILE}
    echo "logInfo=$2" >> ${LOG_ERR_FILE}
    echo "logDetailParam=$3" >> ${LOG_ERR_FILE}
}

# echo err message in red color
ShowError()
{
    printf "\\033[1;31m$1\\033[0m\n"
}

# echo info message in green color
ShowInfo()
{
    printf "\\033[1;32m$1\\033[0m\n"
}

# echo warning message in purple color
ShowWarning()
{
    printf "\\033[1;35m$1\\033[0m\n"
}

VerifySpecialChar()
{
    SPECIAL_CHARS="[\`$;|&<>\!]"
    for arg in $*
    do
        echo "$arg" | grep ${SPECIAL_CHARS} 1>/dev/null 2>&1
        if [ $? -eq 0 ]; then
            Log "The variable[$arg] cannot contain special characters."
            exit 1
        fi
    done
}

IsKylinOS()
{
    if [ -f "/etc/kylin-release" ]; then
        Log "This is kylin system, kylin-release exist."
        return 0
    fi

    cat /etc/os-release | grep PRETTY_NAME | grep -i Kylin >/dev/null 2>&1
    if [ $? -eq 0 ]; then
        Log "This is kylin system, there is kylin in PRETTY_NAME."
        return 0
    fi

    return 1
}

# only bash, ksh and csh
ChooseShell()
{
    shellList="bash ksh csh"
    for PerShell in $shellList
    do
        which ${PerShell} 2>/dev/null | $AWK '{print $1}' >> "${LOG_FILE_NAME}"
        if [ "$?" = "0" ]; then
            USER_SHELL_TYPE=`which ${PerShell} 2>/dev/null | $AWK '{print $1}'`
            return 0
        fi
    done
    echo "There is no proper shell in the current system."
    Log "UTIL: There is no proper shell in the current system."
    LogError "UTIL: There is no proper shell in the current system." ${ERR_WORKINGUSER_ADD_FAILED}
    exit $ERR_WORKINGUSER_ADD_FAILED_RETCODE
}

GetCfg()
{
    cfg_path=$1
    SYS_NAME=`sed '/^SYS_NAME=/!d;s/.*=//' $cfg_path`
    BACKUP_ROLE=`sed '/^BACKUP_ROLE=/!d;s/.*=//' $cfg_path`
    BACKUP_SCENE=`sed '/^BACKUP_SCENE=/!d;s/.*=//' $cfg_path`
    VerifySpecialChar "$SYS_NAME"
    VerifySpecialChar "$BACKUP_ROLE"
    VerifySpecialChar "$BACKUP_SCENE"
    if [ "${BACKUP_ROLE}" = "" ]; then
        echo "Get BACKUP_ROLE from config  failed."
        Log "Get BACKUP_ROLE from config failed."
        exit 1
    fi
}

AddUserGroup()
{
    tempGid=$1
    if [ "${SYS_NAME}" = "AIX" ]; then
        mkgroup id=${tempGid} ${AGENT_GROUP}
    else
        groupadd -g ${tempGid} ${AGENT_GROUP}
    fi
    
    if [ 0 -ne $? ]; then
        echo "DataBackup ProtectAgent working group ${AGENT_GROUP} was added failed."
        echo "The installation of DataBackup ProtectAgent will be stopped."
        LogError "DataBackup ProtectAgent working group ${AGENT_GROUP} was added failed." ${ERR_UPGRADE_FAIL_ADD_USERGROUP}
        exit $ERR_WORKINGUSER_ADD_FAILED_RETCODE
    fi
}

AddDeeGroup()
{
    if [ ${BACKUP_ROLE} -eq ${BACKUP_ROLE_VMWARE} ] && [ "${SYS_NAME}" = "Linux" ]; then
        deeGroupCheckRet=`cat /etc/group | grep "^${DEE_GROUP}:"`
        deeGid=`${INSTALL_PATH}/ProtectClient-E/sbin/xmlcfg read DataProcess dee_gid`
        if [ "${deeGroupCheckRet}" = "" ]; then
            groupadd -g ${deeGid} ${DEE_GROUP}
            if [ 0 -ne $? ]; then
                echo "Add dee group ${DEE_GROUP} failed."
                echo "The installation of DataBackup ProtectAgent will be stopped."
                Log "UTIL: Add dee ${DEE_GROUP} failed."
                LogError "UTIL: Add dee ${DEE_GROUP} failed." ${ERR_UPGRADE_FAIL_ADD_USERGROUP}
                exit $ERR_WORKINGUSER_ADD_FAILED_RETCODE
            fi
        fi
    fi
}

AddUser()
{
    userCheckRet=`cat /etc/passwd | grep "^${AGENT_USER}:"`
    if [ -n "${userCheckRet}" ]; then
        echo "The DataBackup ProtectAgent working user ${AGENT_USER} has already existed, please delete it before install."
        LogError "UTIL: The DataBackup ProtectAgent working user ${AGENT_USER} has already existed, please delete it before install." ${ERR_UPGRADE_FAIL_ADD_USER}
        rm -rf "`dirname "${INSTALL_DIR}"`"
        exit $ERR_WORKINGUSER_EXISTS
    fi

    if [ -d "${DEFAULT_HOME_DIR}" ]; then
        echo "The home directory ${DEFAULT_HOME_DIR} of user ${AGENT_USER} exists. Please delete the directory."
        echo "The installation of DataBackup ProtectAgent will be stopped."
        LogError "UTIL: The home directory ${DEFAULT_HOME_DIR} of user ${AGENT_USER} exists. Please delete the directory." ${ERR_UPGRADE_FAIL_VERIFICATE_PARAMETER}
        exit $ERR_WORKINGUSER_ADD_FAILED_RETCODE
    fi

    Log "UTIL: Agent working user choose shell type ${USER_SHELL_TYPE}."
    
    if [ "${SYS_NAME}" = "SunOS" ] || [ "${SYS_NAME}" = "AIX" ]; then
        useradd -m -d ${DEFAULT_HOME_DIR} -g ${AGENT_GROUP} -u ${tempUid} ${AGENT_USER} >/dev/null 2>&1
    else
        if [ ${BACKUP_SCENE} -eq ${BACKUP_SCENE_INTERNAL} ]; then
            useradd -m -s ${USER_NOLOGIN_SHELL} -g 99 -G ${AGENT_GROUP} ${AGENT_USER}
            if [ $? -ne 0 ]; then
                echo "Add user fail."
                Log "UTIL: Add user fail."
                exit 1
            else
                Log "UTIL: Add user succ."
                return
            fi
        else
            useradd -m -s ${USER_NOLOGIN_SHELL} -g ${AGENT_GROUP} -u ${tempUid} ${AGENT_USER}
        fi
    fi
    
    if [ 0 -ne $? ]; then
        echo "The DataBackup ProtectAgent working user ${AGENT_USER} was added failed."
        echo "The installation of DataBackup ProtectAgent will be stopped."
        LogError "UTIL: The DataBackup ProtectAgent working user ${AGENT_USER} has already existed, please delete it before install." ${ERR_UPGRADE_FAIL_ADD_USER}
        exit $ERR_WORKINGUSER_ADD_FAILED_RETCODE
    fi
}

ModifyGroup()
{
    groupCheckRet=$1
    if [ "" = "${groupCheckRet}" ]; then
        if [ "${SYS_NAME}" = "HP-UX" ]; then
            hpOsVersion=`uname -a | $AWK '{print $3}' | $AWK -F "." '{print $2"."$3}'`
            if [ "${hpOsVersion}" = "11.31" ]; then
                usermod -F -g ${AGENT_GROUP} ${AGENT_USER} 2>/dev/null
            else
                usermod -g ${AGENT_GROUP} ${AGENT_USER} 2>/dev/null
            fi
        else
            usermod -g ${AGENT_GROUP} ${AGENT_USER} 2>/dev/null
        fi
        
        if [ 0 -ne $? ]; then
            echo "Add Agent working user ${AGENT_USER} to group ${AGENT_GROUP} failed."
            echo "The installation of DataBackup ProtectAgent will be stopped."
            Log "UTIL: Add Agent working user ${AGENT_USER} to group ${AGENT_GROUP} failed."
            LogError "UTIL: Add Agent working user ${AGENT_USER} to group ${AGENT_GROUP} failed." $ERR_WORKINGUSER_ADD_FAILED
            exit $ERR_WORKINGUSER_ADD_FAILED_RETCODE
        fi
    else
        if [ "${SYS_NAME}" = "SunOS" ]; then
            tmpGroup=`/usr/xpg4/bin/id -g -n ${AGENT_GROUP}`
        else
            tmpGroup=`id -g -n ${AGENT_GROUP}`
        fi
        if [ "${tmpGroup}" != "${AGENT_GROUP}" ]; then 
            if [ "${SYS_NAME}" = "HP-UX" ]; then
                hpOsVersion=`uname -a | $AWK '{print $3}' | $AWK -F "." '{print $2"."$3}'`
                if [ "${hpOsVersion}" = "11.31" ]; then
                    usermod -F -g ${AGENT_GROUP} ${AGENT_USER} 2>/dev/null
                else
                    usermod -g ${AGENT_GROUP} ${AGENT_USER} 2>/dev/null
                fi
            else
                usermod -g ${AGENT_GROUP} ${AGENT_USER} 2>/dev/null
            fi
        
            if [ 0 -ne $? ]; then
                echo "Add agent working user ${AGENT_USER} to group ${AGENT_GROUP} failed."
                echo "The installation of DataBackup ProtectAgent will be stopped."
                LogError "UTIL: Add agent working user ${AGENT_USER} to group ${AGENT_GROUP} failed." ${ERR_UPGRADE_FAIL_ADD_USERGROUP}
                exit $ERR_WORKINGUSER_ADD_FAILED_RETCODE
            fi
        fi
    fi
}

# Add specified uid[user], gid[group]
AddSpecifyIdUserAndGroup()
{
    tempUid=$1
    tempGid=$2
    GetCfg $3
    groupCheckRet=`cat /etc/group | grep "^${AGENT_GROUP}:"`
    if [ "" = "${groupCheckRet}" ]; then
        AddUserGroup $tempGid
    fi
    AddDeeGroup
    AddUser
    ModifyGroup $groupCheckRet
}

# root exec
AddUserAndGroup()
{
    export LD_LIBRARY_PATH=${AGENT_ROOT_PATH}/bin
    BACKUP_ROLE=$1
    BACKUP_SCENE=$2
    #check Agent group
    GROUP_CHECK=`cat /etc/group | grep "^${AGENT_GROUP}:"`
    if [ "" = "${GROUP_CHECK}" ]; then
        tempGid=`${AGENT_ROOT_PATH}/sbin/xmlcfg read Backup rdadmin_gid`
        
        if [ "$tempGid" = "" ]
        then
            echo "Read radmin group id failed from xml conf."
            exit $ERR_WORKINGUSER_ADD_FAILED_RETCODE
        fi
        if [ "${SYS_NAME}" = "AIX" ]; then
            mkgroup id=${tempGid} ${AGENT_GROUP}
        elif [ "${BACKUP_ROLE}" = "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ]; then
            sanclientGid=`${AGENT_ROOT_PATH}/sbin/xmlcfg read Backup sanclient_gid`
            groupadd -g ${sanclientGid} ${AGENT_GROUP}
        else
            groupadd -g ${tempGid} ${AGENT_GROUP}
        fi

        if [ 0 -ne $? ]; then
            echo "DataBackup ProtectAgent working group ${AGENT_GROUP} was added failed."
            echo "The installation of DataBackup ProtectAgent will be stopped."
            LogError "DataBackup ProtectAgent working group ${AGENT_GROUP} was added failed." ${ERR_UPGRADE_FAIL_ADD_USERGROUP}
            exit $ERR_WORKINGUSER_ADD_FAILED_RETCODE
        fi
    fi

    #add dee group
    if [ ${BACKUP_ROLE} -eq ${BACKUP_ROLE_VMWARE} ] && [ "${SYS_NAME}" = "Linux" ]; then
        DEE_GROUP_CHECK=`cat /etc/group | grep "^${DEE_GROUP}:"`
        DEE_GID=`${AGENT_ROOT_PATH}/sbin/xmlcfg read DataProcess dee_gid`
        if [ "${DEE_GROUP_CHECK}" = "" ]; then
            groupadd -g ${DEE_GID} ${DEE_GROUP}
            if [ 0 -ne $? ]; then
                echo "Add dee group ${DEE_GROUP} failed."
                echo "The installation of DataBackup ProtectAgent will be stopped."
                Log "Add dee ${DEE_GROUP} failed."
                LogError "Add dee ${DEE_GROUP} failed." ${ERR_WORKINGUSER_ADD_FAILED}
                exit $ERR_WORKINGUSER_ADD_FAILED_RETCODE
            fi
        fi
    fi
    
    #check Agent user
    USER_CHECK=`cat /etc/passwd | grep "^${AGENT_USER}:"`
    if [ "" = "${USER_CHECK}" ]; then
        if [ -d "${DEFAULT_HOME_DIR}" ]; then
            echo "The home directory ${DEFAULT_HOME_DIR} of user ${AGENT_USER} exists. Please delete the directory."
            echo "The installation of DataBackup ProtectAgent will be stopped."
            LogError "The home directory ${DEFAULT_HOME_DIR} of user ${AGENT_USER} exists. Please delete the directory." ${ERR_UPGRADE_FAIL_VERIFICATE_PARAMETER}
            exit $ERR_WORKINGUSER_EXISTS
        fi

        ChooseShell
        Log "Agent working user choose shell type ${USER_SHELL_TYPE}."
        
        if [ "${SYS_NAME}" = "SunOS" ] || [ "${SYS_NAME}" = "AIX" ]; then
            tempUid=`${AGENT_ROOT_PATH}/sbin/xmlcfg read Backup rdadmin_uid`
            useradd -m -d ${DEFAULT_HOME_DIR} -g ${AGENT_GROUP} -u ${tempUid} ${AGENT_USER} >/dev/null 2>&1
        else
            if [ ${BACKUP_SCENE} -eq ${BACKUP_SCENE_INTERNAL} ]; then
                useradd -m -s ${USER_NOLOGIN_SHELL} -u 1000 -g 99 -G ${AGENT_GROUP} ${AGENT_USER}
                if [ $? -ne 0 ]; then
                    echo "Add user fail."
                    Log "Add user fail."
                    exit 1
                else
                    chage -M -1 ${AGENT_USER}
                    Log "Add user succ."
                    return
                fi
            elif [ "${BACKUP_ROLE}" = "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ]; then
                sanclientUid=`${AGENT_ROOT_PATH}/sbin/xmlcfg read Backup sanclient_uid`
                useradd -m -s ${USER_NOLOGIN_SHELL} -g ${AGENT_GROUP} -u ${sanclientUid} ${AGENT_USER}
            else
                tempUid=`${AGENT_ROOT_PATH}/sbin/xmlcfg read Backup rdadmin_uid`
                useradd -m -s ${USER_NOLOGIN_SHELL} -g ${AGENT_GROUP} -u ${tempUid} ${AGENT_USER}
            fi
        fi
        
        if [ 0 -ne $? ]; then
            echo "The DataBackup ProtectAgent working user ${AGENT_USER} was added failed."
            echo "The installation of DataBackup ProtectAgent will be stopped."
            LogError "The DataBackup ProtectAgent working user ${AGENT_USER} has already existed, please delete it before install." ${ERR_UPGRADE_FAIL_ADD_USER}
            exit $ERR_WORKINGUSER_EXISTS
        fi

        if [ "${SYS_NAME}" = "SunOS" ]; then
            passwd -x -1 ${AGENT_USER}
        elif [ "${SYS_NAME}" = "AIX" ]; then
            chuser maxage=0 ${AGENT_USER}
        else
            chage -M -1 ${AGENT_USER}
        fi

        if [ 0 -ne $? ]; then
            echo "The passward of DataBackup ProtectAgent working user ${AGENT_USER} set unexpired unsuccessfully, please set manually."
            Log "The passward of DataBackup ProtectAgent working user ${AGENT_USER} set unexpired unsuccessfully, please set manually."
        fi
    else
        echo "The DataBackup ProtectAgent working user ${AGENT_USER} has already existed, please delete it before install."
        LogError "The DataBackup ProtectAgent working user ${AGENT_USER} has already existed, please delete it before install." ${ERR_UPGRADE_FAIL_ADD_USER}
        exit $ERR_WORKINGUSER_EXISTS
    fi
    
    if [ "" = "${GROUP_CHECK}" ]; then
        if [ "${SYS_NAME}" = "HP-UX" ]; then
            HP_OS_VERSION=`uname -a | $AWK '{print $3}' | $AWK -F "." '{print $2"."$3}'`
            if [ "${HP_OS_VERSION}" = "11.31" ]; then
                usermod -F -g ${AGENT_GROUP} ${AGENT_USER} 1>/dev/null 2>>${LOG_FILE_NAME}
            else
                usermod -g ${AGENT_GROUP} ${AGENT_USER} 1>/dev/null 2>>${LOG_FILE_NAME}
            fi
        else
            usermod -g ${AGENT_GROUP} ${AGENT_USER} 1>/dev/null 2>>${LOG_FILE_NAME}
        fi
        
        if [ 0 -ne $? ]; then
            echo "Add Agent working user ${AGENT_USER} to group ${AGENT_GROUP} failed."
            echo "The installation of DataBackup ProtectAgent will be stopped."
            Log "Add Agent working user ${AGENT_USER} to group ${AGENT_GROUP} failed."
            LogError "Add Agent working user ${AGENT_USER} to group ${AGENT_GROUP} failed." ${ERR_UPGRADE_FAIL_ADD_USERGROUP}
            exit $ERR_WORKINGUSER_ADD_FAILED_RETCODE
        fi
    else
        if [ "${SYS_NAME}" = "SunOS" ]; then
            TMP_GROUP=`/usr/xpg4/bin/id -g -n ${AGENT_GROUP}`
        else
            TMP_GROUP=`id -g -n ${AGENT_GROUP}`
        fi
        if [ "${TMP_GROUP}" != "${AGENT_GROUP}" ]; then 
            if [ "${SYS_NAME}" = "HP-UX" ]; then
                HP_OS_VERSION=`uname -a | $AWK '{print $3}' | $AWK -F "." '{print $2"."$3}'`
                if [ "${HP_OS_VERSION}" = "11.31" ]; then
                    usermod -F -g ${AGENT_GROUP} ${AGENT_USER} 1>/dev/null 2>>${LOG_FILE_NAME}
                else
                    usermod -g ${AGENT_GROUP} ${AGENT_USER} 1>/dev/null 2>>${LOG_FILE_NAME}
                fi
            else
                usermod -g ${AGENT_GROUP} ${AGENT_USER} 1>/dev/null 2>>${LOG_FILE_NAME}
            fi
        
            if [ 0 -ne $? ]; then
                echo "Add agent working user ${AGENT_USER} to group ${AGENT_GROUP} failed."
                echo "The installation of DataBackup ProtectAgent will be stopped."
                LogError "Add agent working user ${AGENT_USER} to group ${AGENT_GROUP} failed." ${ERR_UPGRADE_FAIL_ADD_USERGROUP}
                exit $ERR_WORKINGUSER_ADD_FAILED_RETCODE
            fi
        fi
    fi 
}

AddRdadminToNobodyGroup()
{
    # oracle/vmware do not need
    if [ ${BACKUP_ROLE} -eq ${BACKUP_ROLE_VMWARE} ] || [ ${BACKUP_ROLE} -eq ${BACKUP_ROLE_HOST} ]; then
        return 0
    fi
    tempStr=`cat /etc/group | grep -E ":\!:99:${AGENT_USER}|:x:99:${AGENT_USER}"`
    if [ $? -eq 0 ] && [ -n "${tempStr}" ]; then
        Log "User rdadmin has been added to the 99 group."
        return 0
    fi
    tempStr=`cat /etc/group | grep -E ":\!:99:|:x:99:"`
    if [ $? -ne 0 -o -z "${tempStr}" ]; then
        grepStr=`cat /etc/group | grep "^nobody:"`
        if [ $? -ne 0 -o -z "${grepStr}" ]; then
            groupadd -g 99 ${GROUP_NOBODY} 1>/dev/null 2>>${LOG_FILE_NAME}
            if [ $? -ne 0 ]; then
                echo "Add nobody group failed."
                Log "Add nobody group failed."
                exit 1
            fi
        else
            timestamp=`date +%y%m%d`
            groupUser="OP""${timestamp}"
            if [ "${SYS_NAME}" = "AIX" ]; then
                mkgroup ${groupUser} 1>/dev/null 2>>${LOG_FILE_NAME}
                if [ $? -ne 0 ]; then
                    echo "Add ${groupUser} group failed."
                    Log "Add ${groupUser} group failed."
                    exit 1
                fi
                chgroup id=99 ${groupUser} 1>/dev/null 2>>${LOG_FILE_NAME}
                if [ $? -ne 0 ]; then
                    echo "Failed to set the ${groupUser} group ID."
                    Log "Failed to set the ${groupUser} group ID."
                    exit 1
                fi
            else
                groupadd -g 99 ${groupUser} 1>/dev/null 2>>${LOG_FILE_NAME}
            fi
            if [ $? -ne 0 ]; then
                echo "Add ${groupUser} group failed."
                Log "Add ${groupUser} group failed."
                exit 1
            fi
        fi
    fi
    groupStr=`cat /etc/group | grep -E ":\!:99:|:x:99:"`
    groupName=${groupStr%%:*}
    if [ "${SYS_NAME}" = "AIX" ]; then
        usermod -G ${groupName} ${AGENT_USER} 1>/dev/null 2>>${LOG_FILE_NAME}
    elif [ -f "/etc/SuSE-release" ]; then
        if [ "11" = "`cat /etc/SuSE-release 2>/dev/null | ${AWK}  '$2 == "=" {print $3}' | head -1`" ]; then
            # suse 11
            usermod -G ${groupName} ${AGENT_USER} 1>/dev/null 2>>${LOG_FILE_NAME}
        else
            usermod -a -G ${groupName} ${AGENT_USER} 1>/dev/null 2>>${LOG_FILE_NAME}
        fi
    else
        usermod -a -G ${groupName} ${AGENT_USER} 1>/dev/null 2>>${LOG_FILE_NAME}
    fi
    if [ $? -ne 0 ]; then
        echo "Failed to add ${AGENT_USER} to ${groupName}."
        Log "Failed to add ${AGENT_USER} to ${groupName}."
        exit 1
    fi
}

KillDataProcess()
{
    # Stop the dataprocess process.
    if [ "$SYS_NAME" = "Linux" ]; then
        dataprocess_pid=`ps -afx 2>/dev/null | grep dataprocess | grep -v grep | $AWK '{print $1}'` &>/dev/null
        if [ -n "$dataprocess_pid" ]; then
            Log "Send kill to dataprocess, call: kill -9 $dataprocess_pid"
            kill -9 $dataprocess_pid
        fi
    fi
}

SUExecCmd()
{
    cmd=$1
    if [ "${SYS_NAME}" = "SunOS" ] || [ "${SYS_NAME}" = "AIX" ]; then
        su - ${AGENT_USER} -c "${EXPORT_ENV}${cmd}" >>${LOG_FILE_NAME} 2>&1
    else
        su - ${AGENT_USER} -s ${SHELL_TYPE_SH} -c "${EXPORT_ENV}${cmd}" >>${LOG_FILE_NAME} 2>&1
    fi

    return $?
}

SUExecCmdWithOutput()
{
    cmd=$1
    if [ "${SYS_NAME}" = "SunOS" ] || [ "${SYS_NAME}" = "AIX" ]; then
        output=`su - ${AGENT_USER} -c "${EXPORT_ENV}${cmd}"` 
        ret=$?
    else
        output=`su -m ${AGENT_USER} -s ${SHELL_TYPE_SH} -c "${EXPORT_ENV}${cmd}"`
        ret=$?
        if [ $ret -ne 0 ]; then
            output=`su -p ${AGENT_USER} -s ${SHELL_TYPE_SH} -c "${EXPORT_ENV}${cmd}"`
            ret=$?
        fi
    fi
    
    VerifySpecialChar "$output"
    echo $output
    return $ret
}

ChangeDirPrivilege()
{
    if [ "${AGENT_ROOT_TMP}" != "${INSTALL_DIR_UPPER}" ]; then
        CHMOD o+rx "${AGENT_ROOT_TMP}"
        CHMOD 550 "${AGENT_ROOT_TMP}"
        chown -h root:${AGENT_GROUP} "${AGENT_ROOT_TMP}"
        AGENT_ROOT_TMP=`dirname $AGENT_ROOT_TMP`
        ChangeDirPrivilege
    fi
}

ChangePrivilege()
{
    group=$1
    if [ "AIX" = "$SYS_NAME" ];then
        ROOT_GROUP=system
    else
        ROOT_GROUP=root
    fi
    
    chown -h -R ${AGENT_USER}:${AGENT_GROUP} "${AGENT_ROOT_PATH}/"*
    chown -h -R root:${AGENT_GROUP} "${AGENT_ROOT_PATH}/"sbin

    AGENT_ROOT_TMP=${AGENT_ROOT_PATH}
    ChangeDirPrivilege

    CHMOD 555 "${AGENT_ROOT_PATH}"
    CHMOD 555 "${AGENT_ROOT_PATH}/../"

    ############# change dir [bin] ###########################
    chmod -R 500 "${AGENT_ROOT_PATH}/bin"
    find "${AGENT_ROOT_PATH}/bin" -name "*.so" | xargs chown -h root:$group
    find "${AGENT_ROOT_PATH}/bin" -name "*.so" | xargs chmod 550
    find "${AGENT_ROOT_PATH}/bin" -name "*.so.*" | xargs chown -h root:$group
    find "${AGENT_ROOT_PATH}/bin" -name "*.so.*" | xargs chmod 550
    find "${AGENT_ROOT_PATH}/bin" -type d | xargs chown root:$group
    find "${AGENT_ROOT_PATH}/bin" -type d | xargs chmod 550

    SUExecCmd "chmod -R 700 ${AGENT_ROOT_PATH}/nginx"
    SUExecCmd "chmod 500 ${AGENT_ROOT_PATH}/nginx/rdnginx"
    SUExecCmd "chmod 600 ${AGENT_ROOT_PATH}/nginx/conf/"*

    NGINX_LOGS=`ls "${AGENT_ROOT_PATH}/nginx/logs/"`
    if [ "${NGINX_LOGS}" != "" ]; then
        SUExecCmd "chmod 600 ${AGENT_ROOT_PATH}/nginx/logs/"*
    fi

    [ -f "${AGENT_ROOT_PATH}/nginx/conf/server.key" ] && SUExecCmd "chmod 400 ${AGENT_ROOT_PATH}/nginx/conf/server.key"
    [ -f "${AGENT_ROOT_PATH}/nginx/conf/kmc_store_bak.txt" ] && SUExecCmd "chmod 600 ${AGENT_ROOT_PATH}/nginx/conf/kmc_store_bak.txt"

    ############# change dir [sbin] ###########################
    chown -h -R root:${AGENT_GROUP} "${AGENT_ROOT_PATH}/"sbin
    chmod -R 550 "${AGENT_ROOT_PATH}/sbin"
    chown -h root:${ROOT_GROUP} "${AGENT_ROOT_PATH}/"sbin/*
    chmod 500 "${AGENT_ROOT_PATH}/sbin"/*
    chown -h root:${AGENT_GROUP} "${AGENT_ROOT_PATH}/sbin/rootexec"
    CHMOD 550 "${AGENT_ROOT_PATH}/sbin/rootexec"
    CHMOD +s "${AGENT_ROOT_PATH}/sbin/rootexec"
    chown -h root:${AGENT_GROUP} "${AGENT_ROOT_PATH}/sbin/xmlcfg"
    CHMOD 550 "${AGENT_ROOT_PATH}/sbin/xmlcfg"
    chown -h root:${group} "${AGENT_ROOT_PATH}/bin/monitor"
    CHMOD 550 "${AGENT_ROOT_PATH}/bin/monitor"
    chown -h root:${group} "${AGENT_ROOT_PATH}/bin/openssl"
    CHMOD 550 "${AGENT_ROOT_PATH}/bin/openssl"
    chown -h root:${group} "${AGENT_ROOT_PATH}/bin/rdagent"
    CHMOD 550 "${AGENT_ROOT_PATH}/bin/rdagent"
    chown -h root:${group} "${AGENT_ROOT_PATH}/bin/sqlite3"
    CHMOD 550 "${AGENT_ROOT_PATH}/bin/sqlite3"
    chown -h root:${group} "${AGENT_ROOT_PATH}/bin/xmlcfg"
    CHMOD 550 "${AGENT_ROOT_PATH}/bin/xmlcfg"
    chown -h root:${group} "${AGENT_ROOT_PATH}/bin/agentcli"
    CHMOD 550 "${AGENT_ROOT_PATH}/bin/agentcli"
    if [ -f "${AGENT_ROOT_PATH}/bin/dataprocess" ]; then
        chown -h root:${group} "${AGENT_ROOT_PATH}/bin/dataprocess"
        CHMOD 550 "${AGENT_ROOT_PATH}/bin/dataprocess"
    fi

    ############# change dir [slog] ###########################
    chown -h -R root:${ROOT_GROUP} "${AGENT_ROOT_PATH}/slog"
    CHMOD 700 "${AGENT_ROOT_PATH}/slog"

    ############# change dir [log] ###########################
    SUExecCmd "chmod 700 ${AGENT_ROOT_PATH}/log"
    [ -d "${AGENT_ROOT_PATH}/log/Plugins/" ] && SUExecCmd "chmod 700 -R ${AGENT_ROOT_PATH}/log/Plugins/"

    ############# change dir [tmp] ###########################
    CHMOD 700 "${AGENT_ROOT_PATH}/tmp"
    CHMOD o+t "${AGENT_ROOT_PATH}/tmp"

    ############# change dir [stmp] ###########################
    chown -h root:${ROOT_GROUP} "${AGENT_ROOT_PATH}/stmp"
    CHMOD 705 "${AGENT_ROOT_PATH}/stmp"

    ############# change dir [lib] ###########################
    chown -h root:${ROOT_GROUP} "${AGENT_ROOT_PATH}/lib"
    CHMOD 700 "${AGENT_ROOT_PATH}/lib"

    ############# change dir [conf] ###########################
    SUExecCmd "chmod 705 -R ${AGENT_ROOT_PATH}/conf"
    [ -f "${AGENT_ROOT_PATH}/conf/HostSN" ] && SUExecCmd 600 "${AGENT_ROOT_PATH}/conf/HostSN"
    [ -f "${AGENT_ROOT_PATH}/conf/pluginmgr.xml" ] && SUExecCmd "chmod 600 ${AGENT_ROOT_PATH}/conf/pluginmgr.xml"
    [ -f "${AGENT_ROOT_PATH}/conf/agent_cfg.xml" ] && SUExecCmd "chmod 600 ${AGENT_ROOT_PATH}/conf/agent_cfg.xml"
    [ -f "${AGENT_ROOT_PATH}/conf/alarm_info.xml" ] && SUExecCmd "chmod 600 ${AGENT_ROOT_PATH}/conf/alarm_info.xml"
    [ -f "${AGENT_ROOT_PATH}/conf/openssl.cnf" ] && SUExecCmd "chmod 600 ${AGENT_ROOT_PATH}/conf/openssl.cnf"
    [ -f "${AGENT_ROOT_PATH}/conf/package.json" ] && SUExecCmd "chmod 600 ${AGENT_ROOT_PATH}/conf/package.json"
    [ -f "${AGENT_ROOT_PATH}/conf/vddk.cfg" ] && SUExecCmd "chmod 600 ${AGENT_ROOT_PATH}/conf/vddk.cfg"
    [ -f "${AGENT_ROOT_PATH}/conf/script.sig" ] && SUExecCmd "chmod 400 ${AGENT_ROOT_PATH}/conf/script.sig"
    [ -f "${AGENT_ROOT_PATH}/conf/version" ] && SUExecCmd "chmod 400 ${AGENT_ROOT_PATH}/conf/version"
    [ -f "${AGENT_ROOT_PATH}/conf/kmc_config.txt" ] && SUExecCmd "chmod 600 ${AGENT_ROOT_PATH}/conf/kmc_config.txt"
    [ -f "${AGENT_ROOT_PATH}/conf/kmc_store.txt" ] && SUExecCmd "chmod 600 ${AGENT_ROOT_PATH}/conf/kmc_store.txt"
    [ -f "${AGENT_ROOT_PATH}/conf/kmc/agentcli_config.txt" ] && SUExecCmd "chmod 600 ${AGENT_ROOT_PATH}/conf/kmc/agentcli_config.txt"
    [ -f "${AGENT_ROOT_PATH}/conf/kmc/agentcli_config_bak.txt" ] && SUExecCmd "chmod 600 ${AGENT_ROOT_PATH}/conf/kmc/agentcli_config_bak.txt"
    [ -f "${AGENT_ROOT_PATH}/conf/kmc/agentcli_store.txt" ] && SUExecCmd "chmod 600 ${AGENT_ROOT_PATH}/conf/kmc/agentcli_store.txt"
    [ -f "${AGENT_ROOT_PATH}/conf/kmc/agentcli_stroe_bak.txt" ] && SUExecCmd "chmod 600 ${AGENT_ROOT_PATH}/conf/kmc/agentcli_stroe_bak.txt"
    [ -d "${AGENT_ROOT_PATH}/conf/thrift" ] && SUExecCmd "chmod 705 ${AGENT_ROOT_PATH}/conf/thrift"
    [ -d "${AGENT_ROOT_PATH}/conf/thrift/server" ] && SUExecCmd "chmod 700 ${AGENT_ROOT_PATH}/conf/thrift/server"
    [ -d "${AGENT_ROOT_PATH}/conf/thrift/server" ] && SUExecCmd "chmod 600 ${AGENT_ROOT_PATH}/conf/thrift/server/*"
    [ -d "${AGENT_ROOT_PATH}/conf/thrift/client" ] && SUExecCmd "chmod 700 ${AGENT_ROOT_PATH}/conf/thrift/client"
    [ -d "${AGENT_ROOT_PATH}/conf/thrift/client" ] && SUExecCmd "chmod 600 ${AGENT_ROOT_PATH}/conf/thrift/client/*"

    SUExecCmd "chmod 550 \"${INSTALL_PATH}/ProtectClient-E/Open Source Software Notice.doc\""
    
    ############# change dir [db] ###########################
    SUExecCmd "chmod 700 ${AGENT_ROOT_PATH}/db"
    SUExecCmd "chmod 700 ${AGENT_ROOT_PATH}/db/upgrade"
    SUExecCmd "chmod 640 ${AGENT_ROOT_PATH}/db/upgrade/*"
    SUExecCmd "chmod 600 ${AGENT_ROOT_PATH}/db/AgentDB.db"
    SUExecCmd "chmod 640 ${AGENT_ROOT_PATH}/ProtectClient-E/db/upgrade/*"
    [ -d ${AGENT_ROOT_PATH}/db ]  && chmod 600 ${AGENT_ROOT_PATH}/db/*.db

    ############# change dir [upgrade] ###########################
    chown -R root:root "${AGENT_ROOT_PATH}/upgrade"
    CHMOD -R 400 "${AGENT_ROOT_PATH}/upgrade"
    chown root:${AGENT_USER} "${AGENT_ROOT_PATH}/stmp/upgrade_signature.sign"
    CHMOD 660 "${AGENT_ROOT_PATH}/stmp/upgrade_signature.sign"
}

CheckEnvConfig()
{
    SHELL_TYPE=`cat /etc/passwd | grep "^${AGENT_USER}:" | $AWK -F "/" '{print $NF}'`
    if [ "${SHELL_TYPE}" = "bash" ] || [ "${SHELL_TYPE}" = "sh" ]; then
        if [ "${SysRockyFlag}" != "" ] || [ "${SYS_NAME}" = "AIX" ] || [ "${SYS_NAME}" = "HP-UX" ] || [ "${SYS_NAME}" = "SunOS" ]; then
            ENV_FILE=".profile"
        fi
    elif [ "${SHELL_TYPE}" = "ksh" ]; then
        ENV_FILE=".profile"
    elif [ "${SHELL_TYPE}" = "csh" ]; then
        ENV_FILE=".cshrc"
        ENV_EFFECT="setenv"
        ENV_FLAG=" "
    elif [ "${SHELL_TYPE}" = "nologin" ]; then
        ENV_FILE=".profile"
    else
        echo "The shell type bash, ksh and csh only be supported in user ${AGENT_USER} by DataBackup ProtectAgent."
        echo "The installation of DataBackup ProtectAgent will be stopped."
        Log "The shell type bash, ksh and csh only be supported in user ${AGENT_USER} by DataBackup ProtectAgent."
        exit $ERR_ENV_SET_FAILED
    fi

    if [ "${SysRockyFlag}" = "" ]; then
        ECHO_E=-e
    fi

    if [ "${SYS_NAME}" = "AIX" ] || [ "${SYS_NAME}" = "HP-UX" ] || [ "${SYS_NAME}" = "SunOS" ] || [ "${ROCKY4_FLAG}" = "1" ]; then
        ECHO_E=""
        if [ "${SHELL_TYPE}" = "bash" ]; then
            UNIX_CMD=-l
        fi
    fi
}

# $1: /bin:/sbin:/usr/sbin:/opt/VRTS/bin:/opt/VRTSvcs/bin
SetPathEnv()
{
    if [ "${SYS_NAME}" = "SunOS" ] || [ "${SYS_NAME}" = "AIX" ]; then
        ENV_VAR_CHECK=`su - ${AGENT_USER} -c "${EXPORT_ENV}env" | grep "^PATH=" | grep -w "$1"`
    else
        ENV_VAR_CHECK=`su - ${AGENT_USER} -s ${SHELL_TYPE_SH} ${UNIX_CMD} -c "${EXPORT_ENV}env" | grep "^PATH=" | grep -w "$1"`
    fi

    if [ "" = "${ENV_VAR_CHECK}" ]; then
        if [ -d "$1" ]; then
            ENV_PATH=${ENV_PATH}:$1
        fi
    fi
}

CheckRdadminUserEnv()
{
    CheckEnvConfig
    if [ "${SYS_NAME}" = "SunOS" ]; then
        SUExecCmd "touch .hushlogin"
    fi

    RDADMIN_ENV_FILE="${DEFAULT_HOME_DIR}/${ENV_FILE}"
    if [ ! -f "$RDADMIN_ENV_FILE" ]; then
        SUExecCmd "touch $RDADMIN_ENV_FILE"
        SUExecCmd "chown -h ${AGENT_USER}:${AGENT_GROUP} ${RDADMIN_ENV_FILE}"
        SUExecCmd "chmod 700 ${RDADMIN_ENV_FILE}"
        Log "User env file is not exist, touch ${RDADMIN_ENV_FILE}."
    fi

    AGENT_ROOT_OLD=`SUExecCmdWithOutput "cat ${RDADMIN_ENV_FILE} | grep \"${ENV_EFFECT} AGENT_ROOT${ENV_FLAG}\""`
    AGENT_ROOT_OLD_LINENO=`SUExecCmdWithOutput "cat ${RDADMIN_ENV_FILE} | grep -n \"${ENV_EFFECT} AGENT_ROOT${ENV_FLAG}\" | $AWK -F ':' '{print \\$1}'"`
    
    if [ "" != "$AGENT_ROOT_OLD" ]; then
        Log "Environment variable AGENT_ROOT ${AGENT_ROOT_OLD} is exist, then covery it with ${AGENT_ROOT_PATH}."
        SUExecCmd "sed '${AGENT_ROOT_OLD_LINENO}d' ${RDADMIN_ENV_FILE} > ${RDADMIN_ENV_FILE}.bak"
        if [ "${SYS_NAME}" = "SunOS" ] || [ "${SYS_NAME}" = "AIX" ]; then
            su - ${AGENT_USER} -c "mv -f ${RDADMIN_ENV_FILE}.bak ${RDADMIN_ENV_FILE}" >/dev/null
        else
            su - ${AGENT_USER} -s ${SHELL_TYPE_SH} -c "mv -f \"${RDADMIN_ENV_FILE}\".bak \"${RDADMIN_ENV_FILE}\""
        fi
    else
        Log "Environment variable AGENT_ROOT is not exist, then set AGENT_ROOT=${AGENT_ROOT_PATH}."
    fi
    if [ "${SYS_NAME}" = "SunOS" ]; then
        SUExecCmd "echo AGENT_ROOT${ENV_FLAG}${AGENT_ROOT_PATH} >> ${RDADMIN_ENV_FILE}"
        SUExecCmd "echo ${ENV_EFFECT} AGENT_ROOT >> ${RDADMIN_ENV_FILE}"
        SUExecCmd "echo DATA_BACKUP_AGENT_HOME=$DATA_BACKUP_AGENT_HOME >> ${RDADMIN_ENV_FILE}"
        SUExecCmd "echo ${ENV_EFFECT} DATA_BACKUP_AGENT_HOME >> ${RDADMIN_ENV_FILE}"
    else
        SUExecCmd "echo ${ENV_EFFECT} AGENT_ROOT${ENV_FLAG}${AGENT_ROOT_PATH} >> ${RDADMIN_ENV_FILE}"
        SUExecCmd "echo ${ENV_EFFECT} DATA_BACKUP_AGENT_HOME=$DATA_BACKUP_AGENT_HOME >> ${RDADMIN_ENV_FILE}"
    fi
    if [ "AIX" = "${SYS_NAME}" ]; then
        LIBPATH_OLD=`SUExecCmdWithOutput "cat ${RDADMIN_ENV_FILE} | grep \"${ENV_EFFECT} LIBPATH${ENV_FLAG}\""`
        LIBPATH_OLD_LINENO=`SUExecCmdWithOutput "cat ${RDADMIN_ENV_FILE} | grep -n \"${ENV_EFFECT} LIBPATH${ENV_FLAG}\" | $AWK -F ':' '{print \\$1}'"`
    
        if [ "" != "$LIBPATH_OLD" ]; then
            Log "Environment variable LIBPATH ${LIBPATH_OLD} is exist, then covery it with ${LIBPATH}."
            SUExecCmd "sed  '${LIBPATH_OLD_LINENO}d' ${RDADMIN_ENV_FILE} > ${RDADMIN_ENV_FILE}.bak"
            SUExecCmd "mv ${RDADMIN_ENV_FILE}.bak ${RDADMIN_ENV_FILE}"
        else
            Log "Environment variable LIBPATH is not exist, then set LIBPATH=${LIBPATH}."
        fi
        SUExecCmd "echo ${ENV_EFFECT} LIBPATH${ENV_FLAG}${LIBPATH} >> ${RDADMIN_ENV_FILE}"
    else
        LD_LIBRARY_PATH_OLD=`SUExecCmdWithOutput "cat ${RDADMIN_ENV_FILE} | grep \"${ENV_EFFECT} LD_LIBRARY_PATH${ENV_FLAG}\""`
        LD_LIBRARY_PATH_OLD_LINENO=`SUExecCmdWithOutput "cat ${RDADMIN_ENV_FILE} | grep -n \"${ENV_EFFECT} LD_LIBRARY_PATH${ENV_FLAG}\" | $AWK -F ':' '{print \\$1}'"`
    
        if [ "" != "$LD_LIBRARY_PATH_OLD" ]; then
            Log "Environment variable LD_LIBRARY_PATH ${LD_LIBRARY_PATH_OLD} is exist, then covery it with ${LD_LIBRARY_PATH}."
            SUExecCmd "sed  '${LD_LIBRARY_PATH_OLD_LINENO}d' ${RDADMIN_ENV_FILE} > ${RDADMIN_ENV_FILE}.bak"
            if [ "${SYS_NAME}" = "SunOS" ] || [ "${SYS_NAME}" = "AIX" ]; then
                su - ${AGENT_USER} -c "mv -f ${RDADMIN_ENV_FILE}.bak ${RDADMIN_ENV_FILE}" >/dev/null
            else
                su - ${AGENT_USER} -s ${SHELL_TYPE_SH} -c "mv -f \"${RDADMIN_ENV_FILE}\".bak \"${RDADMIN_ENV_FILE}\"" 
            fi
        else
            Log "Environment variable LD_LIBRARY_PATH is not exist, then set LD_LIBRARY_PATH=${AGENT_USER_LD_LIBRARY_PATH}."
        fi
        if [ "${SYS_NAME}" = "SunOS" ]; then
            SUExecCmd "echo LD_LIBRARY_PATH${ENV_FLAG}${AGENT_USER_LD_LIBRARY_PATH} >> ${RDADMIN_ENV_FILE}"
            SUExecCmd "echo ${ENV_EFFECT} LD_LIBRARY_PATH >> ${RDADMIN_ENV_FILE}"
        else
            SUExecCmd "echo ${ENV_EFFECT} LD_LIBRARY_PATH${ENV_FLAG}${AGENT_USER_LD_LIBRARY_PATH} >> ${RDADMIN_ENV_FILE}"
        fi
    fi

    # export SAN_OPTIONS
    if [ ${SYS_NAME} = "Linux" ]; then
        SUExecCmd "echo ${ENV_EFFECT} ASAN_OPTIONS=${ASAN_OPTIONS} >> ${RDADMIN_ENV_FILE}"
        SUExecCmd "echo ${ENV_EFFECT} UBSAN_OPTIONS=${UBSAN_OPTIONS} >> ${RDADMIN_ENV_FILE}"
        SUExecCmd "echo ${ENV_EFFECT} TSAN_OPTIONS=${TSAN_OPTIONS} >> ${RDADMIN_ENV_FILE}"
    fi

    SUExecCmd "mkdir -p /home/rdadmin/logs"
    SUExecCmd "chown ${AGENT_USER}:${AGENT_USER} /home/rdadmin/logs"
    SUExecCmd "chmod 700 /home/rdadmin/logs"
    
    # set PATH: /bin:/sbin:/usr/sbin:/opt/VRTS/bin:/opt/VRTSvcs/bin
    ENV_PATH='.:${PATH}'
    SetPathEnv "/bin"
    SetPathEnv "/sbin"
    SetPathEnv "/usr/sbin"
    SetPathEnv "/opt/VRTS/bin"
    SetPathEnv "/opt/VRTSvcs/bin"

    if [ ${ENV_PATH} != '.:${PATH}' ]; then
        if [ "${SYS_NAME}" = "SunOS" ]; then
            SUExecCmd "echo PATH${ENV_FLAG}${ENV_PATH} >> ${RDADMIN_ENV_FILE}"
            SUExecCmd "echo ${ENV_EFFECT} PATH >> ${RDADMIN_ENV_FILE}"
        else
            SUExecCmd "echo ${ENV_EFFECT} PATH${ENV_FLAG}${ENV_PATH} >> ${RDADMIN_ENV_FILE}"
        fi
    fi

    SUExecCmd "chown -h ${AGENT_USER}:${AGENT_GROUP} ${RDADMIN_ENV_FILE}"
    SUExecCmd "chmod 700 ${RDADMIN_ENV_FILE}"
}

CheckCommandInjection()
{
    expression='[|;&$><`\!]+'
    if [ "${SYS_NAME}" = "AIX" ]; then
        echo "$1" | grep -E "${expression}"
        if [ $? -eq 0 ]; then
            Log "The param cannot contain special character(${expression})."
            return 1
        fi
    elif [ "${SYS_NAME}" = "SunOS" ]; then
        expression='[|;&$><\`\!]'
        echo "$1" | grep "${expression}"
        if [ $? -eq 0 ]; then
            Log "The param cannot contain special character(${expression})."
            return 1
        fi
    elif [ "$1" =~ "${expression}" ]; then
        Log "The param cannot contain special character(${expression})."
        return 1 
    fi
    return 0
}

KillSonPids()
{
	pids=`ps -ef | ${MYAWK} '{if($3=='$1'){print $2} }'`;
        kill -9 $1 2>/dev/null
	if [ -n "$pids" ]; then
		for pid in $pids
		do
		    KillSonPids $pid
		done
	fi
}

# Command timeout function
Timeout()
{
    waitsec=30
    "$@" & pid=$!
    `sleep $waitsec && KillSonPids $pid` & watchdog=$!
    if wait $pid 2>/dev/null; then
        KillSonPids $watchdog
        wait $watchdog 2>/dev/null
        return 0
    else
        status=$?
        pid_exit=`ps -ef | ${MYAWK} '{print $2}'| grep -w $pid`
        if [ ! $pid_exit ]; then
            KillSonPids $watchdog
            wait $watchdog 2>/dev/null
        fi
        return $status
    fi
}

# Check os version
CheckOsType()
{
    CHECK_RESULT=1    # 0-success 1-fail
    
    if [ "$1" = "vmware" ]; then
        if [ "${SYS_NAME}" = "Linux" ] && [ -f "/etc/centos-release" ]; then
            CHECK_RESULT=0
        fi
    elif [ "$1" = "oracle" ]; then
        if [ "${SYS_NAME}" = "Linux" ]; then
            if [ -f "/etc/redhat-release" ] && [ ! -f "/etc/centos-release" ]; then
                CHECK_RESULT=0
            fi
        elif [ "${SYS_NAME}" = "AIX" ] || [ "${SYS_NAME}" = "SunOS" ]; then
            CHECK_RESULT=0
        fi
    fi

    if [ ${CHECK_RESULT} -ne 0 ]; then
        ShowWarning "Warning: The current operating system is not in the support list."
        Log "The current operating system is not in the support list."
    fi
}

# Check disk space
CheckFreeDiskSpace()
{
    FREE_SPACE_MIN_VMWARE=1024000
    FREE_SPACE_MIN_DEFAULT=1024000
    FREE_SPACE_MIN_VMWARE_MB=1000
    FREE_SPACE_MIN_DEFAULT_MB=1000

    FREE_SPACE_MIN=
    FREE_SPACE_MIN_MB=
    FREE_SPACE_MB=

    if [ "$1" = "vmware" ]; then
        FREE_SPACE_MIN=${FREE_SPACE_MIN_VMWARE}
        FREE_SPACE_MIN_MB=${FREE_SPACE_MIN_VMWARE_MB}
    else
        FREE_SPACE_MIN=${FREE_SPACE_MIN_DEFAULT}
        FREE_SPACE_MIN_MB=${FREE_SPACE_MIN_DEFAULT_MB}
    fi

    if [ -n "$DATA_BACKUP_AGENT_HOME" ]; then
        CHECK_DIR=${DATA_BACKUP_AGENT_HOME}
    fi

    if [ "${SYS_NAME}" = "Linux" ]; then
        Timeout df -k ${CHECK_DIR} >/dev/null 2>&1
        if [ $? -eq 0 ]; then
            FREE_SPACE=`df -k ${CHECK_DIR} | grep -n '' | ${MYAWK} 'END{print $4}'`
        else
            ShowWarning "Warning:Checking remaining space failed, please verify manually."
            Log "Checking remaining space failed, please verify manually."
            return 1
        fi
    elif [ "${SYS_NAME}" = "AIX" ]; then
        FREE_SPACE=`df -k ${CHECK_DIR} | grep -n '' | ${MYAWK} '{print $3}' | sed -n '2p'`
    elif [ "${SYS_NAME}" = "HP-UX" ]; then
        FREE_SPACE=`df -k ${CHECK_DIR} | grep -w 'free' | ${MYAWK} '{print $1}'`
    elif [ "${SYS_NAME}" = "SunOS" ]; then
        FREE_SPACE=`df -k ${CHECK_DIR} | ${MYAWK} '{print $4}' | sed -n '2p'`
    fi
    FREE_SPACE_MB=`expr ${FREE_SPACE} / 1024`

    if [ ${FREE_SPACE_MIN} -gt ${FREE_SPACE} ]; then
        ShowWarning "Warning: The available space (${FREE_SPACE_MB}MB) in the installation path is less than the required minimum space (${FREE_SPACE_MIN_MB}MB)."
        Log "The available space (${FREE_SPACE_MB}MB) in the installation path is less than the required minimum space (${FREE_SPACE_MIN_MB}MB)."
    fi
}

# Check cpu cores and memory
CheckCpuAndMem()
{
    CPU_CORES_VMWARE=8
    CPU_CORES_DEFAULT=4
    HOST_MEM_VMWARE=16777216
    HOST_MEM_DEFAULT=4194304
    HOST_MEM_VMWARE_GB=16GB
    HOST_MEM_DEFAULT_GB=4GB
    CPU_CORES_MIN=
    HOST_MEM_MIN=
    HOST_MEM_MIN_GB=

    if [ "$1" = "vmware" ]; then
        CPU_CORES_MIN=${CPU_CORES_VMWARE}
        HOST_MEM_MIN=${HOST_MEM_VMWARE}
        HOST_MEM_MIN_GB=${HOST_MEM_VMWARE_GB}
    else
        CPU_CORES_MIN=${CPU_CORES_DEFAULT}
        HOST_MEM_MIN=${HOST_MEM_DEFAULT}
        HOST_MEM_MIN_GB=${HOST_MEM_DEFAULT_GB}
    fi

    if [ "${SYS_NAME}" = "Linux" ]; then
        CPU_CORES=`cat /proc/cpuinfo| grep "processor"| wc -l`
        HOST_MEM=`cat /proc/meminfo | grep "MemTotal" | ${MYAWK} '{print $2}'`
    elif [ "${SYS_NAME}" = "AIX" ]; then
        CPU_CORES=`lsdev -Cc processor | wc -l`
        HOST_MEM_MB=`prtconf | grep "Memory Size" | head -n 1 | ${MYAWK} '{print $3}'`
        HOST_MEM=`expr ${HOST_MEM_MB} \* 1024`
    elif [ "${SYS_NAME}" = "HP-UX" ]; then
        CPU_CORES=`ioscan -fnk |grep processor |wc -l`
        HOST_MEM=`dmesg | grep -i physical | grep "available" | ${MYAWK} -F "," '{print $NF}' | ${MYAWK} '{print $2}'`
    elif [ "${SYS_NAME}" = "SunOS" ]; then
        CPU_CORES=`mpstat | grep -v CPU | wc -l`
        HOST_MEM_MB=`prtconf | grep Memory | ${MYAWK} '{print $3}'`
        HOST_MEM=`expr ${HOST_MEM_MB} \* 1024`
    fi

    if [ ${CPU_CORES} -lt ${CPU_CORES_MIN} ]; then
        ShowWarning "Warning: The number of logical CPU cores on the host is less than recommanded (${CPU_CORES_MIN})."
        Log "The number of logical CPU cores on the host is less than recommanded (${CPU_CORES_MIN})."
    fi

    if [ ${HOST_MEM} -lt ${HOST_MEM_MIN} ]; then 
        ShowWarning "Warning: The host memory is less than recommanded (${HOST_MEM_MIN_GB})."
        Log "The host memory is less than recommanded (${HOST_MEM_MIN_GB})."
    fi
}

# Check cpu memory disk-space os-version
# vmware: cpu--8 memeory--16G disk--1000M
# oracle & aishu: cpu--4 memeory--4G disk--1000M
CheckHostResource()
{
    Log "Begin to check the host resources."

    # check os type
    if [  ${BACKUP_ROLE} -eq ${BACKUP_ROLE_VMWARE} ]; then      # vmware backup
        CheckOsType vmware
    elif [ ${BACKUP_ROLE} -eq ${BACKUP_ROLE_HOST} ]; then       # oracle backup
        CheckOsType oracle
    fi
    
    # check cpu mem disk
    if [ ${BACKUP_ROLE} -eq ${BACKUP_ROLE_VMWARE} ]; then       # vmware backup
        CheckCpuAndMem vmware
        CheckFreeDiskSpace vmware
    else
        # oracle backup and aishu backup
        CheckCpuAndMem
        CheckFreeDiskSpace
    fi
}

GetBackupRole()
{
    if [ -f "${INSTALL_PACKAGE_PATH}/conf/client.conf" ]; then
    BACKUP_ROLE_CONFIG=`cat ${INSTALL_PACKAGE_PATH}/conf/client.conf | grep "backup_role"`
    fi 
    if [ -z "${BACKUP_ROLE_CONFIG}" ]; then
        Log "Configuration file client.conf of the old version."
        VMWARE_Config=`cat ${INSTALL_PACKAGE_PATH}/conf/client.conf | grep "vmCenter" |  ${MYAWK} -F '=' '{print $2}' | cut -c-4`
        if [ "${VMWARE_Config}" = "true" ]; then
            BACKUP_ROLE=${BACKUP_ROLE_VMWARE}
        fi
    else
        Log "Configuration file client.conf of the new version."
        BACKUP_ROLE=`cat ${INSTALL_PACKAGE_PATH}/conf/client.conf | grep "backup_role" | ${MYAWK} -F '=' '{print $NF}'`
    fi
    if [ "${BACKUP_ROLE}" = "" ]; then
        echo "Get backup_role from config failed."
        LogError "Get backup_role from config failed."
        return 1
    fi
}