#!/bin/bash

NODE_NAME=${NODE_NAME}
DATA_PATH="/opt/third_data/kafka/data"
KAFKA_NAS_PATH="/opt/third_data/kafka"
LOG_PATH="/opt/OceanProtect/logs/${NODE_NAME}/infrastructure/kafka"
INSTALL_LOG_PATH=${LOG_PATH}/install_kafka.log
KAFKA_PATH="/usr/local/kafka/kafka-VERSION"
PRE_DATA_PATH="${KAFKA_PATH}/data"
PRE_LOG_PATH="${KAFKA_PATH}/logs"
zookeeper_path="/usr/local/zookeeper/data"
CERT_PATH="/opt/OceanProtect/infrastructure/cert/internal"
KMC_TOOL_PATH="/usr/bin"

MASTER_KS='/opt/OceanProtect/protectmanager/kmc/master.ks'
BACKUP_KS='/kmc_conf/..data/backup.ks'

# 挂载log目录
if [ ! -d $LOG_PATH ]; then
  mkdir -p $LOG_PATH
  chmod 750 $LOG_PATH
fi
touch ${LOG_PATH}/install_kafka.log
chmod 640 ${LOG_PATH}/install_kafka.log

function log_info() {
  echo "$(date +"[%Y-%m-%d %H:%M:%S,%N]")[INFO][INSTALL_KAFKA: $1][$0][${BASH_LINENO}]"
  echo "$(date +"[%Y-%m-%d %H:%M:%S,%N]")[INFO][INSTALL_KAFKA: $1][$0][${BASH_LINENO}]" >>"${INSTALL_LOG_PATH}"
}

function log_error() {
  echo "$(date +"[%Y-%m-%d %H:%M:%S,%N]")[ERROR][INSTALL_KAFKA: $1][$0][${BASH_LINENO}]"
  echo "$(date +"[%Y-%m-%d %H:%M:%S,%N]")[ERROR][INSTALL_KAFKA: $1][$0][${BASH_LINENO}]" >>"${INSTALL_LOG_PATH}"
}

function check_result() {
  if [ "$1" != "0" ]; then
    log_error "Exec cmd:$2 failed."
  else
    log_info "Exec cmd:$2 success."
  fi
}
mount_result=$(sudo /opt/mount_oper.sh mount_bind ${LOG_PATH} ${PRE_LOG_PATH})
log_info "The result of mount --bind ${LOG_PATH} ${PRE_LOG_PATH} is ${mount_result}"
# 挂载data目录
log_info "Start to mount the data directory."
if [ ! -d $DATA_PATH ]; then
  mkdir -p $DATA_PATH
  check_result "$?" "mkdir -p $DATA_PATH"
  chmod 750 $DATA_PATH
  check_result "$?" "chmod 750 $DATA_PATH"
fi

check_result "$?" "chown -R nobody:nobody $DATA_PATH"

mount_result=$(sudo /opt/mount_oper.sh mount_bind ${DATA_PATH} ${PRE_DATA_PATH})
log_info "The result of mount --bind ${DATA_PATH} ${PRE_DATA_PATH} is ${mount_result}"
log_info "Mount the data directory ends."

# 编写sasl配置的kafka_server_jaas.conf文件
if [ ! -d ${KAFKA_NAS_PATH}/config ]; then
  mkdir ${KAFKA_NAS_PATH}/config
fi
chmod 750 "${KAFKA_NAS_PATH}/config"
KAFKA_JAAS_IN_NAS=${KAFKA_NAS_PATH}/config/kafka_server_jaas.conf
if [ -f ${KAFKA_JAAS_IN_NAS} ]; then
  rm -rf ${KAFKA_JAAS_IN_NAS}
  check_result "$?" "rm -rf ${KAFKA_JAAS_IN_NAS}"
fi
touch ${KAFKA_JAAS_IN_NAS}

chmod 600 ${KAFKA_JAAS_IN_NAS}
check_result "$?" "chmod 600 ${KAFKA_JAAS_IN_NAS}"

python3 update_jaas_conf.py ${MASTER_KS} ${BACKUP_KS}
if [ $? != 0 ]; then
  log_error "Update kafka_server_jaas.conf failed."
  exit 1
fi
mount_result=$(sudo /opt/mount_oper.sh mount_bind ${KAFKA_JAAS_IN_NAS} ${KAFKA_PATH}/config/kafka_server_jaas.conf)
log_info "The result of mount --bind ${KAFKA_JAAS_IN_NAS} ${KAFKA_PATH}/config/kafka_server_jaas.conf is ${mount_result}"

