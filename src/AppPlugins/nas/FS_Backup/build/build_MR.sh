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

SCRIPT_PATH=$(dirname ${BASH_SOURCE[0]})
CURRENT_DIR=$(cd "${SCRIPT_PATH}"; pwd)
BACKUP_HOME=$(cd "${SCRIPT_PATH}/.."; pwd)
arch_type=$(uname -m)

function download_dep()
{
    echo "Begin git Module from ${MODULE_BRANCH}"
    git clone -b ${MODULE_BRANCH} ssh://git@szv-y.codehub.huawei.com:2222/dpa/CBB/CPP/Module.git
}

MODULE_BRANCH=${Target_Branch}

download_dep
if [ "${arch_type}" == "aarch64" ]; then
    if [[ "${Target_Branch}" == "master_backup_software_1.6.0RC1" || "${Target_Branch}" == "BR_SMOKE" ]];then
        sh -x ${BACKUP_HOME}/build/src/build_backup.sh
        exit $?
    fi
    sh -x ${BACKUP_HOME}/test/run_hdt_test.sh
    exit $?
else
    sh -x ${BACKUP_HOME}/build/src/build_backup.sh
    exit $?
fi
