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
  cd $(dirname $0)/
  pwd
)
source $CURRENT_PATH/commParam.sh

# base_image
cd "${CURRENT_PATH}"
sh open_build_base_image.sh
if [ $? -ne 0 ]; then
  echo "build base images failed"
  exit 1
fi

echo "build base images success"

# om
cd "${CURRENT_PATH}"/../../om/build
sh build_opensource.sh
if [ $? -ne 0 ]; then
  echo "build om images failed"
  exit 1
fi

echo "build om images success"

# infra(sftp、zookeeper)
cd "${CURRENT_PATH}"
sh open_build_compile_pkg.sh

if [ $? -ne 0 ]; then
  echo "build infrastructure images failed"
  exit 1
fi

echo "build infrastructure images success"
