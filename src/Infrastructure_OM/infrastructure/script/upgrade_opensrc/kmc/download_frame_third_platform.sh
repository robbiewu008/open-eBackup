#!/bin/bash

if [ -z ${LCRP_HOME} ]; then
    echo "Cant't find LCRP_HOME env, please config LCRP tool first"
    exit 1
fi

PRODUCT=$1
CODE_BRANCH=$2
pkg="platform.tar.gz"

CURRENT_PATH=$(pwd)

if [ -z "${PRODUCT}" ]; then
    PRODUCT="dorado"
fi

if [ -z "${CODE_BRANCH}" ]; then
    CODE_BRANCH="BR_Dev"
fi

echo "Product name ${PRODUCT}"
echo "Use branch ${CODE_BRANCH}"

downloadartifact() {
    echo "Begin down 3rd form cmc"
    sh down_pkg_from_cmc.sh ${PRODUCT} ${CODE_BRANCH} ${pkg} 3rd
    if [ $? -ne 0 ]; then
        echo "Download artifact error"
        exit 1
    fi
}

upzipartifact() {
    # 移动至upgrade_opensrc/kmc/KmcLib/目录下，构成基础镜像从此目录下获取文件
    rm -rf "${CURRENT_PATH}/KmcLib/"
    mkdir -p ${CURRENT_PATH}/KmcLib
    PLATFORM_PATH=${CURRENT_PATH}/platform
    mkdir -p "${PLATFORM_PATH}"

    echo "Begin to untar platform"
    tar xzf platform.tar.gz -C ${PLATFORM_PATH}
    if [ $? -ne 0 ]; then
        echo "Untar platform error"
        exit 1
    fi
    echo "Untar platform success"

    /bin/cp -rf ${PLATFORM_PATH}/KMCv3_infra_rel/lib ${CURRENT_PATH}/KmcLib/
    /bin/cp -rf ${PLATFORM_PATH}/KMCv3_infra_rel/include ${CURRENT_PATH}/KmcLib/
    /bin/cp -rf ${PLATFORM_PATH}/KMCv3_infra_rel/bin/* ${CURRENT_PATH}/KmcLib/lib/

}


main() {
    downloadartifact
    upzipartifact
}

echo "#########################################################"
echo "   Begin download from cmc"
echo "#########################################################"

main

echo "#########################################################"
echo "   Success download from cmc"
echo "#########################################################"
