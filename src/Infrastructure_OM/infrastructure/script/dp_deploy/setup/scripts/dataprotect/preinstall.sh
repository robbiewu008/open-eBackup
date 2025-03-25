#!/bin/bash

cd $(dirname ${BASH_SOURCE[0]})
source ../common.sh

IMAGE_NAME=$1
DEPLOY_TYPE=$2

IMAGE_PATH=${PACKAGE_BASE_PATH}/`basename ${IMAGE_NAME} .tgz`

cd ${IMAGE_PATH}

agent_client_package=$(cat manifest.yml | grep ClientPackageName | awk '{print $2}')
if [ ! -f ${agent_client_package} ]; then
    echo "Failed to get agent client package name from manifest.yml"
    exit 1
fi


function is_e6000_primary() {
    network_config_file=/opt/network/network_config.ini
    role=$(cat "$network_config_file" | grep role | sed 's/role=//')
    if [[ ! "$role" =~ management ]]; then
        return 1
    fi
    om_ip=$(cat "$network_config_file" | grep om_ip | sed 's/om_ip=//')
    manager_remote_ip=$(cat "$network_config_file" | grep manager_remote_ip | sed 's/manager_remote_ip=//')

    if [ "$om_ip" == "$manager_remote_ip" ]; then
        return 0
    fi
    return 1
}


if [ "$DEPLOY_TYPE" == "d7" ]; then
    # All nodes can access pm_nas2, so only then primary node needs to copy the file
    if is_e6000_primary; then
        mkdir -p /opt/pm_nas2
        if ! mountpoint -q /opt/pm-as2; then
            timeout 10 mount -t nfs -o nosuid,nodev,noexec,nolock,soft 127.0.0.1:/pm-nas2 /opt/pm_nas2
        fi
        rm -rf "/opt/pm_nas2/agent/client"
        mkdir -p "/opt/pm_nas2/agent/client"
        cp "${agent_client_package}" "/opt/pm_nas2/agent/client"
        chown 99:99 /opt/pm_nas2/agent -hR
        chmod 440 "/opt/pm_nas2/agent/client/${agent_client_package}"
        umount -l /opt/pm_nas2
    fi

    # 安装完simbaos之后，会将selinux设置为disabled，这里需要设置selinux修为permissive，
    # 避免pacific启动IMA服务依赖selinux，导致系统重启失败
    set_selinux 'permissive'
elif [ "$DEPLOY_TYPE" == "d8" ]; then
    # E1000
    rm -rf "/opt/DataBackup/pm_nas2/agent/client"
    mkdir -p "/opt/DataBackup/pm_nas2/agent/client"
    cp "${agent_client_package}" "/opt/DataBackup/pm_nas2/agent/client"
    chown 99:99 /opt/DataBackup/pm_nas2 -hR
    chmod 440 "/opt/DataBackup/pm_nas2/agent/client/${agent_client_package}"
else
    echo "Unknow deploy type $DEPLOY_TYPE"
    exit 1
fi

isula load -i *.tar.xz