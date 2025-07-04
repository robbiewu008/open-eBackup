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
BASE_PATH=${CUR_PATH}/../..
REPO_PATH=$1
BUILD_PKG_TYPE=$2

#set bep
if [ "${BEP}" == "YES" ]; then
	echo "start set Bep_Time!"
	source bep_env.sh -s ${BASE_PATH}/CI/conf/bep_env.conf
fi

echo LCRP_HOME=${LCRP_HOME}
cp -rf ${BASE_PATH}/CI/LCRP/conf/Setting.xml ${LCRP_HOME}/conf/

function compile() {
	mvn --version
	echo "RUN:mvn dependency:tree"
	cd ${BASE_PATH}/component/PM_Common/
	pwd
	mvn dependency:tree
	PM_MS_LIST="PM_GUI PM_System_Base_Common_Service PM_Data_Protection_Service PM_API_Gateway PM_Database_Version_Migration PM_Config"
	for pmservice in ${PM_MS_LIST}; do
		echo "start compile ${pmservice}!"
		cd ${BASE_PATH}/component/${pmservice}/CI
		if [ "${pmservice}" == "PM_System_Base_Common_Service" ]; then
			sh copy_bin.sh "${BUILD_PKG_TYPE}" "${REPO_PATH}"
			if [ $? -ne 0 ]; then
				echo "${pmservice} compile failed"
				exit 1
			fi
		elif [ "${pmservice}" == "PM_Config" ]; then
			sh copy_bin.sh
			if [ $? -ne 0 ]; then
				echo "${pmservice} compile failed!"
				exit 1
			fi
			cp -f ${BASE_PATH}/component/PM_Config/pkg/PM_Config.tar.gz ${REPO_PATH}
		else
			sh copy_bin.sh "${REPO_PATH}"
			if [ $? -ne 0 ]; then
				echo "${pmservice} compile failed!"
				exit 1
			fi
		fi
	done
}


function main(){
  echo "compile PM_Boot_Dependencies before build"
  cd ${BASE_PATH}/component/PM_Boot_Dependencies
  mvn clean install -gs ${BASE_PATH}/CI/conf/settings.xml
  if [ $? -ne 0 ]; then
  	echo "PM_Boot_Dependencies mvn compile failed!"
  	exit 1
  fi
  compile
}

main