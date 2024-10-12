#!bin/bash

CUR_PATH=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
BASE_PATH=${CUR_PATH}/../..
ROOT_PATH=${CUR_PATH}/../../..

code_xml="${BASE_PATH}/.cloudbuild/euler_arm/code.xml"

if [ -f "${BASE_PATH}/A8000_version" ]; then
    rm -rf ${BASE_PATH}/A8000_version
fi

function GET_SERVICE_REPO_INFO(){
    REPO=$1
    cd ${REPO}
    commit_address=$(git remote -v | grep fetch | awk '{print $2}')
    service_name=$(echo ${commit_address##*/} | awk -F '.' '{print $1}')
    commit_branch=$(cat ${code_xml} | grep ${service_name}.git | awk -F 'revision="' '{print $2}' |  awk -F '"' '{print $1}')
    commit_path=$(cat ${code_xml} | grep ${service_name}.git | awk -F 'path="' '{print $2}' |  awk -F '"' '{print $1}')
    commit_id=$(git log -1 | grep commit | awk '{print $2}')
    echo "<fileidentify repoBase=\"${commit_address}\" repoType=\"git\"  localpath=\"${commit_path}\" branch=\"${commit_branch}\" revision=\"${commit_id}\" />" >> ${BASE_PATH}/A8000_version
    cd - > /dev/null
}

#DPAProduct
GET_SERVICE_REPO_INFO "${BASE_PATH}"
if [ $? -ne 0 ]; then
    echo "get source code info failed!"
    exit 1
fi

#ProtectManager
GET_SERVICE_REPO_INFO "${ROOT_PATH}/ProtectManager"
if [ $? -ne 0 ]; then
    echo "get source code info failed!"
    exit 1
fi
cd ${ROOT_PATH}/ProtectManager/component
PM_DIR=$(ls)
for i in ${PM_DIR[@]}
do
    if [ -d ${i} ]; then
		GET_SERVICE_REPO_INFO "${ROOT_PATH}/ProtectManager/component/${i}"
	fi
done

#DataMoveEngine
GET_SERVICE_REPO_INFO "${ROOT_PATH}/DataMoveEngine"
if [ $? -ne 0 ]; then
    echo "get source code info failed!"
    exit 1
fi
cd ${ROOT_PATH}/DataMoveEngine/component
DME_DIR=$(ls)
for i in ${DME_DIR[@]}
do
    if [ -d ${i} ]; then
		GET_SERVICE_REPO_INFO "${ROOT_PATH}/DataMoveEngine/component/${i}"
	fi
done

#DataEnableEngine
GET_SERVICE_REPO_INFO "${ROOT_PATH}/DataEnableEngine"
if [ $? -ne 0 ]; then
    echo "get source code info failed!"
    exit 1
fi
cd ${ROOT_PATH}/DataEnableEngine/component
DEE_DIR=$(ls)
for i in ${DEE_DIR[@]}
do
    if [ -d ${i} ]; then
		GET_SERVICE_REPO_INFO "${ROOT_PATH}/DataEnableEngine/component/${i}"
	fi
done

#Infrastructure_OM
cd ${ROOT_PATH}/Infrastructure_OM
INF_DIR=$(ls)
for i in ${INF_DIR[@]}
do
    if [ -d ${i} ]; then
		GET_SERVICE_REPO_INFO "${ROOT_PATH}/Infrastructure_OM/${i}"
	fi
done

#Agent
cd ${ROOT_PATH}/ProtectAgent/component
Agent_DIR=$(ls)
for i in ${Agent_DIR[@]}
do
    if [ -d ${i} ]; then
		GET_SERVICE_REPO_INFO "${ROOT_PATH}/ProtectAgent/component/${i}"
	fi
done

#组合源码清单xml
cd ${BASE_PATH}/ 
sed -i '7 r A8000_version' ${BASE_PATH}/CI/LCRP/conf/SourceCode.xml
