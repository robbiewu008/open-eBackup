#!/usr/bin/bash

base_path="/home/kadmin"
image_path="/opt/k8s/image"
chart_path="/opt/k8s/chart"
log="/home/install.log"

sts_num=7

function uninstall()
{
    echo "$(date) Start to helm uninstall" >>${log}
    su - root -c "helm uninstall databackup -n dpa &"
    sleep 30
    su - root -c "kubectl delete ns dpa" >>${log} 2>&1
    echo "$(date) End to helm uninstall" >>${log}
    rm -rf /opt/DataBackup/*
}

function install_application()
{
    # 加载包
    ls ${base_path} | grep OceanProtect | grep image | grep tgz
    if [ $? -eq 0 ];then
        # 加载配置文件
        ls ${base_path} | grep OceanProtect | grep chart | grep tgz
        if [ $? -ne 0 ];then
            echo "[ERROR] $(date) chart file is not found in ${base_path}"
            exit 1
        fi
        tar -zxvf ${base_path}/OceanProtect_*_chart_*.tgz -C ${chart_path}

        ls ${chart_path} | grep databackup | grep tgz
        if [ $? -ne 0 ];then
            echo "[ERROR] $(date) databackup file is not found in ${chart_path}"
            exit 1
        fi
        tar -zxvf ${chart_path}/databackup-*.tgz -C ${chart_path}

        # 加载镜像
        tar -zxvf ${base_path}/OceanProtect_*_image_*.tgz -C ${image_path}
        su - root -c "isula load -i ${image_path}/OceanProtect_*_image_*.tar.xz" >>${log} 2>&1
        if [ $? -ne "0" ];then
            echo "$(date) Failed to load images" >>${log}
            exit 1
        fi
    fi

    # 安装前先清理数据
    rm -rf /opt/DataBackup/*
    # 安装应用
    su - root -c "kubectl get namespace dpa" >>${log} 2>&1
    if [ $? -ne "0" ];then
        echo "$(date) Namespace not exist and create" >>${log}
        su - root -c "kubectl create namespace dpa" >>${log} 2>&1
        if [ $? -ne "0" ];then
            echo "$(date) Failed to create namespace dpa" >>${log}
            exit 1
        fi
    fi
    su - root -c "helm install databackup ${chart_path}/databackup --set global.gaussdbpwd=R2F1c3NkYl8xMjM= --set global.replicas=1 --set global.deploy_type=d8 -n dpa" >>${log} 2>&1
    if [ $? -ne "0" ];then
        echo "$(date) Failed to helm install" >>${log}
        su - root -c "helm list -A | grep -w 'databackup'" >>${log} 2>&1
        if [ "$?" -eq 0 ];then
            uninstall
        fi
        exit 1
    fi
    COUNTER=0
    while [ ${COUNTER} -lt 360 ]; do
        is_ready=$(su - root -c "kubectl get sts -n dpa | grep -w '1/1' |wc -l")
        if [ ${is_ready} -eq "${sts_num}" ];then
            echo "$(date) Succeed to helm install" >>${log}
            return
        fi
        let COUNTER++
        echo "waiting pod finish..."
        sleep 5
    done
    su - root -c "kubectl get pod -n dpa" >>${log} 2>&1
    echo "$(date) Failed to helm install" >>${log}
    uninstall
    exit 1
}

function main()
{
    if [ ! -d "${image_path}" ];then
        mkdir ${image_path}
    fi
    chown -R kadmin ${image_path}

    if [ ! -d "${chart_path}" ];then
        mkdir ${chart_path}
    fi
    chown -R kadmin ${chart_path}

    install_application
    rm -rf ${log}
    echo "success"
    exit 0
}

main
