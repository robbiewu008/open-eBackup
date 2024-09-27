#!/bin/sh
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

Replcace_Script_Path=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
Conf_File=${Replcace_Script_Path}/build.properties

function Replace(){
	if [ $# -eq 0 ]; then
		return
	fi
	rawpath="$1"
	
	#获取目标文件中所有占位符
	placeholders=$(grep -oE '@{[^}]*}' "$rawpath" | sort | uniq | sed '/^ *$/d')
	echo "$placeholders" | while read placeholder
	do
		if [ "$placeholder" = "" ]; then
			continue
		fi
		#获取占位符的键
		key=$(echo "$placeholder"|sed 's/^@{\|}$//g')
		#key=$(echo "$exp"|grep -oE '^[^=/]*')
		echo "$placeholder" "$key" "$rawpath"
		#获取占位符的值
		version=$(ReadConfig "$key")
		echo version=${version}
		if [ "${version}" == "" ]; then
			echo "get version failed!"
			exit 1
		else
			ReplaceWord "$placeholder" "$version" "$rawpath"
		fi
		
	done
}

function ReadConfig(){
	key=$1
	if [ "${key}" == "" ]; then
		echo "key is empty!"
		return
	fi

	value=$(cat ${Conf_File} | grep ${key} | awk -F "=" '{print $2}')
	echo ${value}

}

function ReplaceWord(){
	SOURCE="$1"
	TARGET="$2"
	file="$3"
	if [ ! -f "${file}" ]; then
		echo "file $3 is not exists"
		return 1
	fi

	sed -i 's/'"${SOURCE}"'/'"${TARGET}"'/g' "${file}"

}
