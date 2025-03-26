#!/bin/bash
# build AppPlugins_NAS
SCRIPT_PATH=$(cd $(dirname $0); pwd)
SCRIPT_NAME=$(basename $0)
SCANNER_ROOT=$(cd "${SCRIPT_PATH}/.."; pwd)
LCRP_CONFIG_PATH=${SCRIPT_PATH}/LCRP/conf

# 产品名称
PRODUCT="dorado"
DEFAULT_COMPONENT_VERSION="1.1.0"

CODE_BRANCH=$1
if [ -z "${CODE_BRANCH}" ]; then
    CODE_BRANCH="${branch}"
fi

log_echo()
{
    local level="$1"
    local message="$2"
    echo "[$(date "+%Y-%m-%d %H:%M:%S")][$level][${SCRIPT_NAME}][$(whoami)] ${message}"
}

compress_module_pkg()
{
    mkdir -p ${SCANNER_ROOT}/SCANNER_rel
    cd ${SCANNER_ROOT}/SCANNER_rel
    mkdir -p ${SCANNER_ROOT}/SCANNER_rel/lib
    cp -r ${SCANNER_ROOT}/build-cmake-file/libScanner.so ${SCANNER_ROOT}/SCANNER_rel/lib
    mkdir -p ${SCANNER_ROOT}/SCANNER_rel/Module/
    MODULE_LIST="parser smb_ctx nfs_ctx threadpool metafile_parser system common_util log_util"
    for opensrc in ${MODULE_LIST}; do
        find ${SCANNER_ROOT}/Module/lib -name "*${opensrc}.so*" | xargs -I{} cp -f {} ${SCANNER_ROOT}/SCANNER_rel/Module
    done
    find ${SCANNER_ROOT}/Module/lib -name "*.so*" | xargs -I{} cp -f {} ${SCANNER_ROOT}/SCANNER_rel/Module

    mkdir -p ${SCANNER_ROOT}/SCANNER_rel/3rd/
    OPEN_LIST="boost jsoncpp openssl lsmb2 lnfs icu SecureCLib acl KMCv3_infra"
    # download dep 3rd for Scanner
    for opensrc in ${OPEN_LIST}; do
        find ${SCANNER_ROOT}/Module/third_open_src/${opensrc}_rel/lib -name "*.so*" | xargs -I{} cp -f {} ${SCANNER_ROOT}/SCANNER_rel/3rd
        find ${SCANNER_ROOT}/Module/third_open_src/${opensrc}_rel/libs -name "*.so*" | xargs -I{} cp -f {} ${SCANNER_ROOT}/SCANNER_rel/3rd
        find ${SCANNER_ROOT}/Module/platform/${opensrc}_rel/lib -name "*.so*" | xargs -I{} cp -f {} ${SCANNER_ROOT}/SCANNER_rel/3rd
        find ${SCANNER_ROOT}/Module/platform/${opensrc}_rel/libs -name "*.so*" | xargs -I{} cp -f {} ${SCANNER_ROOT}/SCANNER_rel/3rd
    done
    find ${SCANNER_ROOT}/Module/third_open_src/sqlite_rel/sqlite-autoconf -name "*.so*" | xargs -I{} cp -f {} ${SCANNER_ROOT}/SCANNER_rel/3rd
    find ${SCANNER_ROOT}/SCANNER_rel -name "*.so*" | xargs strip -s
    tar -zcvf SCANNER_rel.tar.gz  lib Module 3rd
    # rm -rf lib Module 3rd
    cd - 2>/dev/null
}

compress_module_pkg_for_AIX()
{
    mkdir -p ${SCANNER_ROOT}/SCANNER_rel
    cd ${SCANNER_ROOT}/SCANNER_rel
    mkdir -p ${SCANNER_ROOT}/SCANNER_rel/lib
    cp -f ${SCANNER_ROOT}/build-cmake-file/libScanner.a ${SCANNER_ROOT}/SCANNER_rel/lib
    mkdir -p ${SCANNER_ROOT}/SCANNER_rel/Module/
    local MODULE_DIR=${SCANNER_ROOT}/../Module
    find ${MODULE_DIR}/lib -name "*.a" | xargs -I{} cp -f {} ${SCANNER_ROOT}/SCANNER_rel/Module

    mkdir -p ${SCANNER_ROOT}/SCANNER_rel/3rd/
    find ${MODULE_DIR}/third_open_src -name "*.a" | xargs -I{} cp -f {} ${SCANNER_ROOT}/SCANNER_rel/3rd
    find ${MODULE_DIR}/platform -name "*.a" | xargs -I{} cp -f {} ${SCANNER_ROOT}/SCANNER_rel/3rd
    tar cvf SCANNER_rel.tar lib Module 3rd
    xz -v SCANNER_rel.tar
    rm -rf lib Module 3rd
    cd - 2>/dev/null
}

upload_plugin_2_cmc()
{
    local product="$1"
    local code_branch="$2"
    local componentType="$3"
    if [ -z "${PRODUCT}" -o -z "${CODE_BRANCH}" -o -z ${componentType} ]; then
        log_echo "ERROR" "Some variable is empty, please check"
        return 1
    fi

    local componentVersion="$4"
    if [ -z "${componentVersion}" ]; then
        componentVersion="${DEFAULT_COMPONENT_VERSION}"
    fi

    log_echo "DEBUG" "Product name ${product}"
    log_echo "DEBUG" "Use branch ${code_branch}"
    log_echo "DEBUG" "Component Version:${componentVersion}"
    log_echo "DEBUG" "Component Type:${componentType}"

    local arch_type=$(uname -m)
    if [ "$arch_type" == "aarch64" ]; then
        ARCH="aarch64"
    elif [ "X${CENTOS}" == "X6" ]; then 
        ARCH="x86_64_centos6"
    else
        ARCH="x86_64_centos7"
    fi

    cd ${LCRP_CONFIG_PATH}
    artget push -d pkg_into_cmc.xml -p "{'componentVersion':'${componentVersion}','PRODUCT':'${product}', \
    'CODE_BRANCH':'${code_branch}','COMPONENT_TYPE':'${componentType}', 'ARCH':'Linux/${ARCH}'}" \
    -ap "${SCANNER_ROOT}/SCANNER_rel/SCANNER_rel.tar.gz" -user ${cmc_user} -pwd ${cmc_pwd}
    if [ $? -ne 0 ]; then
        log_echo "ERROR" "Upload artifact from cmc error"
        return 1
    fi

    log_echo "INFO" "Finish to upload pkgs into cmc"
    return 0
}

upload_plugin_pkg()
{
    log_echo "Begin upload libScanner.so from cmc"
    upload_plugin_2_cmc ${PRODUCT} ${CODE_BRANCH} FS_SCANNER
    if [ $? -ne 0 ]; then
        log_echo "upload artifact error"
        exit 1
    fi
}

main()
{
    if [ "$(uname -s)" = "AIX" ]; then
        compress_module_pkg_for_AIX
        return $?
    else
        compress_module_pkg
    fi
    upload_plugin_pkg "$@"
    return $?
}

main "$@"
exit $?