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
set +x
SYS_NAME=`uname -s`
BASE_PATH=""
if [ "${SYS_NAME}" = "AIX" ]; then
    BASE_PATH="$(cd "$(dirname $0)/../.." && pwd)"
else
    BASE_PATH="$(cd "$(dirname "$BASH_SOURCE")/../.." && pwd)"
fi
BUILD_SRC_PATH=${BASE_PATH}/build
FRAMEWORK_PATH=${BASE_PATH}/../../framework
FRAMEWORK_BUILD_PATH=${FRAMEWORK_PATH}/build
DATABESE_PLUGIN_PATH=${BASE_PATH}/applications

move_generaldb_plugin_code()
{
    # 强制覆盖上层目录
    if [ "${SYS_NAME}" = "AIX" ]; then
        \cp -R "${BASE_PATH}"/plugins/database/* "${BASE_PATH}"/
    else
        cp -rf "${BASE_PATH}"/plugins/database/* "${BASE_PATH}"/
    fi
    if [ $? -ne 0 ]; then
        echo "Failed to move generaldb code."
        exit 1
    fi
}

main()
{
    move_generaldb_plugin_code
    # 内置编译framework框架之前先修改PLUGIN_TYPE=1
    if [ ${INTERNAL_PLUGIN} = "1" ]; then
        sed -i "s/PLUGIN_TYPE=0/PLUGIN_TYPE=1/g" ${FRAMEWORK_BUILD_PATH}/common/common.sh
    fi
    sh ${FRAMEWORK_BUILD_PATH}/build.sh OPENSOURCE
    if [ $? -ne 0 ] ; then
        echo "build framework failed."
        exit 1
    fi

    sh ${BUILD_SRC_PATH}/build.sh
}

main $@