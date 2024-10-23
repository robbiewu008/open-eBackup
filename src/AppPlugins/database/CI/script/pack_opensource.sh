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

SYS_ARCH=$(uname -m)
SYS_NAME=$(uname -s)
DATABASE_PATH=""
if [ "${SYS_NAME}" = "AIX" ]; then
    DATABASE_PATH="$(cd "$(dirname $0)/../.." && pwd)"
else
    DATABASE_PATH="$(cd "$(dirname "$BASH_SOURCE")/../.." && pwd)"
fi
APPLICATIONS_BUILD_PATH="${DATABASE_PATH}/applications/build"
PLUGINS_PATH="${DATABASE_PATH%/*}"
FRAMEWORK_PATH="${PLUGINS_PATH%/*}/AppPlugins/common/framework"
MODULE_PATH="${PLUGINS_PATH%/*}/AppPlugins/common/Module"
OUTPUT_PKG_PATH="${PLUGINS_PATH%/*}/AppPlugins/common/framework/output_pkg"

# open-eBackup-bin
OPEN_OBLIGATION_ROOT_PATH=${binary_path}
if [ -z "$OPEN_OBLIGATION_ROOT_PATH" ]; then
    echo "ERROR: Please export binary_path={open-source-obligation path}"
    exit 1
fi
OPEN_OBLIGATION_TRDPARTY_PATH="${OPEN_OBLIGATION_ROOT_PATH}/ThirdParty"
OPEN_OBLIGATION_PLUGIN_PATH="${OPEN_OBLIGATION_ROOT_PATH}/Plugin"
PYTHON_PLG_PKG_PATH=${PLUGINS_PATH}/python3_pluginFrame

MODULE_PKG_PATH=${OPEN_OBLIGATION_PLUGIN_PATH}/Module
FS_SCANNER_PKG_PATH=${OPEN_OBLIGATION_ROOT_PATH}/FS_SCANNER
FS_BACKUP_PKG_PATH=${OPEN_OBLIGATION_ROOT_PATH}/FS_BACKUP
APP_GENERALDB_PKG_PATH=${OPEN_OBLIGATION_ROOT_PATH}/Plugins

INTERNAL_PLUGIN=0

function download_module_pkg() {
    if [ "${SYS_ARCH}" == "aarch64" ]; then
        ARCH="aarch64"
    elif grep -q "release 6" /etc/redhat-release; then
        ARCH="x86_64_centos6"
    else
        ARCH="x86_64_centos7"
    fi

    local ext_pkg_path=${PLUGINS_PATH}/opensrc_temp/Module
    [ -d ${ext_pkg_path} ] && rm -rf ${ext_pkg_path}
    mkdir -p ${ext_pkg_path}

    local opensrc_pkg_name=Module_rel.tar.gz
    cp ${MODULE_PKG_PATH}/Linux/${ARCH}/${opensrc_pkg_name} ${ext_pkg_path}
    if [ $? -ne 0 ]; then
        echo "down Module pkgs from cmc error"
        return 1
    fi
    cd ${ext_pkg_path}
    tar -zxf ${opensrc_pkg_name} -C "${MODULE_PATH}"
    echo "Finish to down Module pkgs into cmc"
    return 0
}

