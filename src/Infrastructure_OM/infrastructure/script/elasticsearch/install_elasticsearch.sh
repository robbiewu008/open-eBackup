#!/bin/bash

NODE_NAME=$NODE_NAME
DATA_PATH="/opt/third_data/elasticsearch/data"
ES_NAS_PATH="/opt/third_data/elasticsearch"
LOG_PATH="/opt/OceanProtect/logs/${NODE_NAME}/infrastructure/elasticsearch"
INSTALL_LOG_PATH=${LOG_PATH}/install_elasticsearch.log
ELAS_PATH="/usr/local/elasticsearch/elasticsearch-VERSION"
PRE_DATA_PATH="${ELAS_PATH}/data"
PRE_LOG_PATH="${ELAS_PATH}/logs"
rootCAFile="/var/run/secrets/kubernetes.io/serviceaccount/ca.crt"
tokenFile=$(cat /var/run/secrets/kubernetes.io/serviceaccount/token)
CURRENT_DIR="$(dirname $(readlink -f "$0"))"
ES_CLUSTER_PORT=9300

# 挂载log目录
if [ ! -d $LOG_PATH ];then
    mkdir -p $LOG_PATH
fi
touch ${LOG_PATH}/install_elasticsearch.log
chmod 750 $LOG_PATH
chgrp nobody $LOG_PATH
chown nobody:nobody $LOG_PATH -R

chmod 640 ${LOG_PATH}/install_elasticsearch.log

function log_info()
{
    echo "$(date +"[%Y-%m-%d %H:%M:%S,%N]")[INFO][INSTALL_ELASTICSEARCH: $1][$0][${BASH_LINENO}]"
    echo "$(date +"[%Y-%m-%d %H:%M:%S,%N]")[INFO][INSTALL_ELASTICSEARCH: $1][$0][${BASH_LINENO}]" >> "${INSTALL_LOG_PATH}"
}

