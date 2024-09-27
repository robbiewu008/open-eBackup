#!/bin/bash

BUILD_PKG_TYPE=$1
BUILD_OS_TYPE=$2

set -ex
cd ${WORKSPACE}/Agent/ci/script

#set bep
if [ "${BEP}" == "YES" ]; then
	echo "start set Bep_Time!"
	source bep_env.sh -s ${WORKSPACE}/Agent/ci/conf/bep_env.conf
fi

# 下载第三方软件 dataturbo 
rm -rf ${WORKSPACE}/dataturbo_pkg

if [ "${BUILD_PKG_TYPE}" != "OpenSource" ]; then
	mkdir -p ${WORKSPACE}/dataturbo_pkg
	artget pull -d ${WORKSPACE}/ProductComm_DoradoAA/CI/conf/cmc/dataturbo/cmc_cmptversion.xml -ap ${WORKSPACE}/dataturbo_pkg -user ${cmc_user} -pwd ${cmc_pwd}
	if [ $? -ne 0 ]; then
		echo "CMC download dataturbo faild"
    	exit 1
	else
    	mv ${WORKSPACE}/dataturbo_pkg/OceanStor_DataTurbo_*_Linux.zip ${WORKSPACE}/dataturbo_pkg/dataturbo.zip
		mv ${WORKSPACE}/dataturbo_pkg/OceanStor_DataTurbo_*_Windows.zip ${WORKSPACE}/dataturbo_pkg/dataturbo-windows.zip
	fi
fi

sh ci_build_dir.sh ${BUILD_PKG_TYPE} ${BUILD_OS_TYPE}
mkdir -p ${WORKSPACE}/temp

if [ "${BUILD_PKG_TYPE}" != "OpenSource" ]; then
	artget pull -d ${WORKSPACE}/Agent/ci/LCRP/conf/dependency_client.xml -p "{'AGENT_BRANCH':'${AGENT_BRANCH}','componentVersion':'${componentVersion}','PKG_TYPE':'Linux'}" -user ${cmc_user} -pwd ${cmc_pwd} -ap ${WORKSPACE}/temp/
	artget pull -d ${WORKSPACE}/Agent/ci/LCRP/conf/dependency_client.xml -p "{'AGENT_BRANCH':'${AGENT_BRANCH}','componentVersion':'${componentVersion}','PKG_TYPE':'common'}" -user ${cmc_user} -pwd ${cmc_pwd} -ap ${WORKSPACE}/temp/
	artget pull -d ${WORKSPACE}/Agent/ci/LCRP/conf/dependency_client.xml -p "{'AGENT_BRANCH':'${AGENT_BRANCH}','componentVersion':'${componentVersion}','PKG_TYPE':'Windows'}" -user ${cmc_user} -pwd ${cmc_pwd} -ap ${WORKSPACE}/temp/
elif [ "$BUILD_OS_TYPE" = "aarch64" ] || [ "$BUILD_OS_TYPE" = "x86_64" ]; then
	cp -rf ${WORKSPACE}/../../../../open-source-obligation/dependency/Linux/* ${WORKSPACE}/temp
elif [ "$BUILD_OS_TYPE" = "aix" ] || [ "$BUILD_OS_TYPE" = "solaris" ]; then
	cp -rf ${WORKSPACE}/../../../../open-source-obligation/dependency/common/* ${WORKSPACE}/temp
else
	cp -rf ${WORKSPACE}/../../../../open-source-obligation/dependency/Linux/* ${WORKSPACE}/temp
	cp -rf ${WORKSPACE}/../../../../open-source-obligation/dependency/common/* ${WORKSPACE}/temp
	cp -rf ${WORKSPACE}/../../../../open-source-obligation/dependency/Windows/* ${WORKSPACE}/temp
fi
sh ci_upload.sh ${BUILD_PKG_TYPE}