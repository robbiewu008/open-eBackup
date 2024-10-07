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
    MODULE_ROOT_PATH=${CURRENT_DIR}/../../Module
fi
if [ ! -d "${MODULE_ROOT_PATH}" ];then
    echo "ERROR" "Module path no exist"
    exit 1
fi
MODULE_ROOT_PATH=$(cd "${MODULE_ROOT_PATH}"; pwd)
FRAMEWORK_HOME=$(cd "${SCRIPT_PATH}/.."; pwd)
DT_UTILS_DIR=$(cd "${MODULE_ROOT_PATH}/dt_utils"; pwd)
TEST_DIR=$(cd "${FRAMEWORK_HOME}/test"; pwd)

CPPC="g++"
CC="gcc"
cc="gcc"
CFLAGS="-g $ASAN_CFLAGS -pipe -fpic -DLINUX $KMC_OPT"
cFLAGS=${CFLAGS}
OFLAGS="-g $ASAN_CFLAGS -luuid"
oFLAGS="-g $ASAN_CFLAGS -fprofile-arcs -ftest-coverage"
DFLAGS="-g $ASAN_CFLAGS -shared -luuid"
dFLAGS="-g $ASAN_CFLAGS -shared"

sh ${DT_UTILS_DIR}/build_gmock.sh "$@"
if [ $? -ne 0 ];then
    exit 1
fi

export CPPC CC cc CFLAGS cFLAGS OFLAGS oFLAGS DFLAGS dFLAGS

cd ${TEST_DIR}
mkdir -p bin
mkdir -p log

sh ${TEST_DIR}/build.sh
if [ $? -ne 0 ];then
    exit 1
fi

echo "export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}"
if [ "$1" = "gdb" ]; then
    gdb ${TEST_DIR}/build/framework_test $*
else 
    ${TEST_DIR}/build/framework_test $*
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
lcov --rc lcov_branch_coverage=1 -c -d build/framework_src -o ${COVERAGE_FILE}_tmp
srcPath=$(find ${FRAMEWORK_HOME}/src type -d -maxdepth 1 | grep -v thrift_interface)
for path in ${srcPath[*]}
do
    result[${#result[*]}]=${path}/*
done
lcov --rc lcov_branch_coverage=1 -e ${COVERAGE_FILE}_tmp ${result[*]} -o ${COVERAGE_FILE} > coverage.log
genhtml --rc genhtml_branch_coverage=1 ${COVERAGE_FILE} -o ${REPORT_FOLDER}

check_coverage
if [ $? -ne 0 ]; then
    RET=$?
fi

rm -rf ${COVERAGE_FILE}
rm -rf ${COVERAGE_FILE}_tmp

exit $RET