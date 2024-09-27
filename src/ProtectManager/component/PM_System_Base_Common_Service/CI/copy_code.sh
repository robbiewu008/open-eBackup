#!/bin/bash

CUR_PATH=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
PM_MS_DIR=${CUR_PATH}/..
BASE_PATH=${PM_MS_DIR}/../..
CODE_PATH=$1

function copy_code(){
    echo "=========== Start copy code ==========="
    # 拷贝文件
    cp -rf ${PM_MS_DIR}/../PM_System_Base_Common_Service ${CODE_PATH}/ProtectManager/component
    rm -rf ${CODE_PATH}/ProtectManager/component/PM_System_Base_Common_Service/src/pm-framework/*
    rm -rf ${CODE_PATH}/ProtectManager/component/PM_System_Base_Common_Service/src/pm-plugins/*
    cp -rf ${PM_MS_DIR}/src/pm-framework/pm-access-framework ${CODE_PATH}/ProtectManager/component/PM_System_Base_Common_Service/src/pm-framework/
    cp -rf ${PM_MS_DIR}/src/pm-framework/pm-access-provider-sdk ${CODE_PATH}/ProtectManager/component/PM_System_Base_Common_Service/src/pm-framework/
    cp -rf ${PM_MS_DIR}/src/pm-plugins/pm-cnware-protection-plugin ${CODE_PATH}/ProtectManager/component/PM_System_Base_Common_Service/src/pm-plugins/
    cp -rf ${PM_MS_DIR}/src/pm-plugins/pm-database-plugins ${CODE_PATH}/ProtectManager/component/PM_System_Base_Common_Service/src/pm-plugins/
    cp -rf ${PM_MS_DIR}/src/pm-plugins/pm-file-protection-plugins ${CODE_PATH}/ProtectManager/component/PM_System_Base_Common_Service/src/pm-plugins/
    cp -rf ${PM_MS_DIR}/src/pm-plugins/pm-openstack-plugins ${CODE_PATH}/ProtectManager/component/PM_System_Base_Common_Service/src/pm-plugins/
    rm -rf ${CODE_PATH}/ProtectManager/component/PM_System_Base_Common_Service/src/pm-main-server
    echo "=========== End copy code ==========="
}

function main(){
  copy_code
}

main $@