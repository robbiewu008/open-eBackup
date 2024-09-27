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
BASE_PATH=${CUR_PATH}/../..
function compile(){
	cd ${CUR_PATH}/
	python3 setup.py sdist bdist_wheel
	if [ $? -ne 0 ]; then
		echo "opensource apscheduler compile failed."
		exit 1
	fi
	echo "${CUR_PATH}/dist contains:"
	ls -l ${CUR_PATH}/dist
	 [ -d ${BASE_PATH}/output/ ] && rm -rf ${BASE_PATH}/output/
	mkdir -p ${BASE_PATH}/output/
	cp -rf ${CUR_PATH}/dist/* ${BASE_PATH}/output/
	
}
function upload(){
	cd ${CUR_PATH}/../
	sh upload_py.sh apscheduler
}
function main(){
	compile
	upload
}
echo "#########################################################"
echo "   Begin compile opensource apscheduler "
echo "#########################################################"
main
echo "#########################################################"
echo "   apscheduler Compile Success  "
echo "#########################################################"
