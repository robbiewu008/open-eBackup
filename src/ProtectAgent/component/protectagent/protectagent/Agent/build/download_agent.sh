#!/bin/bash
CURRENT_PATH=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
product=$1
code_branch=$2
componentType=$3
componentVersion=$4
BUILD_PKG_TYPE=$5
DEFAULT_COMPONENT_VERSION="1.1.0"
CLIENT_DOWNLOAD_PATH=${CURRENT_PATH}/client

############################################################################
function check_param()
{
    if [ -z "${product}" -o -z "${code_branch}" -o -z "${componentType}" ]; then
        echo "Some variable is empty, please check."
        exit 1
    fi

    if [ -z "${componentVersion}" ]; then
        componentVersion="${DEFAULT_COMPONENT_VERSION}"
    fi
}

function init_artget_agent_env()
{
    # 1.
    if [ ! -d "${CLIENT_DOWNLOAD_PATH}" ]; then
        echo "mkdir ${CLIENT_DOWNLOAD_PATH}"
        mkdir "${CLIENT_DOWNLOAD_PATH}"
    fi

    rm -rf "${CLIENT_DOWNLOAD_PATH}"/*
}

function download_agent()
{
    # 1.
    echo "componentVersion=${componentVersion}"
    echo "product=${product}"
    echo "code_branch=${code_branch}"
    echo "componentType=${componentType}"
    echo "CLIENT_DOWNLOAD_PATH=${CLIENT_DOWNLOAD_PATH}"
    echo "LCRP_HOME=${LCRP_HOME}"

    cd ${CURRENT_PATH}/../ci/LCRP/conf/
    if [ "$BUILD_PKG_TYPE" = "OceanCyber" ]; then
        artget pull -d OceanCyber_agent_pkg_from_cmc.xml -p "{'componentVersion':'${componentVersion}','PRODUCT':'${product}', \
        'CODE_BRANCH':'${code_branch}','COMPONENT_TYPE':'${componentType}'}" \
        -ap ${CLIENT_DOWNLOAD_PATH} -user ${cmc_user} -pwd ${cmc_pwd}
    else
        artget pull -d agent_pkg_from_cmc.xml -p "{'componentVersion':'${componentVersion}','PRODUCT':'${product}', \
        'CODE_BRANCH':'${code_branch}','COMPONENT_TYPE':'${componentType}'}" \
        -ap ${CLIENT_DOWNLOAD_PATH} -user ${cmc_user} -pwd ${cmc_pwd}
    fi

    if [ $? -ne 0 ]; then
        echo "Download agent from cmc error"
        exit 1
    fi
}

# 1.
check_param

# 2.
init_artget_agent_env

# 3.
download_agent

echo "======================download agent package from cmc successfully========================="
exit 0