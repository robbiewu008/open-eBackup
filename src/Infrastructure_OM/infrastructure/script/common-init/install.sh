#!/bin/bash

echo "[$(date "+%Y-%m-%d %H:%M:%S")][${FUNCNAME[0]},${LINENO}] start to install init container"

log_file="/opt/OceanProtect/logs/${NODE_NAME}/${COMPONENT_NAME}/common-init/infrastructure-init.log"

function create_log_file()
{
    if [ ! -d "/opt/OceanProtect/logs/${NODE_NAME}" ]; then
        mkdir -p /opt/OceanProtect/logs/${NODE_NAME}
        chown 99:99 /opt/OceanProtect/logs/${NODE_NAME}
        echo "[$(date "+%Y-%m-%d %H:%M:%S")][${FUNCNAME[0]},${LINENO}] make dir /opt/OceanProtect/logs/${NODE_NAME}"
    fi
    chmod 750 /opt/OceanProtect/logs/${NODE_NAME}
    if [ ! -d "/opt/OceanProtect/logs/${NODE_NAME}/${COMPONENT_NAME}" ]; then
        mkdir /opt/OceanProtect/logs/${NODE_NAME}/${COMPONENT_NAME}
        chown 99:99 /opt/OceanProtect/logs/${NODE_NAME}/${COMPONENT_NAME}
        echo "[$(date "+%Y-%m-%d %H:%M:%S")][${FUNCNAME[0]},${LINENO}] make dir /opt/OceanProtect/logs/${NODE_NAME}/${COMPONENT_NAME}"
    fi
    chmod 750 /opt/OceanProtect/logs/${NODE_NAME}/${COMPONENT_NAME}
    if [ ! -d "/opt/OceanProtect/logs/${NODE_NAME}/${COMPONENT_NAME}/common-init" ]; then
        mkdir /opt/OceanProtect/logs/${NODE_NAME}/${COMPONENT_NAME}/common-init
        chown 99:99 /opt/OceanProtect/logs/${NODE_NAME}/${COMPONENT_NAME}/common-init
        echo "[$(date "+%Y-%m-%d %H:%M:%S")][${FUNCNAME[0]},${LINENO}] make dir /opt/OceanProtect/logs/${NODE_NAME}/${COMPONENT_NAME}/common-init"
    fi
    chmod 750 /opt/OceanProtect/logs/${NODE_NAME}/${COMPONENT_NAME}/common-init
    if [ ! -f "${log_file}" ];then
        touch ${log_file}
        chown 99:99 ${log_file}
        echo "[$(date "+%Y-%m-%d %H:%M:%S")][${FUNCNAME[0]},${LINENO}] make dir ${log_file}"
    fi
    chmod 640 ${log_file}
}

function set_firewall()
{
    export PYTHONPATH=/usr/local/common-init/src
    export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:/usr/local/lib"
    cd /usr/local/common-init/src/app/service
    python3 adapt.py ${log_file}
}

function main()
{
    create_log_file
    # 校验log_file软连接导致提权
    if [[ -L "${log_file}" ]]; then
        echo "[ERROR (common_init install log_file:${log_file}), symbolic link is not allowed][${FUNCNAME[0]}][${LINENO}]"
        exit 1
    fi
    echo "[$(date "+%Y-%m-%d %H:%M:%S")][INFO][start to install init container][${FUNCNAME[0]}][${LINENO}]" >> ${log_file}
    set_firewall
    echo "[$(date "+%Y-%m-%d %H:%M:%S")][INFO][install init container successfully][${FUNCNAME[0]}][${LINENO}]" >> ${log_file}
}

main
