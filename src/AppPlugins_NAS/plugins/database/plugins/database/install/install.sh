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

SYS_ARCH=`uname -m`
SYS_NAME=`uname -s`
GENERALDB_PLUGIN_PATH=""
SCRIPT_NAME=$(basename $0)
AGENT_USER=rdadmin
SHELL_TYPE_SH="/bin/sh"
if [ "${SYS_NAME}" = "AIX" ]; then
    GENERALDB_PLUGIN_PATH="$(cd "$(dirname $0)/../" && pwd)"
else
    GENERALDB_PLUGIN_PATH="$(cd "$(dirname "$BASH_SOURCE")/../" && pwd)"
fi

if [ "${SYS_NAME}" = "SunOS" ]; then
    AWK=nawk
else
    AWK=awk
fi

if [ "`ls -al /bin/sh | ${MYAWK} '{print $NF}'`" = "bash" ]; then
    EXPORT_ENV="if [ -f ~/.profile ]; then source ~/.profile; fi;"
else
    EXPORT_ENV=""
fi
LOG_FILE=${GENERALDB_PLUGIN_PATH}/../../ProtectClient-E/slog/Plugins/GeneralDBPlugin/install.log
log_echo()
{
    type="DEBUG"
    message="$1"
    if [ "$#" -eq 2 ];then
       type="$1"
       message="$2"
    fi
    echo "[$(date "+%Y-%m-%d %H:%M:%S")][${type}][$(whoami)][${SCRIPT_NAME}] ${message}"
}

create_dir()
{
    # 1、create param file path
    if [ ! -d "${GENERALDB_PLUGIN_PATH}/tmp" ]; then
        mkdir -p "${GENERALDB_PLUGIN_PATH}/tmp"
    fi
    chmod 700 "${GENERALDB_PLUGIN_PATH}/tmp"

    # 2、create result file path
    if [ ! -d "${GENERALDB_PLUGIN_PATH}/stmp" ]; then
        mkdir -p "${GENERALDB_PLUGIN_PATH}/stmp"
    fi
    chmod 700 "${GENERALDB_PLUGIN_PATH}/stmp"

    # 3、create scan path
    if [ ! -d "${GENERALDB_PLUGIN_PATH}/scantmp" ]; then
        mkdir -p "${GENERALDB_PLUGIN_PATH}/scantmp"
    fi
    chmod 700 "${GENERALDB_PLUGIN_PATH}/scantmp"
}

chmod_dir()
{
    # 1、chmod dir
    chmod 505 "${GENERALDB_PLUGIN_PATH}/bin"
    chmod 505 "${GENERALDB_PLUGIN_PATH}/bin/applications"
    chmod 500 "${GENERALDB_PLUGIN_PATH}/conf"

    # 2、chown dir
    chown root:root "${GENERALDB_PLUGIN_PATH}/bin"
    chown root:root "${GENERALDB_PLUGIN_PATH}/bin/applications"
    chown root:root "${GENERALDB_PLUGIN_PATH}/conf"
}

# modify plugin main process start.sh
change_start_sh()
{
    cp -f ${GENERALDB_PLUGIN_PATH}/install/start.sh ${GENERALDB_PLUGIN_PATH}/start.sh
    rm -rf ${GENERALDB_PLUGIN_PATH}/install/start.sh
}

create_python_env()
{
    is_internal_agent
    if [ $? -eq 0 ]; then
        log_echo "INFO" "The current env is internal agent. Python does not need to be installed." >> ${LOG_FILE}
        return 0
    fi
    cd ${GENERALDB_PLUGIN_PATH}/install/
    python3_file=python3.pluginFrame.${SYS_ARCH}.tar.gz
    python3_aix_file=python3.pluginFrame.AIX.tar.gz
    if [ ${SYS_NAME} = "AIX" ]; then
        version=`oslevel | grep "^6.*"`
        if [ $? -eq 0 -o -n "${version}" ]; then
            log_echo "The current operating system version is AIX6."  >> ${LOG_FILE}
            rm -f python3.pluginFrame.*.tar.gz
            return 0
        fi
        gunzip -c ${python3_aix_file} | tar -xvf- >/dev/null
    else
        tar xf ${python3_file}
    fi
    rm -f python3.pluginFrame.*.tar.gz
    bash python_env.sh
    if [ $? -ne 0 ]; then
        echo "Failed to execute the python_env script !"
        exit 1
    fi
    cd -
}

install_suse11_boost()
{
    if [ "${SYS_ARCH}" = "aarch64" ] || [ "${SYS_NAME}" = "AIX" ]; then
        return 0
    fi

    #判断是不是suse11 x86
    if [ ! -f /etc/SuSE-release ];then
        if [ ! -f ${GENERALDB_PLUGIN_PATH}/lib/boost_agent_rel.tar.gz ];then
            rm -f ${GENERALDB_PLUGIN_PATH}/lib/boost_agent_rel.tar.gz
        fi
        return 0
    fi

    if [ "11" != `cat /etc/SuSE-release | grep VERSION | awk -F" " '{print $NF}'` ];then
        if [ ! -f ${GENERALDB_PLUGIN_PATH}/lib/boost_agent_rel.tar.gz ];then
            rm -f ${GENERALDB_PLUGIN_PATH}/lib/boost_agent_rel.tar.gz
        fi
        return 0
    fi

    cd ${GENERALDB_PLUGIN_PATH}/lib/
    tar -zxf boost_agent_rel.tar.gz
    if [ $? -ne 0 ]; then
        echo "Uncompress boost_agent_rel.tar.gz failed, pls check"
        exit 1
    fi

    cp -f boost_agent_rel/.libs/lib/*.so 3rd/
    if [ $? -ne 0 ]; then
        echo "cp boost so failed, pls check"
        exit 1
    fi

    rm -fr boost_agent_rel
    rm -f boost_agent_rel.tar.gz
}

SUExecCmd()
{
    cmd=$1
    result=""
    if [ "${SYS_NAME}" = "SunOS" ] || [ "${SYS_NAME}" = "AIX" ]; then
        result=`su - ${AGENT_USER} -c "${EXPORT_ENV}${cmd}"`
    else
        result=`su - ${AGENT_USER} -s ${SHELL_TYPE_SH} -c "${EXPORT_ENV}${cmd}"`
    fi

    echo $result
}

is_internal_agent()
{
    backup_scene=`SUExecCmd "${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient/ProtectClient-E/bin/xmlcfg read Backup backup_scene"`
    if [ "X${backup_scene}" = "X1" ];then
        return 0
    else
        return 1
    fi
}

# 内置agent安装python三方组件
install_python_components()
{
    cd ${GENERALDB_PLUGIN_PATH}/install
    is_internal_agent
    if [ $? -eq 0 ]; then
        mkdir packages
        pip download -d ./packages -r requirements.txt
        if [ $? -ne 0 ]; then
            echo "Download python components failed."
            exit 1
        fi
 
        mv requirements.txt ./packages
        mv psutil-5.9.0-cp39-cp39-linux_aarch64.whl ./packages
        cd ${GENERALDB_PLUGIN_PATH}/install/packages
        rm -rf psutil-5.9.0.tar.gz
        pip install --no-index --find-links=./ -r requirements.txt
        if [ $? -ne 0 ]; then
            echo "Install python components failed."
            exit 1
        fi
        rm -rf ${GENERALDB_PLUGIN_PATH}/install/packages
        return 0
    fi
}

main()
{
    create_dir
    chmod_dir
    change_start_sh
    create_python_env
    install_suse11_boost
    install_python_components
}

main $@