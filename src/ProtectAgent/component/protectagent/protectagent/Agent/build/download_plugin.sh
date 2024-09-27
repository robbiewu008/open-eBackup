#!/bin/bash
CURRENT_PATH=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
product=$1
code_branch=$2
componentType=$3
componentVersion=$4
BUILD_PKG_TYPE=$5
DEFAULT_COMPONENT_VERSION="1.1.0"
ARCH=""
PLUGIN_PKG_DOWNLOAD_PATH=${CURRENT_PATH}/Plugins
PLUGIN_NAME="NasPlugin"

############################################################################
function check_param()
{
    if [ -z "${product}" -o -z "${code_branch}" -o -z "${componentType}" ] ; then
        echo "Some variable is empty, please check."
        exit 1
    fi

    if [ -z "${componentVersion}" ]; then
        componentVersion="${DEFAULT_COMPONENT_VERSION}"
    fi
}

function init_artget_plugin_env()
{
    # 1.
    if [ ! -d "${PLUGIN_PKG_DOWNLOAD_PATH}" ]; then
        echo "mkdir ${PLUGIN_PKG_DOWNLOAD_PATH}"
        mkdir "${PLUGIN_PKG_DOWNLOAD_PATH}"
    fi

    rm -rf "${PLUGIN_PKG_DOWNLOAD_PATH}"/*

    # 2.
    ARCH="euler-arm"
}

function download_plugin()
{
    # 1.
    echo "componentVersion=${componentVersion}"
    echo "product=${product}"
    echo "code_branch=${code_branch}"
    echo "componentType=${componentType}"
    echo "ARCH=${ARCH}"
    echo "PLUGIN_NAME=${PLUGIN_NAME}"
    echo "PLUGIN_PKG_DOWNLOAD_PATH=${PLUGIN_PKG_DOWNLOAD_PATH}"
    echo "LCRP_HOME=${LCRP_HOME}"

    cd ${CURRENT_PATH}/../ci/LCRP/conf/
    if [ "$BUILD_PKG_TYPE" = "OceanCyber" ]; then
        artget pull -d OceanCyber_plugin_pkg_from_cmc.xml -p "{'componentVersion':'${componentVersion}','PRODUCT':'${product}', \
        'CODE_BRANCH':'${code_branch}', \
        'componentType':'${componentType}'}" -ap ${PLUGIN_PKG_DOWNLOAD_PATH} \
        -user ${cmc_user} -pwd ${cmc_pwd}
    else
        artget pull -d plugin_pkg_from_cmc.xml -p "{'componentVersion':'${componentVersion}','PRODUCT':'${product}', \
        'CODE_BRANCH':'${code_branch}','VIRTUALIZATION_BRANCH':'${VIRTUALIZATION_BRANCH}','GENERALDB_BRANCH':'${GENERALDB_BRANCH}','OBS_Plugins_BRANCH':'${OBS_Plugins_BRANCH}', \
        'BLOCKSERVICE_BRANCH':'${BLOCKSERVICE_BRANCH}','componentType':'${componentType}'}" -ap ${PLUGIN_PKG_DOWNLOAD_PATH} \
        -user ${cmc_user} -pwd ${cmc_pwd}
    fi

    if [ $? -ne 0 ]; then
        echo "Download plugin from cmc error"
        exit 1
    fi
}

# 1.
check_param

# 2.
init_artget_plugin_env

# 3.
download_plugin

echo "======================download plugin package from cmc successfully========================="
exit 0