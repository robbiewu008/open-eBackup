#/bin/sh

SCRIPT_PATH=$(dirname ${BASH_SOURCE[0]})
CURRENT_DIR=$(cd "${SCRIPT_PATH}"; pwd)
source $SCRIPT_PATH/check_coverage.sh
if [ -z "${MODULE_ROOT_PATH}" ];then
    MODULE_ROOT_PATH=${CURRENT_DIR}/../../Module
fi
if [ ! -d "${MODULE_ROOT_PATH}" ];then
    MODULE_ROOT_PATH=${CURRENT_DIR}/../../../../../Module
fi
if [ ! -d "${MODULE_ROOT_PATH}" ];then
    echo "ERROR" "Module path no exist"
    exit 1
fi
MODULE_ROOT_PATH=$(cd "${MODULE_ROOT_PATH}"; pwd)
SCANNER_HOME=$(cd "${SCRIPT_PATH}/../.."; pwd)
DT_UTILS_DIR=$(cd "${MODULE_ROOT_PATH}/dt_utils"; pwd)
TEST_DIR=$(cd "${SCANNER_HOME}/test/localhost_test"; pwd)

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

cd ${TEST_DIR}
mkdir -p bin
mkdir -p log

sh ${TEST_DIR}/build.sh "$@"
if [ $? -ne 0 ];then
    exit 1
fi

export LD_LIBRARY_PATH=${MODULE_ROOT_PATH}/lib
echo "export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}"
if [ "$1" = "gdb" ]; then
    gdb ${TEST_DIR}/build/scanner_test $*
else 
    ${TEST_DIR}/build/scanner_test $*
fi
RET=$?

which lcov
if [ $? -ne 0 ]; then
    exit $RET
fi

COMPONENT_BASE_DIR=${TEST_DIR}
echo "COMPONENT_BASE_DIR: " ${COMPONENT_BASE_DIR}
cd ${COMPONENT_BASE_DIR}
COVERAGE_FILE=coverage_scanner.info
REPORT_FOLDER=coverage_report_scanner
rm -rf ${REPORT_FOLDER}
lcov --rc lcov_branch_coverage=1 -c -d build/scanner_src -o ${COVERAGE_FILE}_tmp
lcov --rc lcov_branch_coverage=1 -e ${COVERAGE_FILE}_tmp "${SCANNER_HOME}/localhost_src/*"  -o ${COVERAGE_FILE} > coverage.log
genhtml --rc genhtml_branch_coverage=1 ${COVERAGE_FILE} -o ${REPORT_FOLDER}

check_coverage
if [ $? -ne 0 ]; then
    RET=1
fi

rm -rf ${COVERAGE_FILE}
rm -rf ${COVERAGE_FILE}_tmp
exit $RET