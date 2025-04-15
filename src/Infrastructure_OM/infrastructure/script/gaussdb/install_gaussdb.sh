#!/bin/bash

source ./log.sh
NODE_NAME=${NODE_NAME}
LOG_PATH="/opt/OceanProtect/logs/${NODE_NAME}/infrastructure/gaussdb"
GAUSSDB_PATH="/usr/local/gaussdb"
HA_PATH="/usr/local/ha"
HA_MARK_FILE="/usr/local/gaussdb/data/ha_started"
G_NODE_MARK_FILE="/opt/third_data/ha/gaussdb_node"
tokenFile=$(cat /var/run/secrets/kubernetes.io/serviceaccount/token)
rootCAFile="/var/run/secrets/kubernetes.io/serviceaccount/ca.crt"
buildMarkFile="/usr/local/gaussdb/data/build_completed.start"
CLEAR_FLOAT_IP_YAML_FILE="/usr/local/gaussdb/clear-float-ip-job.yaml"
CP_YAML_FILE="/opt/third_data/ha/clear-float-ip-job.yaml"
CLEAR_FLOAT_IP_JOB_NAME="clear-float-ip-job"
GAUSS_DATA="/usr/local/gaussdb/data"


# 生成证书阶段/opt/OceanProtect/已修改为750,nobody:nobody
if [ ! -d ${LOG_PATH} ];then
    sudo /opt/script/change_permission.sh mkdir ${LOG_PATH}
    if [ "$?" != "0" ];then
        echo "mkdir ${LOG_PATH} failed"
        exit 1
    fi
fi

sudo /opt/script/change_permission.sh chown ${LOG_PATH}
if [ "$?" != "0" ];then
    echo "chown ${LOG_PATH} failed"
    exit 1
fi

if [ ! -f "${LOG_PATH}/install_gaussdb.log" ];then
    touch ${LOG_PATH}/install_gaussdb.log
    chown GaussDB:nobody ${LOG_PATH}/install_gaussdb.log
    chmod 600 ${LOG_PATH}/install_gaussdb.log
fi
sudo /opt/script/change_permission.sh chown "${NODE_NAME}" ${LOG_PATH}/install_gaussdb.log

chmod 700 ${LOG_PATH}
check_result "$?" "${LINENO} chmod 700 ${LOG_PATH}"

if [[ ${DEPLOY_TYPE} == "d0" || ${DEPLOY_TYPE} == "d1" || ${DEPLOY_TYPE} == "d2" || ${DEPLOY_TYPE} == "d6" || ${DEPLOY_TYPE} == "d8" ]]; then
    log_info "Exec ${LINENO} Deploy_type: ${DEPLOY_TYPE}, start to install ha."
    sh /usr/local/gaussdb/install_ha.sh
    if [ "$?" != "0" ];then
        log_error "Exec ${LINENO} Install ha failed."
        exit 1
    fi
    log_info "Exec ${LINENO} Install ha succeed."
fi

# 创建文件名
OLD_DB_DATA_PATH="/opt/db_data"
DB_DATA_PATH="/opt/db_data/GaussDB_V5"
DB_RAW_DATA_PATH="${GAUSSDB_PATH}/data"

# 判断是否需要导入V1数据
config_maps=$(LD_PRELOAD=/usr/lib64/libSecurityStarter.so curl --cacert ${rootCAFile} -X GET -H "Authorization: Bearer $tokenFile" https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/dpa/configmaps/common-conf follow@-H)
is_exist=$(echo "${config_maps} " | python3 -c "import sys, json;print(json.load(sys.stdin)['data'].get('gaussdbUpgrade'))")

# 处理直接部署的场景
cd /opt/db_data
ls | grep dataMigration
res=$(echo $?)
cd -

if [ ${res} != 0 ] && [ "${is_exist}" != "true" ]; then
    log_info "Start to add upgrade tag"
    PAYLOAD="{\"data\":{\"gaussdbUpgrade\":\"true\"}}"
    for i in {1..3}
    do
        curl --cacert ${rootCAFile} \
          -X PATCH \
          -H "Content-Type: application/strategic-merge-patch+json" \
          -H "Authorization: Bearer ${tokenFile}" \
          --data "${PAYLOAD}" \
          https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/dpa/configmaps/common-conf
        log_info "${LINENO} Set up upgrade tag succeed"
    done
fi

# 清理残留nfs文件
find ${LOG_PATH} -type f -name ".nfs*" -delete
check_result "$?" "${LINENO} find ${LOG_PATH} -type f -name \".nfs*\" -delete"

LISTEN_ADDRESS=${POD_IP}

