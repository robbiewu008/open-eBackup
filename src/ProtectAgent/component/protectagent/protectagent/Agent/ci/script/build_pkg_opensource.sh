#!/usr/bin/env bash
set -ex
export CLOUD_BUILD_WORKSPACE="/usr1"
export branch="debug_OceanProtect_DataBackup_1.6.0_openeBackup"
export FusionCompute_Branch="$branch"
export Virtualization_Branch="$branch"
export product_branch="$branch"
export open_code_path="${CLOUD_BUILD_WORKSPACE}/REST_API/${Service_Name}"
export open_bin_path="${CLOUD_BUILD_WORKSPACE}/open-source-obligation/${Service_Name}"
export code_path="${CLOUD_BUILD_WORKSPACE}/REST_API"
export binary_path="${CLOUD_BUILD_WORKSPACE}/open-source-obligation"

export Version=1.6.RC2
export componentVersion=1.1.0
export HADOOP_BRANCH="$branch"
export GENERALDB_BRANCH="$branch"
export FILEPLUGIN_BRANCH="$branch"
export VIRTUALIZATION_BRANCH="$branch"
export FUSIONCOMPUTE_BRANCH="$branch"
export BUILD_TYPE=release

# file build_opensource
cd ${WORKSPACE}/AppPlugins_NAS/plugins/file/CI/script
sh copy_code.sh ${code_path}/
sh copy_bin.sh ${binary_path}/

# hadoop build 
cd ${CLOUD_BUILD_WORKSPACE}/ProtectAgent/AppPlugins_Hadoop/build
sed -i "s#<mirrorOf>central</mirrorOf>#<mirrorOf>central,apache release,apache.snapshots,java.net.Releases,jvnet-nexus-snapshots</mirrorOf>#g" /opt/buildtools/apache_maven*/conf/settings.xml || true
sh copy_code.sh ${code_path}/
sh copy_bin.sh ${binary_path}/

# genaradb build 
cd ${WORKSPACE}/AppPlugins_NAS/plugins/database/CI/script
sh copy_code.sh ${code_path}/
sh copy_bin.sh ${binary_path}/

# vir
cd ${WORKSPACE}/AppPlugins_NAS/plugins/virtualization/plugins/virtualization/build
sh copy_code.sh ${code_path}/
sh copy_bin.sh ${binary_path}/

export WORKSPACE="$CLOUD_BUILD_WORKSPACE"/REST_API/ProtectAgent/component/protectagent
sh "$WORKSPACE"/Agent/ci/script/build_opensource.sh sanclient
sh "$WORKSPACE"/Agent/ci/script/build_opensource.sh arm aarch64 sdk
sh "$WORKSPACE"/Agent/ci/script/build_opensource.sh arm ${BUILD_TYPE}
sh Agent/ci/script/build_pkg.sh OpenSource