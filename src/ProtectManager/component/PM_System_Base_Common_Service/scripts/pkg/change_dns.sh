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

function g_security_check_ipv4() {
  [[ $# -ne 1 ]] && return 1
  local ipv4_regex="^([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])(\.([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])){3}$"
  local ip="$1"
  [[ -z "${ip}" ]] && return 1
  if [[ "${ip}" =~ ${ipv4_regex} ]]; then
    return 0
  fi
  return 2
}

change_dns()
{
    # The kubelet adds a set of environment variables for each active Service.
    # It adds {SVCNAME}_SERVICE_HOST variables, where the Service name is upper-cased and dashes
    # are converted to underscores. We have created a service named 'dme-dns-srv',
    # so DME_DNS_SRV_SERVICE_HOST will be populated.
    service_ip=$1
    g_security_check_ipv4 $service_ip
    if [ $? -ne 0 ]; then
        echo "Change dns nameserver failed"
        return 1
    fi
    echo "Before change, /etc/resolv.conf is:"
    cat "/etc/resolv.conf"
    random_filename=$(uuidgen)
    tmp_resolv="/tmp/resolve_${random_filename}.conf"
    cp /etc/resolv.conf $tmp_resolv
    echo "Need change resolv.conf to ${service_ip}"
    sed -i "s/nameserver.*/nameserver ${service_ip}/" $tmp_resolv
    if [ $? -ne 0 ]; then
        echo "Change dns nameserver failed"
        return 1
    fi
    cp -f $tmp_resolv /etc/resolv.conf
    rm -rf $tmp_resolv
    echo "After change, /etc/resolv.conf is:"
    cat "/etc/resolv.conf"
}

change_dns $1