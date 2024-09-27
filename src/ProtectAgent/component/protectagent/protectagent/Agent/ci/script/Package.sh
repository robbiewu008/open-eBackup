#!bin/bash
CURRENT_PATH=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
BASE_PATH=${CURRENT_PATH}/../../../
BUILD_PKG_TYPE=$1

function main(){
    cd ${BASE_PATH}/Agent/build/
    cp -rf ${BASE_PATH}/Agent/ci/LCRP/conf/Setting.xml ${LCRP_HOME}/conf/
    sh build_image.sh ${AGENT_BRANCH} ${INF_BRANCH} ${BUILD_PKG_TYPE}
    if [ $? -ne 0 ];then
        echo "Image build failed."
        exit 1
    fi
}


echo "#########################################################"
echo "   Begin build ProtectAgent_Image  "
echo "#########################################################"
main
echo "#########################################################"
echo "   ProtectAgent_Image package Success  "
echo "#########################################################"