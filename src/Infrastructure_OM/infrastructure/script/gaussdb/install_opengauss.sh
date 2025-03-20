#!/bin/bash

source ./log.sh
NODE_NAME=${NODE_NAME}
LOG_PATH="/opt/OceanProtect/logs/${NODE_NAME}/infrastructure/gaussdb"
GAUSSDB_PATH="/usr/local/gaussdb"
HA_PATH="/usr/local/ha"
tokenFile=$(cat /var/run/secrets/kubernetes.io/serviceaccount/token)
rootCAFile="/var/run/secrets/kubernetes.io/serviceaccount/ca.crt"
buildMarkFile="/usr/local/gaussdb/data/build_completed.start"

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

# 创建文件名
OLD_DB_DATA_PATH="/opt/db_data"
DB_DATA_PATH="/opt/db_data/GaussDB_V5"
DB_RAW_DATA_PATH="${GAUSSDB_PATH}/data"

LISTEN_ADDRESS=${POD_IP}

# 第一次安装部署，DB_DATA_PATH为pvc下目录，此时为空
if [ ! -f "${DB_DATA_PATH}/postgresql.conf" ];then
    if [ ! -d ${DB_DATA_PATH} ];then
        sudo /opt/script/change_permission.sh mkdir ${DB_DATA_PATH}
    fi

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

    sed -i "s/^#work_mem.*/work_mem = ${GAUSSDB_WORK_MEM}/g" ${DB_DATA_PATH}/postgresql.conf
    check_result "$?" "${LINENO} set work_mem:${GAUSSDB_WORK_MEM} for data/postgresql.conf"

    sed -i "s/^wal_buffers.*/wal_buffers = ${GAUSSDB_WAL_BUFFERS}/g" ${DB_DATA_PATH}/postgresql.conf
    check_result "$?" "${LINENO} set wal_buffers:${GAUSSDB_WAL_BUFFERS} for data/postgresql.conf"

    sed -i "s/^#wal_writer_delay.*/wal_writer_delay = ${WAL_WRITER_DELAY}/g" ${DB_DATA_PATH}/postgresql.conf
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
    config_maps=$(curl --cacert ${rootCAFile} -X GET -H "Authorization: Bearer $tokenFile" https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/dpa/configmaps/cluster-conf)
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
            config_maps=$(curl --cacert ${rootCAFile} -X GET -H "Authorization: Bearer $tokenFile" https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/dpa/configmaps/cluster-conf)
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

config_maps=$(curl --cacert ${rootCAFile} -X GET -H "Authorization: Bearer $tokenFile" https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/dpa/configmaps/common-conf)
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
        config_maps=$(curl --cacert ${rootCAFile} -X GET -H "Authorization: Bearer $tokenFile" https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/dpa/configmaps/common-conf)
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
  /usr/local/gaussdb/app/bin/gs_ctl stop -D /usr/local/gaussdb/data -U GaussDB -m fast
  check_result "$?" 'gs_ctl stop gaussdb (fast mode)'
  exit 0
}
trap term_handler SIGTERM

