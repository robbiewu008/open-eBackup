#!/bin/sh
set +x

# ----------------------------------
# the big package startup
# ----------------------------------
AGENT_USER=rdadmin
AGENT_GROUP=rdadmin
DEFAULT_USER_INTERNAL=nobody
DEFAULT_GROUP_INTERNAL=nobody
EXAGENT_USER=exrdadmin
UNIX_CMD=
INSTALL_PATH="/opt/DataBackup/ProtectClient"
sysName=`uname -s`
ROCKY4_FLAG=0
SHELL_TYPE_SH="/bin/sh"
USER_NOLOGIN_SHELL="/sbin/nologin"

CURL_ERR_CONNECT=7
OPERATION_TIMEOUT=28

MOUNT_SCRIPT_PATH=/opt/script/mount_oper.sh
PERMISSION_SCRIPT_PATH=/opt/script/change_permission.sh

CURRENT_USER=`whoami`
if [ "${CURRENT_USER}" != "${AGENT_USER}" ]; then
    echo "The current user[${CURRENT_USER}] is not the rdadmin user."
    exit 1
fi

##########  ##########
AGENT_ROOT_PATH=/opt/DataBackup/ProtectClient/ProtectClient-E
VIRTUAL_PLUGIN_PATH=/opt/DataBackup/ProtectClient/Plugins/VirtualizationPlugin

CERT_PATH=/opt/logpath/protectmanager/cert
PM_IP_LIST=""
PM_IP=""
PM_NETWORK_TYPE=""
NETWORK_CONF_FILE="/opt/network_conf"
NODE_NAME=`env|grep NODE_NAME |awk -F '=' '{print $NF}'`
PODE_NAME=`env|grep MY_POD_NAME |awk -F '=' '{print $NF}'`
POD_IP=`env|grep POD_IP |awk -F '=' '{print $NF}'`
SEMICOLON=":"
LISTEN_PORT=59529
DOT="."
LOG_FILE_NAME="${AGENT_ROOT_PATH}/log/internal_run.log"
source /home/rdadmin/.bashrc
PERSISTENCE_LOG_ROOT_PATH=/opt/logpath/logs/${NODE_NAME}/protectengine-e/agent
PERSISTENCE_CONF_ROOT_PATH=/opt/protectagent/${PODE_NAME}/config
PERSISTENCE_TMP_ROOT_PATH=/mnt/protectagent
PERSISTENCE_PROTECTAGENT_ROOT_PATH=/opt/protectagent
PERSISTENCE_PODE_ROOT_PATH=/opt/protectagent/${PODE_NAME}
PERSISTENCE_PLUGINS_ROOT_PATH=/opt/protectagent/${PODE_NAME}/Plugins

########## BACKUP_ROLE ##########
BACKUP_ROLE=-1
BACKUP_ROLE_HOST=0
BACKUP_ROLE_AISHU=1
BACKUP_ROLE_VMWARE=2
BACKUP_ROLE_DWS=3
BACKUP_ROLE_GENERAL_PLUGIN=4

###### BACKUP_SCENE ######
BACKUP_SCENE=-1
BACKUP_SCENE_EXTERNAL=0
BACKUP_SCENE_INTERNAL=1

###### ENVIRONMENT_TYPE ######
ENVIRONMENT_TYPE=-1
ENVIRONMENT_TYPE_DORADO=0
ENVIRONMENT_TYPE_GENERAL=1

###### REGISTER_STATUS ######
REGISTER_STATUS=-1
REGISTER_STATUS_RUN=0
REGISTER_STATUS_FAIL=1
REGISTER_STATUS_SUCC=2

###### ANTI-EXTORTION ######
DEPLOY_TYPE_A8000="a8000"

###### 分布式一体机 #######
DEPLOY_DISTRIBUTED_APPLIANCE="d7"

###### 通用服务器 #######
DEPLOY_GENERAL_SERVER="d8"
###### 防勒索
DEPLOY_TYPE_HYPERDETECT="hyperdetect"
###### 白牌防勒索
DEPLOY_TYPE_HYPERDETECT_NO_BRAND="d4"
###### 安全一体机
DEPLOY_TYPE_CYBER_ENGINE="d5"
CONTAINER_CYBER_ENGINE_NIC="eth1"
###### X6000
DEPLOY_TYPE_X6000="d1"
CONTAINER_NIC="eth0"
###### X3000
DEPLOY_TYPE_X3000="d2"
X3000_SUB_JOB_MAX=5
X3000_NOT_INFRA_MAIN_JOB_MAX=10
X3000_DO_INFRA_MAIN_JOB_MAX=5

# x6000部署，框架处理主任务最大并行数
X6000_JOB_MAX=9

###################################################
if [ "$sysName" = "SunOS" ]
then
    AWK=nawk
else
    AWK=awk
fi

function Log()
{
    echo "$1"
    if [ ! -f "${LOG_FILE_NAME}" ]; then
        touch "${LOG_FILE_NAME}"
        chmod 600 "${LOG_FILE_NAME}"
    fi
    
    DATE=`date +%y-%m-%d--%H:%M:%S`
    USER_NAME=`whoami`
    echo "[${DATE}][$$][${USER_NAME}] $1" >> "${LOG_FILE_NAME}"
}

IpPing()
{
    ipAddr=$1
    echo ${ipAddr} | grep "\\:" >/dev/null 2>&1
    if [ 0 -eq $? ]; then
        ping6 -c 1 -w 3 ${ipAddr} > /dev/null 2>&1
        return $?
    fi
    echo ${ipAddr} | grep "\\." >/dev/null 2>&1
    if [ 0 -eq $? ]; then
        ping -c 1 -w 3 ${ipAddr} > /dev/null 2>&1
        return $?
    fi
    return 1
}

function CheckShellType()
{
    SHELL_TYPE=`cat /etc/passwd | grep "^${AGENT_USER}:" | $AWK -F "/" '{print $NF}'`

    if [ "${sysName}" = "Linux" ]; then
        rocky4=`cat /etc/issue | grep 'Rocky'`
        if [ -n "${rocky4}" ]
        then
            ROCKY4_FLAG=1
        fi
    fi

    if [ "${sysName}" = "AIX" ] || [ "${sysName}" = "HP-UX" ] || [ "${sysName}" = "SunOS" ] || [ 1 -eq ${ROCKY4_FLAG} ]; then
        if [ "${SHELL_TYPE}" = "bash" ]; then
            UNIX_CMD=-l
        fi
    fi
}

function GetBackupScene()
{
    echo "Start GetBackupScene."
    BACKUP_SCENE_CONFIG=`cat ${INSTALL_PATH}/ProtectClient-E/conf/testcfg.tmp | grep "BACKUP_SCENE"`
    if [ -z "${BACKUP_SCENE_CONFIG}" ]; then
        BACKUP_SCENE=${BACKUP_SCENE_EXTERNAL}
    else
        BACKUP_SCENE=`cat ${INSTALL_PATH}/ProtectClient-E/conf/testcfg.tmp | grep "BACKUP_SCENE" | awk -F '=' '{print $NF}'`
    fi

    if [ -z "${BACKUP_SCENE}" ]; then
        echo "Get backup_scene from config  failed."
        exit 1
    fi

    echo "backup_scene=${BACKUP_SCENE}"
}

