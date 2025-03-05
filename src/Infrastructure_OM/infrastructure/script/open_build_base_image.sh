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

CURRENT_PATH=$(
  cd $(dirname $0)
  pwd
)
source $CURRENT_PATH/commParam.sh
LCRP_XML_PATH=${CURRENT_PATH}/../conf/
DOCKERFILE_PATH=${CURRENT_PATH}/../../ci/build/baseImage/dockerfiles/
CBB_PYTHON_DOCKERFILE_PATH=${CURRENT_PATH}/../../ci/build/cbb/python/
PACKAGE_PATH="${CURRENT_PATH}/../../../../open-source-obligation/Infrastructure_OM/infrastructure"
BASEIMAGE_PATH=${PACKAGE_PATH}/baseImage
CBB_PYTHON_IMAGE_PATH=${PACKAGE_PATH}/cbb/python
AA_SYS_PATH=${CURRENT_PATH}/../../AA-Sys
INFRASTRUCTURE_PATH=${CURRENT_PATH}/../../infrastructure
CBB_PYTHON_PATH=${CURRENT_PATH}/../../cbb_python
POSTGRESQL_VERSION=15.2

if [ "${CODE_BRANCH}" = "" ]; then
  echo "Please specify base image upload branch!"
  echo "CODE_BRANCH=$2"
  exit 1
else
  inf_branch=$(echo ${CODE_BRANCH} | tr [A-Z] [a-z])
  echo inf_branch=${inf_branch}
fi

function build_slim_image() {
  # 裁剪镜像
  cd ${PACKAGE_PATH}
  # 清理资源
  docker system prune -a --force
  if [ -d "${PACKAGE_PATH}/pkg" ]; then
    rm -rf ${PACKAGE_PATH}/pkg
  fi
  mkdir -p pkg/package
  tar xzf ${PACKAGE1} -C pkg
  tar xzf ${PACKAGE2} -C pkg
  find "${PACKAGE_PATH}/pkg/" -name "*.rpm" -exec cp -rf "{}" "${PACKAGE_PATH}/pkg/package" \;
  # 加载Euler镜像
  docker load -i ${slim_image}
  # docker run进行rpm裁剪与添加
  docker run --name euleros-base -v ${PACKAGE_PATH}/pkg/package:/mnt -v ${INFRASTRUCTURE_PATH}/script/image-slim.sh:/tmp/image-slim.sh ${slim_image_name}:${slim_image_tag} sh /tmp/image-slim.sh ${tag_image}
  if [ $? -ne 0 ]; then
    echo "docker run failed"
    exit 1
  fi
  docker commit euleros-base euleros:base
}

