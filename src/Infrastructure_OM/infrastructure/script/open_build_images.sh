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
BASE_PATH=${CURRENT_PATH}/../..
CI_PATH=${BASE_PATH}/ci
OM_PATH=${BASE_PATH}/om
PKG_DIR="${BASE_PATH}/../../open-source-obligation/Infrastructure_OM/infrastructure/compileLib"
sh ${CI_PATH}/script/commParam.sh
G_INFRASTRUCTURE_NAME="elasticsearch gaussdb kafka redis zookeeper sftp"
L_BASE_TAG="oceanprotect-dataprotect-1.0.rc1:base"

function build_image() {
  local NAME=$1
  echo "begin to build service ${NAME}"

  L_TAG=$(cat ${CI_PATH}/build/Infrastructure/dockerfiles/${NAME}.name)
  L_MS_NAME=$(echo ${L_TAG} | sed 's/:/-/g')
  L_TAG=$(echo ${L_TAG} | awk -F ':' '{print $1 FS}')
  L_TAG=${L_TAG}${MS_IMAGE_TAG}

  mkdir -p ${PKG_DIR}/tmp/${L_MS_NAME}

  if [ -f "${PKG_DIR}/${L_MS_NAME}.tar.gz" ]; then
    cp -rf "${PKG_DIR}/${L_MS_NAME}.tar.gz" "${PKG_DIR}/tmp/${L_MS_NAME}"
    if [ $? != 0 ]; then
      echo "copy $L_MS_NAME.tar.gz failed"
      exit 1
    fi
    if [ "${NAME}" == "gaussdb" ]; then
      tar -zxpf ${PKG_DIR}/tmp/${L_MS_NAME}/${L_MS_NAME}.tar.gz -C ${PKG_DIR}/tmp/${L_MS_NAME}/
      ls -l ${PKG_DIR}
      tar -zvxf ${PKG_DIR}/HA*.gz -C ${PKG_DIR}/tmp/${L_MS_NAME}
      if [ $? != 0 ]; then
        echo "Tar HA failed"
        exit 1
      fi
      chmod 750 -R ${PKG_DIR}/tmp/${L_MS_NAME}/HA*aarch64
    fi
  else
    echo "${PKG_DIR}/${L_MS_NAME}.tar.gz not found for build"
    exit 1
  fi

  if [ "${NAME}" == "zookeeper" ] || [ "${NAME}" == "elasticsearch" ]; then
    tar -zxvf ${PKG_DIR}/tmp/${L_MS_NAME}/${L_MS_NAME}.tar.gz -C ${PKG_DIR}/tmp/${L_MS_NAME}/
  fi

  if [ "${NAME}" == "kafka" ]; then
    tar -zxvf ${PKG_DIR}/tmp/${L_MS_NAME}/${L_MS_NAME}.tar.gz -C ${PKG_DIR}/tmp/${L_MS_NAME}/
    cp -rf "${PKG_DIR}/${NAME}-scripts.tar.gz" ${PKG_DIR}/tmp/${L_MS_NAME}/
    tar -zxvf ${PKG_DIR}/tmp/${L_MS_NAME}/${NAME}-scripts.tar.gz -C ${PKG_DIR}/tmp/${L_MS_NAME}/
  fi

  if [ "${NAME}" == "redis" ]; then
    cp -rf "${PKG_DIR}/${NAME}-scripts.tar.gz" "${PKG_DIR}/tmp/${L_MS_NAME}/"
    tar -zxvf ${PKG_DIR}/tmp/${L_MS_NAME}/${NAME}-scripts.tar.gz -C ${PKG_DIR}/tmp/${L_MS_NAME}/
  fi

  docker build -t ${L_TAG} -f ${CI_PATH}/build/Infrastructure/dockerfiles/${NAME}.dockerfile ${PKG_DIR}/tmp/${L_MS_NAME}
  if [[ $? -ne 0 ]]; then
    echo "docker build ${L_MS_NAME} failed"
    rm -rf "${PKG_DIR}/tmp/${L_MS_NAME}"
    return 1
  fi
  rm -rf "${PKG_DIR}/tmp/${L_MS_NAME}"
  echo "docker build ${L_MS_NAME} success"

  docker tag ${L_TAG} harbors.inhuawei.com/${harbor_project}/${code_branch}/${tag_image}/${L_TAG}
  docker push harbors.inhuawei.com/${harbor_project}/${code_branch}/${tag_image}/${L_TAG}
  if [ $? -ne 0 ]; then
    echo "push image to harbor failed"
    exit 1
  fi

  echo "docker push success."

}

function build() {
  for NAME in ${G_INFRASTRUCTURE_NAME}; do
    build_image ${NAME}
    if [ $? -ne 0 ]; then
      echo "${MS_NAME} docker iamge build failed"
      exit 1
    fi
  done
}

function main() {
  import_base_image
  build
}

main $@