function GetEnvironment()
{
    echo "Start get environment."
    environmentConfig=`env |grep ENVIRONMENT |awk -F '=' '{print $2}'`

    if [ "${environmentConfig}" = "Dorado" ]; then
        ENVIRONMENT_TYPE=${ENVIRONMENT_TYPE_DORADO}
    else
        ENVIRONMENT_TYPE=${ENVIRONMENT_TYPE_GENERAL}
    fi

    TMP_ENVIRONMENT_TYPE=`cat ${INSTALL_PATH}/ProtectClient-E/conf/testcfg.tmp | grep "ENVIRONMENT_TYPE"`
    if [ -z "${TMP_ENVIRONMENT_TYPE}" ]; then
        echo "ENVIRONMENT_TYPE=${ENVIRONMENT_TYPE}" >> ${INSTALL_PATH}/ProtectClient-E/conf/testcfg.tmp
    else
        sed -i "/ENVIRONMENT_TYPE/s/.*/ENVIRONMENT_TYPE=${ENVIRONMENT_TYPE}/g" ${AGENT_ROOT_PATH}/conf/testcfg.tmp
    fi

    echo "ENVIRONMENT_TYPE=${ENVIRONMENT_TYPE}."
}

function GetRegisterStatus()
{
    Log "Start GetRegisterStatus."
    TMP_REGISTER_STATUS=`cat ${AGENT_ROOT_PATH}/conf/testcfg.tmp | grep "register_status"`
    if [ -z "${TMP_REGISTER_STATUS}" ]; then
        echo  "register_status=${REGISTER_STATUS_FAIL}" >> ${AGENT_ROOT_PATH}/conf/testcfg.tmp
        REGISTER_STATUS=${REGISTER_STATUS_FAIL}
    else
        REGISTER_STATUS=`cat ${AGENT_ROOT_PATH}/conf/testcfg.tmp | grep "register_status" | awk -F '=' '{print $NF}'`
    fi

    if [ "${REGISTER_STATUS}" = "" ]; then
        Log "Get register_status failed."
        exit 1
    fi
    Log "register_status=${REGISTER_STATUS}."
}

function ObtainIpByNic()
{
    Log "Start to obtain ip by nic."
    if [ "ipv4" = "${G_NETWORK_TYPE}" ]; then
        LISTEN_IP=`ip addr show $1 | grep -w "inet" | $AWK -F " " '{print $2}' | $AWK -F "/" '{print $1}'`
    elif [ "ipv6" = "${G_NETWORK_TYPE}" ]; then
        LISTEN_IP=`ip addr show $1 | grep -w "inet6" | $AWK -F " " '{print $2}' | $AWK -F "/" '{print $1}'`
    else
        Log "Failed to obtain the IP address type."
    fi
}

function ConfigListenIp()
{    
    Log "Start config listen ip."
    index=1
    while [ 1 ]
    do
        # Split ip
        TMP_LOCAL_IP=`echo "${LOCAL_IP_LIST}" | $AWK -F ',' -v i="$index" '{print $i}'`
        index=`expr $index + 1`
        if [ -z ${TMP_LOCAL_IP} ]; then
            break
        fi

        if [ ! -z  ${LOCAL_IP} ]; then
            continue
        fi
        LOCAL_IP=${TMP_LOCAL_IP}
    done

    # 从backup_nat_plane中获取本节点的ip，即PM ip

    LISTEN_IP=${LOCAL_IP}

    if [ -z "${LISTEN_IP}" ]; then
        Log "No available local IP address found."
        exit 1
    fi

    ${AGENT_ROOT_PATH}/bin/xmlcfg write System agent_ip ${LISTEN_IP}
    Log "Listening ip address:${LISTEN_IP}."

     # 1. fastcgi
    ${AGENT_ROOT_PATH}/bin/xmlcfg write System port ${LISTEN_PORT}
    sed -i "/fastcgi_pass/s/.*/            fastcgi_pass   127.0.0.1:${LISTEN_PORT};/g" "${AGENT_ROOT_PATH}/nginx/conf/nginx.conf"

    # 2. listen
    if [ "ipv4" = "${G_NETWORK_TYPE}" ]; then
        sed -i "/listen/s/.*/        listen       ${LISTEN_IP}:${LISTEN_PORT} ssl;/g" "${AGENT_ROOT_PATH}/nginx/conf/nginx.conf"
    elif [ "ipv6" = "${G_NETWORK_TYPE}" ]; then
        sed -i "/listen/s/.*/        listen       [${LISTEN_IP}]:${LISTEN_PORT} ssl;/g" "${AGENT_ROOT_PATH}/nginx/conf/nginx.conf"
    fi
}

function InnerGetLocalConnectableNic()
{
    local nicName=""
    local retryTime=30
    while [ $retryTime -ge 0 ]
    do
        default=$(grep backupNetPlane /opt/podinfo/annotations | awk -F "\"" '{ print $2 }')
        # multpile backupNetPlane may exist, and named in the 'annotations'
        # file as 'backupNetPlane', 'backupNetPlane_1', 'backupNetPlane_2', etc.
        for defNo in ${default}
        do
            if [ ! -z "${defNo}" ] && [ "${defNo}" != "None" ]; then
                data=$(grep k8s.v1.cni.cncf.io/network-status /opt/podinfo/annotations | awk -F "=" '{ print $2 }')
                netIp=$(python3 ${AGENT_ROOT_PATH}/bin/get_net_plane_ip.pyc "${data}" ${defNo})
                if [ ! -z "${netIp}" ] && [ "${netIp}" != "None" ]; then
                    nicName="$nicName `netstat -ie | grep -B1 "$netIp" | head -n1 | awk -F ":" '{print $1}'`"
                fi
            fi
        done
        if [ ! -z "${nicName}" ]; then
            echo "$nicName"
            return 0
        fi
        retryTime=`expr $retryTime - 1`
        sleep 5
    done
}

function InnerGetLocalConnectableNic()
{
    local nicName=""
    local retryTime=30
    while [ $retryTime -ge 0 ]
    do
        default=$(grep backupNetPlane /opt/podinfo/annotations | awk -F "\"" '{ print $2 }')
        # multpile backupNetPlane may exist, and named in the 'annotations'
        # file as 'backupNetPlane', 'backupNetPlane_1', 'backupNetPlane_2', etc.
        for defNo in ${default}
        do
            if [ ! -z "${defNo}" ] && [ "${defNo}" != "None" ]; then
                data=$(grep k8s.v1.cni.cncf.io/network-status /opt/podinfo/annotations | awk -F "=" '{ print $2 }')
                netIp=$(python3 ${AGENT_ROOT_PATH}/bin/get_net_plane_ip.pyc "${data}" ${defNo})
                if [ ! -z "${netIp}" ] && [ "${netIp}" != "None" ]; then
                    nicName="$nicName `netstat -ie | grep -B1 "$netIp" | head -n1 | awk -F ":" '{print $1}'`"
                fi
            fi
        done
        if [ ! -z "${nicName}" ]; then
            echo "$nicName"
            return 0
        fi
        retryTime=`expr $retryTime - 1`
        sleep 5
    done
}

