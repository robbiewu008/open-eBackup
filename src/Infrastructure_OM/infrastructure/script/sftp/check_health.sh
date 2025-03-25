#!/bin/bash

#########################################
# Copyright (c) 2012-2015 Huawei .
# All rights reserved.
#
# Function 容器的健康检查，只做服务正常检查
# revise note
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
