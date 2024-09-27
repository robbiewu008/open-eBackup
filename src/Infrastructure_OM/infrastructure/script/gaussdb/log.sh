#!/bin/bash

GAUSSDB_PATH="/usr/local/gaussdb"
NODE_NAME=${NODE_NAME}
LOG_PATH="/opt/OceanProtect/logs/${NODE_NAME}/infrastructure/gaussdb"

function write_log_info()
{
    local datetime=`date +"%Y-%m-%d %H:%M:%S"`
    local curUser=`whoami`
    local curScript=$1
    local loglevel=$2
    local message=$3

    echo "[${datetime}][${loglevel}][${message}][${curScript}][${curUser}]"
    echo "[${datetime}][${loglevel}][${message}][${curScript}][${curUser}]" >> "${LOG_PATH}/install_gaussdb.log"
}

function log_info()
{
    if [[ $# != 1 ]];then
        echo "[$(date "+%Y-%m-%d %H:%M:%S")]The number of parameters is incorrect."
        exit 1
    fi

    write_log_info "${0##*/}" "INFO" "$@"
}

function log_error()
{
    if [[ $# != 1 ]];then
        echo "[$(date "+%Y-%m-%d %H:%M:%S")]The number of parameters is incorrect."
        exit 1
    fi

    write_log_info "${0##*/}" "ERROR" "$@"
}

function check_result()
{
    if [ "$1" != "0" ];then
        log_error "Exec cmd:$2 failed."
    else
        log_info "Exec cmd:$2 success."
    fi
}