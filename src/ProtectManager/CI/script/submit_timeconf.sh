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
T_CODE_BRANCH=$1

if [ -z ${T_CODE_BRANCH} ];then
    echo "No branch parameter, please specify"
    exit 1
fi

echo master > ${BASE_PATH}/CI/conf/main_branch.txt

while read line;
do
   if [ "$T_CODE_BRANCH" = "$line" ]; then
      git checkout $T_CODE_BRANCH
      git pull origin $T_CODE_BRANCH
      cd ${BASE_PATH}/CI/conf
      bep_env.sh -c bep_env.conf
      git add ${BASE_PATH}/CI/conf/bep_env.conf
	  git commit -m "
[AR/SR/Story/Defects/CR] UADP123456; submit bep_env.conf
[ModifyDesc] submit bep_env.conf
[Author/ID] zhangling WX538034"
      git push origin $T_CODE_BRANCH
    else
       echo "this branch is not main_branch,so do not need to submit bep_env.conf"
   fi 
done < ${BASE_PATH}/CI/conf/main_branch.txt
