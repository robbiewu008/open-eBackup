#!/bin/bash

NODE_NAME=${NODE_NAME}
DATA_PATH="/opt/third_data/zookeeper/data"
ZK_DATA_NAS_PATH="/opt/third_data/zookeeper"
LOG_PATH="/opt/OceanProtect/logs/${NODE_NAME}/infrastructure/zookeeper"
INSTALL_LOG_PATH=${LOG_PATH}/install_zookeeper.log
ZOOKEEPER_PATH="/usr/local/zookeeper/zookeeper-VERSION"
PRE_DATA_PATH="/usr/local/zookeeper/data"
PRE_LOG_PATH="${ZOOKEEPER_PATH}/logs"

# 挂载data目录
if [ ! -d $LOG_PATH ];then
    mkdir -p $LOG_PATH
fi
touch ${LOG_PATH}/install_zookeeper.log

chmod 750 $LOG_PATH

# 清理残留nfs文件
find ${LOG_PATH} -type f -name ".nfs*" -delete
check_result "$?" "${LINENO} find ${LOG_PATH} -type f -name \".nfs*\" -delete"

chmod 640 ${LOG_PATH}/install_zookeeper.log

function log_info()
{
    echo "$(date +"[%Y-%m-%d %H:%M:%S,%N]")[INFO][INSTALL_ZOOKEEPER: $1][$0][${BASH_LINENO}]"
    echo "$(date +"[%Y-%m-%d %H:%M:%S,%N]")[INFO][INSTALL_ZOOKEEPER: $1][$0][${BASH_LINENO}]" >> "${INSTALL_LOG_PATH}"
}

function log_error()
{
    echo "$(date +"[%Y-%m-%d %H:%M:%S,%N]")[ERROR][INSTALL_ZOOKEEPER: $1][$0][${BASH_LINENO}]"
    echo "$(date +"[%Y-%m-%d %H:%M:%S,%N]")[ERROR][INSTALL_ZOOKEEPER: $1][$0][${BASH_LINENO}]" >> "${INSTALL_LOG_PATH}"
}

function check_result()
{
    if [ "$1" != "0" ];then
        log_error "Exec cmd:$2 failed."
    else
        log_info "Exec cmd:$2 success."
    fi
}
mount_resul=$(sudo /opt/mount_oper.sh mount_bind ${LOG_PATH} ${PRE_LOG_PATH})
log_info "The result of mount --bind ${LOG_PATH} ${PRE_LOG_PATH} is ${mount_result}"
# 挂载data目录
log_info "Start to mount the data directory."
if [ ! -d $DATA_PATH ];then
    mkdir -p $DATA_PATH
    check_result "$?" "mkdir -p $DATA_PATH"
fi

chmod 750 $DATA_PATH
check_result "$?" "chmod 750 $DATA_PATH"
mount_result=$(sudo /opt/mount_oper.sh mount_bind ${DATA_PATH} ${PRE_DATA_PATH})
log_info "The result of mount --bind ${DATA_PATH} ${PRE_DATA_PATH} is ${mount_result}"
log_info "Mount the data directory ends."

# save conf to /opt/third_data/zookeeper/conf (NAS)
if [ ! -d $ZK_DATA_NAS_PATH/conf ];then
  mkdir $ZK_DATA_NAS_PATH/conf
fi
chmod 750 "${ZK_DATA_NAS_PATH}/conf"
ZOO_CFG_IN_NAS=$ZK_DATA_NAS_PATH/conf/zoo.cfg
if [ -f ${ZOO_CFG_IN_NAS} ];then
    rm -rf ${ZOO_CFG_IN_NAS}
fi
cp ${ZOOKEEPER_PATH}/conf/zoo_sample.cfg ${ZOO_CFG_IN_NAS}

LISTEN_ADDRESS=${POD_IP}

sed -i '$a\minSessionTimeout=150000' ${ZOO_CFG_IN_NAS}
check_result "$?" "$a\minSessionTimeout=150000' ${ZOO_CFG_IN_NAS}"

sed -i '$a\maxSessionTimeout=300000' ${ZOO_CFG_IN_NAS}
check_result "$?" "$a\maxSessionTimeout=300000' ${ZOO_CFG_IN_NAS}"

sed -i "/^dataDir=.*/c\dataDir=${PRE_DATA_PATH}" ${ZOO_CFG_IN_NAS}
check_result "$?" "sed dataDir for ${ZOOKEEPER_PATH}/conf/zoo.cfg"

sed -i "/^dataDir=.*/a\clientPortAddress=${LISTEN_ADDRESS}" ${ZOO_CFG_IN_NAS}
check_result "$?" "sed clientPortAddress for ${ZOOKEEPER_PATH}/conf/zoo.cfg"

