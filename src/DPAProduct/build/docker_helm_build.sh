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

if [ -z "${LAST_MS_TAG}" ]; then
    echo "LAST_MS_TAG does not exist."
    exit 1
fi

echo LAST_MS_TAG=${LAST_MS_TAG}

function download_patch_images() {

    echo "Downloading patch Images from harbor!"
    sh ${G_BASE_DIR}/build/download_images_from_harbor.sh patch
    if [ $? -ne 0 ]; then
        echo "download patch image from harbor failed"
        exit 1
    else
        echo "download all patch images success"
    fi

}

function download_open_images() {

    echo "Downloading patch Images from harbor!"
    sh ${G_BASE_DIR}/build/download_images_from_harbor.sh OpenSource
    if [ $? -ne 0 ]; then
        echo "download open image from harbor failed"
        exit 1
    else
        echo "download all open images success"
    fi

}

function build_all_helm() {
    rm -rf ${G_BASE_DIR}/pkg/helm
    mkdir -p ${G_BASE_DIR}/pkg/helm
    rm -rf ${G_BASE_DIR}/build/helm/databackup/charts
    mkdir -p ${G_BASE_DIR}/build/helm/databackup/charts

    echo "Build helm $(ls "${G_BASE_DIR}/build/helm/components")"
    find ./ -name "*.yaml" | xargs -I {} sed -i "s/{{ .Values.global.version }}/${LAST_MS_TAG}/g" {}
    for h in $(ls "${G_BASE_DIR}/build/helm/components"); do
        sed -i "s/^version:.*/version: ${INTERNAL_VERSION}/g" "${G_BASE_DIR}/build/helm/components/$h/Chart.yaml"
        sed -i "s/^appVersion:.*/appVersion: ${G_VERSION}/g" "${G_BASE_DIR}/build/helm/components/$h/Chart.yaml"
        sed -i "s/product_version/${LAST_MS_TAG}/g" "${G_BASE_DIR}/build/helm/databackup/values.yaml"
        helm package "${G_BASE_DIR}/build/helm/components/$h" -d "${G_BASE_DIR}/build/helm/databackup/charts"
        if [ $? -ne 0 ]; then
            echo "helm build ${h} failed"
            exit 1
        fi
    done
    sed -i "s/^version:.*/version:  ${INTERNAL_VERSION}/g" "${G_BASE_DIR}/build/helm/databackup/Chart.yaml"
    sed -i "s/^appVersion:.*/appVersion:  ${G_VERSION}/g" "${G_BASE_DIR}/build/helm/databackup/Chart.yaml"
    helm package "${G_BASE_DIR}/build/helm/databackup" -d "${G_BASE_DIR}/pkg/helm"
    echo "Build all helm success"

    try_check_chart_syntax
}

function initialize_inf_pm() {
    if [ ! -f "${G_BASE_DIR}/component/INF_CI/script/commParam.sh" ]; then
        echo "No commParam.sh found, check component dir"
        exit 1
    fi
    sh "${G_BASE_DIR}/component/INF_CI/script/commParam.sh"

    if [ ! -f "${G_BASE_DIR}/component/PM_CI/CI/script/common.sh" ]; then
        echo "No common.sh found, check component dir"
        exit 1
    fi
    sh "${G_BASE_DIR}/component/PM_CI/CI/script/common.sh"

    #替换基础设施yaml文件中的数字版本号
    sed -i "s/current_digital_version/${digitalVersion}/g" ${G_BASE_DIR}/component/INF_CI/build/helm/infrastructure/templates/DigitalVersionConfig.yaml
}