function ConfigPMIp()
{
    Log "Start config pm ip."

    # 1. Obtain the service ip address of the PM
    if [ ${ENVIRONMENT_TYPE} -eq ${ENVIRONMENT_TYPE_DORADO} ]; then
        # 1 Obtain the service ip address of the PM from the OM.
        nodeCount=0
        while [ 1 ]
        do
            nodeInfo=`python ${AGENT_ROOT_PATH}/bin/nodeinfo.pyc`
            if [ $? -eq 0 ]; then
                nodeCount=`echo "${nodeInfo}" |grep nodeName |wc -l`
                Log "nodeInfo=${nodeInfo}"
                Log "nodeCount=${nodeCount}"
                if [ ${nodeCount} -ne 0 ]; then
                    break
                fi
            else
                Log "Failed to invoke the interface."
            fi

            sleep 30
        done

        while [ 1 ]
        do
            if [ ! -s /opt/network-conf/backup_net_plane ] && [ "${DEPLOY_TYPE}" != "${DEPLOY_TYPE_CYBER_ENGINE}" ]; then
                echo "/opt/network-conf/backup_net_plane is empty, waiting..."
                sleep 30
            else
                echo "success to load backup netplane. "
                break
            fi
        done

        while [ 1 ]
        do
            PM_IP_LIST=`python ${AGENT_ROOT_PATH}/bin/getpmip.pyc | sed -n 1p`
            if [ "${PM_IP_LIST}" = "" ]; then
                Log "Get protect manager ip fail."
                sleep 30
            else
                PM_IP_COUNT=` echo "${PM_IP_LIST}" |grep -o ','|wc -l`
                PM_IP_COUNT=`expr ${PM_IP_COUNT} + 1`
                Log "PM_IP_COUNT=${PM_IP_COUNT}"
                LOCAL_IP_LIST=`python ${AGENT_ROOT_PATH}/bin/getpmip.pyc | sed -n 2p`
                NODE_COUNT=`python ${AGENT_ROOT_PATH}/bin/getpmip.pyc | sed -n 3p`
                INFRA_NODE_NAME=`python ${AGENT_ROOT_PATH}/bin/getpmip.pyc | sed -n 4p`
                if [ -n "$INFRA_NODE_NAME" ]; then
                    Log "Obtaining the IP address from the OM succeeded."
                    break
                fi
            fi
        done
    else
        PM_IP_LIST=`cat ${NETWORK_CONF_FILE} |grep IPADDR |awk -F '=' '{print $NF}'`
    fi
    Log "Get protect manager ip succ, PM_IP_LIST=${PM_IP_LIST}."
    ${AGENT_ROOT_PATH}/bin/xmlcfg write Backup ebk_server_ip ${PM_IP_LIST}
    if [ "${DEPLOY_TYPE}" = "${DEPLOY_TYPE_X3000}" ] && [ "$INFRA_NODE_NAME" == "$NODE_NAME" ]; then
        ${AGENT_ROOT_PATH}/bin/xmlcfg write Frame main_job_cnt_max_inner ${X3000_DO_INFRA_MAIN_JOB_MAX}
    fi
    # 3. Obtaining the registration IP address
    index=1
    while [ 1 ]
    do
        # Split ip
        TMP_PM_IP=`echo "${PM_IP_LIST}" | $AWK -F ',' -v i="$index" '{print $i}'`
        index=`expr $index + 1`
        if [ -z ${TMP_PM_IP} ]; then
            break
        fi

        #Check connectivity.
        if [ ! -z  ${PM_IP} ]; then
            continue
        fi

        G_NETWORK_TYPE="ipv4"
        pingCMD="ping"
        nic="vrf-srv"
        echo ${TMP_PM_IP} | grep "\\${SEMICOLON}" >/dev/null 2>&1
        if [ $? -eq 0 ]; then
            G_NETWORK_TYPE="ipv6"
            pingCMD="ping6"
        fi
        if [ "${DEPLOY_TYPE}" = "${DEPLOY_DISTRIBUTED_APPLIANCE}" ] || [ "${DEPLOY_TYPE}" = "${DEPLOY_GENERAL_SERVER}" ]; then
            ${pingCMD} -c 1 ${TMP_PM_IP} -W 3 > /dev/null 2>&1
        else
            ${pingCMD} -c 1 -I ${nic} ${TMP_PM_IP} -W 3 > /dev/null 2>&1
        fi
        if [ $? -eq 0 ]; then
            echo "The available pm ip: ${TMP_PM_IP}"
            PM_IP=${TMP_PM_IP}
            break
        else
            Log "pm ip is invaild, pm ip: ${TMP_PM_IP}"
        fi
    done

    Log "PM_IP=${PM_IP}"
}

function ConfigIp()
{
    Log "Start config ip, deply_type=[${DEPLOY_TYPE}]."

    if [ "${DEPLOY_TYPE}" = "${DEPLOY_TYPE_HYPERDETECT}" ] || [ "${DEPLOY_TYPE}" = "${DEPLOY_TYPE_HYPERDETECT_NO_BRAND}" ]; then 
        # dorado环境防勒索部署，无法使用业务面ip，固定使用容器卡eth0,仅存在ipv4
        LISTEN_IP=`ip addr show ${CONTAINER_NIC} |  grep -w "inet" | $AWK -F " " '{print $2}' | $AWK -F "/" '{print $1}'`
 
        Log "Listening ip address: ${LISTEN_IP}"
        # 1. fastcgi
        ${AGENT_ROOT_PATH}/bin/xmlcfg write System port ${LISTEN_PORT}
        sed -i "/fastcgi_pass/s/.*/            fastcgi_pass   127.0.0.1:${LISTEN_PORT};/g" "${AGENT_ROOT_PATH}/nginx/conf/nginx.conf"
 
        # 2.
        sed -i "/listen/s/.*/        listen       ${LISTEN_IP}:${LISTEN_PORT} ssl;/g" "${AGENT_ROOT_PATH}/nginx/conf/nginx.conf"
 
        # 3. agent_cfg.xml
        ${AGENT_ROOT_PATH}/bin/xmlcfg write Backup ebk_server_ip ${LISTEN_IP}
        ${AGENT_ROOT_PATH}/bin/xmlcfg write System domain_name_dme  ${HOSTNAME}
    elif [ "${DEPLOY_TYPE}" = "${DEPLOY_TYPE_CYBER_ENGINE}" ]; then
        # dorado环境防勒索部署，无法使用业务面ip，固定使用容器卡eth0,仅存在ipv4
        LISTEN_IP=`ifconfig ${CONTAINER_CYBER_ENGINE_NIC} |  grep -w "inet" | $AWK -F " " '{print $2}' | $AWK -F "/" '{print $1}'`
 
        Log "Listening ip address: ${LISTEN_IP}"
        # 1. fastcgi
        ${AGENT_ROOT_PATH}/bin/xmlcfg write System port ${LISTEN_PORT}
        sed -i "/fastcgi_pass/s/.*/            fastcgi_pass   127.0.0.1:${LISTEN_PORT};/g" "${AGENT_ROOT_PATH}/nginx/conf/nginx.conf"
 
        # 2.
        sed -i "/listen/s/.*/        listen       ${LISTEN_IP}:${LISTEN_PORT} ssl;/g" "${AGENT_ROOT_PATH}/nginx/conf/nginx.conf"
 
        # 3. agent_cfg.xml
        ${AGENT_ROOT_PATH}/bin/xmlcfg write Backup ebk_server_ip ${LISTEN_IP}
        ${AGENT_ROOT_PATH}/bin/xmlcfg write System domain_name_dme  ${HOSTNAME}
    else
        ConfigPMIp
 
        ConfigListenIp
    fi
}

