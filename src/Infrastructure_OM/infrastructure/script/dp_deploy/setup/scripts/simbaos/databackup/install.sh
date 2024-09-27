#!/bin/bash
set -e
cd $(dirname ${BASH_SOURCE[0]})
source ../../common.sh

cd /home/kadmin
sudo -u kadmin ${SIMBAOS_SMARTKUBE_INSTALL_PATH}/smartkube install \
    --folder=${SIMBAOS_PACKAGE_PATH} \
    --ignore-addon-list="DHAC component deployment,Monitor component deployment,App-manager component deployment"
