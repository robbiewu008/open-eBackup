#!bin/bash
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
CURRENT_PATH=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
BASE_PATH=${CURRENT_PATH}/../../../
BUILD_PKG_TYPE=$1

function main(){
    cd ${BASE_PATH}/Agent/build/
    cp -rf ${BASE_PATH}/Agent/ci/LCRP/conf/Setting.xml ${LCRP_HOME}/conf/
    sh build_image.sh ${AGENT_BRANCH} ${INF_BRANCH} ${BUILD_PKG_TYPE}
    if [ $? -ne 0 ];then
        echo "Image build failed."
        exit 1
    fi
}


echo "#########################################################"
echo "   Begin build ProtectAgent_Image  "
echo "#########################################################"
main
echo "#########################################################"
echo "   ProtectAgent_Image package Success  "
echo "#########################################################"