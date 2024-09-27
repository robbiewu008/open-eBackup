#!/bin/bash

NODE_NAME=${NODE_NAME}
DATA_PATH="/opt/third_data/redis/data"
THIRD_REDIS_PATH="/opt/third_data/redis"
LOG_PATH="/opt/OceanProtect/logs/${NODE_NAME}/infrastructure/redis"
INSTALL_LOG_PATH=${LOG_PATH}/install_redis.log
REDIS_PATH="/usr/local/redis/redis-VERSION"
PRE_DATA_PATH="${REDIS_PATH}/data"
PRE_LOG_PATH="${REDIS_PATH}/logs"
PRE_LOG_FILE=$PRE_LOG_PATH/redis.log
CURPATH=$(cd "$(dirname "$0")"; pwd)

tokenFile=$(cat /var/run/secrets/kubernetes.io/serviceaccount/token)
rootCAFile="/var/run/secrets/kubernetes.io/serviceaccount/ca.crt"

CA_CERT="/opt/OceanProtect/infrastructure/cert/redis/ca/ca.crt.pem"
CERT="/opt/OceanProtect/infrastructure/cert/redis/redis.crt.pem"
KEY="/opt/OceanProtect/infrastructure/cert/redis/redis.pem"

INTER_CERT="/opt/OceanProtect/infrastructure/cert/internal/internal.crt.pem"
INTER="/opt/OceanProtect/infrastructure/cert/internal/internal.pem"
INTER_CA_CERT="/opt/OceanProtect/infrastructure/cert/internal/ca/ca.crt.pem"

INTER_CERT_PATH="/opt/OceanProtect/infrastructure/cert/redis"
CA_CERT_PATH="/opt/OceanProtect/infrastructure/cert/redis/ca"

redis_conf_in_nas=$THIRD_REDIS_PATH/conf/redis.conf
REDIS_AOF_PATH="${DATA_PATH}/appendonly.aof"

HIGH_RISK_CMD=(FLUSHALL KEYS SHUTDOWN SAVE DEBUG)

#配置证书文件
function config_cert()
{
    sed -i "/# port 0/i port 0" ${redis_conf_in_nas}
    sed -i "/# port 0/d" ${redis_conf_in_nas}
    sed -i "/# tls-port 6379/i tls-port 6369" ${redis_conf_in_nas}
    sed -i "/# tls-port 6379/d" ${redis_conf_in_nas}

    sed -i "/# tls-cert-file redis.crt/i tls-cert-file ${INTER_CERT_PATH}/redis.crt.pem" \
            ${redis_conf_in_nas}
    sed -i "/# tls-cert-file redis.crt/d" ${redis_conf_in_nas}
    sed -i "/# tls-key-file redis.key/i tls-key-file  ${INTER_CERT_PATH}/redis.pem" \
            ${redis_conf_in_nas}
    sed -i "/# tls-key-file redis.key/d" ${redis_conf_in_nas}
    sed -i "/# tls-ca-cert-file ca.crt/i tls-ca-cert-file ${CA_CERT_PATH}/ca.crt.pem" \
            ${redis_conf_in_nas}
    sed -i "/# tls-ca-cert-file ca.crt/d" ${redis_conf_in_nas}
    sed -i "/# tls-ciphers DEFAULT:!MEDIUM/i tls-ciphers DEFAULT:!AES128-SHA:!AES128-SHA256:!AES256-SHA:!AES256-SHA256:!ECDHE-RSA-AES128-SHA:!ECDHE-RSA-AES128-SHA256:!ECDHE-RSA-AES256-SHA:!ECDHE-RSA-AES256-SHA384:!AES128-GCM-SHA256:!AES256-GCM-SHA384" ${redis_conf_in_nas}

    return 0
}

# 挂载log目录
if [ ! -d $LOG_PATH ];then
    mkdir -p $LOG_PATH
fi
touch ${LOG_PATH}/install_redis.log

chmod 750 $LOG_PATH
chown nobody:nobody ${LOG_PATH}/install_redis.log
chmod 640 ${LOG_PATH}/install_redis.log
if [ ! -f "${LOG_PATH}/redis.log" ];then
    touch ${LOG_PATH}/redis.log
