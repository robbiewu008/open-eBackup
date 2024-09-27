#!/bin/sh
#
#  This file is a part of the open-eBackup project.
#  This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
#  If a copy of the MPL was not distributed with this file, You can obtain one at
#  http://mozilla.org/MPL/2.0/.
# 
#  Copyright (c) [2024] Huawei Technologies Co.,Ltd.
# 
#  THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
#  EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
#  MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# /
set +x

function remove_sudoers()
{
    # 1. remove /etc/sudoers rdadmin
    fileName="/etc/sudoers"
    lineNo=`cat $fileName | grep -n -w "VirtualizationPlugin" | head -n 1 | awk -F ':' '{print $1}'`
    if [ "${lineNo}" != "" ]; then
        sed -i "${lineNo}d" "$fileName"
        remove_sudoers
    fi
}

main()
{
    remove_sudoers

    return 0
}

main