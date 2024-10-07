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
# build AppPlugins_NAS
SCRIPT_PATH=$(dirname ${BASH_SOURCE[0]})
source ${SCRIPT_PATH}/common/common_artget.sh
source ${PLUGIN_ROOT_DIR}/build/common/branch.sh
SCRIPT_NAME="${BASH_SOURCE[0]##*/}"

CODE_BRANCH=$1
if [ -z "${CODE_BRANCH}" ]; then
    CODE_BRANCH="${NAS_BRANCH}"
fi

function upload_plugin_pkg()
{
    log_echo "Begin down 3rd from cmc"
    upload_plugin_2_cmc ${PRODUCT} ${CODE_BRANCH} Plugins
    if [ $? -ne 0 ]; then
        log_echo "Download artifact error"
        exit 1
    fi
}

function main()
{
    upload_plugin_pkg "$@"
    return $?
}

main "$@"
exit $?