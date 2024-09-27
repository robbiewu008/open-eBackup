#!/bin/bash
/# 
#  This file is a part of the open-eBackup project.
#  This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
#  If a copy of the MPL was not distributed with this file, You can obtain one at
#  http://mozilla.org/MPL/2.0/.
# 
#  Copyright (c) [2024] Huawei Technologies Co.,Ltd.
# 
#  THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
#  EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
#  MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# /
# 初始化环境变量
source ./common_opensource.sh

EBACK_BASE_DIR="$(cd "$(dirname "$BASH_SOURCE")/../";pwd)"
CI_PATH="${EBACK_BASE_DIR}/../../CI/script"

CODE_PATH=$1
# 检查参数是否为空
if [ -z "$CODE_PATH" ]; then
    echo "Code path is empty, please input the code path."
    exit 1
fi

CODE_SRC_PATH=${CODE_PATH}/src

internal_source_path_list=(
    "${EBACK_BASE_DIR}/src/protect_engines/apsara_stack"
    "${EBACK_BASE_DIR}/src/protect_engines/hcs"
    "${EBACK_BASE_DIR}/src/protect_engines/kubernetes"
    "${EBACK_BASE_DIR}/src/protect_engines/hyperv"
    "${EBACK_BASE_DIR}/src/volume_handlers/fusionstorage"
    "${EBACK_BASE_DIR}/src/volume_handlers/cloud_volume/apsara_volume"
    "${EBACK_BASE_DIR}/src/volume_handlers/hyperv_volume"
    "${EBACK_BASE_DIR}/src/volume_handlers/oceanstor"
    "${EBACK_BASE_DIR}/script"
)

echo "#########################################################"
echo "   Start to copy virtualization code to opensource directory"
echo "#########################################################"

function compile_virtualization()
{
    pushd $CI_PATH
    sh pack_copy.sh
    if [ $? -ne 0 ];then
        echo -e "Compile virtualization fail"
        exit 1
    fi
    popd
}

function delete_internal_code()
{
    for path in "${internal_source_path_list[@]}"
    do
        # 递归删除所有cpp文件
        echo "find $path -name *.cpp -type f -delete"
        find $path -name "*.cpp" -type f -delete
        echo "find $path -name CMakeLists.txt -type f -delete"
        find $path -name "CMakeLists.txt" -type f -delete
        echo "find $path -name *.py -type f -delete"
        find $path -name "*.py" -type f -delete
    done
}

function replace_cmake()
{
    echo "find ${EBACK_BASE_DIR}/ -name CMakeLists_OpenSource.txt -exec sh -c mv $0 ${0%/*}/CMakeLists.txt {} \;"
    find "${EBACK_BASE_DIR}/" -name "CMakeLists_OpenSource.txt" -exec sh -c 'mv "$0" "${0%/*}/CMakeLists.txt"' {} \;
}

function copy_code()
{
    mkdir -p ${CODE_SRC_PATH}/AppPlugins_virtualization/
    cp -rf ${EBACK_BASE_DIR}/* "${CODE_SRC_PATH}/AppPlugins_virtualization/"

    return
}

function main()
{

    delete_internal_code

    replace_cmake

    copy_code

}

main "$@"
exit $?
