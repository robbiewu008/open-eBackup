#!/bin/bash

SCRIPT_PATH=$(dirname ${BASH_SOURCE[0]})
CURRENT_DIR=$(cd "${SCRIPT_PATH}"; pwd)
BACKUP_HOME=$(cd "${SCRIPT_PATH}/.."; pwd)
arch_type=$(uname -m)

function download_dep()
{
    git clone -b ${MODULE_BRANCH} ssh://git@szv-y.codehub.huawei.com:2222/dpa/CBB/CPP/Module.git
}

if [ "${Target_Branch}" == "SUB_SYSTEM_1.2.1_TR5" ];then
    MODULE_BRANCH="SUB_SYSTEM_1.2.1_TR5"
elif [[ "${Target_Branch}" == "SUB_SYSTEM" || "${Target_Branch}" == "BR_Dev" || "${Target_Branch}" == "BR_SMOKE" || "${Target_Branch}" == "master_backup_software_1.6.0RC1" || "${Target_Branch}" == "develop_backup_software_1.6.0RC1" ]];then
    MODULE_BRANCH=${Target_Branch}
else
    MODULE_BRANCH=develop_backup_software_1.6.0RC1
fi

download_dep

if [ "${arch_type}" == "aarch64" ]; then
    if [[ "${Target_Branch}" == "BR_Dev" || "${Target_Branch}" == "BR_SMOKE" || "${Target_Branch}" == "master_backup_software_1.6.0RC1" ]];then
        sh ${BACKUP_HOME}/build/make_nas_scanner.sh
        exit $?
    fi
    sh ${BACKUP_HOME}/test/localhost_test/run_hdt_test.sh
    exit $?
else
    sh ${BACKUP_HOME}/build/make_nas_scanner.sh
    exit $?
fi

