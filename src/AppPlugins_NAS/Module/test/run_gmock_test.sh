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
DT_UTILS_DIR=$(cd "${SCRIPT_PATH}/../dt_utils"; pwd)
MODULE_HOME=$(cd "${SCRIPT_PATH}/.."; pwd)
TEST_DIR=$(cd "${MODULE_HOME}/test"; pwd)

CPPC="g++"
CC="gcc"
cc="gcc"
CFLAGS="-g $ASAN_CFLAGS -pipe -fpic -DLINUX $KMC_OPT -std=c++1z"
cFLAGS=${CFLAGS}
OFLAGS="-g $ASAN_CFLAGS -luuid"
oFLAGS="-g $ASAN_CFLAGS -fprofile-arcs -ftest-coverage -std=c++1z"
DFLAGS="-g $ASAN_CFLAGS -shared -luuid"
dFLAGS="-g $ASAN_CFLAGS -shared"

sh ${DT_UTILS_DIR}/build_gmock.sh "$@"
if [ $? -ne 0 ];then
    exit 1
fi

export CPPC CC cc CFLAGS cFLAGS OFLAGS oFLAGS DFLAGS dFLAGS

COMPONENT_BASE_DIR="${TEST_DIR}/build"
export COMPONENT_BASE_DIR

cd ${TEST_DIR}
mkdir -p log

sh ${TEST_DIR}/build.sh "$@"
if [ $? -ne 0 ];then
    exit 1
fi

cd ${COMPONENT_BASE_DIR}
echo "export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}"
if [ "$1" = "gdb" ]; then
    gdb ${TEST_DIR}/build/test_module $*
else 
    ${TEST_DIR}/build/test_module $*
fi
RET=$?

which lcov
if [ $? -ne 0 ]; then
    exit $RET
fi

echo "COMPONENT_BASE_DIR: " ${COMPONENT_BASE_DIR}
COVERAGE_FILE=coverage_module.info
REPORT_FOLDER=coverage_report_module
lcov --rc lcov_branch_coverage=1 -c -d ${MODULE_HOME} -o ${COVERAGE_FILE}_tmp
lcov --rc lcov_branch_coverage=1 -e ${COVERAGE_FILE}_tmp  "*/src/data_plane/*" -o ${COVERAGE_FILE}_intermediate
lcov --rc lcov_branch_coverage=1 --remove ${COVERAGE_FILE}_intermediate '*/test/*' '*DataPlaneServer*' '*Session*' -o ${COVERAGE_FILE}
genhtml --rc genhtml_branch_coverage=1 ${COVERAGE_FILE} -o ${REPORT_FOLDER}

rm -rf ${TEST_DIR}/coverage_report_module
cp -rf coverage_report_module ${TEST_DIR}/coverage_report_module

exit $RET