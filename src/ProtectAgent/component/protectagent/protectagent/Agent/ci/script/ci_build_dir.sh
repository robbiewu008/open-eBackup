#!/bin/bash

set -ex

CUR_PATH=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
BASE_PATH=${CUR_PATH}/../../../
PKG_TYPE=Plugins
PKG_TYPE_FFILCLIENT=FileClient
componentVersion_fileclient="1.6.0"    # fileclient包在1.6，需要后续agent和插件包归档到1.6之后需更改

OPENSOURCE_REPOSITORY_DIR=${binary_path}

if [ -z ${branch} ];then
    echo "Please specify build branch!"
    exit 1 
fi

if [ -z "${componentVersion}" ]; then
    componentVersion="1.1.0"
fi

if [ -z "${HADOOP_BRANCH}" ]; then
    echo "Hadoop branch is empty."
    exit 1
fi

if [ -z "${GENERALDB_BRANCH}" ]; then
    GENERALDB_BRANCH="SUB_SYSTEM"
    echo "GeneralDB branch is empty."
fi

if [ -z "${FILEPLUGIN_BRANCH}" ]; then
    FILEPLUGIN_BRANCH="SUB_SYSTEM"
    echo "FilePlugin branch is empty."
fi

if [ -z "${VIRTUALIZATION_BRANCH}" ]; then
    echo "[ERR] VirtualizationPlugin branch is empty."
    exit 1
fi

if [ -z "${FUSIONCOMPUTE_BRANCH}" ]; then
    FUSIONCOMPUTE_BRANCH="SUB_SYSTEM"
    echo "FusionComputePlugin branch is empty."
fi

echo "Component Version:${componentVersion}"

if [ -z "$BUILD_PKG_TYPE" ]; then
    BUILD_PKG_TYPE=$1
fi
echo BUILD_PKG_TYPE=${BUILD_PKG_TYPE}

if [ -z "$BUILD_OS_TYPE" ]; then
    BUILD_OS_TYPE=$2
fi
echo BUILD_OS_TYPE=${BUILD_OS_TYPE}

MakeDir()
{
    if [ ! -d "$1" ]; then
        mkdir -p "$1"
        echo "mkdir $1"
    fi
}

CpFile()
{
    if [ ! -f "$1" ]; then
        echo "=== $1 file noy exist ==="
        return 1
    fi
    cp "$1" "$2"
    echo "copy file $1 to $2"
}

cd ${BASE_PATH}/
MakeDir temp
MakeDir Plugins
MakeDir tmp_zip

MakeDir final_pkg
MakeDir final_pkg/ProtectClient-e
MakeDir final_pkg/third_party_software
MakeDir final_pkg/PackageScript
MakeDir final_pkg/Plugins
MakeDir final_pkg/package
MakeDir final_pkg/PackageScript/windows
MakeDir final_pkg/PackageScript/like-unix

