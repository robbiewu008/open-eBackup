#!/bin/sh
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
