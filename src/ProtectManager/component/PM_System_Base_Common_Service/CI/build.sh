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

CUR_PATH=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
PM_MS_DIR=${CUR_PATH}/..
BASE_PATH=${PM_MS_DIR}/../..
AGENT_BRANCH=$1
STEP_LEVEL="$2"
BUILD_PKG_TYPE=$3

if [ -z "${componentVersion}" ]; then
	componentVersion="1.1.0"
fi

function download_agent(){
    echo "=========== Start download agent ==========="
  	# 下载agent安装包
  	cd ${PM_MS_DIR}/CI/conf/
  	mkdir -p ${PM_MS_DIR}/AGENT
    echo "Using ProtectAgent-Client version is: ${Version}       (from pipeline parameter)"
  	artget pull -d agent_dependency.xml \
      -p "{'componentVersion':'${componentVersion}','AGENT_BRANCH':'${AGENT_BRANCH}','PMClientVersion':'${Version}'}" \
      -ap ${PM_MS_DIR}/AGENT/ \
      -user ${cmc_user} -pwd ${cmc_pwd}
  	if [ $? -ne 0 ]; then
  		echo "download agent failed!"
  		exit 1
  	fi

    # 拷贝agent_client文件
    cp -rf ${PM_MS_DIR}/AGENT/*.zip  ${PM_MS_DIR}/tmp
    ls ${PM_MS_DIR}/tmp/
    echo "=========== Download agent success ==========="
}

function borrow_package(){
    # 解压PM_System_Base_Service.tar.gz
    echo "=========== start to borrow PM_System_Base_Service.tar.gz ==========="
    mkdir -p ${PM_MS_DIR}/tmp/
    tar -zxvf ${PM_MS_DIR}/src/pm-main-server/target/PM_System_Base_Service.tar.gz -C ${PM_MS_DIR}/tmp
    echo "=========== Borrow PM_System_Base_Service.tar.gz success ==========="
}

function download_kmc(){
    echo "=========== Start copy KMC lib ==========="
    # 将CMC DEE库上的KMC Lib拷贝下来，并解压到lib库。
    echo "Download KMC lib"
    artget pull -d ${PM_MS_DIR}/CI/conf/dependency_from_cmc.xml -ap ${PM_MS_DIR}/tmp/ -user ${cmc_user} -pwd ${cmc_pwd}
    tar zxvf ${PM_MS_DIR}/tmp/kmc-3.1.1.tar.gz -C ${PM_MS_DIR}/tmp
    if [ $? -ne 0 ]; then
      echo "Unzip kmc failed"
      exit 1
    fi
    rm -rf ${PM_MS_DIR}/tmp/kmc-3.1.1.tar.gz
    mv ${PM_MS_DIR}/tmp/release/lib ${PM_MS_DIR}/tmp/lib
    echo "=========== End copy KMC lib ==========="
}

function build_package(){
    echo "=========== Build package start ========="
    cd ${PM_MS_DIR}/tmp

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
  # 编译base代码工程
	cd ${PM_MS_DIR}/src/
	if [ 'OceanCyber' == "${BUILD_PKG_TYPE}" ]; then
	  mvn -T 16 -Pocean-cyber install -nsu -DskipTests -Dkmc.build.enabled=true -gs ${BASE_PATH}/CI/conf/settings.xml
	else
	  mvn -T 16 -Preal install -nsu -DskipTests -Dkmc.build.enabled=true -gs ${BASE_PATH}/CI/conf/settings.xml
	fi
	if [ $? -ne 0 ]; then
		echo "maven compile failed."
		exit 1
	fi
}

function main(){
  echo "=========== start to PM_System_Base_Service.tar.gz , STEP_LEVEL=(${STEP_LEVEL})========="
   if [ "${STEP_LEVEL}" == "step1" ]; then
     # 编译大包前置流程
     echo "=========== start to exec step1 ========="
     build_base
     borrow_package
     download_kmc
   elif [ "${STEP_LEVEL}" == "step2" ]; then
     # 编译大包protect_agent_Package_all 执行完后执行后置步骤
     echo "=========== start to exec step2 ========="
     download_agent
     build_package
   else
     # 单独编译不影响
     echo "=========== start to exec all ========="
     build_base
     borrow_package
     download_kmc
     download_agent
     build_package
   fi
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

