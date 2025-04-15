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
set -ex

CUR_PATH=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
BASE_PATH=${CUR_PATH}/../..
export G_BASE_DIR="$(cd "$(dirname "$BASH_SOURCE")/../../";pwd)"
PKG_PATH_NAME=open-eBackup
PBI_NAME_LAST=${PBI_NAME}
G_VERSION=${Version}
PRODUCT=open-eBackup
OFFERING="OceanProtect DataBackup"
LAST_MS_TAG=${MS_IMAGE_TAG}

function initialize_inf_pm() {
    #替换基础设施yaml文件中的数字版本号
    sed -i "s/current_digital_version/${digitalVersion}/g" ${G_BASE_DIR}/component/INF_CI/build/helm/infrastructure/templates/DigitalVersionConfig.yaml
}

function copy_files() {
    # 复制CI库的文件
    cd ${G_BASE_DIR}/..
    cp -rf ProtectManager/CI/* ${G_BASE_DIR}/component/PM_CI
    cp -rf DataMoveEngine/CI/* ${G_BASE_DIR}/component/DME_CI
    cp -rf Infrastructure_OM/ci/* ${G_BASE_DIR}/component/INF_CI
    cp -rf ProtectAgent/* ${G_BASE_DIR}/component/Plugins

    #拷贝PM_API_Gateway配置文件到ProtectManager的chart/template/目录下
 #   cp -rf ProtectManager/component/PM_API_Gateway/IngressRoute/*   ${G_BASE_DIR}/component/PM_CI/build/helm/protect-manager/templates/

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
    rm -f "${G_BASE_DIR}/build/dockerfiles/base.name"

    echo -e "\nAfter copy docker files, ${G_BASE_DIR}/build/dockerfiles contains:"
    ls -l ${G_BASE_DIR}/build/dockerfiles

    echo "Begin to copy all helm files"
    find "${G_BASE_DIR}/component" -maxdepth 4 -type d -iregex '.*helm/.*' -exec cp -rf "{}" "${G_BASE_DIR}/build/helm/components" \;
    echo -e "\nAfter copy helm dirs, ${G_BASE_DIR}/build/helm/components contains:"
    ls -l ${G_BASE_DIR}/build/helm/components
}

#set bep
if [ "${BEP}" == "YES" ]; then
	echo "start set Bep_Time!"
	source bep_env.sh -s ${BASE_PATH}/CI/conf/bep_env.conf
fi

while getopts 's:p:b:B' OPT; do
    case '${OPT}' in
        s) IS_SNAPSHOT="$OPTARG";;
		p) PBI_NAME="$OPTARG";;
		h) usage;;
        ?) usage;;
    esac
done

echo IS_SNAPSHOT=${IS_SNAPSHOT}
echo PBI_NAME=${PBI_NAME}

function main()
{
  # copy_files
  if [ "${OPENSOURCE_BUILD}" == "Y" ];then
    if [ "${BUILD_MODULE}" == "system_dee" ];then
        export PKG_NAME="DataManager"
    elif [ "${BUILD_MODULE}" == "system_dme" ];then
        export PKG_NAME="MediaServer"
    elif [ "${BUILD_MODULE}" == "system_pm" ];then
        export PKG_NAME="MasterServer"
    elif [ "${BUILD_MODULE}" == "system_agent" ];then
        export PKG_NAME="ProtectAgent"
    fi
    cd ${BASE_PATH}/build
    sh docker_helm_build.sh OpenSource
    if [ $? -ne 0 ];then
      echo "docker compile failed."
      exit 1
    fi

    sh package_open_final.sh
    if [ $? -ne 0 ];then
      echo "package failed."
      exit 1
    fi

  else
    #生成源码清单文件
    sed -i "s#{PBI_NAME}#${PBI_NAME}#g" ${BASE_PATH}/CI/LCRP/conf/SourceCode.xml
    sh ${BASE_PATH}/CI/script/SourceCodeFile.sh

    cd ${BASE_PATH}/build
    sh docker_helm_build.sh
    if [ $? -ne 0 ];then
      echo "docker compile failed."
      exit 1
    fi

    sh package_final.sh
    if [ $? -ne 0 ];then
      echo "package failed."
      exit 1
    fi
  fi
}

buildNumber=$(date "+%Y%m%d%H%M%S")

if [[ ${releaseVersion} ]]; then
	PackageVersion=${releaseVersion}
	echo "buildVersion=${releaseVersion}" > ${BASE_PATH}/buildInfo.properties
else
	serviceVersion=`cat ${BASE_PATH}/app_define.json | awk -F '\"version":' '{print $2}' | awk -F ',' '{print $1}' | sed 's/\"//g'`
	PackageVersion=${serviceVersion}.${buildNumber}
	echo "buildVersion=${PackageVersion}" > ${BASE_PATH}/buildInfo.properties
fi

echo "#########################################################"
echo "   Begin package 100P  "
echo "#########################################################"

main

echo "#########################################################"
echo "   100P package Success  "
echo "#########################################################"
