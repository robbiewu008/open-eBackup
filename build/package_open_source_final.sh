#!/usr/bin
set -x

# env params
export WORKSPACE=${1:-"/usr1/"} # 代码存放环境
export code_path=${2:-"${WORKSPACE}/REST_API"}
export binary_path=${3:-"${WORKSPACE}/open-source-obligation"}
export tag_image="release"
export BUILD_TYPE="release"
export MS_IMAGE_TAG=${5:"1.6.RC2.010"}
export Compile_image="Y"
export OPENSOURCE_BUILD="Y"

# 外置插件包编译
cd ${WORKSPACE}/REST_API/src
sh ProtectAgent/component/protectagent/protectagent/Agent/ci/script/build_pkg_opensource.sh

# 镜像模块编译

cd ${WORKSPACE}/REST_API/src
sh Infrastructure_OM/infrastructure/script/build_opensource.sh dorado mspkg

cd ${WORKSPACE}/REST_API/src
cd DataEnableEngine/build
sh build_opensource.sh all "${binary_path}"

cd ${WORKSPACE}/REST_API/src
sh ProtectManager/CI/script/build_opensource.sh "dorado" "ProtectManager" ${binary_path}/ "$@"

cd ${WORKSPACE}/REST_API/src
sh DataMoveEngine/build/build_opensource.sh ${binary_path}/


# build final pkg
cd ${WORKSPACE}/REST_API/src
# DEE 大包
# open-eBackup_1.x.0_DataManager.tgz
export BUILD_MODULE=system_dee
export BUILD_PKG_TYPE=OpenSource

# 镜像制作
sh DPAProduct/CI/script/Package_100P.sh

# DME 大包
# open-eBackup_1.x.0_MediaServer.tgz
export BUILD_MODULE=system_dme
export BUILD_PKG_TYPE=OpenSource

# 镜像制作
sh DPAProduct/CI/script/Package_100P.sh

# INF & PM 大包
# open-eBackup_1.x.0_MasterServer.tgz
export BUILD_MODULE=system_pm
export BUILD_PKG_TYPE=OpenSource

# 镜像制作
sh DPAProduct/CI/script/Package_100P.sh