download_and_pack_module() {
    if [ "${SYS_NAME}" = "AIX" ]; then
        return 0
    fi

    #编译Moudle
    sh ${MODULE_PATH}/build/build_module.sh
    if [ $? -ne 0 ]; then
        exit 1
    fi

    if [ ! -f ${MODULE_PATH}/third_open_src/lnfs_rel/lib/libnfs.so* ]; then
        echo "The libnfs so file cannot be found."
    else
        cp -rf ${MODULE_PATH}/third_open_src/lnfs_rel/lib/libnfs.so* ${OUTPUT_PKG_PATH}/lib/3rd
    fi

    if ! ls ${MODULE_PATH}/third_open_src/lsmb2_rel/lib/libsmb2.so* 1>/dev/null 2>&1; then
        echo "The libsmb2 so file cannot be found."
    else
        cp -rf ${MODULE_PATH}/third_open_src/lsmb2_rel/lib/libsmb2.so* ${OUTPUT_PKG_PATH}/lib/3rd
    fi

    if ! ls ${MODULE_PATH}/third_open_src/protobuf_rel/lib/libprotobuf-lite.so* 1>/dev/null 2>&1; then
        echo "The libprotobuf-lite so file cannot be found."
    else
        cp -rf ${MODULE_PATH}/third_open_src/protobuf_rel/lib/libprotobuf-lite.so* ${OUTPUT_PKG_PATH}/lib/3rd
    fi

    if ! ls ${MODULE_PATH}/third_open_src/protobuf_rel/lib/libprotobuf.so.* 1>/dev/null 2>&1; then
        echo "the libprotobuf so file can not be found."
    else
        cp -rf ${MODULE_PATH}/third_open_src/protobuf_rel/lib/libprotobuf.so.* ${OUTPUT_PKG_PATH}/lib/3rd
    fi

    if ! ls ${MODULE_PATH}/third_open_src/tirpc_rel/libs/libtirpc.so* 1>/dev/null 2>&1; then
        echo "the libtirpc so file can not be found."
    else
        cp -rf ${MODULE_PATH}/third_open_src/tirpc_rel/libs/libtirpc.so* ${OUTPUT_PKG_PATH}/lib/3rd
    fi

    if [ ! -f ${MODULE_PATH}/lib/libsmb_ctx.so ]; then
        echo "The libsmb_ctx.so file cannot be found."
    else
        cp -rf ${MODULE_PATH}/lib/libsmb_ctx.so ${OUTPUT_PKG_PATH}/lib
    fi

    if [ ! -f ${MODULE_PATH}/lib/libnfs_ctx.so ]; then
        echo "The libnfs_ctx.so file cannot be found."
    else
        cp -rf ${MODULE_PATH}/lib/libnfs_ctx.so ${OUTPUT_PKG_PATH}/lib
    fi

    if [ ! -f ${MODULE_PATH}/lib/libdevice_access.so ]; then
        echo "The libdevice_access.so file cannot be found."
    else
        cp -rf ${MODULE_PATH}/lib/libdevice_access.so ${OUTPUT_PKG_PATH}/lib
    fi

    if [ ! -f ${MODULE_PATH}/lib/libcurl_http_util.so ]; then
        echo "The libcurl_http_util.so file cannot be found."
    else
        cp -rf ${MODULE_PATH}/lib/libcurl_http_util.so ${OUTPUT_PKG_PATH}/lib
    fi

    if [ ! -f ${MODULE_PATH}/lib/libparser.so ]; then
        echo "The libparser.so file cannot be found."
        exit 1
    else
        cp -rf ${MODULE_PATH}/lib/libparser.so ${OUTPUT_PKG_PATH}/lib
    fi

    if [ ! -f ${MODULE_PATH}/lib/libthreadpool.so ]; then
        echo "The libthreadpool.so file cannot be found."
        exit 1
    else
        cp -rf ${MODULE_PATH}/lib/libthreadpool.so ${OUTPUT_PKG_PATH}/lib
    fi

    if [ ! -f ${MODULE_PATH}/lib/libmetafile_parser.so ]; then
        echo "The libmetafile_parser.so file cannot be found."
        exit 1
    else
        cp -rf ${MODULE_PATH}/lib/libmetafile_parser.so ${OUTPUT_PKG_PATH}/lib
    fi

    if [ ! -f ${MODULE_PATH}/lib/libsystem.so ]; then
        echo "The libsystem.so file cannot be found."
        exit 1
    else
        cp -rf ${MODULE_PATH}/lib/libsystem.so ${OUTPUT_PKG_PATH}/lib
    fi

    if [ ! -f ${MODULE_PATH}/lib/libndmp_client.so ]; then
        echo "The libndmp_client.so file cannot be found"
    else
        cp -rf ${MODULE_PATH}/lib/libndmp_client.so ${OUTPUT_PKG_PATH}/lib
    fi
}

