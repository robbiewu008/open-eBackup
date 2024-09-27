#!/bin/bash

CURR_DIR=$(cd "$(dirname "$0")" && pwd)
export KUBECONFIG=/etc/rancher/k3s/k3s.yaml

function reloadDataProtection()
{
    local tarPath="$CURR_DIR/PM_Data_Protection_Service.tar.gz"
    if [[ ! -f $tarPath ]]; then
        echo "not find PM_Data_Protection_Service.tar.gz"
        return
    fi
    tar -xzf $tarPath
    docker build -t pm-protection-service:8.1.0 -f PM_Data_Protection_Service.dockerfile .
}

function reloadDataMover()
{
    local tarPath="$CURR_DIR/PM_DataMover_Access_Point.tar.gz"
    if [[ ! -f $tarPath ]]; then
        echo "not find PM_DataMover_Access_Point.tar.gz"
        return
    fi
    tar -xzf $tarPath
    docker build -t pm-dm-access-point:8.1.0 -f PM_DataMover_Access_Point.dockerfile .
}

function reloadBase()
{
    local tarPath="$CURR_DIR/PM_System_Base_Service.tar.gz"
    if [[ ! -f $tarPath ]]; then
        echo "not find PM_System_Base_Service.tar.gz"
        return
    fi
    tar -xzf $tarPath
    docker build -t pm-system-base:8.1.0 -f PM_System_Base_Service.dockerfile .
}

function reloadGui()
{
    local tarPath="$CURR_DIR/PM_GUI.tar.gz"
    if [[ ! -f $tarPath ]]; then
        echo "not find PM_GUI.tar.gz"
        return
    fi
    tar -xzf $tarPath
    docker build -t pm-gui:8.1.0 -f PM_GUI.dockerfile .
}

function reloadApi()
{
    local tarPath="$CURR_DIR/PM_API_Gateway.tar.gz"
    if [[ ! -f $tarPath ]]; then
        echo "not find PM_API_Gateway.tar.gz"
        return
    fi
    tar -xzf $tarPath
    docker build -t pm-gui:8.1.0 -f PM_GUI.dockerfile .
}

function reflash()
{
    local podName=`kubectl get pods | grep "protect-manager" |awk -F " " '{print$1}'`
    if [[ -n $podName ]]; then
        kubectl delete pods ${podName}
    fi
    kubectl get pods
}


mode=$1
if [[ $mode == "dp" ]]; then
    reloadDataProtection
    reflash
elif [[ $mode == "base" ]]; then
    reloadBase
    reflash
elif [[ $mode == "dm" ]]; then
    reloadDataMover
    reflash
elif [[ $mode == "gui" ]]; then
    reloadGui
    reflash
elif [[ $mode == "api" ]]; then
    reloadApi
    reflash
elif [[ $mode == "all" ]]; then
    reloadBase
    reloadDataMover
    reloadDataProtection
    reloadGui
    reloadApi
    reflash
else
    echo "Usage:dp|base|dm|gui|all"
    echo "dp----->PM_Data_Protection_Service"
    echo "base--->PM_System_Base_Service"
    echo "dm----->PM_DataMover_Access_Point"
    echo "gui---->PM_GUI"
    echo "api---->PM_API_Gateway"
    return
fi

