#!/bin/sh
/# 
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
# Set caps for plugin

source "/etc/profile"

VIRT_PLUGIN_PATH="$(
    cd "$(dirname "$BASH_SOURCE")/../"
    pwd
)"

AGENT_INSTALL_PATH=${DATA_BACKUP_AGENT_HOME}
 
if [ -z "${AGENT_INSTALL_PATH}" ]; then
    AGENT_INSTALL_PATH="/opt"
fi

if [ ! -d "${AGENT_INSTALL_PATH}" ]; then
    echo "ERROR" "Agent Install Path [${AGENT_INSTALL_PATH}] : No such file or directory."
    return 1
fi

AGENT_ROOT_PATH="${AGENT_INSTALL_PATH}/DataBackup/ProtectClient"
AGENT_BIN_PATH="${AGENT_INSTALL_PATH}/DataBackup/ProtectClient/ProtectClient-E/bin"

function if_arm_arch()
{
    ARM_ARCH="aarch"
    cpu_arch=`uname -m`
    echo $cpu_arch | grep $ARM_ARCH >> /dev/null
    if [[ $? == 0 ]]
    then
        g_cpu_arch="aarch"
    fi
}
function if_x86_arch()
{
    ARM_ARCH="x86"
    cpu_arch=`uname -m`
    echo $cpu_arch | grep $ARM_ARCH >> /dev/null
    if [[ $? == 0 ]]
    then
        g_cpu_arch="x86"
    fi
}
function get_cpu_arch()
{
   if_arm_arch
   if_x86_arch
   echo $g_cpu_arch
}

function change_permission()
{
    # 1. set dir conf permission
    chmod 640 "${VIRT_PLUGIN_PATH}/conf/app_lib.json"
    chmod 640 "${VIRT_PLUGIN_PATH}/conf/hcpconf.ini"
    chmod 440 "${VIRT_PLUGIN_PATH}/conf/plugin_attribute_1.0.0.json"

    # 2. set dir vbstool permission
    chmod 750 "${VIRT_PLUGIN_PATH}/vbstool"
    chmod -R 640 "${VIRT_PLUGIN_PATH}/vbstool/conf"
    chmod -R 640 "${VIRT_PLUGIN_PATH}/vbstool/lib"
}

function import_cpp_lib()
{
    # copy libstdc++.so.6
    COMMON_LIBSTDCPP="libstdc++.so.6"
    LIB_STD_CPP_SO=`ls -al ${AGENT_BIN_PATH} | grep libstdc++.so.6 | awk -F "->" '{print $2}' | sed 's/ //g'`

    # create tempory dir
    mkdir -p "${AGENT_ROOT_PATH}"/LibStdCpp
    cp -rf "${LIB_STD_CPP_SO}" "${AGENT_ROOT_PATH}"/LibStdCpp

    LIB_STD_CPP_SO=`find "${AGENT_ROOT_PATH}"/LibStdCpp -name "libstdc++.so.6.*"`
    if [ -f "${LIB_STD_CPP_SO}" ]; then
        ln -sf "${LIB_STD_CPP_SO}" "${AGENT_ROOT_PATH}/LibStdCpp/${COMMON_LIBSTDCPP}"
    else
        echo "ERROR: not found libstdc++.so.6. plugin[VirtualizationPlugin] install failed."
        exit 1
    fi

    # UOS
    LIB_SECURE_SO=`find "${AGENT_BIN_PATH}" -name "libsecurec.so"`
    if [ -f "${LIB_SECURE_SO}" ]; then
        cp -f "${LIB_SECURE_SO}" "${AGENT_ROOT_PATH}"/LibStdCpp
    elif [[ ("`cat /etc/os-release | grep -i "uos"`" != "")  && (`get_cpu_arch` == "aarch") ]]; then
        echo "ERROR: not found libsecurec.so. plugin[VirtualizationPlugin] for UOS install failed."
        exit 1
    fi

    chown -R root:rdadmin "${AGENT_ROOT_PATH}"/LibStdCpp
    chmod -R 550 "${AGENT_ROOT_PATH}"/LibStdCpp
    LD_LIBRARY_PATH=${AGENT_ROOT_PATH}/LibStdCpp:$LD_LIBRARY_PATH && export LD_LIBRARY_PATH
}

