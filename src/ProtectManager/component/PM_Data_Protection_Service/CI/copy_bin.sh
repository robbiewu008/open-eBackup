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

function build_postgres() {
    cp -r /usr/share/postgres_user_local-postgresql-9.2.9/ ${PM_MS_DIR}/package/3rd/postgres_user_local
}

function download_python_binary(){
    cd ${PM_MS_DIR}/package/3rd
    export PATH=/usr/share/postgres_user_local-postgresql-9.2.9/bin:${PATH}
    pip download -r ${PM_MS_DIR}/src/requirements
    wget https://cmc-lfg-artifactory.cmc.tools.huawei.com/artifactory/cmc-software-release/GaussDB%20Kernel/GaussDB%20Kernel%20505/GaussDB%20Kernel%20505.0.T1.B062/Lite/Euler2.11_arm_64/GaussDB-Kernel_505.0.T1.B062_Server_ARM_Lite.tar.gz --no-check-certificate
    cd ${PM_MS_DIR}
    rm -rf librdkafka_temp
    mkdir librdkafka_temp
    cd ${PM_MS_DIR}/librdkafka_temp
    wget https://cmc-hgh-artifactory.cmc.tools.huawei.com/artifactory/opensource_general/librdkafka/v1.8.2/package/librdkafka-1.8.2.tar.gz --no-check-certificate
}

function build_librdkafka() {
  if [ -d "${PM_MS_DIR}/librdkafka_temp" ]; then
    cd ${PM_MS_DIR}/librdkafka_temp
    tar zxvf librdkafka-1.8.2.tar.gz
    cd librdkafka-1.8.2/
    chmod 750 configure lds-gen.py Makefile
    ./configure --prefix=${PM_MS_DIR}/librdkafka_temp/librdkafka_user_local
    make -j
    make install
    cp -r ${PM_MS_DIR}/librdkafka_temp/librdkafka_user_local/ ${PM_MS_DIR}/package/3rd/librdkafka_user_local
    rm -rf ${PM_MS_DIR}/librdkafka_temp
  else
    echo "build librdkafka failed, librdkafka_temp folder not exist"
		exit 1
  fi
}

function copy_libpq_and_psycopg2() {
    cd ${PM_MS_DIR}/package/3rd
    tar zxf GaussDB-Kernel_505.0.T1.B062_Server_ARM_Lite.tar.gz
    tar zxf GaussDB-Kernel_505.0.T1_Euler_64bit_Python.tar.gz
    cp lib/libpq.so.5.5   ${PM_MS_DIR}/package/3rd/libpq.so.5.5
    cp lib/libcrypto.so   ${PM_MS_DIR}/package/3rd/libcrypto.so
    cp lib/libssl.so   ${PM_MS_DIR}/package/3rd/libssl.so
    cp -r psycopg2/   ${PM_MS_DIR}/package/3rd/psycopg2
    # 删除解压文件
    rm -rf GaussDB-*
}

function copy_scripts() {
    cp -r ${CUR_PATH}/scripts/* ${PM_MS_DIR}/package/3rd/
}

function build_package(){
    cp ${CUR_PATH}/scripts/build_pyc.py ${PM_MS_DIR}/../PM_App_Common_Lib/CI/
    python ${PM_MS_DIR}/../PM_App_Common_Lib/CI/build_pyc.py
	  cp -r ${PM_MS_DIR}/../PM_App_Common_Lib/src ${PM_MS_DIR}/package/
	  rm -rf ${PM_MS_DIR}/package/common
	  mv ${PM_MS_DIR}/package/common_pyc ${PM_MS_DIR}/package/common
	  cp -r ${PM_MS_DIR}/src ${PM_MS_DIR}/package/
	  rm -rf ${PM_MS_DIR}/package/src/app
	  mv ${PM_MS_DIR}/package/src/app_pyc ${PM_MS_DIR}/package/src/app
	  cp ${PM_MS_DIR}/scripts/app.sh ${PM_MS_DIR}/package/src/
    cp ${PM_MS_DIR}/scripts/check_health.sh ${PM_MS_DIR}/package/src/
    cp ${PM_MS_DIR}/scripts/check_live.sh ${PM_MS_DIR}/package/src/
	  cp ${PM_MS_DIR}/scripts/mount_oper.sh ${PM_MS_DIR}/package/src/
	  cd ${PM_MS_DIR}
	  tar -czf PM_Data_Protection_Service.tar.gz package/
}

function copy_pkgs() {
    mkdir -p ${PM_MS_DIR}/pkg
	  cp -f ${PM_MS_DIR}/PM_Data_Protection_Service.tar.gz ${PM_MS_DIR}/pkg/
	  cp ${PM_MS_DIR}/pkg/PM_Data_Protection_Service.tar.gz ${BIN_PATH}/
	  if [ $? -ne 0 ]; then
		  echo "copy pkg failed."
		  exit 1
	  fi
}

function main() {
  mkdir -p ${PM_MS_DIR}/package/3rd
	mkdir -p ${PM_MS_DIR}/package/src
	echo ${PM_MS_DIR}
	build_postgres
  download_python_binary
  build_librdkafka
  copy_libpq_and_psycopg2
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

# 编译py为pyc
python build_pyc.py
main
