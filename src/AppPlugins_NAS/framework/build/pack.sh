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
umask 0022
# entry of CI pipeline
SCRIPT_PATH=$(cd $(dirname $0); pwd)
COMMON_PATH=${SCRIPT_PATH}/common
. ${COMMON_PATH}/common.sh
SCRIPT_NAME=$(basename $0)
PACK_SUFFIX=".tar.xz"

SYS_NAME=`uname -s`
if [ "${SYS_NAME}" = "SunOS" ]; then
    AWK=nawk
else
    AWK=awk
fi


strip_file()
{
    path=$1
    files=`ls $path`
    chmod 700 $1
    log_echo "INFO" "Begin to strip so under dir $path"
    if [ "${OS_TYPE}" = "AIX" ]; then
        libName="*.a"
    else
        libName=".*so.*"
    fi
    for filename in $files
    do
        if [ "${SYS_NAME}" = "SunOS" ]; then
            echo $filename | /usr/xpg4/bin/grep -e $libName
        else
            echo $filename | grep -e $libName
        fi
        if [ $? == 0 ]; then
            chmod 700 $path/$filename
            strip $path/$filename
        fi
    done
}

package_nas_plugin()
{
    if [ ! -f ${PLUGIN_PACKAGE_PATH}/conf/plugin_*.json ]; then
        log_echo "ERROR" "plugin json not exist"
        return 1
    fi
    cp -rf ${PLUGIN_PACKAGE_PATH}/conf/plugin_*.json ${PLUGIN_PACKAGE_PATH}
    if [ "${PLUGIN_NAME}X" == "X" ];then
        PLUGIN_NAME=$(cat ${PLUGIN_PACKAGE_PATH}/conf/plugin_*.json | grep name | ${AWK} -F \" '{print $4}')
        if [ "${PLUGIN_NAME}X" == "X" ]; then
            log_echo "ERROR" "plugin name is empty"
            return 1
        fi
    fi
    # 清理
    rm -rf ${PLUGIN_NAME}${PACK_SUFFIX}

    # 去除符号表
    if [[ "$1" = "Release" || "$1" = "release" || "${BUILD_TYPE}" = "release" || "${BUILD_TYPE}" = "Release" ]]; then
        log_echo "INFO" "This is a release package"
        strip_file ${PLUGIN_PACKAGE_PATH}/lib/3rd
        strip_file ${PLUGIN_PACKAGE_PATH}/lib
        strip_file ${PLUGIN_PACKAGE_PATH}/lib/service
        chmod 700 ${PLUGIN_PACKAGE_PATH}/bin/${PLUGIN_NAME}
        strip ${PLUGIN_PACKAGE_PATH}/bin/${PLUGIN_NAME}
    fi

    # 打应用包plugin.tar.xz
    typeset system_type=$(uname -s)
    if [ "${system_type}" = "AIX" ]; then
        typeset cpu_type=ppc_$(getconf HARDWARE_BITMODE)
    else
        typeset cpu_type=$(uname -m)
    fi
    cd ${PLUGIN_PACKAGE_PATH}/
    # 改变二进制文件为配置文件的插件名称
    mv -f bin/${EXEC_FILE_NAME} bin/${PLUGIN_NAME}

    typeset folder_list=$(ls -l .| grep "^d"  | ${AWK} '{print $NF}')

    if [ "${system_type}" = "SunOS" ]; then
        tar cvf ${PLUGIN_NAME}_${system_type}_${cpu_type}.tar ${folder_list}
        gzip ${PLUGIN_NAME}_${system_type}_${cpu_type}.tar
    else
        tar cvf ${PLUGIN_NAME}_${system_type}_${cpu_type}.tar ${folder_list}
        xz -v ${PLUGIN_NAME}_${system_type}_${cpu_type}.tar
        rm -rf ${folder_list}
    fi

    # 打插件包
    if [ "${system_type}" = "SunOS" ]; then
        tar -cvf ${PLUGIN_NAME}_${cpu_type}.tar ${PLUGIN_NAME}_${system_type}_${cpu_type}.tar.gz *.sh *.json
        gzip ${PLUGIN_NAME}_${cpu_type}.tar
    else
        tar cvf ${PLUGIN_NAME}_${cpu_type}.tar ${PLUGIN_NAME}_${system_type}_${cpu_type}.tar.xz *.sh *.json
        xz -v ${PLUGIN_NAME}_${cpu_type}.tar
    fi

    # delete plugin.tar
    rm -rf *.sh *.json ${PLUGIN_NAME}_${system_type}_${cpu_type}${PACK_SUFFIX}
    ls -l
}

# centos,编译环境和部署环境不一致，需要拷贝额外动态库
copy_ext_lib()
{
    if [ ! -f /etc/os-release ]; then
        log_echo "INFO" "os-release does not exist, no need to copy ext lib."
        return 0
    fi

    cat /etc/os-release | grep CentOS >/dev/null 2>&1
    if [ $? -ne 0 ]; then
        log_echo "INFO" "No need to copy ext lib"
        return 0
    fi
    mkdir -p ${PLUGIN_PACKAGE_PATH}/lib/ext
    # stdc++
    ldd ${PLUGIN_PACKAGE_PATH}/bin/${PLUGIN_NAME} | grep libstdc++ | ${AWK} '{print $3}' | xargs -I{} cp -f {} ${PLUGIN_PACKAGE_PATH}/lib/ext
}

main()
{
    copy_ext_lib
    package_nas_plugin "$@"
    exit $?
}

main "$@"
