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

BIN_PATH=$1
merge_id=$2

function build_npm(){
    cd ${PM_MS_DIR}/src/service/console/

    tar -zxvf ${BIN_PATH}/PM_GUI.tar.gz -C ${PM_MS_DIR}/src/service/console/
    if [[ $? -ne 0 ]]; then
        echo [INFO] Install Dependences for Frontend Project Failed.
        exit 1
    fi
    npm run open-build
    if [[ $? -ne 0 ]]; then
        echo [INFO] Compile Frontend Project Failed.
        exit 1
    fi

    echo "[INFO] Move dependence files"
    if [ -d /devcloud/node_modules ];then
        echo "Dependence fils already exists!"
        rm -rf /devcloud/node_modules
    fi
    mv ${PM_MS_DIR}/src/service/console/node_modules  /devcloud/
    cp -r dist/pm-gui/* ${PM_MS_DIR}/src/service/console
    rm -rf dist
}

function build_maven(){
	cd ${PM_MS_DIR}/src/service/
	mvn -Preal install -nsu -Dmaven.test.skip=true -gs ${BASE_PATH}/CI/conf/settings.xml
	if [ $? -ne 0 ]; then
		echo [INFO] maven compile Failed.
		exit 1
	fi
}

function copy_pkgs() {
	cd ${PM_MS_DIR}/tmp
	mkdir -p ${PM_MS_DIR}/pkg
	# 重新压缩（含kmc库）
	tar -zcvf ${PM_MS_DIR}/pkg/PM_GUI.tar.gz * --format=gnu

	find  ${PM_MS_DIR}/pkg/ -type d | xargs chmod 700
	find ${PM_MS_DIR}/pkg/ -type f | xargs chmod 550
	cp ${PM_MS_DIR}/pkg/PM_GUI.tar.gz ${BASE_PATH}/pkg/mspkg/
	if [ $? -ne 0 ]; then
		echo [INFO] copy pkg Failed.
		exit 1
	fi
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

function download_kmc(){
    echo "=========== Start copy KMC lib ==========="
    # 将CMC DEE库上的KMC Lib拷贝下来，并解压到lib库。
    echo "Download KMC lib"
    mkdir -p ${PM_MS_DIR}/tmp/tmp
    tar -zxvf ${BIN_PATH}/PM_System_Base_Service.tar.gz -C ${PM_MS_DIR}/tmp/tmp
    cp -rf ${PM_MS_DIR}/tmp/tmp/lib ${PM_MS_DIR}/tmp/lib
    rm -rf ${PM_MS_DIR}/tmp/tmp
}

function borrow_package(){
    # 解压PM_GUI.tar.gz
    echo "=========== start to borrow PM_GUI.tar.gz ==========="
    mkdir -p ${PM_MS_DIR}/tmp/
    tar -zxvf ${PM_MS_DIR}/src/service/target/PM_GUI.tar.gz -C ${PM_MS_DIR}/tmp
    echo "=========== Borrow PM_GUI.tar.gz success ==========="
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
		copy_pkgs
	else
		build_private
		borrow_package
		download_kmc
		copy_pkgs
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