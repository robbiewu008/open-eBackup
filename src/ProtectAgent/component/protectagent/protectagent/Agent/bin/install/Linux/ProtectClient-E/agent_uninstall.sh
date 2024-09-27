#!/bin/sh
set +x
SHELL_TYPE_SH="/bin/sh"
UNIX_CMD=
ENV_FILE=".bashrc"
ENV_EFFECT="export"
ENV_FLAG="="
ROCKY4_FLAG=0
AGENT_USER=rdadmin
SANCLIENT_USER=sanclient
BACKUP_ROLE_SANCLIENT_PLUGIN=5
DEE_GROUP=dataenableengine
AWK=awk
SYS_NAME=`uname -s`
CURRENT_PATH=`pwd`
AGENT_ROOT_PATH="${CURRENT_PATH}/.."
LOG_FILE_NAME="${AGENT_ROOT_PATH}/slog/agent_uninstall.log"
INSTALL_INFO_FILE="${AGENT_ROOT_PATH}/slog/install.info"
PLUGIN_DIR=${AGENT_ROOT_PATH}/../Plugins

########################################################################################
if [ "${SYS_NAME}" = "SunOS" ]; then
    AWK=nawk
else
    AWK=awk
fi

TESTCFG_BACK_ROLE=`su - ${SANCLIENT_USER} 2>/dev/null -s ${SHELL_TYPE_SH} -c "sed '/^BACKUP_ROLE=/!d;s/.*=//' ${AGENT_ROOT_PATH}/conf/testcfg.tmp "`
if [ "${TESTCFG_BACK_ROLE}" = "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ]; then
    AGENT_USER=${SANCLIENT_USER}
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

. "${AGENT_ROOT_PATH}/sbin/agent_sbin_func.sh"
RDADMIN_HOME_PATH=`cat /etc/passwd | grep "^${AGENT_USER}:" | $AWK -F ':' '{print $6}'`
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${AGENT_ROOT_PATH}/bin && export LD_LIBRARY_PATH

########################################################################################
# Function Definition
LogError()
{
    #$1=errorInfo,$2=errorCode, $3=errorDetailParam
    LogErr "$1" "$2" "job_log_agent_storage_update_uninstall_fail_label" "$3"
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

CheckShellType()
{
    SHELL_TYPE=`cat /etc/passwd | grep "^${AGENT_USER}:" | $AWK -F "/" '{print $NF}'`

    if [ "${SYS_NAME}" = "Linux" ]; then
        rocky4=`cat /etc/issue | grep 'Rocky'`
        if [ -n "${rocky4}" ]; then
            ROCKY4_FLAG=1
        fi
    fi

    if [ "${SYS_NAME}" = "AIX" ] || [ "${SYS_NAME}" = "HP-UX" ] || [ "${SYS_NAME}" = "SunOS" ] || [ 1 -eq ${ROCKY4_FLAG} ]; then
        if [ "${SHELL_TYPE}" = "bash" ]; then
            UNIX_CMD=-l
        fi
    fi
}

DeleteAutoStart()
{
    RM_LINENO=""
    if [ -f $1 ]; then
        RM_IMCTL_LINENO=`cat $1 | grep -n -w "cat /proc/devices | grep im_ctldev" | head -n 1 | $AWK -F ':' '{print $1}'`
        if [ "${RM_IMCTL_LINENO}" != "" ]; then
            sed "${RM_IMCTL_LINENO}d" "$1" > "$1".bak
            mv "$1".bak "$1"   
            DeleteAutoStart $1
        fi

        RM_LINENO=`cat $1 | grep -n -w "su - ${AGENT_USER} -s ${SHELL_TYPE_SH} ${UNIX_CMD} -c" | grep -w "bin/monitor" | head -n 1 | $AWK -F ':' '{print $1}'`
        if [ "${RM_LINENO}" != "" ]; then
            sed "${RM_LINENO}d" "$1" > "$1".bak
            mv "$1".bak "$1"
            DeleteAutoStart $1
        fi
    fi
}

GetInput()
{
    while [ 1 ]
    do
        printf "Are you sure you want to uninstall DataBackup ProtectAgent? y/n:\n"
        read isUninstall

        if [ "${isUninstall}" = "y" ] || [ "${isUninstall}" = "yes" ]; then
            break
        elif [ "${isUninstall}" = "n" ] || [ "${isUninstall}" = "no" ]; then
            printf "\\033[1;32mUninstall cancel\\033[0m \n"
            exit 1
        else
            printf "\\033[1;31mInvalid Input\\033[0m \n"
        fi
    done
}


# check user's environment variables
CheckUserEnv()
{
    # adjust profile file name based on the shell type
    if [ "${SYS_NAME}" = "Linux" ]; then
        SysRockyFlag=`cat /etc/issue | grep 'Linx'`
    fi
    SHELL_TYPE=`cat /etc/passwd | grep "^${AGENT_USER}:" | $AWK -F "/" '{print $NF}'`
    if [ "${SHELL_TYPE}" = "bash" ] || [ "${SHELL_TYPE}" = "sh" ]; then
        if [ "${SysRockyFlag}" != "" ] || [ "${SYS_NAME}" = "AIX" ] || [ "${SYS_NAME}" = "HP-UX" ] || [ "${SYS_NAME}" = "SunOS" ]; then
            ENV_FILE=".profile"
        fi
    elif [ "${SHELL_TYPE}" = "ksh" ]; then
        ENV_FILE=".profile"
    elif [ "${SHELL_TYPE}" = "nologin" ]; then
        ENV_FILE=".profile"
    elif [ "${SHELL_TYPE}" = "csh" ]; then
        ENV_FILE=".cshrc"
        ENV_EFFECT="setenv"
        ENV_FLAG=" "
    fi

    RDADMIN_ENV_FILE="${DEFAULT_HOME_DIR}/${ENV_FILE}"
    AGENT_ROOT="${AGENT_ROOT_PATH}"
    LD_LIBRARY_PATH="${LD_LIBRARY_PATH}"
    LIBPATH="$AGENT_ROOT_PATH/bin"

    if [ -f ${RDADMIN_ENV_FILE} ]; then
        Log "User's env file exists."
        return 0
    else
        Log "User's env file does not exist."
        SUExecCmd "touch ${RDADMIN_ENV_FILE}"
        SUExecCmd "chown -h ${AGENT_USER}:${AGENT_GROUP} ${RDADMIN_ENV_FILE}"
        SUExecCmd "chmod 600 ${RDADMIN_ENV_FILE}"
        SUExecCmd "echo ${ENV_EFFECT} AGENT_ROOT${ENV_FLAG}${AGENT_ROOT_PATH} >> ${RDADMIN_ENV_FILE}"
        if [ "AIX" = "${SYS_NAME}" ]; then
            SUExecCmd "echo ${ENV_EFFECT} LIBPATH${ENV_FLAG}${LIBPATH} >> ${RDADMIN_ENV_FILE}"
        else
            SUExecCmd "echo ${ENV_EFFECT} LD_LIBRARY_PATH${ENV_FLAG}${LD_LIBRARY_PATH} >> ${RDADMIN_ENV_FILE}"
        fi
    fi
}

CheckWhetherNeedToStop()
{
    PING_RESULT=`cat ${INSTALL_INFO_FILE} |grep 'PING_RESULT' |head -1 |$AWK -F '=' '{print $2}'`

    # can append other conditions
    if [ 0 -eq ${PING_RESULT} ]; then
        WHETHER_NEED_STOP=0
    else
        WHETHER_NEED_STOP=1
    fi
}

DeleteENV()
{
    Log "Delete environment variables of DataBackup ProtectAgent."

    RDADMIN_ENV_FILE="${RDADMIN_HOME_PATH}/${ENV_FILE}"
    AGENT_ROOT_LINENO=`SUExecCmdWithOutput "cat ${RDADMIN_ENV_FILE} | grep -n \"${ENV_EFFECT} AGENT_ROOT${ENV_FLAG}\" | $AWK -F ':' '{print \\$1}'"`

    if [ "${AGENT_ROOT_LINENO}" != "" ]; then
        SUExecCmd "sed  '${AGENT_ROOT_LINENO}d' ${RDADMIN_ENV_FILE} > ${RDADMIN_ENV_FILE}.bak"
        if [ "${SYS_NAME}" = "SunOS" ] || [ "${SYS_NAME}" = "AIX" ]; then
            su - ${AGENT_USER} -c "mv -f ${RDADMIN_ENV_FILE}.bak ${RDADMIN_ENV_FILE}" >/dev/null
        else
            su - ${AGENT_USER} -s ${SHELL_TYPE_SH} -c "mv -f \"${RDADMIN_ENV_FILE}\".bak \"${RDADMIN_ENV_FILE}\""
        fi
    fi
    
    if [ "AIX" = "${SYS_NAME}" ]; then
        LIBPATH_LINENO=`SUExecCmdWithOutput "cat ${RDADMIN_ENV_FILE} | grep -n \"${ENV_EFFECT} LIBPATH${ENV_FLAG}\" | $AWK -F ':' '{print \\$1}'"`
        if [ "${LIBPATH_LINENO}" != "" ]; then
            SUExecCmd "sed  '${LIBPATH_LINENO}d' ${RDADMIN_ENV_FILE} > ${RDADMIN_ENV_FILE}.bak"
            su - ${AGENT_USER} -c "mv -f \"${RDADMIN_ENV_FILE}\".bak \"${RDADMIN_ENV_FILE}\"" 
        fi
    elif [ "SunOS" = "${SYS_NAME}" ]; then
        LD_LIBRARY_PATH_LINENO=`SUExecCmdWithOutput "cat ${RDADMIN_ENV_FILE} | grep -n \"${ENV_EFFECT} LD_LIBRARY_PATH${ENV_FLAG}\" | $AWK -F ':' '{print \\$1}'"`
        if [ "${LD_LIBRARY_PATH_LINENO}" != "" ]; then
            SUExecCmd "sed  '${LD_LIBRARY_PATH_LINENO}d' ${RDADMIN_ENV_FILE} > ${RDADMIN_ENV_FILE}.bak"
            su - ${AGENT_USER} -c "mv -f ${RDADMIN_ENV_FILE}.bak ${RDADMIN_ENV_FILE}" >/dev/null
        fi
    else
        LD_LIBRARY_PATH_LINENO=`SUExecCmdWithOutput "cat ${RDADMIN_ENV_FILE} | grep -n \"${ENV_EFFECT} LD_LIBRARY_PATH${ENV_FLAG}\" | $AWK -F ':' '{print \\$1}'"`
        if [ "${LD_LIBRARY_PATH_LINENO}" != "" ]; then
            SUExecCmd "sed  '${LD_LIBRARY_PATH_LINENO}d' ${RDADMIN_ENV_FILE} > ${RDADMIN_ENV_FILE}.bak"
            su - ${AGENT_USER} -s ${SHELL_TYPE_SH} -c "mv -f \"${RDADMIN_ENV_FILE}\".bak \"${RDADMIN_ENV_FILE}\""
        fi
    fi

    DEE_GROUP_CHECK=`cat /etc/group | grep "^${DEE_GROUP}:"`
    if [ "${SYS_NAME}" = "Linux" ] && [ "${DEE_GROUP_CHECK}" != "" ]; then
        groupdel ${DEE_GROUP}
        if [ 0 -ne $? ]; then
            echo "Delete dee group ${DEE_GROUP} failed."
        fi
    fi

    Log "Environment variables of ProtectAgent was deleted successfully."
}

# Delete ProtectManager Access Configuration
DelNginxIPConfAndRestart()
{
    # stop process
    if [ "${SYS_NAME}" = "SunOS" ] || [ "${SYS_NAME}" = "AIX" ]; then
        su - ${AGENT_USER} -c "sh ${AGENT_ROOT_PATH}/bin/agent_stop.sh >/dev/null 2>&1" >/dev/null
    else
        su - ${AGENT_USER} -s ${SHELL_TYPE_SH} -c "${EXPORT_ENV}sh ${AGENT_ROOT_PATH}/bin/agent_stop.sh >/dev/null 2>&1"
    fi
    # restart process
    if [ "${SYS_NAME}" = "SunOS" ] || [ "${SYS_NAME}" = "AIX" ]; then
        su - ${AGENT_USER} -c "sh ${AGENT_ROOT_PATH}/bin/agent_start.sh >/dev/null 2>&1" >/dev/null
    else
        su - ${AGENT_USER} -s ${SHELL_TYPE_SH} -c "${EXPORT_ENV}sh ${AGENT_ROOT_PATH}/bin/agent_start.sh >/dev/null 2>&1"
    fi

    return 0
}

DelHostFromPM()
{
    # The upgrade process does not report offline requests
    if [ "$UPGRADE_FLAG_TEMPORARY" = "1" ]; then
        return 0
    fi

    Log "Start reporting offline requests."
    # Delete ProtectManager Access Configuration
    DelNginxIPConfAndRestart

    JudgeRandomNumType
    if [ "${SYS_NAME}" = "SunOS" ] || [ "${SYS_NAME}" = "AIX" ]; then
        su - ${AGENT_USER} -c "${AGENT_ROOT_PATH}/bin/agentcli registerHost DeleteHost >/dev/null 2>&1" >/dev/null
    else
        su - ${AGENT_USER} -s ${SHELL_TYPE_SH} -c "${EXPORT_ENV}${AGENT_ROOT_PATH}/bin/agentcli registerHost DeleteHost >/dev/null 2>&1"
    fi

    if [ "$?" != 0 ]; then
        Log "Delete host to ProtectManager failed."
        return 1
    fi

    Log "Delete host from ProtectManager success."
}

CheckIsNeedUninstallDataturbo()
{
    USER_DELETE=
    #3. delete user
    id -u sanclient >/dev/null 2>&1;
    if [ $? -eq 0 ]; then
        USER_DELETE=0
    else
        return 1
    fi

    #3. delete user
    id -u rdadmin >/dev/null 2>&1
    if [ $? -eq 0 ]; then
        USER_DELETE=0
    else
        return 1
    fi

    return ${USER_DELETE}
}

UninstallFileClient()
{
    FILECLIENT_INSTALL_PATH=${DATA_BACKUP_AGENT_HOME}/DataBackup/FileClient
    if [ ! -d ${FILECLIENT_INSTALL_PATH} ]; then
        Log "FileClient is not installed."
        return
    fi
 
    if [ ! -d ${FILECLIENT_INSTALL_PATH}/install ]; then
        Log "Unintall fileclient failed. The path ${FILECLIENT_INSTALL_PATH}/install does not not exist."
        return
    fi

    tmpPath=`pwd`
    cd ${FILECLIENT_INSTALL_PATH}/install
    ./uninstall.sh >> ${LOG_FILE_NAME} 2>&1
    RET_CODE=$?
    if [ ${RET_CODE} -ne 0 ]; then
        echo "Uninstall FileClient failed."
        Log "Uninstall FileClient failed."
        cd ${tmpPath}
        return
    fi
    cd ${tmpPath}
}

HandleUninstallDataTurbo()
{
    # 适配sanclient和通用代理都安装场景
    CheckIsNeedUninstallDataturbo
    if [ $? -eq 0 ]; then
        Log "No need uninstall dataturbo."   
        return
    fi
  
    # 升级场景无需卸载dataturbo
    if [ "$UPGRADE_FLAG_TEMPORARY" = "1" ]; then
        return
    fi

    # 非Agent一起安装的dataturbo不允许卸载
    if [ ! -f $DATATURBO_INSTALL_BY_AGENT ]; then
        return
    fi

    #执行卸载dataturbo
    ShowInfo "Start uninstall dataturbo."
    UnInstallDataTurbo
    if [ $? -ne 0 ]; then
        ShowWarning "Uninstall dataturbo faild."
        Log "Uninstall dataturbo faild."
        return
    fi
    ShowInfo "Uninstall dataturbo success."
    Log "Uninstall dataturbo success."
}

UninstallPlugins()
{
    # SanClient未安装插件，不需要卸载
    if [ "${TESTCFG_BACK_ROLE}" = "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ]; then
        return 0
    fi

    if [ ! -d "${PLUGIN_DIR}" ]; then
        Log "Plugins path is not exist."
        return 0
    fi

    Log "UninstallPlugins start."
    tmpPath=`pwd`
    #1. Get all plugins
    cd ${PLUGIN_DIR}
    pluginNames=`ls -l . | grep "Plugin" | ${AWK}  '/^d/ {print $NF}'`

    #2. Uninstalling plugins
    for pluginName in $pluginNames; do
        Log "Uninstalling $pluginName."
        tmpPluginPath=`pwd`
        # uninstall plugin
        cd ${pluginName}
        ./uninstall.sh >> ${LOG_FILE_NAME} 2>&1
        RET_CODE=$?
        if [ ${RET_CODE} -ne 0 ]; then
            Log "Uninstall $pluginName fail."
            exit 1
        fi
        cd ${tmpPluginPath}
    done
    cd ${tmpPath}
}

# remove nginx firewall config
RemoveFirewallConfig()
{
    export LD_LIBRARY_PATH=${AGENT_ROOT_PATH}/bin
    if [ $BACKUP_ROLE -ne 2 ]; then
        return 0
    fi
    # if vmware, get nginx port, remove port filter
    NGINX_PORT=`SUExecCmdWithOutput "cat ${AGENT_ROOT_PATH}/nginx/conf/nginx.conf | grep \"listen*\" | $AWK '{print \\$2}' | $AWK -F ':' '{print \\$NF}'"`
    if [ "${SYS_NAME}" = "SunOS" ] || [ "${SYS_NAME}" = "AIX" ]; then
        su - ${AGENT_USER} -c "${AGENT_ROOT_PATH}/bin/xmlcfg read Backup backup_role >${DEFAULT_HOME_DIR}/temporary.xml" >/dev/null
    else
        su - ${AGENT_USER} -s ${SHELL_TYPE_SH} -c "${EXPORT_ENV}${AGENT_ROOT_PATH}/bin/xmlcfg read Backup backup_role >${DEFAULT_HOME_DIR}/temporary.xml"
    fi
    if [ 0 -ne $? ]; then
        printf "\033[31mFail to get backup_role, err exit.\033[0m \n"
        LogError "Fail to get backup_role, err exit." ${ERR_UPGRADE_FAIL_READ_CONFIGURE}
        exit 1
    fi

    BACKUP_ROLE=`SUExecCmdWithOutput "cat ${DEFAULT_HOME_DIR}/temporary.xml| grep '^[^Enter]'"`
    RemovePortFilter ${NGINX_PORT}
}

StopAgent()
{
    Log "Stop DataBackup ProtectAgent."
    CheckWhetherNeedToStop
    if [ ${WHETHER_NEED_STOP} -eq 0 ]; then
        if [ "${SYS_NAME}" = "SunOS" ] || [ "${SYS_NAME}" = "AIX" ]; then
            su - ${AGENT_USER} -c "${AGENT_ROOT_PATH}/bin/agent_stop.sh" 1>/dev/null
        else
            su - ${AGENT_USER} -s ${SHELL_TYPE_SH} ${UNIX_CMD} -c "${EXPORT_ENV}${AGENT_ROOT_PATH}/bin/agent_stop.sh" 1>/dev/null 2>&1
        fi

        if [ $? -ne 0 ]; then
            LogError "DataBackup ProtectAgent was uninstalled failed." ${ERR_UPGRADE_FAIL_STOP_PROTECT_AGENT}
            sleep 1
            exit 1
        fi
    else
        echo "No need to stop DataBackup ProtectAgent."
    fi

    KillDataProcess

    # delete rdadmin other process 卸载sanclient时不需要执行
    if [ "${TESTCFG_BACK_ROLE}" != "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ]; then
        ProcNumber=`ps -ef | grep -w rdadmin  |grep -v grep | wc -l`
        if [ $ProcNumber -gt 0 ]; then
            echo "Other processes exist under the rdadmin account and will be killed."
            Log "Other processes exist under the rdadmin account and will be killed."
            PROCESS=`ps -ef | grep rdadmin | grep -v grep | $AWK '{print $2}'`
            for PROCES_ID in $PROCESS
            do
                Log "Send kill to rdadmin process, call: kill -9 $PROCES_ID"
                PROCESS_NAME=`ps -p ${PROCES_ID} -o pid,args| grep -v grep | grep roach_client`
                if [ "$UPGRADE_FLAG_TEMPORARY" = "1" ] && [ -n "${PROCESS_NAME}" ]; then
                    Log "The process is roach_client."
                else
                    kill -9 $PROCES_ID
                fi
            done
        fi
    fi
}

DeleteAutoStartConfig()
{
    Log "Delete auto start configure file of DataBackup ProtectAgent."
    if [ "${SYS_NAME}" = "Linux" ]; then
        if [ -f "/etc/redhat-release" ]; then
            str_version=`cat /etc/redhat-release 2>/dev/null | ${AWK} -F '(' '{print $1}' | ${AWK} '{print $NF}'`
            if [ "`RDsubstr ${str_version} 1 1`" = "8" ]; then
                # redhat 8
                rm -rf /usr/lib/systemd/system/rdagent.service >/dev/null
                systemctl daemon-reload
                return
            fi
        fi

        if [ -f /etc/SuSE-release ]; then
            DeleteAutoStart "/etc/rc.d/after.local"
            DeleteAutoStart "/etc/rc.d/boot.local"
        elif [ -f /etc/redhat-release ] || [ -f /etc/centos-release ] || [ -f /etc/bclinux-release ] || [ -f /etc/kylin-release ] || [ -f /etc/openEuler-release ]; then
            DeleteAutoStart "/etc/rc.d/rc.local"
        elif [ -f "/etc/debian_version" ] || [ -n "`cat /etc/issue | grep 'Linx'`" ]; then
            DeleteAutoStart "/etc/rc.local"
        else
            Log "Unsupport OS."
        fi
    elif [ "AIX" = "$SYS_NAME" ]; then
        if [ -f /etc/rc_rdagent.local ]; then
            rm "/etc/rc_rdagent.local"
        fi

        if [ -f /etc/inittab ]; then
            lsitab -a | grep /etc/rc_rdagent.local >/dev/null 2>&1
            if [ $? -eq 0 ]; then
                rmitab  rdagent:2:once:/etc/rc_rdagent.local
                init q
            fi
        fi
    elif [ "HP-UX" = "$SYS_NAME" ]; then
        START_SCRIPT=/sbin/init.d/AgentStart
        AGENT_CONF=/etc/rc.config.d/Agentconf
        SCRIPT=/sbin/rc3.d/S99AgentStart

        rm -rf "${SCRIPT}"
        rm -rf "${START_SCRIPT}"
        rm -rf "${AGENT_CONF}"
    elif [ "SunOS" = "$SYS_NAME" ]; then
        START_SCRIPT=/etc/init.d/agentstart
        SCRIPT=/etc/rc2.d/S99agentstart

        rm -rf "${SCRIPT}"
        rm -rf "${START_SCRIPT}"
    else
        Log "This operating system is not supported."
    fi
}

DeleteEnvVar()
{
    if [ "$UPGRADE_FLAG_TEMPORARY" = "1" ]; then
        return
    fi

    if [ "${SYS_NAME}" = "AIX" ]; then
        sed '/export DATA_BACKUP_AGENT_HOME=\//d' /etc/profile > /etc/profile.bak
        mv -f /etc/profile.bak /etc/profile
    elif [ "${SYS_NAME}" = "SunOS" ]; then
        sed '/DATA_BACKUP_AGENT_HOME=/d' /etc/profile > /etc/profile.bak1
        sed '/export DATA_BACKUP_AGENT_HOME/d' /etc/profile.bak1 > /etc/profile.bak
        mv -f /etc/profile.bak /etc/profile
        rm -rf /etc/profile.bak1
    elif [ "${TESTCFG_BACK_ROLE}" = "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ]; then
        sed -i '/export DATA_BACKUP_SANCLIENT_HOME=\//d' /etc/profile
    else
        sed -i '/export DATA_BACKUP_AGENT_HOME=\//d' /etc/profile
    fi
}

DeleteOtherInfo()
{

    rm -rf /lib64/libcommon.so
    rm -rf /lib64/libsecurecom.so

    tmpMountInfo=`mount | grep /home/rdadmin | ${AWK} '{printf " " $3}'`
    if [ "$tmpMountInfo" != "" ]; then
        umount -f -l $tmpMountInfo >/dev/null 2>&1
    fi

    #delete env
    DeleteEnvVar
}

if [ "$UPGRADE_FLAG_TEMPORARY" != "1" ]; then
    echo "You are about to uninstall the DataBackup ProtectAgent. This operation will stop the ProtectAgent service and the protected environment will no longer be protected."
    echo ""
    echo "Suggestion: Check whether the user-defined script needs to be backed up. The user-defined script will be deleted during the uninstallation."
    echo "Log in to the OceanProtect management page to clear the resources corresponding to the host."
    echo ""
    GetInput
fi

# 1.check execute user
CheckExecUser
if [ $? -ne 0 ]; then
    printf "\033[31m Please execute this script as user root. [0m\n"
    return 1
fi

# 2.check shell type
CheckShellType

HandleUninstallDataTurbo

# uninstall fileclient
UninstallFileClient

# 3.Uninstalling external plugin
UninstallPlugins

# 4.check user's environment variables, add env file if not exists
CheckUserEnv
DisableHttpProxy

# 5.delete the created host from ProtectManager
DelHostFromPM

# 6.remove nginx firewall config
RemoveFirewallConfig

# 7.stop protectagent process
StopAgent

# 8.delete auto start config
DeleteAutoStartConfig

# 9.delete env variables
if [ "$UPGRADE_FLAG_TEMPORARY" = "1" ]; then
    Log "The upgrade process does not require users and groups to be reset."
elif [ -f "/etc/kylin-release" ]; then
    Log "Kylin system does not require users and groups to be reset."
else
    cat /etc/os-release | grep PRETTY_NAME | grep -i Kylin >/dev/null
    if [ $? -eq 0 ]; then
        Log "Kylin system does not require users and groups to be reset."
    else
        DeleteENV
    fi
fi

# 10.delete other residual
DeleteOtherInfo

if [ "$UPGRADE_FLAG_TEMPORARY" != "1" ]; then
    printf "\\033[1;32m******************************************************** \\033[0m \n"
    printf "\\033[1;32m     Thanks for using DataBackup ProtectAgent         \\033[0m \n"
    printf "\\033[1;32m********************************************************\\033[0m \n"
fi

printf "\\033[1;32mDataBackup ProtectAgent has been uninstalled successfully.\\033[0m \n"
printf "\\033[1;32mThe DataBackup ProtectAgent installation path and user will be removed.\\033[0m \n"
Log "DataBackup ProtectAgent has been uninstalled successfully."