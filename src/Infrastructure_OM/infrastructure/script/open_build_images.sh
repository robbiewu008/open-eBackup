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
BASE_PATH=${CURRENT_PATH}/../..
CI_PATH=${BASE_PATH}/ci
OM_PATH=${BASE_PATH}/om
PKG_DIR="${binary_path}/Infrastructure_OM/infrastructure"
sh ${CI_PATH}/script/open_comm_param.sh
source $CURRENT_PATH/commParam.sh
G_INFRASTRUCTURE_NAME="elasticsearch gaussdb kafka redis zookeeper sftp"
L_BASE_TAG="open-ebackup-1.0:base"

function compile_gaussdb_package()
{
    if [ -d ${COMPILE_PATH} ];then
        rm -rf ${COMPILE_PATH}
    fi
    mkdir ${COMPILE_PATH}
    cd ${PACKAGE_PATH}
    # GaussDB
    if [ -d gaussdb-${gaussdb_version} ];then
        rm -rf gaussdb-${gaussdb_version}
    fi

    mkdir -p GaussDB-${gaussdb_version}-aarch-64bit-Green


    cp ${PKG_DIR}/../openGauss-Lite-5.0.0-openEuler-aarch64.tar.gz .
    tar -zxf openGauss-Lite-5.0.0-openEuler-aarch64.tar.gz -C GaussDB-${gaussdb_version}-aarch-64bit-Green
    sed -i '561a\kernel=openEuler'  GaussDB-${gaussdb_version}-aarch-64bit-Green/install.sh

    chmod 500 ${CURRENT_PATH}/gaussdb/ -R
    cp -a ${CURRENT_PATH}/gaussdb/install_opengauss.sh GaussDB-${gaussdb_version}-aarch-64bit-Green
    cp -a ${CURRENT_PATH}/gaussdb/logrotate_gaussdb.conf GaussDB-${gaussdb_version}-aarch-64bit-Green
    sed -i "s/gaussdb_port/${gaussdb_port}/g" GaussDB-${gaussdb_version}-aarch-64bit-Green/install_opengauss.sh
    cp -a ${CURRENT_PATH}/gaussdb/gaussdb_kmc.py GaussDB-${gaussdb_version}-aarch-64bit-Green

    cp -a ${CURRENT_PATH}/gaussdb/check_health.sh GaussDB-${gaussdb_version}-aarch-64bit-Green
    cp -a ${CURRENT_PATH}/gaussdb/check_gaussdb_readiness.sh GaussDB-${gaussdb_version}-aarch-64bit-Green

    cp -a ${CURRENT_PATH}/gaussdb/log.sh GaussDB-${gaussdb_version}-aarch-64bit-Green
    cp -a ${CURRENT_PATH}/gaussdb/cert_install.sh GaussDB-${gaussdb_version}-aarch-64bit-Green
    cp -a ${CURRENT_PATH}/gaussdb/set_kmc_password.sh GaussDB-${gaussdb_version}-aarch-64bit-Green
    cp -a ${CURRENT_PATH}/gaussdb/change_config.sh GaussDB-${gaussdb_version}-aarch-64bit-Green

    cp -a ${CURRENT_PATH}/gaussdb/gaussdb_common.py GaussDB-${gaussdb_version}-aarch-64bit-Green
    sed -i "s/gaussdb_port/${gaussdb_port}/g" GaussDB-${gaussdb_version}-aarch-64bit-Green/check_health.sh
    sed -i "s/gaussdb_port/${gaussdb_port}/g" GaussDB-${gaussdb_version}-aarch-64bit-Green/check_gaussdb_readiness.sh
    if [ ! -d "GaussDB-${gaussdb_version}-aarch-64bit-Green/script" ];then
        mkdir GaussDB-${gaussdb_version}-aarch-64bit-Green/script
    fi

    cp -a ${CURRENT_PATH}/gaussdb/change_permission.sh GaussDB-${gaussdb_version}-aarch-64bit-Green/script
    cp -a ${CURRENT_PATH}/gaussdb/common_sudo.sh GaussDB-${gaussdb_version}-aarch-64bit-Green/script
    cp -a ${CURRENT_PATH}/gaussdb/gauss_operation.sh GaussDB-${gaussdb_version}-aarch-64bit-Green/script

    cp -r "${CURRENT_PATH}"/common GaussDB-${gaussdb_version}-aarch-64bit-Green/script/
    cp -r "${CURRENT_PATH}"/kmc GaussDB-${gaussdb_version}-aarch-64bit-Green/script/
    mv GaussDB-${gaussdb_version}-aarch-64bit-Green gaussdb-${gaussdb_version}
    tar -zcf ${COMPILE_PATH}/gaussdb-${gaussdb_version}.tar.gz gaussdb-${gaussdb_version}/
    cp -rf ${COMPILE_PATH}/gaussdb-${gaussdb_version}.tar.gz ${PKG_DIR}
}

