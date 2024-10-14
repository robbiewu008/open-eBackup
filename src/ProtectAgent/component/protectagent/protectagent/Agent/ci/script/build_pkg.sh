#!/bin/bash

BUILD_PKG_TYPE=$1
BUILD_OS_TYPE=$2

set -ex

WORKHOME=${WORKSPACE}/src/ProtectAgent/component/protectagent/protectagent
cd ${WORKHOME}/Agent/ci/script

#set bep
if [ "${BEP}" == "YES" ]; then
	echo "start set Bep_Time!"
	source bep_env.sh -s ${WORKHOME}/Agent/ci/conf/bep_env.conf
fi

# 下载第三方软件 dataturbo 
rm -rf ${WORKHOME}/dataturbo_pkg

if [ "${BUILD_PKG_TYPE}" != "OpenSource" ]; then
	mkdir -p ${WORKHOME}/dataturbo_pkg
	artget pull -d ${WORKHOME}/ProductComm_DoradoAA/CI/conf/cmc/dataturbo/cmc_cmptversion.xml -ap ${WORKHOME}/dataturbo_pkg -user ${cmc_user} -pwd ${cmc_pwd}
	if [ $? -ne 0 ]; then
		echo "CMC download dataturbo faild"
    	exit 1
	else
    	mv ${WORKHOME}/dataturbo_pkg/OceanStor_DataTurbo_*_Linux.zip ${WORKHOME}/dataturbo_pkg/dataturbo.zip
		mv ${WORKHOME}/dataturbo_pkg/OceanStor_DataTurbo_*_Windows.zip ${WORKHOME}/dataturbo_pkg/dataturbo-windows.zip
	fi
fi

sh ci_build_dir.sh ${BUILD_PKG_TYPE} ${BUILD_OS_TYPE}
mkdir -p ${WORKHOME}/temp

if [ "${BUILD_PKG_TYPE}" != "OpenSource" ]; then
	artget pull -d ${WORKHOME}/Agent/ci/LCRP/conf/dependency_client.xml -p "{'AGENT_BRANCH':'${AGENT_BRANCH}','componentVersion':'${componentVersion}','PKG_TYPE':'Linux'}" -user ${cmc_user} -pwd ${cmc_pwd} -ap ${WORKSPACE}/temp/
	artget pull -d ${WORKHOME}/Agent/ci/LCRP/conf/dependency_client.xml -p "{'AGENT_BRANCH':'${AGENT_BRANCH}','componentVersion':'${componentVersion}','PKG_TYPE':'common'}" -user ${cmc_user} -pwd ${cmc_pwd} -ap ${WORKSPACE}/temp/
	artget pull -d ${WORKHOME}/Agent/ci/LCRP/conf/dependency_client.xml -p "{'AGENT_BRANCH':'${AGENT_BRANCH}','componentVersion':'${componentVersion}','PKG_TYPE':'Windows'}" -user ${cmc_user} -pwd ${cmc_pwd} -ap ${WORKSPACE}/temp/
elif [ "$BUILD_OS_TYPE" = "aarch64" ] || [ "$BUILD_OS_TYPE" = "x86_64" ]; then
	cp -rf ${WORKHOME}/../../../../open-source-obligation/dependency/Linux/* ${WORKHOME}/temp
elif [ "$BUILD_OS_TYPE" = "aix" ] || [ "$BUILD_OS_TYPE" = "solaris" ]; then
	cp -rf ${WORKHOME}/../../../../open-source-obligation/dependency/common/* ${WORKHOME}/temp
else
	cp -rf ${WORKHOME}/../../../../open-source-obligation/dependency/Linux/* ${WORKHOME}/temp
	cp -rf ${WORKHOME}/../../../../open-source-obligation/dependency/common/* ${WORKHOME}/temp
	cp -rf ${WORKHOME}/../../../../open-source-obligation/dependency/Windows/* ${WORKHOME}/temp
fi
sh ci_upload.sh ${BUILD_PKG_TYPE}