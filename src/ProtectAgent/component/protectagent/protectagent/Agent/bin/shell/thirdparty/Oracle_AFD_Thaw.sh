#!/bin/sh
# 
#  This file is a part of the open-eBackup project.
#  This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
#  If a copy of the MPL was not distributed with this file, You can obtain one at
#  http://mozilla.org/MPL/2.0/.
# 
#  Copyright (c) [2024] Huawei Technologies Co.,Ltd.
# 
#  THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
#  EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
#  MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
#
PID=$1
AGENT_ROOT_PATH=$2

. "${AGENT_ROOT_PATH}/bin/agent_thirdpartyfunc.sh"
. "${AGENT_ROOT_PATH}/bin/thirdparty/Oracle_AFD_Var.sh"
. "${AGENT_ROOT_PATH}/bin/thirdparty/Oracle_AFD_Common.sh"

LOG_FILE_NAME="${AGENT_THIRDPARTY_LOGPATH}/oracleAFDConsistent.log"
UNFREEZEFILE=$AGENT_THIRDPARTY_TMPPATH"/unfreezeoracle${PID}.sql"
UNFREEZEFILERST=$AGENT_THIRDPARTY_TMPPATH"/unfreezeoracleRST${PID}.txt"

# The related business script code need to be here.
########Begin########
Log "Begin to do unfreeze db ${InstanceName}." 
echo "alter database end backup;" > ${UNFREEZEFILE}
echo "exit;" >> ${UNFREEZEFILE}

touch ${UNFREEZEFILERST}
chmod 666 ${UNFREEZEFILERST}
su - oracle -c "export ORACLE_SID=${InstanceName}; sqlplus / as sysdba @${UNFREEZEFILE} >> ${UNFREEZEFILERST}"

# if the operation is successed, need to write blank string into the result file ${RSTFILE}. 
# Otherwise please write material error infomation into the result file ${$RSTFILE}.
# For example,
# business code result
RSTCODE=$?
if [ ${RSTCODE} -ne 0 ]
then
    [ -f ${UNFREEZEFILERST} ] && rm -f ${UNFREEZEFILERST}
    [ -f ${UNFREEZEFILE} ] && rm -f ${UNFREEZEFILE}
    Exit 1 -log "Close ${InstanceName} hot back up failed." -ret ${RSTCODE}
fi

cat ${UNFREEZEFILERST} | grep "ERROR" >> /dev/null
RSTCODE=$?
if [ ${RSTCODE} -eq 0 ]
then
    cat ${UNFREEZEFILERST} >> ${LOG_FILE_NAME}
    [ -f ${UNFREEZEFILERST} ] && rm -f ${UNFREEZEFILERST}
    [ -f ${UNFREEZEFILE} ] && rm -f ${UNFREEZEFILE}
    Exit 1 -log "Close ${InstanceName} hot back up failed:" -ret ${RSTCODE}  
fi

[ -f ${UNFREEZEFILERST} ] && rm -f ${UNFREEZEFILERST}
[ -f ${UNFREEZEFILE} ] && rm -f ${UNFREEZEFILE}

Exit 0  -log  "Finish doing unfreeze db ${InstanceName}." 
########End#######
