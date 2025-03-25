#!bin/bash
# This file is a part of the open-eBackup project.
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file, You can obtain one at
# http://mozilla.org/MPL/2.0/.
#
# Copyright (c) [2024] Huawei Technologies Co.,Ltd.
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
set -ex

CUR_PATH=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
PM_MS_DIR=${CUR_PATH}/..
BASE_PATH=${PM_MS_DIR}/../..

merge_id=$1

function build_npm(){
	cd ${PM_MS_DIR}/src/service/console/

    if [ -d /devcloud/node_modules ];then
        echo "Dependence fils already exists!"
        mv /devcloud/node_modules ${PM_MS_DIR}/src/service/console/
    fi

    npm install
    if [[ $? -ne 0 ]]; then
        echo [INFO] Install Dependences for Frontend Project Failed.
        exit 1
    fi
    npm run build
    if [[ $? -ne 0 ]]; then
        echo [INFO] Compile Frontend Project Failed.
        exit 1
    fi

    echo "[INFO] Move dependence files"
    mv ${PM_MS_DIR}/src/service/console/node_modules  /devcloud/
    cp -r dist/pm-gui/* ${PM_MS_DIR}/src/service/console
    rm -rf dist
}

function build_maven(){
	cd ${PM_MS_DIR}/src/service/
	mvn -Preal install -nsu -Dmaven.test.skip=true -s ${BASE_PATH}/CI/conf/settings.xml
	if [ $? -ne 0 ]; then
		echo [INFO] maven compile Failed.
		exit 1
	fi
}

function borrow_package(){
    # 解压PM_GUI.tar.gz
    echo "=========== start to borrow PM_GUI.tar.gz ==========="
    mkdir -p ${PM_MS_DIR}/tmp/
    tar -zxvf ${PM_MS_DIR}/src/service/target/PM_GUI.tar.gz -C ${PM_MS_DIR}/tmp
    echo "=========== Borrow PM_GUI.tar.gz success ==========="
}

function download_kmc(){
    echo "=========== Start copy KMC lib ==========="
    # 将CMC DEE库上的KMC Lib拷贝下来，并解压到lib库。
    echo "Download KMC lib"
    artget pull -d ${PM_MS_DIR}/CI/conf/dependency_from_cmc.xml -ap ${PM_MS_DIR}/tmp/ -user ${cmc_user} -pwd ${cmc_pwd}
    tar zxvf ${PM_MS_DIR}/tmp/kmc-3.1.1.tar.gz -C ${PM_MS_DIR}/tmp
    if [ $? -ne 0 ]; then
      echo "Unzip kmc failed"
      exit 1
    fi
    rm -rf ${PM_MS_DIR}/tmp/kmc-3.1.1.tar.gz
    mv ${PM_MS_DIR}/tmp/release/lib ${PM_MS_DIR}/tmp/lib
    echo "=========== End copy KMC lib ==========="
}

function build_package(){
    echo "=========== Build package start ========="
    cd ${PM_MS_DIR}/tmp

    # 重新压缩（含kmc库）
    tar -zcvf PM_GUI.tar.gz * --format=gnu

    mkdir -p ${PM_MS_DIR}/pkg/
    cp -f ${PM_MS_DIR}/tmp/PM_GUI.tar.gz  ${PM_MS_DIR}/pkg/
    if [ $? -ne 0 ]; then
      echo "copy pkg failed."
      exit 1
    fi
    echo "=========== Build package success ==========="
}


function Get_changeFiles()
{
	cd ${PM_MS_DIR}/
	commit_id_source=`git log -1 | grep ^commit | awk '{print $2}'`
	git fetch origin +refs/merge-requests/${merge_id}/head:refs/merge-requests/${merge_id}/head
	git merge refs/merge-requests/${merge_id}/head
	git diff ${commit_id_source} --name-only > ${PM_MS_DIR}/all_diffrentfiles.list
}

function check_suffix(){
	cd ${PM_MS_DIR}/
	for line in `cat all_diffrentfiles.list`
	do
		file=$line
		suffix=${file##*.}
		if [ "$suffix" = "ts" ] || [ "$suffix" = "html" ] || [ "$suffix" = "json" ]; then
			echo "need run webui compile"
			js=true
			break
		fi
	done
	
	for line in `cat all_diffrentfiles.list`
	do
		file=$line
		suffix=${file##*.}
		if [ "$suffix" = "java" ]; then
			echo "need run java compile"
			java=true
			break
		fi
	done
}

function build_npm_i18n(){
	
	cd ${PM_MS_DIR}/src/service/console/
	npm run i18n
    content=`cat repeat-i18n.json |grep "}" | sed 's/{//g' | sed 's/}//g'`
    if [ "$content" != "" ]; then
        echo "repeat-i18n.json 内容不为空，异常退出"
        echo "repeat-i18n.json内容如下："
        cat repeat-i18n.json
        exit 1
    fi
	
}

function build_private(){

	build_npm
	if [ $? -ne 0 ]; then    
		echo [INFO] webui compile Failed.
		exit 1
	fi

	build_maven
	if [ $? -ne 0 ]; then    
		echo [INFO] maven compile Failed.
		exit 1
	fi
	
	build_npm_i18n
	if [ $? -ne 0 ]; then    
		echo [INFO] npm i18n compile Failed.
		exit 1
	fi
}

function Replace_Placeholder(){

	cd ${PM_MS_DIR}/CI/script/
	ConfigFile="${PM_MS_DIR}/CI/script/build.properties"
	. ${PM_MS_DIR}/CI/script/pretreatment.sh
	
	Replace ${PM_MS_DIR}/src/service/console/src/assets/i18n/en-us/common.json
	Replace ${PM_MS_DIR}/src/service/console/src/assets/i18n/en-us/protection.json
	Replace ${PM_MS_DIR}/src/service/console/src/assets/i18n/en-us/error-code/common.json
	Replace ${PM_MS_DIR}/src/service/console/src/assets/i18n/zh-cn/common.json
	Replace ${PM_MS_DIR}/src/service/console/src/assets/i18n/zh-cn/protection.json
	Replace ${PM_MS_DIR}/src/service/console/src/assets/i18n/zh-cn/error-code/common.json

}

function main(){
	
	Replace_Placeholder

	if [ $# = 0 ]; then
		build_npm
		build_maven
		borrow_package
		download_kmc
		build_package
	else
		build_private
		if [ $? -ne 0 ]; then    
			echo [INFO] private compile Failed.
			exit 1
		fi
	fi
}

export buildNumber=$(date +%Y%m%d%H%M%S)

if [[ ${releaseVersion} ]]; then
	PackageVersion=${releaseVersion}
	echo "buildVersion=${releaseVersion}" > ${PM_MS_DIR}/../buildInfo.properties
else
	serviceVersion=`cat ${PM_MS_DIR}/app_define.json | awk -F '\"version":' '{print $2}' | awk -F ',' '{print $1}' | sed 's/\"//g'`
	PackageVersion=${serviceVersion}.${buildNumber}
	echo "buildVersion=${PackageVersion}" > ${PM_MS_DIR}/../buildInfo.properties
fi


main $@
