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
function sync() {
    echo "start ntpdate gaussdb"
    /usr/sbin/ntpdate gaussdb
    hwclock -w
    echo "end ntpdate gaussdb"
}

function start() {
    max_period=3.0
    time_period=$(ntpdate -q gaussdb | grep 'ntpdate' | awk -F 'offset' '{print $2}')
    period_num=$(echo "$time_period" | grep -oP '\d*\.\d+')
    compare_result=$(echo "$period_num $max_period" | awk '{if ($1 > $2) {print 1} else {print 0}}')
    if [[ $compare_result -eq 1 ]]; then
      sync "$time_period"
    fi
}

function stop() {
     kill -9 `ps -ef | grep "ntpd" | grep -v "grep" | awk -F ' ' '{print $2}'`
}

if [ "$1" = "start" ]; then
    start
fi
if [ "$1" = "stop" ]; then
    stop
fi