function insert_pkg_to_image() {
  # 将jdk构建到裁剪镜像中，构成基础镜像，并保存tar包
  cd ${DOCKERFILE_PATH}
  cp ${PACKAGE_PATH}/${jdk_package} ${DOCKERFILE_PATH}/
  tar -zxpf ${jdk_package}
  cp ${INFRASTRUCTURE_PATH}/script/upgrade_opensrc/kmc/KmcLib/lib/libkmcv3.so ${DOCKERFILE_PATH}
  cp ${INFRASTRUCTURE_PATH}/script/upgrade_opensrc/kmc/KmcLib/lib/kmcdecrypt ${DOCKERFILE_PATH}
  cp ${INFRASTRUCTURE_PATH}/script/upgrade_opensrc/kmc/KmcLib/include/kmcv3.h ${DOCKERFILE_PATH}
  cp ${INFRASTRUCTURE_PATH}/script/upgrade_opensrc/kmc/KmcLib/lib/restclient ${DOCKERFILE_PATH}
  cp ${INFRASTRUCTURE_PATH}/script/common-init-${product_version}.tar.gz ${DOCKERFILE_PATH}
  cp ${INFRASTRUCTURE_PATH}/script/cli/get.sh ${DOCKERFILE_PATH}
  cp ${INFRASTRUCTURE_PATH}/script/common/init_logic_ports.py ${DOCKERFILE_PATH}
  # 修改权限
  chmod 550 ${DOCKERFILE_PATH}/kmcdecrypt
  chmod 755 ${DOCKERFILE_PATH}/libkmcv3.so
  chmod 644 ${DOCKERFILE_PATH}/kmcv3.h
  chmod 550 ${DOCKERFILE_PATH}/restclient
  chmod 550 ${DOCKERFILE_PATH}/get.sh
  chmod 550 ${DOCKERFILE_PATH}/init_logic_ports.py
  base_openeuler_yum_url="${base_openeuler_yum_url//\//\\/}"
  sed -i "s/jdk_version/${jdk_version}/g" base.dockerfile
  sed -i "s/jdk_package/${jdk_package}/g" base.dockerfile
  sed -i "s/product_version/${product_version}/g" base.dockerfile
  sed -i "s/openeuler_url/${base_openeuler_yum_url}/g" openeuler.repo
  echo -e "\nsslverify=0" >>openeuler.repo

  mkdir -p ${DOCKERFILE_PATH}/FileClient
  if [ -f "${PACKAGE_PATH}/fileClient_aarch64.tar.gz" ]; then
    tar -zxvf ${PACKAGE_PATH}/fileClient_aarch64.tar.gz -C ${DOCKERFILE_PATH}/FileClient
  fi
  # docker repo名称必须要小写
  docker build -f base.dockerfile -t ${product_name,,}-${product_version,,}:base .
  rm -rf ${jdk_package} ${jdk_version}

  # 如果基础镜像目录存在
  if [ -d ${BASEIMAGE_PATH} ]; then
    rm -rf ${BASEIMAGE_PATH}
  fi
  mkdir ${BASEIMAGE_PATH}
  if [ ${tag_image} == "debug" ] || [ ${tag_image} == "asan" ]; then
    docker save -o ${BASEIMAGE_PATH}/${product_name}-${product_version}-base-image-ARM_64-debug.tar ${product_name,,}-${product_version,,}:base
    if [ $? -ne 0 ]; then
      echo "docker save baseImage is failed. pleaser retry it."
      exit 1
    fi
  else
    docker save -o ${BASEIMAGE_PATH}/${product_name}-${product_version}-base-image-ARM_64.tar ${product_name,,}-${product_version,,}:base
    if [ $? -ne 0 ]; then
      echo "docker save baseImage is failed. pleaser retry it."
      exit 1
    fi
  fi

}

function build_base_image() {
  build_slim_image
  insert_pkg_to_image
}

function upload_base_image_to_harbor() {
  # 将构建的基础镜像上传到harbor
  echo "Upload images to harbor: harbors.inhuawei.com/${harbor_project}/${inf_branch}"
  L_TAG="oceanprotect-dataprotect-1.0.rc1:base"
  echo "RUN: docker tag ${L_TAG} harbors.inhuawei.com/${harbor_project}/${inf_branch}/${tag_image}/${L_TAG}"
  docker tag ${L_TAG} harbors.inhuawei.com/${harbor_project}/${inf_branch}/${tag_image}/${L_TAG}
  echo "RUN: docker push harbors.inhuawei.com/${harbor_project}/${inf_branch}/${tag_image}/${L_TAG}"
  docker push harbors.inhuawei.com/${harbor_project}/${inf_branch}/${tag_image}/${L_TAG}
  if [ $? -ne 0 ]; then
    echo "upload ${L_TAG} image to harbor failed!"
    exit 1
  fi
}