function copy_files() {
    # 复制CI库的文件
    cd ${G_BASE_DIR}/..
    cp -rf ProtectManager/build ${G_BASE_DIR}/component/PM_CI
    cp -rf ProtectManager/CI ${G_BASE_DIR}/component/PM_CI
    # cp -rf ProtectManager/component ${G_BASE_DIR}/component/PM_CI
    cp -rf Infrastructure_OM/ci/* ${G_BASE_DIR}/component/INF_CI

    #拷贝PM_API_Gateway配置文件到ProtectManager的chart/template/目录下
    cp -rf ProtectManager/component/PM_API_Gateway/IngressRoute/*   ${G_BASE_DIR}/component/PM_CI/build/helm/protect-manager/templates/

    # 替换基础设施和管控面文件中的变量
    initialize_inf_pm
    if [ $? -ne 0 ]; then
        echo " INF PM initialization failed!"
        exit 1
    fi

    rm -rf ${G_BASE_DIR}/build/dockerfiles
    mkdir -p ${G_BASE_DIR}/build/dockerfiles
    rm -rf ${G_BASE_DIR}/build/helm/components
    mkdir -p ${G_BASE_DIR}/build/helm/components
    echo "Begin to copy all docker files"

    find "${G_BASE_DIR}/component" \( ! -path "*build-dev*" -a ! -path "*DMA_CI*" \) \( -name "*.dockerfile" -o -name "*.name" \) -exec cp -f "{}" "${G_BASE_DIR}/build/dockerfiles" \;

    rm -f "${G_BASE_DIR}/build/dockerfiles/open-ebackup_base.dockerfile"
    rm -f "${G_BASE_DIR}/build/dockerfiles/ba se.name"

    echo -e "\nAfter copy docker files, ${G_BASE_DIR}/build/dockerfiles contains:"
    ls -l ${G_BASE_DIR}/build/dockerfiles

    echo "Begin to copy all helm files"
    find "${G_BASE_DIR}/component" -maxdepth 4 -type d -iregex '.*helm/.*' -exec cp -rf "{}" "${G_BASE_DIR}/build/helm/components" \;
    echo -e "\nAfter copy helm dirs, ${G_BASE_DIR}/build/helm/components contains:"
    ls -l ${G_BASE_DIR}/build/helm/components
}

function build_open_helm() {
    rm -rf ${G_BASE_DIR}/pkg/helm
    mkdir -p ${G_BASE_DIR}/pkg/helm
    rm -rf ${G_BASE_DIR}/build/helm/databackup/charts
    mkdir -p ${G_BASE_DIR}/build/helm/databackup/charts

    echo "Build helm $(ls "${G_BASE_DIR}/build/helm/components")"

    if [ "${BUILD_MODULE}" == "system_pm" ] ; then
        copy_files
        if [ -d "$DIR_PATH" ]; then
            cp -rf ${G_BASE_DIR}/bak/*.yaml ${G_BASE_DIR}/build/helm/databackup/templates/
        else
            mkdir -p ${G_BASE_DIR}/bak
            cp -rf ${G_BASE_DIR}/build/helm/databackup/templates/*.yaml ${G_BASE_DIR}/bak/
        fi
        cd ${G_BASE_DIR}/build/helm/components
        find ./ -name "*.yaml" | xargs -I {} sed -i "s/{{ .Values.global.version }}/${LAST_MS_TAG}/g" {}
        cp -rf ${G_BASE_DIR}/build/helm/components/protect-engine/conf ${G_BASE_DIR}/build/helm/components/infrastructure/
        rm -rf ${G_BASE_DIR}/build/helm/components/protect-engine
        for h in $(ls "${G_BASE_DIR}/build/helm/components"); do
            sed -i "s/d8/d10/g" "${G_BASE_DIR}/build/helm/components/infrastructure/templates/MultiClusterConf.yaml"
            sed -i "s/^version:.*/version: ${INTERNAL_VERSION}/g" "${G_BASE_DIR}/build/helm/components/$h/Chart.yaml"
            sed -i "s/^appVersion:.*/appVersion: ${G_VERSION}/g" "${G_BASE_DIR}/build/helm/components/$h/Chart.yaml"
            sed -i "s/product_version/${LAST_MS_TAG}/g" "${G_BASE_DIR}/build/helm/databackup/values.yaml"
            find ./ -name "*dee*.yaml" -exec rm -rf {} +
            helm package "${G_BASE_DIR}/build/helm/components/$h" -d "${G_BASE_DIR}/build/helm/databackup/charts"
            if [ $? -ne 0 ]; then
                echo "helm build ${h} failed"
                exit 1
            fi
        done
        echo "helm build ${BUILD_MODULE} success"
    fi
    if [ "${BUILD_MODULE}" == "system_dme" ] ; then
        copy_files
        cd ${G_BASE_DIR}/build/helm/components
        find ./ -name "*.yaml" | xargs -I {} sed -i "s/{{ .Values.global.version }}/${LAST_MS_TAG}/g" {}
        find . -maxdepth 1 -type d ! -name 'protect-engine'  ! -name '.' -exec rm -rf {} +
        #和system_pm的包共用一套pv
        rm -rf ${G_BASE_DIR}/build/helm/databackup/templates/*.yaml
        for h in $(ls "${G_BASE_DIR}/build/helm/components"); do
            sed -i "s/^version:.*/version: ${INTERNAL_VERSION}/g" "${G_BASE_DIR}/build/helm/components/$h/Chart.yaml"
            sed -i "s/^appVersion:.*/appVersion: ${G_VERSION}/g" "${G_BASE_DIR}/build/helm/components/$h/Chart.yaml"
            sed -i "s/product_version/${LAST_MS_TAG}/g" "${G_BASE_DIR}/build/helm/databackup/values.yaml"
            helm package "${G_BASE_DIR}/build/helm/components/$h" -d "${G_BASE_DIR}/build/helm/databackup/charts"
            if [ $? -ne 0 ]; then
                echo "helm build ${h} failed"
                exit 1
            fi
        done
        echo "helm build ${BUILD_MODULE} success"
    fi
    if [ "${BUILD_MODULE}" == "system_dee" ] ; then
        copy_files
        mkdir -p "${G_BASE_DIR}/build/helm/components/dee"
        find ./ -name "*.yaml" | xargs -I {} sed -i "s/{{ .Values.global.version }}/${LAST_MS_TAG}/g" {}
        mkdir -p "${G_BASE_DIR}/build/helm/components/dee/templates"
        cp ${G_BASE_DIR}/build/helm/components/protect-engine/templates/*dee*.yaml ${G_BASE_DIR}/build/helm/components/dee/templates
        cp ${G_BASE_DIR}/build/helm/components/protect-engine/Chart.yaml ${G_BASE_DIR}/build/helm/components/dee
        cp ${G_BASE_DIR}/build/helm/components/protect-engine/values.yaml ${G_BASE_DIR}/build/helm/components/dee
        rm -rf ${G_BASE_DIR}/build/helm/components/infrastructure
        rm -rf ${G_BASE_DIR}/build/helm/components/protect-manager
        rm -rf ${G_BASE_DIR}/build/helm/components/protect-engine
        rm -rf ${G_BASE_DIR}/build/helm/databackup/templates/*.yaml
        for h in $(ls "${G_BASE_DIR}/build/helm/components"); do
            sed -i "s/^version:.*/version: ${INTERNAL_VERSION}/g" "${G_BASE_DIR}/build/helm/components/$h/Chart.yaml"
            sed -i "s/^appVersion:.*/appVersion: ${G_VERSION}/g" "${G_BASE_DIR}/build/helm/components/$h/Chart.yaml"
            sed -i "s/product_version/${LAST_MS_TAG}/g" "${G_BASE_DIR}/build/helm/databackup/values.yaml"
            helm package "${G_BASE_DIR}/build/helm/components/$h" -d "${G_BASE_DIR}/build/helm/databackup/charts"
            if [ $? -ne 0 ]; then
                echo "helm build ${h} failed"
                exit 1
            fi
        done
        echo "helm build ${BUILD_MODULE} success"
    fi
    sed -i "s/^version:.*/version:  ${INTERNAL_VERSION}/g" "${G_BASE_DIR}/build/helm/databackup/Chart.yaml"
    sed -i "s/^appVersion:.*/appVersion:  ${G_VERSION}/g" "${G_BASE_DIR}/build/helm/databackup/Chart.yaml"
    helm package "${G_BASE_DIR}/build/helm/databackup" -d "${G_BASE_DIR}/pkg/helm"
    echo "Build all helm success"

    try_check_chart_syntax
}

