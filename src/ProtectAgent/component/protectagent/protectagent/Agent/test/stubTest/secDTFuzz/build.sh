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