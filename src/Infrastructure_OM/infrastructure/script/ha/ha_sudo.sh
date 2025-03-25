#!/bin/bash
#########################################
# Copyright (c) 2012-2015 Huawei .
# All rights reserved.
#
# Function
# revise note
########################################

set +x
WORK_PATH=$(cd "$(dirname "$BASH_SOURCE")";pwd)
G_HA_SCRIPT_PATH=/usr/local/ha/script
if [ -L "${G_HA_SCRIPT_PATH}/log.sh" ];then
    log_error "[log's symbol link is not allowed."
    return 1
fi
source ${G_HA_SCRIPT_PATH}/log.sh
HAInstallPath=/usr/local/ha
HA_CONF_CTL="${HAInstallPath}/module/hacom/script/config_ha.sh"
HA_STATUS_CTL="${HAInstallPath}/module/hacom/script/status_ha.sh"
HA_START_CTL="${HAInstallPath}/module/hacom/script/start_ha.sh"
HA_STOP_CTL="${HAInstallPath}/module/hacom/script/stop_ha.sh"
HA_QUERY_FSW_CTL="${HAInstallPath}/module/hacom/script/query_fsw.sh"
HA_CLIENT_TOOL="${HAInstallPath}/module/hacom/tools/ha_client_tool"
HA_GET_ROLE_CTL="${HAInstallPath}/module/hacom/script/get_harole.sh"
HA_COM_SCRIPT_PATH=${HAInstallPath}/module/hacom/script
HA_CERT_CTL="${HAInstallPath}/module/hacom/script/gen-cert.sh"
HA_CERT_PATH=${HAInstallPath}/local/cert
SERVER_KEY_FILE=${HA_CERT_PATH}/server.key
OPENSSL_CNF=${G_HA_SCRIPT_PATH}/conf/openssl.cnf
OPENSSL_TMP_CNF=${HA_CERT_PATH}/extfile.cnf
OPENAPI_CA_PATH=/opt/OceanProtect/protectmanager/cert/CA/certs

DB_USER=gaussdbremote
CONTAINER_USER=GaussDB
DBInstallPath=/usr/local/gaussdb
PGDATA=${DBInstallPath}/data
PGCTL="${DBInstallPath}/app/bin/gs_ctl -D ${PGDATA}"
PGCONF=${DBInstallPath}/data/postgresql.conf
PG_HBA_CONF=${DBInstallPath}/data/pg_hba.conf
G_IPV6=1  #1 is ipv4
G_IFCONFIG="$(which ifconfig)"
G_IP="$(which ip)"
G_NDSEND=""
G_ARPING="$(which arping)"

master=/opt/OceanProtect/protectmanager/kmc/master.ks
backup=/kmc_conf/..data/backup.ks
gauss_pwd_field=database.superPassword

CLUSTER_ROLE_PRIMARY="PRIMARY"
CLUSTER_ROLE_Standby="STANDBY"

