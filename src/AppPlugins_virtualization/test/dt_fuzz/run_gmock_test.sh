#/bin/sh
/# 
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
# /

SCRIPT_PATH=$(dirname ${BASH_SOURCE[0]})
CURRENT_DIR=$(cd "${SCRIPT_PATH}"; pwd)
if [ ! -d "${MODULE_ROOT_PATH}" ];then
    MODULE_ROOT_PATH=${CURRENT_DIR}/../../../../Module
fi
if [ ! -d "${MODULE_ROOT_PATH}" ];then
    echo "ERROR" "Module path no exist"
    exit 1
fi
MODULE_ROOT_PATH=$(cd "${MODULE_ROOT_PATH}"; pwd)
FRAME_WORK_PATH=$(cd "${MODULE_ROOT_PATH}/../framework"; pwd)
BACKUP_HOME=$(cd "${SCRIPT_PATH}/../.."; pwd)
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

sh ${DT_UTILS_DIR}/build_gmock.sh "$@"
if [ $? -ne 0 ];then
    exit 1
fi
type="$1"
if [ "${type}" = "DTFUZZ" ]; then
    echo "This is DTFUZZ compile"
    if [ ! -d "${DT_UTILS_DIR}/secodefuzz/libsecodefuzz" ]; then
        dos2unix ${DT_UTILS_DIR}/download_dtfuzz.sh
        chmod u+x ${DT_UTILS_DIR}/download_dtfuzz.sh
        sh ${DT_UTILS_DIR}/download_dtfuzz.sh
        if [ $? -ne 0 ];then
            echo "ERROR" "Download [${DT_UTILS_DIR}/secodefuzz] failed"
            exit 1
        fi
    fi

    dos2unix ${DT_UTILS_DIR}/build_fuzz.sh
    chmod u+x ${DT_UTILS_DIR}/build_fuzz.sh
    sh ${DT_UTILS_DIR}/build_fuzz.sh
    if [ $? -ne 0 ];then
        echo "ERROR" "Compile Osecodefuzz[${DT_UTILS_DIR}/secodefuzz] failed"
        exit 1
    fi
fi
export CPPC CC cc CFLAGS cFLAGS OFLAGS oFLAGS DFLAGS dFLAGS

cd ${TEST_DIR}
mkdir -p bin
mkdir -p log

sh ${TEST_DIR}/dt_fuzz/build.sh "$@"
if [ $? -ne 0 ]; then
    exit 1
fi

echo "export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}"
export LD_LIBRARY_PATH="${FRAME_WORK_PATH}/lib/3rd":"${FRAME_WORK_PATH}/lib/agent_sdk":"${FRAME_WORK_PATH}/../libs":"${FRAME_WORK_PATH}/lib"
export ASAN_OPTIONS=detect_leaks=0
if [ "$1" = "gdb" ]; then
    gdb ${TEST_DIR}/build/virtualization_plugin_test $*
elif [ "${type}" = "DTFUZZ" ]; then
    LD_PRELOAD=/usr/lib64/libasan.so.4 ${TEST_DIR}/build/virtualization_plugin_test $*
else
    ${TEST_DIR}/build/virtualization_plugin_test $*
fi
RET=$?

which lcov
if [ $? -ne 0 ]; then
    echo "ERROR" "The lcov tool is not found."
    exit $RET
fi

COMPONENT_BASE_DIR="${TEST_DIR}"
export COMPONENT_BASE_DIR
echo "COMPONENT_BASE_DIR: " ${COMPONENT_BASE_DIR}
cd ${COMPONENT_BASE_DIR}
COVERAGE_FILE=coverage_virtual.info
REPORT_FOLDER=coverage_report_virtual
rm -rf ${REPORT_FOLDER}

lcov --rc lcov_branch_coverage=1 -c -d build -o ${COVERAGE_FILE}_tmp
srcPath=$(find ${BACKUP_HOME}/src type -d -maxdepth 1)
for path in ${srcPath[*]}
do
    result[${#result[*]}]=${path}/*
done
lcov --rc lcov_branch_coverage=1 -e ${COVERAGE_FILE}_tmp ${result[*]} -o ${COVERAGE_FILE}
genhtml --rc genhtml_branch_coverage=1 ${COVERAGE_FILE} -o ${REPORT_FOLDER}

rm -rf ${TEST_DIR}/coverage_report_virtual
cp -rf coverage_report_virtual ${TEST_DIR}/coverage_report_virtual

exit $RET