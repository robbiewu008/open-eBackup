#!/bin/bash
#
#  This file is a part of the open-eBackup project.
#  This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
#  If a copy of the MPL was not distributed with this file, You can obtain one at
#  http://mozilla.org/MPL/2.0/.
# 
#  Copyright (c) [2024] Huawei Technologies Co.,Ltd.
# 
#  THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
#  EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
#  MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# /
umask 0022

# 初始化环境变量
source ./common.sh

YAMLCPP_EXTRACTED_FOLDER_PATH=${VIRT_DEPS_SRCS_PATH}/${YAMLCPP}
YAMLCPP_INSTALL_PATH=${VIRT_DEPS_INSTALL_PATH}

function make_yamlcpp()
{
    YAMLCPP_VAR=$(ls ${YAMLCPP_INSTALL_PATH}/lib/*yaml*.a 2>/dev/null)
    if [ -n "${YAMLCPP_VAR}" ]; then
        log_echo "INFO: ${YAMLCPP} library alreay exists"
        return 0
    fi

    # cmake yamlcpp
    cd ${YAMLCPP_EXTRACTED_FOLDER_PATH}

    echo "INFO: Begin to cmake ${YAMLCPP} ."

    # outsourcing build
    rm -rf build-cmake
    mkdir -p build-cmake
    cd build-cmake
    cmake .. -DCMAKE_INSTALL_PREFIX=${YAMLCPP_INSTALL_PATH} -DYAML_CPP_BUILD_TESTS=OFF -DYAML_BUILD_SHARED_LIBS=OFF -DCMAKE_CXX_FLAGS="-fPIC"

    if [ $? -ne 0 ]; then
        log_echo "ERROR:   cmake ${YAMLCPP} failed."
        exit 1
    fi

    echo "INFO: Begin to make ${YAMLCPP}."
    make -j${numProc}
    if [ $? -ne 0 ]; then
        log_echo "ERROR:   make ${YAMLCPP} failed."
        exit 1
    fi

    log_echo "INFO: Begin to install ${YAMLCPP}."
    make install
    if [ $? -ne 0 ]; then
        log_echo "ERROR:   install ${YAMLCPP} failed."
        exit 1
    fi

    log_echo "INFO: install ${YAMLCPP} succ."
}

function make_patchelf()
{
    log_echo "INFO: Begin to install patchelf."
    local patchelf_extracted="${VIRT_DEPS_EXT_PKG_PATH}"/patchelf
    local patchelf_install="${VIRT_DEPS_EXT_PKG_PATH}"/patchelf/install

    if [ -f "${patchelf_install}"/bin/patchelf ]; then
        log_echo "INFO: patchelf alreay exists."
        return 0
    fi

    mkdir -p ${patchelf_extracted}
    pushd ${VIRT_DEPS_EXT_PKG_PATH}
    tar -zxf "${VIRT_DEPS_EXT_PKG_PATH}"/patchelf-*.tar.gz -C ${patchelf_extracted}
    cd "${patchelf_extracted}"/patchelf*/
    ./bootstrap.sh > ${VIRT_DEPS_EXT_PKG_PATH}/build_patchelf.log
    ./configure --prefix=${patchelf_install} >> ${VIRT_DEPS_EXT_PKG_PATH}/build_patchelf.log
    make >> ${VIRT_DEPS_EXT_PKG_PATH}/build_patchelf.log
    if [ $? -ne 0 ]; then
        log_echo "ERROR: make patchelf failed."
        exit 1
    fi

    make check >> ${VIRT_DEPS_EXT_PKG_PATH}/build_patchelf.log

    make install >> ${VIRT_DEPS_EXT_PKG_PATH}/build_patchelf.log
    if [ $? -ne 0 ]; then
        log_echo "ERROR: install patchelf failed."
        exit 1
    fi
    popd

    log_echo "INFO: Install patchelf success. install path: ${patchelf_install}."
}

function main()
{
    make_yamlcpp
    if [ $? -ne 0 ]; then
        log_echo "ERROR" "make ${YAMLCPP} failed"
        return 1
    fi


    make_patchelf
}

main