function ip_flag_check() {
    if [ $# -ne 1 ] ;then
       return 1
    fi
    local local_ip="$1"
    if [ "$1" != "${1#*[0-9].[0-9]}" ]; then
       G_IPV6=1
       return 0
    elif [ "$1" != "${1#*:[0-9a-fA-F]}" ]; then
       G_IPV6=0
       G_NDSEND="$(which ndsend)"
       return 0
    else
      return 1
    fi
}

function ip_type_check ()
{
    if [ $# -ne 1 ] && [ $# -ne 2 ] ;then
       return 1
    fi
    local local_ip="$1"
    local exclude_ip="$2"

    # It does not include the network address of the D and E
    if  echo ${local_ip} | xargs echo -n | grep -E '^(22[0-3]|2[0-1][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])\.(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])\.(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])\.(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])$' >/dev/null 2>&1 ; then
        if [[ "X${local_ip}" != "X0.0.0.0" ]] &&\
           [[ "X${local_ip}" != "X255.255.255.255" ]];then
            return 0
        else
            if [ -n "${exclude_ip}" ]; then
                return 0
            fi
            return 1
        fi
    elif echo ${local_ip} | xargs echo -n | grep -E '^((([0-9a-fA-F]{1,4}:){7}([0-9A-Fa-f]{1,4}|:))|(([0-9a-fA-F]{1,4}:){6}((([0-9]{1,3}\.){3}[0-9]{1,4})|([0-9a-fA-F]{1,4})|:))|(([0-9a-fA-F]{1,4}:){5}:((([0-9a-fA-F]{1,4})?)|([0-9a-fA-F]{1,4}:[0-9a-fA-F]{1,4})|(([0-9]{1,3}\.){3}[0-9]{1,4})))|(([0-9a-fA-F]{1,4}:){4}:((([0-9a-fA-F]{1,4})?)|(([0-9a-fA-F]{1,4}:){1,2}[0-9a-fA-F]{1,4})|(([0-9a-fA-F]{1,4}:)?([0-9]{1,3}\.){3}[0-9]{1,4})))|(([0-9a-fA-F]{1,4}:){3}:((([0-9a-fA-F]{1,4}:){0,2}([0-9]{1,3}\.){3}[0-9]{1,4})|(([0-9a-fA-F]{1,4}:){0,3}[0-9a-fA-F]{1,4})|$))|(([0-9a-fA-F]{1,4}:){2}:((([0-9a-fA-F]{1,4}:){0,3}([0-9]{1,3}\.){3}[0-9]{1,4})|(([0-9a-fA-F]{1,4}:){0,4}[0-9a-fA-F]{1,4})|$))|(([0-9a-fA-F]{1,4}:){1}:((([0-9a-fA-F]{1,4}:){0,4}([0-9]{1,3}\.){3}[0-9]{1,4})|(([0-9a-fA-F]{1,4}:){0,5}[0-9a-fA-F]{1,4})|$))|(::((([0-9a-fA-F]{1,4}:){0,5}([0-9]{1,3}\.){3}[0-9]{1,4})|(([0-9a-fA-F]{1,4}:){0,6}[0-9a-fA-F]{1,4})|$)))$' >/dev/null 2>&1;then
    if [[ "X${local_ip}" != "X::" ]] &&\
           [[ "X${local_ip}" != "X::FF:FF" ]] &&\
           [[ "X${local_ip}" != "X::ff:ff" ]] &&\
           [[ "X${local_ip}" != "X::1" ]];then
            return 0
        else
            if [ -n "${exclude_ip}" ]; then
                return 0
            fi
            return 1
        fi

    else
        return 1
    fi
}

#just sport ipv4
function ip_mask_check ()
{
    if [ $# -ne 1 ];then
       return 1
    fi
    local local_ip="$1"

    if  echo ${local_ip} | xargs echo -n | grep -E '^(([0-9][0-9][0-9]|[0-9][0-9]|[0-9])\.?){4}$' >/dev/null 2>&1 ; then
        return 0
    elif echo ${local_ip}| xargs echo -n  | grep -P '^\d{1,}$' >/dev/null 2>&1 ; then
        return 0
    fi
    return 1
}

function float_ip_exist_check ()
{
    local float_ip="$1"
    if ${G_IP} addr 2> /dev/null | grep $float_ip ; then
        return 0
    fi
    return 1
}

function set_floatip()
{
    log_info "[${FUNCNAME[0]},$LINENO] begin set_floatIP"

    if [ $# -ne 4 ];then
       return 1
    fi

    local network_name=$1
    local network_ip=$2
    local route_table=$4

    ip_type_check ${network_ip}
    if [ $? -ne 0 ];then
        log_error "[${FUNCNAME[0]},$LINENO] ${network_ip} is not ip."
        return 1
    fi

    ip_flag_check ${network_ip}
    if [ $? -ne 0 ];then
        log_error "[${FUNCNAME[0]},$LINENO] ${network_ip} is not valid ip."
        return 1
    fi

    float_ip_exist_check $network_ip
    if [ $? -eq 0 ];then
        log_info "[${FUNCNAME[0]},$LINENO] ${network_ip} exist, no need to add again."
        return 0
    fi

    if [ $G_IPV6 -eq 1 ];then
        local network_ip_mask=$3
        local parent_name=$(echo "${network_name}" | awk -F':' '{print $1}')
        ${G_IFCONFIG} | grep $parent_name
        if [ $? -ne 0 ];then
            log_error "[${FUNCNAME[0]},$LINENO] Param error: ${parent_name}"
            return 1
        fi

        ip_mask_check ${network_ip_mask}
        if [ $? -ne 0 ];then
            log_error "[${FUNCNAME[0]},$LINENO] ${network_ip_mask} is not ip."
            return 1
        fi

        ${G_IP} addr add "${network_ip}/${network_ip_mask}" dev ${network_name} 2> /dev/null
        if [ $? -ne 0 ];then
            log_error "[${FUNCNAME[0]},$LINENO] set float ip ${network_ip} failed!"
            return 1
        fi

        sudo ip rule add from ${network_ip} lookup ${route_table}
        ${G_IP} route add local ${network_ip} dev ${network_name} scope host src ${network_ip} table ${route_table}
        log_info "[${FUNCNAME[0]},$LINENO] try to flush arp!"
        local manage_device_name=`echo ${network_name} 2>/dev/null|awk -F: '{print $1}'`
        ${G_ARPING} -c 1 -A -I ${manage_device_name} ${network_ip} >/dev/null 2>&1
        if [ $? -ne 0 ];then
            log_error "[${FUNCNAME[0]},$LINENO]failed to flush arp for float ip!"
        else
            log_info "[${FUNCNAME[0]},$LINENO]succeed to flush arp for float ip!"
        fi
    else
        ${G_IP} -6 addr add "${network_ip}/64" dev ${network_name} 2> /dev/null
        if [ $? -ne 0 ];then
            log_error "[${FUNCNAME[0]},$LINENO] set float ip ${network_ip} failed!"
            return 1
        fi
        sudo ip -6 rule add from ${network_ip} lookup ${route_table}
        ${G_IP} -6 route add local ${network_ip} dev ${network_name} scope host src ${network_ip} table ${route_table}
        ${G_NDSEND} ${network_ip} ${network_name} >/dev/null 2>&1
        if [ $? -ne 0 ];then
           log_error "[${FUNCNAME[0]},$LINENO] set arbitration failed to flush ndsend for float ip!"
        else
           log_info "[${FUNCNAME[0]},$LINENO] set arbitration succeed to flush ndsend for float ip!"
        fi
    fi

    if [ $? -eq 0 ];then
        log_info "[${FUNCNAME[0]},$LINENO] set_floatIP success! ${network_name}"
    else
        log_error "[${FUNCNAME[0]},$LINENO] set_floatIP failed! ${network_name}"
        return 1
    fi
    return 0
}

function unset_floatip()
{
    log_info "[${FUNCNAME[0]},$LINENO] begin unset_floatIP !"

    if [[ ${DEPLOY_TYPE} == "d8" ]]; then
        log_info "[${FUNCNAME[0]}(),$LINENO] no need to set float ip in databackup"
        return 0
    fi

    if [ $# -ne 1 ];then
       return 1
    fi

    local network_ip=$1

    ip_flag_check $network_ip
    if [ $? -ne 0 ];then
        log_error "[${FUNCNAME[0]},$LINENO] $network_ip is not valid ip."
        return 1
    fi

    float_ip_exist_check $network_ip
    if [ $? -ne 0 ];then
        log_info "[${FUNCNAME[0]},$LINENO] ${network_ip} does not exist, no need to delete again."
        return 0
    fi

    local network_name=`${G_IP} a 2> /dev/null | grep -n $network_ip | awk '{print $NF}' |cut -d ":" -f 1`
    local route_table=`ip rule | grep "from ${network_ip} lookup" |  awk -F " " {'print $5'}`
    local network_ip_mask=`${G_IP} addr 2> /dev/null | grep $network_ip | awk '{print $2}' | awk -F '/' '{print $2}'`

    if [ $G_IPV6 -eq 1 ]
    then
        ${G_IP} addr del "${network_ip}/${network_ip_mask}" dev ${network_name} 2> /dev/null
        sudo ip rule del from $network_ip
        ${G_IP} route del local ${network_ip} dev ${network_name} scope host src ${network_ip} table ${route_table}
    else
        ${G_IP} -6 addr del "${network_ip}/64" dev ${network_name} 2> /dev/null
        sudo ip -6 rule del from $2=network_ip
        ${G_IP} -6 route del local ${network_ip} dev ${network_name} scope host src ${network_ip} table ${route_table}
    fi
    if [ $? -eq 0 ];then
        log_info "[${FUNCNAME[0]},$LINENO] unset_floatIP success!"
    else
        log_error "[${FUNCNAME[0]},$LINENO] unset_floatIP failed!"
    fi
    return 0
}

function get_db_password()
{
    cd "${WORK_PATH}"
    echo $(python3 -c "import logging; from manage_db_data import *; initialize_kmc('${master}', '${backup}', get_logger(log_file, logging.ERROR)); print(get_db_pwd_from_api('${gauss_pwd_field}'))")
}

function change_cluster_role() {
    cd "${WORK_PATH}"
    log_info "[${FUNCNAME[0]},$LINENO] begin change cluster role $1."
    local role="$1"
    if [[ "$role" == "${CLUSTER_ROLE_Standby}" || "$role" == "${CLUSTER_ROLE_PRIMARY}" ]]; then
      local ret=$(python3 -c "import logging; from manage_cluster import *; initialize_kmc('${master}', '${backup}', get_logger(log_file, logging.ERROR)); print(change_cluster_role('${role}'))")
      log_info "[${FUNCNAME[0]},$LINENO] change cluster role result $ret."
      return $ret
    fi
    log_error "invalid role:'$role', failed!"
}

function ha_config() {
    local type=$1
    if [ "$type" == "link" ]; then
        su nobody -s /bin/bash -c "$HA_CONF_CTL -m $2 -l $3 -p $4 -b $5 -s $6 -i $7 -g $8 -j $9"
        return $?
    elif [ "$type" == "log" ]; then
        su nobody -s /bin/bash -c "$HA_CONF_CTL -f $2"
        su nobody -s /bin/bash -c "$HA_CONF_CTL -k $2"
        su nobody -s /bin/bash -c "$HA_CONF_CTL -o $2"
        return $?
    elif [ "$type" == "floatIp" ]; then
        su nobody -s /bin/bash -c "$HA_CONF_CTL -i $2"
        return $?
    elif [ "$type" == "gatewayIp" ]; then
        su nobody -s /bin/bash -c "$HA_CONF_CTL -g $2"
        return $?
    elif [ "$type" == "cert" ]; then
        if [ "$2" == "active" ]; then
            # 判断证书文件是否存在软链接
            stat $HA_CERT_PATH/* | grep -q "symbolic link"
            if [ $? -eq 0 ];then
                log_error "Dir: ${HA_CERT_PATH} has symbol link."
                return 1
            fi
            # 保存之前的证书，用于失败后回退
            if [ -f "$HA_CERT_PATH/root-ca.crt" ]; then
                cp -f $HA_CERT_PATH/root-ca.crt $HA_CERT_PATH/root-ca.crt.old
            fi
            if [ -f "$HA_CERT_PATH/root-ca.pem" ]; then
                cp -f $HA_CERT_PATH/root-ca.pem $HA_CERT_PATH/root-ca.pem.old
            fi
            if [ -f "$HA_CERT_PATH/root.key" ]; then
                cp -f $HA_CERT_PATH/root.key $HA_CERT_PATH/root.key.old
            fi
            if [ -f "$HA_CERT_PATH/server.crt" ]; then
                cp -f $HA_CERT_PATH/server.crt $HA_CERT_PATH/server.crt.old
            fi
            if [ -f "$HA_CERT_PATH/server.pem" ]; then
                cp -f $HA_CERT_PATH/server.pem $HA_CERT_PATH/server.pem.old
            fi
            if [ -f "$HA_CERT_PATH/server.key" ]; then
                cp -f $HA_CERT_PATH/server.key $HA_CERT_PATH/server.key.old
            fi
            chown nobody:nobody $HA_CERT_PATH -h -R
            # 创建根证书
            # 生成随机密码,密码中至少包含三种字符
            while true; do
                local password=$(openssl rand -base64 8)
                if [[ $(echo "$password" | grep -c '[a-z]') -ge 1 && \
                      $(echo "$password" | grep -c '[A-Z]') -ge 1 && \
                      $(echo "$password" | grep -c '[0-9]') -ge 1 ]]; then
                    break
                fi
            done
            res=$(su nobody -s /bin/bash -c "cd $HA_COM_SCRIPT_PATH;./gen-cert.sh --root-ca --country=CN --state=SC --city=CD --company=HuaWei --organize=OP --common-name=HA --email=abc@example.com --config=$OPENSSL_CNF" >/dev/null 2>&1<< EOF
${password}
EOF
)

            # 创建用户证书
            # 加密后的密钥取标准输出的第17个空格符后内容
            # gen-cert脚本的无用打印输出到了stderr中
            local ip_dns="IP:127.0.0.1,IP:$3,IP:$4"
            res=$(su nobody -s /bin/bash -c "cd $HA_COM_SCRIPT_PATH;./gen-cert.sh --server-ca --country=CN --state=SC --city=CD --company=HuaWei --organize=OP --common-name=HA-server --IP-DNS='$ip_dns' --email=abc@example.com --config=$OPENSSL_CNF --tmp-config=$OPENSSL_TMP_CNF" 2>/dev/null<< EOF
${password}
${password}
EOF
)
            unset password
            rm -f $OPENSSL_TMP_CNF
            rm -f $HA_CERT_PATH/server.csr
            rm -f $HA_CERT_PATH/root-ca.srl
            server_key=$(echo $res | cut -d ' ' -f 17)
            if [ ! -f "${SERVER_KEY_FILE}" ]; then
                touch "${SERVER_KEY_FILE}"
                if [ -L "${SERVER_KEY_FILE}" ];then
                    log_error "${SERVER_KEY_FILE} with symbol link is not allowed."
                    return 1
                fi
                chown nobody:nobody ${SERVER_KEY_FILE}
                chmod 600 ${SERVER_KEY_FILE}
            fi
            if [ -L "${SERVER_KEY_FILE}" ];then
                log_error "${SERVER_KEY_FILE} with symbol link is not allowed."
                return 1
            fi
            echo $server_key > ${SERVER_KEY_FILE}
        else
            stat $HA_CERT_PATH/tmp/* | grep -q "symbolic link"
            if [ $? -eq 0 ];then
                log_error "Dir: ${HA_CERT_PATH}/tmp has symbol link."
                return 1
            fi
            if [ -d "$HA_CERT_PATH/tmp" ]; then
                cp -f $HA_CERT_PATH/tmp/root-ca.crt $HA_CERT_PATH/root-ca.crt
                cp -f $HA_CERT_PATH/tmp/root-ca.pem $HA_CERT_PATH/root-ca.pem
                cp -f $HA_CERT_PATH/tmp/root.key $HA_CERT_PATH/root.key
                cp -f $HA_CERT_PATH/tmp/server.crt $HA_CERT_PATH/server.crt
                cp -f $HA_CERT_PATH/tmp/server.pem $HA_CERT_PATH/server.pem
                cp -f $HA_CERT_PATH/tmp/server.key $HA_CERT_PATH/server.key
                cp -f $HA_CERT_PATH/tmp/root-ca.crt.old $HA_CERT_PATH/root-ca.crt.old
                cp -f $HA_CERT_PATH/tmp/root-ca.pem.old $HA_CERT_PATH/root-ca.pem.old
                cp -f $HA_CERT_PATH/tmp/root.key.old $HA_CERT_PATH/root.key.old
                cp -f $HA_CERT_PATH/tmp/server.crt.old $HA_CERT_PATH/server.crt.old
                cp -f $HA_CERT_PATH/tmp/server.pem.old $HA_CERT_PATH/server.pem.old
                cp -f $HA_CERT_PATH/tmp/server.key.old $HA_CERT_PATH/server.key.old
                chown nobody:nobody $HA_CERT_PATH -h -R
            fi
            rm -rf $HA_CERT_PATH/tmp
            local server_key=$(cat ${SERVER_KEY_FILE})
        fi
        su nobody -s /bin/bash -c "LD_PRELOAD=/usr/lib64/libSecurityStarter.so $HA_CONF_CTL -S ssl=true,twoway=true,keypass=$server_key between@keypass="
        return $?
    elif [ "$type" == "remove_cert" ]; then
        # 删除HA后删除之前的证书
        if [ -f "$HA_CERT_PATH/root-ca.crt" ]; then
            rm -f $HA_CERT_PATH/root-ca.crt
        fi
        if [ -f "$HA_CERT_PATH/root-ca.pem" ]; then
            rm -f $HA_CERT_PATH/root-ca.pem
        fi
        if [ -f "$HA_CERT_PATH/root.key" ]; then
            rm -f $HA_CERT_PATH/root.key
        fi
        if [ -f "$HA_CERT_PATH/server.crt" ]; then
            rm -f $HA_CERT_PATH/server.crt
        fi
        if [ -f "$HA_CERT_PATH/server.pem" ]; then
            rm -f $HA_CERT_PATH/server.pem
        fi
        if [ -f "$HA_CERT_PATH/server.key" ]; then
            rm -f $HA_CERT_PATH/server.key
        fi
        su nobody -s /bin/bash -c "$HA_CONF_CTL -S ssl=false"
        return $?
    elif [ "$type" == "cert_rollback" ]; then
        if [ -f "$HA_CERT_PATH/root-ca.crt.old" ]; then
            mv -f $HA_CERT_PATH/root-ca.crt.old $HA_CERT_PATH/root-ca.crt
        fi
        if [ -f "$HA_CERT_PATH/root-ca.pem.old" ]; then
            mv -f $HA_CERT_PATH/root-ca.pem.old $HA_CERT_PATH/root-ca.pem
        fi
        if [ -f "$HA_CERT_PATH/root.key.old" ]; then
            mv -f $HA_CERT_PATH/root.key.old $HA_CERT_PATH/root.key
        fi
        if [ -f "$HA_CERT_PATH/server.crt.old" ]; then
            mv -f $HA_CERT_PATH/server.crt.old $HA_CERT_PATH/server.crt
        fi
        if [ -f "$HA_CERT_PATH/server.pem.old" ]; then
            mv -f $HA_CERT_PATH/server.pem.old $HA_CERT_PATH/server.pem
        fi
        if [ -f "$HA_CERT_PATH/server.key.old" ]; then
            mv -f $HA_CERT_PATH/server.key.old $HA_CERT_PATH/server.key
        fi
        chown nobody:nobody $HA_CERT_PATH/ -h -R
        local server_key=$(cat ${SERVER_KEY_FILE})
        su nobody -s /bin/bash -c "$HA_CONF_CTL -S ssl=true,twoway=true,keypass=$server_key"
        return $?
    else
        return 1
    fi
}

function ha_status() {
    su nobody -s /bin/bash -c "$HA_STATUS_CTL"
}

function ha_start() {
    su nobody -s /bin/bash -c "source ${G_HA_SCRIPT_PATH}/event_lib.sh;add_ha_ip_rule"
    su nobody -s /bin/bash -c "$HA_START_CTL"
}

function ha_stop() {
    su nobody -s /bin/bash -c "$HA_STOP_CTL"
}

function ha_query_fsw() {
    su nobody -s /bin/bash -c "$HA_QUERY_FSW_CTL"
}

function ha_forbid_switchover() {
    local process_name=$1
    su nobody -s /bin/bash -c "timeout 5s $HA_CLIENT_TOOL --forbidswitch --name=$process_name --time=10"
}

function ha_allow_switchover() {
    local process_name=$1
    su nobody -s /bin/bash -c "timeout 5s $HA_CLIENT_TOOL --cancelforbidswitch --name=$process_name"
}

function ha_role() {
    local role_num=$(su nobody -s /bin/bash -c "sed -n 's/.*harole value=\"\(.*\)\".*/\1/p' ${HAInstallPath}/local/haarb/conf/haarb_local.xml")
    if [ "${role_num}" == "1" ]; then
        echo "active"
    else
        echo "standby"
    fi
}

function ha_clearrmfault() {
    local name=$1
    su nobody -s /bin/bash -c "timeout 5s $HA_CLIENT_TOOL --clearrmfault --name=$name"
}

function db_status() {
    su $CONTAINER_USER -s /bin/bash -c "source /home/GaussDB/.bashrc;$PGCTL status"
}

function db_query() {
    su $CONTAINER_USER -s /bin/bash -c "source /home/GaussDB/.bashrc;$PGCTL query"
}

function db_querybuild() {
    su $CONTAINER_USER -s /bin/bash -c "source /home/GaussDB/.bashrc;$PGCTL querybuild"
}

function db_build() {
    su $CONTAINER_USER -s /bin/bash -c "source /home/GaussDB/.bashrc;$PGCTL build"
}

# 由于gaussdb V5的bug，数据库无法fast停止，因此使用立即停止
function db_stop() {
    local type=$1
    su $CONTAINER_USER -s /bin/bash -c "source /home/GaussDB/.bashrc;$PGCTL stop -m immediate"
}

function db_start() {
    local state=$1
    if [ -z $state ]; then
        su $CONTAINER_USER -s /bin/bash -c "source /home/GaussDB/.bashrc;$PGCTL start -w"
    else
        su $CONTAINER_USER -s /bin/bash -c "source /home/GaussDB/.bashrc;$PGCTL start -w -M $state"
    fi
}

function db_notify() {
    local role=$1
    su $CONTAINER_USER -s /bin/bash -c "source /home/GaussDB/.bashrc;$PGCTL notify -M $role"
}

function db_switchover() {
    local type=$1
    su $CONTAINER_USER -s /bin/bash -c "source /home/GaussDB/.bashrc;$PGCTL switchover -m $type"
}

function db_config() {
    local line=$(cat $PGCONF | grep -n "\#replconninfo1" | awk -F ":" '{print $1}')
    if [ -z "$line" ]; then
      line=$(cat $PGCONF | grep -n "^replconninfo1" | awk -F ":" '{print $1}')
    fi
    local type=$1
    if [ "$type" == "link" ]; then
        if [ $# -ne 5 ];then
            echo "param is error"
            return 1
        fi
        ip_type_check $2
        if [ $? -ne 0 ];then
            log_error "[${FUNCNAME[0]},$LINENO] $2 is not ip."
            return 1
        fi
        ip_flag_check $2
        if [ $? -ne 0 ];then
            log_error "[${FUNCNAME[0]},$LINENO] $2 is not valid ip."
            return 1
        fi
        ip_type_check $4
        if [ $? -ne 0 ];then
            log_error "[${FUNCNAME[0]},$LINENO] $4 is not ip."
            return 1
        fi
        ip_flag_check $4
        if [ $? -ne 0 ];then
            log_error "[${FUNCNAME[0]},$LINENO] $4 is not valid ip."
            return 1
        fi
        if [ -n "$line" ]; then
          sed -i ${line}s/.*/replconninfo1="'localhost=$2 localport=$3 remotehost=$4 remoteport=$5'"/g $PGCONF
        fi
        echo "host    replication     GaussDB        $4/32            trust" >> $PG_HBA_CONF
        echo "host    replication     GaussDB        $2/32            trust" >> $PG_HBA_CONF
    elif [ "$type" == "clear" ];then
        if [ -n "$line" ]; then
          sed -i ${line}s/.*/replconninfo1="''"/g $PGCONF
        fi
        sed -i "/host    replication     GaussDB/d" $PG_HBA_CONF
    elif [ "$type" == "cert" ];then
        if [ -f "${PGDATA}/ca.crt.pem" ];then
            rm ${PGDATA}/ca.crt.pem
        fi
        if [ -L "${OPENAPI_CA_PATH}/ca.crt.pem" ];then
            log_error "[${FUNCNAME[0]},$LINENO] openapi's symbol link is not allowed."
            return 1
        fi
        if [ -L "${PGDATA}/ca.crt.pem" ];then
            log_error "[${FUNCNAME[0]},$LINENO] pg_data_ca's symbol link is not allowed."
            return 1
        fi
        cp ${OPENAPI_CA_PATH}/ca.crt.pem ${PGDATA}
        chown GaussDB:dbgrp ${PGDATA}/ca.crt.pem
        chmod 600 ${PGDATA}/ca.crt.pem
        local line=$(cat $PGCONF | grep -n ssl_ca_file | awk -F ":" '{print $1}')
        sed -i ${line}s/.*/ssl_ca_file="'ca.crt.pem'"/g $PGCONF
    fi
    su $CONTAINER_USER -s /bin/bash -c "source /home/GaussDB/.bashrc;$PGCTL reload"
}

case "$1" in
    "set_floatip")
        shift; set_floatip "$@"
        exit $?;
    ;;
    "unset_floatip")
        shift; unset_floatip "$@"
        exit $?;
    ;;
    "change_cluster_role")
        shift; change_cluster_role "$@"
    ;;
    "db_status")
        shift; db_status "$@"
    ;;
    "db_query")
        shift; db_query "$@"
    ;;
    "db_querybuild")
        shift; db_querybuild "$@"
    ;;
    "db_build")
        shift; db_build "$@"
    ;;
    "db_stop")
        shift; db_stop "$@"
    ;;
    "db_start")
        shift; db_start "$@"
    ;;
    "db_notify")
        shift; db_notify "$@"
    ;;
    "db_switchover")
        shift; db_switchover "$@"
    ;;
    "db_config")
        shift; db_config "$@"
    ;;
    "ha_config")
        shift; ha_config "$@"
    ;;
    "ha_status")
        shift; ha_status "$@"
    ;;
    "ha_start")
        shift; ha_start "$@"
    ;;
    "ha_stop")
        shift; ha_stop "$@"
    ;;
    "ha_query_fsw")
        shift; ha_query_fsw "$@"
    ;;
    "ha_forbid_switchover")
        shift; ha_forbid_switchover "$@"
    ;;
    "ha_allow_switchover")
        shift; ha_allow_switchover "$@"
    ;;
    "ha_role")
        shift; ha_role "$@"
    ;;
    "ha_clearrmfault")
        shift; ha_clearrmfault "$@"
    ;;
esac