#!/bin/bash
# This file is a part of the open-eBackup project.
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file, You can obtain one at
# http://mozilla.org/MPL/2.0/.
#
# Copyright (c) [2024] Huawei Technologies Co.,Ltd.
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
PID=$1
AGENT_ROOT_PATH=$2

. "${AGENT_ROOT_PATH}/bin/agent_thirdpartyfunc.sh"
. "${AGENT_ROOT_PATH}/bin/thirdparty/Oracle_AFD_Var.sh"
. "${AGENT_ROOT_PATH}/bin/thirdparty/Oracle_AFD_Common.sh"

LOG_FILE_NAME="${AGENT_THIRDPARTY_LOGPATH}/oracleAFD.log"

# The related business script code need to be here.
########Begin########
Log "Begin to do test." 

# scan disk
hot_add

# exists situation, after hot_add, the disk is not exists, wait 10 seconds
sleep 10
SOURCE_LUNS=`echo ${SOURCE_LUN_LIST} | sed -e 's/;/ /g'`
# config lun information
for lunInfo in ${SOURCE_LUNS}
do
    Log "LunInfo=${lunInfo}"
    lunID=`echo ${lunInfo} | awk -F "-" '{print $1}'`
    lunWWN=`echo ${lunInfo} | awk -F "-" '{print $2}'`
    if [ "${AFD_MODE}" = "1" ]
    then
        configLun4Udev ${lunID} ${lunWWN}
    else
        configLun4Native ${lunID} ${lunWWN}
    fi
    
    lastErr=$?
    if [ $lastErr -ne 0 ]
    then
        Exit 1 -log "config lun ${lunInfo} failed, code=${lastErr}."  -ret ${lastErr}
        exit 1
    fi
done
    
if [ "${AFD_MODE}" = "1" ]
then
    # add lun again
    udevadm control --reload-rules && udevadm trigger
fi

# scan afd disk
scanAFDDisk
Log "`su - ${ASM_USER} -c \"asmcmd afd_lsdsk\"`"

# start cluster and DB
startDB ${AGENT_ROOT_PATH}
RSTCODE=$?

Log "Finish doing test." 
########End#######

# if the operation is successed, need to write blank string into the result file ${RSTFILE}. 
# Otherwise please write material error infomation into the result file ${$RSTFILE}.
# For example,
# business code result
if [ ${RSTCODE} -ne 0 ]
then
    Exit 1 -log "Here:record some error, code=$RSTCODE." -ret ${RSTCODE}
else
    Exit 0 -log "Here:record success." 
fi