function ConfigCert()
{
    Log "Start config cert."
    # 1. decrypt and enckey
    python ${AGENT_ROOT_PATH}/bin/decrypt.pyc
    if [ $? -ne 0  ]; then
        Log "Failed to obtain the password."
        exit 1
    fi
    Log "Enckey succ."

    # 2. write ciphertext to xml
    PKEY_PASSWORD=`cat ${AGENT_ROOT_PATH}/tmp/input_tmpcipherFile`
    rm -rf "${AGENT_ROOT_PATH}/tmp/input_tmpcipherFile"
    ${AGENT_ROOT_PATH}/bin/xmlcfg write Monitor nginx ssl_key_password "${PKEY_PASSWORD}"
    if [ $? -ne 0 ]; then
        Log "Failed write ssl_key_password"
        exit 1
    fi

    # 3. copy Certificates
    if [ ! -f "${CERT_PATH}/CA/certs/ca.crt.pem" ];then
        Log "The entered path is incorrect. The pmca.pem file cannot be found."
        exit 1
    else
        cp "${CERT_PATH}/CA/certs/ca.crt.pem" "${AGENT_ROOT_PATH}/nginx/conf/pmca.pem"
        if [ $? -ne 0 ] || [ ! -f "${AGENT_ROOT_PATH}/nginx/conf/pmca.pem" ]; then
            Log "Failed copy pmca.pem."
            exit 1
        fi
    fi

    if [ ! -f "${CERT_PATH}/internal/ProtectAgent/ca/ca.crt.pem" ];then
        Log "The entered path is incorrect. The agentca.pem file cannot be found."
        exit 1
    else
        cp "${CERT_PATH}/internal/ProtectAgent/ca/ca.crt.pem" "${AGENT_ROOT_PATH}/nginx/conf/agentca.pem"
        if [ $? -ne 0 ] || [ ! -f "${AGENT_ROOT_PATH}/nginx/conf/agentca.pem" ]; then
            Log "Failed copy agentca.pem."
            exit 1
        fi
    fi

    if [ ! -f "${CERT_PATH}/internal/ProtectAgent/client.crt.pem" ];then
        Log "The entered path is incorrect. The client.crt.pem file cannot be found."
        exit 1
    else
        cp "${CERT_PATH}/internal/ProtectAgent/client.crt.pem" "${AGENT_ROOT_PATH}/nginx/conf/server.pem"
        if [ $? -ne 0 ] || [ ! -f "${AGENT_ROOT_PATH}/nginx/conf/server.pem" ]; then
            Log "Failed copy server.pem."
            exit 1
        fi
    fi

    if [ ! -f "${CERT_PATH}/internal/ProtectAgent/client.pem" ];then
        Log "The entered path is incorrect. The client.pem file cannot be found."
        exit 1
    else
        rm -rf "${AGENT_ROOT_PATH}/nginx/conf/server.key"
        cp "${CERT_PATH}/internal/ProtectAgent/client.pem" "${AGENT_ROOT_PATH}/nginx/conf/server.key"
        if [ $? -ne 0 ] || [ ! -f "${AGENT_ROOT_PATH}/nginx/conf/server.key" ]; then
            Log "Failed copy server.key."
            exit 1
        fi
    fi
    Log "Config cert succ."
}

function ConfigUUID()
{
    Log "Start config uuid."
    # 1. create UUID path
    if [ ! -d "${PERSISTENCE_CONF_ROOT_PATH}/HostSN" ]; then
        Log "Agent uuid path not exit."
        sudo ${PERMISSION_SCRIPT_PATH} mkdir "${PERSISTENCE_CONF_ROOT_PATH}/HostSN"
        sudo ${PERMISSION_SCRIPT_PATH} chown ${AGENT_USER}:${DEFAULT_GROUP_INTERNAL} "${PERSISTENCE_CONF_ROOT_PATH}/HostSN"
    fi
    chmod 750 "${PERSISTENCE_CONF_ROOT_PATH}/HostSN"
    Log "Create uuid dir end."

    # 2. Generating the UUID
    if [ ! -f "${PERSISTENCE_CONF_ROOT_PATH}/HostSN/HostSN" ]; then
        Log "Generating uuid"
        cat /proc/sys/kernel/random/uuid > ${PERSISTENCE_CONF_ROOT_PATH}/HostSN/HostSN
        chmod 650 "${PERSISTENCE_CONF_ROOT_PATH}/HostSN/HostSN"
    fi
    Log "Generating uuid end."

    # 3. mount log path
    sudo ${MOUNT_SCRIPT_PATH} mount_bind "${PERSISTENCE_CONF_ROOT_PATH}/HostSN/HostSN" "/etc/HostSN/HostSN"
    Log "Mount uuid end."
}

