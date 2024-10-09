#!/bin/bash
# Create python virtual env for DB plugin.

SYS_ARCH=`uname -m`
SYS_NAME=`uname -s`
CURRENT_PATH=`pwd`
OPENSSL_PATH=${CURRENT_PATH}/build_openssl
PYTHON_PATH=${CURRENT_PATH}/build_python
PACKAGE_PATH=${CURRENT_PATH}/packages_required
INITIAL_PATH=${CURRENT_PATH}/initial_packages
PYTHON3=${PYTHON_PATH}/bin/python3.10
LOG_FILE=${CURRENT_PATH}/../../../ProtectClient-E/slog/Plugins/GeneralDBPlugin/python_env.log

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
    if [ ${SYS_NAME} = "AIX" ]; then
        sed "s/.*virtualenvwrapper.*XX.*/file=\"virtualenvwrapper-\$suffix-XXXXXXXXXX\"/g" ${PYTHON_PATH}/bin/virtualenvwrapper.sh > ${PYTHON_PATH}/bin/virtualenvwrapper_bak.sh
        rm -rf ${PYTHON_PATH}/bin/virtualenvwrapper.sh
        mv ${PYTHON_PATH}/bin/virtualenvwrapper_bak.sh ${PYTHON_PATH}/bin/virtualenvwrapper.sh
    fi
    source ${PYTHON_PATH}/bin/virtualenvwrapper.sh >> ${LOG_FILE} 2>&1
    if [ $? -ne 0 ];then
        log "Failed to source ${PYTHON_PATH}/bin/virtualenvwrapper.sh !" >> ${LOG_FILE}
        exit 1
    fi

    mkvirtualenv plugin_env >> ${LOG_FILE} 2>&1
    if [ $? -ne 0 ];then
        log "Failed to mkvirtualenv !" >> ${LOG_FILE}
        exit 1
    fi
    log "mkvirtualenv success." >> ${LOG_FILE}

    cd ${PACKAGE_PATH}
    SITE_PACKAGES_PATH=${CURRENT_PATH}/.virtualenvs/plugin_env/lib/python3.10/site-packages
    if [ ${SYS_NAME} = "AIX" ]; then
        python -m pip install --target ${SITE_PACKAGES_PATH} --no-index --find-links=./ -r requirements-aix.txt >> ${LOG_FILE} 2>&1
    else
        python -m pip install --target ${SITE_PACKAGES_PATH} --no-index --find-links=./ -r requirements.txt >> ${LOG_FILE}  2>&1
    fi
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
    if [ ${SYS_NAME} = "AIX" ]; then
        export PYTHONHASHSEED=1
    fi
    # 安装环境前置
    virtualenvwrapper
    # 创建虚拟环境
    create_env
    # 删除三方包目录
    rm -rf ${CURRENT_PATH}/packages_required
    rm -rf  ${CURRENT_PATH}/initial_packages
}

main