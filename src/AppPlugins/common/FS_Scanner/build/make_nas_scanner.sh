#!/bin/bash

CURRENT_SCRIPT="${BASH_SOURCE[0]}"
SCRIPT_PATH=$(dirname ${CURRENT_SCRIPT})
BUILD_ROOT=$(cd "${SCRIPT_PATH}/.."; pwd)
SCANNER_MODULE_DIR="${BUILD_ROOT}/Module"
source ${SCRIPT_PATH}/common/common.sh
source ${COMMON_PATH}/branch.sh

function download_module_opensrc()
{
    sh ${BUILD_ROOT}/build/download_module_from_cmc.sh ${MODULE_BRANCH}
    if [ $? -ne 0 ]; then
        log_echo "ERROR" "download module failed"
        exit 1
    fi
}

function main() {
    chmod +x ${BUILD_ROOT}/build/build_nas_scanner.sh
    local MODULE_PATH=${1:-${SCANNER_MODULE_DIR}}
    local COMPILE_TYPE=${2:-''}
    if [ $MODULE_PATH = ${SCANNER_MODULE_DIR} ]; then
        download_module_opensrc
    fi

    if [ ${CENTOS} == "6" ]; then
        log_echo "centos6 does not support nas, change nas to file"
        sh ${BUILD_ROOT}/build/build_file_scanner.sh --path=${MODULE_PATH} --type=${COMPILE_TYPE}
    else
        sh ${BUILD_ROOT}/build/build_nas_scanner.sh ${MODULE_PATH} ${COMPILE_TYPE}
    fi
}

main "$@"
exit $?