function ConfigLogPath()
{
    # 1. create log path
    if [ ! -d "${PERSISTENCE_LOG_ROOT_PATH}" ]; then
        echo "Agent root log path not exit."
        sudo ${PERMISSION_SCRIPT_PATH} mkdir "${PERSISTENCE_LOG_ROOT_PATH}"
        sudo ${PERMISSION_SCRIPT_PATH} chown root:nobody "${PERSISTENCE_LOG_ROOT_PATH}"
        sudo ${PERMISSION_SCRIPT_PATH} chmod 550  "${PERSISTENCE_LOG_ROOT_PATH}"
    fi

    if [ ! -d "${PERSISTENCE_LOG_ROOT_PATH}/log" ]; then
        echo "Agent log path not exit."
        sudo ${PERMISSION_SCRIPT_PATH} mkdir "${PERSISTENCE_LOG_ROOT_PATH}/log/Plugins/NasPlugin"
        sudo ${PERMISSION_SCRIPT_PATH} mkdir "${PERSISTENCE_LOG_ROOT_PATH}/log/Plugins/VirtualizationPlugin"
        sudo ${PERMISSION_SCRIPT_PATH} chown -R ${AGENT_USER}:${DEFAULT_GROUP_INTERNAL} "${PERSISTENCE_LOG_ROOT_PATH}/log"
        chmod -R 750 "${PERSISTENCE_LOG_ROOT_PATH}/log"
    fi

    if [ ! -d "${PERSISTENCE_LOG_ROOT_PATH}/log/Plugins/VirtualizationPlugin" ]; then
        sudo ${PERMISSION_SCRIPT_PATH} mkdir "${PERSISTENCE_LOG_ROOT_PATH}/log/Plugins/VirtualizationPlugin"
        sudo ${PERMISSION_SCRIPT_PATH} chown -R ${AGENT_USER}:${DEFAULT_GROUP_INTERNAL} "${PERSISTENCE_LOG_ROOT_PATH}/log/Plugins/VirtualizationPlugin"
        chmod -R 760 "${PERSISTENCE_LOG_ROOT_PATH}/log/Plugins/VirtualizationPlugin"
    fi

    # 2. mount log path
    sudo ${MOUNT_SCRIPT_PATH} mount_bind ${PERSISTENCE_LOG_ROOT_PATH}/log ${AGENT_ROOT_PATH}/log
    Log "Mount log end."

    # 3. create slog path
    if [ ! -d "${PERSISTENCE_LOG_ROOT_PATH}/slog" ]; then
        Log "Agent slog path not exit."
        sudo ${PERMISSION_SCRIPT_PATH} mkdir  "${PERSISTENCE_LOG_ROOT_PATH}/slog/Plugins/NasPlugin"
        sudo ${PERMISSION_SCRIPT_PATH} mkdir  "${PERSISTENCE_LOG_ROOT_PATH}/slog/Plugins/VirtualizationPlugin"
        sudo ${PERMISSION_SCRIPT_PATH} chmod -R 700 "${PERSISTENCE_LOG_ROOT_PATH}/slog"
        sudo ${PERMISSION_SCRIPT_PATH} chown -R root:root "${PERSISTENCE_LOG_ROOT_PATH}/slog"
    fi

    if [ ! -d "${PERSISTENCE_LOG_ROOT_PATH}/slog/Plugins/VirtualizationPlugin" ]; then
        sudo ${PERMISSION_SCRIPT_PATH} mkdir  "${PERSISTENCE_LOG_ROOT_PATH}/slog/Plugins/VirtualizationPlugin"
        sudo ${PERMISSION_SCRIPT_PATH} chmod 700 "${PERSISTENCE_LOG_ROOT_PATH}/slog/Plugins/VirtualizationPlugin"
    fi

    if [ ! -d "${PERSISTENCE_LOG_ROOT_PATH}/slog/Plugins/GeneralDBPlugin" ]; then
        sudo ${PERMISSION_SCRIPT_PATH} mkdir  "${PERSISTENCE_LOG_ROOT_PATH}/slog/Plugins/GeneralDBPlugin"
        sudo ${PERMISSION_SCRIPT_PATH} chmod 700 "${PERSISTENCE_LOG_ROOT_PATH}/slog/Plugins/GeneralDBPlugin"
    fi

    if [ ! -d "${PERSISTENCE_LOG_ROOT_PATH}/slog/Plugins/Block_Service" ]; then
        sudo ${PERMISSION_SCRIPT_PATH} mkdir  "${PERSISTENCE_LOG_ROOT_PATH}/slog/Plugins/Block_Service"
        sudo ${PERMISSION_SCRIPT_PATH} chmod 700 "${PERSISTENCE_LOG_ROOT_PATH}/slog/Plugins/Block_Service"
    fi

    if [ ! -d "${PERSISTENCE_LOG_ROOT_PATH}/slog/Plugins/ObsPlugin" ]; then
        sudo ${PERMISSION_SCRIPT_PATH} mkdir  "${PERSISTENCE_LOG_ROOT_PATH}/slog/Plugins/ObsPlugin"
        sudo ${PERMISSION_SCRIPT_PATH} chmod 700 "${PERSISTENCE_LOG_ROOT_PATH}/slog/Plugins/ObsPlugin"
    fi

    # 4. mount log path
    sudo ${MOUNT_SCRIPT_PATH} mount_bind ${PERSISTENCE_LOG_ROOT_PATH}/slog ${AGENT_ROOT_PATH}/slog
    Log "Mount slog end."
}

