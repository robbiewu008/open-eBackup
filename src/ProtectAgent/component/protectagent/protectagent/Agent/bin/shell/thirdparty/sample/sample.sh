#!/bin/sh
. "$2/bin/agent_thirdpartyfunc.sh"

#for example,How get value from pass parameters' file. 
#if key is "MysqlUserName",the value is :
GetValue "${INPUT_PARAMETER_LIST}" MysqlUserName
value=$ArgValue
Log $value
# The related business script code need to be here.
########Begin########
Log "Begin to do something." 



Log "Finish doing something." 
########End#######

# if the operation is successed, need to write blank string into the result file ${RSTFILE}. 
# Otherwise please write material error infomation into the result file ${$RSTFILE}.
# For example,
# business code result
RSTCODE=$?
if [ ${RSTCODE} -ne 0 ]
then
    Exit 1 -log "Here:record some error."  -ret ${RSTCODE}
else
    Exit 0 -log "Here:record success." 
fi
