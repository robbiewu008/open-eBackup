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


function check_application_and_port()
{
    curl -kv --connect-timeout 3 --max-time 3 $POD_IP:elasticsearch_port 2>&1 | grep 'Connected'
    if [[ $? == 0 ]]; then
        echo "Detect connection to elasticsearch successfully"
        exit 0
    else
        echo "Detect connection to elasticsearch failed"
        exit 1
    fi
}

function main()
{
    check_application_and_port
}

main