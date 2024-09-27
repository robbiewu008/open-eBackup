#!/bin/bash

CUR_PATH=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
BASE_PATH=${CUR_PATH}/../..
CODE_PATH=$1

function copy_code(){
    echo "=========== Start copy code ==========="
    # 拷贝文件
    cp -rf ${BASE_PATH}/build ${CODE_PATH}/ProtectManager
    cp -rf ${BASE_PATH}/build-dev ${CODE_PATH}/ProtectManager
    cp -rf ${BASE_PATH}/CI ${CODE_PATH}/ProtectManager
    cp -rf ${BASE_PATH}/component/PM_Boot_Dependencies ${CODE_PATH}/ProtectManager/component/
    cp -rf ${BASE_PATH}/fossbot ${CODE_PATH}/ProtectManager
    cp -rf ${BASE_PATH}/maven ${CODE_PATH}/ProtectManager
    cp -rf ${BASE_PATH}/OpenSourceCenter ${CODE_PATH}/ProtectManager
    cp -rf ${BASE_PATH}/package ${CODE_PATH}/ProtectManager

    echo "=========== End copy code ==========="
}

function copy_component() {
  PM_MS_LIST="PM_GUI PM_System_Base_Common_Service PM_Data_Protection_Service PM_API_Gateway PM_Database_Version_Migration"
  for pmservice in ${PM_MS_LIST}; do
    echo "start copy ${pmservice}!"
    cd ${BASE_PATH}/component/${pmservice}/CI
    if [ "${pmservice}" == "PM_System_Base_Common_Service" ]; then
      sh copy_code.sh "${CODE_PATH}"
      if [ $? -ne 0 ]; then
        echo "${pmservice} copy_code failed"
        exit 1
      fi
    else
      sh copy_code.sh "${CODE_PATH}"
      if [ $? -ne 0 ]; then
        echo "${pmservice} compile failed!"
        exit 1
      fi
    fi
  done
}

function main(){
  copy_code
  copy_component
}

main $@