function build_cbb_image() {
  cp -r "${CBB_PYTHON_PATH}"/public_cbb "${CBB_PYTHON_DOCKERFILE_PATH}"
  cp -r "${CBB_PYTHON_PATH}"/setup.py "${CBB_PYTHON_DOCKERFILE_PATH}"
  mkdir -p ${PACKAGE_PATH}/gaussdb/gaussdb_python
  tar -zxvf ${PACKAGE_PATH}/GaussDB-Kernel_*_Server_ARM_Lite.tar.gz -C ${PACKAGE_PATH}/gaussdb
  tar -zxvf ${PACKAGE_PATH}/gaussdb/GaussDB-Kernel_505.0.0.SPC1500_Euler_64bit_Python.tar.gz -C ${PACKAGE_PATH}/gaussdb/gaussdb_python
  rm -rf ${CBB_PYTHON_DOCKERFILE_PATH}/psycopg2
  mv ${PACKAGE_PATH}/gaussdb/gaussdb_python/psycopg2 ${CBB_PYTHON_DOCKERFILE_PATH}/
  cd ${CBB_PYTHON_DOCKERFILE_PATH}
  chmod -R 755 psycopg2
  tar -zcf psycopg2.tar.gz psycopg2
  mv ${PACKAGE_PATH}/gaussdb/gaussdb_python/lib/libpq.so.5.5 ${CBB_PYTHON_DOCKERFILE_PATH}/
  mv ${PACKAGE_PATH}/gaussdb/gaussdb_python/lib/libssl.so ${CBB_PYTHON_DOCKERFILE_PATH}/
  mv ${PACKAGE_PATH}/gaussdb/gaussdb_python/lib/libcrypto.so ${CBB_PYTHON_DOCKERFILE_PATH}/
  rm -rf ${PACKAGE_PATH}/GaussDB-Kernel_*_Server_ARM_Lite.tar.gz
  rm -rf ${PACKAGE_PATH}/gaussdb/
  rm -rf ${PACKAGE_PATH}/gaussdb/gaussdb_python
  compile_cbb_dependency
  docker build -f cbb_python.dockerfile -t ${product_name,,}-${product_version,,}-cbb-python:base .
  if [ $? -ne 0 ]; then
    echo "docker build cbb python image is failed. pleaser retry it."
    exit 1
  fi
  if [ -d ${CBB_PYTHON_IMAGE_PATH} ]; then
    rm -rf ${CBB_PYTHON_IMAGE_PATH}
  fi
  mkdir -p ${CBB_PYTHON_IMAGE_PATH}

  if [ ${tag_image} == "debug" ] || [ ${tag_image} == "asan" ]; then
    docker save -o ${CBB_PYTHON_IMAGE_PATH}/${product_name}-${product_version}-cbb-python-image-ARM_64-debug.tar ${product_name,,}-${product_version,,}-cbb-python:base
    if [ $? -ne 0 ]; then
      echo "docker save cbb python image is failed. pleaser retry it."
      exit 1
    fi
  else
    docker save -o ${CBB_PYTHON_IMAGE_PATH}/${product_name}-${product_version}-cbb-python-image-ARM_64.tar ${product_name,,}-${product_version,,}-cbb-python:base
    if [ $? -ne 0 ]; then
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

function upload_python_cbb_image_to_harbor() {
  # 将构建的python-cbb镜像上传到harbor
  echo "Upload images to harbor: harbors.inhuawei.com/${harbor_project}/${inf_branch}"
  L_TAG="${product_name,,}-${product_version,,}-cbb-python:base"
  echo "RUN: docker tag ${L_TAG} harbors.inhuawei.com/${harbor_project}/${inf_branch}/${tag_image}/${L_TAG}"
  docker tag ${L_TAG} harbors.inhuawei.com/${harbor_project}/${inf_branch}/${tag_image}/${L_TAG}
  echo "RUN: docker push harbors.inhuawei.com/${harbor_project}/${inf_branch}/${tag_image}/${L_TAG}"
  docker push harbors.inhuawei.com/${harbor_project}/${inf_branch}/${tag_image}/${L_TAG}
  if [ $? -ne 0 ]; then
    echo "upload ${L_TAG} image to harbor failed!"
    exit 1
  fi
}

function main() {
  initEnv
  build_base_image
  upload_base_image_to_harbor
}

main
