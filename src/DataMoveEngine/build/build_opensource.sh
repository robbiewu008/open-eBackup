#!/bin/sh
EBACK_BASE_DIR="$(cd "$(dirname "$BASH_SOURCE")/../";pwd)"
 
CODE_BRANCH=$1
INF_BRANCH=$2
BIN_PATH=$3
 
cp -r ${BIN_PATH}/DataMoveEngine/pkg ${EBACK_BASE_DIR}/
if [ $? -ne 0 ];then
    echo -e "Copy dme pkg failed"
    return 1
fi
 
cd "${EBACK_BASE_DIR}/CI/script"
sh build_image_opensource.sh ${CODE_BRANCH} ${INF_BRANCH}
if [ $? -ne 0 ];then
    echo -e "Build dme images failed"
    return 1
fi