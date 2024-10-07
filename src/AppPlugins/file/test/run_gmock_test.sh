#/bin/sh
#
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
#

SCRIPT_PATH=$(dirname ${BASH_SOURCE[0]})
CURRENT_DIR=$(cd "${SCRIPT_PATH}"; pwd)
source $SCRIPT_PATH/check_coverage.sh
if [ ! -d "${MODULE_ROOT_PATH}" ];then
    MODULE_ROOT_PATH=${CURRENT_DIR}/../../../Module
fi
if [ ! -d "${MODULE_ROOT_PATH}" ];then
    echo "ERROR" "Module path no exist"
    exit 1
fi
MODULE_ROOT_PATH=$(cd "${MODULE_ROOT_PATH}"; pwd)
BACKUP_HOME=$(cd "${SCRIPT_PATH}/.."; pwd)
DT_UTILS_DIR=$(cd "${MODULE_ROOT_PATH}/dt_utils"; pwd)
TEST_DIR=$(cd "${BACKUP_HOME}/test"; pwd)

CPPC="g++"
CC="gcc"
cc="gcc"
CFLAGS="-g $ASAN_CFLAGS -pipe -fpic -DLINUX $KMC_OPT"
cFLAGS=${CFLAGS}
OFLAGS="-g $ASAN_CFLAGS -luuid"
oFLAGS="-g $ASAN_CFLAGS -fprofile-arcs -ftest-coverage"
DFLAGS="-g $ASAN_CFLAGS -shared -luuid"
dFLAGS="-g $ASAN_CFLAGS -shared"
LDFLAGS="-Wall -fprofile-arcs -ftest-coverage"

sh ${DT_UTILS_DIR}/build_gmock.sh "$@"
if [ $? -ne 0 ];then
    exit 1
fi
type="$1"
if [ "${type}" = "DTFUZZ" ]; then
    echo "This is DTFUZZ compile"
    dos2unix ${DT_UTILS_DIR}/download_dtfuzz.sh
	chmod u+x ${DT_UTILS_DIR}/download_dtfuzz.sh
	sh ${DT_UTILS_DIR}/download_dtfuzz.sh

    dos2unix ${DT_UTILS_DIR}/build_fuzz.sh
	chmod u+x ${DT_UTILS_DIR}/build_fuzz.sh
	sh ${DT_UTILS_DIR}/build_fuzz.sh
	if [ $? -ne 0 ];then
		echo "ERROR" "Compile Osecodefuzz[${DT_UTILS_DIR}/secodefuzz] failed"
		exit 1
	fi
fi
export CPPC CC cc CFLAGS cFLAGS OFLAGS oFLAGS DFLAGS dFLAGS LDFLAGS

cd ${TEST_DIR}
mkdir -p bin
mkdir -p log

sh ${TEST_DIR}/build.sh "$@"
if [ $? -ne 0 ];then
    exit 1
fi

echo "export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}"
if [ "$1" = "gdb" ]; then
    gdb ${TEST_DIR}/build/test_file_plugin $*
elif [ "${type}" = "DTFUZZ" ]; then
    LD_PRELOAD=/usr/lib64/libasan.so.4 ${TEST_DIR}/build/test_file_plugin $*
else
    ${TEST_DIR}/build/test_file_plugin $*
fi
RET=$?

which lcov
if [ $? -ne 0 ]; then
    exit $RET
fi

COMPONENT_BASE_DIR="${TEST_DIR}"
export COMPONENT_BASE_DIR
echo "COMPONENT_BASE_DIR: " ${COMPONENT_BASE_DIR}
cd ${COMPONENT_BASE_DIR}
COVERAGE_FILE=coverage_file.info
REPORT_FOLDER=coverage_report_file
rm -rf ${REPORT_FOLDER}
lcov --rc lcov_branch_coverage=1 -c -d build/file_src -o ${COVERAGE_FILE}_tmp
lcov --rc lcov_branch_coverage=1 -e ${COVERAGE_FILE}_tmp "${BACKUP_HOME}/src/*" -o ${COVERAGE_FILE} > coverage.log
genhtml --rc genhtml_branch_coverage=1 ${COVERAGE_FILE} -o ${REPORT_FOLDER}

check_coverage
if [ $? -ne 0 ]; then
    RET=1
fi

rm -rf ${COVERAGE_FILE}
rm -rf ${COVERAGE_FILE}_tmp

exit $RET