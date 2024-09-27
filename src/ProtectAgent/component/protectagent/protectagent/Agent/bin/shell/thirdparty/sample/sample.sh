#!/bin/sh
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
