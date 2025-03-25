#!/bin/sh
set +x

AGENT_ROOT_PATH=$1
PID=$2
PARAM_NUM=$3
. "${AGENT_ROOT_PATH}/bin/agent_bin_func.sh"

#for log
LOG_FILE_NAME="${LOG_PATH}/shutdown.log"

Log "begin to execute : shutdown -h now"

shutdown -h now

exit 0
