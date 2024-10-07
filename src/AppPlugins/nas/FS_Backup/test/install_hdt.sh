#!/bin/bash
#
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
#

cli_pkg=core-cli-master.tar.gz
cli_addr=https://codehub-y.huawei.com/hdt/core-cli/files?ref=master

huawei_tool_addr=cmc-szver-artifactory.cmc.tools.huawei.com

hdt_link=/usr/local/bin/hdt
gcovr_link=/usr/local/bin/gcovr

function main()
{
    hdt -V
    if [ $? -eq 0 ]; then
        echo "hdt already install"
        return 0
    fi

    if [ ! -e "$cli_pkg" ]
    then
        echo "No pkg $cli_pkg, you can download pkg from $cli_addr"
        #return 1
    fi

    which pip3
    if [ $? != 0 ]; then
        echo "No pip3"
        return 1
    fi

    ping -c 4 $huawei_tool_addr
    if [ $? != 0 ]; then
        echo $?
        echo "Please Check local setting"
        return 1
    fi

    yum install libxml2 -y
    yum install libxslt -y

    pip3 install --upgrade pip

    pip3 install $cli_pkg

    py3_link=`which python3`
    echo "py3: $py3_link"

    # according to link, find exe file path
    py3_path=`ls -l $py3_link | awk  '{print $11}'`
    echo "py3 path: $py3_path"

    py3_dir=`dirname $py3_path`
    echo "py3 dir: $py3_dir"

    if [ ! -e "$hdt_link" ]
    then
        echo "Create ln: $hdt_link"
        ln -s $py3_dir/hdt $hdt_link
    fi
    if [ ! -e "$gcovr_link" ]
    then
        echo "Create ln: $gcovr_link"
        ln -s $py3_dir/gcovr $gcovr_link
    fi
    whereis hdt
}

main "$@"