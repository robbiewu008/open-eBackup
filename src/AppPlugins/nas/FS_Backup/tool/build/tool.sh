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
SCRIPT_PATH=$(cd $(dirname $0); pwd)
ROOT_PATH=${SCRIPT_PATH}/../..
RELEASE_PATH=${SCRIPT_PATH}/bin

find -name *.sh | xargs dos2unix

echo "Backup root path is ${ROOT_PATH}"
rm -rf ${RELEASE_PATH}
mkdir -p $RELEASE_PATH
echo "copy backup demo dependency to ${RELEASE_PATH}"
find $ROOT_PATH -name *.so | xargs -i cp {} $RELEASE_PATH
find $ROOT_PATH -name *.so.\* | xargs -i cp {} $RELEASE_PATH
export LD_LIBRARY_PATH=$RELEASE_PATH

mkdir -p ${SCRIPT_PATH}/build-cmake
cd ${SCRIPT_PATH}/build-cmake
numProc=$(cat /proc/cpuinfo | grep processor | wc -l)
cmake ../../src
make -j${numProc}
if [ $? -ne 0 ]; then
    echo "make scanner demo error"
    exit 1
fi