function try_check_chart_syntax() {
    echo "Try to use helm template to check syntax issue."
    helm template ${G_BASE_DIR}/pkg/helm/databackup-${INTERNAL_VERSION}.tgz --set global.deploy_type=dataprotect --set global.productModel=D5600V6_X
    if [ $? -ne 0 ]; then
        echo "ERROR: Check helm template syntax failed!"
        exit 1
    else
        echo "helm template success."
    fi
}

function build_patch_helm() {
    rm -rf ${G_BASE_DIR}/pkg/helm
    mkdir -p ${G_BASE_DIR}/pkg/helm
    rm -rf ${G_BASE_DIR}/build/helm/databackup/charts
    mkdir -p ${G_BASE_DIR}/build/helm/databackup/charts

    echo "Build helm $(ls "${G_BASE_DIR}/build/helm/components")"
    for h in $(ls "${G_BASE_DIR}/build/helm/components"); do
        sed -i "s/^version:.*/version: ${INTERNAL_VERSION}/g" "${G_BASE_DIR}/build/helm/components/$h/Chart.yaml"
        sed -i "s/^appVersion:.*/appVersion: ${G_VERSION}/g" "${G_BASE_DIR}/build/helm/components/$h/Chart.yaml"
        sed -i "s/current_version/${patch_image_version}/g" "${G_BASE_DIR}/build/helm/components/$h/values.yaml"
        helm package "${G_BASE_DIR}/build/helm/components/$h" -d "${G_BASE_DIR}/build/helm/databackup/charts"
        if [ $? -ne 0 ]; then
            echo "helm build ${h} failed"
            exit 1
        fi
    done
    #补丁包values.yaml文件中的product_version为环境初始镜像tag号
    sed -i "s/product_version/${initial_image_version}/g" "${G_BASE_DIR}/build/helm/databackup/values.yaml"
    #补丁包Chart.yaml文件中的version为C版本号，appVersion为补丁版本号
    sed -i "s/^version:.*/version: ${INTERNAL_VERSION}/g" "${G_BASE_DIR}/build/helm/databackup/Chart.yaml"
    sed -i "s/^appVersion:.*/appVersion: ${PATCH_VERSION}/g" "${G_BASE_DIR}/build/helm/databackup/Chart.yaml"
    helm package "${G_BASE_DIR}/build/helm/databackup" -d "${G_BASE_DIR}/pkg/helm"
    echo "Build all helm success"

    try_check_chart_syntax
}