fi
chmod 640 "${LOG_PATH}/redis.log"

# 清理残留nfs文件
find ${LOG_PATH} -type f -name ".nfs*" -delete
check_result "$?" "${LINENO} find ${LOG_PATH} -type f -name \".nfs*\" -delete"

function log_info()
{
    echo "$(date +"[%Y-%m-%d %H:%M:%S,%N]")[INFO][INSTALL_REDIS: $1][$0][${BASH_LINENO}]"
    echo "$(date +"[%Y-%m-%d %H:%M:%S,%N]")[INFO][INSTALL_REDIS: $1][$0][${BASH_LINENO}]" >> "${INSTALL_LOG_PATH}"
}

function log_error()
{
    echo "$(date +"[%Y-%m-%d %H:%M:%S,%N]")[ERROR][INSTALL_REDIS: $1][$0][${BASH_LINENO}]"
    echo "$(date +"[%Y-%m-%d %H:%M:%S,%N]")[ERROR][INSTALL_REDIS: $1][$0][${BASH_LINENO}]" >> "${INSTALL_LOG_PATH}"
}

function check_result()
{
    if [ "$1" != "0" ];then
        log_error "Exec cmd:$2 failed."
    else
        log_info "Exec cmd:$2 success."
    fi
}
mount_result=$(sudo /opt/mount_oper.sh mount_bind ${LOG_PATH} ${PRE_LOG_PATH})
log_info "The result of mount --bind ${LOG_PATH} ${PRE_LOG_PATH} is ${mount_result}"

# 挂载data目录
log_info "Start to mount the data directory."
if [ ! -d $DATA_PATH ];then
    mkdir -p $DATA_PATH
    check_result "$?" "mkdir -p $DATA_PATH"
fi

chmod 750 $DATA_PATH
check_result "$?" "chmod 750 $DATA_PATH"

chown -R nobody:nobody $DATA_PATH
check_result "$?" "chown -R nobody:nobody $DATA_PATH"

mount_result=$(sudo /opt/mount_oper.sh mount_bind ${DATA_PATH} ${PRE_DATA_PATH})
log_info "The result of mount --bind ${DATA_PATH} ${PRE_DATA_PATH} is ${mount_result}"
log_info "Mount the data directory ends."

cd ${REDIS_PATH}

# 将配置存放于/opt/third_data/redis/conf/
if [ ! -d $THIRD_REDIS_PATH/conf ];then
  mkdir $THIRD_REDIS_PATH/conf
fi
chmod 700 ${THIRD_REDIS_PATH}/conf


if [ -f ${redis_conf_in_nas} ];then
    rm -rf ${redis_conf_in_nas}
fi
cp redis.conf ${redis_conf_in_nas}
chmod 600 ${redis_conf_in_nas}

LISTEN_ADDRESS=${POD_IP}

sed -i "/^bind/c\bind ${LISTEN_ADDRESS}" ${redis_conf_in_nas}
check_result "$?" "sed bind for redis.conf"

sed -i "/^daemonize/c\daemonize no" ${redis_conf_in_nas}
check_result "$?" "sed daemonize for redis.conf"

sed -i "/^appendonly no/c\appendonly yes" ${redis_conf_in_nas}
check_result "$?" "sed appendonly for redis.conf"

sed -i "/^protected-mode/c\protected-mode no" ${redis_conf_in_nas}
check_result "$?" "sed protected-mode for redis.conf"

sed -i "/^logfile.*/c\logfile ${PRE_LOG_FILE}" ${redis_conf_in_nas}
check_result "$?" "sed logfile for ${PRE_LOG_FILE} redis.conf"

sed -i "/^dir .*/c\dir ${PRE_DATA_PATH}" ${redis_conf_in_nas}
check_result "$?" "sed dir for ${PRE_DATA_PATH} redis.conf"

sed -i "/^# appendfsync always/c\appendfsync always" ${redis_conf_in_nas}
check_result "$?" "sed appendfsync always for redis.conf"

sed -i "/appendfsync everysec/c\# appendfsync everysec" ${redis_conf_in_nas}
check_result "$?" "sed appendfsync everysec for redis.conf"

