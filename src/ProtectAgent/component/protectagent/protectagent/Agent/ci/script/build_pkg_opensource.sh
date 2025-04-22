#!/usr/bin/env bash
set -ex
export CLOUD_BUILD_WORKSPACE="/usr1"
export branch="debug_OceanProtect_DataBackup_1.6.0_openeBackup"
export FusionCompute_Branch="$branch"
export Virtualization_Branch="$branch"
export product_branch="$branch"

export Version=1.6.RC2
export componentVersion=1.1.0
export HADOOP_BRANCH="$branch"
export GENERALDB_BRANCH="$branch"
export FILEPLUGIN_BRANCH="$branch"
export VIRTUALIZATION_BRANCH="$branch"
export FUSIONCOMPUTE_BRANCH="$branch"
export BUILD_TYPE=release

CURRENT_DIR=$(cd "$(dirname $0)" && pwd)
export AGENT_CODE_HOME=$(readlink -f "${CURRENT_DIR}/../../../")
echo AGENT_CODE_HOME=$AGENT_CODE_HOME

# file build_opensource
cd ${code_path}/src/AppPlugins/file/CI/script/
#sh pack_opensource.sh
#cp -rf ${code_path}/src/AppPlugins/common/framework/output_pkg/FilePlugin*.tar.xz  ${binary_path}/Plugins/Linux/
echo "FilePlugin build success."

# genaradb build 
cd ${code_path}/src/AppPlugins/database/CI/script/
#sh pack_opensource.sh
#cp -rf ${code_path}/src/AppPlugins/common/framework/output_pkg/GeneralDBPlugin*.tar.xz  ${binary_path}/Plugins/Linux/
echo "GeneralDBPlugin build success."

# vir
cd ${code_path}/src/AppPlugins/virtualization/CI/scripts/
sh pack_opensource.sh
cp -rf ${code_path}/src/AppPlugins/common/framework/output_pkg/Virtualization*.tar.xz  ${binary_path}/Plugins/Linux/$(uname -m)/
echo "Virtualization build success."

if [ $(uname -m) == "x86_64" ]; then
    echo "This is a x86_64 system."
    sh "$code_path"/src/ProtectAgent/component/protectagent/protectagent/Agent/ci/script/build_opensource.sh x86 ${BUILD_TYPE}
    sh "$code_path"/src/ProtectAgent/component/protectagent/protectagent/Agent/ci/script/build_pkg.sh OpenSource x86_64
elif [ $(uname -m) == "aarch64" ]; then
    echo "This is an aarch64 system."
    sh "$code_path"/src/ProtectAgent/component/protectagent/protectagent/Agent/ci/script/build_opensource.sh arm ${BUILD_TYPE}
    sh "$code_path"/src/ProtectAgent/component/protectagent/protectagent/Agent/ci/script/build_pkg.sh OpenSource aarch64
fi
