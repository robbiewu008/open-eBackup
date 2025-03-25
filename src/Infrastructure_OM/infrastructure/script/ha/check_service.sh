#!/bin/bash
set -o pipefail

#******************************************************************#
# Function: check_status_process
# Description: The function show AdminNode process detail status
# Input Parameters:
# None
# Return : 0 runing
#          1 have status in T or Z
#******************************************************************#
G_HA_SCRIPT_PATH=/usr/local/ha/script
source ${G_HA_SCRIPT_PATH}/log.sh

function check_status_process() {
  local resource_name=$1
  local check_ret=""
  local stat=""
  local proc_pid=0
  local proc_ppid=0
  local proc_name_tmp="${resource_name:0:15}"
  local proc_info=$(/bin/ps -elf | grep -E -v '(grep)' | grep -E "${resource_name}")
  if [ $? -ne 0 ]; then
    log_warn "[${FUNCNAME[0]}(),$LINENO] cant find process of ${resource_name}"
    return 0
  fi
  #Z
  stat=$(echo "${proc_info}" | awk -F " " '{print $2}' | grep -w "Z")
  if [[ $stat ]]; then
    proc_pid=$(echo "${proc_info}" | grep -E "\[${proc_name_tmp}\] <defunct>" | grep -E -v '(grep)' | sed -n 1p | awk -F " " '{print $4}')
    proc_ppid=$(echo "${proc_info}" | grep -E "\[${proc_name_tmp}\] <defunct>" | grep -E -v '(grep)' | sed -n 1p | awk -F " " '{print $5}')
    kill -HUP $proc_ppid
    L_RET=$?
    if [ $L_RET -eq 0 ]; then
      log_warn "[${FUNCNAME[0]}(),$LINENO] the process(name:$resource_name, pid:${proc_pid}, ppid:${proc_ppid}) status is Z, kill -HUP ${proc_ppid} result is ${L_RET}."
    else
      log_warn "[${FUNCNAME[0]}(),$LINENO] the process(name:$resource_name, pid:${proc_pid}, ppid:${proc_ppid}) status is Z, kill -HUP ${proc_ppid} result is ${L_RET}."
    fi
  fi
  #T
  local tmp_pid_list=$(echo "${proc_info}" | awk -F " " '{print $2,$4}' | grep -w "T")
  proc_pid=$(echo "${tmp_pid_list}" | awk -F " " '{print $2}')
  if [[ $tmp_pid_list ]]; then
    kill -9 ${proc_pid}
    L_RET=$?
    log_warn "[${FUNCNAME[0]}(),$LINENO] the process(name:$resource_name, pid:${proc_pid} status is T, kill -9 ${proc_pid} result is ${L_RET}."
  fi
  return 0
}