# move server.properties to nas, delete it every time
KAFKA_CFG_IN_NAS=${KAFKA_NAS_PATH}/config/server.properties
if [ -f ${KAFKA_CFG_IN_NAS} ]; then
  rm -rf ${KAFKA_CFG_IN_NAS}
  check_result "$?" "rm -rf ${KAFKA_CFG_IN_NAS}"
fi
cp ${KAFKA_PATH}/config/server.properties ${KAFKA_CFG_IN_NAS}

LISTEN_ADDRESS=${POD_IP}

# 修改kafka配置文件参数server.properties
check_result "$?" "cp ${KAFKA_PATH}/config/server.properties ${KAFKA_CFG_IN_NAS}"

sed -i "/Socket Server Settings.*$/a\ssl.endpoint.identification.algorithm=" ${KAFKA_CFG_IN_NAS}
check_result "$?" "sed ssl.endpoint.identification.algorithm for ${KAFKA_CFG_IN_NAS}"

sed -i "/Socket Server Settings.*$/a\listeners=SASL_SSL://${LISTEN_ADDRESS}:kafka_port" ${KAFKA_CFG_IN_NAS}
check_result "$?" "sed listeners for ${KAFKA_CFG_IN_NAS}"

sed -i "/Socket Server Settings.*$/a\ssl.enabled.protocols=TLSv1.2" ${KAFKA_CFG_IN_NAS}
check_result "$?" "sed ssl.enabled.protocols for ${KAFKA_CFG_IN_NAS}"

sed -i "/Socket Server Settings.*$/a\ssl.cipher.suites=TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384,TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256" ${KAFKA_CFG_IN_NAS}
check_result "$?" "sed ssl.cipher.suites for ${KAFKA_CFG_IN_NAS}"

sed -i "/Socket Server Settings.*$/a\listener.security.protocol.map=SASL_SSL:SASL_SSL" ${KAFKA_CFG_IN_NAS}
check_result "$?" "sed listener.security.protocol.map for ${KAFKA_CFG_IN_NAS}"

sed -i "/Socket Server Settings.*$/a\security.inter.broker.protocol=SASL_SSL" ${KAFKA_CFG_IN_NAS}
check_result "$?" "sed security.inter.broker.protocol for ${KAFKA_CFG_IN_NAS}"

sed -i "/Socket Server Settings.*$/a\sasl.mechanism.inter.broker.protocol=PLAIN" ${KAFKA_CFG_IN_NAS}
check_result "$?" "sed sasl.mechanism.inter.broker.protocol for ${KAFKA_CFG_IN_NAS}"

sed -i "/Socket Server Settings.*$/a\sasl.enabled.mechanisms=PLAIN" ${KAFKA_CFG_IN_NAS}
check_result "$?" "sed sasl.enabled.mechanisms for ${KAFKA_CFG_IN_NAS}"

sed -i "/Socket Server Settings.*$/a\ssl.keystore.type=PKCS12" ${KAFKA_CFG_IN_NAS}
check_result "$?" "sed ssl.keystore.type for ${KAFKA_CFG_IN_NAS}"

sed -i "/Socket Server Settings.*$/a\ssl.keystore.location=${CERT_PATH}/internal.ks" ${KAFKA_CFG_IN_NAS}
check_result "$?" "sed ssl.keystore.location for ${KAFKA_CFG_IN_NAS}"

sed -i "/Socket Server Settings.*$/a\ssl.truststore.type=PKCS12" ${KAFKA_CFG_IN_NAS}
check_result "$?" "sed ssl.truststore.type for ${KAFKA_CFG_IN_NAS}"

sed -i "/Socket Server Settings.*$/a\ssl.truststore.location=${CERT_PATH}/internal.ks" ${KAFKA_CFG_IN_NAS}
check_result "$?" "sed ssl.truststore.location for ${KAFKA_CFG_IN_NAS}"

sed -i "/Socket Server Settings.*$/a\ssl.protocol=TLS" ${KAFKA_CFG_IN_NAS}
check_result "$?" "sed ssl.protocol for ${KAFKA_CFG_IN_NAS}"

sed -i "s/num.partitions.*$/num.partitions=${KAFKA_PARTITIONS}/g" ${KAFKA_CFG_IN_NAS}
check_result "$?" "sed num.partitions for ${KAFKA_CFG_IN_NAS}"

