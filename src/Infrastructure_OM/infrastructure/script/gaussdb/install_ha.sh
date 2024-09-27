#!/bin/bash

source ./log.sh
HA_LOG_PATH="/opt/OceanProtect/logs/${NODE_NAME}/infrastructure/ha"
HA_PATH="/usr/local/ha"
HA_MOUNT_PATH="/opt/third_data/ha/"

if [ ! -d ${HA_LOG_PATH} ];then
    sudo /opt/script/change_permission.sh mkdir ${HA_LOG_PATH}
    if [ "$?" != "0" ];then
        echo "mkdir ${HA_LOG_PATH} failed"
        exit 1
    fi
fi
sudo /opt/script/change_permission.sh chown ${HA_LOG_PATH}
if [ "$?" != "0" ];then
    echo "chown ${HA_LOG_PATH} failed"
    exit 1
fi
chmod 770 ${HA_LOG_PATH}
check_result "$?" "${LINENO} chmod 770 ${HA_LOG_PATH}"

# 清理残留nfs文件
find ${LOG_PATH} -type f -name ".nfs*" -delete
check_result "$?" "${LINENO} find ${LOG_PATH} -type f -name \".nfs*\" -delete"

if [ ! -d ${HA_LOG_PATH}/core ];then
    sudo /opt/script/change_permission.sh mkdir ${HA_LOG_PATH}/core
    if [ "$?" != "0" ];then
        echo "mkdir ${HA_LOG_PATH}/core failed"
        exit 1
    fi
fi
sudo /opt/script/change_permission.sh chown ${HA_LOG_PATH}/core
if [ "$?" != "0" ];then
    echo "chown ${HA_LOG_PATH}/core failed"
    exit 1
fi
chmod 770 ${HA_LOG_PATH}/core
check_result "$?" "${LINENO} chmod 770 ${HA_LOG_PATH}/core"

if [ ! -d ${HA_LOG_PATH}/runlog ];then
    sudo /opt/script/change_permission.sh mkdir ${HA_LOG_PATH}/runlog
    if [ "$?" != "0" ];then
        echo "mkdir ${HA_LOG_PATH}/runlog failed"
        exit 1
    fi
fi
sudo /opt/script/change_permission.sh chown ${HA_LOG_PATH}/runlog
if [ "$?" != "0" ];then
    echo "chown ${HA_LOG_PATH}/runlog failed"
    exit 1
fi
chmod 770 ${HA_LOG_PATH}/runlog
check_result "$?" "${LINENO} chmod 770 ${HA_LOG_PATH}/runlog"

touch ${HA_LOG_PATH}/runlog/ha.log
check_result "$?" "${LINENO} touch ${HA_LOG_PATH}/runlog/ha.log"
chmod 770 ${HA_LOG_PATH}/runlog/ha.log
check_result "$?" "${LINENO} chmod 770 ${HA_LOG_PATH}/runlog/ha.log"

if [ ! -d ${HA_LOG_PATH}/scriptlog ];then
    sudo /opt/script/change_permission.sh mkdir ${HA_LOG_PATH}/scriptlog
    if [ "$?" != "0" ];then
        echo "mkdir ${HA_LOG_PATH}/scriptlog failed"
        exit 1
    fi
fi
sudo /opt/script/change_permission.sh chown ${HA_LOG_PATH}/scriptlog
if [ "$?" != "0" ];then
    echo "chown ${HA_LOG_PATH}/scriptlog failed"
    exit 1
fi
chmod 770 ${HA_LOG_PATH}/scriptlog
check_result "$?" "${LINENO} chmod 770 ${HA_LOG_PATH}/scriptlog"

touch ${HA_LOG_PATH}/scriptlog/ha.log
check_result "$?" "${LINENO} touch ${HA_LOG_PATH}/scriptlog/ha.log"
chmod 770 ${HA_LOG_PATH}/scriptlog/ha.log
check_result "$?" "${LINENO} chmod 770 ${HA_LOG_PATH}/scriptlog/ha.log"
touch ${HA_LOG_PATH}/scriptlog/ha_monitor.log
check_result "$?" "${LINENO} touch ${HA_LOG_PATH}/scriptlog/ha_monitor.log"
chmod 770 ${HA_LOG_PATH}/scriptlog/ha_monitor.log
check_result "$?" "${LINENO} chmod 770 ${HA_LOG_PATH}/scriptlog/ha_monitor.log"

sudo /opt/script/change_permission.sh mkdir $HA_MOUNT_PATH
chmod 770 $HA_MOUNT_PATH

