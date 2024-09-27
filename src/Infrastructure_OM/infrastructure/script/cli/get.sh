#!/bin/bash

NETWORK_CARD_LENGTH_MAX=13
CURL_IP_PORT_LENGTH_MAX=128
CURL_CONNECT_TIMEOUT=20
PORT_MAX=65535
IOSTAT_COUNT_MAX=100
SAR_COUNT_MAX=60
PACKAGE_SIZE_MAX=65507

KAFKA_GROUP_LENGTH_MAX=249
KAFKA_CONF_PATH="/opt/third_data/kafka/config/server.properties"
KAFKA_COMMAND_CONFIG="/tmp/consumer.properties"
KAFKA_TOPIC_SCRIPT="/usr/local/kafka/kafka-${KAFKA_VERSION}/bin/kafka-topics.sh"
KAFKA_CONSUMER_SCRIPT="/usr/local/kafka/kafka-${KAFKA_VERSION}/bin/kafka-consumer-groups.sh"
KAFKA_SERVER="infrastructure-zk-kafka:9092"
INTER_CERT_PATH="/opt/OceanProtect/infrastructure/cert/internal/internal.crt.pem"
INTER_KEY_PATH="/opt/OceanProtect/infrastructure/cert/internal/internal.pem"
INTER_CA_CERT_PATH="/opt/OceanProtect/infrastructure/cert/internal/ca/ca.crt.pem"
INFRA_QUERY_PWD_URL="https://infrastructure.dpa.svc.cluster.local:8088/v1/infra/secret/info?nameSpace=dpa&secretName=common-secret"
INFRA_CHANGE_EP_URL="https://infrastructure.dpa.svc.cluster.local:8088/v1/infra/cluster/operation"
INFRA_DELETE_POD_URL="https://infrastructure.dpa.svc.cluster.local:8088/v1/infra/pod/delete"

REDIS_KEY_LENGTH_MAX=1024
REDIS_CLIENT_SCRIPT="/usr/local/redis/redis-${REDIS_VERSION}/redis-cli"
REDIS_CONF_PATH="/opt/third_data/redis/conf/redis.conf"
REDIS_CERT_PATH="/opt/OceanProtect/infrastructure/cert/redis/redis.crt.pem"
REDIS_KEY_PATH="/opt/OceanProtect/infrastructure/cert/redis/redis.pem"
REDIS_CA_CERT_PATH="/opt/OceanProtect/infrastructure/cert/redis/ca/ca.crt.pem"

AGENT_USER=rdadmin
DEFAULT_GROUP_INTERNAL="nobody"
HA_PRIMARY_ROLE="PRIMARY"
HA_STANDBY_ROLE="STANDBY"
HA_MARK_FILE="/usr/local/gaussdb/data/ha_started"
rootCAFile="/var/run/secrets/kubernetes.io/serviceaccount/ca.crt"
PERMISSION_SCRIPT_PATH="/opt/script/change_permission.sh"
PERSISTENCE_TMP_ROOT_PATH="/mnt/protectagent"
AGENT_CONF_PATH="/opt/DataBackup/ProtectClient/ProtectClient-E/conf"

# 不带参命令白名单
COMMAND_WITHOUT_PARAMS_WHITE_LIST=(
top
netstat_ntulp
netstat_anp
df_hiT
free_h
ip_a
route_n
iptables_nvL
ps_efww
ps_efTww
ps_auxww
ulimit_a
lsof_i
ifconfig_a
arp
date
nfsiostat
rpm_qa
uname_a
mount
users
)

FILE_WHITE_LIST=(
/opt/third_data/kafka/config/server.properties
/opt/third_data/redis/conf/redis.conf
/opt/third_data/zookeeper/conf/zoo.cfg
/opt/db_data/GaussDB_V5/postgresql.conf
/opt/om/package/src/app/config/config.ini
/opt/OceanStor/100P/ProtectEngine-E/dme_unifiedbackupcontroller/app/.env
/opt/DataBackup/ProtectClient/Plugins/NasPlugin/conf/hcpconf.ini
/opt/OceanStor/100P/ProtectEngine-E/dme_archive/conf/hcpconf.ini
/opt/OceanStor/100P/ProtectEngine-E/dme_archive/conf/odbc.ini
/opt/OceanStor/100P/ProtectEngine-E/dme_archive/conf/odbcinst.ini
/opt/OceanStor/100P/ProtectEngine-E/dme_archive/conf/permission.json
/opt/OceanStor/100P/ProtectEngine-E/dme_archive/conf/pluginmgr.xml
/opt/OceanStor/100P/ProtectEngine-E/dme_archive/conf/privateconf.ini
/opt/OceanStor/100P/ProtectEngine-E/dme_archive/conf/srv_checked_processes.xml
/opt/OceanStor/100P/ProtectEngine-E/dme_archive/conf/trace_config.yaml
)

