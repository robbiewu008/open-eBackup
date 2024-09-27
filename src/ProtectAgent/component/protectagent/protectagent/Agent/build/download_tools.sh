#!/bin/sh
# CIE提供得编译镜像不支持asan编译，需要在编译时下载asan库到容器中

CURRENT_PATH=""
CMCCONF_PATH=""
SYS_ARCH="arm"

PreASAN()
{
    echo "PRODUCT: ${PRODUCT}"
    echo "componentVersion: ${componentVersion}"
    echo "PRODUCT: ${PRODUCT}"
    echo "CMCCONF_PATH: ${CMCCONF_PATH}"
    echo "SYS_ARCH: ${SYS_ARCH}"

    #1.
    if [ ! -d ${WORKSPACE}/Tools} ];then
        mkdir -p ${WORKSPACE}/Tools
    fi
    artget pull -d ${CMCCONF_PATH}/tools_from_cmc.xml -p "{'PRODUCT': '${PRODUCT}', 'SYS_ARCH': '${SYS_ARCH}', \
    'componentVersion': '${componentVersion}'}" -ap "${WORKSPACE}/Tools" -user ${cmc_user} -pwd ${cmc_pwd}

    #2.
    sudo cp -f ${WORKSPACE}/Tools/ASAN/* /usr/lib64
    sudo cp -f ${WORKSPACE}/Tools/ASAN/* /usr/lib/gcc/aarch64-linux-gnu/7.3.0
    SO_Files=$(ls ${WORKSPACE}/Tools/ASAN/)
    for so_file in ${SO_Files}
    do
        echo "${so_file}"
        sudo chmod 777 /usr/lib64/${so_file} /usr/lib/gcc/aarch64-linux-gnu/7.3.0/${so_file}
        sudo ln -sf /usr/lib64/${so_file} /usr/lib64/${so_file%????}
        sudo ln -sf /usr/lib64/${so_file} /usr/lib64/${so_file%??????}
        sudo ln -sf /usr/lib/gcc/aarch64-linux-gnu/7.3.0/${so_file} /usr/lib/gcc/aarch64-linux-gnu/7.3.0/${so_file%????}
        sudo ln -sf /usr/lib/gcc/aarch64-linux-gnu/7.3.0/${so_file} /usr/lib/gcc/aarch64-linux-gnu/7.3.0/${so_file%??????}
    done
}

GetPath()
{
    CURRENT_PATH=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
    CMCCONF_PATH=${CURRENT_PATH}/../ci/LCRP/conf
}

#1.
GetPath

#2. download the asan library
PreASAN
