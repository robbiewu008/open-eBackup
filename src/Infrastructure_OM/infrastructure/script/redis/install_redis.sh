#!/bin/bash

NODE_NAME=${NODE_NAME}
DATA_PATH="/opt/third_data/redis/data"
REPLICA_DATA_PATH="/opt/third_data/redis/data-1"
THIRD_REDIS_PATH="/opt/third_data/redis"
LOG_PATH="/opt/OceanProtect/logs/${NODE_NAME}/infrastructure/redis"
INSTALL_LOG_PATH=${LOG_PATH}/install_redis.log
REDIS_PATH="/usr/local/redis/redis-VERSION"
PRE_DATA_PATH="${REDIS_PATH}/data"
PRE_LOG_PATH="${REDIS_PATH}/logs"
PRE_LOG_FILE=$PRE_LOG_PATH/redis.log
CURPATH=$(
  cd "$(dirname "$0")"
  pwd
)

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
function config_cert() {
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
  sed -i "/# tls-replication yes/i tls-replication yes" ${redis_conf_in_nas}

  return 0
}

# 挂载log目录
if [ ! -d $LOG_PATH ]; then
  mkdir -p $LOG_PATH
fi
touch ${LOG_PATH}/install_redis.log

chmod 750 $LOG_PATH
chown nobody:nobody ${LOG_PATH}/install_redis.log
chmod 640 ${LOG_PATH}/install_redis.log
if [ ! -f "${LOG_PATH}/redis.log" ]; then
  touch ${LOG_PATH}/redis.log
fi
chmod 640 "${LOG_PATH}/redis.log"

function log_info() {
  echo "$(date +"[%Y-%m-%d %H:%M:%S,%N]")[INFO][INSTALL_REDIS: $1][$0][${BASH_LINENO}]"
  echo "$(date +"[%Y-%m-%d %H:%M:%S,%N]")[INFO][INSTALL_REDIS: $1][$0][${BASH_LINENO}]" >>"${INSTALL_LOG_PATH}"
}

function log_error() {
  echo "$(date +"[%Y-%m-%d %H:%M:%S,%N]")[ERROR][INSTALL_REDIS: $1][$0][${BASH_LINENO}]"
  echo "$(date +"[%Y-%m-%d %H:%M:%S,%N]")[ERROR][INSTALL_REDIS: $1][$0][${BASH_LINENO}]" >>"${INSTALL_LOG_PATH}"
}

function check_result() {
  if [ "$1" != "0" ]; then
    log_error "Exec cmd:$2 failed."
  else
    log_info "Exec cmd:$2 success."
  fi
}

# 清理残留nfs文件
find ${LOG_PATH} -type f -name ".nfs*" -delete
check_result "$?" "${LINENO} find ${LOG_PATH} -type f -name \".nfs*\" -delete"

mount_result=$(sudo /opt/mount_oper.sh mount_bind ${LOG_PATH} ${PRE_LOG_PATH})
log_info "The result of mount --bind ${LOG_PATH} ${PRE_LOG_PATH} is ${mount_result}"

# 挂载data目录
log_info "Start to mount the data directory."
if [ ! -d $DATA_PATH ]; then
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
if [ ! -d $THIRD_REDIS_PATH/conf ]; then
  mkdir $THIRD_REDIS_PATH/conf
fi
chmod 700 ${THIRD_REDIS_PATH}/conf

if [ -f ${redis_conf_in_nas} ]; then
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
  check_result "$?" "sed rename ${cmd} for redis.conf"
done

sed -i "/^port 6379/c\port 6369" ${redis_conf_in_nas}
check_result "$?" "sed port for redis.conf"

sed -i "/^# maxclients 10000/c\maxclients ${REDIS_MAXCLIENTS}" ${redis_conf_in_nas}
check_result "$?" "set maxclients:${REDIS_MAXCLIENTS} for redis.conf"

sed -i "/^timeout.*/c\timeout 1800" ${redis_conf_in_nas}
check_result "$?" "sed timeout for redis.conf"

# 纯软集群
config_map="/opt/multicluster-conf"

if [[ "${CLUSTER}" == "TRUE" ]]; then
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


