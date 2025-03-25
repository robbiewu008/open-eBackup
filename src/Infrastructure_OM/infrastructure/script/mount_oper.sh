#!/bin/bash

whitelist=(
  # es
  "^/opt/OceanProtect/logs/${NODE_NAME}/infrastructure/elasticsearch$"
  "^/usr/local/elasticsearch/elasticsearch.*/logs$"
  "^/opt/third_data/elasticsearch/data$"
  "^/usr/local/elasticsearch/elasticsearch.*/data$"
  "^/opt/third_data/elasticsearch/config$"
  "^/usr/local/elasticsearch/elasticsearch.*/config$"

  # kafka
  "^/opt/OceanProtect/logs/${NODE_NAME}/infrastructure/kafka$"
  "^/usr/local/kafka/kafka.*/logs$"
  "^/opt/third_data/kafka/data$"
  "^/usr/local/kafka/kafka.*/data$"
  "^/opt/third_data/kafka/config/kafka_server_jaas.conf$"
  "^/usr/local/kafka/kafka.*/config/kafka_server_jaas.conf$"
  "^/opt/third_data/kafka/config/zookeeper.properties$"
  "^/usr/local/kafka/kafka.*/config/zookeeper.properties$"
  "^/usr/local/kafka/kafka.*/bin/kafka-server-start.sh$"
  "^/opt/third_data/kafka/config/kafka-server-start.sh$"

  # redis
  "^/opt/OceanProtect/logs/${NODE_NAME}/infrastructure/redis$"
  "^/usr/local/redis/redis.*/logs$"
  "^/opt/third_data/redis/data$"
  "^/usr/local/redis/redis.*/data$"

  # zk
  "^/opt/OceanProtect/logs/${NODE_NAME}/infrastructure/zookeeper$"
  "^/usr/local/zookeeper/zookeeper.*/logs$"
  "^/opt/third_data/zookeeper/data$"
  "^/usr/local/zookeeper/data$"
  "^/opt/third_data/zookeeper/conf/zoo.cfg$"
  "^/usr/local/zookeeper/zookeeper.*/conf/zoo.cfg$"
  "^/opt/third_data/zookeeper/conf/zkEnv.sh$"
  "^/usr/local/zookeeper/zookeeper.*/bin/zkEnv.sh$"
)

check_whitelist() {
  local name=$1
  for wdir in "${whitelist[@]}"; do
    if [[ $name =~ $wdir ]]; then
      return 0
    fi
  done
  return 1
}

function check_path_common() {
  local path="$1"
  if [ -z "${path}" ]; then
    echo "No path:${path} specified."
    return 1
  fi

  if [ -L "${path}" ]; then
    echo "Symbolic link path:${path} is not allowed."
    return 1
  fi

  filepat='[|;&$><`\!]+'
  if [[ "${path}" =~ "${filepat}" ]]; then
    echo "The path:${path} cannot contain special characters:${filepat}."
    return 1
  fi

  if [[ "${path}" =~ '..' ]]; then
    echo "The path:${path} cannot contain special characters(..)."
    return 1
  fi

  return 0
}

mount_bind() {
  echo "Begin mount."
  local mount_src="$1"
  local mount_target="$2"

  check_path_common "${mount_src}"
  if [ $? -ne 0 ]; then
    echo "ERROR: check_path_common mount_src:${mount_src} failed!"
    return 1
  fi

  check_path_common "${mount_target}"
  if [ $? -ne 0 ]; then
    echo "ERROR: check_path_common mount_src:${mount_src} failed!"
    return 1
  fi

  # white list
  check_whitelist "${mount_src}"
  if [ $? -ne 0 ]; then
    echo "ERROR: mount_src:${mount_src} not in whitelist!"
    return 1
  fi

  check_whitelist "${mount_target}"
  if [ $? -ne 0 ]; then
    echo "ERROR: mount_target:${mount_target} not in whitelist!"
    return 1
  fi

  echo "Mount start. mount bind mount_src:"${mount_src}" to mount_target:"${mount_target}"."

  if [[ ${mount_src} == *kafka-server-start.sh ]]; then
    # kafka-server-start.sh需要执行权限，不能添加 -o noexec -o nosuid参数
    mount --bind $mount_src $mount_target
  else
    mount --bind -o noexec -o nosuid $mount_src $mount_target
  fi
  if [ $? -ne 0 ]; then
    echo "ERROR: mount --bind mount_src:${mount_src} mount_target:${mount_target} failed!"
    return 1
  fi
}

main() {
  #eg: mount_oper.sh mount_bind /nas/path/abc /usr/local/ef
  if [ $(whoami) != "root" ]; then
    echo "Error:Current operation must perform by root account."
    return 1
  fi
  local L_OPERATION="$1"
  case "${L_OPERATION}" in
  mount_bind)
    #params: mount_point, mount_src
    mount_bind "${2}" "${3}"
    return $?
    ;;
  *)
    echo "Error:Invalid parameter."
    return 1
    ;;
  esac
  return 0
}

main $@
exit $?
