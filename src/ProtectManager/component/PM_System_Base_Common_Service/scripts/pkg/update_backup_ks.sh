#!/bin/bash

name="kmc-store-conf"
namespace="dpa"
read -s BACKUPKSINFO
tokenFile="/var/run/secrets/kubernetes.io/serviceaccount/token"
KUBE_CACRT_PATH="/var/run/secrets/kubernetes.io/serviceaccount/ca.crt"
host=$KUBERNETES_SERVICE_HOST
port=$KUBERNETES_SERVICE_PORT
Host="https://"$host:$port
TOKEN=$(<$tokenFile)

echo "modify ${name}"
if [ "${BACKUPKSINFO}X" == "X" ]; then
  echo "modify ${name} failed, parameter is no exit"
  exit 1
fi
result=$(curl --cacert $KUBE_CACRT_PATH -X PATCH -H "Authorization: Bearer $TOKEN" $Host/api/v1/namespaces/${namespace}/configmaps/${name} -H "Content-Type: application/strategic-merge-patch+json" --data "{\"binaryData\":{\"backup.ks\": \"${BACKUPKSINFO}\"}}")
check_result=$(echo ${result} |grep "Failure")
if [ -n "${check_result}" ]; then
    time=$(date "+%Y-%m-%d %H:%M:%S")}
    echo "[${time}] get ${name} failed"
    exit 1
fi
echo "modify ${name} success"