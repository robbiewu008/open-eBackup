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

VIRT_ROOT_DIR=$(cd $(dirname $0)/../..; pwd)
APPPLUGINS_ROOT_PATH=$(cd "${VIRT_ROOT_DIR}/.."; pwd)
PLUGINS_PATH="${APPPLUGINS_ROOT_PATH%/*}"
FRAMEWORK_DIR=$(cd "${APPPLUGINS_ROOT_PATH}/common/framework"; pwd)
OUTPUT_PKG_PATH=${FRAMEWORK_DIR}/output_pkg
MODULE_PATH=$(cd "${APPPLUGINS_ROOT_PATH}/common/Module"; pwd)
MODULE_THIRD_DIR=$(cd "${APPPLUGINS_ROOT_PATH}/common/Module/third_open_src"; pwd)
MODULE_LIB_DIR=${APPPLUGINS_ROOT_PATH}/common/Module/lib
FRAMEWORK_OUTPUT=${FRAMEWORK_DIR}/output_pkg
COMMON_PATH=${FRAMEWORK_DIR}/build/common
. ${COMMON_PATH}/common.sh
SCRIPT_NAME=$(basename $0)

type=$1

function copy_hcs()
{
    # 1、vrmVBSTools.jar
    if [ ! -f "${VIRT_ROOT_DIR}/vbstool/lib/vrmVBSTool.jar" ]; then
        echo "The vrmVBSTools.jar file cannot be found."
        exit 1
    else
        cp -rf "${MODULE_PATH}/src/dsware/build/jar/vrmVBSTool.jar" "${OUTPUT_PKG_PATH}/vbstool/lib"
    fi
 
    # 2、dependency lib
    if [ ! -d "${VIRT_ROOT_DIR}/vbstool/lib" ]; then
        echo "The vbstool depenency file[log4j/commons-lang3] cannot be found."
        exit 1
    else
        cp -rf "${VIRT_ROOT_DIR}/vbstool/lib/log4j*.jar" "${OUTPUT_PKG_PATH}/vbstool/lib"
        cp -rf "${VIRT_ROOT_DIR}/vbstool/lib/commons-lang3*.jar" "${OUTPUT_PKG_PATH}/vbstool/lib"
    fi
 
    # 3、depenency scripts
    cp -rf "${MODULE_PATH}/script/dsware/reg_fs.sh" "${OUTPUT_PKG_PATH}/bin"
    cp -rf "${MODULE_PATH}/script/dsware/security_sudo_disk.sh" "${OUTPUT_PKG_PATH}/bin"
    cp -rf "${MODULE_PATH}/script/dsware/security_sudo_vbs_cli.sh" "${OUTPUT_PKG_PATH}/bin"
    cp -rf "${MODULE_PATH}/script/dsware/superlog.sh" "${OUTPUT_PKG_PATH}/bin"
    cp -rf "${MODULE_PATH}/script/dsware/vrmVBSTool.sh" "${OUTPUT_PKG_PATH}/vbstool"

    # 4、copy script
    if [ ! -f "${VIRT_ROOT_DIR}/install/install.sh" ]; then
        echo "The install file cannot be found."
        exit 1
    else
        cp -rf ${VIRT_ROOT_DIR}/install/install.sh ${OUTPUT_PKG_PATH}/install
    fi

    if [ ! -f ${VIRT_ROOT_DIR}/install/uninstall.sh ]; then
        echo "The install file cannot be found."
        exit 1
    else
        cp -rf ${VIRT_ROOT_DIR}/install/uninstall.sh ${OUTPUT_PKG_PATH}/install
    fi

    if [ ! -f ${VIRT_ROOT_DIR}/install/sudo_set_caps.sh ]; then
        echo "The sudo_set_caps.sh file cannot be found."
        exit 1
    else
        cp -rf ${VIRT_ROOT_DIR}/install/sudo_set_caps.sh ${OUTPUT_PKG_PATH}/install
    fi

    if [ ! -f ${VIRT_ROOT_DIR}/build/python/python_env.sh ]; then
        echo "The python_env file cannot be found."
        exit 1
    else
        cp -rf ${VIRT_ROOT_DIR}/build/python/python_env.sh ${OUTPUT_PKG_PATH}/install
    fi
}

