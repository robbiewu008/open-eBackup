#!/usr/bin/bash

managementIP=$1
nodeIP=$2
k8sVIP=$3
serviceVIP=$4
networkCard=$5

base_path="/home/kadmin"
SimbaOS_path="/opt/k8s/SimbaOS/package"
log="/home/install.log"

function install_container_service()
{
    echo "$(date) Start to install container service" >>${log}
    package_name=$(ls ${base_path} | grep SimbaOS | grep "tar.gz")
    if [ -n "${package_name}" ];then
        tar -zxvf ${base_path}/${package_name} -C ${base_path}
        rm -rf ${base_path}/${package_name}
        # 安装smartkube
        install ${base_path}/SimbaOS/action/smartkube /usr/bin
        if [ $? -ne "0" ];then
            echo "$(date) Failed to install smartkube" >>${log}
            exit 1
        fi

        # 预安装
        smartkube preinstall env --packagePath=${base_path}/SimbaOS --createSSHUser
        cp -rf  ${base_path}/SimbaOS/* ${SimbaOS_path}
        chown kadmin:kgroup ${base_path}
        chown -R kadmin:kgroup ${SimbaOS_path}
    fi

    # 安装
    ${SimbaOS_path}/repo/conf/scripts/appctl.py install --managementIP=${managementIP} --nodeIP=${nodeIP} --k8sVIP=${k8sVIP} --serviceVIP=${serviceVIP} --interface=${networkCard}
    if [ $? -ne "0" ];then
        echo "$(date) Failed to install container service" >>${log}
        exit 1
    fi
    echo "$(date) End to install container service" >>${log}
}

function main()
{
    kubectl get pod -A
    if [ $? -ne "0" ];then
        install_container_service
    fi
}

main