sed -i "/^# maxmemory <bytes>/c\maxmemory ${REDIS_MAXMEMORY}" ${redis_conf_in_nas}
check_result "$?" "set maxmemory:${REDIS_MAXMEMORY} for ${redis_conf_in_nas}"

sed -i "/^# ignore-warnings ARM64-COW-BUG/c\ignore-warnings ARM64-COW-BUG" ${redis_conf_in_nas}
check_result "$?" "sed ARM64-COW-BUG for redis.conf"

for cmd in ${HIGH_RISK_CMD[*]}; do
    sed -i "/^#\+\sSECURITY\s#\+/a\rename-command ${cmd} \"\"" ${redis_conf_in_nas}
    check_result  "$?" "sed rename ${cmd} for redis.conf"
done

sed -i "/^port 6379/c\port 6369" ${redis_conf_in_nas}
check_result "$?" "sed port for redis.conf"

sed -i "/^# maxclients 10000/c\maxclients ${REDIS_MAXCLIENTS}" ${redis_conf_in_nas}
check_result "$?" "set maxclients:${REDIS_MAXCLIENTS} for redis.conf"

sed -i "/^timeout.*/c\timeout 1800" ${redis_conf_in_nas}
check_result "$?" "sed timeout for redis.conf"

# 纯软集群
config_map="/opt/multicluster-conf"
cluster=$(cat "${config_map}/CLUSTER")

if [[ "${cluster}" == "true" ]]; then
  sed -i "/^# cluster-enabled yes/c\cluster-enabled yes" ${redis_conf_in_nas}
  check_result "$?" "set cluster-enabled yes for redis.conf"

  sed -i "/^# cluster-config-file nodes-6379.conf/c\cluster-config-file nodes-6369.conf" ${redis_conf_in_nas}
  check_result "$?" "set cluster-config-file for redis.conf"

  sed -i "/^# cluster-node-timeout 15000/c\cluster-node-timeout 30000" ${redis_conf_in_nas}
  check_result "$?" "set cluster-node-timeout for redis.conf"

  sed -i "/^# tls-cluster yes/c\tls-cluster yes" ${redis_conf_in_nas}
  check_result "$?" "set tls-cluster for redis.conf"

  sed -i 's/# cluster-announce-tls-port .*/cluster-announce-tls-port 6369/' ${redis_conf_in_nas}
  check_result "$?" "set cluster-announce-tls-port for redis.conf"
fi