#################################### 拷贝第三方软件 ####################################
# package
cp -rf Agent/ci/script/PackingRules/* ${BASE_PATH}/final_pkg/package

# Plugins
cd ${BASE_PATH}/Agent/ci/LCRP/conf
if [ "$BUILD_PKG_TYPE" = "OpenSource" ]; then
    CpFile ${OPENSOURCE_REPOSITORY_DIR}/Plugins/Linux/x86_64/HadoopPlugin.tar.gz ${BASE_PATH}/Plugins
    CpFile ${OPENSOURCE_REPOSITORY_DIR}/Plugins/Linux/x86_64/ElasticSearchPlugin.tar.gz ${BASE_PATH}/Plugins

    CpFile ${OPENSOURCE_REPOSITORY_DIR}/Plugins/Linux/NasPlugin.tar.xz ${BASE_PATH}/Plugins
    CpFile ${OPENSOURCE_REPOSITORY_DIR}/Plugins/Linux/FilePlugin.tar.xz ${BASE_PATH}/Plugins
    CpFile ${OPENSOURCE_REPOSITORY_DIR}/Plugins/Linux/GeneralDBPlugin.tar.xz ${BASE_PATH}/Plugins
    CpFile ${OPENSOURCE_REPOSITORY_DIR}/Plugins/Linux/Block_Service.tar.xz ${BASE_PATH}/Plugins
    CpFile ${OPENSOURCE_REPOSITORY_DIR}/Plugins/Linux/VirtualizationPlugin.tar.xz ${BASE_PATH}/Plugins
    CpFile ${OPENSOURCE_REPOSITORY_DIR}/Plugins/Linux/FusionComputePlugin.tar.xz ${BASE_PATH}/Plugins
    CpFile ${OPENSOURCE_REPOSITORY_DIR}/Plugins/Linux/ObsPlugin.tar.xz ${BASE_PATH}/Plugins

    CpFile ${OPENSOURCE_REPOSITORY_DIR}/Plugins/Linux/aarch64/cppframework-Linux_aarch64.tar.xz ${BASE_PATH}/Plugins
    CpFile ${OPENSOURCE_REPOSITORY_DIR}/Plugins/Linux/x86_64/cppframework-Linux_x86_64.tar.xz ${BASE_PATH}/Plugins

    CpFile ${OPENSOURCE_REPOSITORY_DIR}/Plugins/Windows/FilePlugin.zip ${BASE_PATH}/Plugins
    CpFile ${OPENSOURCE_REPOSITORY_DIR}/Plugins/Windows/VirtualizationPlugin.zip ${BASE_PATH}/Plugins
    CpFile ${OPENSOURCE_REPOSITORY_DIR}/Plugins/Windows/GeneralDBPlugin.zip ${BASE_PATH}/Plugins

    CpFile ${OPENSOURCE_REPOSITORY_DIR}/Plugins/AIX/FilePlugin_ppc_64.tar.xz ${BASE_PATH}/Plugins
    CpFile ${OPENSOURCE_REPOSITORY_DIR}/Plugins/AIX/cppframework-AIX_ppc_64.tar.xz ${BASE_PATH}/Plugins

    CpFile ${OPENSOURCE_REPOSITORY_DIR}/Plugins/Solaris/FilePlugin_sun4v.tar.gz ${BASE_PATH}/Plugins
    CpFile ${OPENSOURCE_REPOSITORY_DIR}/Plugins/Solaris/cppframework-SunOS_sun4v.tar.gz ${BASE_PATH}/Plugins
fi

if [ $? -ne 0 ]; then
    echo "CMC download failed"
    exit 1
fi

# download fileclient
echo "start download fileclient"
if [ "$BUILD_PKG_TYPE" = "OpenSource" ]; then
    MakeDir ${BASE_PATH}/FileClient
    CpFile ${OPENSOURCE_REPOSITORY_DIR}/Plugins/fileClient_aarch64.tar.gz ${BASE_PATH}/FileClient/
    CpFile ${OPENSOURCE_REPOSITORY_DIR}/Plugins/fileClient_x86_64.tar.gz ${BASE_PATH}/FileClient/
elif [ "$BUILD_PKG_TYPE" = "OpenSource" ] && [ "$BUILD_OS_TYPE" != "aix" ] && [ "$BUILD_OS_TYPE" != "solaris" ]; then
    MakeDir ${BASE_PATH}/FileClient
    CpFile ${OPENSOURCE_REPOSITORY_DIR}/Plugins/fileClient_aarch64.tar.gz ${BASE_PATH}/FileClient/
    CpFile ${OPENSOURCE_REPOSITORY_DIR}/Plugins/fileClient_x86_64.tar.gz ${BASE_PATH}/FileClient/
else
    artget pull -d fileclient_pkg_from_cmc.xml -p "{'componentVersion':'${componentVersion}', 'componentVersion_fileclient':'${componentVersion_fileclient}','AGENT_BRANCH':'${branch}','PKG_TYPE':'${PKG_TYPE_FFILCLIENT}'}" -ap ${BASE_PATH}/FileClient -user ${cmc_user} -pwd ${cmc_pwd}
    if [ $? -ne 0 ]; then
        echo "CMC download fileclient failed"
    fi
fi

cd ${BASE_PATH}/
# es & hadoop is in tar.gz format
# other linux packages are in tar.xz format
# win packages are in zip format
if [ "$BUILD_PKG_TYPE" = "OceanCyber" ]; then
    cp -rf ${BASE_PATH}/Plugins/cppframework-Linux*.tar.xz ${BASE_PATH}/final_pkg/Plugins
elif [ "$BUILD_PKG_TYPE" = "OpenSource" ]; then
    cp -rf ${BASE_PATH}/Plugins/*.tar.gz ${BASE_PATH}/final_pkg/Plugins
    cp -rf ${BASE_PATH}/Plugins/*.tar.xz ${BASE_PATH}/final_pkg/Plugins
    cp -rf ${BASE_PATH}/Plugins/*.zip ${BASE_PATH}/final_pkg/Plugins
else
    cp -rf ${BASE_PATH}/Plugins/*.tar.gz ${BASE_PATH}/final_pkg/Plugins
    cp -rf ${BASE_PATH}/Plugins/*.tar.xz ${BASE_PATH}/final_pkg/Plugins
    cp -rf ${BASE_PATH}/Plugins/*.zip ${BASE_PATH}/final_pkg/Plugins
fi

# PackageScript
cd tmp_zip
find ${BASE_PATH}/Agent/bin/install/ -maxdepth 1 \( -name "*sh" -o -name "*txt*" \)  -exec cp -f "{}" "." \;
zip -r ${BASE_PATH}/final_pkg/PackageScript/package-like-unix.zip .
rm -rf ${BASE_PATH}/tmp_zip/*

find ${BASE_PATH}/Agent/bin/install/ -maxdepth 1 \( -name "*bat" -o -name "*txt*" \)  -exec cp -f "{}" "." \;
zip -r ${BASE_PATH}/final_pkg/PackageScript/package-windows.zip .
rm -rf ${BASE_PATH}/tmp_zip/*

cp ${BASE_PATH}/Agent/bin/bat/push_install_check.bat ${BASE_PATH}/final_pkg/PackageScript/windows
cp ${BASE_PATH}/Agent/bin/shell/push_install_check.sh ${BASE_PATH}/final_pkg/PackageScript/like-unix
cd ${BASE_PATH}/

# third_party_software
# linux 3rd zip
cd tmp_zip
if [ "$BUILD_PKG_TYPE" != "OpenSource" ]; then
    cp -rf ${WORKSPACE}/dataturbo_pkg/dataturbo.zip .
fi
cp -rf ${BASE_PATH}/FileClient/fileClient* .
zip -r ${BASE_PATH}/final_pkg/third_party_software/3rd-linux.zip .

# windows 3rd zip
rm -rf ${BASE_PATH}/tmp_zip/*

if [ "$BUILD_PKG_TYPE" = "OpenSource" ]; then
    mkdir -p ${BASE_PATH}/Agent/open_src
    cp ${OPENSOURCE_REPOSITORY_DIR}/ThirdParty/Windows2012/x64/third_party_groupware/OceanProtect_X8000_1.2.1RC1_Opensrc_3rd_SDK_Windows2012_x64.zip ${BASE_PATH}/Agent/open_src/
else
    artget pull -d ${BASE_PATH}/Agent/ci/LCRP/conf/dependency_opensrc_win.xml -p "{'OPENSOURCE_BRANCH': '${OPENSOURCE_BRANCH}', 'SYS_NAME':'Windows2012', 'SYS_ARCH':'x64', 'Version':'1.2.1RC1'}" -user ${cmc_user} -pwd ${cmc_pwd} -ap ${BASE_PATH}/Agent/open_src
    if [ $? -ne 0 ]; then
        echo "CMC download opensrc windows failed"
        exit 1
    fi
fi
unzip ${BASE_PATH}/Agent/open_src/OceanProtect_X8000_1.2.1RC1_Opensrc_3rd_SDK_Windows2012_x64.zip "7zip/lib/7z.*" -d ./unzip
mkdir ./7Zip
cp -rf ${BASE_PATH}/tmp_zip/unzip/7zip/lib/7z.* ./7Zip
if [ "$BUILD_PKG_TYPE" != "OpenSource" ]; then
    cp -rf ${WORKSPACE}/dataturbo_pkg/dataturbo-windows.zip .
fi
zip -r ${BASE_PATH}/final_pkg/third_party_software/3rd-windows.zip .
rm -rf ${BASE_PATH}/tmp_zip/*
cd ${BASE_PATH}/