#!/bin/bash
source /usr/local/gaussdb/log.sh
MANAGE_IP=$POD_IP

function change_config_and_restart()
{
    action=$1
    stage=$2
    line_no=$(cat "/usr/local/gaussdb/data/pg_hba.conf" | grep -n "0.0.0.0/0           sha256" | awk -F':' '{print $1}')
    sed -i "${line_no}d" /usr/local/gaussdb/data/pg_hba.conf
    if [ "${action}" == "open" ];then
        # 允许远程连接
        echo "hostssl    all             all            0.0.0.0/0           sha256" >> /usr/local/gaussdb/data/pg_hba.conf
    else
        # 禁止远程连接
        echo "#hostssl    all             all            0.0.0.0/0           sha256" >>/usr/local/gaussdb/data/pg_hba.conf
    fi

    # 添加gaussdb pod ip到pg_hba.conf
    if [ ${stage} == "stage_import" ];then
        # 判断ip是否存在于pg_hba.conf中
        if ! grep -q "${MANAGE_IP}" /usr/local/gaussdb/data/pg_hba.conf; then
            # ip不存在，则添加
            echo "hostssl    all             all        ${MANAGE_IP}/18               sha256" >> /usr/local/gaussdb/data/pg_hba.conf
        fi
        line_no_manage_ip=$(cat "/usr/local/gaussdb/data/pg_hba.conf" | grep -n "${MANAGE_IP}" | awk -F':' '{print $1}')
        sed -i "${line_no_manage_ip}d" /usr/local/gaussdb/data/pg_hba.conf
        if [ "${action}" == "open" ];then
        # 不允许gaussdb pod远程连接
            echo "#hostssl    all             all        ${MANAGE_IP}/18               sha256" >> /usr/local/gaussdb/data/pg_hba.conf
        else
        # 仅允许gaussdb pod远程连接
            echo "hostssl    all             all        ${MANAGE_IP}/18               sha256" >>/usr/local/gaussdb/data/pg_hba.conf
        fi
    fi


    export GAUSSHOME=/usr/local/gaussdb/app;
    export LD_LIBRARY_PATH=/usr/local/gaussdb/app/lib;/usr/local/gaussdb/app/bin/gs_ctl restart -D /usr/local/gaussdb/data
    if [ $? -eq "0" ];then
        echo "success"
        exit 0
    else
        echo "fail"
        exit 1
    fi
}

function main() {
    if [ ${type} == "open" ]; then
        if [ ${stage} == "stage_import" ];then
            change_config_and_restart "open" "stage_import"
        else
            change_config_and_restart "open" "stage_drop"
        fi
    elif [ ${type} == "close" ]; then
        if [ ${stage} == "stage_import" ];then
            change_config_and_restart "close" "stage_import"
        else
            change_config_and_restart "close" "stage_drop"
        fi
    else
        log_error "param error"
        echo "fail"
        exit 1
    fi
}

type=$1
stage=$2
main