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

CUR_PATH=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
PM_MS_DIR=${CUR_PATH}/..
BASE_PATH=${PM_MS_DIR}/../..

merge_id=$1

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
		UI=`echo $file | grep src/Main`
		if [ "$UI" != "" ]; then
			if [ "$suffix" = "json" ]; then
				echo "need run webui compile"
				json=true
				break
			fi
		fi
	done
}

function build_maven(){
	cd ${PM_MS_DIR}/src/
	mvn -Preal install -nsu -Dmaven.test.skip=true -Dkmc.build.enabled=true
	if [ $? -ne 0 ]; then
		echo "maven compile failed."
		exit 1
	fi
}

function build_gui_npm(){
	cp -rf ${PM_MS_DIR}/src/Main/*.json ${PM_MS_DIR}/../PM_GUI/src/service/console/swagger/json/
	cd ${PM_MS_DIR}/../PM_GUI/src/service/console/swagger/json
	npm install
	if [ $? -ne 0 ]; then
		echo "npm install failed."
		exit 1
	fi
	
	npm run build
	if [ $? -ne 0 ]; then
		echo "npm run build failed."
		exit 1
	fi
}

function build_private(){
	
	Get_changeFiles
	check_suffix
	if [ "$json" = "true" ]; then
		build_gui_npm
	fi
	
	build_maven
	
}


json=false

build_private
