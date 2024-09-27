#!/bin/bash
set -e

cd $(dirname ${BASH_SOURCE[0]})
source ../common.sh

IMAGE_NAME=$1

IMAGE_PATH=${PACKAGE_BASE_PATH}/`basename ${IMAGE_NAME} .tgz`

cd ${IMAGE_PATH}
isula load -i *.tar.xz