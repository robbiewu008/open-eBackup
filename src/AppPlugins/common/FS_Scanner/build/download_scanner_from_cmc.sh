#!/bin/bash
# build AppPlugins_NAS
CURRENT_SCRIPT="${BASH_SOURCE[0]}"
SCRIPT_PATH=$(cd "$(dirname ${BASH_SOURCE[0]})"; pwd)
SCRIPT_NAME="${BASH_SOURCE[0]##*/}"
SCANNER_ROOT=$(cd "${SCRIPT_PATH}/.."; pwd)
LCRP_CONFIG_PATH=${SCRIPT_PATH}/LCRP/conf
DOWNLOAD_PATH=${SCANNER_ROOT}/ext_pkg

# 产品名称
PRODUCT="dorado"
DEFAULT_COMPONENT_VERSION="1.1.0"

CODE_BRANCH=$1
if [ -z "${CODE_BRANCH}" ]; then
    CODE_BRANCH="${branch}"
fi

function log_echo()
{
    local level="$1"
    local message="$2"
    echo "[$(date "+%Y-%m-%d %H:%M:%S")][$level][${SCRIPT_NAME}][$(whoami)] ${message}"
}


function download_plugin_4_cmc()
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
    elif [ ${CENTOS} == "6" ]; then 
        ARCH="x86_64_centos6"
    else
        ARCH="x86_64_centos7"
    fi

    rm -rf  ${DOWNLOAD_PATH}
    mkdir -p ${DOWNLOAD_PATH}
    cd ${LCRP_CONFIG_PATH}
    artget pull -d module_pkg_from_cmc.xml -p "{'componentVersion':'${componentVersion}','PRODUCT':'${product}', \
    'CODE_BRANCH':'${code_branch}','COMPONENT_TYPE':'${componentType}', 'ARCH':'Linux/${ARCH}/*'}" \
    -ap "${DOWNLOAD_PATH}/" -user ${cmc_user} -pwd ${cmc_pwd}
    if [ $? -ne 0 ]; then
        log_echo "ERROR" "Upload artifact from cmc error"
        return 1
    fi

    log_echo "INFO" "Finish to upload pkgs into cmc"
    return 0
}

function download_lib_pkg()
{
    log_echo "Begin download scanner from cmc"
    download_plugin_4_cmc ${PRODUCT} ${CODE_BRANCH} FS_SCANNER
    if [ $? -ne 0 ]; then
        log_echo "upload artifact error"
        return 1
    fi
}

function main()
{
    download_lib_pkg "$@"
    if [ $? -ne 0 ];then
        return 1
    fi
}

main "$@"
exit $?