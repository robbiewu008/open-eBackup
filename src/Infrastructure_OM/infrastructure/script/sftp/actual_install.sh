#!/bin/bash
MY_POD_NAME=${MY_POD_NAME}
NODE_NAME=${NODE_NAME}
ENVIRONMENT=${ENVIRONMENT}
KUBERNETES_SERVICE_HOST=${KUBERNETES_SERVICE_HOST}
KUBERNETES_SERVICE_PORT=${KUBERNETES_SERVICE_PORT}
KUBE_TOKEN=$(cat /var/run/secrets/kubernetes.io/serviceaccount/token)
KUBE_CACRT_PATH="/var/run/secrets/kubernetes.io/serviceaccount/ca.crt"
HOST_KEY_PATH="/etc/ssh"
LOG_PATH="/opt/OceanProtect/logs/${NODE_NAME}/infrastructure/sftp"
OPERATION=$1

if [ $# -ne 1 ]; then
  echo "param is error"
  exit 1
fi

function open_sftp() {
  if [ ! -d "${HOST_KEY_PATH}" ]; then
    mkdir "${HOST_KEY_PATH}"
  fi
  chmod 750 "${HOST_KEY_PATH}"
  if [ ! -d "${LOG_PATH}" ]; then
    mkdir -p "${LOG_PATH}"
  fi
  mkdir "/sftp"
  chmod 755 "/sftp"
  chmod 750 "/opt/OceanProtect/logs/${NODE_NAME}/infrastructure/"
  chmod 750 "/opt/OceanProtect/logs/${NODE_NAME}/infrastructure/sftp"
  # 日志目录infrastructure必须是nobody:nobody
  chown -h 99:99 "/opt/OceanProtect/logs/${NODE_NAME}/infrastructure/"
  chown -h 15004:99 "/opt/OceanProtect/logs/${NODE_NAME}/infrastructure/sftp"
  echo "127.0.0.1 nas.storage.sftp.host" >>/etc/hosts

  export PYTHONPATH=/opt/sftp/package/src
  python3 /opt/sftp/package/src/app/service/sftp_management.pyc start &
  python3 /opt/sftp/package/src/app/service/apis/sftp_model.pyc &
}

function delete_env() {
  # 删除不安全环境变量传递
  sed -i '/NOPASSWD:SETENV/d' /etc/sudoers
}

main() {
  if [[ "${OPERATION}" == "open" ]]; then
    open_sftp
    delete_env
  elif [ "${OPERATION}" == "waiting" ]; then
    delete_env
  else
    echo "param is error"
    exit 1
  fi
}

main
