#!/bin/bash
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

CUR_PATH=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
PM_MS_DIR=${CUR_PATH}/../..
BASE_PATH=${PM_MS_DIR}/../..
CODE_PATH=$1

function copy_code(){
    echo "=========== Start copy code ==========="
    # 拷贝文件
    cp -rf ${PM_MS_DIR}/PM_GUI ${CODE_PATH}/ProtectManager/component
    echo "=========== End copy code ==========="
}

function main(){

  copy_code

}

main $@