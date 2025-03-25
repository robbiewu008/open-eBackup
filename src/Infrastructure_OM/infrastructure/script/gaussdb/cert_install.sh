#!/bin/bash

source ./log.sh
INTER_CERT_PATH=/opt/OceanProtect/infrastructure/cert/internal
GAUSSDB_PATH=/usr/local/gaussdb
DATA_PATH=${GAUSSDB_PATH}/data
KEY_STORE_DIR=/opt/OceanProtect/protectmanager/kmc
DECRYP_TOOL=/usr/bin/restclient

# cp internal.crt.pem
if [ -f "${DATA_PATH}/internal.crt.pem" ]; then
  rm ${DATA_PATH}/internal.crt.pem
fi
cp ${INTER_CERT_PATH}/internal.crt.pem ${DATA_PATH}
check_result "$?" "${LINENO} cp internal.crt.pem"

# cp internal.pem
if [ -f "${DATA_PATH}/internal.pem" ]; then
  rm ${DATA_PATH}/internal.pem
fi
cp ${INTER_CERT_PATH}/internal.pem ${DATA_PATH}
check_result "$?" "${LINENO} cp internal.pem"

# 删除密文文件和加密因子，适配证书替换
if [ -f "${DATA_PATH}/server.key.cipher" ]; then
  rm ${DATA_PATH}/server.key.cipher
fi
if [ -f "${DATA_PATH}/server.key.rand" ]; then
  rm ${DATA_PATH}/server.key.rand
fi

# 修改属主和权限
chown GaussDB:dbgrp ${DATA_PATH}/internal.crt.pem
check_result "$?" "${LINENO} chown GaussDB:dbgrp internal.crt.pem"
chown GaussDB:dbgrp ${DATA_PATH}/internal.pem
check_result "$?" "${LINENO} chown GaussDB:dbgrp internal.pem"
chmod 600 ${DATA_PATH}/internal.crt.pem
check_result "$?" "${LINENO} chmod 600 internal.crt.pem"
chmod 600 ${DATA_PATH}/internal.pem
check_result "$?" "${LINENO} chmod 600 internal.pem"

# 修改pg_hba.conf
result=$(cat /usr/local/gaussdb/data/pg_hba.conf | grep "^hostssl")
if [ -z "${result}" ]; then
  sed -i "s/^host/hostssl/g" /usr/local/gaussdb/data/pg_hba.conf
fi

log_info "Starting configure the certificate."

export GAUSSDATA=/usr/local/gaussdb/data
export PATH=$PATH:/usr/local/gaussdb/app/bin

# 开启SSL认证
gs_guc set -D ${GAUSSDATA} -c ssl=on

# 加密, 工具-g参数会调用 set_kmc_passwoed.sh 脚本
${DECRYP_TOOL} ${KEY_STORE_DIR}/master.ks /kmc_conf/..data/backup.ks ${INTER_CERT_PATH}/internal_cert -g

# 配置为证书参数
gs_guc set -D ${GAUSSDATA} -c "ssl_cert_file='internal.crt.pem'"
gs_guc set -D ${GAUSSDATA} -c "ssl_key_file='internal.pem'"
gs_guc set -D ${GAUSSDATA} -c "ssl_ciphers='ALL'"

log_info "end to configure the certificate."
