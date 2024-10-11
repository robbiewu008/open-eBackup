#!/bin/sh
CURRENT_PATH=$(cd "$(dirname $0)" && pwd)


SYS_ARCH=""
SYS_NAME=""
LIB_SYS_ARCH=""
Version="1.2.1RC1"
declare -A opensrc_3rd_map=()
declare -A platform_map=()
sys=`uname -s`
if [ "$sys" = "SunOS" ]
then
    AWK=nawk
else
    AWK=awk
fi

OPENSOURCE_REPOSITORY_DIR=$1

AGENT_ROOT=${CURRENT_PATH}/..
OPENSOURCE_PKG_DOWNLOAD_PATH=${AGENT_ROOT}/build/Opensource
OPENSOURCE_PKG_WORK_PATH=${AGENT_ROOT}/open_src/

PLATFORM_PKG_WORK_PATH=${AGENT_ROOT}/platform

echo "OPENSOURCE_REPOSITORY_DIR=$OPENSOURCE_REPOSITORY_DIR"

if [ ! -d "$OPENSOURCE_REPOSITORY_DIR" ]; then
    echo "OPENSOURCE_REPOSITORY_DIR=$OPENSOURCE_REPOSITORY_DIR is not exist."
    exit 1
fi

init_artget_opensource_env()
{
    #1. init dir
    if [ ! -d "${OPENSOURCE_PKG_DOWNLOAD_PATH}" ]; then
        echo "mkdir ${OPENSOURCE_PKG_DOWNLOAD_PATH}."
        mkdir "${OPENSOURCE_PKG_DOWNLOAD_PATH}"
    fi
    rm -rf "${OPENSOURCE_PKG_DOWNLOAD_PATH}"/*

    #2. get sys information
    arch_type=`uname -m`
    if [ "${arch_type}" = "aarch64" ]; then
        SYS_NAME="Euler2.10"
        cat /etc/system-release | grep CentOS >>/dev/null 2>&1
        if [ $? -eq 0 ]; then
            SYS_NAME="CentOS7.6"
        fi
        SYS_ARCH="ARM"
    else
        SYS_NAME="CentOS6.10"
        SYS_ARCH="X86"
    fi

    #3.
    opensrc_3rd_map["boost_agent_rel.tar.gz"]="boost"
    opensrc_3rd_map["curl_agent_rel.tar.gz"]="curl"
    opensrc_3rd_map["fcgi_agent_rel.tar.gz"]="fcgi"
    opensrc_3rd_map["gperftools_agent_rel.tar.gz"]="gperftools"
    opensrc_3rd_map["jsoncpp_agent_rel.tar.gz"]="jsoncpp"
    opensrc_3rd_map["libevent_rel.tar.gz"]="libevent"
    opensrc_3rd_map["libuuid_agent_rel.tar.gz"]="libuuid"
    opensrc_3rd_map["nginx_agent_rel.tar.gz"]="nginx"
    opensrc_3rd_map["openssl_agent_rel.tar.gz"]="openssl"
    opensrc_3rd_map["opensslv1_agent_rel.tar.gz"]="opensslv1"
    opensrc_3rd_map["snmp_agent_rel.tar.gz"]="snmp++"
    opensrc_3rd_map["sqlite_agent_rel.tar.gz"]="sqlite"
    opensrc_3rd_map["thrift_agent_rel.tar.gz"]="thrift"
    opensrc_3rd_map["thriftv1_agent_rel.tar.gz"]="thriftv1"
    opensrc_3rd_map["tinyxml2_agent_rel.tar.gz"]="tinyxml"
    opensrc_3rd_map["util-linux_rel.tar.gz"]="util-linux"
    opensrc_3rd_map["zlib_agent_rel.tar.gz"]="zlib"
    opensrc_3rd_map["libaio_rel.tar.gz"]="libaio"
}

copy_opensource()
{
    echo "SYS_NAME: ${SYS_NAME}"
    echo "SYS_ARCH: ${SYS_ARCH}"
    echo "Version: ${Version}"

    cp -rf ${OPENSOURCE_REPOSITORY_DIR}/${SYS_NAME}/${SYS_ARCH}/third_party_groupware/* ${OPENSOURCE_PKG_DOWNLOAD_PATH}

    echo "cp ${OPENSOURCE_REPOSITORY_DIR}/${SYS_NAME}/${SYS_ARCH}/third_party_groupware to ${OPENSOURCE_PKG_DOWNLOAD_PATH} finish"
}

get_platform()
{
    PLATFORM_PKG_TMP_PATH=$PLATFORM_PKG_WORK_PATH/tmp
    if [ ! -d PLATFORM_PKG_TMP_PATH ]; then
        mkdir -p $PLATFORM_PKG_TMP_PATH
        echo "mkdir $PLATFORM_PKG_TMP_PATH"
    fi

    platform_pkg="OceanProtect_X8000_${Version}_Platform_SDK_${SYS_NAME}_${SYS_ARCH}.tar.gz"

    cp -rf ${OPENSOURCE_REPOSITORY_DIR}/${SYS_NAME}/${SYS_ARCH}/platform/${platform_pkg} ${PLATFORM_PKG_TMP_PATH}

    echo "cp ${OPENSOURCE_REPOSITORY_DIR}/${SYS_NAME}/${SYS_ARCH}/platform to ${PLATFORM_PKG_TMP_PATH} finish"

    if [ ! -f "${OPENSOURCE_REPOSITORY_DIR}/${SYS_NAME}/${SYS_ARCH}/platform/${platform_pkg}" ]; then
        echo "The platform package does not exist."
        exit 1
    fi

    tar zxvf "${OPENSOURCE_REPOSITORY_DIR}/${SYS_NAME}/${SYS_ARCH}/platform/${platform_pkg}" -C ${PLATFORM_PKG_TMP_PATH}
    if [ $? -ne 0 ]; then
        echo "Failed to decompress the plat form package."
        exit 1
    fi

    platform_map["SecureCLib_rel.tar.gz"]="securec"
    platform_map["KMCV3_rel.tar.gz"]="kmc"

    for pkg_name in "${!platform_map[@]}"
    do
    {
        if [ ! -f "${PLATFORM_PKG_TMP_PATH}/${pkg_name}" ]; then
            echo "Packege ${PLATFORM_PKG_TMP_PATH}/${pkg_name} not exist."
            exit 1
        fi

        cp -rf ${PLATFORM_PKG_TMP_PATH}/${pkg_name} $PLATFORM_PKG_WORK_PATH
        echo "Copy platform pkg ${pkg_name} of ${platform_map[${pkg_name}]} finish."
    } done

    rm -rf $PLATFORM_PKG_TMP_PATH
}

decompress_opensource()
{
    if [ ! -d $OPENSOURCE_PKG_WORK_PATH ]; then
        mkdir -p $OPENSOURCE_PKG_WORK_PATH
        echo "mkdir $OPENSOURCE_PKG_WORK_PATH"
    fi

    cd "${OPENSOURCE_PKG_DOWNLOAD_PATH}"
    cp "${OPENSOURCE_PKG_DOWNLOAD_PATH}/lib"* "${OPENSOURCE_PKG_WORK_PATH}"

    #1. check opensource pkg
    opensource_pkg="OceanProtect_X8000_${Version}_Opensrc_3rd_SDK_${SYS_NAME}_${SYS_ARCH}.tar.gz"
    if [ ! -f "${opensource_pkg}" ]; then
        echo "The open-source package does not exist."
        exit 1
    fi
    tar zxvf "${opensource_pkg}" > /dev/null
    if [ $? -ne 0 ]; then
        echo "Failed to decompress the package."
        exit 1
    fi
    rm -rf "${OPENSOURCE_PKG_DOWNLOAD_PATH}"*.tar.gz

    #2. decompress pkg_3rd
    cd "${OPENSOURCE_PKG_DOWNLOAD_PATH}/agent"
    cp ../libaio_rel.tar.gz .
    for pkg_3rd_name in "${!opensrc_3rd_map[@]}"
    do
    {
        echo "Start decompress ${pkg_3rd_name}."
        if [ ! -f "${pkg_3rd_name}" ]; then
            echo "The package: -${pkg_3rd_name}- does not exist.."
            exit 1
        fi
        tar zxvf "${pkg_3rd_name}" > /dev/null
        if [ $? -ne 0 ]; then
            echo "Failed to decompress the package: ${pkg_3rd_name}."
            exit 1
        fi
        echo "decompress ${pkg_3rd_name} succ."

        old_folder=`echo ${pkg_3rd_name} |${AWK} -F '.' '{print $1}'`
        new_folder="${opensrc_3rd_map[${pkg_3rd_name}]}"
        rm -rf "${OPENSOURCE_PKG_DOWNLOAD_PATH}/agent/${pkg_3rd_name}"
        rm -rf "$OPENSOURCE_PKG_WORK_PATH/${new_folder}"
        mv "${old_folder}" "$OPENSOURCE_PKG_WORK_PATH/${new_folder}"
    }&
    done
    wait

    #3. copy the opensource Binary
    cp -rf "$OPENSOURCE_PKG_WORK_PATH/openssl/.openssl/bin/openssl" "${CURRENT_PATH}/../bin/"
    cp -rf "$OPENSOURCE_PKG_WORK_PATH/openssl/.openssl/ssl/openssl.cnf" "${CURRENT_PATH}/../conf/"
    cp -rf "$OPENSOURCE_PKG_WORK_PATH/sqlite/bin/sqlite3" "${CURRENT_PATH}/../bin/"
    cp -rf "$OPENSOURCE_PKG_WORK_PATH/nginx" "${CURRENT_PATH}/../bin/"
    cp -rf "${CURRENT_PATH}/../conf/backup/nginx.conf" "${CURRENT_PATH}/../bin/nginx/conf"
    mv "${CURRENT_PATH}/../bin/nginx/nginx" "${CURRENT_PATH}/../bin/nginx/rdnginx"
}

# #1. Initialize the download directory and obtain the system information.
init_artget_opensource_env

# #2. get the open-source third-party package.
copy_opensource

# #3. decompress and adapt  folder name
decompress_opensource

# 4. get platform pkg
get_platform

echo "#########################################################"
echo "    Get open third party finish."
echo "#########################################################"