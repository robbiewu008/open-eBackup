#!/usr/bin
########################################
#  This file is part of the open-eBackup project.
# Copyright (c) 2024 Huawei Technologies Co.,Ltd.
#
# open-eBackup is licensed under MPL v2.
# You can use this software according to the terms and conditions of the MPL v2.
# You may obtain a copy of MPL v2 at:
#
#          https://www.mozilla.org/en-US/MPL/2.0
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the MPL v2 for more details.
########################################
set -x

# env params
export WORKSPACE=${1:-"/usr1/"} # 工作目录
export code_path=${2:-"${WORKSPACE}/REST_API"}  # 开源代码存放路径
export binary_path=${3:-"${WORKSPACE}/open-source-obligation"} # 开源二进制文件存放路径
export gui_code_path=${4:-"${WORKSPACE}/gui"} # gui代码存放路径
export tag_image="release"
export BUILD_TYPE="release"
export MS_IMAGE_TAG=${5:-"1.6.RC2.010"}
export Version=${6:-"1.6.RC2"}
export INTERNAL_VERSION=${7:-"1.6.0-RC2"}
export Compile_image="Y"
export OPENSOURCE_BUILD="Y"

# 外置插件包编译
cd ${code_path}/src
sh ProtectAgent/component/protectagent/protectagent/Agent/ci/script/build_pkg_opensource.sh || exit 1

# 镜像模块编译

cd ${code_path}/src
sh Infrastructure_OM/infrastructure/script/build_opensource.sh mspkg || exit 1

cd ${code_path}/src
sh ProtectManager/CI/script/build_opensource.sh ${binary_path}/ "$@" || exit 1

cd ${code_path}/src
sh DataMoveEngine/build/build_opensource.sh ${binary_path}/ || exit 1


# build final pkg
cd ${code_path}/src


# INF & PM 大包
# open-eBackup_1.x.0_MasterServer.tgz
export BUILD_MODULE=system_pm
export BUILD_PKG_TYPE=OpenSource

# 镜像制作
sh DPAProduct/CI/script/Package_100P.sh || exit 1

# DME 大包
# open-eBackup_1.x.0_MediaServer.tgz
export BUILD_MODULE=system_dme
export BUILD_PKG_TYPE=OpenSource

# 镜像制作
sh DPAProduct/CI/script/Package_100P.sh || exit 1