function redis_cli() {
  IP="$1"
  PORT="$2"
  shift 2
  CMD="$@"
  /usr/local/redis/redis-*/redis-cli -h $IP -p $PORT --user default --tls --cacert ${CA_CERT} --cert ${CERT} --key ${KEY} $CMD
}


function wait_for_redis_ready() {
  while true; do
    pong=$(redis_cli $1 $2 PING)
    if [[ "$pong" == "PONG" ]]; then
      break
    fi
    sleep 1
  done
}


function reset_redis_node() {
  IP=$1
  PORT=$2

  wait_for_redis_ready $IP $PORT
  redis_cli $IP $PORT FLUSHDB
  redis_cli $IP $PORT CLUSTER RESET HARD
  if [ $? -eq 0 ]; then
    log_info "${LINENO} Reset redis cluster ${IP} succeed"
  else
    log_error "${LINENO} Reset redis cluster ${IP} failed"
    exit 1
  fi
}


function get_redis_password() {
  url="https://$POD_IP:8088/v1/infra/secret/info?nameSpace=dpa&secretName=common-secret&secretKey=redis.password"
  pass="$(cat /opt/third_data/kafka/config/server.properties | grep ssl.truststore.password= | uniq | awk -F = '{print $2}')"
  response=$(LD_PRELOAD=/usr/lib64/libSecurityStarter.so curl -ks -X GET -H "accept: */*" -H "Content-Type: application/json" --cert $INTER_CERT --key $INTER --pass $pass --cacert $INTER_CA_CERT $url follow@--pass)
  echo "$response" | grep -o '"redis.password": "[^"]*' | sed 's/.*: "//'
}


function start_master_cluster() {
  log_info "Start to create redis master cluster!"
  REDIS_NODES=''
  for IP in "${REDIS_MASTER_IP_LIST[@]}"; do
    REDIS_NODES="$REDIS_NODES $IP:$REDIS_MASTER_PORT"
  done

  /usr/local/redis/redis-*/redis-cli --cluster create --cluster-replicas 0 ${REDIS_NODES} --user default --tls --cacert ${CA_CERT} --cert ${CERT} --key ${KEY} --cluster-yes
  if [ $? -eq 0 ]; then
    log_info "${LINENO} Start redis master cluster succeed"
  else
    log_error "${LINENO} Start redis master cluster failed"
    exit 1
  fi
}

function get_redis_id() {
  IP=$1
  PORT=$2
  CLUSTER_NODES=$(redis_cli $IP $PORT CLUSTER NODES)
  echo "$CLUSTER_NODES" | grep "myself" | awk '{print $1}'
}

function add_replicas_to_cluster() {
  for index in ${!REDIS_REPLICAS_IP_LIST[@]}; do
    REPLICA_IP=${REDIS_REPLICAS_IP_LIST[$index]}
    MASTER_IP=${REDIS_MASTER_IP_LIST[$index]}
    log_info "start to meet"
    redis_cli $MASTER_IP $REDIS_MASTER_PORT CLUSTER MEET $REPLICA_IP $REDIS_REPLICAS_PORT
    if [ $? -eq 0 ]; then
      log_info "Successfully cluster meet $REPLICA_IP"
    else
      log_error "Failed to cluster meet $REPLICA_IP "
      exit 1
    fi

    MASTER_ID=$(get_redis_id "$MASTER_IP" "$REDIS_MASTER_PORT")
    echo "start to set ${REPLICA_IP} to replicate of ${MASTER_IP} ${MASTER_ID}"
    while true; do
      # 等待从节点同步主节点信息
      redis_cli $REPLICA_IP $REDIS_REPLICAS_PORT CLUSTER NODES | grep -c $MASTER_ID
      if [ $? -eq 0 ];then
        log_info replica $REPLICA_IP get master info $MASTER_ID success
        break
      fi
      sleep 1
    done
    redis_cli $REPLICA_IP $REDIS_REPLICAS_PORT CLUSTER REPLICATE $MASTER_ID
    if [ $? -eq 0 ]; then
      log_info "Successfully set $REPLICA_IP to replicate $MASTER_IP ${MASTER_ID}"
    else
      log_error "Failed to set $REPLICA_IP to replicate $MASTER_IP ${MASTER_ID}"
      exit 1
    fi
  done
}


