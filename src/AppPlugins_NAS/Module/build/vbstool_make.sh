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

CPWD=`dirname $0` && cd ${CPWD} && CPWD=`pwd`
SRC_DIR="${CPWD}/../src/dsware/"
PLATFORM_DIR="${CPWD}/../platform/"

function dependency_pre()
{
    # unzip [FusionStorage_api_8.0.1.SPH606.tar.gz]
    if [ ! -d "${PLATFORM_DIR}/DSware" ]; then
        echo "ERROR: FusionStorage api package no found."
        exit 1
    fi

    if [ -d "${PLATFORM_DIR}/dsware_rel" ]; then
        rm -rf "${PLATFORM_DIR}/dsware_rel"
    fi

    mkdir -p "${PLATFORM_DIR}/dsware_rel/api_lib"

    # unzip [log4j]
    mkdir -p "${PLATFORM_DIR}/dsware_rel/dependency_lib"

    if ls ${PLATFORM_DIR}/DSware/FusionStorage* 1> /dev/null 2>&1; then
        tar -zxvf ${PLATFORM_DIR}/DSware/FusionStorage* -C "${PLATFORM_DIR}/dsware_rel/api_lib"
    elif ls ${PLATFORM_DIR}/DSware/OceanStor* 1> /dev/null 2>&1; then
        tar -zxvf ${PLATFORM_DIR}/DSware/OceanStor* -C "${PLATFORM_DIR}/dsware_rel/api_lib"
        cp ${PLATFORM_DIR}/dsware_rel/api_lib/OceanStor*/lib/*.jar "${PLATFORM_DIR}/dsware_rel/dependency_lib"
        cp ${PLATFORM_DIR}/dsware_rel/api_lib/OceanStor*/*.jar "${PLATFORM_DIR}/dsware_rel/dependency_lib"
    fi

    if ls ${PLATFORM_DIR}/DSware/apache-log4j*.tar.gz 1> /dev/null 2>&1; then
        tar -zxf ${PLATFORM_DIR}/DSware/apache-log4j* -C "${PLATFORM_DIR}/dsware_rel/dependency_lib"
        cp "${PLATFORM_DIR}/dsware_rel/dependency_lib/apache-log4j-2.18.0-bin/log4j-1.2-api-2.18.0.jar" "${PLATFORM_DIR}/dsware_rel/dependency_lib"
        cp "${PLATFORM_DIR}/dsware_rel/dependency_lib/apache-log4j-2.18.0-bin/log4j-api-2.18.0.jar" "${PLATFORM_DIR}/dsware_rel/dependency_lib"
        cp "${PLATFORM_DIR}/dsware_rel/dependency_lib/apache-log4j-2.18.0-bin/log4j-core-2.18.0.jar" "${PLATFORM_DIR}/dsware_rel/dependency_lib"
    elif ls ${PLATFORM_DIR}/DSware/apache-log4j*.zip 1> /dev/null 2>&1; then
        unzip ${PLATFORM_DIR}/DSware/apache-log4j* log4j-1.2-api-2.23.1.jar log4j-api-2.23.1.jar  log4j-core-2.23.1.jar -d "${PLATFORM_DIR}/dsware_rel/dependency_lib"
    fi

    # copy [common-lang3]
    cp ${PLATFORM_DIR}/DSware/commons-lang3*.jar "${PLATFORM_DIR}/dsware_rel/dependency_lib"
}

function compile_vbstools()
{
    pushd ${SRC_DIR} > /dev/null
    ant -file build.xml clean
    if [ $? -ne 0 ]; then
        echo "clean vbstool failed"
        exit 1
    fi
    ant -file build.xml init
    if [ $? -ne 0 ]; then
        echo "init vbstool compile failed"
        exit 1
    fi
    ant -file build.xml compile
    if [ $? -ne 0 ]; then
        echo "compile vbstool failed"
        exit 1
    fi
    popd > /dev/null
}

main()
{
    dependency_pre

    # compile
    compile_vbstools
}

main