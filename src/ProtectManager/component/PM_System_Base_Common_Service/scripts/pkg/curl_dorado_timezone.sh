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

if [ $# -ne 0 ]; then
  echo "param is error"
  exit 1
fi

timezoneJson=$(curl http://127.0.0.1:5555/deviceManager/rest/dorado/system_timezone)

if [[ $a == *"CMO_SYS_TIME_ZONE_NAME"* ]]; then
  echo "${timezoneJson}"
fi
