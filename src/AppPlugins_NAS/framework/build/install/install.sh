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
# Install nas plugin for agent
CUR_PATH=`dirname $0`
SCRIPT_PATH=`cd ${CUR_PATH} && pwd`
COMMON_PATH=${SCRIPT_PATH}
. ${COMMON_PATH}/common.sh
SCRIPT_NAME=`basename $0`

if [ ! -d $DATA_BACKUP_AGENT_HOME ];then
    echo "Agent home dir do not exist"
    exit 1
fi
MOUNT_SCRIPT_PATH=${DATA_BACKUP_AGENT_HOME}/script/mount_oper.sh

PLUGIN_INSTALL_PATH=${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient/Plugins
AGENT_CONF_FILE=${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient/ProtectClient-E/conf/agent_cfg.xml

set_agent_backup_scene()
{
    backup_scene=`sed -n 's/.*backup_scene.*value="\(\S*\)"\/>.*$/\1/p' ${AGENT_CONF_FILE}`
    # Agent配置，默认1为内置，0是外置
    if [ "X${backup_scene}" = "X1" ];then
        sed -i "s/.*\"PluginUsageScene\":.*/    \"PluginUsageScene\": \"internal\",/g" ${APP_LIB_FILE}
    else
        sed_local_modify "s/.*\"PluginUsageScene\":.*/    \"PluginUsageScene\": \"external\",/g" ${APP_LIB_FILE}
    fi
    # 临时继续保留hcpconf.ini配置方式
    if [ "X${backup_scene}" = "X0" ];then
        sed_local_modify "s/AGENT_PLUGIN_USAGE_SCENE=.*/AGENT_PLUGIN_USAGE_SCENE=0/g" ${PLUGIN_CONF_FILE}
    fi
}

change_Permission()
{
    echo "[$(date "+%Y-%m-%d %H:%M:%S")][INFO][ Begin to modify plugin ${PLUGIN_NAME} permission ]" >> ${LOG_FILE}
    backup_scene=`sed -n 's/.*backup_scene.*value="\(\S*\)"\/>.*$/\1/p' ${AGENT_CONF_FILE}`
    cd ${PLUGIN_INSTALL_PATH}
    if [ "X${backup_scene}" = "X1" ]; then
        # 内置Agent配置
        chown -R root:nobody ${PLUGIN_NAME}
    else
        chown -R root:rdadmin ${PLUGIN_NAME}
    fi
    chmod 750 ${PLUGIN_NAME}
    cd ${PLUGIN_NAME}
    chmod -R 550 bin lib
    chmod 750 conf
    chmod 640 conf/hcpconf.ini
    chmod 440 conf/plugin_attribute_1.0.0.json
    chmod 440 plugin_attribute_1.0.0.json
    chmod 550 start.sh stop.sh install.sh uninstall.sh 
    echo "[$(date "+%Y-%m-%d %H:%M:%S")][INFO][ Finish to modify plugin permission. ]" >> ${LOG_FILE}
}

invoke_app_install()
{
    echo "[$(date "+%Y-%m-%d %H:%M:%S")][INFO][Enter invoke_app_install.]" >> ${LOG_FILE}
    appInstall=${PLUGIN_INSTALL_PATH}/${PLUGIN_NAME}/install
    if [ -f ${appInstall}/install.sh ]; then
        if [ "${OS_TYPE}" = "AIX" ]; then
            chmod +x ${appInstall}/install.sh
            ${appInstall}/install.sh
        else
            source ${appInstall}/install.sh "$@"
        fi
        ret=$?
        if [ ${ret} -ne 0 ]; then
            echo "[$(date "+%Y-%m-%d %H:%M:%S")][ERROR][ excute app install.sh failed. ret is ${ret} ]" >> ${LOG_FILE}
            exit ${ret}
        fi
    fi
    echo "[$(date "+%Y-%m-%d %H:%M:%S")][INFO][EXIT invoke_app_install.]" >> ${LOG_FILE}
    return 0
}

set_alias()
{
    if [ "${OS_TYPE}" = "AIX" ]; then
        echo "AIX use ksh, not bash!"
        return 0
    fi
    if [ "${OS_TYPE}" = "SunOS" ]; then
        return 0
    fi
    BASHRC_PATH=~/.bashrc
	if [ ! -f ${BASHRC_PATH} ]; then
		touch $BASHRC_PATH
	fi
	sed_local_modify "/^alias log=/d" $BASHRC_PATH
	sed_local_modify "/^alias slog=/d" $BASHRC_PATH
	sed_local_modify "/^alias bin=/d" $BASHRC_PATH
	echo "alias slog='cd ${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient/ProtectClient-E/slog/Plugins/${PLUGIN_NAME}'" >> $BASHRC_PATH
	echo "alias log='cd ${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient/ProtectClient-E/log'" >> $BASHRC_PATH
	echo "alias bin='cd ${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient/Plugins/${PLUGIN_NAME}/bin'" >> $BASHRC_PATH
	source $BASHRC_PATH
    return 0
}

unzip_cppframework()
{
    if [ "X${backup_scene}" = "X1" ]; then
        # 内置Agent配置暂时没有unzip_cppframework
        return 0
    fi

    if [ "${OS_TYPE}" = "AIX" ]; then
        FRAMEWORK_PKG_NAME_TAR="cppframework-$(uname -s)_ppc_$(getconf HARDWARE_BITMODE).tar"
        FRAMEWORK_PKG_NAME="cppframework-$(uname -s)_ppc_$(getconf HARDWARE_BITMODE).tar.xz"
    elif [ "${OS_TYPE}" = "SunOS" ]; then
        FRAMEWORK_PKG_NAME_TAR="cppframework-`uname -s`_`uname -m`.tar"
        FRAMEWORK_PKG_NAME="cppframework-`uname -s`_`uname -m`.tar.gz"
    else
        FRAMEWORK_PKG_NAME_TAR="cppframework-$(uname -s)_$(uname -m).tar"
        FRAMEWORK_PKG_NAME="cppframework-$(uname -s)_$(uname -m).tar.xz"
    fi
    if [ ! -f "${PLUGIN_INSTALL_PATH}/${FRAMEWORK_PKG_NAME}" ]; then
        return 0
    fi
    CPPFRAMEWORK_DIR="${PLUGIN_INSTALL_PATH}/${PLUGIN_NAME}/cppframework"
    mkdir -p ${CPPFRAMEWORK_DIR}

    if [ "${OS_TYPE}" = "AIX" ]; then
        cp "${PLUGIN_INSTALL_PATH}/${FRAMEWORK_PKG_NAME}" "${CPPFRAMEWORK_DIR}"
        xz -d "${CPPFRAMEWORK_DIR}/${FRAMEWORK_PKG_NAME}"
        cd "${CPPFRAMEWORK_DIR}"
        tar xf "${CPPFRAMEWORK_DIR}/${FRAMEWORK_PKG_NAME_TAR}"
        if [ $? -ne 0 ]; then
            echo "Decompression failed."
            cd -
            return 1
        fi
        cd -
        rm -rf "${PLUGIN_INSTALL_PATH}/${PLUGIN_NAME}/${FRAMEWORK_PKG_NAME_TAR}"
        find ${CPPFRAMEWORK_DIR} ! -path ${CPPFRAMEWORK_DIR} -prune -type f | xargs -I{} cp -f {} ${PLUGIN_INSTALL_PATH}/${PLUGIN_NAME}/lib
        if [ $? -ne 0 ]; then
            echo "Failed to copy the framework library."
            return 1
        fi
    elif [ "${OS_TYPE}" = "SunOS" ]; then
        curPath=`pwd`
        cd "${CPPFRAMEWORK_DIR}"
        cp "${PLUGIN_INSTALL_PATH}/${FRAMEWORK_PKG_NAME}" "${CPPFRAMEWORK_DIR}"
        gzip -d "${CPPFRAMEWORK_DIR}"/${FRAMEWORK_PKG_NAME}
        tar -xf "${CPPFRAMEWORK_DIR}"/${FRAMEWORK_PKG_NAME_TAR}
        if [ $? -ne 0 ]; then
            echo "Decompression failed."
            cd "${curPath}"
            return 1
        fi
        cp -f ${CPPFRAMEWORK_DIR}/lib*.so ${PLUGIN_INSTALL_PATH}/${PLUGIN_NAME}/lib
        if [ $? -ne 0 ]; then
            echo "Failed to copy the framework library."
            cd "${curPath}"
            return 1
        fi
        cd "${curPath}"
    else
        tar xf "${PLUGIN_INSTALL_PATH}/${FRAMEWORK_PKG_NAME}" -C "${CPPFRAMEWORK_DIR}"
        if [ $? -ne 0 ]; then
            echo "Decompression failed."
            return 1
        fi
        find ${CPPFRAMEWORK_DIR} -maxdepth 1 -type f | xargs -I{} cp -f {} ${PLUGIN_INSTALL_PATH}/${PLUGIN_NAME}/lib
        if [ $? -ne 0 ]; then
            echo "Failed to copy the framework library."
            return 1
        fi
    fi

    mkdir -p ${PLUGIN_INSTALL_PATH}/${PLUGIN_NAME}/lib/agent_sdk
    find ${CPPFRAMEWORK_DIR}/agent_sdk -type f | xargs -I{} cp -f {} ${PLUGIN_INSTALL_PATH}/${PLUGIN_NAME}/lib/agent_sdk

    # 拷贝so
    mkdir -p ${PLUGIN_INSTALL_PATH}/${PLUGIN_NAME}/lib/3rd
    find ${CPPFRAMEWORK_DIR}/3rd -type f | xargs -I{} cp -f {} ${PLUGIN_INSTALL_PATH}/${PLUGIN_NAME}/lib/3rd
    if [ "${OS_TYPE}" != "SunOS" ]; then
        # 记录链接信息，重新链接
        cd ${PLUGIN_INSTALL_PATH}/${PLUGIN_NAME}/lib/3rd
        LINK_INFO=`ls -l ${CPPFRAMEWORK_DIR}/3rd | grep ^l | ${AWK} 'NF{print $(NF-2)$(NF-1)$NF}'`
        for info in ${LINK_INFO}; do
            LINK_FILE_NAME=`echo ${info} | ${AWK} -F '->' '{print $1}'`
            SO_FILE_NAME=`echo ${info} | ${AWK} -F '->' '{print $2}'`
            rm -rf ${LINK_FILE_NAME}
            ln -s ${SO_FILE_NAME} ${LINK_FILE_NAME}
        done
    fi
    rm -rf ${CPPFRAMEWORK_DIR}
    return 0
}

check_os_adapt()
{
    # SUSE11 glibc版本较低，boost三方不兼容，需使用SUSE11编译的boost三方
    if [ -f /etc/SuSE-release ]; then
        local suseVersion=`cat /etc/SuSE-release | grep VERSION | awk -F ' ' '{print $3}'`
        if [ ${suseVersion} -eq 11 ] && [ ${PLUGIN_NAME} = "FilePlugin" ]; then
            /bin/cp ${PLUGIN_INSTALL_PATH}/${PLUGIN_NAME}/lib/3rd/suse/* /${PLUGIN_INSTALL_PATH}/${PLUGIN_NAME}/lib/3rd/
        fi
    fi
    rm -rf ${PLUGIN_INSTALL_PATH}/${PLUGIN_NAME}/lib/3rd/suse
}

main()
{
    if [ -z "$DATA_BACKUP_AGENT_HOME" ]; then
        echo "The environment variable: DATA_BACKUP_AGENT_HOME is empty."
        exit 1
    fi

    PLUGIN_NAME=`get_plugin_name`
    if [ $? -ne 0 ]; then
        exit 1
    fi
    # install.log 日志是root权限，输出在slog下
    LOG_FILE_PREFIX=${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient/ProtectClient-E/slog/Plugins/${PLUGIN_NAME}

    # 安装后首次建立日志目录
    mkdir -p ${LOG_FILE_PREFIX}
    LOG_FILE=${LOG_FILE_PREFIX}/install.log
    APP_LIB_FILE=${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient/Plugins/${PLUGIN_NAME}/conf/app_lib.json
    PLUGIN_CONF_FILE=${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient/Plugins/${PLUGIN_NAME}/conf/hcpconf.ini

    cd "${SCRIPT_PATH}"
    mkdir -p "${PLUGIN_INSTALL_PATH}/${PLUGIN_NAME}"
    if [ "${OS_TYPE}" = "AIX" ]; then
        cpu_type=ppc_`getconf HARDWARE_BITMODE`
    else
        cpu_type=`uname -m`
    fi
    if [ "${OS_TYPE}" = "SunOS" ]; then
        PKG_NAME="${PLUGIN_NAME}_`uname -s`_${cpu_type}.tar.gz"
    else
        PKG_NAME="${PLUGIN_NAME}_`uname -s`_${cpu_type}.tar.xz"
    fi
    PKG_NAME_TAR="${PLUGIN_NAME}_`uname -s`_${cpu_type}.tar"

    echo "[$(date "+%Y-%m-%d %H:%M:%S")][INFO] Unpacking plugin package. PLUGIN_NAME:${PLUGIN_NAME}, PKG_NAME:${PKG_NAME}" >> ${LOG_FILE}
    if [ "${OS_TYPE}" = "SunOS" ]; then
        gzip -d "${PKG_NAME}"
    else
        xz -d "${PKG_NAME}"
    fi
    tar -xf "${PKG_NAME_TAR}"
    rm -f "${PKG_NAME_TAR}"

    if [ $? -eq 0 ]; then
        unzip_cppframework
        if [ $? -ne 0 ];then
            echo "[$(date "+%Y-%m-%d %H:%M:%S")][INFO][ Failed to decompress the framework ]" >> ${LOG_FILE}
            return 1
        fi
        echo "[$(date "+%Y-%m-%d %H:%M:%S")][INFO][ Install agent plugin ${PLUGIN_NAME} successfully ]" >> ${LOG_FILE}
        change_Permission
        invoke_app_install
        set_agent_backup_scene
        set_alias
        check_os_adapt
        if [ $? -ne 0 ]; then
            return 1
        fi
        rm -rf ${PLUGIN_NAME}*.tar*
        return 0
    fi
    echo "[$(date "+%Y-%m-%d %H:%M:%S")][ERROR][ Failed to install agent plugin. ]" >> ${LOG_FILE}
    return 1
}

main "$@"
exit $?
