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

#
# Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
#

CUR_PATH=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
PM_MS_DIR=${CUR_PATH}/..
BASE_PATH=${PM_MS_DIR}/../..
AGENT_BRANCH=$1
STEP_LEVEL="$2"
BUILD_PKG_TYPE=$3
BIN_PATH=$4


function borrow_package(){
    # 解压PM_System_Base_Service.tar.gz
    echo "=========== start to borrow PM_System_Base_Service.tar.gz ==========="
    mkdir -p ${PM_MS_DIR}/tmp/
    tar -zxvf ${BIN_PATH}/PM_System_Base_Service.tar.gz -C ${PM_MS_DIR}/tmp
    cp ${BASE_PATH}/../ProtectAgent/component/protectagent/protectagent/final_pkg/DataProtect_*_client.zip ${PM_MS_DIR}/tmp
    echo "=========== Borrow PM_System_Base_Service.tar.gz success ==========="
}

function build_package(){
    echo "=========== Build package start ========="
    cd ${PM_MS_DIR}/src/pm-main-server
    build_base
    cd ${PM_MS_DIR}/tmp
    if [ -f "PM_System_Base_Service.tar.gz" ]; then
        rm -f PM_System_Base_Service.tar.gz
    fi
    cp ${PM_MS_DIR}/src/pm-main-server/target/pm-main-server.jar ${PM_MS_DIR}/tmp

    find "${PM_MS_DIR}/tmp" -type d | xargs chmod 700
    find "${PM_MS_DIR}/tmp" -type f | xargs chmod 550
    find ${PM_MS_DIR}/tmp/DataProtect_* -type f | xargs chmod 600

    # 重新压缩（含kmc库 和 Agent相关文件）
    tar -zcvf PM_System_Base_Service.tar.gz * --format=gnu

    mkdir -p ${PM_MS_DIR}/pkg/
    cp -f ${PM_MS_DIR}/tmp/PM_System_Base_Service.tar.gz  ${PM_MS_DIR}/pkg/
    if [ $? -ne 0 ]; then
      echo "copy pkg failed."
      exit 1
    fi
    echo "=========== Build package success ==========="
}

function build_base(){
  echo "start build"
  # 编译base代码工程
	if [ 'OceanCyber' == "${BUILD_PKG_TYPE}" ]; then
	  mvn -T 16 -Pocean-cyber install -nsu -DskipTests -Dkmc.build.enabled=true -gs ${BASE_PATH}/CI/conf/settings.xml
	else
	  mvn -T 16 -Preal clean install -nsu -DskipTests -Dkmc.build.enabled=true -gs ${BASE_PATH}/CI/conf/settings.xml
	fi
	if [ $? -ne 0 ]; then
		echo "maven compile failed."
		exit 1
	fi
}

function main(){
    echo "=========== start to PM_System_Base_Service.tar.gz , STEP_LEVEL=(${STEP_LEVEL})========="
    echo "=========== start to exec all ========="
    tar -zxvf ${BIN_PATH}/pm-system-base-jar.tar.gz -C ${PM_MS_DIR}/src
    cd ${PM_MS_DIR}/src/pm-common
    build_base
    cd ${PM_MS_DIR}/src/pm-framework/pm-access-framework
    build_base
    cd ${PM_MS_DIR}/src/pm-framework/pm-access-provider-sdk
    build_base
    cd ${PM_MS_DIR}/src/pm-plugins/pm-cnware-protection-plugin
    build_base
    cd ${PM_MS_DIR}/src/pm-plugins/pm-database-plugins
    build_base
    cd ${PM_MS_DIR}/src/pm-plugins/pm-file-protection-plugin
    build_base
    cd ${PM_MS_DIR}/src/pm-plugins/pm-openstack-plugins
    build_base
    cd ${PM_MS_DIR}/src/pm-plugins/pm-k8s-csi-protection-plugin
    build_base
    borrow_package
    build_package
   echo "=========== build PM_System_Base_Service.tar.gz success ========="
}

export buildNumber=$(date +%Y%m%d%H%M%S)

if [[ ${releaseVersion} ]]; then
	PackageVersion=${releaseVersion}
	echo "buildVersion=${releaseVersion}" > ${PM_MS_DIR}/buildInfo.properties
else
	serviceVersion=`cat ${PM_MS_DIR}/app_define.json | awk -F '\"version":' '{print $2}' | awk -F ',' '{print $1}' | sed 's/\"//g'`
	PackageVersion=${serviceVersion}.${buildNumber}
	echo "buildVersion=${PackageVersion}" > ${PM_MS_DIR}/buildInfo.properties
fi

main $@