function ConfigNginxPath()
{
    Log "Start config nginx path."
    # 1. create /tmp/agent path
    if [ ! -d "${PERSISTENCE_TMP_ROOT_PATH}/nginx" ]; then
        Log "Nginx path not exist."
        sudo ${PERMISSION_SCRIPT_PATH} mkdir "${PERSISTENCE_TMP_ROOT_PATH}/nginx"
        sudo ${PERMISSION_SCRIPT_PATH} chown ${AGENT_USER}:${DEFAULT_GROUP_INTERNAL} "${PERSISTENCE_TMP_ROOT_PATH}/nginx"
        chmod 750 "${PERSISTENCE_TMP_ROOT_PATH}/nginx"
    fi

    # 2. copy nginx file
    cp -rf ${AGENT_ROOT_PATH}/nginx ${PERSISTENCE_TMP_ROOT_PATH} >/dev/null 2>&1

    # 3. mount nginx path
    sudo ${MOUNT_SCRIPT_PATH} mount_bind "${PERSISTENCE_TMP_ROOT_PATH}/nginx" ${AGENT_ROOT_PATH}/nginx

    # 4. create nginx log path
    if [ ! -d "${PERSISTENCE_LOG_ROOT_PATH}/nginx" ]; then
        Log "Agent nginx logs path not exist."
        sudo ${PERMISSION_SCRIPT_PATH} mkdir "${PERSISTENCE_LOG_ROOT_PATH}/nginx/logs"
        sudo ${PERMISSION_SCRIPT_PATH} chown -R ${AGENT_USER}:${DEFAULT_GROUP_INTERNAL} "${PERSISTENCE_LOG_ROOT_PATH}/nginx"
        chmod -R 750 "${PERSISTENCE_LOG_ROOT_PATH}/nginx"
    fi

    # 5. mount nginx logs path
    sudo ${MOUNT_SCRIPT_PATH} mount_bind "${PERSISTENCE_LOG_ROOT_PATH}/nginx/logs" "${AGENT_ROOT_PATH}/nginx/logs"
    Log "Mount nginx logs end."

    # 6. create nginx conf path
    if [ ! -d "${PERSISTENCE_CONF_ROOT_PATH}/nginx" ]; then
        sudo ${PERMISSION_SCRIPT_PATH} mkdir "${PERSISTENCE_CONF_ROOT_PATH}/nginx/conf"
        sudo ${PERMISSION_SCRIPT_PATH} chown -R ${AGENT_USER}:${DEFAULT_GROUP_INTERNAL} "${PERSISTENCE_CONF_ROOT_PATH}/nginx"
        chmod -R 750 "${PERSISTENCE_CONF_ROOT_PATH}/nginx"
    fi

    # 7. mount nginx conf path
    cp -rf "${AGENT_ROOT_PATH}/nginx/conf"/* "${PERSISTENCE_CONF_ROOT_PATH}/nginx/conf" >/dev/null 2>&1
    sudo ${MOUNT_SCRIPT_PATH} mount_bind "${PERSISTENCE_CONF_ROOT_PATH}/nginx/conf" "${AGENT_ROOT_PATH}/nginx/conf"
    Log "Mount nginx conf end."
}

function ConfigDBPath()
{
    Log "Start config db path."
    # 1. create db path
    if [ ! -d "${PERSISTENCE_CONF_ROOT_PATH}/db" ]; then
        Log "Agent db path not exit."
        sudo ${PERMISSION_SCRIPT_PATH} mkdir "${PERSISTENCE_CONF_ROOT_PATH}/db"
        sudo ${PERMISSION_SCRIPT_PATH} chmod 700 "${PERSISTENCE_CONF_ROOT_PATH}/db"
        sudo ${PERMISSION_SCRIPT_PATH} chown ${AGENT_USER}:${DEFAULT_GROUP_INTERNAL} "${PERSISTENCE_CONF_ROOT_PATH}/db"
        cp -rf -aux "${AGENT_ROOT_PATH}/db/"* "${PERSISTENCE_CONF_ROOT_PATH}/db" >/dev/null 2>&1
    else
        Log "Start upgrade database."
        sh "${AGENT_ROOT_PATH}/bin/agent_upgrade_sqlite.sh" "${PERSISTENCE_CONF_ROOT_PATH}/db" "${AGENT_ROOT_PATH}/db"
        if [ $? -ne 0 ]; then
            Log "Failed upgrade database."
            exit 1
        fi
        cp -rf "${AGENT_ROOT_PATH}/db/upgrade" "${PERSISTENCE_CONF_ROOT_PATH}/db"
    fi

    # 2. mount database path
    sudo ${MOUNT_SCRIPT_PATH} mount_bind ${PERSISTENCE_CONF_ROOT_PATH}/db ${AGENT_ROOT_PATH}/db
    Log "Mount db end"
}

function ConfigConfPath()
{
    Log "Start config conf path."
    # 1. create conf path
    if [ ! -d "${PERSISTENCE_CONF_ROOT_PATH}/conf" ]; then
        Log "Agent conf path not exit."
        sudo ${PERMISSION_SCRIPT_PATH} mkdir "${PERSISTENCE_CONF_ROOT_PATH}/conf"
        sudo ${PERMISSION_SCRIPT_PATH} chmod 750 "${PERSISTENCE_CONF_ROOT_PATH}/conf"
        sudo ${PERMISSION_SCRIPT_PATH} chown ${AGENT_USER}:${DEFAULT_GROUP_INTERNAL} "${PERSISTENCE_CONF_ROOT_PATH}/conf"
    else
        REGISTER_STATUS=`cat ${PERSISTENCE_CONF_ROOT_PATH}/conf/testcfg.tmp | grep "register_status" | awk -F '=' '{print $NF}'`
    fi

    sudo ${PERMISSION_SCRIPT_PATH} chown -R ${AGENT_USER}:${DEFAULT_GROUP_INTERNAL} "${VIRTUAL_PLUGIN_PATH}/conf"
    sudo ${PERMISSION_SCRIPT_PATH} chmod -R 750 "${VIRTUAL_PLUGIN_PATH}/conf"

    if [ ! -d "$PERSISTENCE_PLUGINS_ROOT_PATH/VirtualizationPlugin/conf" ]; then
        Log "Hcs Plugin conf path not exit."
        sudo ${PERMISSION_SCRIPT_PATH} mkdir "${PERSISTENCE_PLUGINS_ROOT_PATH}/VirtualizationPlugin/conf"
        sudo ${PERMISSION_SCRIPT_PATH} chmod 750 "${PERSISTENCE_PLUGINS_ROOT_PATH}/VirtualizationPlugin/conf"
        sudo ${PERMISSION_SCRIPT_PATH} chown -R ${AGENT_USER}:${DEFAULT_GROUP_INTERNAL} "${PERSISTENCE_PLUGINS_ROOT_PATH}/VirtualizationPlugin"
    fi
    sudo ${PERMISSION_SCRIPT_PATH} chmod -R 750 "${PERSISTENCE_PLUGINS_ROOT_PATH}/VirtualizationPlugin"
    # 2. mount conf path
    cp -rf ${AGENT_ROOT_PATH}/conf/* ${PERSISTENCE_CONF_ROOT_PATH}/conf >/dev/null 2>&1

    cp -rf ${VIRTUAL_PLUGIN_PATH}/conf/* ${PERSISTENCE_PLUGINS_ROOT_PATH}/VirtualizationPlugin/conf >/dev/null 2>&1

    echo "register_status=${REGISTER_STATUS}" >> ${PERSISTENCE_CONF_ROOT_PATH}/conf/testcfg.tmp
    sudo ${MOUNT_SCRIPT_PATH} mount_bind ${PERSISTENCE_CONF_ROOT_PATH}/conf ${AGENT_ROOT_PATH}/conf
    sudo ${MOUNT_SCRIPT_PATH} mount_bind ${PERSISTENCE_PLUGINS_ROOT_PATH}/VirtualizationPlugin/conf ${VIRTUAL_PLUGIN_PATH}/conf
    Log "Mount conf end."
}

function ConfigTmpPath()
{
    Log "Start config tmp path."

    if [ ! -d "${PERSISTENCE_TMP_ROOT_PATH}" ]; then
        Log "Agent tmp root path not exist"
        mkdir "${PERSISTENCE_TMP_ROOT_PATH}"
        chmod 700 "${PERSISTENCE_TMP_ROOT_PATH}"
    fi

    # 1. create tmp path
    if [ ! -d "${PERSISTENCE_TMP_ROOT_PATH}/tmp" ]; then
        Log "Agent tmp path not exist."
        sudo ${PERMISSION_SCRIPT_PATH} mkdir "${PERSISTENCE_TMP_ROOT_PATH}/tmp"
        sudo ${PERMISSION_SCRIPT_PATH} chmod 703 "${PERSISTENCE_TMP_ROOT_PATH}/tmp"
        sudo ${PERMISSION_SCRIPT_PATH} chmod +t "${PERSISTENCE_TMP_ROOT_PATH}/tmp"
        sudo ${PERMISSION_SCRIPT_PATH} chown root:root "${PERSISTENCE_TMP_ROOT_PATH}/tmp"
    fi

    # 2. mount tmp path
    sudo ${MOUNT_SCRIPT_PATH} mount_bind ${PERSISTENCE_TMP_ROOT_PATH}/tmp ${AGENT_ROOT_PATH}/tmp
    Log "Mount tmp end."
    
    # 3. create stmp path
    if [ ! -d "${PERSISTENCE_TMP_ROOT_PATH}/stmp" ]; then
        Log "Agent stmp path not exist."
        sudo ${PERMISSION_SCRIPT_PATH} mkdir "${PERSISTENCE_TMP_ROOT_PATH}/stmp"
        sudo ${PERMISSION_SCRIPT_PATH} chmod 705 "${PERSISTENCE_TMP_ROOT_PATH}/stmp"
        sudo ${PERMISSION_SCRIPT_PATH} chown root:root "${PERSISTENCE_TMP_ROOT_PATH}/stmp"
    fi
    
    # 4. mount tmp path
    sudo ${MOUNT_SCRIPT_PATH} mount_bind ${PERSISTENCE_TMP_ROOT_PATH}/stmp ${AGENT_ROOT_PATH}/stmp
    Log "Mount tmp end"
}

function ConfigPersistence()
{
    ### 1. create dir
    ## create log dir
    if [ ! -d "${PERSISTENCE_LOG_ROOT_PATH}" ]; then
        echo "Agent log root path not exit."
        sudo ${PERMISSION_SCRIPT_PATH} mkdir "${PERSISTENCE_LOG_ROOT_PATH}"

        sudo ${PERMISSION_SCRIPT_PATH} chmod 550 "${PERSISTENCE_LOG_ROOT_PATH}"
        sudo ${PERMISSION_SCRIPT_PATH} chown root:${DEFAULT_GROUP_INTERNAL} "${PERSISTENCE_LOG_ROOT_PATH}"
    fi

    ## create conf dir
    sudo ${PERMISSION_SCRIPT_PATH} mkdir "${PERSISTENCE_CONF_ROOT_PATH}"

    # /opt/logpath/protectagent/${PODE_NAME}/config
    sudo ${PERMISSION_SCRIPT_PATH} chown root:${DEFAULT_GROUP_INTERNAL} "${PERSISTENCE_CONF_ROOT_PATH}"
    sudo ${PERMISSION_SCRIPT_PATH} chmod 550 "${PERSISTENCE_CONF_ROOT_PATH}"

    # /opt/logpath/protectagent/${PODE_NAME}
    sudo ${PERMISSION_SCRIPT_PATH} chown root:${DEFAULT_GROUP_INTERNAL} "${PERSISTENCE_PODE_ROOT_PATH}"
    sudo ${PERMISSION_SCRIPT_PATH} chmod 550 "${PERSISTENCE_PODE_ROOT_PATH}"

    # /opt/logpath/protectagent
    sudo ${PERMISSION_SCRIPT_PATH} chown root:${DEFAULT_GROUP_INTERNAL} "${PERSISTENCE_PROTECTAGENT_ROOT_PATH}"
    sudo ${PERMISSION_SCRIPT_PATH} chmod 550 "${PERSISTENCE_PROTECTAGENT_ROOT_PATH}"


    ## create plugins dir
    if [ ! -d "${PERSISTENCE_PLUGINS_ROOT_PATH}" ]; then
        Log "Plugins root path not exit."
        sudo ${PERMISSION_SCRIPT_PATH} mkdir "${PERSISTENCE_PLUGINS_ROOT_PATH}"
        sudo ${PERMISSION_SCRIPT_PATH} chmod 550 "${PERSISTENCE_PLUGINS_ROOT_PATH}"
        sudo ${PERMISSION_SCRIPT_PATH} chown root:${DEFAULT_GROUP_INTERNAL} "${PERSISTENCE_PLUGINS_ROOT_PATH}"
    fi

    # 2. 
    ConfigLogPath

    # 5. 
    ConfigDBPath

    # 6. 
    ConfigConfPath

    # 7. 
    ConfigTmpPath

    # 8. 
    ConfigUUID

    # 9.
    ConfigNginxPath

    GetEnvironment

    ConfigHosts

    # 10. 
    GetNASIp

    # 11.
    ConfigHcsMapping
}

function RegisterHost()
{
    Log "Start register host."

    # 1. Setting the init tate
    sed -i "/register_status/s/.*/register_status=0/g" ${AGENT_ROOT_PATH}/conf/testcfg.tmp

    # 2. Invoke the register interface.
    while [ 1 ]
    do
        ${AGENT_ROOT_PATH}/bin/agentcli registerHost RegisterHost
        if [ $? -ne 0 ]; then
            Log "Register host to ProtectManager failed."
            REGISTER_STATUS=${REGISTER_STATUS_FAIL}
            sh ${AGENT_ROOT_PATH}/bin/agent_stop.sh
        else
            Log "Register host to ProtectManager successfully."
            REGISTER_STATUS=${REGISTER_STATUS_SUCC}
            sleep 5
            break
        fi
        sleep 30
    done

    # 3. Change the registration status.
    sh ${AGENT_ROOT_PATH}/bin/agent_stop.sh
    sed -i "/register_status/s/.*/register_status=${REGISTER_STATUS}/g" ${AGENT_ROOT_PATH}/conf/testcfg.tmp

    Log "Register host end."
}

