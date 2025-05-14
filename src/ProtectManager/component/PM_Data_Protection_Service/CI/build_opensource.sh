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
PM_MS_DIR=${CUR_PATH}/..
BASE_PATH=${PM_MS_DIR}/../..
THIRD_PATCH_DIR=${PM_MS_DIR}/../temp_third
BIN_PATH=$1

function copy_scripts() {
    cd ${PM_MS_DIR}/
    rm -rf package PM_Data_Protection_Service.tar.gz
    mkdir package
    tar -zxvf ${BIN_PATH}/PM_Data_Protection_Service.tar.gz -C ${PM_MS_DIR}
    cp -r ${CUR_PATH}/scripts/* ${PM_MS_DIR}/package/3rd/
}

function build_package(){
    rm -rf ${PM_MS_DIR}/package/src
    cp -r ${PM_MS_DIR}/src ${PM_MS_DIR}/package/
	  cp ${PM_MS_DIR}/scripts/app.sh ${PM_MS_DIR}/package/src/
    cp ${PM_MS_DIR}/scripts/check_health.sh ${PM_MS_DIR}/package/src/
    cp ${PM_MS_DIR}/scripts/check_live.sh ${PM_MS_DIR}/package/src/
	  cp ${PM_MS_DIR}/scripts/mount_oper.sh ${PM_MS_DIR}/package/src/
	  tar -czf PM_Data_Protection_Service.tar.gz package/
}

function copy_pkgs() {
	  cp ${PM_MS_DIR}/pkg/PM_Data_Protection_Service.tar.gz ${BASE_PATH}/pkg/mspkg/
	  if [ $? -ne 0 ]; then
		  echo "copy pkg failed."
		  exit 1
	  fi
}

function main() {
	echo ${PM_MS_DIR}
  copy_scripts
  build_package
	copy_pkgs
}

export buildNumber=$(date +%Y%m%d%H%M%S)

if [[ ${releaseVersion} ]]; then
	PackageVersion=${releaseVersion}
	echo "buildVersion=${releaseVersion}" > ${PM_MS_DIR}/../buildInfo.properties
else
	PackageVersion=1.0.0.${buildNumber}
	echo "buildVersion=${PackageVersion}" > ${PM_MS_DIR}/../buildInfo.properties
fi

main