sed -i "/^log.dirs=.*/c\log.dirs=${PRE_DATA_PATH}" ${KAFKA_CFG_IN_NAS}
check_result "$?" "sed log.dirs for ${KAFKA_CFG_IN_NAS}"

sed -i "/^num.network.threads=.*/c\num.network.threads=${KFAKA_NUM_NETWORK_THREADS}" ${KAFKA_CFG_IN_NAS}
check_result "$?" "sed num.network.threads for ${KAFKA_CFG_IN_NAS}"

sed -i "/^num.io.threads=.*/c\num.io.threads=${KFAKA_NUM_IO_THREADS}" ${KAFKA_CFG_IN_NAS}
check_result "$?" "sed num.io.threads for ${KAFKA_CFG_IN_NAS}"

sed -i "/^#log.flush.interval.messages=.*/c\log.flush.interval.messages=1" ${KAFKA_CFG_IN_NAS}
check_result "$?" "sed log.flush.interval.messages for ${KAFKA_CFG_IN_NAS}"

ZK_CFG_IN_NAS=${KAFKA_NAS_PATH}/config/zookeeper.properties
if [ -f ${ZK_CFG_IN_NAS} ]; then
  rm -rf ${ZK_CFG_IN_NAS}
fi
cp ${KAFKA_PATH}/config/zookeeper.properties ${ZK_CFG_IN_NAS}
check_result "$?" "cp ${KAFKA_PATH}/config/zookeeper.properties ${ZK_CFG_IN_NAS}"

sed -i "/^dataDir=.*/c\dataDir=${zookeeper_path}" ${ZK_CFG_IN_NAS}
check_result "$?" "sed dataDir for ${ZK_CFG_IN_NAS}"
mount_result=$(sudo /opt/mount_oper.sh mount_bind ${ZK_CFG_IN_NAS} ${KAFKA_PATH}/config/zookeeper.properties)
log_info "The result of mount --bind ${ZK_CFG_IN_NAS} ${KAFKA_PATH}/config/zookeeper.properties is ${mount_result}"

sed -i "/^log.retention.hours=.*/c\log.retention.hours=24" ${KAFKA_CFG_IN_NAS}
check_result "$?" "sed log.retention.hours for ${KAFKA_CFG_IN_NAS}"

echo 'message.max.bytes=2097152' >>${KAFKA_CFG_IN_NAS}
check_result "$?" "echo message.max.bytes=2097152 for ${KAFKA_CFG_IN_NAS}"

echo 'replica.fetch.max.bytes=2621440' >>${KAFKA_CFG_IN_NAS}
check_result "$?" "echo replica.fetch.max.bytes=2621440 for ${KAFKA_CFG_IN_NAS}"

# 该连接host为zookeeper监听的ip
sed -i "s%zookeeper.connect=.*$%zookeeper.connect=infrastructure:zookeeper_port%g" ${KAFKA_CFG_IN_NAS}
check_result "$?" "sed zookeeper.connect for ${KAFKA_CFG_IN_NAS}"

sed -i "/^num.recovery.threads.per.data.dir=.*/c\num.recovery.threads.per.data.dir=${KFAKA_NUM_RECOVERY_THREADS_PER_DATA_DIR}" ${KAFKA_CFG_IN_NAS}
check_result "$?" "sed num.recovery.threads.per.data.dir for ${KAFKA_CFG_IN_NAS}"

sed -i "/^log.retention.check.interval.ms=.*/c\log.retention.check.interval.ms=300000\nlog.cleanup.policy=delete\nlog.cleaner.enable=true" ${KAFKA_CFG_IN_NAS}
check_result "$?" "sed log.retention.check.interval.ms for ${KAFKA_CFG_IN_NAS}"

sed -i "/^zookeeper.connection.timeout.ms=.*/c\zookeeper.connection.timeout.ms=180000" ${KAFKA_CFG_IN_NAS}
check_result "$?" "sed zookeeper.connection.timeout.ms for ${KAFKA_CFG_IN_NAS}"

# 软硬解耦集群配置
config_map="/opt/multicluster-conf"
zk_cluster=$(cat "${config_map}/ZK_CLUSTER")
NODE_NUMBER=$(echo "$NODE_NAME" | awk '{print substr($0,length,1)}')

