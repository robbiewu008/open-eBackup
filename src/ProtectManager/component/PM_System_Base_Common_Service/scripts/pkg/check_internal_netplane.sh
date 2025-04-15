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

function g_security_check_ipv6() {
  [[ $# -ne 1 ]] && return 1
  local ipv6_regex="^((([0-9a-fA-F]{1,4}:){7}([0-9A-Fa-f]{1,4}|:))|(([0-9a-fA-F]{1,4}:){6}((([0-9]{1,3}\.){3}[0-9]{1,4})|([0-9a-fA-F]{1,4})|:))|(([0-9a-fA-F]{1,4}:){5}:((([0-9a-fA-F]{1,4})?)|([0-9a-fA-F]{1,4}:[0-9a-fA-F]{1,4})|(([0-9]{1,3}\.){3}[0-9]{1,4})))|(([0-9a-fA-F]{1,4}:){4}:((([0-9a-fA-F]{1,4})?)|(([0-9a-fA-F]{1,4}:){1,2}[0-9a-fA-F]{1,4})|(([0-9a-fA-F]{1,4}:)?([0-9]{1,3}\.){3}[0-9]{1,4})))|(([0-9a-fA-F]{1,4}:){3}:((([0-9a-fA-F]{1,4}:){0,2}([0-9]{1,3}\.){3}[0-9]{1,4})|(([0-9a-fA-F]{1,4}:){0,3}[0-9a-fA-F]{1,4})|$))|(([0-9a-fA-F]{1,4}:){2}:((([0-9a-fA-F]{1,4}:){0,3}([0-9]{1,3}\.){3}[0-9]{1,4})|(([0-9a-fA-F]{1,4}:){0,4}[0-9a-fA-F]{1,4})|$))|(([0-9a-fA-F]{1,4}:){1}:((([0-9a-fA-F]{1,4}:){0,4}([0-9]{1,3}\.){3}[0-9]{1,4})|(([0-9a-fA-F]{1,4}:){0,5}[0-9a-fA-F]{1,4})|$))|(::((([0-9a-fA-F]{1,4}:){0,5}([0-9]{1,3}\.){3}[0-9]{1,4})|(([0-9a-fA-F]{1,4}:){0,6}[0-9a-fA-F]{1,4})|$)))$"
  local ip="$1"
  [[ -z "${ip}" ]] && return 1
  if [[ "${ip}" =~ ${ipv6_regex} ]]; then
    return 0
  fi
  return 2
}

check_internal_netplane()
{
    source_ip=$1
    target_ip=$2
    g_security_check_ipv4 $source_ip
    if [ $? -ne 0 ]; then
        echo "Source ip not ipv4"
        g_security_check_ipv6 $source_ip
    fi
    if [ $? -ne 0 ]; then
        echo "Source ip neither ipv4 nor ipv6"
        return 1
    fi
    g_security_check_ipv4 $target_ip
    if [ $? -ne 0 ]; then
        echo "Target ip not ipv4"
        g_security_check_ipv6 $source_ip
    fi
    if [ $? -ne 0 ]; then
        echo "Target ip neither ipv4 nor ipv6"
        return 1
    fi

    ip vrf exec vrf-srv curl --interface $source_ip -kv $target_ip:6432
}

check_internal_netplane "$@"