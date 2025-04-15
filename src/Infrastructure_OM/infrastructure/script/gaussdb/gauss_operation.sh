#!/bin/bash

#########################################
# Copyright (c) 2022-2022 Huawei .
# All rights reserved.
#
# Function gauss运维账户提权脚本
# revise note
########################################

GAUSS_OP_USER=GaussOp

master=/opt/OceanProtect/protectmanager/kmc/master.ks
backup=/kmc_conf/..data/backup.ks
POD_IP=$(env | grep POD_IP)
gauss_ip=$(echo ${POD_IP} | awk -F '=' '{print $2}')
gauss_port=6432
gauss_usr=gaussdbremote
gauss_pwd_field=database.superPassword

# 特殊字符集 ~!@#$%^&*()-_=+,<.>;:?[]{}'"/\|` 其中/*?单独处理
# GaussDB密码特殊字符集 ~!@#$%^&*()-_=+,<.>:?[]{}|
SPECIAL_PARAMS=("|" "(" ")" "{" "<" ">" "[" "\`" "~" "!" "@" "#" "$" "%" "^" "&" "-" "=" "+" "}" "]" ";" ":" "\"" "," "." "_" "'" "/" " ")

database_whitelist=(
  "admindb" "anon_policy" "anti_ransomware" "applicationdb" "archivedb" "base_parser" "datamoverdb" "dee_scheduler"
  "dme_unified" "generalschedulerdb" "indexer" "postgres" "protect_manager" "replicationdb"
)

ENV_NODE=$(env | grep node)
node_name=$(echo ${ENV_NODE} | awk -F '=' '{print $2}')

function log_info() {
  if [ $# -ne 3 ]; then
    echo "log info param is error"
    exit 1
  fi

  # 写日志降权处理
  msg=[$(date +"%Y-%m-%d %H:%M:%S")][INFO][$3][${gauss_usr}][$2][$1]
  echo ${msg}
  su - ${gauss_usr} -s /bin/bash -c "echo '${msg}' >>${LOG_FILE}"
}

check_database_whitelist() {
  local database=$1
  for tmp_database in "${database_whitelist[@]}"; do
    if [ ${database} == ${tmp_database} ]; then
      return 0
    fi
  done
  return 1
}

transfor_special_characters() {
  local input_params=$1
  # 首先要转义/
  out_params=$(echo ${input_params//\\/\\\\})
  for ((i = 0; i < ${#SPECIAL_PARAMS[@]}; i++)); do
    out_params=${out_params//${SPECIAL_PARAMS[i]}/\\${SPECIAL_PARAMS[i]}}
  done

  # 对*和？做单独的转义
  out_params=${out_params//\?/\\?}
  out_params=$(echo ${out_params} | sed 's/\*/\\*/g')
  echo ${out_params}
}

function enter_database() {
  local database=$(echo "$1" | tr '[:upper:]' '[:lower:]')
  check_database_whitelist "${database}"
  if [ $? -ne 0 ]; then
    echo "database: ${database} not allowed. try use ${database_whitelist[@]}."
    return 1
  fi

  log_info ${FUNCNAME[0]} ${LINENO} "login database: ${database}."
  export PYTHONPATH=/opt/script
  export NODE_NAME=${node_name}
  export gauss_ip=${gauss_ip}
  if [[ ${gauss_ip} =~ ^[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}$ ]]; then
    export gauss_ip=${gauss_ip}
  else
    log_error "Invalid IP address: ${gauss_ip}"
    exit 1
  fi
  gaussdb_pwd=$(python3 -c "import sys;sys.path.append('/opt/script');import logging; from manage_db_data import *; initialize_kmc('${master}', '${backup}', get_logger(log_file, logging.ERROR)); print(get_db_pwd_from_api('${gauss_pwd_field}'))")
  gaussdb_pwd=$(transfor_special_characters "${gaussdb_pwd}")
  # 使用LD_PRELOAD=/usr/Lib64/LibSecurityStarter.so, 把-c后面的内容屏蔽
  LD_PRELOAD=/usr/lib64/libSecurityStarter.so su - ${GAUSS_OP_USER} -s /bin/bash -c "export LD_LIBRARY_PATH=/usr/local/gaussdb/app/lib::/usr/local/app/lib; /usr/local/gaussdb/app/bin/gsql -U ${gauss_usr} -d ${database} -h ${gauss_ip} -p ${gauss_port} -W ${gaussdb_pwd}" follow@-c
  log_info ${FUNCNAME[0]} ${LINENO} "logout database: ${database}."
}

type="$1"
if [ -z "${type}" ]; then
  echo "type error"
  exit 1
fi

if [ "${type}" == "enter_sql" ]; then
  if [ $# -ne 2 ]; then
    echo "param is error"
    exit 1
  fi
  enter_database "$2"
else
  echo "param is error"
  exit 1
fi