execute_build_script() {
    sh ${DATABASE_PATH}/CI/script/build_opensource.sh
    if [ $? -ne 0 ]; then
        echo "Failed to execute the build script."
        exit 1
    fi
}

create_dir() {
    # 1、create bin/applications path
    if [ ! -d "${OUTPUT_PKG_PATH}/bin/applications" ]; then
        mkdir -p "${OUTPUT_PKG_PATH}/bin/applications"
    fi

    # 2、create conf path
    if [ ! -d "${OUTPUT_PKG_PATH}/conf" ]; then
        mkdir -p "${OUTPUT_PKG_PATH}/conf"
    fi

    # 3、create service path
    if [ ! -d "${OUTPUT_PKG_PATH}/lib/service" ]; then
        mkdir -p "${OUTPUT_PKG_PATH}/lib/service"
    fi

    # 4、create service path
    if [ ! -d "${OUTPUT_PKG_PATH}/install" ]; then
        mkdir -p "${OUTPUT_PKG_PATH}/install"
    fi
    chmod 700 ${OUTPUT_PKG_PATH}/install
}

download_python3_pluginFrame() {
    # 内置插件不需要打包python
    if [ ${INTERNAL_PLUGIN} = "1" ]; then
        echo "Internal plugins does not need packag python."
        psutil_packages=psutil-5.9.0-cp39-cp39-linux_aarch64.whl
        cp ${PYTHON_PLG_PKG_PATH}/${psutil_packages} ${OUTPUT_PKG_PATH}/install
        if [ $? -ne 0 ]; then
            echo "Download psutil pkg from cmc error."
            exit 1
        fi
        return 0
    fi
    python3_file=python3.pluginFrame.${SYS_ARCH}.tar.gz
    if [ -e ${OUTPUT_PKG_PATH}/install/${python3_file} ]; then
        rm -f ${OUTPUT_PKG_PATH}/install/${python3_file}
    fi
    if [ "${SYS_NAME}" = "AIX" ]; then
        # CI下载python安装包，这里只负责拷贝
        python3_aix_file=python3.pluginFrame.AIX.tar.gz
        cp ${PYTHON_PLG_PKG_PATH}/${python3_aix_file} ${OUTPUT_PKG_PATH}/install
        if [ $? -ne 0 ]; then
            echo "Copy aix python package failed."
            exit 1
        fi
        return 0
    fi

    if [ "${SYS_ARCH}" == "aarch64" ]; then
        SYSTEM_NAME="CentOS7.6"
        OS_TYPE="ARM"
    else
        SYSTEM_NAME="CentOS6.10"
        OS_TYPE="X86"
    fi
    local ext_pkg_path=${PLUGINS_PATH}/opensrc_temp/python3
    [ -d ${ext_pkg_path} ] && rm -rf ${ext_pkg_path}
    mkdir -p ${ext_pkg_path}

    COMPOENT_VERSION="1.2.1RC1"
    OPEN_SOURCE_TYPE="third_party_groupware"
    pkg_name=OceanProtect_X8000_${COMPOENT_VERSION}_Opensrc_3rd_SDK_${SYSTEM_NAME}_${OS_TYPE}.tar.gz
    cp ${OPEN_OBLIGATION_TRDPARTY_PATH}/${SYSTEM_NAME}/${OS_TYPE}/${OPEN_SOURCE_TYPE}/${pkg_name} ${ext_pkg_path}
    if [ $? -ne 0 ]; then
        echo "Copy ${pkg_name} python package failed."
        exit 1
    fi
    cd ${ext_pkg_path}
    tar -zxvf ${pkg_name}
    tar -zxvf python3_rel.tar.gz
    cp python3_rel/${python3_file} ${OUTPUT_PKG_PATH}/install

    echo "======================Copy python3 plugin package successfully========================="
}

