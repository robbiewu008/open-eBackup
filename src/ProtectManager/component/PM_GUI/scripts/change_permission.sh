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

whitelist=(
  "^/opt/OceanStor-100P/$"
  "^/tmp$"
  "^/tmp/BOOT-INF$"
  "^/tmp/app.jar$"
  "^/opt/ProtectManager/whitebox/oem_package.tgz$"
  "^/opt/ProtectManager/i18n$"
)

permisionlist=(
   "^640$"
   "^700$"
   "^750$"
)

function check_while_list() {
  local path="$1"
  # 白名单校验
  for wdir in "${whitelist[@]}"; do
    if [[ "$path" =~ $wdir ]]; then
      return 0
    fi
  done
  return 1
}

function check_permision() {
  local permision="$1"
  for per in "${permisionlist[@]}"; do
    if [[ "$permision" =~ $per ]]; then
      return 0
    fi
  done
  return 1
}

function check_path_common() {
  local path="$1"
  if [ -z "${path}" ]; then
    return 1
  fi

  filepat='[|;&$><`!+]'
  result=$(echo "${path}" | grep "${filepat}")
  if [ ! -z "${result}" ]; then
    echo "${path} contains special characters."
    return 1
  fi
  result=$(echo "${path}" | grep -w "\..")
  if [ ! -z "${result}" ]; then
    echo "${path} contains special characters."
    return 1
  fi
  # 软链接校验
  if [[ -L "$path" ]]; then
    echo "ERROR: ${path} file or folder has a soft link."
    return 1
  fi

  # 白名单校验
  check_while_list "${path}"
  if [ "$?" != "0" ]; then
    echo "ERROR: ${path} not in whitelist!"
    return 1
  fi
}

function change_owner_nobody_nobody() {
  local dir_name="$1"
  check_path_common "${dir_name}"
  if [ "$?" != "0" ]; then
    return 1
  fi
  chown -h 15012:99 "${dir_name}" -R
}

function change_mod() {
  local dir_name="$1"
  local permission="$2"
  check_path_common "${dir_name}"
  if [ "$?" != "0" ]; then
    return 1
  fi
  check_permision "${permission}"
  if [ "$?" != "0" ]; then
    echo "ERROR: permision ${permission} not allow!"
    return 1
  fi
  chmod "${permission}" "${dir_name}" -R
}

function change_mod_group_write() {
  local dir_name="$1"
  check_path_common "${dir_name}"
  if [ "$?" != "0" ]; then
    return 1
  fi
  chmod g+w "${dir_name}" -R
}

type="$1"
if [ -z "${type}" ]; then
  echo 'type error'
  exit 1
fi

if [ "${type}" == "change_owner_nobody_nobody" ]; then
  if [ "$#" -ne 2 ]; then
    echo "param is error"
    return 1
  fi
  change_owner_nobody_nobody "$2"
elif [ "${type}" == "chmod" ]; then
  if [ "$#" -ne 3 ]; then
    echo "param is error"
    return 1
  fi
  change_mod "$2" "$3"
elif [ "${type}" == "chmod_group_write" ]; then
  if [ "$#" -ne 2 ]; then
    echo "param is error"
    return 1
  fi
  change_mod_group_write "$2"
else
  echo "param is error"
  return 1
fi