# 检查集群状态，如果6369对应的redis实例不是master,使用failover将其设置为master
# 避免出现3节点都正常的情况下，2个master都在一个节点上的情况
function failover_to_master() {
  PORT=$REDIS_MASTER_PORT
  while true; do
    for IP in ${REDIS_MASTER_IP_LIST[@]}; do
      redis_cli $IP $PORT role | grep -q ^slave
      if [[ $? -eq 0 ]]; then
        log_info "Found $IP $PORT is slave, start to failover to master"
        redis_cli $IP $PORT CLUSTER FAILOVER
        if [ $? -eq 0 ]; then
          log_info "Successfully cluster failover current redis node $IP:$PORT to master"
        else
          log_error "Failed to cluster failover current redis node $IP:$PORT to master"
        fi
      fi
    done
    sleep 60
  done
}


function begin_redis_write_aof() {
  redis_cli $1 $2 BGREWRITEAOF
  while true; do
    aof_in_progress=$(redis_cli $1 $2 INFO PERSISTENCE | grep 'aof_rewrite_in_progress' | cut -d: -f2)
    aof_in_progress=${aof_in_progress//[[:space:]]/}
    if [ "$aof_in_progress" -eq 0 ]; then
      log_info "Successfully rewrite redis aof data"
      break
    fi
    sleep 1
  done
}


# 保存当前节点单机redis数据，用于组建完集群后，导入集群
function save_redis_data_for_migration() {
  begin_redis_write_aof $POD_IP $REDIS_MASTER_PORT
  rm -rf /opt/third_data/redis/migrate
  mkdir -p /opt/third_data/redis/migrate
  cp -r /opt/third_data/redis/data /opt/third_data/redis/migrate/data
}


function start_redis_data_migration() {
  # kill redis进程，替换aof文件，最后使用--cluster fix修复数据，完成数据导入
  redis_pid=$(ps -ef | grep $POD_IP:6369 | grep -v grep | awk '{print $2}')
  kill "$redis_pid"
  wait "$redis_pid"
  cp /opt/third_data/redis/migrate/data/appendonly.aof /opt/third_data/redis/data/appendonly.aof

  ${REDIS_PATH}/redis-server ${redis_conf_in_nas} &
  redis_pid=$!
  wait_for_redis_ready $POD_IP $REDIS_MASTER_PORT

  if ! redis_cli $POD_IP $REDIS_MASTER_PORT --cluster fix $POD_IP:$REDIS_MASTER_PORT; then
    log_error "Failed to migrate redis data"
  else
    log_info "Successfully migrate redis data"
  fi
}


function start_redis() {
  ${REDIS_PATH}/redis-server ${redis_conf_in_nas} &
  redis_pid=$!

  config_map="/opt/multicluster-conf"
  redis_cluster=$(cat "${config_map}/REDIS_CLUSTER")
  redis_replicas_tag=$(cat "${config_map}/REDIS_CLUSTER_REPLICAS")

  if [[ "${CLUSTER}" != "TRUE" ]]; then
    log_info "$(date +"[%Y-%m-%d %H:%M:%S,%N]")[INFO] No need to start redis cluster!"
    return
  fi

  export REDISCLI_AUTH=$(get_redis_password)
  if [ -z "$REDISCLI_AUTH" ]; then
    log_error "failed to get redis cli auth"
    exit 1
  fi
  # start replicas redis instance
  if [ ! -d $REPLICA_DATA_PATH ]; then
    mkdir -p $REPLICA_DATA_PATH
    check_result "$?" "mkdir -p $REPLICA_DATA_PATH"
  fi
  redis_backup_conf_in_nas=$THIRD_REDIS_PATH/conf/redis-1.conf
  cp ${redis_conf_in_nas} ${redis_backup_conf_in_nas}
  sed -i "s/^tls-port 6369/tls-port 6370/g" ${redis_backup_conf_in_nas}
  sed -i "s/^cluster-config-file nodes-6369.conf/cluster-config-file nodes-6370.conf/g" ${redis_backup_conf_in_nas}
  sed -i "s/^cluster-announce-tls-port 6369/cluster-announce-tls-port 6370/g" ${redis_backup_conf_in_nas}
  sed -i 's|^dir /usr/local/redis/redis-6.2.14/data|dir /opt/third_data/redis/data-1|' ${redis_backup_conf_in_nas}
  sed -i 's|^logfile .*$|logfile /opt/third_data/redis/redis-1.log|' ${redis_backup_conf_in_nas}
  ${REDIS_PATH}/redis-server ${redis_backup_conf_in_nas} &

  if [[ "${POD_NAME}" != "infrastructure-0" ]]; then
    return
  fi

  master_ip=$(cat "${config_map}/MASTER_IP")
  standby_ip=$(cat "${config_map}/STANDBY_IP")
  slave_ip=$(cat "${config_map}/SLAVE_IP")
  if [[ -z "${master_ip}" ]] || [[ -z "${standby_ip}" ]] || [[ -z "${slave_ip}" ]]; then
    log_error "Invalid master_ip=${master_ip}, standby_ip=${standby_ip} slave_ip=${slave_ip}"
    exit 1
  fi

  REDIS_MASTER_IP_LIST=($master_ip $standby_ip $slave_ip)
  REDIS_MASTER_PORT=6369
  REDIS_REPLICAS_IP_LIST=($standby_ip $slave_ip $master_ip)
  REDIS_REPLICAS_PORT=6370

  if [[ "${redis_cluster}" != "true" ]]; then
    save_redis_data_for_migration

    for IP in "${REDIS_MASTER_IP_LIST[@]}"; do
      reset_redis_node "$IP" "$REDIS_MASTER_PORT"
    done
    start_master_cluster
    start_redis_data_migration

    PAYLOAD="{\"data\":{\"REDIS_CLUSTER\":\"true\"}}"
    update_k8s "${PAYLOAD}" "configmaps" "multicluster-conf"
    log_info "${LINENO} Set up multi-cluster redis tag succeed"
  fi

  # 添加3个备节点到集群
  if [[ "${redis_replicas_tag}" != "true" ]]; then
    for IP in "${REDIS_REPLICAS_IP_LIST[@]}"; do
      reset_redis_node "$IP" "$REDIS_REPLICAS_PORT"
    done
    add_replicas_to_cluster
    PAYLOAD="{\"data\":{\"REDIS_CLUSTER_REPLICAS\":\"true\"}}"
    update_k8s "${PAYLOAD}" "configmaps" "multicluster-conf"
    log_info "${LINENO} Set up multi-cluster redis replica tag succeed"
  fi

  failover_to_master&

  unset REDISCLI_AUTH
}


function update_k8s() {
  local payload=$1
  local type=$2
  local type_info=$3
  for i in {1..3}; do
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

while [ ${retry_count} -le ${MAX_RETRY} ]; do

  start_redis
  wait ${redis_pid}
  L_RET=$?

  if [ ${L_RET} -eq 0 ]; then
    log_info "$(date +"[%Y-%m-%d %H:%M:%S,%N]")[INFO] Start redis server success!"
    break
  fi
  # 修复AOF文件故障导致的redis服务起不来
  if [ ${L_RET} -ne 0 ]; then
    log_error "$(date +"[%Y-%m-%d %H:%M:%S,%N]")[ERROR] Redis server error:${L_RET}."
    tail -n 10 "${LOG_PATH}/redis.log" | grep "Bad file format reading the append only file"
    # 返回值等于0说明AOF文件有异常
    if [ $? -eq 0 ]; then
      log_info "Try to fix redis AOF: ${REDIS_AOF_PATH}."
      # Bad file format reading the append only file: make a backup of your AOF file,then use ./redis-check-aof --fix <filename
      echo "y" | ${REDIS_PATH}/redis-check-aof --fix "${REDIS_AOF_PATH}"
      log_info "$(date +"[%Y-%m-%d %H:%M:%S,%N]") Start redis server again."
      ${REDIS_PATH}/redis-server ${redis_conf_in_nas}
      if [ $? -eq 0 ]; then
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
  retry_count=$((retry_count + 1))
  sleep 10
done
