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

FILE_ROOT_DIR=$(cd $(dirname $0)/../..; pwd)
APPPLUGINS_ROOT_PATH=$(cd "${FILE_ROOT_DIR}/.."; pwd)
FRAMEWORK_DIR=$(cd "${APPPLUGINS_ROOT_PATH}/common/framework"; pwd)
MODULE_THIRD_DIR=$(cd "${APPPLUGINS_ROOT_PATH}/common/Module/third_open_src"; pwd)
MODULE_LIB_DIR=${APPPLUGINS_ROOT_PATH}/common/Module/lib
FRAMEWORK_OUTPUT=${FRAMEWORK_DIR}/output_pkg
COMMON_PATH=${FRAMEWORK_DIR}/build/common
. ${COMMON_PATH}/common.sh
SCRIPT_NAME=$(basename $0)

type=$1

copy_boost_for_suse()
{
    local system_name=$(uname -p)
    if [ ${system_name} != "x86_64" ]; then
        return 0
    fi
    local third_branch="debug_OceanProtect_DataBackup_1.6.0_openeBackup_v2"
    if [ ${branch} != "" ];then
        third_branch=${branch}
    fi

    mkdir -p ${FRAMEWORK_OUTPUT}/lib/3rd/suse_temp
    mkdir -p ${FRAMEWORK_OUTPUT}/lib/3rd/suse
    # download suse third_open_src from cmc, copy this when install on SUSE11
    artget pull -d ${FRAMEWORK_DIR}/../Module/build/LCRP/conf/opensrc_suse_cmc.xml -p "{'COMPOENT_VERSION':'1.2.1RC1','PRODUCT':'dorado', \
    'THIRD_BRANCH':'${third_branch}','OS_TYPE':'X86','SYSTEM_NAME':'Suse11'}" -ap ${FRAMEWORK_OUTPUT}/lib/3rd/suse_temp -user ${cmc_user} -pwd ${cmc_pwd}
    cd ${FRAMEWORK_OUTPUT}/lib/3rd/suse_temp
    tar -zxf OceanProtect_X8000_1.2.1RC1_Opensrc_3rd_SDK_Suse11_X86.tar.gz
    tar -zxf agent/boost_agent_rel.tar.gz
    find ${FRAMEWORK_OUTPUT}/lib/3rd/suse_temp -name *.so* | xargs -I file cp file ${FRAMEWORK_OUTPUT}/lib/3rd/suse
    cd ..
    rm -rf ${FRAMEWORK_OUTPUT}/lib/3rd/suse_temp
}

copy_file()
{
    if [ "${OS_TYPE}" = "AIX" ]; then
        libName="*.a"
    else
        libName="*.so*"
    fi
    # Copy conf files
    mkdir -p  ${FRAMEWORK_OUTPUT}/conf
    cp -f ${FILE_ROOT_DIR}/conf/* ${FRAMEWORK_OUTPUT}/conf

    # Copy bin script files
    mkdir -p  ${FRAMEWORK_OUTPUT}/bin
    cp -f ${FILE_ROOT_DIR}/bin/* ${FRAMEWORK_OUTPUT}/bin

    # Copy file plugin library
    NAS_LIB_PATH=${FILE_ROOT_DIR}/build-cmake
    mkdir -p ${FRAMEWORK_OUTPUT}/lib/service
    find ${NAS_LIB_PATH} -name $libName | xargs -I{} cp -f {} ${FRAMEWORK_OUTPUT}/lib/service
    find ${MODULE_LIB_DIR} -name $libName | xargs -I{} cp -f {} ${FRAMEWORK_OUTPUT}/lib/service

    # Copy file plugin library
    SCANNER_DIR=$(cd "${APPPLUGINS_ROOT_PATH}/common/FS_Scanner"; pwd)
    SCANNER_LIB_PATH=${SCANNER_DIR}/lib
    find ${SCANNER_LIB_PATH} -name $libName | xargs -I{} cp -f {} ${FRAMEWORK_OUTPUT}/lib/service

    # Copy file plugin library
    BACKUP_DIR=$(cd "${APPPLUGINS_ROOT_PATH}/common/FS_Backup"; pwd)
    BACKUP_LIB_PATH=${BACKUP_DIR}/build-cmake
    find ${BACKUP_LIB_PATH} -name $libName | xargs -I{} cp -f {} ${FRAMEWORK_OUTPUT}/lib/service

    # Copy opensource
    if [ "${OS_TYPE}" = "SunOS" ]; then
        OPENSRC_LIST="sqlite jsoncpp openssl"
    else
        OPENSRC_LIST="sqlite jsoncpp openssl 7z c-ares libuuid"
    fi
    mkdir -p ${FRAMEWORK_OUTPUT}/lib/3rd
    for opensrc in ${OPENSRC_LIST}; do
        find ${MODULE_THIRD_DIR}/${opensrc}_rel -name ${libName} | xargs -I{} cp -f {} ${FRAMEWORK_OUTPUT}/lib/3rd
    done

    # Copy extral 3rd for SUSE11
    copy_boost_for_suse
    
    # Copy script into package
    cp -f ${FILE_ROOT_DIR}/build/install/*.sh ${FRAMEWORK_OUTPUT}/
}

main()
{
    # build plugin
    sh ${FILE_ROOT_DIR}/CI/script/build_opensource.sh "$@"
    if [ $? -ne 0 ]; then
        log_echo "ERROR" "Building file lib failed"
        exit 1
    fi

    # build file plugin
    copy_file

    # Execute framework package
    sh ${FRAMEWORK_DIR}/build/pack.sh
    sh ${FRAMEWORK_DIR}/build/generate_full_pkg.sh FilePlugin
}

main "$@"
