#!/bin/bash
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

function build_open_helm() {
    rm -rf ${G_BASE_DIR}/pkg/helm
    mkdir -p ${G_BASE_DIR}/pkg/helm
    rm -rf ${G_BASE_DIR}/build/helm/databackup/charts
    mkdir -p ${G_BASE_DIR}/build/helm/databackup/charts

    echo "Build helm $(ls "${G_BASE_DIR}/build/helm/components")"
    find ./ -name "*.yaml" | xargs -I {} sed -i "s/{{ .Values.global.version }}/${LAST_MS_TAG}/g" {}

    if [ "${BUILD_MODULE}" == "system_pm" ] ; then
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
        rm -rf ${G_BASE_DIR}/build/helm/components/infrastructure
        rm -rf ${G_BASE_DIR}/build/helm/components/protect-manager
        rm -rf ${G_BASE_DIR}/build/helm/databackup/templates/*.yaml
        for h in $(ls "${G_BASE_DIR}/build/helm/components"); do
            find ./ -name "*dee*.yaml" -exec rm -rf {} +
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
        mkdir -p "${G_BASE_DIR}/build/helm/components/dee"
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
        save_open_docker ${G_VERSION}
    else
        echo "Invalid params"
    fi
}

function main() {
    dispatchTask $@
    return $?
}

main $@
exit $?

