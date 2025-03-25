#!/bin/sh
set +x

AGENT_ROOT_PATH=$1

. "${AGENT_ROOT_PATH}/sbin/agent_sbin_func.sh"

#for log
LOG_FILE_NAME="${LOG_PATH}/sanclientaction.log"

qla2xxx_status=`cat /sys/module/qla2xxx/parameters/qlini_mode`
if [ ${qla2xxx_status} != 'dual' ]; then
  Log "qla2xxx_status is not nomoral"
  exit ${ERROR_QLA2XXX_STAUTS}
fi

systemctl status targetcli
if [ $? -ne 0 ]; then
  Log "targetcli status is not nomoral"
  exit ${ERROR_TARGETCLI_STATUS}
fi

Log "check sanclient success"
exit 0