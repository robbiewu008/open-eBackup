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
PACKAGE_PATH="${CURRENT_PATH}/../../../open-source-obligation/Infrastructure_OM/infrastructure"
BASEIMAGE_PATH=${PACKAGE_PATH}/baseImage
CBB_PYTHON_IMAGE_PATH=${PACKAGE_PATH}/cbb/python
LCRP_XML_PATH=${CURRENT_PATH}/../conf/
INFRASTRUCTURE_PATH=${CURRENT_PATH}/../../infrastructure
FILECLIENT_BRANCH="master_OceanProtect_DataBackup_1.6.0_smoke"
if [ -z "${componentVersion}" ]; then
  componentVersion="1.1.0"
fi
echo "Component Version:${componentVersion}"

if [ ! -d "${PACKAGE_PATH}" ]; then
  mkdir -p "${PACKAGE_PATH}"
fi

function down_package_from_cmc() {
  #artget下载依赖
  cd ${LCRP_XML_PATH}
  artget pull -d public_dependency_cmc.xml -p "{'componentVersion':'${componentVersion}'}" -ap ${PACKAGE_PATH} -user ${cmc_user} -pwd ${cmc_pwd}
  if [ $? -ne 0 ]; then
    echo "Download artifact from cmc error"
    exit 1
  fi

  artget pull -os public_dependency_opensource.xml -ap ${PACKAGE_PATH} -user ${opensource_user} -pwd ${opensource_pwd} -at opensource
  if [ $? -ne 0 ]; then
    echo "Download artifact from cmc error"
    exit 1
  fi

  artget pull -d opensrouce_from_centralized.xml -p "{'CODE_BRANCH':'${CODE_BRANCH}'}" -ap ${PACKAGE_PATH} -user ${cmc_user} -pwd ${cmc_pwd}
  if [ $? -ne 0 ]; then
    echo "Download opensource pkg from centralized cmc error."
    exit 1
  fi

  artget pull -d baseImage_dependency_cmc.xml -p "{ \
        'componentVersion':'${componentVersion}', \
        'slim_image':'${slim_image}', \
        'PACKAGE1':'${PACKAGE1}', \
        'PACKAGE2':'${PACKAGE2}', \
        'jdk_package':'${jdk_package}', \
        'gaussdb_python':'${gaussdb_python}', \
        'CODE_BRANCH':'${CODE_BRANCH}', \
        'FILECLIENT_BRANCH': '${FILECLIENT_BRANCH}' \
    }" -ap ${PACKAGE_PATH} -user ${cmc_user} -pwd ${cmc_pwd}

  if [ $? -ne 0 ]; then
    echo "Download artifact from cmc error"
    exit 1
  fi
  artget pull -os cbb_dependency_opensource.xml -ap ${PACKAGE_PATH} -user ${opensource_user} -pwd ${opensource_pwd} -at opensource
  if [ $? -ne 0 ]; then
    echo "Download opensource from cmc error"
    exit 1
  fi

  echo "down_package_from_cmc success"
}

function build_common_init_pkg() {
  cd ${CURRENT_PATH}
  tar -zcf common-init-${product_version}.tar.gz common-init/
}

function build_libkmcv3_pkg() {
  cd ${INFRASTRUCTURE_PATH}/script/upgrade_opensrc/kmc
  sh -e download_frame_third_platform.sh ${PRODUCT} ${CODE_BRANCH}
}

function main() {
  down_package_from_cmc
  build_common_init_pkg
  build_libkmcv3_pkg

  cd "${CURRENT_PATH}"/../../om/build
  sh copy_bin.sh
  if [ $? -ne 0 ]; then
    echo "cp bin error"
    exit 1
  fi

  echo "cp bin success"
}

main
