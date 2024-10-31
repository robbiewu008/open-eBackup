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
set -x
source "$(dirname "$BASH_SOURCE")/buildcommon.sh"
G_CURRENT_PATH=$(cd `dirname $0`; pwd)
echo MS_IMAGE_TAG=${MS_IMAGE_TAG}
echo tag_image=${tag_image}
current_user=`whoami`
function package_final() {
    PKG_VERSION=$1
    if [ "${PKG_VERSION}" == "" ]; then
        echo "please confirm PKG_VERSION"
        exit 1
    fi
    cur_time=$(date "+%Y%m%d%H%M%S")
    rm -rf "${G_BASE_DIR}/pkg/final"
    mkdir -p "${G_BASE_DIR}/pkg/final/${PRODUCT}_${PKG_VERSION}_${PKG_NAME}"
    if [ "${BUILD_PKG_TYPE}" == "OpenSource" ]; then
        mkdir -p "${G_BASE_DIR}/pkg/open-eBackup/final"
        sed -i "s/package_name/${PKG_NAME}/g" $G_CURRENT_PATH/../CI/conf/image_manifest_${PKG_NAME}.yml
        sed -i "s/current_version/${PKG_VERSION}/g" $G_CURRENT_PATH/../CI/conf/image_manifest_${PKG_NAME}.yml
        sed -i "s/^ReleaseTime:.*/ReleaseTime:  ${cur_time}/g" $G_CURRENT_PATH/../CI/conf/image_manifest_${PKG_NAME}.yml
        sed -i "s/tag:.*/tag: ${LAST_MS_TAG}/g" $G_CURRENT_PATH/../CI/conf/image_manifest_${PKG_NAME}.yml
        sed -i "s/^DigitalVersion:.*/DigitalVersion:  ${digitalVersion}/g" $G_CURRENT_PATH/../CI/conf/image_manifest_${PKG_NAME}.yml
        cp -f $G_CURRENT_PATH/../CI/conf/image_manifest_${PKG_NAME}.yml "${G_BASE_DIR}/pkg/final/${PRODUCT}_${PKG_VERSION}_${PKG_NAME}/manifest.yml"
    else
        mkdir -p "${G_BASE_DIR}/pkg/DataProtect/final"
        sed -i "s/current_version/${PKG_VERSION}/g" $G_CURRENT_PATH/../CI/conf/image_manifest.yml
        sed -i "s/^ReleaseTime:.*/ReleaseTime:  ${cur_time}/g" $G_CURRENT_PATH/../CI/conf/image_manifest.yml
        sed -i "s/tag:.*/tag: ${LAST_MS_TAG}/g" $G_CURRENT_PATH/../CI/conf/image_manifest.yml
        sed -i "s/^DigitalVersion:.*/DigitalVersion:  ${digitalVersion}/g" $G_CURRENT_PATH/../CI/conf/image_manifest.yml
        cp -f $G_CURRENT_PATH/../CI/conf/image_manifest.yml "${G_BASE_DIR}/pkg/final/${PRODUCT}_${PKG_VERSION}_${PKG_NAME}/manifest.yml"
    fi
    cp -f $G_CURRENT_PATH/../CI/script/image_appctl "${G_BASE_DIR}/pkg/final/${PRODUCT}_${PKG_VERSION}_${PKG_NAME}/appctl"
    chmod 750 "${G_BASE_DIR}/pkg/final/${PRODUCT}_${PKG_VERSION}_${PKG_NAME}/appctl"
    mv ${G_BASE_DIR}/pkg/images/* "${G_BASE_DIR}/pkg/final/${PRODUCT}_${PKG_VERSION}_${PKG_NAME}/"
    if [ $? -ne 0 ]; then
        echo "Copy images error"
        exit 1
    fi
    chown_to_root ${G_BASE_DIR}/pkg/final/${PRODUCT}_${PKG_VERSION}_${PKG_NAME}/
    sudo chmod 777 ${G_BASE_DIR}/pkg/final/${PRODUCT}_${PKG_VERSION}_${PKG_NAME}/

    mkdir -p "${G_BASE_DIR}/pkg/final/${PRODUCT}_${PKG_NAME}_chart"
    sed -i "s/^Version:.*/Version:  ${PKG_VERSION}/g" $G_CURRENT_PATH/../CI/conf/chart_manifest.yml
    sed -i "s/^ReleaseTime:.*/ReleaseTime:  ${cur_time}/g" $G_CURRENT_PATH/../CI/conf/chart_manifest.yml
    sed -i "s/^DigitalVersion:.*/DigitalVersion:  ${digitalVersion}/g" $G_CURRENT_PATH/../CI/conf/chart_manifest.yml
    cp -f $G_CURRENT_PATH/../CI/conf/chart_manifest.yml "${G_BASE_DIR}/pkg/final/${PRODUCT}_${PKG_NAME}_chart/manifest.yml"
    #替换app_upg.yml版本号和镜像tag号
    mkdir -p "${G_BASE_DIR}/pkg/final/${PRODUCT}_${PKG_NAME}_chart/swm_script"
    mkdir -p "${G_BASE_DIR}/pkg/final/${PRODUCT}_${PKG_NAME}_chart/databackup_script"
    sed -i "s/current_version/${PKG_VERSION}/g"  ${G_BASE_DIR}/../Infrastructure_OM/Third_tool/swm_script/app_upg.yml
    sed -i "s/current_image_version/${MS_IMAGE_TAG}/g"  ${G_BASE_DIR}/../Infrastructure_OM/Third_tool/swm_script/app_upg.yml
    rm -rf ${G_BASE_DIR}/../Infrastructure_OM/Third_tool/swm_script/tests
    #swm_script升级脚本赋权
    cd ${G_BASE_DIR}/../Infrastructure_OM/Third_tool/
    chmod 500 swm_script -R
    chmod 400 swm_script/app_upg.yml
    chmod 500 databackup_script -R
    cp -rf ${G_BASE_DIR}/../Infrastructure_OM/Third_tool/swm_script/*  "${G_BASE_DIR}/pkg/final/${PRODUCT}_${PKG_NAME}_chart/swm_script/"
    cp -rf ${G_BASE_DIR}/../Infrastructure_OM/infrastructure/script/databackup/* "${G_BASE_DIR}/pkg/final/${PRODUCT}_${PKG_NAME}_chart/databackup_script/"

    mv ${G_BASE_DIR}/pkg/helm/* "${G_BASE_DIR}/pkg/final/${PRODUCT}_${PKG_NAME}_chart/"
    if [ $? -ne 0 ]; then
        echo "Copy charts error"
        exit 1
    fi
    chown_to_root ${G_BASE_DIR}/pkg/final/${PRODUCT}_${PKG_NAME}_chart/
    sudo chmod 777 ${G_BASE_DIR}/pkg/final/${PRODUCT}_${PKG_NAME}_chart/


    cd ${G_BASE_DIR}/pkg/final/${PRODUCT}_${PKG_VERSION}_${PKG_NAME}
    if [ "${tag_image}" == "debug" ]; then
        sudo tar cf - ./* | pigz -p 12 > ${G_BASE_DIR}/pkg/${PKG_PATH_NAME}/final/${PRODUCT}_${PKG_VERSION}_${PKG_NAME}_debug.tgz
    else
        sudo tar cf - ./* | pigz -p 12 > ${G_BASE_DIR}/pkg/${PKG_PATH_NAME}/final/${PRODUCT}_${PKG_VERSION}_${PKG_NAME}.tgz
    fi

    cd ${G_BASE_DIR}/pkg/final/${PRODUCT}_${PKG_NAME}_chart

    sudo tar cf - ./* | pigz -p 12 > ${G_BASE_DIR}/pkg/${PKG_PATH_NAME}/final/${PRODUCT}_${PKG_NAME}_chart.tgz
    if [ $? -ne 0 ]; then
        echo "Tar final package error"
        exit 1
    fi
    sudo chmod o+rx -R ${G_BASE_DIR}/pkg/${PKG_PATH_NAME}/final
    if [ "${BUILD_TYPE}" == "release" ];then
        if [ "${PRODUCT_CODE_LAST}" != "" ];then
          mkdir -p ${G_BASE_DIR}/pkg/${PKG_PATH_NAME}/final/${PRODUCT_CODE_LAST}
          sudo cp ${G_BASE_DIR}/pkg/${PKG_PATH_NAME}/final/*.tgz* ${G_BASE_DIR}/pkg/${PKG_PATH_NAME}/final/${PRODUCT_CODE_LAST}
        fi
    fi
    sudo chmod o+rx -R ${G_BASE_DIR}/pkg/${PKG_PATH_NAME}/final
    # update PBI_NAME
    cd ${G_BASE_DIR}/pkg/${PKG_PATH_NAME}/final
    if [ "${tag_image}" == "debug" ]; then
        sudo tar cf - ./*${PKG_NAME}* | pigz -p 12 > ${G_BASE_DIR}/package/${PRODUCT}_${PKG_VERSION}_${PKG_NAME}_debug.tgz
    else
        sudo tar cf - ./*${PKG_NAME}* | pigz -p 12 > ${G_BASE_DIR}/package/${PRODUCT}_${PKG_VERSION}_${PKG_NAME}.tgz
    fi
    echo "Package final pkg finish"
    cd ${G_BASE_DIR}/package
    zip -r open-ebackup-1.0.zip  ./*.tgz
    echo "The address where the package exists is：${G_BASE_DIR}/package/ "
    return 0
}

function chown_to_root(){
        sudo chown root:root $1
        if [ $# -eq 0 ];then
                exit 1
        fi
        if [ `sudo find $1 |awk 'NR>1'|wc -l` -eq 0 ];then
                return
        fi
        sudo find $1 |awk 'NR>1' |while read filename
        do
                file_user=`sudo ls -ld ${filename}|awk -F " " '{print $3}'`
                if [ "${file_user}"x == "${current_user}"x ];then
                        sudo chown root:root ${filename}
                fi
                if [ -d ${filename} ];then
                        chown_to_root ${filename}
                fi
        done
}

function main() {
    if [ "$1" == "patch" ]; then
        package_final ${PATCH_VERSION}
        if [ $? -ne 0 ]; then
            echo "pack final patch package error"
            exit 1
        fi
    else
        package_final ${G_VERSION}
        if [ $? -ne 0 ]; then
            echo "pack final package error"
            exit 1
        fi
    fi
}

main $@
exit $?
