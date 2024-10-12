#!/bin/bash

set -ex

CUR_PATH=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
BASE_PATH=${CUR_PATH}/../..

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
	#生成源码清单文件
	sed -i "s#{PBI_NAME}#${PBI_NAME}#g" ${BASE_PATH}/CI/LCRP/conf/SourceCode.xml
	sh ${BASE_PATH}/CI/script/SourceCodeFile.sh

	cd ${BASE_PATH}/build
	sh image_init.sh patch
	if [ $? -ne 0 ];then
		echo "init failed."
		exit 1
	fi

	sh docker_helm_build.sh patch
	if [ $? -ne 0 ];then
		echo "docker compile failed."
		exit 1
	fi

	sh package_final.sh patch
	if [ $? -ne 0 ];then
		echo "package failed."
		exit 1
	fi

    sh upload_final_pkg_to_cmc.sh ${PRODUCT} ${CODE_BRANCH} mspkg
    if [ $? -ne 0 ]; then
        echo "upload to cmc failed!"
        exit 1
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
