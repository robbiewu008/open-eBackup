#!/bin/bash
########################################
#  This file is part of the open-eBackup project.
# Copyright (c) 2024 Huawei Technologies Co.,Ltd.
#
# open-eBackup is licensed under MPL v2.
# You can use this software according to the terms and conditions of the MPL v2.
# You may obtain a copy of MPL v2 at:
#
#          https://www.mozilla.org/en-US/MPL/2.0
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the MPL v2 for more details.
########################################
set -ex

CURRENT_PATH=$(
  cd $(dirname $0)
  pwd
)
source $CURRENT_PATH/open_comm_param.sh
LCRP_XML_PATH=${CURRENT_PATH}/../conf/
DOCKERFILE_PATH=${CURRENT_PATH}/../../ci/build/baseImage/dockerfiles/
CBB_PYTHON_DOCKERFILE_PATH=${CURRENT_PATH}/../../ci/build/cbb/python/
PACKAGE_PATH="${binary_path}/Infrastructure_OM/infrastructure"
KMC_PAYH=${binary_path}/Infrastructure_OM/infrastructure/kmc
BASEIMAGE_PATH=${PACKAGE_PATH}/baseImage
CBB_PYTHON_IMAGE_PATH=${PACKAGE_PATH}/cbb/python
AA_SYS_PATH=${CURRENT_PATH}/../../AA-Sys
INFRASTRUCTURE_PATH=${CURRENT_PATH}/../../infrastructure
CBB_PYTHON_PATH=${CURRENT_PATH}/../../cbb_python
POSTGRESQL_VERSION=15.2


function build_slim_image()
{
    docker load -i ${PACKAGE_PATH}/../../openEuler-docker.aarch64.tar.xz
    if [ $? != 0 ];then
        echo "Failed to load openEuler image."
        exit 1
    fi
}

function build_common_init_pkg() {
  cd ${INFRASTRUCTURE_PATH}/script/
  tar -zcf common-init-${product_version}.tar.gz common-init/
  cp -f common-init-${product_version}.tar.gz ${DOCKERFILE_PATH}
}

function insert_pkg_to_image()
{
  # 将jdk构建到裁剪镜像中，构成基础镜像，并保存tar包
  cd ${DOCKERFILE_PATH}
  cp ${PACKAGE_PATH}/${jdk_package} ${DOCKERFILE_PATH}/
  tar -zxpf ${jdk_package}
  cp ${KMC_PAYH}/KmcLib/lib/libSecurityStarter.so ${DOCKERFILE_PATH}
  cp ${KMC_PAYH}/KmcLib/lib/libkmcv3.so ${DOCKERFILE_PATH}
  cp ${KMC_PAYH}/KmcLib/lib/kmcdecrypt ${DOCKERFILE_PATH}
  cp ${KMC_PAYH}/KmcLib/include/kmcv3.h ${DOCKERFILE_PATH}
  cp ${KMC_PAYH}/KmcLib/lib/restclient ${DOCKERFILE_PATH}
  cp ${INFRASTRUCTURE_PATH}/script/cli/get.sh ${DOCKERFILE_PATH}
  cp ${INFRASTRUCTURE_PATH}/script/common/init_logic_ports.py ${DOCKERFILE_PATH}
  cp -f ${INFRASTRUCTURE_PATH}/script/upgrade_opensrc/sudoers ${DOCKERFILE_PATH}
  # 修改权限
  chmod 550 ${DOCKERFILE_PATH}/kmcdecrypt
  chmod 755 ${DOCKERFILE_PATH}/libkmcv3.so
  chmod 644 ${DOCKERFILE_PATH}/kmcv3.h
  chmod 550 ${DOCKERFILE_PATH}/restclient
  chmod 550 ${DOCKERFILE_PATH}/get.sh
  chmod 550 ${DOCKERFILE_PATH}/init_logic_ports.py
  base_openeuler_yum_url="${base_openeuler_yum_url//\//\\/}"
  sed -i "s/jdk_version/${jdk_version}/g" ${product_name}_base.dockerfile
  sed -i "s/jdk_package/${jdk_package}/g" ${product_name}_base.dockerfile
  sed -i "s/product_version/${product_version}/g" ${product_name}_base.dockerfile
  sed -i "s/openeuler_url/${base_openeuler_yum_url}/g" openeuler.repo
  echo -e "\nsslverify=0">>openeuler.repo

  mkdir -p ${DOCKERFILE_PATH}/FileClient
  if [ -f "${PACKAGE_PATH}/fileClient_aarch64.tar.gz" ]; then
      tar -zxvf ${PACKAGE_PATH}/fileClient_aarch64.tar.gz -C ${DOCKERFILE_PATH}/FileClient
  fi
  # docker repo名称必须要小写
  docker build -f ${product_name}_base.dockerfile -t ${product_name,,}-${product_version,,}:base .
}

