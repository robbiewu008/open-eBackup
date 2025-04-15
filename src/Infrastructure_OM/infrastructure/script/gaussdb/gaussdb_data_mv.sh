#!/bin/bash

source ./log.sh
namespace=dpa
config_map=common-conf
cluster_config_map=cluster-conf
secret=common-secret
tokenFile="/var/run/secrets/kubernetes.io/serviceaccount/token"
rootCAFile="/var/run/secrets/kubernetes.io/serviceaccount/ca.crt"
host=$KUBERNETES_SERVICE_HOST
port=$KUBERNETES_SERVICE_PORT
Host="https://"$host:$port
TOKEN=$(<$tokenFile)
LOG_PATH="/opt/OceanProtect/logs/${NODE_NAME}/infrastructure/gaussdb"
GAUSSDB_PATH="/usr/local/gaussdb"
DATASYNC_PATH="/usr/local/gaussdb/GaussDB_T_1.9.0-DATASYNC/DataSync"
PVC="/opt/db_data"

function curl_cmd() {
  cmd_type=$1
  content_type=$2
  payload=$3
  url=$4
  curl --cacert ${rootCAFile} \
  -X "${cmd_type}" \
  -H "Content-Type: $content_type" \
  -H "Authorization: Bearer ${TOKEN}" \
  --data "${payload}" \
  "${url}"
}

