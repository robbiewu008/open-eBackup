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
QUERYFILE=$AGENT_THIRDPARTY_TMPPATH"/querystate${PID}.sql"
QUERYFILERST=$AGENT_THIRDPARTY_TMPPATH"/querystateRST${PID}.txt"

# The related business script code need to be here.
########Begin########
[ -f "${AGENT_THIRDPARTY_RSTFILE}" ] && rm -rf "${AGENT_THIRDPARTY_RSTFILE}"

Log "Begin to do query freeze database ${InstanceName}." 
echo "select count(*) from v\$backup where status='ACTIVE' and con_id not in (select con_id from v\$pdbs where name='PDB\$SEED' or open_mode!='READ WRITE');" > ${QUERYFILE}
echo "exit;" >> ${QUERYFILE}

touch ${QUERYFILERST}
chmod 666 ${QUERYFILERST}
su - oracle -c "export ORACLE_SID=${InstanceName}; sqlplus / as sysdba @${QUERYFILE} >> ${QUERYFILERST}" 

# if the operation is successed, need to write blank string into the result file ${RSTFILE}. 
# Otherwise please write material error infomation into the result file ${$RSTFILE}.
# For example,
# business code result
RSTCODE=$?
if [ ${RSTCODE} -ne 0 ]
then
    [ -f ${QUERYFILE} ] && rm -f ${QUERYFILE}
    [ -f ${QUERYFILERST} ] && rm -f ${QUERYFILERST}
    Exit 1 -log "Query ${InstanceName} freeze state failed." -ret ${RSTCODE}
fi

cat ${QUERYFILERST} | grep "ERROR" >> /dev/null
RSTCODE=$?
if [ ${RSTCODE} -eq 0 ]
then
    cat ${QUERYFILERST} >> ${LOG_FILE_NAME}
    [ -f ${QUERYFILE} ] && rm -f ${QUERYFILE}
    [ -f ${QUERYFILERST} ] && rm -f ${QUERYFILERST}
    Exit 1 -log "Query freeze ${InstanceName} state failed:" -ret ${RSTCODE}
fi

ACTIVECOUNT=`sed -n '/-----/,/^ *$/p' "${QUERYFILERST}" | sed -e '/-----/d' -e '/^ *$/d' | awk '{print $1}'`
if [ "${ACTIVECOUNT}" -eq "0" ]
then
    Log "There are no backup tablespace ${InstanceName}." 
    echo 1 > "${AGENT_THIRDPARTY_RSTFILE}"
else
    Log "Database is in hot backup mode:${ACTIVECOUNT}, ${InstanceName}." 
    echo 0 > "${AGENT_THIRDPARTY_RSTFILE}"
fi
        
[ -f ${QUERYFILE} ] && rm -f ${QUERYFILE}
[ -f ${QUERYFILERST} ] && rm -f ${QUERYFILERST}

Exit 0  -log "Finish doing query freeze database ${InstanceName}."
########End#######
