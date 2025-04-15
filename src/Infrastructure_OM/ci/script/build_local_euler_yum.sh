#!/bin/bash
########################################
#  This file is part of the open-eBackup project.
# Copyright (c) 2024 Huawei Technologies Co.,Ltd.
#
# open-eBackup is licensed under MPL v2.
# You can use this software according to the terms and conditions of the MPL v2.
# You may obtain a copy of MPL v2 at:
#
#          https://www.mozilla.org/en-US/MPL/2.0
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the MPL v2 for more details.
########################################

PACKAGE1="EulerOS_Server_V200R012C00SPC500B750.tar.gz"
PACKAGE2="devel_tools.tar.gz"
PACKAGE5="EulerOS-Virtual-V200R012C00SPC502B016-aarch64.tar.gz"
PACKAGE6="Euler_compile_RPMS-aarch64.tar.gz"

CURRENT_PATH=$(cd `dirname $0`; pwd)
CMC_CONF_PATH=${CURRENT_PATH}/../conf/
BASE_PATH=${CURRENT_PATH}/../


upload_artifact()
{
    cd ${CMC_CONF_PATH}
    artget push -d yum_dependency_cmc.xml -ap ${BASE_PATH}/pkg/package -user ${cmc_user} -pwd ${cmc_pwd}
    if [ $? -ne 0 ]; then
        echo "Upload artifact to cmc error"
        exit 1
    fi
}

creat_local_repo()
{
    mkdir -p package
    cd package
    find "${BASE_PATH}/pkg" -name "*.rpm" -exec cp -rf "{}" "${BASE_PATH}/pkg/package" \;
    createrepo ${BASE_PATH}/pkg/package
}

ungzip_package()
{
    cd ${BASE_PATH}/pkg/
    tar xzf ${PACKAGE1}
    tar xzf ${PACKAGE2}
    tar xzf ${PACKAGE5}
    tar xzf ${PACKAGE6}
    rm -rf ${PACKAGE1} ${PACKAGE2} ${PACKAGE5} ${PACKAGE6}
}

download_artifact()
{
    mkdir -p ${BASE_PATH}/pkg/
    cd ${CMC_CONF_PATH}
    artget pull -d yum_dependency_cmc.xml -p "{'PACKAGE1':'${PACKAGE1}', 'PACKAGE2':'${PACKAGE2}','PACKAGE5':'${PACKAGE5}','PACKAGE6':'${PACKAGE6}'}" -ap ${BASE_PATH}/pkg -user ${cmc_user} -pwd ${cmc_pwd}
    if [ $? -ne 0 ]; then
        echo "Download artifact from cmc error"
        exit 1
    fi
}

main()
{
    download_artifact
    if [ $? -ne 0 ]; then
        exit 1
    fi

    ungzip_package
    if [ $? -ne 0 ]; then
        exit 1
    fi

    creat_local_repo
    if [ $? -ne 0 ]; then
        exit 1
    fi

    upload_artifact
    if [ $? -ne 0 ]; then
        exit 1
    fi
    
}

main