rotate_log() {
    while true; do
        /usr/sbin/logrotate ${GAUSSDB_PATH}/logrotate_gaussdb.conf -v -s ${GAUSSDB_PATH}/data/pg_log/logrotate.status | tee -a ${GAUSSDB_PATH}/data/pg_log/logrotate.log 2>&1
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

log_info "Starting gaussdb"
log_info "${LINENO} Start gaussdb with NORMAL"
gs_ctl start -D /usr/local/gaussdb/data

# 判断数据库是否连接成功
check_connection=$(gs_ctl status -D /usr/local/gaussdb/data | grep "server is running")


# 检查是否已经创建gaussdbremote，已存在则不再创建
username=$(gsql postgres -p 6432 -v ON_ERROR_STOP=on -Atc "select usename from pg_user where usename='gaussdbremote';")

# ON_ERROR_STOP=on时，return 0代表sql执行成功
if [ $? -eq 0 ] && [ "$username" != "gaussdbremote" ]; then
    log_info "${LINENO} Start to create gaussdbremote"
    log_info "${LINENO} Username:${username}"
    # 生成随机密码,密码中至少包含三种字符
    while true; do
      gaussdbremote_password=$(LD_LIBRARY_PATH=;openssl rand -base64 8)
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
        secrets=$(curl --cacert ${rootCAFile} -X GET -H "Authorization: Bearer $tokenFile" https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/dpa/secrets/common-secret)
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

DATASYNC_PATH="/opt/db_data/GaussDB_V5/GaussDB_T_1.9.0-DATASYNC/DataSync"

config_info=$(curl --cacert ${rootCAFile} -X GET -H "Authorization: Bearer $tokenFile" https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/dpa/configmaps/common-conf)
config_tag=$(echo ${config_info} | python3 -c "import sys, json;print(json.load(sys.stdin)['data'].get('gaussdbUpgrade'))")
secrets=$(curl --cacert ${rootCAFile} -X GET -H "Authorization: Bearer $tokenFile" https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/dpa/secrets/common-secret)
if [ "${config_tag}" != "true" ] && [ -n "$check_connection" ]; then
    generaldb_is_exist=$(echo $(gsql postgres -p 6432 -c "select usename from pg_user;" | grep "generaldb"))
    if [ -z "$generaldb_is_exist" ];then
        echo "start to create generaldb"
        # 创建普通数据库用户密码,密码中至少包含三种字符
        while true; do
          general_passwd=$(LD_LIBRARY_PATH=;openssl rand -base64 8)
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
            secrets=$(curl --cacert ${rootCAFile} -X GET -H "Authorization: Bearer $tokenFile" https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/dpa/secrets/common-secret)
            is_exist=$(echo "${secrets} " | python3 -c "import sys, json;print(json.load(sys.stdin)['data'].get('database.gaussdbPassword_V5'))")
            if [ ! -z "${is_exist}" ] && [ "${is_exist}" != "None" ];then
                log_info "${LINENO} Succeed to add generaldb secret"
                break
            fi
            log_error "${LINENO} Failed to add generaldb secret"
            sleep 5
        done

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
        pod_info=$(curl --cacert ${rootCAFile} -X GET -H "Authorization: Bearer $tokenFile" https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/dpa/pods)
        gaussdb_name=$(echo ${pod_info} | python3 -c "import sys, json, gaussdb_common;print(gaussdb_common.get_pod_name(json.load(sys.stdin)['items']))")
        gaussdb_info=$(curl --cacert ${rootCAFile} -X GET -H "Authorization: Bearer $tokenFile" https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/dpa/pods/${gaussdb_name})
        gaussdb_ip=$(echo ${gaussdb_info} | python3 -c "import sys, json;print(json.load(sys.stdin)['status'].get('podIP'))")
        if [ "${gaussdb_ip}" != "None" ];then
            log_info "${LINENO} Succeed to get gaussdb pod ip"
        else
            log_error "${LINENO} Failed to get gaussdb pod ip"
            exit 1
        fi

        # 交互式输入密码
        type=2
        expect auto_password_input.sh $GAUSSDB_PATH $DATASYNC_PATH $passwd $type

        # 写入参数至cfg.ini
        result=`python3 -c 'import gaussdb_common;print(gaussdb_common.write_import_cfg("'${gaussdb_ip}'"))'`

        # 配置白名单
        echo "host  all  all  ${gaussdb_ip}/32  sha256" >> ${DB_DATA_PATH}/pg_hba.conf
        gs_ctl reload -D ${GAUSSDB_PATH}/data
        check_result "$?" "${LINENO} gs_ctl reload -D ${GAUSSDB_PATH}/data"

    else
        echo "generaldb already exists"
    fi

    log_info "Succeed update GaussDB"
else
    log_info "Do not need to update GaussDB"
fi

(rotate_log &)

# wait forever
while true
do
  tail -f /dev/null & wait ${!}
done
