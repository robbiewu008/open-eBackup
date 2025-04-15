#!bin/bash
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
BASE_PATH=${CUR_PATH}/../..
Service_Name="ProtectManager"
REPO_PATH=$1
STEP_LEVEL=$2
BUILD_PKG_TYPE=$3
PRODUCT_IMAGE_PATH=$4

#set bep
if [ "${BEP}" == "YES" ]; then
	echo "start set Bep_Time!"
	source bep_env.sh -s ${BASE_PATH}/CI/conf/bep_env.conf
fi

echo LCRP_HOME=${LCRP_HOME}
cp -rf ${BASE_PATH}/CI/LCRP/conf/Setting.xml ${LCRP_HOME}/conf/

#获取源码信息
echo "Getting Source Code Information "
sh ${CUR_PATH}/SourceCodeFile.sh

function compile() {
	mvn --version
	echo "RUN:mvn dependency:tree"
	cd ${BASE_PATH}/component/PM_Common/
	pwd
	mkdir -p ${BASE_PATH}/pkg/mspkg
	PM_MS_LIST="PM_GUI PM_System_Base_Common_Service PM_Data_Protection_Service PM_Nginx PM_Database_Version_Migration PM_Config"
	for pmservice in ${PM_MS_LIST}; do
		echo "start compile ${pmservice}!"
		if [ "${pmservice}" == "PM_System_Base_Common_Service" ]; then
			cd ${BASE_PATH}/component/${pmservice}/CI
			sh build_opensource.sh "${AGENT_BRANCH}" "${STEP_LEVEL}" "${BUILD_PKG_TYPE}" "${REPO_PATH}/ProtectManager"
			if [ $? -ne 0 ]; then
				echo "${pmservice} compile failed"
				exit 1
			fi
		elif [ "${pmservice}" == "PM_GUI" ]; then
			cd ${BASE_PATH}/component/${pmservice}/CI
			sh build_opensource.sh "${REPO_PATH}/ProtectManager"
			if [ $? -ne 0 ]; then
				echo "${pmservice} compile failed"
				exit 1
			fi
		else 
			local L_COMPONENTS_DIR="${BASE_PATH}/component"
			if [ -d ${L_COMPONENTS_DIR}/${pmservice}/pkg ]; then
				cp  ${REPO_PATH}/ProtectManager/${pmservice}.tar.gz ${L_COMPONENTS_DIR}/${pmservice}/pkg/
			fi
		  cp ${REPO_PATH}/ProtectManager/${pmservice}.tar.gz  ${BASE_PATH}/pkg/mspkg/
		fi
	done
}

function compile_base_images() {
  echo "start compile PM_System_Base_Common_Service"
  # 单独编译不影响
  echo "=========== compile_base_images start to exec all ========="
  ms_compile_base_pre
  ms_compile_base
  NAME=$(ls "${BASE_PATH}/component/${Service_Name}/pkg")
  MS_NAME=$(basename $NAME .tar.gz)
  compile_image "${MS_NAME}" "${BUILD_PKG_TYPE}" "${PRODUCT_IMAGE_PATH}"
}

function ms_compile_base_pre() {
   mvn --version
   echo "${Service_Name}:RUN:mvn dependency:tree"
   cd ${BASE_PATH}/component/${Service_Name}/src
   pwd
   mvn dependency:tree
   echo "compile gui first"
   cd ${BASE_PATH}/component/PM_GUI/src/service/console
   node parse-omrp.js
   if [ $? -ne 0 ]; then
     echo "GUI omrp failed"
     exit 1
   fi
}

function ms_compile_base() {
    cd ${BASE_PATH}/component/${Service_Name}/CI
		sh build.sh "${AGENT_BRANCH}" "${STEP_LEVEL}" "${BUILD_PKG_TYPE}"
		if [ $? -ne 0 ]; then
			echo "${Service_Name} compile failed"
			exit 1
		fi
}


function ms_compile() {
	echo "start compile $Service_Name"
	cd ${BASE_PATH}/component/${Service_Name}/CI
	sh build.sh
	if [ $? -ne 0 ]; then
		echo "${Service_Name} compile failed"
		exit 1
	fi
}

function compile_image() {
	if [[ "${Compile_image}" == "Y" ]]; then
		sh ${BASE_PATH}/CI/script/build_image_opensource.sh
		if [ $? -ne 0 ]; then
			echo "docker images build failed!"
			exit 1
		fi
	fi
}

function main() {
  mkdir ${BASE_PATH}/repo
  tar -zxvf ${REPO_PATH}/ProtectManager/PM_MAVEN.tar.gz -C ${BASE_PATH}/repo
	echo "compile PM_Boot_Dependencies before build"
	cd ${BASE_PATH}/component/PM_Boot_Dependencies
	mvn clean install -gs ${BASE_PATH}/CI/conf/settings.xml
	if [ $? -ne 0 ]; then
		echo "PM_Boot_Dependencies mvn compile failed!"
		exit 1
	fi

	if [[ "${Service_Name}" == "ProtectManager" ]] || [[ -z ${Service_Name} ]]; then
		compile
		compile_image
	elif [[ "${Service_Name}" == "PM_System_Base_Common_Service" ]]; then
	  compile_base_images
	else
		ms_compile
		NAME=$(ls "${BASE_PATH}/component/${Service_Name}/pkg")
		MS_NAME=$(basename $NAME .tar.gz)
		compile_image "${MS_NAME}"
	fi

}

echo "#########################################################"
echo "   Begin compile ProtectManager  "
echo "#########################################################"

main

echo "#########################################################"
echo "   ProtectManager package Success  "
echo "#########################################################"
