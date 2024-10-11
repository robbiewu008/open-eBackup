#!/bin/sh
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
EBACK_BASE_DIR="$(cd "$(dirname "$BASH_SOURCE")/../";pwd)"
 
BIN_PATH=$1
 
function main() {
  cp -r ${BIN_PATH}/DataMoveEngine/pkg ${EBACK_BASE_DIR}/
  if [ $? -ne 0 ];then
      echo -e "Copy dme pkg failed"
      return 1
  fi

  docker load -i ${EBACK_BASE_DIR}/pkg/mspkg/oceanprotect-dataprotect-1.0.rc1-cbb-python.tar
  if [ $? -ne 0 ];then
      echo -e "Load cbb-python image failed"
      return 1
  fi

  cd "${EBACK_BASE_DIR}/CI/script"
  sh build_image_opensource.sh
  if [ $? -ne 0 ];then
    echo -e "Build dme images failed"
    return 1
  fi
}

main