SENSITIVE_WORDS="*pass*|*pwd*|*key*|*crypto*|*session*|*token*|*fingerprint*|*auth*|*enc*|*dec*|*tgt*|*iqn*|*initiator*|*secret*|*cert*|*salt*|*private*|*user_info*|*verfiycode*|*rand*|*safe*|*PKCS1*|*base64*|*AES128*|*AES256*|*RSA*|*SHA1*|*SHA256*|*SHA384*|*SHA512*|*algorithm*|sk|mk|iv"

# 检查字符串长度
function check_str_length() {
    str_length="$1"
    max_length="$2"
    if [ "${str_length}" -gt "${max_length}" ];then
        echo "ERROR: Invalid parameter length. The maximum length is ${max_length}."
        return 1
    fi
    return 0
}

# 检查package_size大小
function check_package_size() {
    local package_size="$1"
    expr "$package_size" "+" 10 &> /dev/null
    if [ $? -ne 0 ]; then
        echo "ERROR: The parameter $package_size is invalid."
        return 1
    fi
    if [ "$package_size" -lt 0 ] || [ "$package_size" -gt ${PACKAGE_SIZE_MAX} ]; then
        echo "ERROR: The parameter ${package_size} is out of range. The parameter range is 0 - ${PACKAGE_SIZE_MAX}."
        return 1
    fi
    return 0
}

# 检查ipv4地址合法性
function check_ipv4()
{
    local ipStr="$1"
    local isValid=$(echo ${ipStr}|grep -E "^([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\.([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\.([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\.([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])$"|wc -l);
    if [ ${isValid} -gt 0 ];then
        return 0
    fi

    echo "ERROR: The ip address $ipStr is invalid."
    return 1
}

# 检查ipv6地址合法性
function check_ipV6()
{
    if echo $1 | grep -E '^((([0-9a-fA-F]{1,4}:){7}([0-9A-Fa-f]{1,4}|:))|(([0-9a-fA-F]{1,4}:){6}((([0-9]{1,3}\.){3}[0-9]{1,4})|([0-9a-fA-F]{1,4})|:))|(([0-9a-fA-F]{1,4}:){5}:((([0-9a-fA-F]{1,4})?)|([0-9a-fA-F]{1,4}:[0-9a-fA-F]{1,4})|(([0-9]{1,3}\.){3}[0-9]{1,4})))|(([0-9a-fA-F]{1,4}:){4}:((([0-9a-fA-F]{1,4})?)|(([0-9a-fA-F]{1,4}:){1,2}[0-9a-fA-F]{1,4})|(([0-9a-fA-F]{1,4}:)?([0-9]{1,3}\.){3}[0-9]{1,4})))|(([0-9a-fA-F]{1,4}:){3}:((([0-9a-fA-F]{1,4}:){0,2}([0-9]{1,3}\.){3}[0-9]{1,4})|(([0-9a-fA-F]{1,4}:){0,3}[0-9a-fA-F]{1,4})|$))|(([0-9a-fA-F]{1,4}:){2}:((([0-9a-fA-F]{1,4}:){0,3}([0-9]{1,3}\.){3}[0-9]{1,4})|(([0-9a-fA-F]{1,4}:){0,4}[0-9a-fA-F]{1,4})|$))|(([0-9a-fA-F]{1,4}:){1}:((([0-9a-fA-F]{1,4}:){0,4}([0-9]{1,3}\.){3}[0-9]{1,4})|(([0-9a-fA-F]{1,4}:){0,5}[0-9a-fA-F]{1,4})|$))|(::((([0-9a-fA-F]{1,4}:){0,5}([0-9]{1,3}\.){3}[0-9]{1,4})|(([0-9a-fA-F]{1,4}:){0,6}[0-9a-fA-F]{1,4})|$)))$' >/dev/null 2>&1;then
        return 0
    fi
    echo "ERROR: The ip address $1 is invalid."
    return 1
}

# 检查域名合法性
function check_hcs_domain() {
    if echo $1 | grep -E '(^sc|^evs|^ecs|^iam-apigateway-proxy|^volume|^compute)(\.[a-zA-Z0-9][-a-zA-Z0-9]{0,50})+$' >/dev/null 2>&1;then
        return 0
    fi
    echo "ERROR: The domain $1 is invalid."
    return 1
}

