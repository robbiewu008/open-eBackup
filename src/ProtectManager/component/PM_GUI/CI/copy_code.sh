#!/bin/bash

CUR_PATH=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
PM_MS_DIR=${CUR_PATH}/../..
BASE_PATH=${PM_MS_DIR}/../..
CODE_PATH=$1

function copy_code(){
    echo "=========== Start copy code ==========="
    # 拷贝文件
    cp -rf ${PM_MS_DIR}/PM_GUI ${CODE_PATH}/ProtectManager/component
    echo "=========== End copy code ==========="
}

function main(){

  copy_code

}

main $@