function UpdateServer()
{
    Log "Start notify manager server."
    if [ ${ENVIRONMENT_TYPE} -eq ${ENVIRONMENT_TYPE_GENERAL} ]; then
        Log "The common server does not support updating the PM IP address."
        return
    fi

    python ${AGENT_ROOT_PATH}/bin/update_cluster.pyc
    if [ $? -ne 0 ]; then
        Log "Update server failed."
        exit 1
    fi

    Log "Update server succ."
}

function ConfigHosts()
{
    Log "Start config hosts."
    # 1. 
    if [ -f "${PERSISTENCE_TMP_ROOT_PATH}/hosts" ]; then
        sudo ${PERMISSION_SCRIPT_PATH} chown ${AGENT_USER}:${DEFAULT_GROUP_INTERNAL} "${PERSISTENCE_TMP_ROOT_PATH}/hosts"
        rm -rf "${PERSISTENCE_TMP_ROOT_PATH}/hosts"
    fi

    # 2. 
    cp /etc/hosts "${PERSISTENCE_TMP_ROOT_PATH}/hosts"
    chmod 644 "${PERSISTENCE_TMP_ROOT_PATH}/hosts"
    sudo ${PERMISSION_SCRIPT_PATH} chown root:root "${PERSISTENCE_TMP_ROOT_PATH}/hosts"
    sudo ${MOUNT_SCRIPT_PATH} mount_bind "${PERSISTENCE_TMP_ROOT_PATH}/hosts" "/etc/hosts"
}

function GetNASIp()
{
    Log "Start get nas ip."
    if [ ${ENVIRONMENT_TYPE} -ne ${ENVIRONMENT_TYPE_DORADO} ]; then
        Log "Environment type is not dorado, will not get nas ip."
        return
    fi

    # 2. 普通容器使用127.0.0.1进行挂载
    sudo ${PERMISSION_SCRIPT_PATH} chown ${AGENT_USER}:${DEFAULT_GROUP_INTERNAL} "${PERSISTENCE_TMP_ROOT_PATH}/hosts"
    echo "127.0.0.1 nas.storage.protectengine_a.host" >> "${PERSISTENCE_TMP_ROOT_PATH}/hosts"

    # 3. 
    sudo ${PERMISSION_SCRIPT_PATH} chown root:root "${PERSISTENCE_TMP_ROOT_PATH}/hosts"
}

function ConfigHcsMapping()
{
    if [ ! -f "${PERSISTENCE_CONF_ROOT_PATH}/conf/hcs_domain" ];then
        Log "The hcs domain name does not need to be configured."
        return
    fi

    Log "Start config hcs domain."
    sudo "${PERMISSION_SCRIPT_PATH}" chown "${AGENT_USER}:${DEFAULT_GROUP_INTERNAL}" "${PERSISTENCE_TMP_ROOT_PATH}/hosts"

    echo "`cat ${PERSISTENCE_CONF_ROOT_PATH}/conf/hcs_domain`" >> "${PERSISTENCE_TMP_ROOT_PATH}/hosts"

    sudo ${PERMISSION_SCRIPT_PATH} chown root:root "${PERSISTENCE_TMP_ROOT_PATH}/hosts"
}

