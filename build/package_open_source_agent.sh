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
set -ex

# env params
export WORKSPACE=${1:-"/usr1/"} # 工作目录
export code_path=${2:-"${WORKSPACE}/REST_API"}  # 开源代码存放路径
export binary_path=${3:-"${WORKSPACE}/open-source-obligation"} # 开源二进制文件存放路径
export tag_image="release"
export BUILD_TYPE=${4:-"release"}
export MS_IMAGE_TAG=${5:-"1.6.RC2.010"}
export Version=${6:-"1.6.RC2"}
export INTERNAL_VERSION=${7:-"1.6.0-RC2"}
export Compile_image="Y"
export OPENSOURCE_BUILD="Y"

export CLOUD_BUILD_WORKSPACE="/usr1"
export branch="debug_OceanProtect_DataBackup_1.6.0_openeBackup"
export FusionCompute_Branch="$branch"
export Virtualization_Branch="$branch"
export product_branch="$branch"

export componentVersion=1.1.0
export HADOOP_BRANCH="$branch"

CURRENT_DIR=$(cd "$(dirname $0)" && pwd)
export AGENT_CODE_HOME=$(readlink -f "${CURRENT_DIR}/../../../")
echo AGENT_CODE_HOME=$AGENT_CODE_HOME

# 外置插件包编译
if [ $(uname -m) == "x86_64" ]; then
    echo "This is a x86_64 system."
    sh "$code_path"/src/ProtectAgent/component/protectagent/protectagent/Agent/ci/script/build_opensource.sh x86 ${BUILD_TYPE}
elif [ $(uname -m) == "aarch64" ]; then
    echo "This is an aarch64 system."
    sh "$code_path"/src/ProtectAgent/component/protectagent/protectagent/Agent/ci/script/build_opensource.sh arm ${BUILD_TYPE}
fi
