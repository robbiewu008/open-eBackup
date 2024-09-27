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
BASE_PATH=${CUR_PATH}/../..

cd ${BASE_PATH}/
commit_address=$(git remote -v | grep fetch | awk '{print $2}')
commit_branch=$(git branch | awk -F ' ' '{print $2}')
commit_id=$(git log -1 | grep commit | awk '{print $2}')
commit_name="A8000_CI/component/PM_CI"
echo "<fileidentify repoBase=\"${commit_address}\" repoType=\"git\"  localpath=\"${commit_name}\" branch=\"${commit_branch}\" revision=\"${commit_id}\" />" > PM_version

cd ${BASE_PATH}/component
DIR=`ls`
for i in ${DIR[@]}
	do
        if [ -d ${i} ]; then
			cd ${i}
			commit_address=$(git remote -v | grep fetch | awk '{print $2}')
			commit_name="A8000_CI/component/PM_CI/component/${i}"
			commit_branch=$(git branch | awk -F ' ' '{print $2}')
			commit_id=$(git log -1 | grep commit | awk '{print $2}')
			echo "<fileidentify repoBase=\"${commit_address}\" repoType=\"git\"  localpath=\"${commit_name}\" branch=\"${commit_branch}\" revision=\"${commit_id}\" />" >> ${BASE_PATH}/PM_version
			cd ${BASE_PATH}/component
		fi
done
