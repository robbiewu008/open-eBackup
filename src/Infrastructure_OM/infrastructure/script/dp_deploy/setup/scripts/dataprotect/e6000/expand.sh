#!/bin/bash
set -e
cd $(dirname ${BASH_SOURCE[0]})
source ../../common.sh

PACKAGE_NAME=$1
PM_REPLICAS=$2
PE_REPLICAS=$3

PACKAGE_PATH=${PACKAGE_BASE_PATH}/`basename ${PACKAGE_NAME} .tgz`
cd ${PACKAGE_PATH}

helm upgrade dataprotect databackup-*.tgz \
    --set global.pm_replicas=${PM_REPLICAS} \
    --set global.pe_replicas=${PE_REPLICAS} \
    --set global.deploy_type=d7