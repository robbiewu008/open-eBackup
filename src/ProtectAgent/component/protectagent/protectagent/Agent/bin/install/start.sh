#!/bin/sh
set +x

#####################################################################
# The function of this script is to start the ProtectAgent service.
#####################################################################
AGENT_USER=rdadmin
AGENT_GROUP=rdadmin
SANCLIENT_USER=sanclient
SANCLIENT_GROUP=sanclient
UNIX_CMD=
SANCLIENT_INSTALL_PATH="/opt/DataBackup/SanClient"
SYS_NAME=`uname -s`
ROCKY4_FLAG=0
SHELL_TYPE_SH="/bin/sh"
USER_NOLOGIN_SHELL="/sbin/nologin"
BACKUP_ROLE_SANCLIENT_PLUGIN=5
CURRENT_PATH=`dirname $0` && cd $CURRENT_PATH && CURRENT_PATH=`pwd`
LO_IP_IPV4="127.0.0.1"
. ${CURRENT_PATH}/func_util.sh

if [ "$SYS_NAME" = "SunOS" ]; then
    AWK=nawk
else
    AWK=awk
fi

###### Custom installation directory ######
TMP_DATA_BACKUP_AGENT_HOME=`cat /etc/profile | grep "DATA_BACKUP_AGENT_HOME=" |${AWK} -F "=" '{print $NF}'`
TMP_DATA_BACKUP_SANCLIENT_HOME=`cat /etc/profile | grep "export DATA_BACKUP_SANCLIENT_HOME=" |${AWK} -F "=" '{print $NF}'`
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
INSTALL_PATH=${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient
SANCLIENT_INSTALL_PATH="${DATA_BACKUP_SANCLIENT_HOME}/DataBackup/SanClient"


###################################################

if [ "`ls -al /bin/sh | ${AWK} '{print $NF}'`" = "bash" ]; then
    EXPORT_ENV="if [ -f ~/.profile ]; then . ~/.profile; fi;"
else
    EXPORT_ENV=""
fi

CLIENT_BACK_ROLE=`cat ${CURRENT_PATH}/conf/client.conf 2>/dev/null | grep "backup_role" | ${AWK} -F '=' '{print $NF}'`
TESTCFG_BACK_ROLE=`su - ${SANCLIENT_USER} 2>/dev/null -s ${SHELL_TYPE_SH} -c "sed '/^BACKUP_ROLE=/!d;s/.*=//' ${CURRENT_PATH}/ProtectClient-E/conf/testcfg.tmp "`
if [ "${TESTCFG_BACK_ROLE}" = "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ]; then
    INSTALL_PATH=${SANCLIENT_INSTALL_PATH}
    AGENT_USER=${SANCLIENT_USER}
    AGENT_GROUP=${SANCLIENT_GROUP}
fi
AGENT_ROOT_PATH="${INSTALL_PATH}/ProtectClient-E"

id -u ${AGENT_USER} >/dev/null 2>&1
if [ $? -ne 0 ]; then
    useradd -m -s ${USER_NOLOGIN_SHELL} -g ${AGENT_GROUP} ${AGENT_USER}
fi

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

CheckHosts()
{
    temp_path=${LD_LIBRARY_PATH}
    if [ -z "$LD_LIBRARY_PATH" ]; then
        export LD_LIBRARY_PATH=${AGENT_ROOT_PATH}/bin
    else
        export LD_LIBRARY_PATH=${AGENT_ROOT_PATH}/bin:$LD_LIBRARY_PATH
    fi
    HOST_NAME=`SUExecCmdWithOutput "${AGENT_ROOT_PATH}/bin/agentcli gethostname"`
    export LD_LIBRARY_PATH=${temp_path}
    if [ "${HOST_NAME}" = "" ]; then
        echo "Get hostname failed!"
        return 0
    fi
    
    ret=`grep "${HOST_NAME}" /etc/hosts`
    if [ "${ret}" = "" ]; then
        echo "Hostname not exist!"
        VerifySpecialChar ${LO_IP_IPV4} ${HOST_NAME}
        echo "Add configration in file /etc/hosts."
        echo "${LO_IP_IPV4} ${HOST_NAME}" >> /etc/hosts
    fi
}

SelectStart()
{
    if [ -d "$INSTALL_PATH" ]; then
        if [ "${SYS_NAME}" = "AIX" ] || [ "${SYS_NAME}" = "SunOS" ]; then
            su - ${AGENT_USER} -c "${EXPORT_ENV}${INSTALL_PATH}/ProtectClient-E/bin/agent_start.sh"
            if [ $? -ne 0 ]; then
                su - ${AGENT_USER} -c "${EXPORT_ENV}${INSTALL_PATH}/ProtectClient-E/bin/agent_stop.sh"
                echo "Failed to start the DataBackup ProtectAgent, error exit."
                exit 1
            fi
        else
            su - ${AGENT_USER} -s ${SHELL_TYPE_SH} ${UNIX_CMD} -c "${EXPORT_ENV}\"${INSTALL_PATH}/ProtectClient-E/bin/agent_start.sh\""
            if [ $? -ne 0 ]; then
                su - ${AGENT_USER} -s ${SHELL_TYPE_SH} ${UNIX_CMD} -c "${EXPORT_ENV}\"${INSTALL_PATH}/ProtectClient-E/bin/agent_stop.sh\""
                echo "Failed to start the DataBackup ProtectAgent, error exit."
                exit 1
            fi
        fi
    else
        echo "Not install any client,the startup will be stopped."
        return 1
    fi

    return 0
}

ModifyTTYService()
{
    ls /etc/systemd/system/getty.target.wants/getty@tty1.service >/dev/null 2>&1
    if [ $? -eq 0 ]; then
        # 通过tty终端启动agent时，避免退出终端后系统杀掉agent相关进程
        cat /etc/systemd/system/getty.target.wants/getty@tty1.service | grep 'KillMode=' >/dev/null 2>&1
        if [ $? -ne 0 ]; then
            sed -i 's/\[Service\]/\[Service\]\nKillMode=process/g' /etc/systemd/system/getty.target.wants/getty@tty1.service
            systemctl daemon-reload
        fi
    fi
}

CheckInputParams()
{
    if [ $# -eq 1 ] && [ $1 = "service" ]; then
        echo "exec start.sh service"
        su - ${AGENT_USER} -s ${SHELL_TYPE_SH} ${UNIX_CMD} -c "${AGENT_ROOT_PATH}/bin/monitor &" > /dev/null 2>&1
        return 1
    fi
    return 0
}

# Check Whether Input Params
CheckInputParams $*
if [ $? -eq 1 ]; then
    exit 0
fi

ModifyTTYService
# Check the shell type.
CheckShellType

# Start the service process.
SelectStart

if [ -d ${AGENT_ROOT_PATH}/conf/thrift ]; then
    CheckHosts
fi
exit $?