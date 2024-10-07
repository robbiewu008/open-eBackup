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
BACKUP_DIR=$(cd "${FILE_ROOT_DIR}/../../FS_Backup"; pwd)
MODULE_DIR=$(cd "${FILE_ROOT_DIR}/../../Module"; pwd)
build_type=$1
build_backup()
{
    export MODULE_ROOT_PATH=${MODULE_DIR}
    sh ${BACKUP_DIR}/build/src/make_backup.sh --path=${MODULE_DIR} --mode=VOLUME
    exit $?
}

build_backup
exit $?