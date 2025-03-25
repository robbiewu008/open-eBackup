#!/bin/sh
PID=$1
AGENT_ROOT_PATH=$2

. "${AGENT_ROOT_PATH}/bin/agent_thirdpartyfunc.sh"
. "${AGENT_ROOT_PATH}/bin/thirdparty/Oracle_AFD_Var.sh"
. "${AGENT_ROOT_PATH}/bin/thirdparty/Oracle_AFD_Common.sh"

LOG_FILE_NAME="${AGENT_THIRDPARTY_LOGPATH}/oracleAFDConsistent.log"
FREEZEFILE=$AGENT_THIRDPARTY_TMPPATH"/freezeoracle${PID}.sql"
FREEZEFILERST=$AGENT_THIRDPARTY_TMPPATH"/freezeoracleRST${PID}.txt"

# The related business script code need to be here.
########Begin########
Log "Begin to do freeze db ${InstanceName}." 
echo "alter database begin backup;" > ${FREEZEFILE}
echo "exit;" >> ${FREEZEFILE}

touch ${FREEZEFILERST}
chmod 666 ${FREEZEFILERST}
su - oracle -c "export ORACLE_SID=${InstanceName}; sqlplus / as sysdba @${FREEZEFILE} >> ${FREEZEFILERST}"

# if the operation is successed, need to write blank string into the result file ${RSTFILE}. 
# Otherwise please write material error infomation into the result file ${$RSTFILE}.
# For example,
# business code result
RSTCODE=$?
if [ ${RSTCODE} -ne 0 ]
then
    [ -f ${FREEZEFILERST} ] && rm -f ${FREEZEFILERST}
    [ -f ${FREEZEFILE} ] && rm -f ${FREEZEFILE}
    Exit 1  -log "Open ${InstanceName} hot back up failed:"  -ret  ${RSTCODE}
fi

cat ${FREEZEFILERST} | grep "ERROR" >> /dev/null
RSTCODE=$?
if [ ${RSTCODE} -eq 0 ]
then
    cat ${FREEZEFILERST} >> ${LOG_FILE_NAME}
    [ -f ${FREEZEFILERST} ] && rm -f ${FREEZEFILERST}
    [ -f ${FREEZEFILE} ] && rm -f ${FREEZEFILE}
    Exit 1 -log "Open ${InstanceName} hot back up failed."  -ret  ${RSTCODE}
fi

[ -f ${FREEZEFILERST} ] && rm -f ${FREEZEFILERST}
[ -f ${FREEZEFILE} ] && rm -f ${FREEZEFILE}

Exit 0 -log "Finish doing freeze db ${InstanceName}."
########End#######
