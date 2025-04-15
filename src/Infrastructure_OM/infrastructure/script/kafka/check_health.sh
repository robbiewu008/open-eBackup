#!/bin/bash

########################################
#  This file is part of the open-eBackup project.
# Copyright (c) 2024 Huawei Technologies Co.,Ltd.
#
# open-eBackup is licensed under MPL v2.
# You can use this software according to the terms and conditions of the MPL v2.
# You may obtain a copy of MPL v2 at:
#
#          https://www.mozilla.org/en-US/MPL/2.0
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the MPL v2 for more details.
########################################

tokenFile=$(cat /var/run/secrets/kubernetes.io/serviceaccount/token)
rootCAFile="/var/run/secrets/kubernetes.io/serviceaccount/ca.crt"

INTERNAL_CERT="/opt/OceanProtect/infrastructure/cert/internal/internal.crt.pem"
INTERNAL="/opt/OceanProtect/infrastructure/cert/internal/internal.pem"
CA_CERT="/opt/OceanProtect/infrastructure/cert/internal/ca/ca.crt.pem"
URL='https://infrastructure.dpa.svc.cluster.local:8088/v1/infra/secret/info?nameSpace=dpa&secretName=common-secret'

NODE_NUMBER=$(echo "$NODE_NAME" | awk '{print substr($0,length,1)}')
key_pass=$(cat /opt/third_data/kafka/config/server.properties | grep ssl.truststore.password= | uniq | awk -F = '{print $2}')
kafka_pwd=$(curl -v --cert ${INTERNAL_CERT} --key ${INTERNAL} --cacert ${CA_CERT} ${URL} --pass "$key_pass" | grep kafka.password | awk -F ': ' '{print $2}')

if [ -z $kafka_pwd ]; then
  echo "Failed to get kafka_pwd"
  exit 0
fi

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

function check_application_and_port() {
  netstat -tunple | grep kafka_port >>/dev/null &&
    ps aux | grep kafka | grep -v grep >>/dev/null
}

function check_kafka_service() {
  generate_kafka_client_config

  if [[ "${CLUSTER}" == "TRUE" ]]; then
    check_kafka_cluster
  else
    /usr/local/kafka/kafka-*/bin/kafka-broker-api-versions.sh --bootstrap-server ${POD_IP}:9092 --command-config /opt/third_data/kafka/client.properties
  fi
}

function check_kafka_cluster() {
  # broker�Ƿ�ע�ᵽzk
  broker_list=$(/usr/local/kafka/kafka-*/bin/zookeeper-shell.sh ${POD_IP}:2181 ls /brokers/ids | awk 'END {print}')
  broker_list_cleaned=$(echo $broker_list | tr -d '[]')
  if ! echo "$broker_list_cleaned" | grep -q "\b$NODE_NUMBER\b"; then
    echo "Broker ID $NODE_NUMBER is NOT in the list."
    rm -rf /opt/third_data/kafka/data/meta.properties
    return 1
  fi

  # broker�Ƿ���������
  /usr/local/kafka/kafka-*/bin/kafka-broker-api-versions.sh --bootstrap-server ${POD_IP}:9092 --command-config /opt/third_data/kafka/client.properties
  if [ $? -ne 0 ]; then
    echo "Kafka broker error."
    return 1
  fi

  config_map="/opt/multicluster-conf"
  kafka_tag=$(cat "${config_map}/ZK_CLUSTER")
  broker_length=$(echo $broker_list_cleaned | awk -F, '{print NF}')
  if [[ "${kafka_tag}" == "false" ]] && [[ "${POD_NAME}" == "infrastructure-0" ]] && [[ "${broker_length}" == "3" ]]; then
    PAYLOAD="{\"data\":{\"ZK_CLUSTER\":\"true\"}}"
    update_k8s "${PAYLOAD}" "configmaps" "multicluster-conf"
    echo "Set up multi-cluster zk-kafka tag succeed."
    return 0
  fi
}

function main() {
  check_application_and_port
  check_kafka_service
  if [ $? != 0 ]; then
    rm -f "/opt/third_data/kafka/client.properties"
  fi
}

function generate_kafka_client_config() {
  kafka_client_config="/opt/third_data/kafka/client.properties"
  if [[ -f "$kafka_client_config" ]]; then
    if grep -qF "$kafka_pwd" "$kafka_client_config"; then
      echo "client.preperties already exist"
      return
    fi
  fi

  cat <<EOT >"$kafka_client_config"
sasl.jaas.config=org.apache.kafka.common.security.plain.PlainLoginModule required \
  username=kafka_usr \
  password=$kafka_pwd;
sasl.mechanism=PLAIN
security.protocol=SASL_SSL
ssl.truststore.location=/opt/OceanProtect/infrastructure/cert/internal/internal.ks
ssl.truststore.password=$key_pass
ssl.endpoint.identification.algorithm=
EOT
}

main
