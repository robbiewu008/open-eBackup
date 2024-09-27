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

TEST_ROOT_DIR=$(cd $(dirname ${BASH_SOURCE[0]});pwd)

function main()
{
    mkdir -p ${TEST_ROOT_DIR}/log
    if [ "$1" == "clean" ]; then
        pushd ${TEST_ROOT_DIR}/
        sh build.sh clean
        popd
        hdt clean ./
        rm -rf ${TEST_ROOT_DIR}/log
        return 0
    fi

    pushd ${TEST_ROOT_DIR}
    sh build.sh
    popd

    cd ${TEST_ROOT_DIR}
    if [ "$1" == "gdb" ]; then
        hdt test -c on -d on -s off -vvv .
    else
        hdt test -c on -vvv -s off .
    fi
    cd -
}

main "$@"