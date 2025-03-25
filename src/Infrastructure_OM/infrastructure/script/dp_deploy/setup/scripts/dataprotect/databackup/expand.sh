#!/bin/bash
set -e
cd $(dirname ${BASH_SOURCE[0]})
source ../../common.sh

PACKAGE_NAME=$1
REPLICAS=$2
NODE_NAME0=$3
NODE_NAME1=$4
NODE_NAME2=$5
FLOAT_IP=$6
GATEWAY_IP=$7

PACKAGE_PATH=${PACKAGE_BASE_PATH}/`basename ${PACKAGE_NAME} .tgz`
cd ${PACKAGE_PATH}

helm upgrade dataprotect databackup-*.tgz \
    --set global.cluster_enable=true \
    --set global.replicas=${REPLICAS} \
    --set global.master_node0=${NODE_NAME0} \
    --set global.master_node1=${NODE_NAME1} \
    --set global.master_node2=${NODE_NAME2} \
    --set global.floatip=${FLOAT_IP} \
    --set global.gatewayip=${GATEWAY_IP} \
    --set global.deploy_type=d8 \
    --wait --timeout=30m
