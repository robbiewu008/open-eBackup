#!/bin/sh
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

PROTECT_MANAGER_PATH="/opt/OceanProtect/protectmanager/"
PROTECT_MANAGER_LOG_NODE_PATH="/opt/OceanProtect/logs/${NODE_NAME}/"
PROTECT_MANAGER_PM_LOG_NODE_PATH="${PROTECT_MANAGER_LOG_NODE_PATH}protectmanager/"
PROTECT_MANAGER_KMC_PATH="${PROTECT_MANAGER_PATH}kmc/"
PROTECT_MANAGER_CERT_PATH="${PROTECT_MANAGER_PATH}cert/"
PROTECT_MANAGER_CRL_PATH="${PROTECT_MANAGER_CERT_PATH}crl/"
PROTECT_MANAGER_CERT_CA_PATH="${PROTECT_MANAGER_CERT_PATH}CA/"
PROTECT_MANAGER_CERT_CA_CERTS_PATH="${PROTECT_MANAGER_CERT_CA_PATH}certs/"
PROTECT_MANAGER_CERT_INTERNAL_PATH="${PROTECT_MANAGER_CERT_PATH}internal/"
PROTECT_MANAGER_CERT_INTERNAL_OpenAPI_PATH="${PROTECT_MANAGER_CERT_INTERNAL_PATH}OpenAPI/"
PROTECT_MANAGER_CERT_INTERNAL_Agent_PATH="${PROTECT_MANAGER_CERT_INTERNAL_PATH}ProtectAgent/"
PROTECT_MANAGER_CERT_INTERNAL_Agent_Ca_PATH="${PROTECT_MANAGER_CERT_INTERNAL_Agent_PATH}ca/"


if [[ $(python -c 'import sys; print(sys.version_info[:2])') != "(3, 9)" ]]; then
  echo "python version is not 3.9"
  exit 1
fi

if [[ ! -d "${PROTECT_MANAGER_PATH}" ]]; then
  mkdir "${PROTECT_MANAGER_PATH}"
  chmod 750 "${PROTECT_MANAGER_PATH}"
fi

if [[ ! -d "${PROTECT_MANAGER_LOG_NODE_PATH}" ]]; then
  mkdir "${PROTECT_MANAGER_LOG_NODE_PATH}"
fi

if [[ ! -d "${PROTECT_MANAGER_PM_LOG_NODE_PATH}" ]]; then
  mkdir "${PROTECT_MANAGER_PM_LOG_NODE_PATH}"
fi

sudo /script/mount_oper.sh mount_bind "${PROTECT_MANAGER_PM_LOG_NODE_PATH}" "/context/src/logs"

find "${PROTECT_MANAGER_PM_LOG_NODE_PATH}PM_Data_Protection_Service" -type d | xargs chmod 750
find "${PROTECT_MANAGER_PM_LOG_NODE_PATH}PM_Data_Protection_Service" -type f | xargs chmod 640

find "${PROTECT_MANAGER_PATH}" -type d | grep -v "/opt/OceanProtect/protectmanager/kmc" | grep -v -w "/opt/OceanProtect/protectmanager/" | xargs chmod 750
find "${PROTECT_MANAGER_PATH}" -type f | grep -v "/opt/OceanProtect/protectmanager/kmc/master.ks" | grep -v "/opt/OceanProtect/protectmanager/kmc/backup.ks" | xargs chmod 640


# 避免重启后cert权限被修改
if [[ ! -d "${PROTECT_MANAGER_CERT_CA_PATH}" ]]; then
  mkdir "${PROTECT_MANAGER_CERT_CA_PATH}"
fi

if [[ ! -d "${PROTECT_MANAGER_CERT_CA_CERTS_PATH}" ]]; then
  mkdir "${PROTECT_MANAGER_CERT_CA_CERTS_PATH}"
fi

if [[ ! -d "${PROTECT_MANAGER_CERT_INTERNAL_PATH}" ]]; then
  mkdir "${PROTECT_MANAGER_CERT_INTERNAL_PATH}"
fi

if [[ ! -d "${PROTECT_MANAGER_CERT_INTERNAL_OpenAPI_PATH}" ]]; then
  mkdir "${PROTECT_MANAGER_CERT_INTERNAL_OpenAPI_PATH}"
fi

if [[ ! -d "${PROTECT_MANAGER_CERT_INTERNAL_Agent_Ca_PATH}" ]]; then
  mkdir -p "${PROTECT_MANAGER_CERT_INTERNAL_Agent_Ca_PATH}"
fi

if [[ ! -d "${PROTECT_MANAGER_CRL_PATH}" ]]; then
  mkdir "${PROTECT_MANAGER_CRL_PATH}"
fi

PM_KEY_STORE_FILE_PATH="${PROTECT_MANAGER_CERT_PATH}pm.store.p12"
if [[ -e "${PM_KEY_STORE_FILE_PATH}" ]]; then
  chmod 640 "${PM_KEY_STORE_FILE_PATH}"
fi

PM_KEY_STORE_PWD_PATH="${PROTECT_MANAGER_CERT_PATH}pm.store.p12.cnf"
if [[ -e "${PM_KEY_STORE_PWD_PATH}" ]]; then
  chmod 640 "${PM_KEY_STORE_PWD_PATH}"
fi

chmod 750 "${PROTECT_MANAGER_PATH}"
chmod 750 "${PROTECT_MANAGER_KMC_PATH}"
chmod 750 "${PROTECT_MANAGER_CERT_PATH}"
chmod 750 "${PROTECT_MANAGER_CERT_CA_PATH}"
chmod 750 "${PROTECT_MANAGER_CRL_PATH}"
chmod 750 "${PROTECT_MANAGER_CERT_CA_CERTS_PATH}"
chmod 750 "${PROTECT_MANAGER_CERT_INTERNAL_PATH}"
chmod 750 "${PROTECT_MANAGER_CERT_INTERNAL_OpenAPI_PATH}"
chmod -R 750 "${PROTECT_MANAGER_CERT_INTERNAL_Agent_PATH}"
chmod 750 "${PROTECT_MANAGER_CERT_INTERNAL_Agent_Ca_PATH}"

isHas=`ifconfig |grep "cbri1601"`
if [ "${DEPLOY_TYPE}" = "d5" ]; then
    # 安全一体机d5场景使用eth1网桥
    ip=${POD_IP}
    if [[ -f "${PROTECT_MANAGER_PATH}/timezone" ]]; then
          rm -rf "${PROTECT_MANAGER_PATH}/timezone"
    fi
    if [[ -f "/etc/timezone" ]]; then
          rm -rf "/etc/timezone"
    fi
elif [[ ! -z "${isHas}" ]]; then
    ip=`ifconfig cbri1601 | grep "inet " | awk '{print $2}'` &> /dev/null
else
    ip="0.0.0.0"
fi
export SERVICE_HOST=$ip

if [ "$DEPLOY_TYPE" = "cloudbackup" ] || [ "$DEPLOY_TYPE" = "d3" ]; then
  export DATABASE_POOL_SIZE=10
else
  export DATABASE_POOL_SIZE=10
fi

python3 -m app