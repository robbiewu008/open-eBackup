#!/bin/bash
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

LOG_FILE_NAME="${AGENT_THIRDPARTY_LOGPATH}/oracleAFD.log"

# The related business script code need to be here.
########Begin########
Log "Begin to do clear AFD." 

if [ "${AFD_MODE}" = "1" ]
then
    # delete udev config
    SOURCE_LUNS=`echo ${SOURCE_LUN_LIST} | sed -e 's/;/ /g'`
    for lunInfo in ${SOURCE_LUNS}
    do
        lunID=`echo ${lunInfo} | awk -F "-" '{print $1}'`
        removeLunConfig ${lunID}
        lastErr=$?
        if [ $lastErr -ne 0 ]
        then
            Exit 1 -log "remove lun config failed, code=${lastErr}." -ret ${lastErr}
        fi
    done
fi

hot_add

Log "Finish doing clear AFD." 
########End#######

# if the operation is successed, need to write blank string into the result file ${RSTFILE}. 
# Otherwise please write material error infomation into the result file ${$RSTFILE}.
# For example,
# business code result
RSTCODE=$?
if [ ${RSTCODE} -ne 0 ]
then
    Exit 1 -log "Here:record some error, code=$RSTCODE." -ret ${RSTCODE}
else
    Exit 0 -log "Here:record success." 
fi