function build_base_image() {
  build_slim_image
  build_common_init_pkg
  insert_pkg_to_image
}

function build_cbb_image()
{
  cp -r "${CBB_PYTHON_PATH}"/public_cbb "${CBB_PYTHON_DOCKERFILE_PATH}"
  cp -r "${CBB_PYTHON_PATH}"/setup.py "${CBB_PYTHON_DOCKERFILE_PATH}"
  mkdir -p ${PACKAGE_PATH}/gaussdb/gaussdb_python
  tar -zxvf ${binary_path}/openGauss-*-openEuler-*-Python.tar.gz -C ${PACKAGE_PATH}/gaussdb/gaussdb_python
  rm -rf ${CBB_PYTHON_DOCKERFILE_PATH}/psycopg2
  cp -rf ${PACKAGE_PATH}/gaussdb/gaussdb_python/psycopg2 ${CBB_PYTHON_DOCKERFILE_PATH}
  cp -rf ${PACKAGE_PATH}/gaussdb/gaussdb_python/lib ${CBB_PYTHON_DOCKERFILE_PATH}
  cd ${CBB_PYTHON_DOCKERFILE_PATH}
  chmod -R 755 psycopg2 lib
  rm -f ${CBB_PYTHON_DOCKERFILE_PATH}/lib/libstdc++.*
  rm -f ${CBB_PYTHON_DOCKERFILE_PATH}/lib/libgcc_s.*
  rm -f ${CBB_PYTHON_DOCKERFILE_PATH}/lib/libssl.*
  rm -f ${CBB_PYTHON_DOCKERFILE_PATH}/lib/libcrypto.*
  rm -rf ${PACKAGE_PATH}/gaussdb/

  compile_cbb_dependency
  cd ${CBB_PYTHON_DOCKERFILE_PATH}
  if [ ! -d lib ]; then
    mkdir lib
  fi
  tar -zxvf ${PACKAGE_PATH}/../../Infrastructure_OM/infrastructure/cbb/python/cbb_lib.tar.gz -C ./lib

  docker build -f ${product_name}_cbb_python.dockerfile -t ${product_name,,}-${product_version,,}-cbb-python:base .
  if [ $? -ne 0 ];then
      echo "docker build cbb python image is failed. pleaser retry it."
      exit 1
  fi

  if [ ${tag_image} == "debug" ] || [ ${tag_image} == "asan" ];then
      docker save -o ${CBB_PYTHON_IMAGE_PATH}/${product_name}-${product_version}-cbb-python-image-ARM_64-debug.tar ${product_name,,}-${product_version,,}-cbb-python:base
      if [ $? -ne 0 ];then
          echo "docker save cbb python image is failed. pleaser retry it."
          exit 1
      fi
  else
      docker save -o ${CBB_PYTHON_IMAGE_PATH}/${product_name}-${product_version}-cbb-python-image-ARM_64.tar ${product_name,,}-${product_version,,}-cbb-python:base
      if [ $? -ne 0 ];then
          echo "docker save cbb python image is failed. pleaser retry it."
          exit 1
      fi
  fi
}

function compile_cbb_dependency() {
  tar -zxf "${CBB_PYTHON_IMAGE_PATH}"/postgresql-${POSTGRESQL_VERSION}.tar.gz
  cd postgresql-${POSTGRESQL_VERSION}
  ./configure --without-readline --without-zlib --disable-rpath --disable-spinlocks --prefix="${CBB_PYTHON_DOCKERFILE_PATH}"/postgresql_dep CFLAGS="-O2 -fPIE -fstack-protector-all -pie" LDFLAGS="-Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack"
  make && make install
  if [ $? -ne 0 ]; then
    echo "failed to build postgresql"
    exit 1
  fi
  rm -f "${CBB_PYTHON_DOCKERFILE_PATH}"/postgresql_dep/lib64/*.a
  find "${CBB_PYTHON_DOCKERFILE_PATH}/postgresql_dep" -exec strip {} \;
  cd ..
}

function initEnv() {
  local arch_type=$(uname -m)
  if [ "$arch_type" == "aarch64" ]; then
    ARCH="euler-arm"
  else
    ARCH="euler-x86"
  fi
}


function main() {
  initEnv
  build_base_image
  build_cbb_image
}

main