# 第一次安装部署，DB_DATA_PATH为pvc下目录，此时为空
if [ ! -f "${DB_DATA_PATH}/postgresql.conf" ];then
    if [ ! -d ${DB_DATA_PATH} ];then
        sudo /opt/script/change_permission.sh mkdir ${DB_DATA_PATH}
    fi
    # 解决安全一体机hostpath权限不对的问题
    sudo /opt/script/change_permission.sh chown ${OLD_DB_DATA_PATH}
    sudo /opt/script/change_permission.sh chown ${DB_DATA_PATH}
    check_result "$?" "${LINENO} chown GaussDB:dbgrp ${DB_DATA_PATH}"
    cp -rpf ${DB_RAW_DATA_PATH}/* ${DB_DATA_PATH}

    # 允许GaussDB用户本地连接，pg_hba的匹配规则为从上到下
    echo "hostssl    all             GaussDB        0.0.0.0/0           trust" >> ${DB_DATA_PATH}/pg_hba.conf
    # 添加密码保护
    echo "hostssl    all             all            0.0.0.0/0           sha256" >> ${DB_DATA_PATH}/pg_hba.conf

fi

change_config()
{
    # 如果 GAUSSDB_LOG_QUERY 开启，打开数据库慢查询记录
    if [ "$GAUSSDB_LOG_QUERY" = "on" ]; then
        gs_guc reload -c log_min_duration_statement=1000 -c log_duration=on -D "$GAUSS_DATA"
        log_info "Turned on slow query log successfully"
    fi

    # 修改pg_csnlog大小配置
    sed -i "s/^#autovacuum_freeze_max_age.*/autovacuum_freeze_max_age = 200000000/g" ${DB_DATA_PATH}/postgresql.conf
    check_result "$?" "${LINENO} set autovacuum_freeze_max_age:200000000 for data/postgresql.conf"

    #修改密码错误锁定时间
    sed -i "s/^#password_reuse_time.*/password_reuse_time=0/g" ${DB_DATA_PATH}/postgresql.conf
    check_result "$?" "${LINENO} set password_reuse_time:0 for data/postgresql.conf"

    sed -i "s/^#password_reuse_max.*/password_reuse_max=0/g" ${DB_DATA_PATH}/postgresql.conf
    check_result "$?" "${LINENO} set password_reuse_max:0 for data/postgresql.conf"

    sed -i "s/^#password_lock_time.*/password_lock_time=0/g" ${DB_DATA_PATH}/postgresql.conf
    check_result "$?" "${LINENO} set password_lock_time:0 for data/postgresql.conf"

    # 修改shared_buffers大小配置
    sed -i "s/^shared_buffers.*/shared_buffers = ${GAUSSDB_SHARED_BUFFERS}/g" ${DB_DATA_PATH}/postgresql.conf
    check_result "$?" "${LINENO} set shared_buffers:${GAUSSDB_SHARED_BUFFERS} for data/postgresql.conf"

    sed -i "s/^#\?work_mem.*/work_mem = ${GAUSSDB_WORK_MEM}/g" ${DB_DATA_PATH}/postgresql.conf
    check_result "$?" "${LINENO} set work_mem:${GAUSSDB_WORK_MEM} for data/postgresql.conf"

    sed -i "s/^wal_buffers.*/wal_buffers = ${GAUSSDB_WAL_BUFFERS}/g" ${DB_DATA_PATH}/postgresql.conf
    check_result "$?" "${LINENO} set wal_buffers:${GAUSSDB_WAL_BUFFERS} for data/postgresql.conf"

    sed -i "s/^#\?wal_writer_delay.*/wal_writer_delay = ${WAL_WRITER_DELAY}/g" ${DB_DATA_PATH}/postgresql.conf
    check_result "$?" "${LINENO} set wal_writer_delay:${WAL_WRITER_DELAY} for data/postgresql.conf"

    sed -i "s/^log_filename.*/log_filename = 'postgresql.log'/g" ${DB_DATA_PATH}/postgresql.conf
    check_result "$?" "${LINENO} set log_filename:postgresql.log for data/postgresql.conf"

    # MAX_CONNECTIONS 在configmap中设置, 并传递给了环境变量
    sed -i "s/^max_connections.*/max_connections = ${GAUSSDB_MAX_CONNECTIONS}/g" ${DB_DATA_PATH}/postgresql.conf
    check_result "$?" "${LINENO} set max_connections:${GAUSSDB_MAX_CONNECTIONS} for data/postgresql.conf"

    # V5关闭线程池
    sed -i "s/^enable_thread_pool.*/enable_thread_pool = off/g" ${DB_DATA_PATH}/postgresql.conf
    check_result "$?" "${LINENO} set enable_thread_pool:off for data/postgresql.conf"

    # 修改session_timeout
    sed -i "s/^session_timeout.*/session_timeout = 0/g" ${DB_DATA_PATH}/postgresql.conf
    check_result "$?" "${LINENO} set session_timeout:0 for data/postgresql.conf"

    # 修改synchronous_standby_names
    sed -i "s/^synchronous_standby_names.*/synchronous_standby_names = ''/g" ${DB_DATA_PATH}/postgresql.conf
    check_result "$?" "${LINENO} set synchronous_standby_names:'' for data/postgresql.conf"

    # 避免在系统日志中记录出错的SQL语句
    sed -i "s/^#log_min_error_statement.*/log_min_error_statement = panic/g" ${DB_DATA_PATH}/postgresql.conf
    check_result "$?" "${LINENO} set log_min_error_statement:panic for data/postgresql.conf"

    # 修改gaussdb的tcp连接超时时间
    sed -i "s/^#tcp_keepalives_idle = 0/tcp_keepalives_idle = 600/g" ${DB_DATA_PATH}/postgresql.conf
    check_result "$?" "${LINENO} sed tcp_keepalives_idle:600 for data/postgresql.conf"

    sed -i "s/^#tcp_keepalives_interval = 0/tcp_keepalives_interval = 5/g" ${DB_DATA_PATH}/postgresql.conf
    check_result "$?" "${LINENO} sed tcp_keepalives_interval:5 for data/postgresql.conf"

    sed -i "s/^#tcp_keepalives_count = 0/tcp_keepalives_count = 3/g" ${DB_DATA_PATH}/postgresql.conf
    check_result "$?" "${LINENO} sed tcp_keepalives_count:3 for data/postgresql.conf"

    # gaussdb v5不转储，利用logrotate转储
    sed -i "s/^log_rotation_size.*/log_rotation_size = 0/g" ${DB_DATA_PATH}/postgresql.conf
    check_result "$?" "${LINENO} set log_rotation_size:0 for data/postgresql.conf"

    # 修改监听地址
    sed -i "/^listen_addresses = .*/c\listen_addresses = '${LISTEN_ADDRESS}'" ${DB_DATA_PATH}/postgresql.conf
    check_result "$?" "${LINENO} sed listen_addresses:${LISTEN_ADDRESS} for data/postgresql.conf"

    timezone=$(python -c "import time; tz=time.strftime('%z'); print(f\"{'-' if tz[0] == '+' else '+'}{tz[1:3]}:{tz[3:5]}\")")
    if [[ -n "$timezone" ]] && [[ ${#timezone} == 6 ]]; then
      sed -i "s/#\?log_timezone.*/log_timezone = '${timezone}'/g" ${DB_DATA_PATH}/postgresql.conf
      check_result "$?" "${LINENO} sed log_timezone:${timezone} for data/postgresql.conf"
    fi
}

change_config

# 此时覆盖老的安装目录
sudo /opt/script/change_permission.sh mount ${DB_DATA_PATH} ${DB_RAW_DATA_PATH}

# 存在软连接先删除，适配POD迁移
if [ -L "${DB_DATA_PATH}/pg_log" ];then
    rm -rf ${DB_DATA_PATH}/pg_log
    check_result "$?" "${LINENO} delete ln"
fi
if [ -d "${DB_DATA_PATH}/pg_log" ];then
    rm -rf ${DB_DATA_PATH}/pg_log
    check_result "$?" "${LINENO} delete pg_log"
fi
ln -s ${LOG_PATH}/ ${DB_DATA_PATH}/pg_log
check_result "$?" "${LINENO} create ln"

# gauss要写archive
if [ ! -d "/tmp/archive" ];then
    mkdir /tmp/archive
    chown GaussDB:dbgrp /tmp/archive
fi
chmod 700 "/tmp/archive"
sudo /opt/script/change_permission.sh mount /tmp/archive /usr/local/gaussdb/archive

# 修改lost+found目录权限
if [ -d "${DB_DATA_PATH}/lost+found" ];then
    sudo /opt/script/change_permission.sh chown ${DB_DATA_PATH}/lost+found
    check_result "$?" "${LINENO} chown lost+found"
fi

# 适配升级，gaussdb_backup需要修改权限为Gaussdb:dbgrp
if [ -d "${DB_DATA_PATH}/gaussdb_backup" ];then
    sudo /opt/script/change_permission.sh chown ${DB_DATA_PATH}/gaussdb_backup
    check_result "$?" "${LINENO} chown gaussdb_backup"
fi

chmod 700 data
check_result "$?" "${LINENO} chmod 700 data"

CLUSTER_ROLE=""
if [[ ${DEPLOY_TYPE} == "d0" || ${DEPLOY_TYPE} == "d1" || ${DEPLOY_TYPE} == "d2" || ${DEPLOY_TYPE} == "d6" ]]; then
    config_maps=$(LD_PRELOAD=/usr/lib64/libSecurityStarter.so curl --cacert ${rootCAFile} -X GET -H "Authorization: Bearer $tokenFile" https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/dpa/configmaps/cluster-conf follow@-H)
    is_exist=$(echo "${config_maps}" | python3 -c "import sys, json;print(json.load(sys.stdin).get('data',{}).get('CLUSTER_ROLE'))")
    CLUSTER_ROLE=${is_exist}
    if [ "${is_exist}" == "None" ];then
        log_info "Start to add cluster conf"
        PAYLOAD="{\"data\":{\"CLUSTER_ROLE\":\"\",\"CLUSTER_ESN\":\"\",\"CLUSTER_SERVICE_IPS\":\"\",\"CLUSTER_ADDING_FLAG\":\"\"}}"
        for i in {1..3}
        do
            curl --cacert ${rootCAFile} \
              -X PATCH \
              -H "Content-Type: application/strategic-merge-patch+json" \
              -H "Authorization: Bearer ${tokenFile}" \
              --data "${PAYLOAD}" \
              https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/dpa/configmaps/cluster-conf
            log_info "${LINENO} Start to check cluster conf"
            config_maps=$(LD_PRELOAD=/usr/lib64/libSecurityStarter.so curl --cacert ${rootCAFile} -X GET -H "Authorization: Bearer $tokenFile" https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/dpa/configmaps/cluster-conf follow@-H)
            is_exist=$(echo "${config_maps} " | python3 -c "import sys, json;print(json.load(sys.stdin).get('data',{}).get('CLUSTER_ROLE'))")
            if [ "${is_exist}" != "None" ];then
                log_info "${LINENO} Succeed to add cluster conf"
                break
            fi
            log_error "${LINENO} Failed to add cluster conf"
            sleep 5
        done
    fi
fi

config_maps=$(LD_PRELOAD=/usr/lib64/libSecurityStarter.so curl --cacert ${rootCAFile} -X GET -H "Authorization: Bearer $tokenFile" https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/dpa/configmaps/common-conf follow@-H)
is_exist=$(echo "${config_maps} " | python3 -c "import sys, json;print(json.load(sys.stdin)['data'].get('rollbacking'))")
image_tag=${IMAGE_VERSION}
ori_dir=${DB_RAW_DATA_PATH}/gaussdb_backup/${image_tag}
dest_dir=${DB_RAW_DATA_PATH}
# 备、成员升级失败不能回滚，否则会将主节点同步后的数据恢复，导致配置错乱
if [ ! -z "${is_exist}" ] && [ "${is_exist}" != "None" ];then
    # set gaussdb rollback flag
    # gaussdb rollback flag is used in post rollback job
    log_info "Start to set gaussdb rollback flag"
    PAYLOAD="{\"data\":{\"gaussdb_rollback\":\"true\"}}"
    for i in {1..3}
    do
        curl --cacert ${rootCAFile} \
          -X PATCH \
          -H "Content-Type: application/strategic-merge-patch+json" \
          -H "Authorization: Bearer ${tokenFile}" \
          --data "${PAYLOAD}" \
          https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/dpa/configmaps/common-conf
        log_info "${LINENO} Start to check gaussdb_rollback flag"
        config_maps=$(LD_PRELOAD=/usr/lib64/libSecurityStarter.so curl --cacert ${rootCAFile} -X GET -H "Authorization: Bearer $tokenFile" https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/dpa/configmaps/common-conf follow@-H)
        is_exist=$(echo "${config_maps} " | python3 -c "import sys, json;print(json.load(sys.stdin)['data'].get('gaussdb_rollback'))")
        if [ ! -z "${is_exist}" ] && [ "${is_exist}" == "true" ];then
            log_info "${LINENO} Succeed to set gaussdb rollback flag"
            break
        fi
        log_error "${LINENO} Failed to set gaussdb rollback flag"
        sleep 5
    done

    # do gaussdb rollback
    if [ -d "${DB_RAW_DATA_PATH}/gaussdb_backup/${image_tag}" ] && [ "${CLUSTER_ROLE}" != "STANDBY" ] && [ "${CLUSTER_ROLE}" != "MEMBER" ];then
        log_info "${LINENO} Start to rollback data"
        cd ${dest_dir}
        rm -rf `ls ${dest_dir} | grep -v gaussdb_backup | grep -v pg_log| xargs`
        cd ${ori_dir}
        cp -rpf `ls ${ori_dir} | grep -v gaussdb_backup | grep -v pg_log| xargs` ${dest_dir}
        cd ${GAUSSDB_PATH}
        log_info "${LINENO} End to rollback data"
    fi
    # cancel roll back flag
    log_info "Start to cancel recover flag"
    PAYLOAD="{\"data\":{\"rollbacking\":\"\",\"gaussdb_rollback\":\"\"}}"
    for i in {1..3}
    do
        curl --cacert ${rootCAFile} \
          -X PATCH \
          -H "Content-Type: application/strategic-merge-patch+json" \
          -H "Authorization: Bearer ${tokenFile}" \
          --data "${PAYLOAD}" \
          https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/dpa/configmaps/common-conf
        log_info "${LINENO} Start to check recover flag"
        config_maps=$(LD_PRELOAD=/usr/lib64/libSecurityStarter.so curl --cacert ${rootCAFile} -X GET -H "Authorization: Bearer $tokenFile" https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/dpa/configmaps/common-conf follow@-H)
        is_exist=$(echo "${config_maps} " | python3 -c "import sys, json;print(json.load(sys.stdin)['data'].get('rollbacking'))")
        if [ -z "${is_exist}" ];then
            log_info "${LINENO} Succeed to cancel recover flag"
            break
        fi
        log_error "${LINENO} Failed to cancel recover flag"
        sleep 5
    done
fi

is_exist=$(echo "${config_maps} " | python3 -c "import sys, json;print(json.load(sys.stdin)['data'].get('loglevel'))")
if [ "${is_exist}" == "None" ];then
    log_info "Start to add log level"
    PAYLOAD="{\"data\":{\"loglevel\":\"INFO\"}}"
    for i in {1..3}
    do
        curl --cacert ${rootCAFile} \
          -X PATCH \
          -H "Content-Type: application/strategic-merge-patch+json" \
          -H "Authorization: Bearer ${tokenFile}" \
          --data "${PAYLOAD}" \
          https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/dpa/configmaps/common-conf
        log_info "${LINENO} Start to check log level"
        config_maps=$(LD_PRELOAD=/usr/lib64/libSecurityStarter.so curl --cacert ${rootCAFile} -X GET -H "Authorization: Bearer $tokenFile" https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/dpa/configmaps/common-conf follow@-H)
        is_exist=$(echo "${config_maps} " | python3 -c "import sys, json;print(json.load(sys.stdin)['data'].get('loglevel'))")
        if [ ! -z "${is_exist}" ] && [ "${is_exist}" != "None" ];then
            log_info "${LINENO} Succeed to add log level"
            break
        fi
        log_error "${LINENO} Failed to add log level"
        sleep 5
    done
fi

source /home/GaussDB/.bashrc

# 配置证书，开启SSL
sh cert_install.sh

term_handler()
{
  # gs_ctl has 3 modes: smart, fast, immediate.
  # In order to stop gaussdb as soon as possible, we use fast mode.
  # Fast mode:
  # Don't wait for the client to disconnect.
  # All active transactions are rolled back and clients are forcibly disconnected.
  # Then the server will be shut down.
  /usr/local/gaussdb/app/bin/gs_ctl stop -D /usr/local/gaussdb/data -U GaussDB -m fast
  check_result "$?" 'gs_ctl stop gaussdb (fast mode)'
  exit 0
}
trap term_handler SIGTERM

# 多集群HA场景，备节点如果存在Guassdb重建中的文件，表示上次Gaussdb重建时pod被杀，需要重新重建gaussdb
if [ "${CLUSTER_ROLE}" == "STANDBY" ] && [ -f ${buildMarkFile} ]; then
    ${GAUSSDB_PATH}/app/bin/gs_ctl -D ${DB_RAW_DATA_PATH} -U GaussDB build
fi

rotate_log() {
    while true; do
        /usr/sbin/logrotate ${GAUSSDB_PATH}/logrotate_gaussdb.conf  -s ${GAUSSDB_PATH}/data/pg_log/logrotate.status >> ${GAUSSDB_PATH}/data/pg_log/logrotate.log 2>&1
        sleep 60
    done
}

function update_k8s()
{
  local payload=$1
  local type=$2
  local type_info=$3
  curl --cacert ${rootCAFile} \
    -X PATCH \
    -H "Content-Type: application/strategic-merge-patch+json" \
    -H "Authorization: Bearer ${tokenFile}" \
    --data "${payload}" \
    https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/dpa/${type}/${type_info}
}

function delete_k8s()
{
  local payload=$1
  local type=$2
  local type_info=$3
  curl --cacert ${rootCAFile} \
    -X PATCH \
    -H "Content-Type: application/json-patch+json" \
    -H "Authorization: Bearer ${tokenFile}" \
    --data "${payload}" \
    https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/dpa/${type}/${type_info}
}

function remove_other_controller_float_ip()
{
    local last_node=`cat ${G_NODE_MARK_FILE}`
    cat $CLEAR_FLOAT_IP_YAML_FILE > $CP_YAML_FILE
    local count=0
    while [ ${count} -lt 3 ]; do
        # 切控则发起job清理浮动ip
        if [ "$NODE_NAME" != $last_node ];then
            log_info "${LINENO} Start to remove previous gaussdb float ip."
            sed -i "s/\(values:\).*$/\1 [\"$last_node\"]/" $CP_YAML_FILE
            sed -i "s/\(image:\).*$/\1 "gaussdb:$IMAGE_VERSION"/" $CP_YAML_FILE
            create_job_result=$(curl --cacert ${rootCAFile} \
            -X POST \
            -H "Content-Type: application/yaml" \
            -H "Authorization: Bearer $tokenFile" \
            --data-binary "@$CP_YAML_FILE" \
            https://${KUBERNETES_SERVICE_HOST}/apis/batch/v1/namespaces/dpa/jobs)
            is_exist=$(echo "${create_job_result} " | python3 -c "import sys, json;print(json.load(sys.stdin)['metadata'].get('name'))")
            # 失败重试3次
            if [ "$is_exist" != $CLEAR_FLOAT_IP_JOB_NAME ];then
                log_error "${LINENO} create remove float ip job failed. ${create_job_result}"
                let count++
            else
                log_info "${LINENO} create remove float ip job success."
                return 0
            fi
        # 未切控直接退出
        else
            return 0
        fi
    done
    return 1
}

log_info "Starting gaussdb"
if [[ ${DEPLOY_TYPE} == "d8" ]];then
  if [[ "${CLUSTER}" == "TRUE" ]];then
    config_maps=$(LD_PRELOAD=/usr/lib64/libSecurityStarter.so curl -f --cacert ${rootCAFile} -X GET -H "Authorization: Bearer $tokenFile" https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/dpa/configmaps/multicluster-conf follow@-H)
    if [ $? -ne 0 ]; then
        log_error "Failed to get multi-cluster conf, exit code $?"
        exit 1
    fi
    CHECK_CLUSTER=$(echo "${config_maps}" | python3 -c "import sys, json;print(json.load(sys.stdin).get('data',{}).get('GAUSSDB_CLUSTER'))")
    CHECK_PRIMARY=$(echo "${config_maps}" | python3 -c "import sys, json;print(json.load(sys.stdin).get('data',{}).get('HA_PRIMARY'))")
    CHECK_STANDBY=$(echo "${config_maps}" | python3 -c "import sys, json;print(json.load(sys.stdin).get('data',{}).get('HA_STANDBY'))")

    if [[ "${CHECK_CLUSTER}" == "false" ]];then
      # 更新标记
      PAYLOAD="{\"data\":{\"GAUSSDB_CLUSTER\":\"true\"}}"
      update_k8s "${PAYLOAD}" "configmaps" "multicluster-conf"
      log_info "${LINENO} Set up multi-cluster gaussdb tag succeed"

      # 更新主节点标签以便区分k8s service
      PRIMARY_PAYLOAD="{\"metadata\":{\"labels\":{\"role\":\"primary\"}}}"
      update_k8s "${PRIMARY_PAYLOAD}" "pods" "gaussdb-0"
      log_info "${LINENO} Set up init gaussdb master role succeed"
    fi

    # 添加标记
    if [ "${CHECK_CLUSTER}" == "None" ];then
      INIT_PAYLOAD="{\"data\":{\"GAUSSDB_CLUSTER\":\"false\",\"HA_PRIMARY\":\"\",\"HA_STANDBY\":\"\",\"OM_CLUSTER\":\"false\",\"ZK_CLUSTER\":\"false\",\"ES_CLUSTER\":\"false\",\"REDIS_CLUSTER\":\"false\"}}"
      update_k8s "${INIT_PAYLOAD}" "configmaps" "multicluster-conf"
      sleep 5
      log_info "${LINENO} Set up multi-cluster initiation tag succeed"
    fi

    if [[ "${CHECK_PRIMARY}" == "${NODE_NAME}" ]] && [[ -f ${HA_MARK_FILE} ]];then
      log_info "${LINENO} Start gaussdb with MASTER"
      gs_ctl start -D /usr/local/gaussdb/data -M primary
      # 更新主节点标签以便区分k8s service
      ROLE_PAYLOAD="{\"metadata\":{\"labels\":{\"role\":\"primary\"}}}"
      update_k8s "${ROLE_PAYLOAD}" "pods" "${POD_NAME}"
      log_info "${LINENO} Set up gaussdb master role succeed"
    elif [[ -n "${CHECK_STANDBY}" ]];then
      # 如果当前节点不是主节点，并且PRIMARY和STANDBY的值非空，组建过集群，则当前节点以Standby模式启动
      log_info "${LINENO} Start gaussdb with STANDBY"
      gs_ctl start -D /usr/local/gaussdb/data -M standby
      # 更新从节点标签以便区分k8s service
      ROLE_PAYLOAD="{\"metadata\":{\"labels\":{\"role\":\"standby\"}}}"
      update_k8s "${ROLE_PAYLOAD}" "pods" "${POD_NAME}"
      log_info "${LINENO} Set up gaussdb standby role succeed"
    else
      log_info "${LINENO} Start gaussdb with NORMAL"
      gs_ctl start -D /usr/local/gaussdb/data
      if [[ "${POD_NAME}" == "gaussdb-1" ]];then
        (rotate_log &)

        while true
        do
          tail -f /dev/null & wait ${!}
        done
      fi
    fi
  else
    log_info "${LINENO} Start gaussdb with NORMAL"
    gs_ctl start -D /usr/local/gaussdb/data
  fi
else
  # 在主备情况下指定启动角色，否则会以normal启动，主节点只在启动HA时才以primary启动gaussdb
  if [ "${CLUSTER_ROLE}" == "PRIMARY" ] && [ -f ${HA_MARK_FILE} ];then
      log_info "${LINENO} Start gaussdb with PRIMARY"
      gs_ctl start -D /usr/local/gaussdb/data -M primary
      remove_other_controller_float_ip
      if [ $? -ne 0 ]; then
          log_error "${LINENO} Remove float ip job create failed."
      fi
  elif [ "${CLUSTER_ROLE}" == "STANDBY" ];then
      log_info "${LINENO} Start gaussdb with STANDBY"
      gs_ctl start -D /usr/local/gaussdb/data -M standby
  else
      log_info "${LINENO} Start gaussdb with NORMAL"
      gs_ctl start -D /usr/local/gaussdb/data
  fi
fi

# 判断数据库是否连接成功
check_connection=$(gs_ctl status -D /usr/local/gaussdb/data | grep "server is running")

#判断场景为主节点卸载HA情况时，移除HA标记文件，不启动OMM HA组件
if [ "${CLUSTER_ROLE}" == "STANDBY" ] && [ -f ${HA_MARK_FILE} ]; then
    COUNTER=0
    while [ ${COUNTER} -lt 5 ]; do
        if [ ${COUNTER} -lt 4 ]; then
            FLOAT_IP=$(sed -n "s/.*arpip.*value=\"\(.*\)\".*/\1/p" ${HA_PATH}/module/haarb/conf/haarb.xml)
            ping -c 3 $FLOAT_IP > /dev/null
            if [ $? -eq 0 ]; then
                log_info "${LINENO} Float IP detected."
                break
            fi
            curl -kv --connect-timeout 3 --max-time 3 gaussdb-cluster:$GAUSSDB_PORT 2>&1 | grep 'Empty reply from server'
            if [ $? -eq 1 ]; then
                log_info "${LINENO} Gaussdb service undetected."
                break
            fi
            log_info "${LINENO} Checking again..."
            sleep 60
        else
            rm -f ${HA_MARK_FILE}
            log_info "${LINENO} Remove ha mark file."
        fi
        let COUNTER++
    done
fi

common_secert=$(curl --cacert ${rootCAFile} -X GET -H "Content-Type: application/strategic-merge-patch+json" -H "Authorization: Bearer ${tokenFile}" https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/dpa/secrets/common-secret)
remote_passwd_exist=$(echo $common_secert | python -c "import json, sys; print('true' if json.load(sys.stdin).get('data', {}).get('database.remotePassword_V5') else 'false')")
# 检查是否已经创建gaussdbremote，已存在则不再创建
username=$(gsql postgres -p 6432 -v ON_ERROR_STOP=on -Atc "select usename from pg_user where usename='gaussdbremote';")

# ON_ERROR_STOP=on时，return 0代表sql执行成功
if [ $? -eq 0 ] && [ "$username" != "gaussdbremote" ] && [ $remote_passwd_exist == "false" ]; then
    log_info "${LINENO} Start to create gaussdbremote"
    log_info "${LINENO} Username:${username}"
    # 生成随机密码,密码中至少包含三种字符
    while true; do
      gaussdbremote_password=$(openssl rand -base64 8)
      if [[ $(echo "$gaussdbremote_password" | grep -c '[a-z]') -ge 1 && \
            $(echo "$gaussdbremote_password" | grep -c '[A-Z]') -ge 1 && \
            $(echo "$gaussdbremote_password" | grep -c '[0-9]') -ge 1 ]]; then
              break
      fi
    done
    gsql postgres -p 6432 -v ON_ERROR_STOP=on -c "CREATE USER gaussdbremote WITH SYSADMIN password '${gaussdbremote_password}'"
    check_result "$?" "${LINENO} create gaussdbremote user"
    # kmc加密gaussdbremote_password
    kmc_password=`python3 -c 'import gaussdb_kmc; print(gaussdb_kmc.encrypt_secret("'${gaussdbremote_password}'"))'`
    if [ "${kmc_password}" != "None" ];then
      log_info "${LINENO} Succeed to decrypt gaussdbremote password"
    else
      log_error "${LINENO} Failed to decrypt gaussdbremote password"
      exit 1
    fi

    PAYLOAD="{\"data\":{\"database.remoteUsername_V5\": \"Z2F1c3NkYnJlbW90ZQ==\", \"database.remotePassword_V5\": \"${kmc_password}\"}}"
    for i in {1..3}
    do
        curl --cacert ${rootCAFile} -o /dev/null -s\
          -X PATCH \
          -H "Content-Type: application/strategic-merge-patch+json" \
          -H "Authorization: Bearer ${tokenFile}" \
          --data "${PAYLOAD}" \
          https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/dpa/secrets/common-secret
        log_info "${LINENO} Start to check gaussdbremote secret"
        secrets=$(LD_PRELOAD=/usr/lib64/libSecurityStarter.so curl --cacert ${rootCAFile} -X GET -H "Authorization: Bearer $tokenFile" https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/dpa/secrets/common-secret follow@-H)
        is_exist=$(echo "${secrets} " | python3 -c "import sys, json;print(json.load(sys.stdin)['data'].get('database.remotePassword_V5'))")
        if [ ! -z "${is_exist}" ] && [ "${is_exist}" != "None" ];then
            log_info "${LINENO} Succeed to add gaussdbremote secret"
            break
        fi
        log_error "${LINENO} Failed to add gaussdbremote secret"
        sleep 5
    done
else
    log_info "${LINENO} Username:${username}"
    log_info "${LINENO} Gaussdbremote already exists or sql execute faild"
fi

config_info=$(LD_PRELOAD=/usr/lib64/libSecurityStarter.so curl --cacert ${rootCAFile} -X GET -H "Authorization: Bearer $tokenFile" https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/dpa/configmaps/common-conf follow@-H)
cluster_tag=$(echo ${config_info} | python3 -c "import sys, json;print(json.load(sys.stdin)['data'].get('clusterSync'))")
secrets=$(LD_PRELOAD=/usr/lib64/libSecurityStarter.so curl --cacert ${rootCAFile} -X GET -H "Authorization: Bearer $tokenFile" https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/dpa/secrets/common-secret follow@-H)
# 多集群数据迁移成员节点同步密钥
if [ "${CLUSTER_ROLE}" == "MEMBER" ] && [ "${cluster_tag}" != "true" ]; then
    # 获取原数据库密码
    remote_password=$(echo "${secrets} " | python3 -c "import sys, json;print(json.load(sys.stdin)['data'].get('database.superPassword_new'))")
    general_password=$(echo "${secrets} " | python3 -c "import sys, json;print(json.load(sys.stdin)['data'].get('database.generalPassword_new'))")

    if [ "$remote_password" != "None" ] && [ "$general_password" != "None" ]; then
        # 将原数据库密码覆盖到新的
        REMOTE_PAYLOAD="{\"data\":{\"database.remoteUsername_V5\": \"Z2F1c3NkYnJlbW90ZQ==\", \"database.remotePassword_V5\": \"${remote_password}\"}}"
        for i in {1..3}
        do
            curl --cacert ${rootCAFile} -o /dev/null -s\
              -X PATCH \
              -H "Content-Type: application/strategic-merge-patch+json" \
              -H "Authorization: Bearer ${tokenFile}" \
              --data "${REMOTE_PAYLOAD}" \
              https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/dpa/secrets/common-secret
            log_info "${LINENO} Start to check gaussdbremote secret"
            secrets=$(LD_PRELOAD=/usr/lib64/libSecurityStarter.so curl --cacert ${rootCAFile} -X GET -H "Authorization: Bearer $tokenFile" https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/dpa/secrets/common-secret follow@-H)
            is_exist=$(echo "${secrets} " | python3 -c "import sys, json;print(json.load(sys.stdin)['data'].get('database.remotePassword_V5'))")
            if [ ! -z "${is_exist}" ] && [ "${is_exist}" != "None" ];then
                log_info "${LINENO} Succeed to add gaussdbremote secret"
                break
            fi
            log_error "${LINENO} Failed to add gaussdbremote secret"
            sleep 5
        done

        GENERAL_PAYLOAD="{\"data\":{\"database.gaussdbUsername_V5\": \"Z2VuZXJhbGRi\", \"database.gaussdbPassword_V5\": \"${general_password}\"}}"
        for i in {1..3}
        do
            curl --cacert ${rootCAFile} -o /dev/null -s\
              -X PATCH \
              -H "Content-Type: application/strategic-merge-patch+json" \
              -H "Authorization: Bearer ${tokenFile}" \
              --data "${GENERAL_PAYLOAD}" \
              https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/dpa/secrets/common-secret
            log_info "${LINENO} Start to check gaussdbremote secret"
            secrets=$(LD_PRELOAD=/usr/lib64/libSecurityStarter.so curl --cacert ${rootCAFile} -X GET -H "Authorization: Bearer $tokenFile" https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/dpa/secrets/common-secret follow@-H)
            is_exist=$(echo "${secrets} " | python3 -c "import sys, json;print(json.load(sys.stdin)['data'].get('database.gaussdbPassword_V5'))")
            if [ ! -z "${is_exist}" ] && [ "${is_exist}" != "None" ];then
                log_info "${LINENO} Succeed to add generaldb secret"
                break
            fi
            log_error "${LINENO} Failed to add generaldb secret"
            sleep 5
        done

        # 创建用户并修改密码
        tmp_remote_password=`python3 -c 'import gaussdb_kmc; print(gaussdb_kmc.decrypt_secret("'${remote_password}'"))'`
        if [ "${tmp_remote_password}" != "None" ];then
            log_info "${LINENO} Succeed to decrypt gaussdbremote password"
        else
            log_error "${LINENO} Failed to decrypt gaussdbremote password"
            exit 1
        fi
        gsql postgres -p 6432 -c "CREATE USER gaussdbremote WITH password '${tmp_remote_password}'"

        tmp_general_passwd=`python3 -c 'import gaussdb_kmc; print(gaussdb_kmc.decrypt_secret("'${general_password}'"))'`
        if [ "${tmp_general_passwd}" != "None" ];then
            log_info "${LINENO} Succeed to decrypt generaldb password"
        else
            log_error "${LINENO} Failed to decrypt generaldb password"
            exit 1
        fi
        gsql postgres -p 6432 -c "CREATE USER generaldb WITH password '${tmp_general_passwd}'"
    fi
fi

# 数据迁移
GAUSSDB_PATH="/usr/local/gaussdb"
DATASYNC_PATH="/opt/db_data/GaussDB_V5/GaussDB_T_1.9.0-DATASYNC/DataSync"

config_info=$(LD_PRELOAD=/usr/lib64/libSecurityStarter.so curl --cacert ${rootCAFile} -X GET -H "Authorization: Bearer $tokenFile" https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/dpa/configmaps/common-conf follow@-H)
config_tag=$(echo ${config_info} | python3 -c "import sys, json;print(json.load(sys.stdin)['data'].get('gaussdbUpgrade'))")
secrets=$(LD_PRELOAD=/usr/lib64/libSecurityStarter.so curl --cacert ${rootCAFile} -X GET -H "Authorization: Bearer $tokenFile" https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/dpa/secrets/common-secret follow@-H)
if [ "${config_tag}" != "true" ] && [ -n "$check_connection" ]; then
    # 设置迁移标记
    log_info "${LINENO} Start to add migrate tag"
    PAYLOAD="{\"data\":{\"gaussdbMigrate\":\"true\"}}"
    update_k8s "${PAYLOAD}" "configmaps" "common-conf"
    log_info "${LINENO} Set up migrate tag succeed"

    generaldb_is_exist=$(echo $(gsql postgres -p 6432 -c "select usename from pg_user;" | grep "generaldb"))
    if [ -z "$generaldb_is_exist" ];then
        echo "start to create generaldb"
        # 创建普通数据库用户密码,密码中至少包含三种字符
        while true; do
          general_passwd=$(openssl rand -base64 8)
          if [[ $(echo "$general_passwd" | grep -c '[a-z]') -ge 1 && \
                $(echo "$general_passwd" | grep -c '[A-Z]') -ge 1 && \
                $(echo "$general_passwd" | grep -c '[0-9]') -ge 1 ]]; then
                  break
          fi
        done
        gsql postgres -p 6432 -c "CREATE USER generaldb WITH password '${general_passwd}'"
        check_result "$?" "${LINENO} create generaldb user"
        # kmc加密
        kmc_password=`python3 -c 'import gaussdb_kmc; print(gaussdb_kmc.encrypt_secret("'${general_passwd}'"))'`
        if [ "${kmc_password}" != "None" ];then
            log_info "${LINENO} Succeed to encrypt generaldb password"
        else
            log_error "${LINENO} Failed to encrypt generaldb password"
            exit 1
        fi

        # 写入普通数据库用户名和密码
        PAYLOAD="{\"data\":{\"database.gaussdbUsername_V5\": \"Z2VuZXJhbGRi\", \"database.gaussdbPassword_V5\": \"${kmc_password}\"}}"
        for i in {1..3}
        do
            curl --cacert ${rootCAFile} -o /dev/null -s\
              -X PATCH \
              -H "Content-Type: application/strategic-merge-patch+json" \
              -H "Authorization: Bearer ${tokenFile}" \
              --data "${PAYLOAD}" \
              https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/dpa/secrets/common-secret
            log_info "${LINENO} Start to check generaldb secret"
            secrets=$(LD_PRELOAD=/usr/lib64/libSecurityStarter.so curl --cacert ${rootCAFile} -X GET -H "Authorization: Bearer $tokenFile" https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/dpa/secrets/common-secret follow@-H)
            is_exist=$(echo "${secrets} " | python3 -c "import sys, json;print(json.load(sys.stdin)['data'].get('database.gaussdbPassword_V5'))")
            if [ ! -z "${is_exist}" ] && [ "${is_exist}" != "None" ];then
                log_info "${LINENO} Succeed to add generaldb secret"
                break
            fi
            log_error "${LINENO} Failed to add generaldb secret"
            sleep 5
        done

        # 多集群数据迁移主节点同步密码
        if [ "${CLUSTER_ROLE}" == "PRIMARY" ]; then
            # 获取原数据库密码
            remote_password=$(echo "${secrets} " | python3 -c "import sys, json;print(json.load(sys.stdin)['data'].get('database.superPassword_new'))")
            general_password=$(echo "${secrets} " | python3 -c "import sys, json;print(json.load(sys.stdin)['data'].get('database.generalPassword_new'))")

            if [ "$remote_password" != "None" ] && [ "$general_password" != "None" ]; then
                # 将原数据库密码覆盖到新的
                REMOTE_PAYLOAD="{\"data\":{\"database.remoteUsername_V5\": \"Z2F1c3NkYnJlbW90ZQ==\", \"database.remotePassword_V5\": \"${remote_password}\"}}"
                for i in {1..3}
                do
                    curl --cacert ${rootCAFile} -o /dev/null -s\
                      -X PATCH \
                      -H "Content-Type: application/strategic-merge-patch+json" \
                      -H "Authorization: Bearer ${tokenFile}" \
                      --data "${REMOTE_PAYLOAD}" \
                      https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/dpa/secrets/common-secret
                    log_info "${LINENO} Start to check gaussdbremote secret"
                    secrets=$(LD_PRELOAD=/usr/lib64/libSecurityStarter.so curl --cacert ${rootCAFile} -X GET -H "Authorization: Bearer $tokenFile" https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/dpa/secrets/common-secret follow@-H)
                    is_exist=$(echo "${secrets} " | python3 -c "import sys, json;print(json.load(sys.stdin)['data'].get('database.remotePassword_V5'))")
                    if [ ! -z "${is_exist}" ] && [ "${is_exist}" != "None" ];then
                        log_info "${LINENO} Succeed to add gaussdbremote secret"
                        break
                    fi
                    log_error "${LINENO} Failed to add gaussdbremote secret"
                    sleep 5
                done

                GENERAL_PAYLOAD="{\"data\":{\"database.gaussdbUsername_V5\": \"Z2VuZXJhbGRi\", \"database.gaussdbPassword_V5\": \"${general_password}\"}}"
                for i in {1..3}
                do
                    curl --cacert ${rootCAFile} -o /dev/null -s\
                      -X PATCH \
                      -H "Content-Type: application/strategic-merge-patch+json" \
                      -H "Authorization: Bearer ${tokenFile}" \
                      --data "${GENERAL_PAYLOAD}" \
                      https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/dpa/secrets/common-secret
                    log_info "${LINENO} Start to check gaussdbremote secret"
                    secrets=$(LD_PRELOAD=/usr/lib64/libSecurityStarter.so curl --cacert ${rootCAFile} -X GET -H "Authorization: Bearer $tokenFile" https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/dpa/secrets/common-secret follow@-H)
                    is_exist=$(echo "${secrets} " | python3 -c "import sys, json;print(json.load(sys.stdin)['data'].get('database.gaussdbPassword_V5'))")
                    if [ ! -z "${is_exist}" ] && [ "${is_exist}" != "None" ];then
                        log_info "${LINENO} Succeed to add generaldb secret"
                        break
                    fi
                    log_error "${LINENO} Failed to add generaldb secret"
                    sleep 5
                done

                # 修改密码
                tmp_remote_password=`python3 -c 'import gaussdb_kmc; print(gaussdb_kmc.decrypt_secret("'${remote_password}'"))'`
                if [ "${tmp_remote_password}" != "None" ];then
                    log_info "${LINENO} Succeed to decrypt gaussdbremote password"
                else
                    log_error "${LINENO} Failed to decrypt gaussdbremote password"
                    exit 1
                fi
                gsql postgres -p 6432 -c "ALTER USER gaussdbremote WITH password '${tmp_remote_password}'"

                tmp_general_passwd=`python3 -c 'import gaussdb_kmc; print(gaussdb_kmc.decrypt_secret("'${general_password}'"))'`
                if [ "${tmp_general_passwd}" != "None" ];then
                    log_info "${LINENO} Succeed to decrypt generaldb password"
                else
                    log_error "${LINENO} Failed to decrypt generaldb password"
                    exit 1
                fi
                gsql postgres -p 6432 -c "ALTER USER generaldb WITH password '${tmp_general_passwd}'"
            fi
        fi

        # 校验工具
        cd ${GAUSSDB_PATH}/GaussDB_T_1.9.0-DATASYNC
        res=$(sha256sum -c DataSync.tar.gz.sha256)
        check_res=$(echo $res | grep "OK")
        if [ -z "${check_res}" ]; then
            log_error "${LINENO} Check datasync integrity failed"
            exit 1
        fi
        log_info "Check datasync integrity succeed"
        cd -

        # 由于容器内部只读，挪用工具到pvc
        cp -r GaussDB_T_1.9.0-DATASYNC ${DB_DATA_PATH}
        cp gsjdbc4.jar ${DATASYNC_PATH}/dependency-jars/

        # 导入校验文件
        mv /opt/db_data/ddlResult ${DATASYNC_PATH}/

        # 创建数据库
        remote_passwd=$(echo "${secrets} " | python3 -c "import sys, json;print(json.load(sys.stdin)['data'].get('database.remotePassword_V5'))")
        passwd=`python3 -c 'import gaussdb_kmc; print(gaussdb_kmc.decrypt_secret("'${remote_passwd}'"))'`

        gsql postgres -p 6432 -U "gaussdbremote" -W ${passwd} -c "CREATE DATABASE admindb dbcompatibility = 'PG';"
        check_result "$?" "${LINENO} create admindb database"
        gsql postgres -p 6432 -U "gaussdbremote" -W ${passwd} -c "CREATE DATABASE anon_policy dbcompatibility = 'PG' ENCODING 'UTF8' TEMPLATE template0;"
        check_result "$?" "${LINENO} create anon_policy database"
        gsql postgres -p 6432 -U "gaussdbremote" -W ${passwd} -c "CREATE DATABASE anti_ransomware dbcompatibility = 'PG' ENCODING 'UTF8' TEMPLATE template0;"
        check_result "$?" "${LINENO} create anti_ransomware database"
        gsql postgres -p 6432 -U "gaussdbremote" -W ${passwd} -c "CREATE DATABASE applicationdb dbcompatibility = 'PG';"
        check_result "$?" "${LINENO} create applicationdb database"
        gsql postgres -p 6432 -U "gaussdbremote" -W ${passwd} -c "CREATE DATABASE archivedb dbcompatibility = 'PG';"
        check_result "$?" "${LINENO} create archivedb database"
        gsql postgres -p 6432 -U "gaussdbremote" -W ${passwd} -c "CREATE DATABASE base_parser dbcompatibility = 'PG' ENCODING 'UTF8' TEMPLATE template0;"
        check_result "$?" "${LINENO} create base_parser database"
        gsql postgres -p 6432 -U "gaussdbremote" -W ${passwd} -c "CREATE DATABASE datamoverdb dbcompatibility = 'PG';"
        check_result "$?" "${LINENO} create datamoverdb database"
        gsql postgres -p 6432 -U "gaussdbremote" -W ${passwd} -c "CREATE DATABASE dee_scheduler dbcompatibility = 'PG' ENCODING 'UTF8' TEMPLATE template0;"
        check_result "$?" "${LINENO} create dee_scheduler database"
        gsql postgres -p 6432 -U "gaussdbremote" -W ${passwd} -c "CREATE DATABASE dme_unified dbcompatibility = 'PG';"
        check_result "$?" "${LINENO} create dme_unified database"
        gsql postgres -p 6432 -U "gaussdbremote" -W ${passwd} -c "CREATE DATABASE generalschedulerdb dbcompatibility = 'PG';"
        check_result "$?" "${LINENO} create generalschedulerdb database"
        gsql postgres -p 6432 -U "gaussdbremote" -W ${passwd} -c "CREATE DATABASE indexer dbcompatibility = 'PG' ENCODING 'UTF8' TEMPLATE template0;"
        check_result "$?" "${LINENO} create indexer database"
        gsql postgres -p 6432 -U "gaussdbremote" -W ${passwd} -c "CREATE DATABASE protect_manager dbcompatibility = 'PG' ENCODING 'UTF8' TEMPLATE template0;"
        check_result "$?" "${LINENO} create protect_manager database"
        gsql postgres -p 6432 -U "gaussdbremote" -W ${passwd} -c "CREATE DATABASE replicationdb dbcompatibility = 'PG';"
        check_result "$?" "${LINENO} create replicationdb database"

        # 获取数据库ip
        pod_info=$(LD_PRELOAD=/usr/lib64/libSecurityStarter.so curl --cacert ${rootCAFile} -X GET -H "Authorization: Bearer $tokenFile" https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/dpa/pods follow@-H)
        gaussdb_name=$(echo ${pod_info} | python3 -c "import sys, json, gaussdb_common;print(gaussdb_common.get_pod_name(json.load(sys.stdin)['items']))")
        gaussdb_info=$(LD_PRELOAD=/usr/lib64/libSecurityStarter.so curl --cacert ${rootCAFile} -X GET -H "Authorization: Bearer $tokenFile" https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/dpa/pods/${gaussdb_name} follow@-H)
        gaussdb_ip=$(echo ${gaussdb_info} | python3 -c "import sys, json;print(json.load(sys.stdin)['status'].get('podIP'))")
        if [ "${gaussdb_ip}" != "None" ];then
            log_info "${LINENO} Succeed to get gaussdb pod ip"
        else
            log_error "${LINENO} Failed to get gaussdb pod ip"
            exit 1
        fi

        # 输入密码
        type=2
        LD_PRELOAD=/usr/lib64/libSecurityStarter.so expect auto_password_input.sh $GAUSSDB_PATH $DATASYNC_PATH $passwd $type index@4

        # 写入参数至cfg.ini
        result=`python3 -c 'import gaussdb_common;print(gaussdb_common.write_import_cfg("'${gaussdb_ip}'"))'`

        # 配置白名单
        echo "host  all  all  ${gaussdb_ip}/32  sha256" >> ${DB_DATA_PATH}/pg_hba.conf
        gs_ctl reload -D ${GAUSSDB_PATH}/data
        check_result "$?" "${LINENO} gs_ctl reload -D ${GAUSSDB_PATH}/data"

    else
        echo "generaldb already exists"
    fi

    # 执行命令
    cd $DATASYNC_PATH
    java -jar DSS.jar -p config/cfg.ini
    result=$(echo $?)
    cp -r ${DATASYNC_PATH}/logs/* ${LOG_PATH}/import_logs/
    if [ ${result} != 0 ]; then
        log_error "${LINENO} Failed in backup data"
        exit 1
    fi
    cd -

    # 清除迁移标记
    log_info "${LINENO} Start to modify migrate tag"
    PAYLOAD="{\"data\":{\"gaussdbMigrate\":\"false\"}}"
    update_k8s "${PAYLOAD}" "configmaps" "common-conf"
    log_info "${LINENO} Revise migrate tag succeed"

    # 迁移成功后清除PVC内的迁移工具和迁移数据
    rm -rf ${DB_DATA_PATH}/GaussDB_T_1.9.0-DATASYNC
    rm -rf /opt/db_data/dataMigration

    log_info "Succeed update GaussDB"
else
    log_info "Do not need to update GaussDB"
fi

if [ -f ${HA_MARK_FILE} ]; then
    # HA运行安装脚本
    log_info "Starting ha"
    sudo /opt/script/ha_sudo.sh ha_start
fi

if [[ ${DEPLOY_TYPE} == "d0" || ${DEPLOY_TYPE} == "d1" || ${DEPLOY_TYPE} == "d2" || ${DEPLOY_TYPE} == "d6" ]]; then
    sudo /usr/sbin/ntpd -u ntp:ntp -g
fi

(rotate_log &)

# wait forever
while true
do
  tail -f /dev/null & wait ${!}
done