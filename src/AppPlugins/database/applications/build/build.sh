#!/bin/sh
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
set +x
# compile script
SYS_NAME=`uname -s`
BASEDIR=""
if [ "${SYS_NAME}" = "AIX" ]; then
    BASEDIR="$(cd "$(dirname $0)" && pwd)"
else
    BASEDIR=$(cd $(dirname ${BASH_SOURCE[0]}); pwd)
fi

script_dest_path=$1
conf_dest_path=$2

cp -rp "${BASEDIR}/../../applications/." $script_dest_path
cp -rp "${BASEDIR}/../conf/." $conf_dest_path

#make any user can build package and remove file
chmod -R 777 "$script_dest_path"
rm -rf "$script_dest_path/build"
rm -rf "$script_dest_path/conf"
rm -rf "$script_dest_path/test"

#make install package mini permission
chmod -R 550 "$script_dest_path"