if [[ "${CLUSTER}" == "TRUE" ]]; then
  master_node=$(cat "${config_map}/MASTER")
  standby_node=$(cat "${config_map}/STANDBY")

  # 查询基础设施所在三个节点ip
  master_ip=$(cat "${config_map}/MASTER_IP")
  standby_ip=$(cat "${config_map}/STANDBY_IP")
  slave_ip=$(cat "${config_map}/SLAVE_IP")

  if [[ -n "${master_ip}" ]] && [[ -n "${standby_ip}" ]] && [[ -n "${slave_ip}" ]]; then
    sed -i "s%zookeeper.connect=.*$%zookeeper.connect=${master_ip}:2181,${standby_ip}:2181,${slave_ip}:2181%g" ${KAFKA_CFG_IN_NAS}
    check_result "$?" "sed zookeeper cluster for ${KAFKA_CFG_IN_NAS} in node ${LISTEN_ADDRESS}"

    sed -i '$a\default.replication.factor=3' ${KAFKA_CFG_IN_NAS}
    check_result "$?" "default.replication.factor for ${KAFKA_CFG_IN_NAS}"

    sed -i 's/offsets.topic.replication.factor.*$/offsets.topic.replication.factor=3/g' ${KAFKA_CFG_IN_NAS}
    check_result "$?" "sed offsets.topic.replication.factor for ${KAFKA_CFG_IN_NAS}"

    sed -i '$a\min.insync.replicas=2' ${KAFKA_CFG_IN_NAS}
    check_result "$?" "sed min.insync.replicas for ${KAFKA_CFG_IN_NAS}"

    sed -i '$a\unclean.leader.election.enable=false' ${KAFKA_CFG_IN_NAS}
    check_result "$?" "sed unclean.leader.election.enable for ${KAFKA_CFG_IN_NAS}"

    sed -i 's/num.partitions.*$/num.partitions=6/g' ${KAFKA_CFG_IN_NAS}
    check_result "$?" "sed num.partitions for ${KAFKA_CFG_IN_NAS}"

    sed -i 's/transaction.state.log.replication.factor.*$/transaction.state.log.replication.factor=3/g' ${KAFKA_CFG_IN_NAS}
    check_result "$?" "sed transaction.state.log.replication.factor for ${KAFKA_CFG_IN_NAS}"

    sed -i 's/transaction.state.log.min.isr.*$/transaction.state.log.min.isr=2/g' ${KAFKA_CFG_IN_NAS}
    check_result "$?" "sed transaction.state.log.min.isr for ${KAFKA_CFG_IN_NAS}"

    if [[ ${NODE_NAME} == ${master_node} ]]; then
      sed -i "s%broker.id=.*$%broker.id=${NODE_NUMBER}%g" ${KAFKA_CFG_IN_NAS}
      check_result "$?" "sed broker ID for ${KAFKA_CFG_IN_NAS}"
    elif [[ ${NODE_NAME} == ${standby_node} ]]; then
      sed -i "s%broker.id=.*$%broker.id=${NODE_NUMBER}%g" ${KAFKA_CFG_IN_NAS}
      check_result "$?" "sed broker ID for ${KAFKA_CFG_IN_NAS}"
    else
      sed -i "s%broker.id=.*$%broker.id=${NODE_NUMBER}%g" ${KAFKA_CFG_IN_NAS}
      check_result "$?" "sed broker ID for ${KAFKA_CFG_IN_NAS}"
      # 组建集群需要清除单节点数据，防止脑裂
      if [[ "${zk_cluster}" == "false" ]]; then
        if [[ ! -d ${KAFKA_NAS_PATH}/backup ]]; then
          mkdir ${KAFKA_NAS_PATH}/backup
        fi
        cp -rf ${DATA_PATH}/* ${KAFKA_NAS_PATH}/backup
        rm -rf ${DATA_PATH}/*
      fi
    fi
  else
    log_error "Starting kafka cluster in node ${LISTEN_ADDRESS} failed."
    exit 1
  fi
else
  sed -i "/Socket Server Settings.*$/a\advertised.listeners=SASL_SSL://infrastructure:kafka_port" ${KAFKA_CFG_IN_NAS}
  check_result "$?" "sed advertised.listeners for ${KAFKA_CFG_IN_NAS}"
fi

# move to nas, delete it every time
KAFKA_START_SHELL_IN_NAS=${KAFKA_NAS_PATH}/config/kafka-server-start.sh
if [ -f ${KAFKA_START_SHELL_IN_NAS} ]; then
  rm -rf ${KAFKA_START_SHELL_IN_NAS}
  check_result "$?" "rm -rf ${KAFKA_START_SHELL_IN_NAS}"
fi
cp ${KAFKA_PATH}/bin/kafka-server-start.sh ${KAFKA_START_SHELL_IN_NAS}
sed -i "s%export KAFKA_HEAP_OPTS=.*$%export KAFKA_HEAP_OPTS=\"${KAFKA_HEAP_OPTS}\"%g" ${KAFKA_START_SHELL_IN_NAS}
check_result "$?" "set KAFKA_HEAP_OPTS:${KAFKA_HEAP_OPTS} for ${KAFKA_START_SHELL_IN_NAS}"

mount_result=$(sudo /opt/mount_oper.sh mount_bind ${KAFKA_START_SHELL_IN_NAS} ${KAFKA_PATH}/bin/kafka-server-start.sh)
log_info "The result of sudo /opt/mount_oper.sh mount_bind ${KAFKA_START_SHELL_IN_NAS} ${KAFKA_PATH}/bin/kafka-server-start.sh is ${mount_result}"

# kmc工具为server.properties配置文件编写关键配置存于临时文件中
${KMC_TOOL_PATH}/kmcdecrypt ${MASTER_KS} ${BACKUP_KS} ${CERT_PATH}/internal_cert -k tmp_server.properties
check_result "$?" "${KMC_TOOL_PATH}/kmcdecrypt ${MASTER_KS} ${BACKUP_KS} ${CERT_PATH}/internal_cert -k"

# 覆盖原有配置文件
mv ${KAFKA_NAS_PATH}/config/tmp_server.properties ${KAFKA_CFG_IN_NAS}
check_result "$?" "mv ${KAFKA_NAS_PATH}/config/tmp_server.properties ${KAFKA_CFG_IN_NAS}"

chmod 600 ${KAFKA_CFG_IN_NAS}
check_result "$?" "chmod 600 ${KAFKA_CFG_IN_NAS}"

# 等待ZK启动，端口可用
while true; do
  netstat -tunple | grep zookeeper_port >>/dev/null
  if [ $? -ne 0 ]; then
    log_info "Waiting for zookeeper service."
    sleep 1
  else
    log_info "Find zookeeper_port success."
    break
  fi
done

# 主动删除/brokers/ids 防止zk上的node未删除造成kafka启动失败
if [[ "${CLUSTER}" == "TRUE" ]]; then
  python3 remove_zk_broker_id.py "$NODE_NUMBER"
  if [ "$?" != 0 ]; then
    log_error "Remove /brokers/ids/'${NODE_NUMBER}' failed."
    exit 1
  fi
  check_result "$?" "Remove /brokers/ids/'${NODE_NUMBER}'."
else
  python3 remove_zk_broker_id.py "0"
  if [ "$?" != 0 ]; then
    log_error "Remove /brokers/ids/0 failed."
    exit 1
  fi
  check_result "$?" "Remove /brokers/ids/0."
fi

umask 027
log_info "export SASL Kafka config"
export KAFKA_OPTS="-Djava.security.auth.login.config=${KAFKA_PATH}/config/kafka_server_jaas.conf"
log_info "export JMX options"
export KAFKA_JMX_OPTS="-Dcom.sun.management.jmxremote.host=127.0.0.1"
log_info "Starting kafka server."
nohup ${KAFKA_PATH}/bin/kafka-server-start.sh ${KAFKA_CFG_IN_NAS} 1>/dev/null 2>&1 &

if [[ -f "/opt/third_data/kafka/kafka_state_flag.txt" ]]; then
  rm -rf /opt/third_data/kafka/kafka_state_flag.txt
  check_result "$?" "rm -rf /opt/third_data/kafka/kafka_state_flag.txt"
fi

while true; do
  if [[ "${CLUSTER}" != "TRUE" ]]; then
    if [[ -f "/opt/third_data/kafka/kafka_state_flag.txt" ]]; then
      log_error "The service will be restarted. Kafka will recover by itself."
      exit 1
    fi
  fi

  ps -ef | grep "supervise_kafka_partition_state_log" | grep -v "grep"
  if [ $? -ne 0 ]; then
    log_info "start to check if kafka log contains error log"
    python3 supervise_kafka_partition_state_log.py &
    if [ $? != 0 ]; then
      log_error "execute supervise_kafka_partition_state_log.py failed."
      exit 1
    fi
  fi
  sleep 60
done