function saveall_docker() {
    PKG_VERSION=$1
    if [ "${PKG_VERSION}" == "" ]; then
        echo "please confirm PKG_VERSION"
        exit 1
    fi

    local arch_type=$(uname -m)
    if [ "$arch_type" == "aarch64" ]; then
        ARCH="ARM_64"
    else
        ARCH="X86_64"
    fi

    rm -rf ${G_BASE_DIR}/pkg/images
    mkdir ${G_BASE_DIR}/pkg/images

    docker images | grep -v rancher | grep -v '<none>' |  grep -v 'oceanprotect-dataprotect' | grep -v 'harbor' | grep -v 'dme_3rd' | grep -v "dee_common" | grep -v "pm-app-common" | grep "${LAST_MS_TAG}"
    docker images | sed 1d | grep -v rancher | grep -v '<none>' |  grep -v 'oceanprotect-dataprotect' | grep -v 'harbor' | grep -v 'dme_3rd' | grep -v "dee_common" | grep -v "pm-app-common" | grep "${LAST_MS_TAG}" | tr -s ' ' | awk '{printf "%s:%s ",$1,$2}'



    if [ -f /usr/bin/xz ]; then
        echo "Run docker save to ${G_BASE_DIR}/pkg/images/${PRODUCT}_${PKG_VERSION}_image_${ARCH}.tar.xz"
        echo "Using xz to zip. It will be slow maybe, but high compress ratio."
        echo "xz -9e -T0 ${G_BASE_DIR}/pkg/images/${PRODUCT}_${PKG_VERSION}_image_${ARCH}.tar"
        echo "Please wait, this gonna take some time(10min+) for compressing..."
        docker save $(docker images | sed 1d | grep -v rancher | grep -v '<none>' |  grep -v 'oceanprotect-dataprotect' | grep -v 'harbor' | grep -v 'dme_3rd' | grep -v "dee_common" | grep -v "pm-app-common" | tr -s ' ' | awk '{printf "%s:%s ",$1,$2}') | xz -9e -T0 > ${G_BASE_DIR}/pkg/images/${PRODUCT}_${PKG_VERSION}_image_${ARCH}.tar.xz
        if [ $? -ne 0 ]; then
            echo "docker save xz failed"
            exit 1
        fi
    elif [ -f /usr/bin/pigz ]; then
        echo "Run docker save to ${G_BASE_DIR}/pkg/images/${PRODUCT}_${PKG_VERSION}_image_${ARCH}.tar"
        docker save -o ${G_BASE_DIR}/pkg/images/${PRODUCT}_${PKG_VERSION}_image_${ARCH}.tar $(docker images | sed 1d | grep -v rancher | grep -v '<none>' |  grep -v 'oceanprotect-dataprotect' | grep -v 'harbor' | grep -v 'dme_3rd' | grep -v "dee_common" | grep -v "pm-app-common" | tr -s ' ' | awk '{printf "%s:%s ",$1,$2}')
        if [ $? -ne 0 ]; then
            echo "docker save failed"
            exit 1
        fi
        du -sh ${G_BASE_DIR}/pkg/images/${PRODUCT}_${PKG_VERSION}_image_${ARCH}.tar
        echo "Using pigz to zip"
        pigz -6 -p 12 ${G_BASE_DIR}/pkg/images/${PRODUCT}_${PKG_VERSION}_image_${ARCH}.tar
        if [ $? -ne 0 ]; then
            echo "docker save pigz failed"
            exit 1
        fi
    else
        echo "Run docker save to ${G_BASE_DIR}/pkg/images/${PRODUCT}_${PKG_VERSION}_image_${ARCH}.tar"
        docker save -o ${G_BASE_DIR}/pkg/images/${PRODUCT}_${PKG_VERSION}_image_${ARCH}.tar $(docker images | sed 1d | grep -v rancher | grep -v '<none>' |  grep -v 'oceanprotect-dataprotect' | grep -v 'harbor' | grep -v 'dme_3rd' | grep -v "dee_common" | grep -v "pm-app-common" | tr -s ' ' | awk '{printf "%s:%s ",$1,$2}')
        if [ $? -ne 0 ]; then
            echo "docker save failed"
            exit 1
        fi
        du -sh ${G_BASE_DIR}/pkg/images/${PRODUCT}_${PKG_VERSION}_image_${ARCH}.tar
        echo "Using gzip to zip, it's very slow!!!"
        gzip ${G_BASE_DIR}/pkg/images/${PRODUCT}_${PKG_VERSION}_image_${ARCH}.tar
        if [ $? -ne 0 ]; then
            echo "docker save gzip failed"
            exit 1
        fi
    fi

    echo "docker save success."
}

