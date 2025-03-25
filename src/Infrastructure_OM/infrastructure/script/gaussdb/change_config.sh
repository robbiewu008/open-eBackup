#!/bin/bash
source /usr/local/gaussdb/log.sh
MANAGE_IP=$POD_IP

function change_config_and_restart() {
  action=$1
  line_no=$(cat "/usr/local/gaussdb/data/pg_hba.conf" | grep -n "0.0.0.0/0           sha256" | awk -F':' '{print $1}')
  sed -i "${line_no}d" /usr/local/gaussdb/data/pg_hba.conf
  if [ "${action}" == "open" ]; then
    # 允许远程连接
    echo "hostssl    all             all            0.0.0.0/0           sha256" >>/usr/local/gaussdb/data/pg_hba.conf
  else
    # 禁止远程连接
    echo "#hostssl    all             all            0.0.0.0/0           sha256" >>/usr/local/gaussdb/data/pg_hba.conf
  fi

  export GAUSSHOME=/usr/local/gaussdb/app
  export LD_LIBRARY_PATH=/usr/local/gaussdb/app/lib
  /usr/local/gaussdb/app/bin/gs_ctl restart -D /usr/local/gaussdb/data
  if [ $? -eq "0" ]; then
    echo "success"
    exit 0
  else
    echo "fail"
    exit 1
  fi
}

function main() {
  if [ ${type} == "open" ]; then
      change_config_and_restart "open"
  elif [ ${type} == "close" ]; then
      change_config_and_restart "close"
  else
    log_error "param error"
    echo "fail"
    exit 1
  fi
}

type=$1
main
