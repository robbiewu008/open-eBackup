#!/bin/bash
# This file is a part of the open-eBackup project.
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file, You can obtain one at
# http://mozilla.org/MPL/2.0/.
#
# Copyright (c) [2024] Huawei Technologies Co.,Ltd.
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.

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
result=$(LD_PRELOAD=/usr/lib64/libSecurityStarter.so curl --cacert $KUBE_CACRT_PATH -X PATCH -H "Authorization: Bearer $TOKEN" $Host/api/v1/namespaces/${namespace}/configmaps/${name} -H "Content-Type: application/strategic-merge-patch+json" --data "{\"binaryData\":{\"backup.ks\": \"${BACKUPKSINFO}\"}}" index@6)
check_result=$(echo ${result} |grep "Failure")
if [ -n "${check_result}" ]; then
    time=$(date "+%Y-%m-%d %H:%M:%S")}
    echo "[${time}] get ${name} failed"
    exit 1
fi
echo "modify ${name} success"