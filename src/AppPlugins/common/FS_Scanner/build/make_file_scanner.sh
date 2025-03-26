#!/bin/sh

SCRIPT_PATH=$(cd $(dirname $0); pwd)
BUILD_ROOT=$(cd "${SCRIPT_PATH}/.."; pwd)
SCANNER_MODULE_DIR="${BUILD_ROOT}/Module"
COMMON_PATH=${SCRIPT_PATH}/common
source ${COMMON_PATH}/common.sh
source ${COMMON_PATH}/branch.sh

#################### 获 取 入 参 ##########################
# 设置默认值
MODULE_PATH=${SCANNER_MODULE_DIR}
TYPE=
NAS="OFF"

for option
do
    case $option in

    -path=* | --path=*)
      MODULE_PATH=`expr "x$option" : "x-*path=\(.*\)"`
      ;;

    -type=* | --type=*)
      TYPE=`expr "x$option" : "x-*type=\(.*\)"`
      ;;

    -NAS=* | --NAS=*)
      NAS=`expr "x$option" : "x-*NAS=\(.*\)"`
      ;;

    esac
done
#################### 获 取 入 参 ##########################

download_module_opensrc() {
    sh ${BUILD_ROOT}/build/download_module_from_cmc.sh ${MODULE_BRANCH}
    if [ $? -ne 0 ]; then
        log_echo "ERROR" "download module failed"
        exit 1
    fi
}

main() {
    chmod +x ${BUILD_ROOT}/build/build_file_scanner.sh
    if [ $MODULE_PATH = ${SCANNER_MODULE_DIR} ]; then
        download_module_opensrc
    fi

    sh ${BUILD_ROOT}/build/build_file_scanner.sh --path=${MODULE_PATH} --type=${TYPE} --NAS=${NAS}
}

main "$@"
exit $?