function build_image() {
    local NAME=$1
    echo "begin to build service ${NAME}"

    L_TAG=$(cat ${CI_PATH}/build/Infrastructure/dockerfiles/${NAME}.name)
    L_MS_NAME=$(echo ${L_TAG} | sed 's/:/-/g')
    L_TAG=$(echo ${L_TAG} | awk -F ':' '{print $1 FS}')
    L_TAG=${L_TAG}${MS_IMAGE_TAG}

    mkdir -p ${PKG_DIR}/tmp/${L_MS_NAME}

    if [ -f "${PKG_DIR}/${L_MS_NAME}.tar.gz" ]; then
        cp -rf "${PKG_DIR}/${L_MS_NAME}.tar.gz"  "${PKG_DIR}/tmp/${L_MS_NAME}"
        if [ $? != 0 ]; then
            echo "copy $L_MS_NAME.tar.gz failed"
            exit 1
        fi
    else
        echo "${PKG_DIR}/${L_MS_NAME}.tar.gz not found for build"
        exit 1
    fi

    if [ "${NAME}" == "gaussdb" ]; then
        tar -zxpf ${PKG_DIR}/tmp/${L_MS_NAME}/${L_MS_NAME}.tar.gz -C ${PKG_DIR}/tmp/${L_MS_NAME}/
    fi

    if [ "${NAME}" == "zookeeper" ] || [ "${NAME}" == "elasticsearch" ]; then
      tar -zxvf ${PKG_DIR}/tmp/${L_MS_NAME}/${L_MS_NAME}.tar.gz -C ${PKG_DIR}/tmp/${L_MS_NAME}/
    fi

    if [ "${NAME}" == "kafka" ]; then
      tar -zxvf ${PKG_DIR}/tmp/${L_MS_NAME}/${L_MS_NAME}.tar.gz -C ${PKG_DIR}/tmp/${L_MS_NAME}/
      cp -rf "${PKG_DIR}/${NAME}-scripts.tar.gz"  ${PKG_DIR}/tmp/${L_MS_NAME}/
      tar -zxvf ${PKG_DIR}/tmp/${L_MS_NAME}/${NAME}-scripts.tar.gz -C ${PKG_DIR}/tmp/${L_MS_NAME}/
    fi

    if [ "${NAME}" == "redis" ]; then
      cp -rf "${PKG_DIR}/${NAME}-scripts.tar.gz"  "${PKG_DIR}/tmp/${L_MS_NAME}/"
      tar -zxvf ${PKG_DIR}/tmp/${L_MS_NAME}/${NAME}-scripts.tar.gz -C ${PKG_DIR}/tmp/${L_MS_NAME}/
    fi

    docker build -t ${L_TAG} -f ${CI_PATH}/build/Infrastructure/dockerfiles/open-ebackup_${NAME}.dockerfile ${PKG_DIR}/tmp/${L_MS_NAME}
    if [[ $? -ne 0 ]]; then
        echo "docker build ${L_MS_NAME} failed"
        rm -rf "${PKG_DIR}/tmp/${L_MS_NAME}"
        return 1
    fi
    rm -rf "${PKG_DIR}/tmp/${L_MS_NAME}"
    echo "docker build ${L_MS_NAME} success"

}

function build() {
    for NAME in ${G_INFRASTRUCTURE_NAME}
    do
        build_image ${NAME}
        if [ $? -ne 0 ];then
            echo "${MS_NAME} docker iamge build failed"
            exit 1
        fi
     done
}

function main() {
    compile_gaussdb_package
    build
}

main $@