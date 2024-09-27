#!/bin/bash

#定义 redis路径，VERSION在打包时替换为实际路径
REDIS_PATH=/usr/local/redis/redis-VERSION
INTER_CERT_PATH=/opt/OceanProtect/infrastructure/cert/redis
THIRD_REDIS_PATH="/opt/third_data/redis"
redis_conf_in_nas=$THIRD_REDIS_PATH/conf/redis.conf
TMP_CMD=${THIRD_REDIS_PATH}/tmpcmd.sh
REDIS_PASSWD=""
NODE_NAME=$NODE_NAME
LOG_PATH="/opt/OceanProtect/logs/${NODE_NAME}/infrastructure/redis"
MASTER_KS='/opt/OceanProtect/protectmanager/kmc/master.ks'
BACKUP_KS='/kmc_conf/..data/backup.ks'

function log_info()
{
    echo "$(date +"[%Y-%m-%d %H:%M:%S,%N]")[INFO][INSTALL_REDIS: $1][$0][${BASH_LINENO}]"
    echo "$(date +"[%Y-%m-%d %H:%M:%S,%N]")[INFO][INSTALL_REDIS: $1][$0][${BASH_LINENO}]" >> ${LOG_PATH}/install_redis.log
}

function log_error()
{
    echo "$(date +"[%Y-%m-%d %H:%M:%S,%N]")[ERROR][INSTALL_REDIS: $1][$0][${BASH_LINENO}]"
    echo "$(date +"[%Y-%m-%d %H:%M:%S,%N]")[ERROR][INSTALL_REDIS: $1][$0][${BASH_LINENO}]" >> ${LOG_PATH}/install_redis.log
}

#获取OM配置的新密码
function wait_om_generate_redis_passwd()
{
    log_info "Start wait om generate redis password."
    while true
    do
        if [ -s /etc/common-secret/redis.username ] && [ -s /etc/common-secret/redis.password ]; then
          break
        fi
        sleep 2
    done
    log_info "OM has generated redis password."
    return 0
}

#配置生效密码
function set_passwd()
{
    #redis.conf中没有密码，则重新配置，密码与新密码一样，直接返回，否则配置新密码
    OLD_PASS=$(grep "requirepass " ${redis_conf_in_nas} | grep -v "#" | awk '{print $2}')
    # Use -m option to avoid specify the absolute path of python script
    python3 -m replace_password_in_conf ${MASTER_KS} ${BACKUP_KS}
    if [ $? != 0 ];then
      log_error "Replace redis password in config file failed."
      return 1
    fi
    if [ "${OLD_PASS}X" == "X" ];then
        # Old password in the config file is empty means it is first start, no need do anything.
        return 0
    fi
    # Old password in the config file is not empty means it is triggered by OM(update password action).
    echo "AUTH ${OLD_PASS}" > ${TMP_CMD}
    echo "CONFIG SET requirepass $1" >> ${TMP_CMD}
    echo "exit" >> ${TMP_CMD}
    ./redis-cli --tls --cert ${INTER_CERT_PATH}/redis.crt.pem \
                --key ${INTER_CERT_PATH}/redis.pem \
                --cacert ${INTER_CERT_PATH}/ca/ca.crt.pem \
                < ${TMP_CMD} -p 6369 -h infrastructure
    if [ $? -ne 0 ];then
        return 1
    fi
    return 0
}

function main()
{
    cd ${REDIS_PATH}
    if [ ! -d ${TMP_CMD} ];then
        touch ${TMP_CMD}
    fi
    chmod 640 "${TMP_CMD}"
    #获取OM密码，如果不成功则等待成功
    wait_om_generate_redis_passwd
    #配置生效密码
    set_passwd ${REDIS_PASSWD}
    if [ $? != 0 ];then
      log_error "Set redis password failed."
      rm -rf ${TMP_CMD}
      exit 1
    fi
}
main
