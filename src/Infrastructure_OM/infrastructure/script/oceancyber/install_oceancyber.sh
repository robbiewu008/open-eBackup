#!/usr/bin/bash

base_path="/home/kadmin"
SimbaOS_path="/opt/k8s/SimbaOS/package/"
image_path="/opt/k8s/image"
chart_path="/opt/k8s/chart"
oceancyber_path="/opt/huawei/oceancyber"
install_script_path="/opt/k8s/chart/install_script"
sysbackup_recovery_script="/sysbackup_recovery.sh"
bond_100GE_network_script="/bond_100GE_network.sh"
config_100GE_network_script="/config_100GE_network.sh"
init_network_script="/init_network.py"
sts_num=5

managementIP="192.168.128.101"
nodeIP="172.16.128.101"
k8sVIP="172.16.128.101"
serviceVIP="172.16.128.102"

function uninstall()
{
    echo "$(date) Start to helm uninstall" >>/home/install.log
    su - root -c "helm uninstall dataprotect -n dpa &"
    sleep 30
    su - root -c "kubectl delete ns dpa" >>/home/install.log 2>&1
    echo "$(date) End to helm uninstall" >>/home/install.log
    rm /opt/cyberengine/* -rf
}

function clear_resource()
{
    rm -rf /opt/cyberengine/agent_data/*
    rm -rf /opt/cyberengine/comm_data/*
    rm -rf /opt/cyberengine/db_data/*
    rm -rf /opt/cyberengine/d-data/*
    rm -rf /opt/cyberengine/mgt_cfg_data/*
    rm -rf /opt/cyberengine/third_data/*
}

function install_application()
{
    # 加载镜像
    ls ${base_path} | grep OceanCyber | grep image | grep tgz
    if [ $? -eq 0 ];then
        tar -zxvf ${base_path}/OceanCyber_*_image_*.tgz -C ${image_path}
        /usr/bin/xz -dk ${image_path}/OceanCyber_*_image_*.tar.xz
        rm ${image_path}/OceanCyber_*_image_*.tar.xz
        /opt/k8s/SimbaOS/package/repo/conf/scripts/imagectl.py --load ${image_path}/OceanCyber_*_image_*.tar --name OceanCyber
        if [ $? -ne "0" ];then
            echo "$(date) Failed to load images" >>/home/install.log
            exit 1
        fi
        rm ${base_path}/OceanCyber_*_chart_*.tgz
        rm ${base_path}/OceanCyber_*_image_*.tgz
    fi

    # 安装前先清理数据
    clear_resource
    # 安装应用
    su - root -c "kubectl get namespace dpa" >>/home/install.log 2>&1
    if [ $? -ne "0" ];then
        echo "$(date) Namespace not exist and create" >>/home/install.log
        su - root -c "kubectl create namespace dpa" >>/home/install.log 2>&1
        if [ $? -ne "0" ];then
            echo "$(date) Failed to create namespace dpa" >>/home/install.log
            exit 1
        fi
    fi
    su - root -c "helm install dataprotect ${chart_path}/OceanCyber-*.tgz --set global.gaussdbpwd=R2F1c3NkYl8xMjM= --set global.replicas=1 --set global.deploy_type=d5 -n dpa" >>/home/install.log 2>&1
    if [ $? -ne "0" ];then
        echo "$(date) Failed to helm install" >>/home/install.log
        su - root -c "helm list -A | grep -w 'dataprotect'" >>/home/install.log 2>&1
        if [ "$?" -eq 0 ];then
            uninstall
        fi
        exit 1
    fi
    COUNTER=0
    while [ ${COUNTER} -lt 360 ]; do
        is_ready=$(su - root -c "kubectl get sts -n dpa | grep -w '1/1' |wc -l")
        if [ ${is_ready} -eq "${sts_num}" ];then
            echo "$(date) Succeed to helm install" >>/home/install.log
            return
        fi
        let COUNTER++
        echo "waiting pod finish..."
        sleep 5
    done
    su - root -c "kubectl get pod -ndpa" >>/home/install.log 2>&1
    echo "$(date) Failed to helm install" >>/home/install.log
    uninstall
    exit 1
}

function install_ibma() {
    # 当前ibma是否已经存在，存在则跳过
    ibma_status=$(systemctl  status iBMA 2>&1)
    echo ${ibma_status} |grep -q 'not be found'
    if [ $? -ne 0 ];then
      echo "ibma already exists."
      return;
    fi

    # ibma安装包在 ${image_path} = /opt/k8s/image
    if [ $(ls ${image_path} |grep 'iBMA-' -c) -gt 0 ]; then
      echo "start install ibma"
      ibma_tar_name=$(su - root -c "ls ${image_path}/|grep iBMA- |head -n 1")
      # 解压安装包并更改默认端口8090/8091 为28090/28091
      su - root -c "tar -zxvf ${image_path}/${ibma_tar_name} -C ${image_path}/; sed -i 's/HTTP_PORT=8090/HTTP_PORT=28090/g' ${image_path}/iBMA2.0/install.sh;
      sed -i 's/SOCKET_PORT=8091/SOCKET_PORT=28091/g' ${image_path}/iBMA2.0/install.sh;"
      # 执行安装
      su - root -c "sh ${image_path}/iBMA2.0/install.sh -s --enable-iBMA_https=true"
      if [ $? -ne "0" ];then
        echo "$(date) Failed to install iBMA" >>/home/install.log
        exit 1
      fi
    fi
}

function install_container_service()
{
    echo "$(date) Start to install container service" >>/home/install.log
    package_name=$(ls ${base_path} | grep SimbaOS | grep "tar.gz")
    if [ -n "${package_name}" ];then
        tar -zxvf ${base_path}/${package_name} -C ${base_path}
        rm -rf ${base_path}/${package_name}
        mv ${base_path}/SimbaOS/* ${SimbaOS_path}
    fi
    chown -R kadmin:kgroup ${SimbaOS_path}

    # 安装smartkube
    install ${SimbaOS_path}/action/smartkube /usr/bin
    if [ $? -ne "0" ];then
        echo "$(date) Failed to install smartkube" >>/home/install.log
        exit 1
    fi

    # 安装
    install_cmd="${SimbaOS_path}/repo/conf/scripts/appctl.py install --managementIP=${managementIP} --nodeIP=${nodeIP} --k8sVIP=${k8sVIP} --serviceVIP=${serviceVIP}"
    su - kadmin -c "${install_cmd}"
    if [ $? -ne "0" ];then
        echo "$(date) Failed to install container service" >>/home/install.log
        exit 1
    fi
    echo "$(date) End to install container service" >>/home/install.log
}

function cp_scripts() {
    for script in "$@" ; do
        source_script=$install_script_path$script
        target_script=$oceancyber_path$script
        if [ -f "$source_script" ] && [ ! -L "$source_script" ]; then
          cp -fP "$source_script" $oceancyber_path
          chown 0:0 "$target_script"
          chmod 600 "$target_script"
        else
          echo "$(date) Failed copy '$source_script' to '$target_script'" >>/home/install.log
        fi
    done
}

function main()
{
    if [ -f "/opt/k8s/chart/install_script/verify_tool" ];then
        install /opt/k8s/chart/install_script/verify_tool /usr/bin/ && chmod 0750 /usr/bin/verify_tool
        chown root:kgroup /usr/bin/verify_tool
    fi
    kubectl get pod -A
    if [ $? -ne "0" ];then
        install_container_service
    fi
    if [ ! -d "${image_path}" ];then
        mkdir ${image_path}
    fi
    chown -R kadmin ${image_path}

    if [ ! -d "${chart_path}" ];then
        mkdir ${chart_path}
    fi

    if [ ! -d "${oceancyber_path}" ];then
        mkdir -p ${oceancyber_path}
    fi
    chmod 750 $oceancyber_path
    scripts=("$sysbackup_recovery_script" "$bond_100GE_network_script" "$config_100GE_network_script" "$init_network_script")
    cp_scripts "${scripts[@]}"

    chown -R kadmin ${chart_path}
    install_application
    install_ibma
    rm -rf /home/install.log
    echo "success"
    exit 0
}

main