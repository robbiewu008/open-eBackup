#!/bin/bash
set -e
cd $(dirname ${BASH_SOURCE[0]})
source ../../common.sh
SIMBAOS_PACKAGE_NAME=$1
NODE_IP=$2

cd ${PACKAGE_BASE_PATH}
SIMBAOS_UNPACK_PATH=${PACKAGE_BASE_PATH}/`basename ${SIMBAOS_PACKAGE_NAME} .tar.gz`

install ${SIMBAOS_UNPACK_PATH}/SimbaOS/action/smartkube ${SIMBAOS_SMARTKUBE_INSTALL_PATH}

smartkube preinstall env --packagePath=${SIMBAOS_UNPACK_PATH}/SimbaOS --createSSHUser

rm -rf /opt/k8s/db/etcd
rm -rf ${SIMBAOS_PACKAGE_PATH}/* && \
    cp -r ${SIMBAOS_UNPACK_PATH}/SimbaOS/* ${SIMBAOS_PACKAGE_PATH}
chown -hR kadmin:kgroup ${SIMBAOS_PACKAGE_PATH}

echo "preinstall simbaos succeed"

# wait for smartkube agent to start
sleep 10
