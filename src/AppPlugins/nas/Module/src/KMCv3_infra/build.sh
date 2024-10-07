#!/bin/bash
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
#Compile libkmcv3.so for Framework in CBB, output is: libkmcv3.so
 
SCRIPT_PATH=$(dirname ${BASH_SOURCE[0]})
CUR_PATH=$(cd "${SCRIPT_PATH}"; pwd)
BASE_PATH=$(cd ${CUR_PATH}/../..;pwd)

function clean_temp_file ()
{
    rm -rf ${CUR_PATH}/build ${CUR_PATH}/External ${CUR_PATH}/lib
}
 
function main ()
{
    # Check 
    if [ ! -f ${BASE_PATH}/platform/KMCV3_rel/lib/libKMC.a ] || [ ! -f ${BASE_PATH}/platform/KMCV3_rel/lib/libSDP.a ]
    then
        echo "No libKMC.a or libSDP.a found, compile libkmcv3.so error"
        exit 1
    fi
 
    clean_temp_file
    # get head file
    rm -rf ${BASE_PATH}/platform/KMCv3_infra_rel
    mkdir -p ${BASE_PATH}/platform/KMCv3_infra_rel/include
    cp -rf ${CUR_PATH}/kmcv3_src/kmcv3.h ${BASE_PATH}/platform/KMCv3_infra_rel/include/
    # compile libkmcv3.so
    mkdir -p ${CUR_PATH}/build
    cd ${CUR_PATH}/build
    cmake ..
    make -j16
    if [ $? -ne 0 ] || [ ! -f ${BASE_PATH}/src/KMCv3_infra/lib/libkmcv3.so ];then
        echo "Compile libkmcv3.so failed"
        exit 2
    else
        cp -rf ${BASE_PATH}/src/KMCv3_infra/lib ${BASE_PATH}/platform/KMCv3_infra_rel/
        clean_temp_file
        echo "Compile libkmcv3.so sucessful!"
        exit 0
    fi
}
 

main