function start_cluster()
{
config_map="/opt/multicluster-conf"
cluster=$(cat "${config_map}/CLUSTER")
redis_tag=$(cat "${config_map}/REDIS_CLUSTER")

if [[ "${cluster}" == "true" ]]; then
  if [[ "${redis_tag}" == "false" ]] && [[ "${POD_NAME}" == "infrastructure-0" ]]; then

    master_ip=$(cat "${config_map}/MASTER_IP")
    standby_ip=$(cat "${config_map}/STANDBY_IP")
    slave_ip=$(cat "${config_map}/SLAVE_IP")
    PORT=6369

    url='https://infrastructure.dpa.svc.cluster.local:8088/v1/infra/secret/info?nameSpace=dpa&secretName=common-secret&secretKey=redis.password'
    pass="$(cat /opt/third_data/kafka/config/server.properties |grep ssl.truststore.password= | uniq | awk -F = '{print $2}')"
    response=$(curl -s -X GET -H "accept: */*" -H "Content-Type: application/json" --cert $INTER_CERT --key $INTER --pass $pass --cacert $INTER_CA_CERT $url)
    passwd="$(echo "$response" | grep -o '"redis.password": "[^"]*' | sed 's/.*: "//')"

    if [[ -n "${master_ip}" ]] && [[ -n "${standby_ip}" ]] && [[ -n "${slave_ip}" ]];then
      IP_LIST=("$master_ip" "$standby_ip" "$slave_ip")

      for IP in "${IP_LIST[@]}"; do
        /usr/local/redis/redis-*/redis-cli -h ${IP} -p ${PORT} --user default --pass ${passwd} --tls --cacert ${CA_CERT} --cert ${CERT} --key ${KEY} FLUSHDB
        /usr/local/redis/redis-*/redis-cli -h ${IP} -p ${PORT} --user default --pass ${passwd} --tls --cacert ${CA_CERT} --cert ${CERT} --key ${KEY} CLUSTER RESET
        if [ $? -eq 0 ];then
          log_info "${LINENO} Reset redis cluster ${IP} succeed"
        else
          log_error "${LINENO} Reset redis cluster ${IP} failed"
          exit 1
        fi
      done

      /usr/local/redis/redis-*/redis-cli --cluster create --cluster-replicas 0 ${master_ip}:${PORT} ${standby_ip}:${PORT} ${slave_ip}:${PORT} --user default --pass ${passwd} --tls --cacert ${CA_CERT} --cert ${CERT} --key ${KEY} --cluster-yes
      if [ $? -eq 0 ];then
        log_info "${LINENO} Start redis cluster succeed"
        PAYLOAD="{\"data\":{\"REDIS_CLUSTER\":\"true\"}}"
        update_k8s "${PAYLOAD}" "configmaps" "multicluster-conf"
        log_info "${LINENO} Set up multi-cluster redis tag succeed"
      else
        log_info "${LINENO} Start redis cluster failed"
        exit 1
      fi
    fi
  fi
else
  log_info "$(date +"[%Y-%m-%d %H:%M:%S,%N]")[INFO] No need to start redis cluster!"
fi
}

function update_k8s()
{
  local payload=$1
  local type=$2
  local type_info=$3
  for i in {1..3}
  do
    curl --cacert ${rootCAFile} \
      -X PATCH \
      -H "Content-Type: application/strategic-merge-patch+json" \
      -H "Authorization: Bearer ${tokenFile}" \
      --data "${payload}" \
      https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/dpa/${type}/${type_info}
  done
}

log_info "Starting update_password."
sh ${CURPATH}/update_password.sh
check_result "$?" "sh ${CURPATH}/update_password.sh"

log_info "Config Redis CERT Info."
config_cert

umask 027
# 启动redis最大重试次数
MAX_RETRY=3
# 当前重试次数
retry_count=0
log_info "$(date +"[%Y-%m-%d %H:%M:%S,%N]")[INFO] Starting redis-server. redis_path:${REDIS_PATH}/redis-server redis_conf_in_nas:${redis_conf_in_nas}"
while [ ${retry_count} -le ${MAX_RETRY} ];do
  ${REDIS_PATH}/redis-server ${redis_conf_in_nas} &
  L_RET=$?
  start_cluster
  redis_pid=$!
  wait ${redis_pid}

  if [ ${L_RET} -eq 0 ];then
    log_info "$(date +"[%Y-%m-%d %H:%M:%S,%N]")[INFO] Start redis server success!"
    break
  fi
  # 修复AOF文件故障导致的redis服务起不来
  if [ ${L_RET} -ne 0 ];then
    log_error "$(date +"[%Y-%m-%d %H:%M:%S,%N]")[ERROR] Redis server error:${L_RET}."
    tail -n 10 "${LOG_PATH}/redis.log" | grep "Bad file format reading the append only file"
    # 返回值等于0说明AOF文件有异常
    if [ $? -eq 0 ];then
      log_info "Try to fix redis AOF: ${REDIS_AOF_PATH}."
      # Bad file format reading the append only file: make a backup of your AOF file,then use ./redis-check-aof --fix <filename
      echo "y" | ${REDIS_PATH}/redis-check-aof --fix "${REDIS_AOF_PATH}"
      log_info "$(date +"[%Y-%m-%d %H:%M:%S,%N]") Start redis server again."
      ${REDIS_PATH}/redis-server ${redis_conf_in_nas}
      if [ $? -eq 0 ];then
        log_info "$(date +"[%Y-%m-%d %H:%M:%S,%N]")[INFO] Redis server fix AOF success:$?."
        break
      fi
      # 命令执行失败，检查是否已达到最大重试次数
      if [ ${retry_count} -ge ${MAX_RETRY} ]; then
        log_info "$(date +"[%Y-%m-%d %H:%M:%S,%N]")[ERROR] Restart redis server failed after ${MAX_RETRY} attempts."
        exit 1
      fi
    fi
  fi
  log_info "Command start redis server failed, retrying in 10 seconds..."
  retry_count=$((retry_count+1))
  sleep 10
done