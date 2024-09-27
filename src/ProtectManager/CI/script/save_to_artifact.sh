#!/bin/bash

function initEnv() {
    BASE_PATH="$(
        cd "$(dirname "$BASH_SOURCE")/../../"
        pwd
    )"

    local arch_type=$(uname -m)
    if [ "$arch_type" == "aarch64" ]; then
        ARCH="euler-arm"
    else
        ARCH="euler-x86"
    fi

    CI_CONFIG_PATH=${BASE_PATH}/CI/LCRP/conf/
    SOURCE_PKG_PATH=${BASE_PATH}/pkg
}

function copy_pkgs() {
    mkdir -p ${BASE_PATH}/pkg/mspkg
    local L_COMPONENTS_DIR="${BASE_PATH}/component"
    for DIR_NAME in $(ls ${L_COMPONENTS_DIR}); do
        if [ -d ${L_COMPONENTS_DIR}/${DIR_NAME}/pkg ]; then
            echo "Copy pkgs from ${L_COMPONENTS_DIR}/${DIR_NAME}/pkg to ${BASE_PATH}/pkg/mspkg, pkg:"
            ls -l "${L_COMPONENTS_DIR}/${DIR_NAME}/pkg"
            cp -f "${L_COMPONENTS_DIR}/${DIR_NAME}/pkg/"*.tar.gz "${BASE_PATH}/pkg/mspkg"
        fi
    done
	
	echo "copy version file to ${BASE_PATH}/pkg/mspkg/"
	cp ${BASE_PATH}/PM_version ${BASE_PATH}/pkg/mspkg/

    echo -e "\nAfter copy pkg, ${BASE_PATH}/pkg/mspkg contains:"
    ls -l ${BASE_PATH}/pkg/mspkg
    ls -l ${BASE_PATH}/pkg/image
}

function savems_docker() {
    Service_Name=$1
    NAME=$(ls "${BASE_PATH}/component/${Service_Name}/pkg")
    L_MS_NAME=$(basename $NAME .tar.gz)
    L_TAG=$(cat "${BASE_PATH}/build/dockerfiles/$L_MS_NAME.name")

    echo "Run docker save to ${BASE_PATH}/pkg/images/${L_MS_NAME}_docker.tar.gz"
    docker save -o ${BASE_PATH}/pkg/image/${L_MS_NAME}_docker.tar ${L_TAG}
    if [ $? -ne 0 ]; then
        echo "${L_MS_NAME} docker save failed"
        exit 1
    fi

    if [ -f /usr/bin/pigz ]; then
        echo "Using pigz to zip"
        pigz -6 -p 12 ${BASE_PATH}/pkg/image/${L_MS_NAME}_docker.tar
        if [ $? -ne 0 ]; then
            echo "${L_MS_NAME} docker save pigz failed"
            exit 1
        fi
    else
        echo "Using gzip to zip, it's very slow!!!"
        gzip ${BASE_PATH}/pkg/image/${L_MS_NAME}_docker.tar
        if [ $? -ne 0 ]; then
            echo "${L_MS_NAME} docker save gzip failed"
            exit 1
        fi
    fi

    echo "${L_MS_NAME} docker save success."
}

function saveall_docker() {
    echo "Run docker save to ${BASE_PATH}/pkg/images/ProtectManager_docker.tar.gz"
    docker save -o ${BASE_PATH}/pkg/image/ProtectManager_docker.tar $(docker images | sed 1d | grep -v rancher | grep -v '<none>' | grep -v 'oceanprotect-dataprotect' | grep -v harbor | grep "pm-" | tr -s ' ' | awk '{printf "%s:%s ",$1,$2}')
    if [ $? -ne 0 ]; then
        echo "docker save failed"
        exit 1
    fi

    if [ -f /usr/bin/pigz ]; then
        echo "Using pigz to zip"
        pigz -6 -p 12 ${BASE_PATH}/pkg/image/ProtectManager_docker.tar
        if [ $? -ne 0 ]; then
            echo "docker save pigz failed"
            exit 1
        fi
    else
        echo "Using gzip to zip, it's very slow!!!"
        gzip ${BASE_PATH}/pkg/image/ProtectManager_docker.tar
        if [ $? -ne 0 ]; then
            echo "docker save gzip failed"
            exit 1
        fi
    fi

    echo "docker save success."
  
}

function save_docker() {
    rm -rf ${BASE_PATH}/pkg/image
    mkdir ${BASE_PATH}/pkg/image
    L_MS_NAME=$1
    if [ "${L_MS_NAME}" == "" ]; then
        echo "save all pm microservice images"
        saveall_docker
        if [ $? -ne 0 ]; then
            echo "docker save gzip failed"
            exit 1
        fi
    else
        echo "save ${L_MS_NAME} docker image"
        savems_docker ${L_MS_NAME}
        if [ $? -ne 0 ]; then
            echo "${L_MS_NAME} docker save gzip failed"
            exit 1
        fi

    fi

}

echo "#########################################################"
echo "   Starting save artifact"
echo "#########################################################"

initEnv
if [[ "${Compile_image}" == "Y" ]]; then
    save_docker $@
    if [ $? -ne 0 ]; then
        echo "docker save failed!"
    fi
fi
copy_pkgs

echo "#########################################################"
echo "   Save artifact Success"
echo "#########################################################"
