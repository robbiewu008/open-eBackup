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

rm -rf ${AGENT_ROOT}/test/stubTest/html
#if you just wanna run your own binary file
#you shuld change run.sh to your own binary file name
sh ${AGENT_ROOT}/test/stubTest/bin/run.sh

mkdir -p ${AGENT_ROOT}/test/stubTest/temp
#lcov -d ${AGENT_ROOT} -o ${AGENT_ROOT}/test/stubTest/temp/total.info -b ${AGENT_ROOT} -c
#genhtml -o ${AGENT_ROOT}/test/stubTest/html ${AGENT_ROOT}/test/stubTest/temp/total.info
#rm -rf ${AGENT_ROOT}/test/stubTest/temp
#find ${AGENT_ROOT}/test/stubTest/obj -name "*.gcno" | xargs rm -f
#find ${AGENT_ROOT}/test/stubTest/obj -name "*.gcda" | xargs rm -f

find ${AGENT_ROOT}/test/stubTest/obj -name "*.gcda" | xargs rm -rf
export COVERAGE_FILE_TMP=total_tmp.info
export COVERAGE_FILE=total.info
lcov --rc lcov_branch_coverage=1 -c -d ${AGENT_ROOT} -o ${COVERAGE_FILE_TMP}
lcov --rc lcov_branch_coverage=1 -e ${COVERAGE_FILE_TMP} \
"*src/src/agent/*"                                      \
"*src/src/alarm/*"                                      \
"*src/src/apps/app/appprotect/"                                   \
"*src/src/apps/appprotect/AppProtectService.cpp"                            \
"*src/src/apps/appprotect/plugininterface/JobServiceHandler.cpp"            \
"*src/src/apps/appprotect/plugininterface/PluginRegisterHandler.cpp"        \
"*src/src/apps/appprotect/plugininterface/ShareResourceHandler.cpp"         \
"*src/src/apps/dws/XBSAClient/DataConversion.cpp"       \
"*src/src/apps/dws/XBSAClient/File.cpp"                 \
"*src/src/apps/dws/XBSAClient/ThriftClientMgr.cpp"      \
"*src/src/apps/dws/XBSAClient/xbsa.cpp"                 \
"*src/src/apps/dws/XBSACom/TSSLSocketFactoryPassword.cpp"                   \
"*src/src/apps/dws/XBSAServer/BsaDb.cpp"                \
"*src/src/apps/dws/XBSAServer/BsaIntfAdaptor.cpp"       \
"*src/src/apps/dws/XBSAServer/BsaMountManager.cpp"      \
"*src/src/apps/dws/XBSAServer/BSAServiceHandler.cpp"    \
"*src/src/apps/dws/XBSAServer/BsaSession.cpp"           \
"*src/src/apps/dws/XBSAServer/BsaSessionManager.cpp"    \
"*src/src/apps/dws/XBSAServer/BsaTransManager.cpp"      \
"*src/src/apps/dws/XBSAServer/BsaTransState.cpp"        \
"*src/src/apps/dws/XBSAServer/CTimer.cpp"               \
"*src/src/apps/dws/XBSAServer/ThriftServer.cpp"         \
"*src/src/common/*"                                     \
"*src/src/host/*"                                       \
"*src/src/message/curlclient/*"                         \
"*src/src/message/rest/*"                               \
"*src/src/message/tcp/*"                                \
"*src/src/message/thrfit/*"                             \
"*src/src/plugins/appprotect/*"                         \
"*src/src/plugins/dws/*"                                \
"*src/src/rootexec/*"                                   \
"*src/src/securecom/*"                                  \
"*src/src/servicecenter/certifcateservice/*"            \
"*src/src/servicecenter/services/messageservice/*"      \
"*src/src/servicecenter/servicefactory/*"               \
"*src/src/servicecenter/services/device/*"              \
"*src/src/servicecenter/services/jobservice/*"          \
"*src/src/servicecenter/thriftservice/*"                \
"*src/src/servicecenter/timerservice/*"                 \
"*src/src/taskmanager/externaljob/*"                    \
-o ${COVERAGE_FILE}

genhtml --rc genhtml_branch_coverage=1 ${COVERAGE_FILE} -o ${AGENT_ROOT}/test/stubTest/html
rm -rf ${COVERAGE_FILE_TMP}