function set_cap()
{
    # virtual plugin rdadmin need capablity
    local caps_list=$1
    local pluginPath=$2

    if [ -f "${pluginPath}" ] && [ ! -L "${pluginPath}" ]; then
        setcap "${caps_list}" "${pluginPath}"
        return 0
    else
        echo "ERROR" "Set cap for [${pluginPath}] failed. start plugin failed."
        return 1
    fi
}

function patch_target_file()
{
    local targetLibPath=$1
    local retry_time=0
    local max_retry=5
    local wait_time=3
    while [ "$retry_time" -lt "$max_retry" ]
    do
        needed_names=`ldd ${targetLibPath} | grep -v "libgcc_s.so.1" | awk -F "=>" '{print $1}'`
        if [ $? -ne 0 ]; then
            retry_time=$((retry_time+1))
            echo "Command ldd failed $retry_time time, retry."
            sleep $wait_time
        else
            break
        fi
    done
    if [ "$retry_time" -eq "$max_retry" ]; then
        echo "Ldd failed, max retry time. "
        exit 1
    fi
    for needed_name in ${needed_names}
    do
        local neededLibPath=$(find ${VIRT_PLUGIN_PATH} ${AGENT_BIN_PATH} -name ${needed_name} 2>/dev/null | head -n 1)
        if [ ! -z ${neededLibPath} ]; then
            # set lib path
            "${VIRT_PLUGIN_PATH}"/bin/patchelf --replace-needed ${needed_name} ${neededLibPath} ${targetLibPath}
            patch_target_file ${neededLibPath}
        fi
    done
}

function link_python()
{
    local backup_scene=`cat ${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient/ProtectClient-E/conf/testcfg.tmp | grep "BACKUP_SCENE" | cut -d'=' -f2`
    if [ $backup_scene == "1" ]; then
        echo "Internal agent dont link libpython!"
        return 0
    fi
    LIB_PYTHON=`ldd ${VIRT_PLUGIN_PATH}/lib/service/libvirtualization_plugin.so | grep "libpython" | awk -F "=>" '{print $1}' | awk -F " " '{print $1}'`
    PYTHON_PATH="${VIRT_PLUGIN_PATH}/install/build_python"
    if [ ! -f "${VIRT_PLUGIN_PATH}/lib/${LIB_PYTHON}" ]; then
        echo "Link python3.10.so to ${LIB_PYTHON}!"
        ln -s "${PYTHON_PATH}/lib/libpython3.10.so.1.0" "${VIRT_PLUGIN_PATH}/lib/libpython3.9.so.1.0"
    fi
}
 

main()
{
    # 1. change permission
    change_permission

    # 2. set tempory libstdc++so.6
    import_cpp_lib

    link_python

    # 3. patchelf dependent libs
    echo "INFO" "Begin patchelf deps libs for virtualization plugin[rdadmin]. Please wait."
    patch_target_file "${VIRT_PLUGIN_PATH}/bin/VirtualizationPlugin"
    patch_target_file "${VIRT_PLUGIN_PATH}/lib/service/libvirtualization_plugin.so"
    local backup_scene=`cat ${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient/ProtectClient-E/conf/testcfg.tmp | grep "BACKUP_SCENE" | cut -d'=' -f2`
    if [ $backup_scene != "1" ]; then
        patch_target_file "${VIRT_PLUGIN_PATH}/install/build_python/bin/python3.10"
    fi

    # 4. set capability for read/write block device.
    local capLists="CAP_IPC_OWNER+eip CAP_DAC_OVERRIDE+eip"
    set_cap "${capLists}" "${VIRT_PLUGIN_PATH}/bin/VirtualizationPlugin"
    set_cap "${capLists}" "${VIRT_PLUGIN_PATH}/lib/service/libvirtualization_plugin.so"
    set_cap "${capLists}" "${VIRT_PLUGIN_PATH}/lib/service/libkubernetes_engine.so"

    # 5. clean
    if [ -d "${AGENT_ROOT_PATH}"/LibStdCpp ]; then
        rm -rf "${AGENT_ROOT_PATH}"/LibStdCpp
    fi
}

main

