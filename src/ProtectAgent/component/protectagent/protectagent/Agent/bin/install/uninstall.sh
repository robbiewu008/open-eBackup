#!/bin/sh
set +x

# ----------------------------
# the big package uninstall
# ----------------------------
AGENT_USER=rdadmin
AGENT_GROUP=rdadmin
SANCLIENT_USER=sanclient
SANCLIENT_GROUP=sanclient
DEFAULT_HOME_DIR="/home/rdadmin"
SANCLIENT_DEFAULT_HOME_DIR="/home/sanclient"
SYS_NAME=`uname -s`
USER_NOLOGIN_SHELL="/sbin/nologin"
SHELL_TYPE_SH="/bin/sh"

UNINSTALL_LOG_DIR="/var/log/ProtectAgent"
SANCLIENT_UNINSTALL_LOG_DIR="/var/log/ProtectSanClient"
BACKUP_ROLE=-1
FORCE_UNINSTALL=""
CURRENT_PATH=`dirname $0` && cd $CURRENT_PATH && CURRENT_PATH=`pwd`
BACKUP_ROLE_SANCLIENT_PLUGIN=5

if [ "$SYS_NAME" = "SunOS" ]; then
    MYAWK=nawk
    AGENT_USER_PATH="/export/home"
else
    MYAWK=awk
    AGENT_USER_PATH="/home"
fi

. ${CURRENT_PATH}/func_util.sh

###### Custom installation directory ######
TMP_DATA_BACKUP_AGENT_HOME=`cat /etc/profile | grep "DATA_BACKUP_AGENT_HOME=" |${MYAWK} -F "=" '{print $NF}'`
TMP_DATA_BACKUP_SANCLIENT_HOME=`cat /etc/profile | grep "export DATA_BACKUP_SANCLIENT_HOME=" |${MYAWK} -F "=" '{print $NF}'`
if [ -n "${TMP_DATA_BACKUP_AGENT_HOME}" ] || [ -n "${TMP_DATA_BACKUP_SANCLIENT_HOME}" ] ; then
    . /etc/profile
else
    DATA_BACKUP_AGENT_HOME=/opt
    DATA_BACKUP_SANCLIENT_HOME=/opt
    export DATA_BACKUP_AGENT_HOME DATA_BACKUP_SANCLIENT_HOME
fi
DEPLOY_TOP_DIR=${DATA_BACKUP_AGENT_HOME}
##############################################################

