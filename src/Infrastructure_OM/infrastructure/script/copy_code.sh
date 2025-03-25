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
  cd $(dirname $0)/../../../
  pwd
)
TARGET_PATH="${CURRENT_PATH}/REST_API/"
AA_TARGET_PATH="${CURRENT_PATH}/REST_API/Infrastructure_OM/infrastructure/script"
AA_SYS_PATH=${CURRENT_PATH}/Infrastructure_OM/AA-Sys

if [ ! -d "${TARGET_PATH}" ]; then
  mkdir -p "${TARGET_PATH}"
fi

if [ ! -d "${AA_TARGET_PATH}" ]; then
  mkdir -p "${AA_TARGET_PATH}"
fi

cp -r "$CURRENT_PATH/Infrastructure_OM" "$TARGET_PATH/"
cp ${AA_SYS_PATH}/Product/taishan_src/devm/src/net/script/net_cnet_config_firewall.sh ${AA_TARGET_PATH}/common-init
cp ${AA_SYS_PATH}/Product/taishan_src/devm/src/net/script/net_cnet_declare_neigbour.sh ${AA_TARGET_PATH}/common-init

if [ $? -ne 0 ]; then
  echo "cp code to WORKSPACE failed"
  exit 1
fi

echo "cp code success"
