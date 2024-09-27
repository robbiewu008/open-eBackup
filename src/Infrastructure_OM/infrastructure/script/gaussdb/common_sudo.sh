#!/bin/bash

# 检验NODE_NAME环境变量，防止日志攻击
NODE_NAME=${NODE_NAME}
KUBERNETES_SERVICE_HOST=$KUBERNETES_SERVICE_HOST
KUBE_TOKEN=$(cat /var/run/secrets/kubernetes.io/serviceaccount/token)
KUBE_CACRT_PATH="/var/run/secrets/kubernetes.io/serviceaccount/ca.crt"

LOG_PATH="/opt/OceanProtect/logs/${NODE_NAME}/infrastructure/gaussdb"
LOG_FILE="${LOG_PATH}/install_gaussdb.log"
GAUSS_USER=GaussDB

function check_path_symbolic_link()
{
    path=$1
    echo "$path"
    if [[ -L "${path}" ]]; then
        echo "[ERROR (common_sudo path:${path}), symbolic link is not allowed.]"
        exit 1
    fi
}

check_path_symbolic_link "${LOG_PATH}"
check_path_symbolic_link "/opt/OceanProtect/logs/${NODE_NAME}"
check_path_symbolic_link "/opt/OceanProtect/logs/${NODE_NAME}/infrastructure"
check_path_symbolic_link "${LOG_PATH}/install_gaussdb.log"


if [ ! -d ${LOG_PATH} ]; then
    mkdir -p ${LOG_PATH}
    chown 99:nobody /opt/OceanProtect/logs/${NODE_NAME}
    chown 99:nobody /opt/OceanProtect/logs/${NODE_NAME}/infrastructure
    chmod 750 /opt/OceanProtect/logs/${NODE_NAME}
    chmod 750 /opt/OceanProtect/logs/${NODE_NAME}/infrastructure
    chown GaussDB:dbgrp ${LOG_PATH}
    chmod 750 ${LOG_PATH}
fi

if [ ! -f ${LOG_PATH}/install_gaussdb.log ]; then
    touch ${LOG_PATH}/install_gaussdb.log
fi
chown GaussDB:nobody ${LOG_PATH}/install_gaussdb.log
chmod 640 ${LOG_PATH}/install_gaussdb.log

function log_error()
{
    if [ $# -ne 3 ];then
        echo "log error param is error"
        exit 1
    fi

    # 写日志降权处理
    err_msg=[`date +"%Y-%m-%d %H:%M:%S"`][ERROR][$3][${GAUSS_USER}][$2][$1]
    echo ${err_msg}
    su - ${GAUSS_USER} -s /bin/bash -c "echo '${err_msg}' >>${LOG_FILE}"
}

function log_info()
{
    if [ $# -ne 3 ];then
        echo "log info param is error"
        exit 1
    fi

    # 写日志降权处理
    msg=[`date +"%Y-%m-%d %H:%M:%S"`][INFO][$3][${GAUSS_USER}][$2][$1]
    echo ${msg}
    su - ${GAUSS_USER} -s /bin/bash -c "echo '${msg}' >>${LOG_FILE}"
}
