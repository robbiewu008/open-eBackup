#!/bin/bash
source /opt/script/common_sudo.sh

MASTER_KS='/opt/OceanProtect/protectmanager/kmc/master.ks'
BACKUP_KS='/kmc_conf/..data/backup.ks'
MANAGE_IP=$POD_IP
BACKUP_INFRA_PATH='/opt/OceanProtect/protectmanager/sysbackup/backup/infrastructure/'
RECOVER_INFRA_PATH='/opt/OceanProtect/protectmanager/sysbackup/recovery/infrastructure/'
# 数据库列表
DATABASE_ARRY=('admindb' 'anon_policy' 'anti_ransomware' 'applicationdb' 'archivedb' 'base_parser' 'datamoverdb'
            'dee_scheduler' 'dme_unified' 'generalschedulerdb' 'indexer' 'postgres' 'protect_manager' 'replicationdb')
SQL_ARRY=()
log_info "" ${LINENO} "start manage data script"

# export {general_user} {general_pwd_field} {datname} {file_path}
if [ $# -eq 5 ]; then
    type=$1
    user_name=$2
    pwd_field=$3
    database=$4
    path=$5
else
    log_info "" ${LINENO} "param is error"
    exit 1
fi

function gen_sql_path_arry() {
    for ((i=0; i<${#DATABASE_ARRY[@]}; i++))
    do
        database_name=${DATABASE_ARRY[${i}]}
        backup_sql="${BACKUP_INFRA_PATH}gaussdb_${database_name}_backup.sql"
        SQL_ARRY+=("${backup_sql}")
        recover_sql="${RECOVER_INFRA_PATH}gaussdb_${database_name}_backup.sql"
        SQL_ARRY+=("${recover_sql}")
    done
}

function check_path_common()
{
    gen_sql_path_arry
    local path="$1"
    if [ -z "${path}" ]; then
        return 1
    fi
    if [ -L "${path}" ]; then
        log_error ${FUNCNAME[0]} ${LINENO} "path:${path} is a symbolic link."
        return 1
    fi
    # 校验路径白名单
    if [[ ! "${SQL_ARRY[*]}" =~ "${path}" ]]; then
        log_error ${FUNCNAME[0]} ${LINENO} "path:${path} not in sql arry path list."
        echo "sql path not in sql arry list"
        return 1
    fi
    filepat='[|;&$><`!+]'
    result=$(echo "${path}" | grep ${filepat})
    if [ ! -z "${result}" ]; then
        log_error ${FUNCNAME[0]} ${LINENO} "path:${path} contains special characters."
        return 1
    fi
    result=$(echo "${path}" | grep -w "\..")
    if [ ! -z "${result}" ]; then
        log_error ${FUNCNAME[0]} ${LINENO} "path:${path} contains special characters."
        return 1
    fi
}

function export_databases() {
    export LD_LIBRARY_PATH='/usr/local/gaussdb/app/lib':$LD_LIBRARY_PATH
    mkdir -p $(dirname ${path})
    chown -hR 99:99 $(dirname ${path})
    export PYTHONPATH=/opt/script
    python3 /opt/script/manage_db_data.pyc ${MASTER_KS} ${BACKUP_KS} ${type} ${database} ${user_name} ${pwd_field} ${path} "gaussdb"
    if [ "$?" != "0" ]; then
        log_error ${FUNCNAME[0]} ${LINENO} "export database:${database} error."
        exit 1
    fi
    chown -h 99:99 ${path}
    log_info ${FUNCNAME[0]} ${LINENO} "export database:${database} success."
}

function import_databases() {
    export LD_LIBRARY_PATH='/usr/local/gaussdb/app/lib':$LD_LIBRARY_PATH
    export PYTHONPATH=/opt/script
    python3 /opt/script/manage_db_data.pyc ${MASTER_KS} ${BACKUP_KS} ${type} ${database} ${user_name} ${pwd_field} ${path} ${MANAGE_IP}
    if [ "$?" != "0" ]; then
        log_error ${FUNCNAME[0]} ${LINENO} "import database:${database} error."
        echo "import database:fail"
        exit 1
    fi
    echo "import database:success"
    log_info ${FUNCNAME[0]} ${LINENO} "import database:${database} success."
}

function main() {
    check_path_common "${path}"
    if [ "$?" != "0" ]; then
        log_error ${FUNCNAME[0]} ${LINENO} "param error"
        exit 1
    fi

    if [ ${type} == "export" ]; then
        export_databases
    else
        import_databases
    fi
}

main
