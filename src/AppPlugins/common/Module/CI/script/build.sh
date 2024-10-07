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
BUILD_ROOT_DIR=$(cd "${CURRENT_DIR}/../../build"; pwd)
# 下载 open_src 和 platform
sh ${BUILD_ROOT_DIR}/download_3rd.sh
if [ $? -ne 0 ];then
    echo "Downlaod 3rd failed"
    exit 1
fi

# 编译Module
if [ ${CENTOS} == "6" ]; then 
    NAS=OFF
else
    NAS=ON
fi
sh ${BUILD_ROOT_DIR}/build_module.sh $@ --NAS=${NAS}
if [ $? -ne 0 ];then
    echo "Build module failed"
    exit 1
fi

echo "Compile module success"
