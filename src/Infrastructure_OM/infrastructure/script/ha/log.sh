#!/bin/sh
set +x

NODE_NAME=${NODE_NAME}
LOG_PATH="/opt/OceanProtect/logs/${NODE_NAME}/infrastructure/ha/scriptlog"

if [ ! -d $LOG_PATH ]; then
  mkdir -p $LOG_PATH
fi

function write_log_info() {
  local datetime=$(date +"%Y-%m-%d %H:%M:%S")
  local cur_user=$(whoami)
  local cur_script=$1
  local loglevel=$2
  local message=$3

  echo "[${datetime}][${loglevel}][${message}][${cur_script}][${cur_user}]"
  echo "[${datetime}][${loglevel}][${message}][${cur_script}][${cur_user}]" >>"${LOG_PATH}/ha.log"
}

function log_info() {
  if [[ $# != 1 ]]; then
    echo "[$(date "+%Y-%m-%d %H:%M:%S")]The number of parameters is incorrect."
    exit 1
  fi

  write_log_info "${0##*/}" "INFO" "$@"
}

function log_error() {
  if [[ $# != 1 ]]; then
    echo "[$(date "+%Y-%m-%d %H:%M:%S")]The number of parameters is incorrect."
    exit 1
  fi

  write_log_info "${0##*/}" "ERROR" "$@"
}

function log_warn() {
  if [[ $# != 1 ]]; then
    echo "[$(date "+%Y-%m-%d %H:%M:%S")]The number of parameters is incorrect."
    exit 1
  fi

  write_log_info "${0##*/}" "WARN" "$@"
}

function check_result() {
  if [ "$1" != "0" ]; then
    log_error "Exec cmd:$2 failed."
  else
    log_info "Exec cmd:$2 success."
  fi
}
