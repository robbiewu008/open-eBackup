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
set -x
SYS_NAME=`uname -s`
BASE_PATH=""
if [ "${SYS_NAME}" = "AIX" ]; then
    BASE_PATH="$(cd "$(dirname $0)/../" && pwd)"
else
    BASE_PATH="$(cd "$(dirname "$BASH_SOURCE")/../" && pwd)"
fi
DB_INC_PATH="${BASE_PATH}/inc"
DB_LIB_PATH="${BASE_PATH}/lib"
DB_BIN_PATH="${BASE_PATH}/bin"
DB_BUILD_PATH="${BASE_PATH}/build-cmake"
MODULE_DIR="${BASE_PATH}/../common/Module"

ReplaceFlagsMake()
{
    cd "${BASE_PATH}/build-cmake"
    flags_list=`find ./ -name flags.make`
    for item in $flags_list; do
        sed 's/-isystem /-I/g' $item > $item.bk
        mv $item.bk $item
    done
}

MakePlugin()
{
    # 内置插件
    INTERNAL_CMAKE=""
    if [ "${INTERNAL_PLUGIN}" = "1" ]; then
        INTERNAL_CMAKE="-DINTERNAL_PLUGIN=ON"
    fi
    if [ "$1" == "LLT" ]; then
        mkdir -p "${BASE_PATH}/build-llt"
        cd "${BASE_PATH}/build-llt"
        cmake -D LLT=ON ../src
    elif [[ "$1" == "release" || "$1" == "Release" ]]; then
        mkdir -p "${BASE_PATH}/build-cmake"
        cd "${BASE_PATH}/build-cmake"
        cmake -D ${INTERNAL_CMAKE} CMAKE_BUILD_TYPE=Release ../src
    else
        mkdir -p "${BASE_PATH}/build-cmake"
        cd "${BASE_PATH}/build-cmake"
        cmake ${INTERNAL_CMAKE} ../src
    fi

    if [ $? -ne 0 ]; then
        echo "cmake error!!!"
        exit 1
    fi
    if [ ${SYS_NAME} = "AIX" ]; then
        ReplaceFlagsMake
    fi
    make -j8 $@
    if [ $? -ne 0 ]; then
        echo "make error!!!"
        exit 1
    fi
}

CopyFilesToPath()
{
    if [ "$1" == "LLT" ]; then
        echo "build type is llt, cannot install."
        return 0
    fi

    mkdir -p ${DB_INC_PATH}
    mkdir -p ${DB_LIB_PATH}
    mkdir -p ${DB_BIN_PATH}
    if [ -d ${DB_BUILD_PATH} ]; then
        cp -f ${DB_BUILD_PATH}/libdatabase* ${DB_LIB_PATH}
        cp -f ${DB_BUILD_PATH}/tools/dbrpctool/dbrpctool ${DB_BIN_PATH}
        cp -f ${DB_BUILD_PATH}/applications/backint/backint ${DB_BIN_PATH}
        strip ${DB_BIN_PATH}/backint
        strip ${DB_BIN_PATH}/dbrpctool
        cp -f ${BASE_PATH}/src/job/*.h ${DB_INC_PATH}
    else
        echo "${DB_BUILD_PATH} is not exist."
    fi
}

main()
{
    if [ "$1" == "clean" ]; then
        echo "${BASE_PATH}/build-cmake"
        rm -rf "${BASE_PATH}/build-cmake"
        echo "clean success."
        exit 0
    fi
    echo "#######################################################################################################"
    echo "Start compile DbPlugin."
    echo "#######################################################################################################"
    MakePlugin $@
    CopyFilesToPath $@
    echo "#######################################################################################################"
    echo "DbPlugin compile success."
    echo "#######################################################################################################"
}

main $@
