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

rootCAFile="/var/run/secrets/kubernetes.io/serviceaccount/ca.crt"
tokenFile=$(cat /var/run/secrets/kubernetes.io/serviceaccount/token)

function main() {
  # 检查configmap中检查是否拉起服务
  configmap=$(LD_PRELOAD=/usr/lib64/libSecurityStarter.so curl --cacert ${rootCAFile} -X GET -H "Authorization: Bearer ${tokenFile}" https://${KUBERNETES_SERVICE_HOST}/api/v1/namespaces/dpa/configmaps/common-conf follow@-H)
  sftp_enable=$(echo "${configmap} " | python3 -c "import sys, json;print(json.load(sys.stdin)['data'].get('sftp_enable', ''))")
  if [[ "${sftp_enable}" != "open" ]]; then
    # 服务未开启，直接返回成功
    return 0
  fi
  ps aux | grep sftp_model.pyc | grep -v grep >>/dev/null &&
    ps aux | grep sshd | grep -v grep >>/dev/null
}

main
