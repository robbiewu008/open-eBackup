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
FRAMEWORK_DIR=$(cd $(dirname $0)/../..; pwd)
COMMON_PATH=${FRAMEWORK_DIR}/build/common
. ${COMMON_PATH}/common.sh
SCRIPT_NAME=$(basename $0)

CODE_BRANCH=$1
if [ -z "${CODE_BRANCH}" ]; then
    CODE_BRANCH="${NAS_BRANCH}"
fi

main()
{
    # build
    chmod u+x ${FRAMEWORK_DIR}/build/build.sh
    ${FRAMEWORK_DIR}/build/build.sh
    if [ $? -ne 0 ]; then
        log_echo "ERROR" "Building framework lib failed"
        exit 1
    fi

    # packe
    cd ${PLUGIN_FRAMEWORK_LIB_PATH}
    typeset system_type=$(uname -s)
    if [ "${system_type}" = "AIX" ]; then
        typeset cpu_type=ppc_$(getconf HARDWARE_BITMODE)
        tar cvf cppframework-${system_type}_${cpu_type}.tar *
        xz -v -T2 -2 cppframework-${system_type}_${cpu_type}.tar
    elif [ "${system_type}" = "SunOS" ]; then
        typeset cpu_type=$(uname -m)
        tar cvf cppframework-${system_type}_${cpu_type}.tar *
        gzip cppframework-${system_type}_${cpu_type}.tar
    else
        cpu_type=$(uname -m)
        tar cvf cppframework-${system_type}_${cpu_type}.tar *
        xz -v -T0 -9e cppframework-${system_type}_${cpu_type}.tar
    fi
    rm -rf ${PLUGIN_PACKAGE_PATH}/*
    if [ "${system_type}" = "SunOS" ]; then
        mv cppframework-${system_type}_${cpu_type}.tar.gz ${PLUGIN_PACKAGE_PATH}
    else
        mv cppframework-${system_type}_${cpu_type}.tar.xz ${PLUGIN_PACKAGE_PATH}
    fi
    
}

main "$@"
