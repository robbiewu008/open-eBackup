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
# build plugin framework
FILE_ROOT_DIR=$(cd $(dirname $0)/..; pwd)
FRAMEWORK_DIR=$(cd "${FILE_ROOT_DIR}/../common/framework"; pwd)
MODULE_PATH=$(cd "${FILE_ROOT_DIR}/../common/Module"; pwd)
build_type=$1
type=$2
build_framework()
{
    if [ -z ${MODULE_BRANCH} ];then
        if [ -z "${branch}" ];then
            MODULE_BRANCH=master_OceanProtect_DataBackup
        else
            MODULE_BRANCH=${branch}
        fi
    fi
    if [ "$(uname -s)" != "AIX" ] && [ "$(uname -s)" != "SunOS" ]; then
        if [ "X${type}" == "XOPENSOURCE" ];then
            sh ${MODULE_PATH}/build/build_module.sh "-type=${build_type}"
        else
            sh ${FRAMEWORK_DIR}/build/download_module_from_cmc.sh "${MODULE_BRANCH}"
        fi
        if [ $? -ne 0 ];then
            return 1
        fi
    else
        # AIX上Module的动态库无法从cmc下载，通过编译得到
        echo "start build module."
        sh ${MODULE_PATH}/build/build_module.sh
        if [ $? -ne 0 ];then
            return 1
        fi
    fi

    # sh ${FRAMEWORK_DIR}/build/build.sh "${build_type}"
    exit $?
}

build_framework
exit $?