function save_open_docker() {
    PKG_VERSION=$1
    if [ "${PKG_VERSION}" == "" ]; then
        echo "please confirm PKG_VERSION"
        exit 1
    fi

    local arch_type=$(uname -m)
    if [ "$arch_type" == "aarch64" ]; then
        ARCH="ARM_64"
    else
        ARCH="X86_64"
    fi

    rm -rf ${G_BASE_DIR}/pkg/images
    mkdir ${G_BASE_DIR}/pkg/images


    if [ "${BUILD_MODULE}" == "system_dee" ];then
      docker images | grep -v rancher | grep -v '<none>' |  grep -v 'oceanprotect-dataprotect' | grep -v 'harbor' | grep -v 'om ' | grep -v 'dme_3rd' | grep -v "dee_common" | grep -v "pm-app-common" | grep "${LAST_MS_TAG}"
      docker images | sed 1d | grep -v rancher | grep -v '<none>' |  grep -v 'oceanprotect-dataprotect' | grep -v 'harbor' | grep -v 'om ' | grep -v 'dme_3rd' | grep -v "dee_common" | grep -v "pm-app-common" | grep "${LAST_MS_TAG}" | tr -s ' ' | awk '{printf "%s:%s ",$1,$2}'
    elif [ "${BUILD_MODULE}" == "system_dme" ];then
      docker images | grep -v rancher | grep -v '<none>' |  grep -v 'oceanprotect-dataprotect' | grep -v 'harbor' | grep -v 'om ' | grep -v 'dme_3rd' | grep -v "dee_common" | grep -v "pm-app-common" | grep "${LAST_MS_TAG}"
      docker images | sed 1d | grep -v rancher | grep -v '<none>' |  grep -v 'oceanprotect-dataprotect' | grep -v 'harbor' | grep -v 'om ' | grep -v 'dme_3rd' | grep -v "dee_common" | grep -v "pm-app-common" | grep "${LAST_MS_TAG}" | tr -s ' ' | awk '{printf "%s:%s ",$1,$2}'
    elif [ "${BUILD_MODULE}" == "system_pm" ];then
      docker images | grep -v rancher | grep -v '<none>' |  grep -v 'oceanprotect-dataprotect' | grep -v 'harbor' | grep -v 'dme_3rd' | grep -v "dee_common" | grep -v "pm-app-common" | grep "${LAST_MS_TAG}"
      docker images | sed 1d | grep -v rancher | grep -v '<none>' |  grep -v 'oceanprotect-dataprotect' | grep -v 'harbor' | grep -v 'dme_3rd' | grep -v "dee_common" | grep -v "pm-app-common" | grep "${LAST_MS_TAG}" | tr -s ' ' | awk '{printf "%s:%s ",$1,$2}'
    elif [ "${BUILD_MODULE}" == "system_agent" ];then
      echo "agent none docker"
    fi

    if [ -f /usr/bin/xz ]; then
        echo "Run docker save to ${G_BASE_DIR}/pkg/images/${PRODUCT}_${PKG_VERSION}_${PKG_NAME}.tar.xz"
        echo "Using xz to zip. It will be slow maybe, but high compress ratio."
        echo "xz -9e -T0 ${G_BASE_DIR}/pkg/images/${PRODUCT}_${PKG_VERSION}_${PKG_NAME}.tar"
        echo "Please wait, this gonna take some time(10min+) for compressing..."
        echo "BUILD_MODULE:${BUILD_MODULE}"
        if [ "${BUILD_MODULE}" == "system_dee" ];then
          dee_list="dee_base_parser:${MS_IMAGE_TAG} dee_indexer:${MS_IMAGE_TAG} dee_anti_ransomware:${MS_IMAGE_TAG} dee_db_anonymization:${MS_IMAGE_TAG} dee_global_search:${MS_IMAGE_TAG} dee_initcontainer:${MS_IMAGE_TAG}"
          docker save ${dee_list} | xz -9e -T0 > ${G_BASE_DIR}/pkg/images/${PRODUCT}_${PKG_VERSION}_${PKG_NAME}.tar.xz
          if [ $? -ne 0 ]; then
              echo "docker save xz failed"
              exit 1
          fi
        elif [ "${BUILD_MODULE}" == "system_dme" ];then
          dme_list="dme_vmware:${MS_IMAGE_TAG} dme_unifiedbackupcontroller:${MS_IMAGE_TAG} dme_replication:${MS_IMAGE_TAG} dme_openstorageapi:${MS_IMAGE_TAG} dme_jobmanager:${MS_IMAGE_TAG} dme_openstorageapi_data:${MS_IMAGE_TAG} dme_openstorageapi_controller:${MS_IMAGE_TAG} dme_nginx:${MS_IMAGE_TAG} dme_dns:${MS_IMAGE_TAG} dme_initcontainer:${MS_IMAGE_TAG} dme_dmc:${MS_IMAGE_TAG} dme_archive:${MS_IMAGE_TAG} dmc_nginx:${MS_IMAGE_TAG} dma_nginx:${MS_IMAGE_TAG}"
          dme_list="${dme_list} dme-openstorageapi-csi-snapshotter:${MS_IMAGE_TAG} dme-openstorageapi-csi-provisioner:${MS_IMAGE_TAG} dme-openstorageapi-csi-node-driver-registrar:${MS_IMAGE_TAG} dme-openstorageapi-snapshot-controller:${MS_IMAGE_TAG} dme-openstorageapi-csi-attacher:${MS_IMAGE_TAG} dme-openstorageapi-oceanprotect-csi:${MS_IMAGE_TAG}"
          docker save ${dme_list} | xz -9e -T0 > ${G_BASE_DIR}/pkg/images/${PRODUCT}_${PKG_VERSION}_${PKG_NAME}.tar.xz
          if [ $? -ne 0 ]; then
              echo "docker save xz failed"
              exit 1
          fi
        elif [ "${BUILD_MODULE}" == "system_pm" ];then
          pm_list="pm-database-version-migration:${MS_IMAGE_TAG} pm-protection-service:${MS_IMAGE_TAG} pm-gui:${MS_IMAGE_TAG} pm-nginx:${MS_IMAGE_TAG} pm-system-base:${MS_IMAGE_TAG} om:${MS_IMAGE_TAG} sftp:${MS_IMAGE_TAG} gaussdb:${MS_IMAGE_TAG} kafka:${MS_IMAGE_TAG} elasticsearch:${MS_IMAGE_TAG} redis:${MS_IMAGE_TAG} zookeeper:${MS_IMAGE_TAG} pm-config:${MS_IMAGE_TAG}"
          docker save ${pm_list} | xz -9e -T0 > ${G_BASE_DIR}/pkg/images/${PRODUCT}_${PKG_VERSION}_${PKG_NAME}.tar.xz
          if [ $? -ne 0 ]; then
              echo "docker save xz failed"
              exit 1
          fi
        elif [ "${BUILD_MODULE}" == "system_agent" ];then
          echo "agent none docker"
        fi

    elif [ -f /usr/bin/pigz ]; then
        echo "Run docker save to ${G_BASE_DIR}/pkg/images/${PRODUCT}_${PKG_VERSION}_image_${ARCH}.tar"
        docker save -o ${G_BASE_DIR}/pkg/images/${PRODUCT}_${PKG_VERSION}_image_${ARCH}.tar $(docker images | sed 1d | grep -v rancher | grep -v '<none>' |  grep -v 'oceanprotect-dataprotect' | grep -v 'harbor' | grep -v 'dme_3rd' | grep -v "dee_common" | grep -v "pm-app-common" | tr -s ' ' | awk '{printf "%s:%s ",$1,$2}')
        if [ $? -ne 0 ]; then
            echo "docker save failed"
            exit 1
        fi
        du -sh ${G_BASE_DIR}/pkg/images/${PRODUCT}_${PKG_VERSION}_image_${ARCH}.tar
        echo "Using pigz to zip"
        pigz -6 -p 12 ${G_BASE_DIR}/pkg/images/${PRODUCT}_${PKG_VERSION}_image_${ARCH}.tar
        if [ $? -ne 0 ]; then
            echo "docker save pigz failed"
            exit 1
        fi
    else
        echo "Run docker save to ${G_BASE_DIR}/pkg/images/${PRODUCT}_${PKG_VERSION}_image_${ARCH}.tar"
        docker save -o ${G_BASE_DIR}/pkg/images/${PRODUCT}_${PKG_VERSION}_image_${ARCH}.tar $(docker images | sed 1d | grep -v rancher | grep -v '<none>' |  grep -v 'oceanprotect-dataprotect' | grep -v 'harbor' | grep -v 'dme_3rd' | grep -v "dee_common" | grep -v "pm-app-common" | tr -s ' ' | awk '{printf "%s:%s ",$1,$2}')
        if [ $? -ne 0 ]; then
            echo "docker save failed"
            exit 1
        fi
        du -sh ${G_BASE_DIR}/pkg/images/${PRODUCT}_${PKG_VERSION}_image_${ARCH}.tar
        echo "Using gzip to zip, it's very slow!!!"
        gzip ${G_BASE_DIR}/pkg/images/${PRODUCT}_${PKG_VERSION}_image_${ARCH}.tar
        if [ $? -ne 0 ]; then
            echo "docker save gzip failed"
            exit 1
        fi
    fi

    echo "docker save success."
}

function dispatchTask() {
    if [ "$1" == "OpenSource" ]; then
        build_open_helm
        save_open_docker "${G_VERSION}"
    else
        echo "Invalid params"
    fi
}

function main() {
    dispatchTask "$@"
    return $?
}

main "$@"
exit $?