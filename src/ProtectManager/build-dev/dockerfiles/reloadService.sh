#!/bin/bash

CURR_DIR=$(cd "$(dirname "$0")" && pwd)
export KUBECONFIG=/etc/rancher/k3s/k3s.yaml
TAG=1.0.0

reload_dvm() {
  reload dvm PM_Database_Version_Migration.tar.gz pm-database-version-migration:$TAG PM_Database_Version_Migration.dockerfile|awk '{print "[dvm]" $0}' &
}

reload_sch() {
  reload sch PM_Scheduler.tar.gz pm-scheduler:1.0.0 PM_Scheduler.dockerfile|awk '{print "[sch]" $0}' &
}

reload_cl() {
  reload cl PM_Copies_Catalog.tar.gz pm-copies-catalog:$TAG PM_Copies_Catalog.dockerfile|awk '{print "[cl]" $0}' &
}

reload_rl() {
  reload rl PM_Resource_Lock_Manager.tar.gz pm-resource-lock-manager:$TAG PM_Resource_Lock_Manager.dockerfile|awk '{print "[rl]" $0}' &
}

reload_rm() {
  reload rm PM_Resource_Manager.tar.gz pm-resource-manager:$TAG PM_Resource_Manager.dockerfile|awk '{print "[rm]" $0}' &
}

reload_dp() {
  reload dp PM_Data_Protection_Service.tar.gz pm-protection-service:$TAG PM_Data_Protection_Service.dockerfile|awk '{print "[dp]" $0}' &
}

reload_dm() {
  reload dm PM_DataMover_Access_Point.tar.gz pm-dm-access-point:$TAG PM_DataMover_Access_Point.dockerfile|awk '{print "[dm]" $0}' &
}

reload_base() {
  reload base PM_System_Base_Service.tar.gz pm-system-base:$TAG PM_System_Base_Service.dockerfile|awk '{print "[base]" $0}' &
}

reload_gui() {
  reload gui PM_GUI.tar.gz pm-gui:$TAG PM_GUI.dockerfile|awk '{print "[gui]" $0}' &
}

reload_api() {
  reload api PM_API_Gateway.tar.gz pm-api:$TAG PM_API_Gateway.dockerfile|awk '{print "[api]" $0}' &
}

reload_lm() {
  reload lm PM_Live_Mount_Main.tar.gz pm-live-mount-manager:$TAG PM_Live_Mount_Main.dockerfile|awk '{print "[lm]" $0}' &
}

reload_crm() {
  reload crm PM_Client_Resource_Manager.tar.gz pm-agent-manager:$TAG PM_Client_Resource_Manager.dockerfile|awk '{print "[crm]" $0}' &
}

function reload() {
  local dir=$1
  local file=$2
  local tag=$3
  local dockerfile=$4
  if [ "$ONLY_COMPILE" == "1" ]; then
    if [ -d "./$dir" ]; then
      echo compile command: docker build -t "$tag" -f "$dockerfile" ./"$dir"
      build "$tag" "$dockerfile" "$dir"
    else
      echo "[ERROR] ./$dir is missing."
    fi
    return
  fi
  cleanPackage "$dir"
  if WGET "$file" "$dir"; then
    build "$tag" "$dockerfile" "$dir"
  fi
}

function build() {
  if [ ! -f "$2" ]; then
    echo "dockerfile($1) is missing"
    return
  fi
  docker build -t "$1" -f "$2" ./"$3"
}

function cleanPackage() {
  rm -rf "$1"
}

function reflash() {
  if [[ $ONLY_RELEASE -eq 1 ]]; then
    return
  fi
  wait
  local podName=$(kubectl get pods -n dpa | grep "protect-manager" | awk -F " " '{print$1}')
  if [[ -n "$podName" ]]; then
    kubectl delete pods "${podName}" -n dpa
  fi
  kubectl get pods -n dpa
}

function WGET() {
  local source="https://cmc-ctu-artifactory.cmc.tools.huawei.com/artifactory/cmc-software-release/OceanStor%20100P/ProtectManager/$TAG/dorado/master/mspkg/euler-arm/$1"
  local target="$CURR_DIR/$1"
  if [[ $USE_LOCAL_PACKAGE -eq 0 ]]; then
    if [[ -f "$target" ]]; then
      echo "deleting old package: $1"
      rm -f "$target"*
    fi
    echo "downloading new package: $1"
    wget "$source" -O "$target"
  fi
  if [[ ! -f "$target" ]]; then
    echo "[Error] Not find $1"
    return 1
  fi
  echo "unpack $1"
  if [[ ! -d "$2" ]]; then
    mkdir -p "$2"
  fi
  tar -xzf "$target" -C "$2"
  if [[ $ONLY_RELEASE -eq 1 ]]; then
    return 1
  fi
  return 0
}

USE_LOCAL_PACKAGE=1
ONLY_RELEASE=0
ONLY_COMPILE=0

for index in $(seq $#); do
  echo index > /dev/null
  if [[ "$1" == "--download" ]] || [[ "$1" == "-d" ]]; then
    USE_LOCAL_PACKAGE=0
    shift 1
  elif [[ "$1" == "--unpack" ]] || [[ "$1" == "-u" ]]; then
    ONLY_RELEASE=1
    shift 1
  elif [[ "$1" == "--compile" ]] || [[ "$1" == "-c" ]]; then
    ONLY_COMPILE=1
    shift 1
  else
    break
  fi
done

if [ $# -eq 0 ]; then
  echo "Usage: [-d|--download] [-u|--unpack] [-c|--compile] all|dp|base|dm|gui|rm|rl|cl|lm|sch|dvm|crm"
  echo "dp    PM_Data_Protection_Service"
  echo "base  PM_System_Base_Service"
  echo "dm    PM_DataMover_Access_Point"
  echo "gui   PM_GUI"
  echo "api   PM_API_Gateway"
  echo "rm    PM_Resource_Manager"
  echo "rl    PM_Resource_Lock_Manager"
  echo "cl    PM_Copies_Catalog"
  echo "lm    PM_Live_Mount_Manager"
  echo "sch   PM_Scheduler"
  echo "dvm   PM_Database_Version_Migration"
  echo "crm   PM_Client_Resource_Manager"
  exit 0
fi

for i in $(seq $#)
do
  echo "$i" > /dev/null
  mode=$1
  if [[ $mode == "all" ]]; then
    methods=$(grep -E ^reload_ "$0"|awk -F'(' '{print $1}')
    for method in $methods
    do
      $method
    done
  else
    method=$(grep "reload_$mode" "$0"|awk -F'(' '{print $1}')
    if [ "$method" != "" ]; then
      $method
    else
      echo "not support $mode"
    fi
  fi
  shift
done
reflash
