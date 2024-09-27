#! /bin/sh
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

AGENT_ROOT=${HOME}/Agent
AGENT_TEST_BIN=${AGENT_ROOT}/test/stubTest/bin
export LD_LIBRARY_PATH=${AGENT_ROOT}/bin:${LD_LIBRARY_PATH}
export LD_LIBRARY_PATH=${AGENT_ROOT}/open_src/libevent/.libs/lib:${AGENT_ROOT}/open_src/gperftools/tcmalloc/lib:${LD_LIBRARY_PATH}
rm -rf ${AGENT_TEST_BIN}/report

ls -l ${AGENT_TEST_BIN} | grep Agent > temp
while read line 
do 
	#第一个字符为-，表示为普通文件第四个字符为x表示可执行
	if [[ ${line:0:1} = "-" ]] && [[ ${line:3:1} = "x" ]];
	then 
		script=`echo $line | awk '{print $NF}'`
		echo "*********** begin to test ${script} ******************"
		${AGENT_TEST_BIN}/${script}
		if [ $? -ne 0 ]; then
			echo "*********** exec ${script} failed ******************"
		fi
		echo "*********** finish testing ${script} ******************"
	fi;
done < temp;
rm temp;

# 将LLT报告合并
testNum=0
testFailuresNum=0
testDisabledNum=0
testErrorsNum=0
testTime=0
ls -l ${AGENT_TEST_BIN}/report | grep -v llt_detail.xml | grep .xml > temp
while read line; do
    fileName=`echo $line | awk '{print $NF}'`
	for item in `sed -n '2p' ${AGENT_TEST_BIN}/report/${fileName}`; do
		itemKey=`echo ${item} | awk -F "=" '{print $1}'`
		itemValue=`echo ${item} | awk -F "=" '{print $2}' | sed 's/\"//g'`
		if [ "${itemKey}" = "tests" ]; then
			testNum=$(expr ${testNum} + ${itemValue})
		fi
		if [ "${itemKey}" = "failures" ]; then
			testFailuresNum=$(expr ${testFailuresNum} + ${itemValue})
		fi
		if [ "${itemKey}" = "disabled" ]; then
			testDisabledNum=$(expr ${testDisabledNum} + ${itemValue})
		fi
		if [ "${itemKey}" = "errors" ]; then
			testErrorsNum=$(expr ${testErrorsNum} + ${itemValue})
		fi
		if [ "${itemKey}" = "time" ]; then
			testTime=$(echo "scale=3; $testTime + $itemValue" | bc)
		fi
	done
    RowNum=`wc -l ${AGENT_TEST_BIN}/report/${fileName} | awk -F " "  '{print $1}'`
    cat ${AGENT_TEST_BIN}/report/${fileName} | tail -n +3 | head -n $(expr $RowNum - 3) >> ${AGENT_TEST_BIN}/report/llt_detail
done < temp;
if [ "${testTime:0:1}" = "." ]; then
	testTime="0${testTime}"
fi
echo "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" > ${AGENT_TEST_BIN}/report/llt_detail.xml
echo "<testsuites tests=\"${testNum}\" failures=\"${testFailuresNum}\" disabled=\"${testDisabledNum}\" \
errors=\"${testErrorsNum}\" time=\"${testTime}\" name=\"AllTests\">" >> ${AGENT_TEST_BIN}/report/llt_detail.xml
cat ${AGENT_TEST_BIN}/report/llt_detail >> ${AGENT_TEST_BIN}/report/llt_detail.xml
echo "</testsuites>" >> ${AGENT_TEST_BIN}/report/llt_detail.xml

chmod +x ${AGENT_TEST_BIN}