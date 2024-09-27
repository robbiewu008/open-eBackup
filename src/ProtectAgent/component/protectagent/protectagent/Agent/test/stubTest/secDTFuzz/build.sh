#!/bin/sh
set -x
 
CUR_PATH="$(cd "$(dirname "$BASH_SOURCE")" && pwd)"
BUILD_PATH=${CUR_PATH}/../../../build
TEST_BUILD_PATH=${CUR_PATH}/../build
HOME=${CUR_PATH}/../../../..
export AGENT_ROOT=${HOME}/Agent
export PATH=.:${PATH}:${AGENT_ROOT}/bin
export BUILD_CMAKE=OFF
OPEN_SRC_PATH=${AGENT_ROOT}/open_src
 
if [ -z ${LD_LIBRARY_PATH} ]; then
    export LD_LIBRARY_PATH=${AGENT_ROOT}/bin
else
    export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${AGENT_ROOT}/bin
fi
 
export cmc_user=p_ciArtifact
export cmc_pwd="encryption:ETMsDgAAAYWZguqfABFBRVMvR0NNL05vUGFkZGluZwCAABAAEBMG1JSYl+HNdWoh2xTsIOoAAAAqylgeKzik6xoE+eMga6I3TrTiY9lcodqK86EW4waRd53dbSqXZ5O2E/ruABTp7d8K52StBves9rACbK+2rWBlvA=="
export opensource_user=p_ciOpenSource
export opensource_pwd="encryption:ETMsDgAAAYYMPEgSABFBRVMvR0NNL05vUGFkZGluZwCAABAAEFJn+pOPurvRnoPftH426+8AAAAwAIXLEokPz/bIzvi4yb1DJdM92prAxA/CYDK8U/OB5tDDI4sxFhoBnsz1B8Lht5SfABSlwcVBRbZYP36QkvBvjXvC/wmvnQ=="
 
main()
{
    chmod -R 777 ${HOME}
    export OPENSOURCE_BRANCH=master_backup_software_1.5.0RC1
    cd ${BUILD_PATH}
    sh download_opensrc.sh
    sed -i 's/tar cJf/# tar cJf/' ${BUILD_PATH}/agent_pack_common.sh
    sed -i 's/Create_python_executalbe_file$/# Create_python_executalbe_file/' ${BUILD_PATH}/agent_make.sh
    sh agent_pack_backup.sh
    if [ $? -ne 0 ]; then
        echo "################## build agent src failed ! ##################"
        exit 1
    fi
    cd ${TEST_BUILD_PATH}
    sh test_clean.sh
    sh test_make.sh coverage
    if [ $? -ne 0 ]; then
        echo "################## build agent test failed ! ##################"
        exit 1
    fi
    cd ${TEST_BUILD_PATH}/../bin
    sh gen_llt_coverage.sh
    if [ $? -ne 0 ]; then
        echo "################## gen llt coverage failed ! ##################"
        exit 1
    fi
    cd ${TEST_BUILD_PATH}
    sh test_make.sh fuzz
    if [ $? -ne 0 ]; then
        echo "################## build agent fuzz failed ! ##################"
        exit 1
    fi
    cp ${TEST_BUILD_PATH}/../bin/Fuzz* /out
    export LD_LIBRARY_PATH=${OPEN_SRC_PATH}/gperftools/.libs:${OPEN_SRC_PATH}/libevent/.libs:${LD_LIBRARY_PATH}
    cp -rf ${OPEN_SRC_PATH}/gperftools/.libs/libtcmalloc.so* /SecDTFuzz/lib
    cp -rf ${OPEN_SRC_PATH}/libevent/.libs/libevent*.so* /SecDTFuzz/lib
}
 
main $@