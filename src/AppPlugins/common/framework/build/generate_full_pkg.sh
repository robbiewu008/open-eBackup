#!/bin/bash
umask 0022
# entry of CI pipeline
CURRENT_SCRIPT="${BASH_SOURCE[0]}"
SCRIPT_PATH=$(cd $(dirname ${CURRENT_SCRIPT});pwd)
LCRP_CONFIG_PATH=${SCRIPT_PATH}/LCRP/conf
source ${SCRIPT_PATH}/common/common.sh
SCRIPT_NAME="${BASH_SOURCE[0]##*/}"
EXT_PKG_DOWNLOAD_PATH=${PLUGIN_ROOT_DIR}/ext_pkg

function uncompress_pkg()
{
    rm -rf ${PLUGIN_PACKAGE_PATH}
    mkdir -p ${PLUGIN_PACKAGE_PATH}
    cd ${PLUGIN_PACKAGE_PATH}
    for pkg in $(ls -1 ${EXT_PKG_DOWNLOAD_PATH}/Plugins/${PLUGIN_NAME}_*.tar.xz)
    do
        local pkg_name=$(echo ${pkg} | awk -F '/' '{print $NF}' | awk -F '.' '{print $1}')
        if [ -z ${pkg_name} ];then
            log_echo "pkg[${pkg}] not exist"
            continue
        fi
        mkdir -p ${pkg_name}
        tar -xvf ${pkg} -C ${pkg_name}
        if [ $? -ne 0 ];then
            log_echo "pkg[${pkg}] format error"
            continue
        fi
        local local_type=$(echo "${pkg_name}" | sed "s/${PLUGIN_NAME}_//")
        local inner_pkg=$(ls -1 ${pkg_name}/*.tar.xz | grep "${PLUGIN_NAME}" | tail -1)
        if [ -z "${inner_pkg}" ];then
            log_echo "pkg[${pkg_name}] not exist"
            continue
        fi
        mv -f ${inner_pkg}  "${pkg_name}/${PLUGIN_NAME}_$(uname -s)_${local_type}.tar.xz"
        cp -rf ${pkg_name}/*.tar.xz .
        cp -rf ${pkg_name}/*.sh .
        cp -rf ${pkg_name}/*.json .
        rm -rf ${pkg_name}
    done

    echo "Repacking plugins..."
    local upload_pkg_name="${PLUGIN_NAME}.tar"
    echo "tar -cvf ${upload_pkg_name} *.tar.xz *.sh *.json"
    tar -cvf ${upload_pkg_name} *.tar.xz *.sh *.json
    echo "xz -v ${upload_pkg_name}"
    xz -v ${upload_pkg_name}
    rm -rf ${PLUGIN_NAME}_*.tar.xz *.sh *.json
}

function main()
{
    PLUGIN_NAME="$1"
    if [ -z "${PLUGIN_NAME}" ];then
        log_echo "Please input plugin name."
        exit 1
    fi
    log_echo "Input plugin name is ${PLUGIN_NAME}."
    rm -rf ${EXT_PKG_DOWNLOAD_PATH}/
    log_echo "Begin pack plugin"
    mkdir -p ${EXT_PKG_DOWNLOAD_PATH}/Plugins
    cp -rf ${PLUGIN_ROOT_DIR}/output_pkg/${PLUGIN_NAME}*.tar.xz ${EXT_PKG_DOWNLOAD_PATH}/Plugins
    if [ $? -ne 0 ]; then
        log_echo "pack plugin error"
        continue
    fi
    uncompress_pkg
}

main "$@"
exit $?