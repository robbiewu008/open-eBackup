#!/bin/bash
# nas依赖的dme的包的下载
CURRENT_SCRIPT="${BASH_SOURCE[0]}"
SCRIPT_PATH=$( cd $(dirname ${CURRENT_SCRIPT});pwd)
BUILD_ROOT=$(cd ${SCRIPT_PATH}/../../build; pwd)
LCRP_CONFIG_PATH=${SCRIPT_PATH}/../LCRP/conf

source ${BUILD_ROOT}/common/common_artget.sh

SCRIPT_NAME="${BASH_SOURCE[0]##*/}"
THIRDPART_PATH=${FRAMEWORK_ROOT_PATH}/third_party_groupware
PLATFORM_PATH=${FRAMEWORK_ROOT_PATH}/platform
ARTIFACT_PATH=${EXT_PKG_DOWNLOAD_PATH}/3rd
COMPOENT_VERSION=1.2.1RC1
arch_type=$(uname -m)
if [ "$arch_type" == "aarch64" ]; then
    OS_TYPE="ARM"
    SYSTEM_NAME="Euler2.10"
else
    OS_TYPE="X86"
    SYSTEM_NAME="CentOS7.9"
fi
CODE_BRANCH="BR_Dev"

function clean_3rd()
{
    rm -rf ${THIRDPART_PATH}/*_rel
    rm -rf ${PLATFORM_PATH}/*_rel
    rm -rf ${PLATFORM_PATH}/var
    rm -rf ${ARTIFACT_PATH}
    exit 0
}


# 从cmc上下载开源三方和platform编译包
function download_pkg_from_cmc()
{
    local ext_pkg_path=${ARTIFACT_PATH}
    [ -d ${ext_pkg_path} ] && rm -rf ${ext_pkg_path}
    mkdir -p ${ext_pkg_path}
    artget pull -d ${LCRP_CONFIG_PATH}/opensrc_platform_dependencies.xml -p "{'COMPOENT_VERSION':'${COMPOENT_VERSION}','PRODUCT':'dorado', \
    'THIRD_BRANCH':'BR_Dev','OS_TYPE':'${OS_TYPE}','SYSTEM_NAME':'${SYSTEM_NAME}'}" \
    -ap ${ext_pkg_path} -user ${cmc_user} -pwd ${cmc_pwd}
}

# 适配解压platform包到platform目录
function uncompress_platform_pkg()
{
    local pkg_name=OceanProtect_X8000_${COMPOENT_VERSION}_Platform_SDK_${SYSTEM_NAME}_${OS_TYPE}.tar.gz
    local ext_pkg_path=${ARTIFACT_PATH}
    cd ${ext_pkg_path}
    if [ ! -f "${pkg_name}" ];then
        echo "${pkg_name} not exist, pls check"
        exit 1
    fi
    mkdir -p ${ext_pkg_path}/platform
    tar -zxvf ${pkg_name} -C ${ext_pkg_path}/platform
    if [ $? -ne 0 ];then
        echo "Uncompress ${pkg_name} failed, pls check"
        exit 1
    fi
    cd - >/dev/null

    [ -d ${PLATFORM_PATH} ] && rm -rf ${PLATFORM_PATH}/*_rel
    for rel_pkg_name in $(ls -1 ${ext_pkg_path}/platform/)
    do
        local pkg_full_path=${ext_pkg_path}/platform/${rel_pkg_name}
        if [ ! -z "${rel_pkg_name}" -a  -f "${pkg_full_path}" ];then
            tar -zxvf "${pkg_full_path}" -C ${PLATFORM_PATH}
        fi
    done
}

# 适配解压开源三方包到third_open_src目录
function uncompress_3rd_pkg()
{
    local pkg_name=OceanProtect_X8000_${COMPOENT_VERSION}_Opensrc_3rd_SDK_${SYSTEM_NAME}_${OS_TYPE}.tar.gz
    local ext_pkg_path=${ARTIFACT_PATH}
    cd ${ext_pkg_path}
    if [ ! -f "${pkg_name}" ];then
        echo "${pkg_name} not exist, pls check"
        exit 1
    fi
    mkdir -p ${ext_pkg_path}/3rd
    tar -zxvf ${pkg_name} -C ${ext_pkg_path}/3rd
    if [ $? -ne 0 ];then
        echo "Uncompress ${pkg_name} failed, pls check"
        exit 1
    fi
    cd - >/dev/null

    [ -d ${THIRDPART_PATH} ] && rm -rf ${WORKSPACE}/third_open_src/*_rel
    for rel_pkg_name in $(ls -1 ${ext_pkg_path}/3rd/)
    do
        local pkg_full_path=${ext_pkg_path}/3rd/${rel_pkg_name}
        if [ ! -z "${rel_pkg_name}" -a  -f "${pkg_full_path}" ];then
            tar -zxvf "${pkg_full_path}" -C ${THIRDPART_PATH}/
        fi
    done
}

function download_framework_3rd()
{
    log_echo "Begin down 3rd form cmc"
    download_artifact dorado BR_Dev 3rd
    if [ $? -ne 0 ]; then
        log_echo "Download artifact error"
        exit 1
    fi
}

function upzipartifact()
{
    log_echo "INFO" "Begin untar artifact"

    cd ${ARTIFACT_PATH}
    log_echo "DEBUG" "Begin to untar platform"
    tar xzf platform.tar.gz -C ${PLATFORM_PATH} KMCv3_infra_rel
    if [ $? -ne 0 ]; then
        log_echo "ERROR" "Untar platform error"
        exit 1
    fi
    log_echo "DEBUG" "Untar platform success"

    cd - >/dev/null
}

function main()
{
    if [ "X$1" == Xclean ];then
        log_echo "DEBUG" "Begin to clean DME third libs"
        clean_3rd
        log_echo "DEBUG" "Finish to clean DME third libs"
        exit 0
    fi
    download_pkg_from_cmc
    uncompress_platform_pkg
    uncompress_3rd_pkg

    download_framework_3rd
    upzipartifact
}

main "$@"
