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

PMTLSSecretKeyName=secret-pm-tls
PMMTLSSecretKeyName=secret-pm-mtls
PMHASSecretKeyName=secret-pm-ha
namespace=dpa

tokenFile="/var/run/secrets/kubernetes.io/serviceaccount/token"
rootCAFile="/var/run/secrets/kubernetes.io/serviceaccount/ca.crt"
host=$KUBERNETES_SERVICE_HOST
port=$KUBERNETES_SERVICE_PORT
Host="https://"$host:$port
TOKEN=$(<$tokenFile)

echo "[$(date "+%Y-%m-%d %H:%M:%S")][INFO] delete ${PMTLSSecretKeyName}"
tls_secrets=$(curl -k -X GET -H "Authorization: Bearer $TOKEN" $Host/api/v1/namespaces/${namespace}/secrets/${PMTLSSecretKeyName})
is_tls_exit=$(echo ${tls_secrets} | grep -w "${namespace}" | grep -w "${PMTLSSecretKeyName}")
if [ -n "${is_tls_exit}" ]; then
  result=$(curl -k -X DELETE -H "Authorization: Bearer $TOKEN" $Host/api/v1/namespaces/${namespace}/secrets/${PMTLSSecretKeyName})
  check_result=$(echo ${result} |grep "Failure")
  if [ -n "${check_result}" ]; then
      echo "[$(date "+%Y-%m-%d %H:%M:%S")][ERROR] delete ${PMTLSSecretKeyName} failed"
      exit 1
  fi
  echo "[$(date "+%Y-%m-%d %H:%M:%S")][INFO] delete ${PMTLSSecretKeyName} success"
else
  echo "[$(date "+%Y-%m-%d %H:%M:%S")][INFO] ${PMTLSSecretKeyName} does not exist"
fi

echo "[$(date "+%Y-%m-%d %H:%M:%S")][INFO] delete ${PMMTLSSecretKeyName}"
mtls_secrets=$(curl -k -X GET -H "Authorization: Bearer $TOKEN" $Host/api/v1/namespaces/${namespace}/secrets/${PMMTLSSecretKeyName})
is_mtls_exist=$(echo ${mtls_secrets} | grep -w "${namespace}" | grep -w "${PMMTLSSecretKeyName}")
if [ -n "${is_mtls_exist}" ]; then
  result=$(curl -k -X DELETE -H "Authorization: Bearer $TOKEN" $Host/api/v1/namespaces/${namespace}/secrets/${PMMTLSSecretKeyName})
  check_result=$(echo ${result} |grep "Failure")
  if [ -n "${check_result}" ]; then
      echo "[$(date "+%Y-%m-%d %H:%M:%S")][ERROR] delete ${PMMTLSSecretKeyName} failed"
      exit 1
  fi
  echo "[$(date "+%Y-%m-%d %H:%M:%S")][INFO] delete ${PMMTLSSecretKeyName} success"
else
  echo "[$(date "+%Y-%m-%d %H:%M:%S")][INFO] ${PMMTLSSecretKeyName} does not exist"
fi

echo "[$(date "+%Y-%m-%d %H:%M:%S")][INFO] delete ${PMHASSecretKeyName}"
ha_secrets=$(curl -k -X GET -H "Authorization: Bearer $TOKEN" $Host/api/v1/namespaces/${namespace}/secrets/${PMHASSecretKeyName})
is_ha_exist=$(echo ${ha_secrets} | grep -w "${namespace}" | grep -w "${PMHASSecretKeyName}")
if [ -n "${is_ha_exist}" ]; then
  result=$(curl -k -X DELETE -H "Authorization: Bearer $TOKEN" $Host/api/v1/namespaces/${namespace}/secrets/${PMHASSecretKeyName})
  check_result=$(echo ${result} |grep "Failure")
  if [ -n "${check_result}" ]; then
      echo "[$(date "+%Y-%m-%d %H:%M:%S")][ERROR] delete ${PMHASSecretKeyName} failed"
      exit 1
  fi
  echo "[$(date "+%Y-%m-%d %H:%M:%S")][INFO] delete ${PMHASSecretKeyName} success"
else
  echo "[$(date "+%Y-%m-%d %H:%M:%S")][INFO] ${PMHASSecretKeyName} does not exist"
fi