function InitIscsiName()
{
    sudo "${PERMISSION_SCRIPT_PATH}" chown "${AGENT_USER}:${DEFAULT_GROUP_INTERNAL}" "/etc/iscsi/initiatorname.iscsi"
    # 初始化iscsi名称，防止两控内置agent的iscsi名称相同
    iscsi_name=`sed -n "/^InitiatorName/p"  /etc/iscsi/initiatorname.iscsi|cut -d'=' -f2`
    echo "" >  /etc/iscsi/initiatorname.iscsi

    host_uuid=`cat ${PERSISTENCE_CONF_ROOT_PATH}/HostSN/HostSN`
    echo "InitiatorName=${iscsi_name}:${host_uuid}" > /etc/iscsi/initiatorname.iscsi

    sudo "${PERMISSION_SCRIPT_PATH}" chown root:root "/etc/iscsi/initiatorname.iscsi"

    Log "Init iscsi name finished."
}

function ChangePrivilegeForGroup() {
    Log "Start to change privilege for exrdadmin."
    chmod -R g+rx ${AGENT_ROOT_PATH}/conf
    chmod -R g+rx ${AGENT_ROOT_PATH}/nginx
    chmod -R 770 ${AGENT_ROOT_PATH}/log/Plugins/VirtualizationPlugin
    chmod -R 640 ${PERSISTENCE_CONF_ROOT_PATH}/nginx/conf/*
    Log "Change privilege for exrdadmin finished."
}

function DoSetCapsForVirtualization()
{
    Log "Start to do set caps for virtualization."
    sudo ${PERMISSION_SCRIPT_PATH} chmod g+rx -R ${EXAGENT_USER}:${DEFAULT_GROUP_INTERNAL} "${VIRTUAL_PLUGIN_PATH}"
    sudo ${VIRTUAL_PLUGIN_PATH}/install/sudo_set_caps.sh
    Log "Set caps for virtualization finished."
}

function ConfigDomainName()
{
    Log "Start config domain name."
    DomainName=`${AGENT_ROOT_PATH}/bin/openssl x509 -subject -in ${AGENT_ROOT_PATH}/nginx/conf/server.pem -noout | ${AWK} -F '=' '{print $NF}'`
    if [ -z "${DomainName}" ];then
        Log "Get ssl domain failed."
        exit 1
    fi

    tmpDomainName=`cat /etc/hosts | grep ${DomainName}`
    if [ -z "${tmpDomainName}" ]; then
        sudo ${PERMISSION_SCRIPT_PATH} chown ${AGENT_USER}:${DEFAULT_GROUP_INTERNAL} "${PERSISTENCE_TMP_ROOT_PATH}/hosts"
        echo "127.0.0.1 ${DomainName}" >> "/etc/hosts"
        sudo ${PERMISSION_SCRIPT_PATH} chown root:root "${PERSISTENCE_TMP_ROOT_PATH}/hosts"
    fi
}

function ConfigDns()
{
    Log "Start config dns."
    cp -f /etc/resolv.conf "${PERSISTENCE_TMP_ROOT_PATH}/resolv.conf"
    if [ "${DEPLOY_TYPE}" != "${DEPLOY_TYPE_CYBER_ENGINE}" ]; then
        sed -i "s/nameserver.*/nameserver ${DME_DNS_SRV_SERVICE_HOST}/" "${PERSISTENCE_TMP_ROOT_PATH}/resolv.conf"
    fi
    chmod 644 "${PERSISTENCE_TMP_ROOT_PATH}/resolv.conf"
    sudo ${PERMISSION_SCRIPT_PATH} chown root:root "${PERSISTENCE_TMP_ROOT_PATH}/resolv.conf"
    sudo ${MOUNT_SCRIPT_PATH} mount_bind "${PERSISTENCE_TMP_ROOT_PATH}/resolv.conf" "/etc/resolv.conf"
}

WriteParams()
{
    # testcfg.tmp
    echo "NODE_NAME=${NODE_NAME}" >> "${AGENT_ROOT_PATH}/conf/testcfg.tmp"
    echo "PODE_NAME=${PODE_NAME}" >> "${AGENT_ROOT_PATH}/conf/testcfg.tmp"
    echo "POD_IP=${POD_IP}" >> "${AGENT_ROOT_PATH}/conf/testcfg.tmp"

    # xmlxfg
    ${AGENT_ROOT_PATH}/bin/xmlcfg write Backup backup_scene ${BACKUP_SCENE}
    ${AGENT_ROOT_PATH}/bin/xmlcfg write System deploy_type "${DEPLOY_TYPE}"

    if [ "${DEPLOY_TYPE}" = "${DEPLOY_TYPE_X6000}" ]; then
        ${AGENT_ROOT_PATH}/bin/xmlcfg write Frame sub_job_cnt_max_inner ${X6000_JOB_MAX}
    fi

    if [ "${DEPLOY_TYPE}" = "${DEPLOY_TYPE_X3000}" ]; then
        ${AGENT_ROOT_PATH}/bin/xmlcfg write Frame main_job_cnt_max_inner ${X3000_NOT_INFRA_MAIN_JOB_MAX}
        ${AGENT_ROOT_PATH}/bin/xmlcfg write Frame sub_job_cnt_max_inner ${X3000_SUB_JOB_MAX}
    fi
}

function ConfigInternalProxy()
{
    echo "ConfigInternalProxy."
    # 1. get scene
    GetBackupScene
    if [ ${BACKUP_SCENE} -ne ${BACKUP_SCENE_INTERNAL} ]; then
        echo "Not internal agent."
        exit 1
    fi
    # 2. 
    ConfigPersistence

    WriteParams

    # 3. ip
    ConfigIp

    # 5. Cert
    ConfigCert

    # 6. 
    ConfigDomainName

    # 7. register
    RegisterHost

    # 8. configer dns
    ConfigDns

    # 9.init iscsi name
    InitIscsiName

    Log "ConfigInternalProxy succ."
}

function StartIscsi()
{
    Log "Start iscsi service."
    sudo ${MOUNT_SCRIPT_PATH} mount_iscsi

    sudo iscsid -f >> "${AGENT_ROOT_PATH}/log/iscsi_service.log" 2>&1 &

    #  适配安全容器内无法查询挂载卷的情况
    sudo "${PERMISSION_SCRIPT_PATH}" chown "${AGENT_USER}:${DEFAULT_GROUP_INTERNAL}" "/sys/module/scsi_mod/parameters/scan"
    echo "async" > /sys/module/scsi_mod/parameters/scan
    sudo "${PERMISSION_SCRIPT_PATH}" chown root:root "/sys/module/scsi_mod/parameters/scan"
    Log "Iscsi service start end."
}

function StartAgent()
{
    Log "Start agent."
    while [ 1 ]
    do
        sh ${AGENT_ROOT_PATH}/bin/agent_start.sh
        Log "Start agent fail."
        sh ${AGENT_ROOT_PATH}/bin/agent_stop.sh
        sleep 10
    done
}

function ConfigCgroupDevice()
{
    sudo ${MOUNT_SCRIPT_PATH} mount_cgroup_device
}

function main()
{
    GetEnvironment

    # ARM GENERAL ENV WAIT 2 min
    if [ ${ENVIRONMENT_TYPE} -eq ${ENVIRONMENT_TYPE_GENERAL} ]; then
        sleep 120
    fi

    CheckShellType

    ConfigInternalProxy

    ConfigCgroupDevice

    StartIscsi

    ChangePrivilegeForGroup

    DoSetCapsForVirtualization

    StartAgent
}

main

exit 0
