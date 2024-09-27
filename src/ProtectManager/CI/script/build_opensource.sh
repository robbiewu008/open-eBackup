#!bin/bash

#
# Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
#

CUR_PATH=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
BASE_PATH=${CUR_PATH}/../..
PRODUCT=$1
CODE_BRANCH=$2
Service_Name=$3
REPO_PATH=$4
STEP_LEVEL=$5
BUILD_PKG_TYPE=$6
PRODUCT_IMAGE_PATH=$7

if [ -z ${PRODUCT} ]; then
  echo "No product parameter, please specify"
	exit 1
fi

if [ -z ${CODE_BRANCH} ]; then
	echo "No branch parameter, please specify"
	exit 1
fi

if [ -z ${AGENT_BRANCH} ]; then
	AGENT_BRANCH=${CODE_BRANCH}
fi

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
	mvn dependency:tree
	PM_MS_LIST="PM_GUI PM_System_Base_Common_Service PM_Data_Protection_Service PM_Nginx PM_Database_Version_Migration PM_Config"
	for pmservice in ${PM_MS_LIST}; do
		echo "start compile ${pmservice}!"
		cd ${BASE_PATH}/component/${pmservice}/CI
		if [ "${pmservice}" == "PM_System_Base_Common_Service" ]; then
			sh build_opensource.sh "${AGENT_BRANCH}" "${STEP_LEVEL}" "${BUILD_PKG_TYPE}" "${REPO_PATH}"
			if [ $? -ne 0 ]; then
				echo "${pmservice} compile failed"
				exit 1
			fi
		elif [ "${pmservice}" == "PM_GUI" ]; then
      sh build_opensource.sh "${REPO_PATH}"
      if [ $? -ne 0 ]; then
        echo "${pmservice} compile failed"
        exit 1
      fi
		else 
		  local L_COMPONENTS_DIR="${BASE_PATH}/component"
		  if [ -d ${L_COMPONENTS_DIR}/${pmservice}/pkg ]; then
        cp  ${REPO_PATH}/${pmservice}.tar.gz ${L_COMPONENTS_DIR}/${pmservice}/pkg/
      fi
		  cp ${REPO_PATH}/${pmservice}.tar.gz  ${BASE_PATH}/pkg/mspkg
		fi
	done
}

function compile_base_images() {
  echo "start compile PM_System_Base_Common_Service"
  if [ "${STEP_LEVEL}" == "step1" ]; then
    echo "=========== compile_base_images start to exec step1 ========="
    # 编译大包前置流程
    ms_compile_base_pre
    ms_compile_base
  elif [ "${STEP_LEVEL}" == "step2" ]; then
    echo "=========== compile_base_images start to exec step2 ========="
    # 编译大包protect_agent_Package_all 执行完后执行后置步骤
    ms_compile_base
		NAME=$(ls "${BASE_PATH}/component/${Service_Name}/pkg")
		MS_NAME=$(basename $NAME .tar.gz)
		compile_image "${MS_NAME}" "${BUILD_PKG_TYPE}" "${PRODUCT_IMAGE_PATH}"
  else
    # 单独编译不影响
    echo "=========== compile_base_images start to exec all ========="
    ms_compile_base_pre
    ms_compile_base
		NAME=$(ls "${BASE_PATH}/component/${Service_Name}/pkg")
		MS_NAME=$(basename $NAME .tar.gz)
		compile_image "${MS_NAME}" "${BUILD_PKG_TYPE}" "${PRODUCT_IMAGE_PATH}"
  fi
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
	echo "compile PM_Boot_Dependencies before build"
	cd ${BASE_PATH}/component/PM_Boot_Dependencies
	mvn clean install
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