function copy_3rd_lib()
{
    # copy third libs
    cp -rf ${MODULE_PATH}/third_open_src/lz4_rel/lib/*.so* ${OUTPUT_PKG_PATH}/lib/3rd
    cp -rf ${MODULE_PATH}/third_open_src/libaio_rel/lib/*.so* ${OUTPUT_PKG_PATH}/lib/3rd
    cp -rf ${MODULE_PATH}/third_open_src/openssl_rel/lib/*.so* ${OUTPUT_PKG_PATH}/lib/3rd
    cp -rf ${MODULE_PATH}/third_open_src/boost_rel/lib/*.so* ${OUTPUT_PKG_PATH}/lib/3rd
    cp -rf ${MODULE_PATH}/third_open_src/thrift_rel/lib/*.so* ${OUTPUT_PKG_PATH}/lib/3rd
    cp -rf ${MODULE_PATH}/third_open_src/jsoncpp_rel/libs/*.so* ${OUTPUT_PKG_PATH}/lib/3rd
    cp -rf ${MODULE_PATH}/third_open_src/curl_rel/lib/*.so* ${OUTPUT_PKG_PATH}/lib/3rd
    cp -rf ${MODULE_PATH}/third_open_src/libssh2_rel/lib/*.so* ${OUTPUT_PKG_PATH}/lib/3rd
    cp -rf ${MODULE_PATH}/third_open_src/c-ares_rel/lib/*.so* ${OUTPUT_PKG_PATH}/lib/3rd

    # copy platform libs
    cp -rf ${MODULE_PATH}/platform/KMCv3_infra_rel/lib/*.so* ${OUTPUT_PKG_PATH}/lib/3rd
    cp -rf ${MODULE_PATH}/platform/SecureCLib_rel/lib/*.so* ${OUTPUT_PKG_PATH}/lib/3rd
}

function copy_dep_binary()
{
    # copy patchelf
    if [ ! -f "${VIRT_ROOT_DIR}"/deps/patchelf ]; then
        echo "The patchelf file cannot be found."
        exit 1
    else
        cp -rf "${VIRT_ROOT_DIR}"/deps/patchelf "${OUTPUT_PKG_PATH}"/bin
    fi
}

function copy_file()
{
    # 1、copy plugin_attribute_1.0.0.json
    if [ ! -f ${VIRT_ROOT_DIR}/conf/plugin_attribute_*.json ]; then
        echo "The plugin attribute file cannot be found."
        exit 1
    else
        cp -rf ${VIRT_ROOT_DIR}/conf/plugin_*.json ${OUTPUT_PKG_PATH}/conf
    fi

    if [ ! -f ${VIRT_ROOT_DIR}/conf/param_check.xml ]; then
        echo "The param_check.xml file cannot be found."
        exit 1
    else
        cp -rf ${VIRT_ROOT_DIR}/conf/param_check.xml ${OUTPUT_PKG_PATH}/conf
    fi

    # 2、copy compile so file
    cp -rf ${VIRT_ROOT_DIR}/lib/*.so ${OUTPUT_PKG_PATH}/lib/service

    # 3、copy 3rd libs
    copy_3rd_lib

    # 4、copy hcpconf.ini
    if [ ! -f ${VIRT_ROOT_DIR}/conf/hcpconf.ini ]; then
        echo "The hcpconf.ini file cannot be found."
        exit 1
    else
        cp -rf ${VIRT_ROOT_DIR}/conf/hcpconf.ini ${OUTPUT_PKG_PATH}/conf
    fi

    # 5、copy hcs script file
    copy_hcs

    # 6、copy dependency file
    copy_dep_binary

    # 7、copy python file
    python3_file=${VIRT_ROOT_DIR}/deps/python3.pluginFrame.${SYS_ARCH}.tar.gz
    # 内置代理不拷python包
    if [ ${INTERNAL_PLUGIN} = "1" ]; then
        echo "Internal plugins does not need packag python."
        return 0
    fi
    if [ ! -f ${python3_file} ]; then
        echo "The ${python3_file} file cannot be found."
        exit 1
    else
        cp -rf ${python3_file} "${OUTPUT_PKG_PATH}/install"
    fi
}

main()
{
    # build plugin
    sh ${VIRT_ROOT_DIR}/CI/scripts/build_virt_opensource.sh "$@"
    if [ $? -ne 0 ]; then
        log_echo "ERROR" "Building file lib failed"
        exit 1
    fi

    # build file plugin
    copy_file

    # Execute framework package
    sh ${FRAMEWORK_DIR}/build/pack.sh
}

main "$@"