AGENT_ROOT=${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient/ProtectClient-E/
INSTALL_DIR=${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient
INSTALL_PATH=${DATA_BACKUP_AGENT_HOME}/DataBackup
SANCLIENT_AGENT_ROOT=${DATA_BACKUP_SANCLIENT_HOME}/DataBackup/SanClient/ProtectClient-E/
SANCLIENT_INSTALL_DIR=${DATA_BACKUP_SANCLIENT_HOME}/DataBackup/SanClient
SANCLIENT_INSTALL_PATH=${DATA_BACKUP_SANCLIENT_HOME}/DataBackup

##############################################################

CLIENT_BACK_ROLE=`cat ${CURRENT_PATH}/conf/client.conf 2>/dev/null | grep "backup_role" | ${MYAWK} -F '=' '{print $NF}'`
TESTCFG_BACK_ROLE=`su - ${SANCLIENT_USER} 2>/dev/null -s ${SHELL_TYPE_SH} -c "sed '/^BACKUP_ROLE=/!d;s/.*=//' ${CURRENT_PATH}/ProtectClient-E/conf/testcfg.tmp "`
if [ "${CLIENT_BACK_ROLE}" = "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ] || [ "${TESTCFG_BACK_ROLE}" = "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ] ; then
    AGENT_GROUP=${SANCLIENT_GROUP}
    AGENT_USER=${SANCLIENT_USER}
    UNINSTALL_LOG_DIR=${SANCLIENT_UNINSTALL_LOG_DIR}
    AGENT_ROOT=${SANCLIENT_AGENT_ROOT}
    INSTALL_DIR=${SANCLIENT_INSTALL_DIR}
    INSTALL_PATH=${SANCLIENT_INSTALL_PATH}
    DEFAULT_HOME_DIR=${SANCLIENT_DEFAULT_HOME_DIR}
    DEPLOY_TOP_DIR=${DATA_BACKUP_SANCLIENT_HOME}
fi
LOG_FILE="${INSTALL_DIR}/uninstall.log"

Log()
{
    if [ ! -f "${LOG_FILE}" ]; then
        touch "${LOG_FILE}"
        chmod 600 "${LOG_FILE}" 
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

    echo "[${DATE}][$$][${USER_NAME}] $1" >> "${LOG_FILE}"
}

CollectLog()
{
    if [ ! -d "${UNINSTALL_LOG_DIR}" ]; then
        mkdir -p ${UNINSTALL_LOG_DIR}
    else
        rm -rf "${UNINSTALL_LOG_DIR}"/*
    fi

    sh ${INSTALL_DIR}/collectlog.sh >/dev/null 2>&1
    if [ $? -ne 0 ]; then
        Log "Collect log fail."
        cp -r "${INSTALL_DIR}"/ProtectClient-E/slog ${UNINSTALL_LOG_DIR}/ 1>/dev/null 2>&1
        cp -r "${INSTALL_DIR}"/ProtectClient-E/log ${UNINSTALL_LOG_DIR}/ 1>/dev/null 2>&1
    else
        Log "Collect log succ."
        sysinfo_PATH="${INSTALL_DIR}"/ProtectClient-E/stmp/
        cp "${sysinfo_PATH}"/AGENTLOG* ${UNINSTALL_LOG_DIR}/ 1>/dev/null 2>&1
    fi
    cp ${LOG_FILE} ${UNINSTALL_LOG_DIR}/ >/dev/null 2>&1
}

ExitHandle()
{
    EXIT_CODE=$1
    CollectLog
    if [ ${EXIT_CODE} -eq 0 ]; then
        cd "${DEPLOY_TOP_DIR}"
        rm -rf "${INSTALL_DIR}"
    fi
    exit ${EXIT_CODE}
}

# only bash, ksh and csh
ChooseShell()
{
    PATH_SHELL=`which bash 2>/dev/null | $MYAWK '{print $1}'`
    if [ "" != "${PATH_SHELL}" ]; then
        USER_SHELL_TYPE=${PATH_SHELL}
        return 0
    fi
    
    PATH_SHELL=`which ksh 2>/dev/null | $MYAWK '{print $1}'`
    if [ "" != "${PATH_SHELL}" ]; then
        USER_SHELL_TYPE=${PATH_SHELL}
        return 0
    fi
    
    PATH_SHELL=`which csh 2>/dev/null | $MYAWK '{print $1}'`
    if [ "" != "${PATH_SHELL}" ]; then
        USER_SHELL_TYPE=${PATH_SHELL}
        return 0
    fi

    echo "There is no proper shell in the current system."
    Log "There is no proper shell in the current system."
    ExitHandle 1
}

CheckUserAndGroupInfo()
{
    #check Agent group
    GROUP_CHECK=`cat /etc/group | grep "^${AGENT_GROUP}:"`
    if [ "" = "${GROUP_CHECK}" ]; then
        if [ "${SYS_NAME}" = "AIX" ]; then
            mkgroup ${AGENT_GROUP}
        else
            groupadd ${AGENT_GROUP}
        fi

        if [ 0 -ne $? ]; then
            echo "DataBackup ProtectAgent working group ${AGENT_GROUP} was added failed."
            echo "The uninstall of the DataBackup ProtectAgent will be stopped."
            Log "The uninstall of the DataBackup ProtectAgent will be stopped."
            ExitHandle 1
        fi
    fi

    #check Agent user
    USER_CHECK=`cat /etc/passwd | grep "^${AGENT_USER}:"`
    if [ "" = "${USER_CHECK}" ]; then
        ChooseShell
        if [ "${SYS_NAME}" = "SunOS" ]; then
            useradd -m -s ${USER_SHELL_TYPE} -d ${DEFAULT_HOME_DIR} -g ${AGENT_GROUP} ${AGENT_USER}
        elif [ "${SYS_NAME}" = "AIX" ] || [ "${SYS_NAME}" = "HP-UX" ]; then
            useradd -m -d ${DEFAULT_HOME_DIR} -g ${AGENT_GROUP} ${AGENT_USER} >/dev/null 2>&1
        else
            useradd -m -s ${USER_NOLOGIN_SHELL} -g ${AGENT_GROUP} ${AGENT_USER}
        fi

        if [ 0 -ne $? ]; then
            echo "DataBackup ProtectAgent working user ${AGENT_USER} was added failed."
            echo "The uninstall of the DataBackup ProtectAgent will be stopped."
            Log "The uninstall of the DataBackup ProtectAgent will be stopped."
            ExitHandle 1
        fi
    fi
}

ExecuteUninstall() 
{
    cd "${INSTALL_DIR}/ProtectClient-E/sbin"
    . ./agent_uninstall.sh
    return $?
}

RemoveUserAndGroupInfo()
{
    # 升级情况 不需要移除用户和用户组
    if [ "$UPGRADE_FLAG_TEMPORARY" = "1" ]; then
        Log "The upgrade process does not require users and groups to be reset."
        return 0
    fi

    # kylin 也不需要移除用户和用户组
    IsKylinOS
    if [ $? -eq 0 ]; then
        Log "Kylin system does not require users and groups to be reset."
        return 0
    fi

    # Agent的移除用户和用户组
    id ${AGENT_USER} 1>/dev/null 2>&1
    if [ $? -eq 0 ]; then
        userdel -r ${AGENT_USER} 1>/dev/null 2>&1
    else
        echo "The ${AGENT_USER} account does not exist."
        Log "The ${AGENT_USER} account does not exist."
    fi
    cat /etc/group | grep '^'${AGENT_GROUP}:'' 1>/dev/null 2>&1
    if [ $? -eq 0 ]; then
        if [ "${SYS_NAME}" = "AIX" ]; then
            rmgroup ${AGENT_GROUP}
        else
            groupdel ${AGENT_GROUP}
        fi
    fi
    Log "Delete Defaults:rdadmin !requiretty from /etc/sudoers."
    if [ "${SYS_NAME}" = "SunOS" ]; then
        timestamp=`date +%y%m%d`
        sed '/Defaults:rdadmin !requiretty/d' /etc/sudoers > /etc/sudoers.${timestamp}
        mv -f /etc/sudoers.${timestamp} /etc/sudoers
    else
        sed -i '/Defaults:rdadmin !requiretty/d'  /etc/sudoers 1>/dev/null 2>&1
    fi
}

UnmountBackupFile()
{
    # Unmounting VMware Backup Files
    if [ -d "/opt/advbackup/vmware/data" ]; then
        umount -f -l /opt/advbackup/vmware/data/* >/dev/null 2>&1
    fi
}

JudgeIfUninstall()
{
    # 安装根目录不存在
    if [ ! -d "${INSTALL_DIR}" ]; then
        echo "The DataBackup ProtectAgent does not exist in current system. The uninstallation process exits."
        exit 0
    fi

    # ProtectClient-E 目录不存在 
    if [ ! -d "${INSTALL_DIR}/ProtectClient-E" ]; then
        echo "The DataBackup ProtectAgent fails to be uninstalled. The uninstallation process exits abnormally."
        Log "The DataBackup ProtectAgent fails to be uninstalled. The uninstallation process exits abnormally."
        ExitHandle 0
    fi
 
    # 可以正常卸载
    echo "The DataBackup ProtectAgent will be uninstalled."
}

#######################################

JudgeIfUninstall

CheckUserAndGroupInfo

ExecuteUninstall

UnmountBackupFile

RemoveUserAndGroupInfo

ExitHandle 0