function log_error()
{
    echo "$(date +"[%Y-%m-%d %H:%M:%S,%N]")[ERROR][INSTALL_ELASTICSEARCH: $1][$0][${BASH_LINENO}]"
    echo "$(date +"[%Y-%m-%d %H:%M:%S,%N]")[ERROR][INSTALL_ELASTICSEARCH: $1][$0][${BASH_LINENO}]" >> "${INSTALL_LOG_PATH}"
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
chmod 750 $DATA_PATH -R
check_result "$?" "chmod 750 $DATA_PATH -R"

chgrp nobody $DATA_PATH
check_result "$?" "chgrp nobody $DATA_PATH"

chown nobody:nobody $DATA_PATH -R
check_result "$?" "chown nobody:nobody $DATA_PATH -R"

mount_result=$(sudo /opt/mount_oper.sh mount_bind ${DATA_PATH} ${PRE_DATA_PATH})
log_info "The result of sudo mount --bind ${DATA_PATH} ${PRE_DATA_PATH} is ${mount_result}"

log_info "Mount the data directory ends."
# move to nas
if [ ! -d ${ES_NAS_PATH}/config ];then
  mkdir ${ES_NAS_PATH}/config
fi
chmod 750 "${ES_NAS_PATH}/config"

rm -rf ${ES_NAS_PATH}/config/*
cp -ar ${ELAS_PATH}/config/* ${ES_NAS_PATH}/config/

LISTEN_ADDRESS="127.0.0.1"

ES_CFG_IN_NAS=${ES_NAS_PATH}/config/elasticsearch.yml
sed -i "s%#network.host.*$%network.host: ${LISTEN_ADDRESS}%g" ${ES_CFG_IN_NAS}
check_result "$?" "sed network.host for ${ELAS_PATH}/config/elasticsearch.yml"

# 配置 publish_host
infra_inner_commnunicate_ip_file="/opt/network-conf/infrastructure_internal_communicate_net_plane"
if [ -f $infra_inner_commnunicate_ip_file ];then
    inf_ip=$(grep -o '"ip"\s*:\s*"[^"]*"' $infra_inner_commnunicate_ip_file | sed 's/"ip"\s*:\s*"\([^"]*\)"/\1/')
    echo "network.bind_host:  ${LISTEN_ADDRESS}" >> ${ES_CFG_IN_NAS}
    echo "network.publish_host: $inf_ip" >> ${ES_CFG_IN_NAS}
fi

config_map="/opt/multicluster-conf"
es_cluster=$(cat "${config_map}/ES_CLUSTER")

if [[ "${CLUSTER}" == "TRUE" ]];then
  master_node=$(cat "${config_map}/MASTER")
  standby_node=$(cat "${config_map}/STANDBY")

  # 查询基础设施所在三个节点ip
  master_ip=$(cat "${config_map}/MASTER_IP")
  standby_ip=$(cat "${config_map}/STANDBY_IP")
  slave_ip=$(cat "${config_map}/SLAVE_IP")

  if [[ -n "${master_ip}" ]] && [[ -n "${standby_ip}" ]] && [[ -n "${slave_ip}" ]];then
    # 软硬解耦集群形态参数
    echo 'cluster.name: databackup' >>${ES_CFG_IN_NAS}
    echo "node.name: ${NODE_NAME}" >>${ES_CFG_IN_NAS}
    echo 'node.master: true' >>${ES_CFG_IN_NAS}
    echo 'node.data: true' >>${ES_CFG_IN_NAS}
    echo 'http.port: 9200' >>${ES_CFG_IN_NAS}
    echo 'transport.port: 9300' >>${ES_CFG_IN_NAS}
    echo "transport.host: $POD_IP" >>${ES_CFG_IN_NAS}

    # 用于集群发现其余节点
    if [[ ${NODE_NAME} == ${master_node} ]];then
      echo "discovery.seed_hosts: ['${standby_ip}:${ES_CLUSTER_PORT}', '${slave_ip}:${ES_CLUSTER_PORT}']" >> ${ES_CFG_IN_NAS}
    elif [[ ${NODE_NAME} == ${standby_node} ]];then
      echo "discovery.seed_hosts: ['${master_ip}:${ES_CLUSTER_PORT}', '${slave_ip}:${ES_CLUSTER_PORT}']" >> ${ES_CFG_IN_NAS}
    else
      echo "discovery.seed_hosts: ['${master_ip}:${ES_CLUSTER_PORT}', '${standby_ip}:${ES_CLUSTER_PORT}']" >> ${ES_CFG_IN_NAS}
      # 组建集群需要清除单节点数据，防止脑裂
      if [[ "${es_cluster}" == "false" ]]; then
        if [[ ! -d ${ES_NAS_PATH}/backup ]];then
          mkdir ${ES_NAS_PATH}/backup
        fi
        cp -rf ${DATA_PATH}/* ${ES_NAS_PATH}/backup
        rm -rf ${DATA_PATH}/*
      fi
    fi

    # 初始主节点为第一个启动的节点
    echo "cluster.initial_master_nodes: ${master_node}" >> ${ES_CFG_IN_NAS}

    # 集群超时和重试机制
    echo 'discovery.zen.fd.ping_timeout: 10s' >> ${ES_CFG_IN_NAS}
    echo 'discovery.zen.fd.ping_retries: 3' >> ${ES_CFG_IN_NAS}
    echo 'discovery.zen.join_timeout: 120s' >> ${ES_CFG_IN_NAS}

    log_info "Starting es cluster in node $POD_IP."
  else
    log_error "Starting es cluster in node $POD_IP failed."
    exit 1
  fi
else
  echo 'node.master: "true"' >> ${ES_CFG_IN_NAS}
  echo 'discovery.type: single-node' >> ${ES_CFG_IN_NAS}
fi

# move to nas
JVM_OPT_IN_NAS=${ES_NAS_PATH}/config/jvm.options
if [ -f ${JVM_OPT_IN_NAS} ];then
  rm -rf ${JVM_OPT_IN_NAS}
  check_result "$?" "rm -rf ${JVM_OPT_IN_NAS}"
fi
cp ${ELAS_PATH}/config/jvm.options ${JVM_OPT_IN_NAS}
# es会在config目录下写入临时文件，所以将config目录挂载至nas
mount_resul=$(sudo /opt/mount_oper.sh mount_bind ${ES_NAS_PATH}/config ${ELAS_PATH}/config)
log_info "The result of sudo mount --bind ${ES_NAS_PATH}/config ${ELAS_PATH}/config is ${mount_result}"
log_info "Starting umask."

sed -i "s/^-Xms.*/-Xms${ES_JVM_XMS}/g" ${JVM_OPT_IN_NAS}
check_result "$?" "set Xms:${ES_JVM_XMS} for ${JVM_OPT_IN_NAS}"
sed -i "s/^-Xmx.*/-Xmx${ES_JVM_XMX}/g" ${JVM_OPT_IN_NAS}
check_result "$?" "set Xmx:${ES_JVM_XMX} for ${JVM_OPT_IN_NAS}"
sed -i "s/^8:-XX:NumberOfGCLogFiles.*/8:-XX:NumberOfGCLogFiles=0/g" ${JVM_OPT_IN_NAS}
check_result "$?" "set NumberOfGCLogFiles=0 for ${JVM_OPT_IN_NAS}"

umask 027
log_info "umask ends."

log_info "Starting elasticsearch."
${ELAS_PATH}/bin/elasticsearch