deploy_ha()
{
    if [ ! -d $1 ];then
        # 在pvc新建目录以待存放
        mkdir -p $1
        # 赋予父目录770权限
        chmod 770 $1
        # 修改属主为nobody:nobody
        sudo /opt/script/change_permission.sh chown $1
        # 拷贝安装目录下的内容到pvc，通配避免父目录拷贝，同时保留子目录权限
        cp -Rp $2/* $1
        log_info "Exec ${LINENO} Copy conf finished, start to mount path."
    else
        log_info "Exec ${LINENO} HA has been installed, start to mount path."
    fi
    # 挂载老目录到新的目录中
    sudo /opt/script/change_permission.sh mount $1 $2
    if [ "$?" != "0" ];then
        log_error "Exec ${LINENO} mount path $1 to $2 failed"
        exit 1
    fi
}

deploy_ha /opt/third_data/ha/local /usr/local/ha/local
deploy_ha /opt/third_data/ha/module /usr/local/ha/module

# 整体修改/opt/third_data/ha/module目录的属主
sudo /opt/script/change_permission.sh chown /opt/third_data/ha/module
sudo /opt/script/change_permission.sh chown /opt/third_data/ha/local

# 整体修改目录权限
sudo /opt/script/change_permission.sh chmod recur 750 /opt/third_data/ha
check_result "$?" "${LINENO} chmod 750 /opt/third_data/ha"
sudo /opt/script/change_permission.sh chmod recur 550 /opt/third_data/ha/module/haarb/script
check_result "$?" "${LINENO} chmod 550 /opt/third_data/ha/module/haarb/script"
sudo /opt/script/change_permission.sh chmod recur 550 /opt/third_data/ha/module/haarb/plugin/script
check_result "$?" "${LINENO} chmod 550 /opt/third_data/ha/module/haarb/plugin/script"
sudo /opt/script/change_permission.sh chmod recur 550 /opt/third_data/ha/module/hacom/script
check_result "$?" "${LINENO} chmod 550 /opt/third_data/ha/module/hacom/script"
sudo /opt/script/change_permission.sh chmod recur 550 /opt/third_data/ha/module/hacom/plugin/script
check_result "$?" "${LINENO} chmod 550 /opt/third_data/ha/module/hacom/plugin/script"
sudo /opt/script/change_permission.sh chmod recur 550 /opt/third_data/ha/module/hacom/tools
check_result "$?" "${LINENO} chmod 550 /opt/third_data/ha/module/hacom/tools"
sudo /opt/script/change_permission.sh chmod recur 550 /opt/third_data/ha/module/hacom/bin
check_result "$?" "${LINENO} chmod 550 /opt/third_data/ha/module/hacom/bin"
sudo /opt/script/change_permission.sh chmod recur 550 /opt/third_data/ha/module/hacom/lib
check_result "$?" "${LINENO} chmod 550 /opt/third_data/ha/module/hacom/lib"
sudo /opt/script/change_permission.sh chmod recur 550 /opt/third_data/ha/module/hamon/script
check_result "$?" "${LINENO} chmod 550 /opt/third_data/ha/module/hamon/script"
sudo /opt/script/change_permission.sh chmod recur 550 /opt/third_data/ha/module/hamon/plugin/script
check_result "$?" "${LINENO} chmod 550 /opt/third_data/ha/module/hamon/plugin/script"
sudo /opt/script/change_permission.sh chmod recur 550 /opt/third_data/ha/module/hamon/bin
check_result "$?" "${LINENO} chmod 550 /opt/third_data/ha/module/hamon/bin"
sudo /opt/script/change_permission.sh chmod recur 550 /opt/third_data/ha/module/harm/script
check_result "$?" "${LINENO} chmod 550 /opt/third_data/ha/module/harm/script"
sudo /opt/script/change_permission.sh chmod recur 550 /opt/third_data/ha/module/harm/plugin/script
check_result "$?" "${LINENO} chmod 550 /opt/third_data/ha/module/harm/plugin/script"
sudo /opt/script/change_permission.sh chmod recur 550 /opt/third_data/ha/module/hasync/script
check_result "$?" "${LINENO} chmod 550 /opt/third_data/ha/module/hasync/script"
sudo /opt/script/change_permission.sh chmod recur 550 /opt/third_data/ha/module/hasync/plugin/script
check_result "$?" "${LINENO} chmod 550 /opt/third_data/ha/module/hasync/plugin/script"
sudo /opt/script/change_permission.sh chmod non-recur 640 /opt/third_data/ha/module/haarb/conf/haarb.xml
check_result "$?" "${LINENO} chmod 640 /opt/third_data/ha/module/haarb/conf/haarb.xml"
sudo /opt/script/change_permission.sh chmod non-recur 640 /opt/third_data/ha/module/harm/conf/harm.xml
check_result "$?" "${LINENO} chmod 640 /opt/third_data/ha/module/harm/conf/harm.xml"
sudo /opt/script/change_permission.sh chmod non-recur 640 "/opt/third_data/ha/module/harm/plugin/conf/*"
check_result "$?" "${LINENO} chmod 640 /opt/third_data/ha/module/harm/plugin/conf/*"
sudo /opt/script/change_permission.sh chmod non-recur 640 /opt/third_data/ha/module/hasync/conf/hasync.xml
check_result "$?" "${LINENO} chmod 640 /opt/third_data/ha/module/hasync/conf/hasync.xml"
sudo /opt/script/change_permission.sh chmod non-recur 640 "/opt/third_data/ha/module/hasync/plugin/conf/*"
check_result "$?" "${LINENO} chmod 640 /opt/third_data/ha/module/hasync/plugin/conf/*"
sudo /opt/script/change_permission.sh chmod non-recur 640 /opt/third_data/ha/local/haarb/conf/haarb_local.xml
check_result "$?" "${LINENO} chmod 640 /opt/third_data/ha/local/haarb/conf/haarb_local.xml"
sudo /opt/script/change_permission.sh chmod non-recur 640 /opt/third_data/ha/local/hacom/conf/hacom_local.xml
check_result "$?" "${LINENO} chmod 640 /opt/third_data/ha/local/hacom/conf/hacom_local.xml"
sudo /opt/script/change_permission.sh chmod non-recur 750 "`find /opt/third_data/ha/module -type d`"
check_result "$?" "${LINENO} chmod 750 find /opt/third_data/ha/module -type d"

# 设置ha的日志路径
HA_LOG_PATH=/opt/OceanProtect/logs/${NODE_NAME}/infrastructure
G_SUDO_SCRIPT_PATH=/opt/script
sudo ${G_SUDO_SCRIPT_PATH}/ha_sudo.sh ha_config log $HA_LOG_PATH