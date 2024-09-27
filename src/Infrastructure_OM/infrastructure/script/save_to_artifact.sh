#!/bin/bash

CURRENT_PATH=$(cd `dirname $0`; pwd)
BASE_PATH=${CURRENT_PATH}/../..

function copy_pkgs() {
    cp -rf ${BASE_PATH}/package/compileLib/*.tar.gz  ${BASE_PATH}/pkg
    echo -e "\nAfter copy pkg, ${BASE_PATH}/pkg contains:"
    ls -l ${BASE_PATH}/pkg
}

function saveall_docker() {
    echo "Run docker save to ${BASE_PATH}/pkg/Infrastructure_OM_docker.tar.gz"
    docker save -o ${BASE_PATH}/pkg/Infrastructure_OM_docker.tar $(docker images | sed 1d | grep -v rancher | grep -v '<none>' | grep -v 'oceanprotect-dataprotect' | grep -v harbor | tr -s ' ' | awk '{printf "%s:%s ",$1,$2}')
    if [ $? -ne 0 ]; then
        echo "docker save failed"
        exit 1
    fi

    if [ -f /usr/bin/pigz ]; then
        echo "Using pigz to zip"
        pigz -6 -p 12 ${BASE_PATH}/pkg/Infrastructure_OM_docker.tar
        if [ $? -ne 0 ]; then
            echo "docker save pigz failed"
            exit 1
        fi
    else
        echo "Using gzip to zip, it's very slow!!!"
        gzip ${BASE_PATH}/pkg/Infrastructure_OM_docker.tar
        if [ $? -ne 0 ]; then
            echo "docker save gzip failed"
            exit 1
        fi
    fi

    echo "docker save success."
  
}

function save_docker() {
    
    echo "save all Infrastructure microservice images"
    saveall_docker
    if [ $? -ne 0 ]; then
        echo "docker save gzip failed"
        exit 1
    fi

}

echo "#########################################################"
echo "   Starting save artifact"
echo "#########################################################"

rm -rf ${BASE_PATH}/pkg
mkdir -p ${BASE_PATH}/pkg

if [[ "${Compile_image}" == "Y" ]]; then
    save_docker
    if [ $? -ne 0 ]; then
        echo "docker save failed!"
    fi
fi
copy_pkgs

echo "#########################################################"
echo "   Save artifact Success"
echo "#########################################################"
