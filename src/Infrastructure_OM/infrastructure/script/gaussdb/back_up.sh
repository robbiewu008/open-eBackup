#!/bin/bash

db_path="/opt/db_data/GaussDB_V5"
back_paths="/opt/db_data/GaussDB_V5/gaussdb_backup/"
log_path="/opt/db_data/GaussDB_V5/pg_log/back_up.log"
INTERVAL_TIME=5
echo $1

if [ ! -d ${db_path} ];then
    echo "[`date "+%Y-%m-%d %H:%M:%S"`][ERROR][Path:${db_path} not exist][back_up.sh][$LINENO]" >> $log_path
    exit 1
fi

if [ $1 == "backup" ];then
    echo "[`date "+%Y-%m-%d %H:%M:%S"`][INFO][Backup db_data][back_up.sh][$LINENO]" >> $log_path
    ori_dir=${db_path}
    dest_dir=${back_paths}
elif [ $1 == "recover" ];then
    echo "[`date "+%Y-%m-%d %H:%M:%S"`][INFO][Recover db_data][back_up.sh][$LINENO]" >> $log_path
    dest_dir=$db_path
    ori_dir=${back_paths}
    if [ ! -d ${ori_dir} ];then
        echo "[`date "+%Y-%m-%d %H:%M:%S"`][ERROR][Can not recover, ${ori_dir} not exist][back_up.sh][$LINENO]" >> $log_path
        echo "recover failed"
        exit 1
    fi
else
    echo "[`date "+%Y-%m-%d %H:%M:%S"`][ERROR][Param error][back_up.sh][$LINENO]" >> $log_path
    echo "param error"
    exit 1
fi

action=$1

stop_service()
{
    # 设置停止服务标记
    touch /tmp/backup_flag
    # 停止gaussdb服务
    gs_ctl stop -D /usr/local/gaussdb/data
    sleep ${INTERVAL_TIME}
    # 查询服务是否停止成功
    result=$(gs_ctl status -D /usr/local/gaussdb/data)
    echo ${result} | grep "no server running"
    if [ $? -eq 0 ];then
        echo "[`date "+%Y-%m-%d %H:%M:%S"`][INFO][Succeed to stop gaussdb service][back_up.sh][$LINENO]" >> $log_path
        return 0
    fi
    echo "[`date "+%Y-%m-%d %H:%M:%S"`][ERROR][Failed to stop gaussdb service][back_up.sh][$LINENO]" >> $log_path
    return 1
}

cp_func()
{
    echo "[`date "+%Y-%m-%d %H:%M:%S"`][INFO][start to ${action}][back_up.sh][$LINENO]" >> $log_path
    if [ -d ${dest_dir} ];then
        cd ${dest_dir}
        rm -rf `ls ${dest_dir} | grep -v gaussdb_backup | grep -v pg_log| xargs`
        echo "[`date "+%Y-%m-%d %H:%M:%S"`][INFO][rm ${dest_dir} for ${action} db_data][back_up.sh][$LINENO]" >> $log_path
    else
        mkdir ${dest_dir}
        echo "[`date "+%Y-%m-%d %H:%M:%S"`][INFO][mkdir ${dest_dir} for ${action} db_data][back_up.sh][$LINENO]" >> $log_path
    fi
    cd ${ori_dir}
    cp -rpf `ls ${ori_dir} | grep -v gaussdb_backup | grep -v pg_log| xargs` ${dest_dir}
}

start_service()
{
    if [ -f "/tmp/backup_flag" ];then
        rm /tmp/backup_flag
    fi
    gs_ctl start -D /usr/local/gaussdb/data
    echo "[`date "+%Y-%m-%d %H:%M:%S"`][INFO][start gaussdb service][back_up.sh][$LINENO]" >> $log_path
}

# 备份或恢复前先停止gaussdb服务
source /home/GaussDB/.bashrc
stop_service
if [ $? -eq 0 ];then
    cp_func
    # 备份或恢复后拉起gaussdb服务
    start_service
    echo "[`date "+%Y-%m-%d %H:%M:%S"`][INFO][Backup gaussdb succeed][back_up.sh][$LINENO]" >> $log_path
    echo "success"
    exit 0
else
    start_service
    echo "[`date "+%Y-%m-%d %H:%M:%S"`][ERROR][Backup gaussdb failed][back_up.sh][$LINENO]" >> $log_path
    echo "fail"
    exit 1
fi