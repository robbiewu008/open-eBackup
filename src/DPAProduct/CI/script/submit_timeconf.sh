#!bin/bash
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
CUR_PATH=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
BASE_PATH=${CUR_PATH}/../..

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
