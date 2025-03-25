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
. "$2/bin/agent_thirdpartyfunc.sh"

FREEZEFILE=$TMPPATH"/freezeoracle${PID}.sql"
FREEZEFILERST=$TMPPATH"/freezeoracleRST${PID}.txt"

# The related business script code need to be here.
########Begin########
Log "Begin to do freeze db." 
echo "alter database begin backup;" > ${FREEZEFILE}
echo "exit;" >> ${FREEZEFILE}

touch ${FREEZEFILERST}
chmod 666 ${FREEZEFILERST}
su - oracle -c "export ORACLE_SID=dbora; sqlplus / as sysdba @${FREEZEFILE} >> ${FREEZEFILERST}"

# if the operation is successed, need to write blank string into the result file ${RSTFILE}. 
# Otherwise please write material error infomation into the result file ${$RSTFILE}.
# For example,
# business code result
RSTCODE=$?
if [ ${RSTCODE} -ne 0 ]
then
    [ -f ${FREEZEFILERST} ] && rm -f ${FREEZEFILERST}
    [ -f ${FREEZEFILE} ] && rm -f ${FREEZEFILE}
    Exit 1  -log "Open hot back up failed:"  -ret  ${RSTCODE}
fi

cat ${FREEZEFILERST} | grep "ERROR" >> /dev/null
RSTCODE=$?
if [ ${RSTCODE} -ne 0 ]
then
    cat ${FREEZEFILERST} >> ${LOG_FILE_NAME}
    [ -f ${FREEZEFILERST} ] && rm -f ${FREEZEFILERST}
    [ -f ${FREEZEFILE} ] && rm -f ${FREEZEFILE}
    Exit 1 -log "Open hot back up failed."  -ret  ${RSTCODE}
fi

[ -f ${FREEZEFILERST} ] && rm -f ${FREEZEFILERST}
[ -f ${FREEZEFILE} ] && rm -f ${FREEZEFILE}

Exit 0 -log "Finish doing freeze db."
########End#######
