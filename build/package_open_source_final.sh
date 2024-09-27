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
export OPENSOURCE_BUILD="True"
export OPENSOURCE_BUILD="Y"
export product_branch=debug_OceanProtect_DataBackup_1.6.0_openeBackup_v2
export branch=${product_branch}
export DEE_BRANCH=${product_branch}
export PM_BRANCH=${product_branch}
export INF_BRANCH=${product_branch}
export CODE_BRANCH=${branch}
export  FILECLIENT_BRANCH="master_OceanProtect_DataBackup"
export archive_path="$(date +%Y%m%d)10"

<<<<<<< HEAD
# 外置插件包编译
cd ${WORKSPACE}/REST_API/src
sh ProtectAgent/component/protectagent/protectagent/Agent/ci/script/build_pkg_opensource.sh

# 镜像模块编译
=======
>>>>>>> 633902ddc410bf9a602ab6d1cbb7277bf44ffb40
cd ${WORKSPACE}/REST_API/src
sh Infrastructure_OM/infrastructure/script/build_opensource.sh dorado ${INF_BRANCH} mspkg

cd ${WORKSPACE}/REST_API/src
cd DataEnableEngine/build
sh build_opensource.sh "${DEE_BRANCH}" "${INF_BRANCH}" all "${binary_path}"

cd ${WORKSPACE}/REST_API/src
sh ProtectManager/CI/script/build_opensource.sh "dorado" "${PM_BRANCH}" "ProtectManager" ${binary_path}/ "$@"

cd ${WORKSPACE}/REST_API/src
sh DataMoveEngine/build/build_opensource.sh "${DME_BRANCH}" "${INF_BRANCH}" ${binary_path}/


# build final pkg
cd ${WORKSPACE}/REST_API/src
# DEE 大包
# open-eBackup_1.x.0_DataManager.tgz
export BUILD_MODULE=system_dee
export BUILD_PKG_TYPE=OpenSource

# 镜像制作
sh DPAProduct/CI/script/Package_100P.sh -s "${IS_SNAPSHOT}" -p "${PBI_NAME}"

# DME 大包
# open-eBackup_1.x.0_MediaServer.tgz
export BUILD_MODULE=system_dme
export BUILD_PKG_TYPE=OpenSource

# 镜像制作
sh DPAProduct/CI/script/Package_100P.sh -s "${IS_SNAPSHOT}" -p "${PBI_NAME}"

# INF & PM 大包
# open-eBackup_1.x.0_MasterServer.tgz
export BUILD_MODULE=system_pm
export BUILD_PKG_TYPE=OpenSource

# 镜像制作
sh DPAProduct/CI/script/Package_100P.sh -s "${IS_SNAPSHOT}" -p "${PBI_NAME}"
