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
# Create python virtual env for Virtualization plugin.

SYS_ARCH=`uname -m`
SYS_NAME=`uname -s`
CURRENT_PATH=`pwd`
OPENSSL_PATH=${CURRENT_PATH}/build_openssl
PYTHON_PATH=${CURRENT_PATH}/build_python
PACKAGE_PATH=${CURRENT_PATH}/packages_required
INITIAL_PATH=${CURRENT_PATH}/initial_packages
PYTHON3=${PYTHON_PATH}/bin/python3.10
LOG_FILE=${CURRENT_PATH}/../../../ProtectClient-E/slog/Plugins/VirtualizationPlugin/python_env.log

log()
{
    message="$1"
    echo "[$(date "+%Y-%m-%d %H:%M:%S")][$(whoami)] ${message}"
}

virtualenvwrapper() {
    cd ${INITIAL_PATH}
    export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}
    export LIBPATH=${LD_LIBRARY_PATH}
    ${PYTHON3} -m pip install --no-index --find-links=./ -r initial.txt >> ${LOG_FILE} 2>&1
    if [ $? -ne 0 ];then
        log "Failed to install virtualenvwrapper !" >> ${LOG_FILE}
        exit 1
    fi
    chmod 755 ${CURRENT_PATH}/build_python/bin/virtualenvwrapper*.sh
    cd - >> ${LOG_FILE} 2>&1
}

create_env() {
    if [ -d ${CURRENT_PATH}/.virtualenvs ];then
        log "${CURRENT_PATH}/.virtualenvs exist, will rm." >> ${LOG_FILE}
        rm -rf ${CURRENT_PATH}/.virtualenvs
    fi
    mkdir ${CURRENT_PATH}/.virtualenvs

    export VIRTUALENVWRAPPER_PYTHON=${PYTHON_PATH}/bin/python3.10
    export VIRTUALENVWRAPPER_VIRTUALENV=${PYTHON_PATH}/bin/virtualenv
    export WORKON_HOME=${CURRENT_PATH}/.virtualenvs
    source ${PYTHON_PATH}/bin/virtualenvwrapper.sh >> ${LOG_FILE} 2>&1
    if [ $? -ne 0 ];then
        log "Failed to source ${PYTHON_PATH}/bin/virtualenvwrapper.sh !" >> ${LOG_FILE}
        exit 1
    fi

    mkvirtualenv virtual_plugin_env >> ${LOG_FILE} 2>&1
    if [ $? -ne 0 ];then
        log "Failed to mkvirtualenv !" >> ${LOG_FILE}
        exit 1
    fi
    log "mkvirtualenv success." >> ${LOG_FILE}

    cd ${PACKAGE_PATH}
    python -m pip install --no-index --find-links=./ -r requirements.txt >> ${LOG_FILE}  2>&1
    if [ $? -ne 0 ];then
        log "Failed to pip install requirements!" >> ${LOG_FILE}
        exit 1
    fi
    cd - >> ${LOG_FILE} 2>&1
    log "Create plugin virtualenv success." >> ${LOG_FILE}
}

main() {
    log "Start create plugin python virtual env" >> ${LOG_FILE}
    export LD_LIBRARY_PATH=${PYTHON_PATH}/lib:${CURRENT_PATH}:${LD_LIBRARY_PATH}
    # 安装环境前置
    virtualenvwrapper
    # 创建虚拟环境
    create_env
    # 删除三方包目录
    rm -rf ${CURRENT_PATH}/packages_required
    rm -rf  ${CURRENT_PATH}/initial_packages
}

main