#!/bin/bash
########################################
#  This file is part of the open-eBackup project.
# Copyright (c) 2024 Huawei Technologies Co.,Ltd.
#
# open-eBackup is licensed under MPL v2.
# You can use this software according to the terms and conditions of the MPL v2.
# You may obtain a copy of MPL v2 at:
#
#          https://www.mozilla.org/en-US/MPL/2.0
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the MPL v2 for more details.
########################################
set -x
CURRENT_PATH=$(cd `dirname $0`; pwd)
source $CURRENT_PATH/commParam.sh
PACKAGE_PATH="${binary_path}/Infrastructure_OM/infrastructure"
LCRP_XML_PATH=${CURRENT_PATH}/../conf/
FILECLIENT_BRANCH="master_OceanProtect_DataBackup_1.6.0_smoke"
KMC_PATH=${CURRENT_PATH}/upgrade_opensrc/kmc
BASE_PATH=${PACKAGE_PATH}/kmc
PRODUCT="dorado"
pkg="platform.tar.gz"
if [ -z "${componentVersion}" ]; then
    componentVersion="1.1.0"
fi
echo "Component Version:${componentVersion}"

if [ ! -d "${PACKAGE_PATH}" ];then
  mkdir -p "${PACKAGE_PATH}"
fi

function down_package_from_cmc()
{
    #artget下载依赖
    cd ${LCRP_XML_PATH}
    artget pull -d baseImage_dependency_cmc.xml -p "{ \
        'componentVersion':'${componentVersion}', \
        'slim_image':'${slim_image}', \
        'PACKAGE1':'${PACKAGE1}', \
        'PACKAGE2':'${PACKAGE2}', \
        'jdk_package':'${jdk_package}', \
        'gaussdb_python':'${gaussdb_python}', \
        'CODE_BRANCH':'${CODE_BRANCH}', \
        'FILECLIENT_BRANCH': '${FILECLIENT_BRANCH}' \
    }" -ap ${PACKAGE_PATH} -user ${cmc_user} -pwd ${cmc_pwd}

    if [ $? -ne 0 ];then
        echo "Download artifact from cmc error"
        exit 1
    fi
    artget pull -os cbb_dependency_opensource.xml -ap ${PACKAGE_PATH} -user ${opensource_user} -pwd ${opensource_pwd} -at opensource
    if [ $? -ne 0 ];then
        echo "Download opensource from cmc error"
        exit 1
    fi

    echo "down_package_from_cmc success"
}

downloadartifact() {
    echo "Begin down 3rd form cmc"
    mkdir -p "${BASE_PATH}"
    mkdir -p "${KMC_PATH}"
    cd ${KMC_PATH}
    sh down_pkg_from_cmc.sh ${PRODUCT} ${CODE_BRANCH} ${pkg} 3rd
    if [ $? -ne 0 ]; then
        echo "Download artifact error"
        exit 1
    fi
}

upzipartifact() {
    # 移动至upgrade_opensrc/kmc/KmcLib/目录下，构成基础镜像从此目录下获取文件
    rm -rf "${BASE_PATH}/KmcLib/"
    mkdir -p ${BASE_PATH}/KmcLib
    PLATFORM_PATH=${BASE_PATH}/platform
    mkdir -p "${PLATFORM_PATH}"

    echo "Begin to untar platform"
    tar xzf platform.tar.gz -C ${PLATFORM_PATH}
    if [ $? -ne 0 ]; then
        echo "Untar platform error"
        exit 1
    fi
    echo "Untar platform success"

    /bin/cp -rf ${PLATFORM_PATH}/KMCv3_infra_rel/lib ${BASE_PATH}/KmcLib/
    /bin/cp -rf ${PLATFORM_PATH}/KMCv3_infra_rel/include ${BASE_PATH}/KmcLib/
    /bin/cp -rf ${PLATFORM_PATH}/KMCv3_infra_rel/bin/* ${BASE_PATH}/KmcLib/lib/

}

down_kmc() {
    downloadartifact
    upzipartifact
}

function main()
{
    down_package_from_cmc
    down_kmc

    cd "${CURRENT_PATH}"
    sh open_build_compile_pkg.sh
    if [ $? -ne 0 ];then
        echo "cp inf bin error"
        exit 1
    fi
    echo "cp inf bin success"

    cd "${CURRENT_PATH}"/../../om/build
    sh copy_bin.sh
    if [ $? -ne 0 ];then
        echo "cp om bin error"
        exit 1
    fi
    rm -f ${PACKAGE_PATH}/gaussdb*.tar.gz
    rm -f ${PACKAGE_PATH}/GaussDB*.tar.gz
    rm -f ${PACKAGE_PATH}/euleros*.tar.xz
    rm -f ${PACKAGE_PATH}/EulerOS*.tar.gz
    rm -f ${PACKAGE_PATH}/devel_tools.tar.gz

    echo "cp bin success"
}

main