config_info=$(LD_PRELOAD=/usr/lib64/libSecurityStarter.so curl --cacert ${rootCAFile} -X GET -H "Authorization: Bearer $TOKEN" -H "Content-Type: application/json" $Host/api/v1/namespaces/${namespace}/configmaps/${config_map} between@-H@-H)
config_tag=$(echo ${config_info} | python3 -c "import sys, json;print(json.load(sys.stdin)['data'].get('gaussdbUpgrade'))")
cluster_info=$(LD_PRELOAD=/usr/lib64/libSecurityStarter.so curl --cacert ${rootCAFile} -X GET -H "Authorization: Bearer $TOKEN" -H "Content-Type: application/json" $Host/api/v1/namespaces/${namespace}/configmaps/${cluster_config_map} between@-H@-H)
cluster_tag=$(echo ${cluster_info} | python3 -c "import sys, json;print(json.load(sys.stdin)['data'].get('CLUSTER_ROLE'))")
if [ "${config_tag}" != "true" ] && [ "${cluster_tag}" != "MEMBER" ]; then
  # 清除升级失败回滚之后的V5配置
  cd ${PVC}
  ls | grep GaussDB_V5
  res_v5=$(echo $?)
  if [ ${res_v5} == 0 ]; then
    rm -rf GaussDB_V5
  fi

  # 清除升级失败回滚之后的迁移数据
  ls | grep dataMigration
  res_dm=$(echo $?)
  if [ ${res_dm} == 0 ]; then
    rm -rf dataMigration
  fi

  ls | grep ddlResult
  res_ddl=$(echo $?)
  if [ ${res_ddl} == 0 ]; then
    rm -rf ddlResult
  fi
  cd -

  # 清除secret gaussdbremote密码
  log_info "start to delete gaussderemote relate secret"
  PAYLOAD="[{\"op\": \"remove\", \"path\": \"/data/database.remoteUsername_V5\"}]"
  common_secret_url="https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/${namespace}/secrets/common-secret"
  for i in {1..3}; do
    if curl_cmd "PATCH" "application/json-patch+json" "${PAYLOAD}" "$common_secret_url"; then
      log_info "${LINENO} delete gaussdbremote user succeed"
      break
    fi
  done

  PAYLOAD="[{\"op\": \"remove\", \"path\": \"/data/database.remotePassword_V5\"}]"
  for i in {1..3}; do
    if curl_cmd "PATCH" "application/json-patch+json" "${PAYLOAD}" "$common_secret_url"; then
      log_info "${LINENO} delete gaussdbremote password succeed"
      break
    fi
  done

  # 校验完整性
  cd ${GAUSSDB_PATH}/GaussDB_T_1.9.0-DATASYNC
  res=$(sha256sum -c DataSync.tar.gz.sha256)
  check_res=$(echo $res | grep "OK")
  if [ -z "${check_res}" ]; then
    log_error "${LINENO} check datasync integrity failed"
    exit 1
  fi
  log_info "check datasync integrity succeed"
  cd -
  mv gsjdbc4.jar ${DATASYNC_PATH}/dependency-jars/

  # 查询数据库ip
  pod_info=$(LD_PRELOAD=/usr/lib64/libSecurityStarter.so curl --cacert ${rootCAFile} -X GET -H "Authorization: Bearer $TOKEN" $Host/api/v1/namespaces/${namespace}/pods follow@-H)
  gaussdb_name=$(echo ${pod_info} | python3 -c "import sys, json, gaussdb_common;print(gaussdb_common.get_pod_name(json.load(sys.stdin)['items']))")
  gaussdb_info=$(LD_PRELOAD=/usr/lib64/libSecurityStarter.so curl --cacert ${rootCAFile} -X GET -H "Authorization: Bearer $TOKEN" $Host/api/v1/namespaces/${namespace}/pods/${gaussdb_name} follow@-H)
  gaussdb_ip=$(echo ${gaussdb_info} | python3 -c "import sys, json;print(json.load(sys.stdin)['status'].get('podIP'))")
  if [ "${gaussdb_ip}" != "None" ]; then
    log_info "${LINENO} succeed to get gaussdb pod ip"
  else
    log_error "${LINENO} failed to get gaussdb pod ip"
    exit 1
  fi

  # 获取数据库超级用户密码
  user_info=$(LD_PRELOAD=/usr/lib64/libSecurityStarter.so curl --cacert ${rootCAFile} -X GET -H "Authorization: Bearer $TOKEN" $Host/api/v1/namespaces/${namespace}/secrets/${secret} follow@-H)
  encrypt_passwd=$(echo ${user_info} | python3 -c "import sys, json;print(json.load(sys.stdin)['data'].get('database.superPassword_new'))")
  passwd=$(python3 -c 'import gaussdb_kmc; print(gaussdb_kmc.decrypt_secret("'${encrypt_passwd}'"))')
  if [ "${passwd}" != "None" ]; then
    log_info "${LINENO} succeed to decrypt generaldb password"
  else
    log_error "${LINENO} failed to decrypt generaldb password"
    exit 1
  fi

  # 输入密码
  type=1
  LD_PRELOAD=/usr/lib64/libSecurityStarter.so expect auto_password_input.sh $GAUSSDB_PATH $DATASYNC_PATH $passwd $type index@4
  check_result "$?" "${LINENO} expect password"

  # 写入参数至cfg.ini
  result=$(python3 -c 'import gaussdb_common;print(gaussdb_common.write_export_cfg("'${gaussdb_ip}'"))')

  # 修改和规范数据表
  if [ -n "$LD_LIBRARY_PATH" ]; then
    export LD_LIBRARY_PATH='/usr/local/gaussdb/app/lib':$LD_LIBRARY_PATH
  else
    export LD_LIBRARY_PATH='/usr/local/gaussdb/app/lib'
  fi

  /usr/local/gaussdb/app/bin/gsql PROTECT_MANAGER -W ${passwd} -p 6432 -h ${gaussdb_ip} -c "alter table LOCK_RESOURCES rename constraint RESOURCES_PKEY to LOCK_RESOURCES_PKEY;"
  /usr/local/gaussdb/app/bin/gsql PROTECT_MANAGER -W ${passwd} -p 6432 -h ${gaussdb_ip} -c "update PROTECT_MANAGER_MIGRATE set REPOSITORY_ID = 'protect_manager';"

  # 执行命令
  cd $DATASYNC_PATH
  java -jar DSS.jar -p config/cfg.ini
  result=$(echo $?)
  cp -r ${DATASYNC_PATH}/logs/* ${LOG_PATH}/export_logs/
  if [ ${result} != 0 ]; then
    log_error "${LINENO} failed in backup data"
    exit 1
  fi
  cd -

  # 将文件备份到PVC下
  mv /home/GaussDB/dataMigration ${PVC}/
  mv ${DATASYNC_PATH}/ddlResult ${PVC}/

  log_info "succeed backup GaussDB data"
else
  log_info "Do not need to update GaussDB"
fi

exit 0