copy_file() {
    # 1、copy plugin_attribute_1.2.1.json
    if [ ! -f ${DATABASE_PATH}/conf/plugin_attribute_*.json ]; then
        echo "The plugin attribute file cannot be found."
        exit 1
    else
        cp -rf ${DATABASE_PATH}/conf/plugin_*.json ${OUTPUT_PKG_PATH}/conf
    fi

    # 2、copy so file
    if [ ! -f ${DATABASE_PATH}/lib/libdatabase* ]; then
        echo "The generaldb plugin so file cannot be found."
        exit 1
    else
        cp -rf ${DATABASE_PATH}/lib/libdatabase* ${OUTPUT_PKG_PATH}/lib/service
    fi

    if [ "${SYS_NAME}" != "AIX" ]; then
        if ! ls ${MODULE_PATH}/third_open_src/libevent_rel/lib/libevent-* 1>/dev/null 2>&1; then
            echo "The libevent so file cannot be found."
            exit 1
        else
            cp -rf ${MODULE_PATH}/third_open_src/libevent_rel/lib/libevent-* ${OUTPUT_PKG_PATH}/lib
        fi

        if ! ls ${MODULE_PATH}/third_open_src/openssl_rel/lib/libssl.so* 1>/dev/null 2>&1; then
            echo "The libssl so file cannot be found."
            exit 1
        else
            cp -rf ${MODULE_PATH}/third_open_src/openssl_rel/lib/libssl.so* ${OUTPUT_PKG_PATH}/lib/3rd
        fi

        if ! ls ${MODULE_PATH}/third_open_src/curl_rel/lib/libcurl.so* 1>/dev/null 2>&1; then
            echo "The libcurl so file cannot be found."
            exit 1
        else
            cp -rf ${MODULE_PATH}/third_open_src/curl_rel/lib/libcurl.so* ${OUTPUT_PKG_PATH}/lib/3rd
        fi

        if ! ls ${MODULE_PATH}/third_open_src/openssl_rel/lib/libcrypto.so* 1>/dev/null 2>&1; then
            echo "The libcrypto so file cannot be found."
            exit 1
        else
            cp -rf ${MODULE_PATH}/third_open_src/openssl_rel/lib/libcrypto.so* ${OUTPUT_PKG_PATH}/lib/3rd
        fi

        if ! ls ${MODULE_PATH}/platform/KMCv3_infra_rel/lib/libkmcv3.so 1>/dev/null 2>&1; then
            echo "The libkmcv3 so file cannot be found."
            exit 1
        else
            cp -rf ${MODULE_PATH}/platform/KMCv3_infra_rel/lib/libkmcv3.so ${OUTPUT_PKG_PATH}/lib
        fi

        if ! ls ${MODULE_PATH}/third_open_src/c-ares_rel/lib/libcares.so* 1>/dev/null 2>&1; then
            echo "The libcares so file cannot be found."
            exit 1
        else
            cp -rf ${MODULE_PATH}/third_open_src/c-ares_rel/lib/libcares.so* ${OUTPUT_PKG_PATH}/lib/3rd
        fi

        if ! ls ${MODULE_PATH}/third_open_src/libssh2_rel/lib/libssh2.so* 1>/dev/null 2>&1; then
            echo "The libssh2 so file cannot be found."
            exit 1
        else
            cp -rf ${MODULE_PATH}/third_open_src/libssh2_rel/lib/libssh2.so* ${OUTPUT_PKG_PATH}/lib/3rd
        fi
    fi

    # 3、copy other conf file
    cp -rf ${DATABASE_PATH}/conf/*.conf ${OUTPUT_PKG_PATH}/conf

    # 4、copy hcpconf.ini
    if [ ! -f ${DATABASE_PATH}/conf/hcpconf.ini ]; then
        echo "The hcpconf.ini file cannot be found."
    else
        cp -rf ${DATABASE_PATH}/conf/hcpconf.ini ${OUTPUT_PKG_PATH}/conf
    fi

    # 5、copy install.sh
    if [ ! -f ${DATABASE_PATH}/install/install.sh ]; then
        echo "The install file cannot be found."
        exit 1
    else
        cp -rf ${DATABASE_PATH}/install/install.sh ${OUTPUT_PKG_PATH}/install
    fi

    # 6、copy start.sh
    if [ ! -f ${DATABASE_PATH}/install/start.sh ]; then
        echo "The start file cannot be found."
        exit 1
    else
        cp -rf ${DATABASE_PATH}/install/start.sh ${OUTPUT_PKG_PATH}/install
    fi

    # 7、copy python_env.sh
    if [ ! -f ${DATABASE_PATH}/build/python/python_env.sh ]; then
        echo "The python_env file cannot be found."
        exit 1
    else
        cp -rf ${DATABASE_PATH}/build/python/python_env.sh ${OUTPUT_PKG_PATH}/install
    fi

    # 8、copy rpc tool file
    if [ ! -f ${DATABASE_PATH}/bin/dbrpctool ]; then
        echo "The dbrpctool cannot be found."
        exit 1
    else
        cp -rf ${DATABASE_PATH}/bin/dbrpctool ${OUTPUT_PKG_PATH}/bin
    fi
    if [ ! -f ${DATABASE_PATH}/src//tools/script/rpctool.sh ]; then
        echo "The dbrpctool script cannot be found."
        exit 1
    else
        cp -rf ${DATABASE_PATH}/src//tools/script/rpctool.sh ${OUTPUT_PKG_PATH}/bin
    fi

    if [ "${SYS_NAME}" != "AIX" ]; then
        # 9、copy internal agent python requirements.txt
        if [ ! -f ${DATABASE_PATH}/install/requirements.txt ]; then
            echo "The requirements.txt file cannot be found."
            exit 1
        else
            cp -rf ${DATABASE_PATH}/install/requirements.txt ${OUTPUT_PKG_PATH}/install
        fi
    fi
}

function download_fscanner_pkg() {
    if [ "${SYS_ARCH}" == "aarch64" ]; then
        ARCH="aarch64"
    elif grep -q "release 6" /etc/redhat-release; then
        ARCH="x86_64_centos6"
    else
        ARCH="x86_64_centos7"
    fi

    local ext_pkg_path=${PLUGINS_PATH}/opensrc_temp/FS_SCANNER
    [ -d ${ext_pkg_path} ] && rm -rf ${ext_pkg_path}
    mkdir -p ${ext_pkg_path}

    local opensrc_pkg_name=SCANNER_rel.tar.gz
    SYS_NAME_TMEP="Linux"
    if [ "${SYS_NAME}" = "AIX" ]; then
        SYS_NAME_TMEP="AIX"
        opensrc_pkg_name="SCANNER_rel.tar.xz"
    fi
    cp ${FS_SCANNER_PKG_PATH}/${SYS_NAME_TMEP}/${ARCH}/${opensrc_pkg_name} ${ext_pkg_path}
    if [ $? -ne 0 ]; then
        echo "down SCANNER pkgs from cmc error"
        return 1
    fi
    echo "======================Copy scanner package successfully========================="
    cd ${ext_pkg_path}
    tar -xf ${opensrc_pkg_name}
    echo "Finish to down SCANNER pkgs into cmc"
    return 0
}

download_and_pack_scanner() {
    download_fscanner_pkg
    if [ $? -ne 0 ]; then
        return 1
    fi
    if [ "${SYS_NAME}" = "AIX" ]; then
        local pkg_name=SCANNER_rel.tar.xz
        local pkg_name_tar=SCANNER_rel.tar
        local ext_pkg_path=${PLUGINS_PATH}/opensrc_temp/FS_SCANNER
        cd ${ext_pkg_path}
        if [ ! -f "${pkg_name}" ]; then
            echo "${pkg_name} not exist, pls check."
            exit 1
        fi

        xz -d ${pkg_name}
        if [ $? -ne 0 ]; then
            echo "Uncompress ${pkg_name} failed, pls check."
            exit 1
        fi

        mkdir -p ${ext_pkg_path}/SCANNER_TMP
        cp ${pkg_name_tar} ${ext_pkg_path}/SCANNER_TMP
        cd ${ext_pkg_path}/SCANNER_TMP
        tar -xf ${pkg_name_tar}
        if [ $? -ne 0 ]; then
            echo "Uncompress ${pkg_name_tar} failed, pls check."
            exit 1
        fi
        # 拷贝libScanner.a
        if [ ! -f ${ext_pkg_path}/SCANNER_TMP/lib/libScanner.a ]; then
            echo "The libScanner.so file cannot be found."
            exit 1
        else
            cp -rf ${ext_pkg_path}/SCANNER_TMP/lib/libScanner.a ${OUTPUT_PKG_PATH}/lib
        fi
        # 拷贝依赖
        cp -rf ${ext_pkg_path}/SCANNER_TMP/Module/* ${OUTPUT_PKG_PATH}/lib

        return 0
    fi

    #下载FS_Scanner软件包
    download_fscanner_pkg
    if [ $? -ne 0 ]; then
        return 1
    fi
    local ext_pkg_path=${PLUGINS_PATH}/opensrc_temp/FS_SCANNER
    if [ ! -f ${ext_pkg_path}/lib/libScanner.so ]; then
        echo "The libScanner.so file cannot be found."
        exit 1
    else
        cp -rf ${ext_pkg_path}/lib/libScanner.so ${OUTPUT_PKG_PATH}/lib
    fi
}

function download_backup_pkg() {
    if [ "${SYS_ARCH}" == "aarch64" ]; then
        ARCH="aarch64"
    elif grep -q "release 6" /etc/redhat-release; then
        ARCH="x86_64_centos6"
    else
        ARCH="x86_64_centos7"
    fi

    local ext_pkg_path=${PLUGINS_PATH}/opensrc_temp/FS_BACKUP
    [ -d ${ext_pkg_path} ] && rm -rf ${ext_pkg_path}
    mkdir -p ${ext_pkg_path}

    local opensrc_pkg_name=BACKUP_rel.tar.gz
    cp ${FS_BACKUP_PKG_PATH}/Linux/${ARCH}/${opensrc_pkg_name} ${ext_pkg_path}
    if [ $? -ne 0 ]; then
        echo "Upload BACKUP pkgs from cmc error"
        return 1
    fi
    cd ${ext_pkg_path}
    tar -xf ${opensrc_pkg_name}
    echo "Finish to down BACKUP pkgs into cmc"
    return 0
}

download_and_pack_backup() {
    if [ "${SYS_NAME}" = "AIX" ]; then
        return 0
    fi

    #下载BACKUP_rel.tar.gz
    download_backup_pkg
    if [ $? -ne 0 ]; then
        return 1
    fi
    local ext_pkg_path=${PLUGINS_PATH}/opensrc_temp/FS_BACKUP
    if [ ! -f ${ext_pkg_path}/lib/libBackup.so ]; then
        echo "The libBackup.so file cannot be found."
        exit 1
    else
        cp -rf ${ext_pkg_path}/lib/libBackup.so ${OUTPUT_PKG_PATH}/lib
    fi
}

execute_app_build() {
    mkdir -p ${OUTPUT_PKG_PATH}/bin/applications
    mkdir -p ${OUTPUT_PKG_PATH}/conf
    sh ${APPLICATIONS_BUILD_PATH}/build.sh ${OUTPUT_PKG_PATH}/bin/applications ${OUTPUT_PKG_PATH}/conf
    if [ $? -ne 0 ]; then
        echo "Failed to execute the application build script."
        exit 1
    fi
}

copy_app_binary() {
    mkdir -p ${PLUGINS_PATH}/opensrc_temp
    tar -xvf $APP_GENERALDB_PKG_PATH/database_x86_64.tar.gz -C ${PLUGINS_PATH}/opensrc_temp
    local apps_path="${PLUGINS_PATH}/opensrc_temp/applications"
    local script_dest_path="${OUTPUT_PKG_PATH}/bin/applications"
    cp -rf $apps_path/* $script_dest_path
    rm -rf $script_dest_path/conf
    cp -rf $apps_path/conf/*  ${OUTPUT_PKG_PATH}/conf
    cp -rf ${PLUGINS_PATH}/opensrc_temp/backint ${OUTPUT_PKG_PATH}/bin
    chmod -R 550 "$script_dest_path"
    rm -rf ${PLUGINS_PATH}/opensrc_temp
}

execute_pack_script() {
    sh ${FRAMEWORK_PATH}/build/pack.sh
    if [ $? -ne 0 ]; then
        echo "Failed to execute framework pack script."
        exit 1
    fi
}

download_xtrabackup_script() {
    if [ "${SYS_NAME}" = "AIX" ]; then
        return 0
    fi

    SYSTEM_NAME="CentOS7.6"
    if [ "${SYS_ARCH}" == "aarch64" ]; then
        OS_TYPE="ARM"
    else
        OS_TYPE="X86"
    fi
    local ext_pkg_path=${PLUGINS_PATH}/opensrc_temp/xtrabackup
    [ -d ${ext_pkg_path} ] && rm -rf ${ext_pkg_path}
    mkdir -p ${ext_pkg_path}

    COMPOENT_VERSION="1.2.1RC1"
    OPEN_SOURCE_TYPE="third_party_groupware"
    local pkg_name="OceanProtect_X8000_${COMPOENT_VERSION}_Opensrc_3rd_SDK_${SYSTEM_NAME}_${OS_TYPE}.tar.gz"
    cp ${OPEN_OBLIGATION_TRDPARTY_PATH}/${SYSTEM_NAME}/${OS_TYPE}/${OPEN_SOURCE_TYPE}/${pkg_name} ${ext_pkg_path}
    if [ $? -ne 0 ]; then
        echo "Copy ${pkg_name} failed, pls check"
        exit 1
    fi
    cd ${ext_pkg_path}
    if [ ! -f "${pkg_name}" ]; then
        echo "${pkg_name} not exist, pls check"
        exit 1
    fi

    mkdir -p ${ext_pkg_path}/3rd
    tar -zxvf ${pkg_name} -C ${ext_pkg_path}/3rd
    if [ $? -ne 0 ]; then
        echo "Uncompress ${pkg_name} failed, pls check"
        exit 1
    fi

    local agent_open_src_list="xtrabackup"
    for rel_pkg_name in $(ls -1 ${ext_pkg_path}/3rd/agent/); do
        if [ $(echo "${rel_pkg_name}" | grep -Ec "${agent_open_src_list}") -eq 0 ]; then
            continue
        fi
        local pkg_full_path=${ext_pkg_path}/3rd/agent/${rel_pkg_name}
        if [ ! -z "${rel_pkg_name}" -a -f "${pkg_full_path}" ]; then
            tar -zxvf "${pkg_full_path}" -C ${ext_pkg_path}/3rd/agent
        fi
    done

    if [ ! -f ${ext_pkg_path}/3rd/agent/xtrabackup2.4.25_agent_rel/bin/libatomic.so* ]; then
        echo "The libatomic so file cannot be found."
        exit 1
    else
        cp -f ${ext_pkg_path}/3rd/agent/xtrabackup2.4.25_agent_rel/bin/libatomic.so* ${OUTPUT_PKG_PATH}/lib/3rd
    fi

    if [ ! -f ${ext_pkg_path}/3rd/agent/xtrabackup2.4.25_agent_rel/bin/xtrabackup ]; then
        echo "The xtrabackup2.4.25 file cannot be found."
        exit 1
    else
        cp -f ${ext_pkg_path}/3rd/agent/xtrabackup2.4.25_agent_rel/bin/xtrabackup ${OUTPUT_PKG_PATH}/bin/xtrabackup2
    fi

    if [ ! -f ${ext_pkg_path}/3rd/agent/xtrabackup_agent_rel/bin/xtrabackup ]; then
        echo "The xtrabackup8 file cannot be found."
        exit 1
    else
        cp -f ${ext_pkg_path}/3rd/agent/xtrabackup_agent_rel/bin/xtrabackup ${OUTPUT_PKG_PATH}/bin/xtrabackup8
    fi

    if [ ! -f ${ext_pkg_path}/3rd/agent/xtrabackup_agent_rel/lib/libgcrypt.so.11 ]; then
        echo "The libgcrypt.so.11 cannot be found."
        exit 1
    else
        cp -Pf ${ext_pkg_path}/3rd/agent/xtrabackup_agent_rel/lib/libgcrypt.so.11* ${OUTPUT_PKG_PATH}/lib/3rd/
    fi
}

download_suse11_boost() {
    if [ "${SYS_ARCH}" = "aarch64" ] || [ "${SYS_NAME}" = "AIX" ]; then
        return 0
    fi

    SYSTEM_NAME="Suse11"
    OS_TYPE="X86"
    COMPOENT_VERSION=1.2.1RC1
    OPEN_SOURCE_TYPE="third_party_groupware"
    local ext_pkg_path=${PLUGINS_PATH}/opensrc_temp/boost/
    [ -d ${ext_pkg_path} ] && rm -rf ${ext_pkg_path}
    mkdir -p ${ext_pkg_path}
    local pkg_name=OceanProtect_X8000_${COMPOENT_VERSION}_Opensrc_3rd_SDK_${SYSTEM_NAME}_${OS_TYPE}.tar.gz
    cp ${OPEN_OBLIGATION_TRDPARTY_PATH}/${SYSTEM_NAME}/${OS_TYPE}/${OPEN_SOURCE_TYPE}/${pkg_name} ${ext_pkg_path}
    if [ $? -ne 0 ]; then
        echo "Copy ${pkg_name} failed, pls check"
        exit 1
    fi
    cd ${ext_pkg_path}
    mkdir -p ${ext_pkg_path}/3rd
    tar -zxvf ${pkg_name} -C ${ext_pkg_path}/3rd
    if [ $? -ne 0 ]; then
        echo "Uncompress ${pkg_name} failed, pls check"
        exit 1
    fi

    if [ ! -f ${ext_pkg_path}/3rd/agent/boost_agent_rel.tar.gz ]; then
        echo "The boost_agent_rel.tar.gz file cannot be found."
        exit 1
    else
        cp -f ${ext_pkg_path}/3rd/agent/boost_agent_rel.tar.gz ${OUTPUT_PKG_PATH}/lib
    fi
    echo "======================download boost package from cmc successfully========================="
}

main() {
    echo "#######################################################################################################"
    echo "Start pack GeneralDbPlugin."
    echo "#######################################################################################################"

    execute_build_script

    create_dir

    download_python3_pluginFrame

    download_xtrabackup_script

    download_suse11_boost

    copy_file

    download_and_pack_module

    download_and_pack_scanner

    download_and_pack_backup

    execute_app_build

    copy_app_binary

    execute_pack_script

    echo "#######################################################################################################"
    echo "GeneralDbPlugin pack success."
    echo "#######################################################################################################"
}

main "$@"
