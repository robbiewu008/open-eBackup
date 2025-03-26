#/bin/sh

set +x
_CUR_PATH=$(
    cd $(dirname $0)
    pwd
)

CPPC="g++"
CC="gcc"
cc="gcc"
CFLAGS="-g $ASAN_CFLAGS -pipe -fpic -DLINUX $KMC_OPT -std=c++1z"
cFLAGS=${CFLAGS}
OFLAGS="-g $ASAN_CFLAGS -luuid"
oFLAGS="-g $ASAN_CFLAGS -fprofile-arcs -ftest-coverage -std=c++1z"
DFLAGS="-g $ASAN_CFLAGS -shared -luuid"
dFLAGS="-g $ASAN_CFLAGS -shared"
export MS_BASE_DIR=${_CUR_PATH}/..

build_gmock() {
    GMOCK_DIR=${_CUR_PATH}/third_party_software/gmock/googletest-release
    cd ${GMOCK_DIR}
    pwd
    if [ "$1" = "clean" ]; then
        rm -rf ${_CUR_PATH}/third_party_software/gmock/googletest-release/lib/*
        make clean
        return
    fi

    cmake .
    make
}

build_gmock $@

export CPPC CC cc CFLAGS cFLAGS OFLAGS oFLAGS DFLAGS dFLAGS
LIB_TEST=${MS_BASE_DIR}/test/third_party_groupware/gmock/gmock

COMPONENT_BASE_DIR="${MS_BASE_DIR}/build-cmake"

export COMPONENT_BASE_DIR

cd ${_CUR_PATH}
mkdir -p bin
mkdir -p logs
mkdir -p conf
#cp -n ${MS_BASE_DIR}/conf/* conf/ 2>/dev/null

build_test() {
    if [ "$1" = "clean" ]; then
        rm -rf build-llt
        echo "==================make clean success====================="
        exit 0
    fi
    mkdir -p build-llt
    cd ${_CUR_PATH}/build-llt
    cmake ..
    if [ $? -ne 0 ]; then
        echo "==================cmake fail====================="
        exit 1
    fi
    numProc=$(cat /proc/cpuinfo | grep processor | wc -l)
    make AGENT_VERSION=${AGENT_VERSION} AGENT_BUILD_NUM=${AGENT_BUILD_NUM} -j${numProc}
    if [ $? -ne 0 ]; then
        exit 1
    fi
    echo "==================make success====================="
    cd -
}

build_test $@

echo "export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}"
if [ "$1" = "gdb" ]; then
    gdb ${_CUR_PATH}/bin/test_scanner $*
else 
    ${_CUR_PATH}/bin/test_scanner $*
fi
RET=$?

which lcov
if [ $? -ne 0 ]; then
    exit $RET
fi

echo "COMPONENT_BASE_DIR: " ${COMPONENT_BASE_DIR}
cd ${COMPONENT_BASE_DIR}
COVERAGE_FILE=coverage_scanner.info
REPORT_FOLDER=coverage_report_scanner
lcov --rc lcov_branch_coverage=1 -c -d ./ -o ${COVERAGE_FILE}_tmp
lcov --rc lcov_branch_coverage=1 -e ${COVERAGE_FILE}_tmp "*/src/config/*" "*/src/interface/*" "*/src/scanner/*" -o ${COVERAGE_FILE}
genhtml --rc genhtml_branch_coverage=1 ${COVERAGE_FILE} -o ${REPORT_FOLDER}

rm -rf ${_CUR_PATH}/${REPORT_FOLDER}
cp -rf ${REPORT_FOLDER} ${_CUR_PATH}/${REPORT_FOLDER}

exit $RET