# snapRetainCount和purgeInterval版本规范要求1024个，24小时自动清理一次
sed -i "/^#autopurge.snapRetainCount=.*/c\autopurge.snapRetainCount=${ZK_AUTOPURGE_SNAPRETAINCOUNT}" ${ZOO_CFG_IN_NAS}
check_result "$?" "sed autopurge.snapRetainCount for ${ZOOKEEPER_PATH}/conf/zoo.cfg"
sed -i "/^#autopurge.purgeInterval=.*/c\autopurge.purgeInterval=24" ${ZOO_CFG_IN_NAS}
check_result "$?" "sed autopurge.purgeInterval for ${ZOOKEEPER_PATH}/conf/zoo.cfg"

# 纯软集群配置
config_map="/opt/multicluster-conf"
cluster=$(cat "${config_map}/CLUSTER")

if [[ "${cluster}" == "true" ]]; then
  # 查询基础设施所在三个节点ip
  master_ip=$(cat "${config_map}/MASTER_IP")
  standby_ip=$(cat "${config_map}/STANDBY_IP")
  slave_ip=$(cat "${config_map}/SLAVE_IP")

  if [[ -n "${master_ip}" ]] && [[ -n "${standby_ip}" ]] && [[ -n "${slave_ip}" ]];then

    echo "server.1=${master_ip}:2888:3888" >> ${ZOO_CFG_IN_NAS}
    echo "server.2=${standby_ip}:2888:3888" >> ${ZOO_CFG_IN_NAS}
    echo "server.3=${slave_ip}:2888:3888" >> ${ZOO_CFG_IN_NAS}

    if [[ ${NODE_NAME} == ${MASTER_NODE} ]];then
      touch ${DATA_PATH}/myid
      echo "1" >> ${DATA_PATH}/myid
    elif [[ ${NODE_NAME} == ${STANDBY_NODE} ]];then
      touch ${DATA_PATH}/myid
      echo "2" >> ${DATA_PATH}/myid
    else
      touch ${DATA_PATH}/myid
      echo "3" >> ${DATA_PATH}/myid
    fi

    log_info "Starting zookeeper in cluster mode with node ${LISTEN_ADDRESS}."
  else
    log_error "Starting zookeeper in cluster mode with node ${LISTEN_ADDRESS} failed."
    exit 1
  fi
fi

mount_result=$(sudo /opt/mount_oper.sh mount_bind ${ZOO_CFG_IN_NAS} ${ZOOKEEPER_PATH}/conf/zoo.cfg)
log_info "The result of mount --bind ${ZOO_CFG_IN_NAS} ${ZOOKEEPER_PATH}/conf/zoo.cfg is ${mount_result}"

chmod 550 ${ZOO_CFG_IN_NAS}
check_result "$?" "chmod 550 ${ZOO_CFG_IN_NAS}"

ZK_ENV_IN_NAS=${ZK_DATA_NAS_PATH}/conf/zkEnv.sh
if [ -f ${ZK_ENV_IN_NAS} ];then
  rm -rf ${ZK_ENV_IN_NAS}
  check_result "$?" "rm -rf ${ZK_ENV_IN_NAS}"
fi
cp ${ZOOKEEPER_PATH}/bin/zkEnv.sh ${ZK_ENV_IN_NAS}
check_result "$?" "cp ${ZOOKEEPER_PATH}/bin/zkEnv.sh ${ZK_ENV_IN_NAS}"

sed -i "/ZOO_LOG4J_PROP=.*/c\    ZOO_LOG4J_PROP=\"INFO,RFAAUDIT\"" ${ZK_ENV_IN_NAS}
check_result "$?" "sed -i "/ZOO_LOG4J_PROP=.*/c\    ZOO_LOG4J_PROP=\"INFO,RFAAUDIT\"" ${ZK_ENV_IN_NAS}"

sed -i 's%ZK_SERVER_HEAP=.*$%ZK_SERVER_HEAP="${ZK_SERVER_HEAP:-'"$ZK_SERVER_HEAP"'}"%g' ${ZOO_CFG_IN_NAS}
mount_result==$(sudo /opt/mount_oper.sh mount_bind ${ZK_ENV_IN_NAS} ${ZOOKEEPER_PATH}/bin/zkEnv.sh)
log_info "The result of sudo /opt/mount_oper.sh mount_bind ${ZK_ENV_IN_NAS} ${ZOOKEEPER_PATH}/bin/zkEnv.sh is ${mount_result}"

cd ${ZOOKEEPER_PATH}

log_info "Starting umask."
umask 027
log_info "umask ends."

log_info "export JMX options"
export JMXDISABLE=true
log_info "Starting zkServer."
${ZOOKEEPER_PATH}/bin/zkServer.sh start-foreground