# 生成kafka秘钥文件
function create_command_config() {
    # 获取truststore密码
    truststore_pwd=$(grep "ssl.truststore.password=" ${KAFKA_CONF_PATH} | awk -F '=' '{print $2}')
    if [ -z "${truststore_pwd}" ];then
        echo "ERROR: Get kafka truststore password failed."
        return 1
    fi

    # 获取kafka密码
    kafka_pwd=$(curl -v --cert ${INTER_CERT_PATH} --key ${INTER_KEY_PATH} --pass "${truststore_pwd}" --cacert ${INTER_CA_CERT_PATH} "${INFRA_QUERY_PWD_URL}" | grep "kafka.password" | awk '{print $2}')
    if [ -z "${kafka_pwd}" ];then
        echo "ERROR: Get kafka password failed."
        return 1
    fi

    cat <<  EOT > ${KAFKA_COMMAND_CONFIG}
sasl.jaas.config=org.apache.kafka.common.security.plain.PlainLoginModule required \\
username=kafka_usr \\
password=${kafka_pwd};
sasl.mechanism=PLAIN
security.protocol=SASL_SSL
ssl.truststore.location=/opt/OceanProtect/infrastructure/cert/internal/internal.ks
ssl.truststore.password=${truststore_pwd}
ssl.endpoint.identification.algorithm=
EOT
   if [ $? != 0 ]; then
       echo "ERROR: Create kafka command config:${KAFKA_COMMAND_CONFIG} failed."
       return 1
   fi

   return 0
}

# 处理不带参命令
function exec_command() {
    local command="$1"
    # bash字符串替换方法：${variable//search/replace}
    # 处理ip_a命令
    if [ "${command}" == "ip_a" ];then
        sh -c "${command//_/ }"
        return $?
    fi
    # 处理top命令
    if [ "${command}" == "top" ];then
        top -n 1
        return $?
    fi
    # 处理users命令，取 /etc/passwd 文件第一列用户名
    if [ "${command}" == "users" ]; then
        awk -F ":" '{print $1}' /etc/passwd
        return $?
    fi
    # 其他命令
    sh -c "${command//_/ -}"
    return $?
}

