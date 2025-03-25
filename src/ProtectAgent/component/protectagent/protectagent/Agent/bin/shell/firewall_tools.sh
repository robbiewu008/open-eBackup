#!/bin/sh
set +x
SHELL_TYPE_SH="/bin/sh"
sysName=`uname -s`
SANCLIENT_USER=sanclient
if [ "$sysName" = "SunOS" ]
then
    MYAWK=nawk
else
    MYAWK=awk
fi

###### Custom installation directory ######
AGENT_ROOT_PATH=${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient/ProtectClient-E

##################################################################

BACKUP_ROLE_SANCLIENT_PLUGIN=5
SANCLIENT_AGENT_ROOT_PATH="${DATA_BACKUP_SANCLIENT_HOME}/DataBackup/SanClient/ProtectClient-E"
CLIENT_BACK_ROLE=`cat ${CURRENT_PATH}/conf/client.conf 2>/dev/null | grep "backup_role" | ${MYAWK} -F '=' '{print $NF}'`
TESTCFG_BACK_ROLE=`su - ${SANCLIENT_USER} 2>/dev/null -s ${SHELL_TYPE_SH} -c "sed '/^BACKUP_ROLE=/!d;s/.*=//' ${SANCLIENT_AGENT_ROOT_PATH}/conf/testcfg.tmp "`
if [ "${CLIENT_BACK_ROLE}" = "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ]  || [ "${TESTCFG_BACK_ROLE}" = "${BACKUP_ROLE_SANCLIENT_PLUGIN}" ] ; then
    AGENT_ROOT_PATH=${SANCLIENT_AGENT_ROOT_PATH}
fi 

. ${AGENT_ROOT_PATH}/sbin/agent_sbin_func.sh
LOG_FILE_NAME=${LOG_PATH}/agent_firewall_tool.log

CheckAndAddFirewall()
{
    GetNginxIPandPort
    if [ "${NGINX_LISTEN_DEFAULT_IP}" = "${NGINX_LISTEN_IP}" ]
    then
        SetFirewall "${NGINX_LISTEN_PORT}"
        if [ 0 -ne $? ]
        then
            Log "Set firewall failed."
            return 1
        fi
        return 0
    fi
    
    Log "No need set firewall."
    return 0
}

if [ $# -eq 1 ]
then
    if [ "$1" = "add" ]
    then
        CheckAndAddFirewall
        if [ 0 -ne $? ]
        then
            Log "Set firewall failed."
            exit 1
        fi
        exit 0
    fi
fi

echo "Invalid parameter."
exit 1

