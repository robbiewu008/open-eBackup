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
# build plugin framework
FILE_ROOT_DIR=$(cd $(dirname $0)/..; pwd)
SCANNER_DIR=${FILE_ROOT_DIR}/../common/FS_Scanner
MODULE_DIR=${FILE_ROOT_DIR}/../common/Module
OBLIGATION_ROOT=$1
if [ -z "$OBLIGATION_ROOT" ]; then
    echo "ERROR: Please provide open-source-obligation path"
    exit 1
fi
build_scanner()
{
    arch_type=$(uname -m)
    if [ "${arch_type}" = "x86_64" ]; then
        arch_type_dir="x86_64_centos6"
    elif [ "${arch_type}" = "aarch64" ]; then
        arch_type_dir="aarch64"
    else
        echo "ERR: Unsupported system architecture"
    fi
    mkdir -p ${SCANNER_DIR}/lib
    tar xzf ${OBLIGATION_ROOT}/FS_SCANNER/Linux/${arch_type_dir}/scanner.tar.gz -C ${SCANNER_DIR}
}

build_scanner
exit $?