# 处理 ls_lt|stat|du 带文件绝对路径或目录绝对路径参数
function exec_path_command() {
    local command="$1"
    if [ $# -gt 2 ]; then
        echo "ERROR: The parameter is invalid. Usage: ls_lt|stat|du params"
        return 1
    fi

    # 处理ls_lt命令
    if [ "${command}" == "ls_lt" ];then
        ls -lt "$2"
        return $?
    fi

    # 执行命令
    "$1" "$2"
    return $?
}

function cat_command() {
    if [ $# -gt 2 ]; then
        echo "ERROR: The parameter is invalid. Usage: cat input_file"
        return 1
    fi

    local input_file="$1"
    # 判断参数是否在白名单中
    for file_path in "${FILE_WHITE_LIST[@]}"
    do
        if [ "${input_file}" == "${file_path}" ]; then
            # 执行命令
            grep -v -E "${SENSITIVE_WORDS}" "${input_file}"
            return $?
        fi
    done

    echo "ERROR: Invalid input_file: ${input_file}."
    return 1
}

function ping_command() {
    if [ $# -eq 1 ]; then
        local ip_addr="$1"
        check_ipv4 "${ip_addr}"
        if [ $? -ne 0 ]; then
          return $?
        fi
        command="-c 4 ${ip_addr}"
        ping ${command}
        return $?
    fi

    if [ $# -gt 3 ]; then
        echo "ERROR: The parameter is invalid. Usage: ping [-s package_size] ip."
        return 1
    fi

    if [ $1 == "-s" ]; then
        package_size="$2"
        check_package_size "$package_size"
        if [ $? -ne 0 ]; then
          return $?
        fi
        ip_addr="$3"
        check_ipv4 "${ip_addr}"
        if [ $? -ne 0 ]; then
          return $?
        fi

        # 执行命令
        command="$ip_addr -c 4"
        if [ ! -z "$package_size" ]; then
          command="$command -s $package_size"
        fi
        ping ${command}
        return $?
    else
       echo "ERROR: The parameter is invalid. Usage: ping [-s package_size] ip."
       return 1
    fi


}

function sed_command() {
    # sed old_value new_value input_file <row>
    if [ $# -lt 3 ] || [ $# -gt 4 ]; then
        echo "ERROR: The parameter is invalid. Usage: sed old_value new_value input_file <row>"
        return 1
    fi

    # 检查参数合法性
    local old_value="$1"
    local new_value="$2"
    local input_file="$3"
    # 判断参数是否在白名单中
    for file_path in "${FILE_WHITE_LIST[@]}"
    do
        if [ "${input_file}" == "${file_path}" ]; then
            # 执行命令
            if [ $# -eq 4 ]; then
                local row="$4"
                if [ "${row}" -lt 0 ];then
                    echo "ERROR: The parameter of row ${row} is out of range. The row range is greater than or equal to 0."
                    return 1
                fi
                sed -i "${row}s/${old_value}/${new_value}/" "${input_file}"
                return $?
            fi
            sed -i "s/${old_value}/${new_value}/g" "${input_file}"
            return $?
        fi
    done

    echo "ERROR: Invalid input_file: ${input_file}."
    return 1
}

function pstree_command() {
    # 仅支持指定pid查询
    local pid="$1"
    # 判断pid是否合法
    if [ "$pid" -lt 1 ];then
        echo "ERROR: The parameter of pid ${pid} is out of range. The pid range is greater than or equal to 1."
        return 1
    fi

    # 执行命令
    pstree "$pid"
    return $?

}

function ethtool_command() {
    local network_card="$1"
    card_length=$(expr length "${network_card}")

    # 校验网卡长度是否合法
    check_str_length "${card_length}" ${NETWORK_CARD_LENGTH_MAX}
    if [ $? -ne 0 ]; then
        return 1
    fi

    # 执行命令
    ethtool "${network_card}"
    return $?
}
function curl_kv_command() {
    local ip_port="$1"

    # 校验参数长度是否合法
    ip_port_length=$(expr length "${ip_port}")
    check_str_length "${ip_port_length}" ${CURL_IP_PORT_LENGTH_MAX}
    if [ $? -ne 0 ]; then
        return 1
    fi

    # 校验是否为ip:port格式，暂不支持ipv6
    split_length=$(echo "${ip_port}" | awk -F ":" '{print NF}')
    if [ ${split_length} -ne 2 ];then
        echo "ERROR: The parameter of ip:port ${ip_port} is invalid. Usage: curl_kv ip:port"
        return 1
    fi

    # 获取ip地址和port
    ip_addr=${ip_port%:*}
    port=${ip_port##*:}

    # 校验ip是否合法
    echo "${ip_addr}" | grep ':' 1>/dev/null
    if [ $? -ne 0 ];then
        check_ipv4 "${ip_addr}"
        if [ $? -ne 0 ];then
            return 1
        fi
    else
        check_ipV6 "${ip_addr}"
        if [ $? -ne 0 ];then
            return 1
        fi
    fi

    # 校验port是否合法
    if [ "$port" -lt 1 ] 2>/dev/null || [ "$port" -gt "${PORT_MAX}" ] 2>/dev/null ;then
        echo "ERROR: The parameter of port ${port} is out of range. The port range is 1 - ${PORT_MAX}."
        return 1
    fi

    # 执行命令
    curl -kv --connect-timeout ${CURL_CONNECT_TIMEOUT} "${ip_port}"
    return $?
}

function redis_command() {
    # Usage: redis get key
    if [ $# -ne 2 ]; then
        echo "ERROR: The parameter is invalid. Usage: redis get key"
        return 1
    fi

    # 校验是否为redis容器
    redis_pid=$(ps axww | grep "redis-server" | grep -v grep | awk '{print $1}')
    if [ -z "${redis_pid}" ]; then
        echo "ERROR: This is not redis container, or redis service is not running."
        return 1
    fi

    # 校验参数是否合法
    local param="$1"
    if [ "$param" != "get" ];then
        echo -e "ERROR: The parameter $param is invalid. Usage: redis get key"
        return 1
    fi
    local key="$2"
    key_length=$(expr length "${key}")
    check_str_length "${key_length}" ${REDIS_KEY_LENGTH_MAX}
    if [ $? -ne 0 ]; then
        echo "ERROR: The parameter of key ${key} is out of range. The length is not greater than ${REDIS_KEY_LENGTH_MAX}."
        return 1
    fi

    # 查询redis密码
    redis_pwd=$(grep -E ^requirepass ${REDIS_CONF_PATH} | awk '{print $2}')
    if [ -z "${redis_pwd}" ];then
        echo "ERROR: Get redis password failed."
        return 1
    fi

    # 执行命令，redis_pwd值两边引号需要去除，否则不识别
    ${REDIS_CLIENT_SCRIPT} -h infrastructure -p 6369 --tls --cert ${REDIS_CERT_PATH} --key ${REDIS_KEY_PATH} --cacert ${REDIS_CA_CERT_PATH} <<EOF
      AUTH "${redis_pwd:1:-1}"
      $@
EOF
    return $?
}

function change_endpoint_command() {
    if [ $# -ne 1 ]; then
        echo "ERROR: The parameter is invalid. Usage: change_ep [ip]"
        exit 1
    fi
    if ! [[ $1 =~ ^([0-9]{1,3}\.){3}[0-9]{1,3}$ ]]; then
        echo "Error: Invalid IPv4 address"
        exit 1
    fi
    local endpoint_ip="$1"

    truststore_pwd=$(grep "ssl.truststore.password=" ${KAFKA_CONF_PATH} | awk -F '=' '{print $2}')
    if [ -z "${truststore_pwd}" ];then
        echo "ERROR: Get kafka truststore password failed."
        return 1
    fi

    change_ep_cmd=$(curl -X POST -H "Content-Type: application/json" -d '{"action": "update", "needRestart": false, "ipList": ["'"$endpoint_ip"'"]}' --cert ${INTER_CERT_PATH} --key ${INTER_KEY_PATH} --pass "${truststore_pwd}" --cacert ${INTER_CA_CERT_PATH} "${INFRA_CHANGE_EP_URL}" > /dev/null 2>&1)
    if [ $? -ne 0 ];then
        echo "ERROR: Change gaussdb cluster endpoint failed."
        return 1
    fi
    echo "Change gaussdb endpoint successfully."
}

function modify_badblock_switch_command() {
    hcpconf_path="/opt/DataBackup/ProtectClient/Plugins/VirtualizationPlugin/conf/hcpconf.ini"
    # 校验参数
    local param="$1"
    echo "Start modify badblock switch"
    if [ "$param" != "yes" ] && [ "$param" != "no" ];then
        echo "ERROR: The parameter $param is invalid. Usage: modify_badblock_switch_command: modify_badblock_switch [yes | no]"
        return 1
    fi
    if [ $# -ne 1 ]; then
        echo "ERROR: The parameter count is:${$#} invalid. modify_badblock_switch_command: modify_badblock_switch [yes | no]"
        return 1
    fi
    # 校验路径
    if [ ! -f "${hcpconf_path}" ];then
        echo "ERROR: The parameter hcpconf_path: ${hcpconf_path} is not exist."
        return 1
    fi
    # 校验路径是否是软连接
    if [ -L "${hcpconf_path}" ]; then
        echo "ERROR modify_badblock_switch_command hcpconf_path:${hcpconf_path}, symbolic link is not allowed."
        exit 1
    fi
    change_str="RecoverIgnoreBadBlock=${param}"
    goal_line=$(sed -n '/RecoverIgnoreBadBlock=/=' ${hcpconf_path})
    if [ "${goal_line}" = "" ];then
        sed -i '/RecoverIgnoreBadBlock=/d' ${hcpconf_path}
        goal_line1=17
        sed -i "${goal_line1}i ${change_str}" ${hcpconf_path}
        if [ $? -eq "0" ];then
            echo "Insert RecoverIgnoreBadBlock successfully."
        fi
    else
        sed -i  "${goal_line}c ${change_str}" ${hcpconf_path}
        if [ $? -eq "0" ];then
            echo "Change RecoverIgnoreBadBlock successfully."
        fi
    fi
}

function kafka_command() {
    # Usage: kafka [list_topic | list_group | {list_consumer_group group}]
    # 校验参数是否合法
    local param="$1"
    if [ "$param" != "list_topic" ] && [ "$param" != "list_group" ] && [ "$param" != "list_consumer_group" ];then
        echo "ERROR: The parameter $param is invalid. Usage: kafka [list_topic | list_group | {list_consumer_group group}]"
        return 1
    fi

    if [ "$param" == "list_topic" ] || [ "$param" == "list_group" ]; then
        if [ $# -gt 1 ]; then
            echo "ERROR: The parameter is invalid. Usage: kafka [list_topic | list_group | {list_consumer_group group}]"
            return 1
        fi
    fi
    if [ $# -gt 2 ]; then
        echo "ERROR: The parameter is invalid. Usage: kafka [list_topic | list_group | {list_consumer_group group}]"
        return 1
    fi

    # 校验是否为kafka容器
    kafka_pid=$(ps axww | grep " kafka\.Kafka ${KAFKA_CONF_PATH}" | grep java | grep -v grep | awk '{print $1}')
    if [ -z "${kafka_pid}" ]; then
        echo "ERROR: This is not kafka container, or kafka service is not running."
        return 1
    fi

    # 校验参数为list_consumer_group时必须有group参数
    if [ "$param" == "list_consumer_group" ]; then
        if [ $# -ne 2 ];then
            echo "ERROR: The parameter $param is invalid. Usage: kafka list_consumer_group group"
            return 1
        fi

        group="$2"
        group_length=$(expr length "$group")
        # 校验group参数长度是否合法
        check_str_length "${group_length}" ${KAFKA_GROUP_LENGTH_MAX}
        if [ $? -ne 0 ]; then
            echo "ERROR: The parameter of group ${group} is out of range. The length is not greater than ${KAFKA_GROUP_LENGTH_MAX}."
            return 1
        fi
    fi

    # 生成kafka秘钥文件
    create_command_config
    if [ $? != 0 ]; then
       return 1
    fi

    # 执行命令
    case "${param}" in
        list_topic)
            ${KAFKA_TOPIC_SCRIPT} --bootstrap-server ${KAFKA_SERVER} --command-config ${KAFKA_COMMAND_CONFIG} --list
            ;;
        list_group)
            ${KAFKA_CONSUMER_SCRIPT} --bootstrap-server ${KAFKA_SERVER} --command-config ${KAFKA_COMMAND_CONFIG} --list
            ;;
        list_consumer_group)
            ${KAFKA_CONSUMER_SCRIPT} --bootstrap-server ${KAFKA_SERVER} --command-config ${KAFKA_COMMAND_CONFIG} --group "$2" --describe
            ;;
        *)
            echo "ERROR: Invalid parameter: $*."
            ;;
    esac

    # 删除kafka秘钥文件
    rm -rf ${KAFKA_COMMAND_CONFIG}
    return 0
}

function iostat_xm_command() {
    # Usage: iostat_xm interval count <device>
    if [ $# -lt 2 ] || [ $# -gt 3 ]; then
        echo "ERROR: The parameter is invalid. Usage: iostat_xm interval count <device>"
        return 1
    fi

    # 校验参数是否合法
    local interval="$1"
    if [ "$interval" -lt 1 ];then
        echo "ERROR: The parameter of internal ${interval} is out of range. The interval range is greater than or equal to 1."
        return 1
    fi
    local count="$2"
    if [ "$count" -lt 1 ] || [ "$count" -gt ${IOSTAT_COUNT_MAX} ];then
        echo "ERROR: The parameter of count ${count} is out of range. The count range is 1 - ${IOSTAT_COUNT_MAX}."
        return 1
    fi

    if [ $# -ne 3 ];then
        local device="$3"
        device_length=$(expr length "$device")
        # 校验网卡长度是否合法
        check_str_length "${device_length}" ${NETWORK_CARD_LENGTH_MAX}
        if [ $? -ne 0 ]; then
            echo "ERROR: The parameter of device ${device} is out of range. The length is not greater than ${NETWORK_CARD_LENGTH_MAX}."
            return 1
        fi
    fi

    # 执行命令
    iostat -xm "$@"
    return $?
}

function sar_n_command() {
    # Usage: sar_n DEV interval count
    if [ $# -ne 3 ]; then
        echo "ERROR: The parameter is invalid. Usage: sar_n DEV interval count"
        return 1
    fi

    # 校验参数是否合法
    local device_type="$1"
    if [ "${device_type}" != "DEV" ]; then
        echo "ERROR: The parameter of device_type ${device_type} is invalid. Usage: sar_n DEV interval count"
        return 1
    fi
    local interval="$2"
    if [ "$interval" -lt 1 ];then
        echo "ERROR: The parameter of internal ${interval} is out of range. The interval range is greater than or equal to 1."
        return 1
    fi
    local count="$3"
    if [ "$count" -lt 1 ] || [ "$count" -gt ${SAR_COUNT_MAX} ];then
        echo "ERROR: The parameter of count ${count} is out of range. The count range is 1 - ${SAR_COUNT_MAX}."
        return 1
    fi

    # 执行命令
    sar -n "$@"
    return $?
}

function exec_sar_P_command(){
    local interval="$1"
    if [ "$interval" -lt 1 ];then
        echo "ERROR: The parameter of internal ${interval} is out of range. The interval range is greater than or equal to 1."
        return 1
    fi
    local count="$2"
    if [ "$count" -lt 1 ] || [ "$count" -gt ${SAR_COUNT_MAX} ];then
        echo "ERROR: The parameter of count ${count} is out of range. The count range is 1 - ${SAR_COUNT_MAX}."
        return 1
    fi

    # 执行命令
    sar -P "$@"
    return $?
}

function sar_P_command() {
    # Usage: sar_P [ALL | device_id] interval count
    if [ $# -ne 3 ]; then
        echo "ERROR: The parameter is invalid. Usage: sar_P [ALL | device_id] interval count"
        return 1
    fi

    # 校验参数是否合法
    local device_type="$1"
    if [ "${device_type}" == "ALL" ]; then
        exec_sar_P_command "$2" "$3"
        return $?
    elif [ "${device_type}" -ge 0 ];then
        exec_sar_P_command "$2" "$3"
        return $?
    else
        echo "ERROR: The parameter of device_type ${device_type} is invalid. Usage: sar_P [ALL | device_id] interval count"
        return 1
    fi
}

ha_recover_command() {
    tokenFile=$(cat /var/run/secrets/kubernetes.io/serviceaccount/token)
    # Usage: ha_recover [PRIMARY | STANDBY]
    if [ $# -ne 1 ]; then
        echo "ERROR: The parameter is invalid. Usage: ha_recover [PRIMARY | STANDBY]"
        exit 1
    fi
    local role=$(echo "$1" | tr [a-z] [A-Z])
    local master=/opt/OceanProtect/protectmanager/kmc/master.ks
    local backup=/kmc_conf/..data/backup.ks
    if [ "${role}" == "${HA_PRIMARY_ROLE}" ] || [ "${role}" == "${HA_STANDBY_ROLE}" ]; then
        echo "start to add cluster conf"
        PAYLOAD="{\"data\":{\"CLUSTER_ROLE\":\"${role}\"}}"
        curl --cacert ${rootCAFile} \
          -X PATCH \
          -H "Content-Type: application/strategic-merge-patch+json" \
          -H "Authorization: Bearer ${tokenFile}" \
          --data "${PAYLOAD}" \
          https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/dpa/configmaps/cluster-conf > /dev/null 2>&1
        if [ $? -ne 0 ]; then
            echo "ERROR: Modify cluster role failed."
            return $?
        fi
        touch "${HA_MARK_FILE}"
        if [ $? -ne 0 ]; then
            echo "ERROR: Generate HA mark file failed."
            return $?
        fi
        echo "HA config recover successfully, please restart gaussdb pod."
        return 0
    else
        echo "ERROR: The parameter of role ${role} is invalid. Usage: ha_recover [PRIMARY | STANDBY]"
        return 1
    fi
}

function record_hcs_mapping_to_temp() {
    OLD_IFS="$IFS"
    IFS=","
    array=($1)
    IFS="$OLD_IFS"
    if [ ${#array[@]} -ne 2 ]; then
        echo "ERROR: Invalid param number ${#array[@]}!"
        return 1
    fi

    hcs_ip="${array[0]}"
    hcs_domain="${array[1]}"

    check_ipv4 "${hcs_ip}"
    if [ $? -ne 0 ];then
        return 1
    fi

    check_hcs_domain "${hcs_domain}"
    if [ $? -ne 0 ];then
        return 1
    fi
    ip_domain="${hcs_ip} ${hcs_domain}"
    echo "${ip_domain}" >> "${AGENT_CONF_PATH}/hcs_temp"
    return $?
}

function record_hcs_mapping_to_hosts() {
    start=`cat ${PERSISTENCE_TMP_ROOT_PATH}/hosts | grep -n "${1}" | awk -F ":" '{print $1}'`
    end=`cat ${PERSISTENCE_TMP_ROOT_PATH}/hosts | grep -n "${2}" | awk -F ":" '{print $1}'`
    if [ "${start}" != "" ] && [ "${end}" != "" ]; then
        # 清除原来的域名映射
        sed_res=`sed  "${start},${end}"d ${PERSISTENCE_TMP_ROOT_PATH}/hosts`
        echo "${sed_res}" > ${PERSISTENCE_TMP_ROOT_PATH}/hosts
        if [ $? -ne 0 ];then
            return 1
        fi
    fi
    # 写入hcs域名映射
    hcs_mapping=`cat ${AGENT_CONF_PATH}/hcs_temp`
    echo "${hcs_mapping}" >> "${PERSISTENCE_TMP_ROOT_PATH}/hosts"
    return $?
}

function hcs_mapping_command() {
    separator_start="########## HCS Domain Mapping ##########"
    separator_end="########## HCS Domain Mapping End ##########"

    echo "${separator_start}" >> "${AGENT_CONF_PATH}/hcs_temp"
    for map in $*;
    do
        record_hcs_mapping_to_temp ${map}
        if [ $? -ne 0 ]; then
            rm -f "${AGENT_CONF_PATH}/hcs_temp"
            echo "ERROR: Record hcs mapping to temp failed!"
            return 1
        fi
    done

    echo "${separator_end}" >> "${AGENT_CONF_PATH}/hcs_temp"

    sudo "${PERMISSION_SCRIPT_PATH}" chown "${AGENT_USER}:${DEFAULT_GROUP_INTERNAL}" "${PERSISTENCE_TMP_ROOT_PATH}/hosts"
    record_hcs_mapping_to_hosts "${separator_start}" "${separator_end}"
    if [ $? -ne 0 ]; then
        sudo "${PERMISSION_SCRIPT_PATH}" chown root:root "${PERSISTENCE_TMP_ROOT_PATH}/hosts"
        rm -f "${AGENT_CONF_PATH}/hcs_temp"
        echo "ERROR: Record hcs mapping to host failed!"
        return 1
    fi
    sudo "${PERMISSION_SCRIPT_PATH}" chown root:root "${PERSISTENCE_TMP_ROOT_PATH}/hosts"
    mv "${AGENT_CONF_PATH}/hcs_temp" "${AGENT_CONF_PATH}/hcs_domain"
    return $?
}

# 处理不带参命令
function handle_one_param() {
    param="$1"
    # 判断是否为不带参白名单命令
    for command in "${COMMAND_WITHOUT_PARAMS_WHITE_LIST[@]}"
    do
        if [ "${param}" == "${command}" ]; then
            exec_command "${param}"
            return $?
        fi
    done

    echo "ERROR: Invalid parameter: $*."
    return 1
}

# 处理参数为2的命令
function handle_two_params() {
    first_param="$1"
    case "${first_param}" in
        ls_lt|stat|du)
            exec_path_command "${first_param}" "$2"
            return $?
            ;;
        cat)
            cat_command "$2"
            return $?
            ;;
        pstree)
            pstree_command "$2"
            return $?
            ;;
        ethtool)
            ethtool_command "$2"
            return $?
            ;;
        curl_kv)
            curl_kv_command "$2"
            return $?
            ;;
        ping)
            ping_command "$2"
            return $?
            ;;
        kafka)
            kafka_command "$2"
            return $?
            ;;
        change_ep)
            change_endpoint_command "$2"
            return $?
            ;;
        modify_badblock_switch)
            modify_badblock_switch_command "$2"
            return $?
            ;;
        ha_recover)
            ha_recover_command "$2"
            ;;
        *)
            echo "ERROR: Invalid parameter: $*."
            return 1
            ;;
    esac
}

# 处理带多个参命令
function handle_more_params() {
    first_param="$1"
    case "${first_param}" in
        redis)
            redis_command "${@:2}"
            return $?
            ;;
        ping)
            ping_command "${@:2}"
            return $?
            ;;
        kafka)
            kafka_command "${@:2}"
            return $?
            ;;
        iostat_xm)
            iostat_xm_command "${@:2}"
            return $?
            ;;
        sar_n)
            sar_n_command "${@:2}"
            return $?
            ;;
        sar_P)
            sar_P_command "${@:2}"
            return $?
            ;;
        hcs_mapping)
            hcs_mapping_command "${@:2}"
            return $?
            ;;
        *)
            echo "ERROR: Invalid parameter: $*."
            return 1
            ;;
    esac
}

function main() {
    # 没有参数直接报错
    if [ $# -eq 0 ]; then
        echo "ERROR: Invalid parameter: $*."
        return 1
    fi

    local COMMAND="$1"
    # 参数长度为1
    # eg: get.sh top
    if [ $# -eq 1 ]; then
        handle_one_param "${COMMAND}"
        return $?
    fi

    # 参数长度为2
    # eg: get.sh cat file_name
    if [ $# -eq 2 ]; then
        handle_two_params "${COMMAND}" "$2"
        return $?
    fi

    # 参数大于2
    # eg: get.sh kafka list_topic
    handle_more_params "$@"
    return $?
}

main "$@"
exit $?