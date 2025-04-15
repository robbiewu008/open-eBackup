#!/bin/bash

OLD_DATA_PATH="/opt/db_data"
DATA_PATH="/opt/db_data/GaussDB_V5"
RAW_DATA_PATH="/usr/local/gaussdb/data"
HA_DATA_PATH="/opt/third_data/ha"
RAW_HA_DATA_PATH="/usr/local/ha"
LOG_PATH="/opt/OceanProtect/logs/${NODE_NAME}/infrastructure/gaussdb"
HA_LOG_PATH="/opt/OceanProtect/logs/${NODE_NAME}/infrastructure/ha"
GAUSS_USER=GaussDB

function log_error()
{
    if [ $# -ne 2 ];then
        echo "log error param is error"
        exit 1
    fi

    # 写日志降权处理
    err_msg=[`date +"%Y-%m-%d %H:%M:%S"`][ERROR][$3][${GAUSS_USER}][$2][$1]
    echo ${err_msg}
    su - ${GAUSS_USER} -s /bin/bash -c "echo '${err_msg}' >>${LOG_FILE}"
}

function create_dir()
{
    local dir_name="$1"
    if [ "${dir_name}" != "${LOG_PATH}" ] && [ "${dir_name}" != "${HA_LOG_PATH}" ] && [ "${dir_name}" != "${HA_LOG_PATH}/scriptlog" ] && [ "${dir_name}" != "${HA_LOG_PATH}/core" ] && [ "${dir_name}" != "${HA_LOG_PATH}/runlog" ] && [ "${dir_name}" != "${DATA_PATH}" ];then
        log_error ${FUNCNAME[0]} ${LINENO} "create dir ${dir_name} not allowed"
        return 1
    fi
    mkdir -p ${dir_name}
}

function change_owner()
{
    local dir_name="$1"
    if [ -L "${dir_name}" ];then
        log_error ${FUNCNAME[0]} ${LINENO}  "symbolic link not allowed"
        return 1
    fi
    if [[ "${dir_name}" =~ "${HA_LOG_PATH}"* ]]; then
        chown GaussDB:nobody ${dir_name} -h -R
        return $?
    fi
    if [[ "${dir_name}" =~ "${HA_DATA_PATH}"* ]]; then
        chown nobody:nobody ${dir_name} -h -R
        return $?
    fi
    if [ "${dir_name}" != "${LOG_PATH}" ] && [ "${dir_name}" != "${OLD_DATA_PATH}" ] && [ "${dir_name}" != "${DATA_PATH}" ] && [ "${dir_name}" != "${DATA_PATH}/lost+found" ] && [ "${dir_name}" != "${DATA_PATH}/gaussdb_backup" ] && [ "${dir_name}" != "${LOG_PATH}/install_gaussdb.log" ];then
        log_error ${FUNCNAME[0]} ${LINENO}  "chown dir ${dir_name} not allowed"
        return 1
    fi
    if [ "${dir_name}" == "${LOG_PATH}/install_gaussdb.log" ];then
        chown GaussDB:nobody ${dir_name} -h
    else
        chown GaussDB:dbgrp ${dir_name} -h
    fi
}

function mount_dir()
{
    local ori_dir="$1"
    local dest_dir="$2"
    if [ -L "${ori_dir}" ] || [ -L "${dest_dir}" ];then
        log_error ${FUNCNAME[0]} ${LINENO}  "symbolic link not allowed"
        return 1
    fi
    if [ "${ori_dir}" == "${DATA_PATH}" ] && [ "${dest_dir}" == "${RAW_DATA_PATH}" ];then
        mount --bind -o noexec -o nosuid "${ori_dir}" "${dest_dir}"
    elif [ "${ori_dir}" == "/tmp/archive" ] && [ "${dest_dir}" == "/usr/local/gaussdb/archive" ];then
        mount --bind -o noexec -o nosuid "${ori_dir}" "${dest_dir}"
    elif [ "${ori_dir}" == "/tmp/gaussdb-start" ] && [ "${dest_dir}" ==  "/usr/local/gaussdb/start-script/gaussdb" ];then
        # gaussdb-start需要执行权限，不能添加 -o noexec -o nosuid参数
        mount --bind "${ori_dir}" "${dest_dir}"
    elif [[ "${ori_dir}" =~ "${HA_DATA_PATH}"* ]] && [[ "${dest_dir}" =~ "${RAW_HA_DATA_PATH}"* ]];then
        mount --bind -o nosuid "${ori_dir}" "${dest_dir}"
    else
        echo "mount ${ori_dir} ${dest_dir} not allowed"
        return 1
    fi
}

function chmod_path() {
    local chmod_type="$1"
    local permissions="$2"
    local dir_name="$3"
    if [[ "${dir_name}" =~ "${RAW_HA_DATA_PATH}"* ]]; then
        if [ "$chmod_type" == "recur" ]; then
            chmod ${permissions} -R ${dir_name}
        else
            chmod ${permissions} ${dir_name}
        fi
        return $?
    elif [[ "${dir_name}" =~ "${HA_DATA_PATH}"* ]]; then
        if [ "$chmod_type" == "recur" ]; then
            chmod ${permissions} -R ${dir_name}
        else
            chmod ${permissions} ${dir_name}
        fi
        return $?
    else
        echo "chmod ${dir_name} not allowed"
        return 1
    fi
}

if [[ ! "${NODE_NAME}" =~ ^[a-zA-Z0-9]([a-zA-Z0-9._-]*[a-zA-Z0-9])?$ ]]; then
  echo "NODE_NAME ${NODE_NAME} Invalid"
  exit 1
fi

type="$1"
if [ -z "${type}" ];then
    echo "type error"
    exit 1
fi
if [ "${type}" == "mkdir" ];then
    # mkdir path
    if [ $# -ne 2 ];then
        echo "param is error"
        exit 1
    fi
    create_dir "$2"
elif [ "${type}" == "chown" ];then
    if [ $# -ne 2 ];then
        echo "param is error"
        exit 1
    fi
    change_owner "$2"
elif [ "${type}" == "mount" ];then
    if [ $# -ne 3 ];then
        echo "param is error"
        exit 1
    fi
    mount_dir "$2" "$3"
elif [ "${type}" == "chmod" ];then
    if [ $# -ne 4 ];then
        echo "param is error"
        exit 1
    fi
    chmod_path "$2" "$3" "$4"
else
    echo "param is error"
    exit 1
fi