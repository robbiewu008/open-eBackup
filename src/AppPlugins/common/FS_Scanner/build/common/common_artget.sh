#!/bin/bash
# artget操作封装
CURRENT_SCRIPT="${BASH_SOURCE[0]}"
SCRIPT_PATH=$(dirname ${BASH_SOURCE[0]})
LCRP_CONFIG_PATH=${SCRIPT_PATH}/../LCRP/conf
source ${SCRIPT_PATH}/common.sh
OPEN_SRC_EXT_PKG_PATH=${EXT_PKG_DOWNLOAD_PATH}/nas_open_src
OPEN_SRC_INNER_PACKET_PATH=${NAS_ROOT_DIR}/third_open_src/output_pkg
SCRIPT_NAME="${BASH_SOURCE[0]##*/}"
DEFAULT_COMPONENT_VERSION="1.1.0"
function init_artget_env()
{
    if [ -z ${LCRP_HOME} ]; then
        log_echo "ERROR" "Cant't find LCRP_HOME env, please config LCRP tool first"
        exit 1
    fi
    local arch_type=$(uname -m)
    if [ "$arch_type" == "aarch64" ]; then
        ARCH="euler-arm"
    else
        ARCH="euler-x86"
    fi
    local system_name=$(cat /etc/os-release 2>/dev/null | grep -E "\<ID\>" | awk -F "=" '{print $2}'| tr -d '"')
    if [ "X${system_name}" == "Xcentos" ];then
        ARCH="x86_64"
    fi
}
function download_artifact()
{
    local product="$1"
    local code_branch="$2"
    local componentType="$3"
    if [ -z "${product}" -o -z "${code_branch}" -o -z ${componentType} ]; then
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
    init_artget_env
    mkdir -p ${EXT_PKG_DOWNLOAD_PATH}/${componentType}
    cd ${LCRP_CONFIG_PATH}
    artget pull -d pkg_from_cmc.xml -p "{'componentVersion':'${componentVersion}','PRODUCT':'${product}', \
    'CODE_BRANCH':'${code_branch}','COMPONENT_TYPE':'${componentType}', 'ARCH':'${ARCH}'}" \
    -ap ${EXT_PKG_DOWNLOAD_PATH}/${componentType} -user ${cmc_user} -pwd ${cmc_pwd}
    if [ $? -ne 0 ]; then
        log_echo "ERROR" "Download artifact from cmc error"
        return 1
    fi
    log_echo "INFO" "Finish to download pkgs from cmc"
    echo "The pkgs in ${EXT_PKG_DOWNLOAD_PATH}/${componentType} :"
    ls -l ${EXT_PKG_DOWNLOAD_PATH}/${componentType}
    return 0
}

function download_scanner_3rd_4_cmc()
{
    local product="$1"
    local code_branch="$2"
    local componentType="Plugins"
    if [ -z "${product}" -o -z "${code_branch}" ]; then
        log_echo "ERROR" "Some variable is empty, please check"
        return 1
    fi
    local componentVersion="$3"
    if [ -z "${componentVersion}" ]; then
        componentVersion="${DEFAULT_COMPONENT_VERSION}"
    fi
    local src_name="NasOpenSource"
    log_echo "DEBUG" "Product name ${product}"
    log_echo "DEBUG" "Use branch ${code_branch}"
    log_echo "DEBUG" "Component Version:${componentVersion}"
    log_echo "DEBUG" "Component Type:${componentType}"
    init_artget_env
    mkdir -p ${EXT_PKG_DOWNLOAD_PATH}/${componentType}
    cd ${LCRP_CONFIG_PATH}
    echo "{'componentVersion':'${componentVersion}','PRODUCT':'${product}', \
    'CODE_BRANCH':'${code_branch}','COMPONENT_TYPE':'${componentType}', 'ARCH':'${ARCH}/${src_name}'}"
    artget pull -d scanner_3rd_from_cmc.xml -p "{'componentVersion':'${componentVersion}','PRODUCT':'${product}', \
    'CODE_BRANCH':'${code_branch}','COMPONENT_TYPE':'${componentType}', 'ARCH':'${ARCH}/${src_name}'}" \
    -ap ${EXT_PKG_DOWNLOAD_PATH}/${componentType} -user ${cmc_user} -pwd ${cmc_pwd}
    if [ $? -ne 0 ]; then
        log_echo "ERROR" "Download artifact from cmc error"
        return 1
    fi
    log_echo "INFO" "Finish to download pkgs from cmc"
    echo "The pkgs in ${EXT_PKG_DOWNLOAD_PATH}/${componentType} :"
    ls -l ${EXT_PKG_DOWNLOAD_PATH}/${componentType}
    return 0
}