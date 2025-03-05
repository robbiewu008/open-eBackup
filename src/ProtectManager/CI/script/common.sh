#!/bin/bash
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
BASE_PATH="$(
        cd "$(dirname "$BASH_SOURCE")/../../"
        pwd
    )"

if [ -z "${MS_IMAGE_TAG}" ]; then
    echo "MS_IMAGE_TAG does not exist."
    exit 1
fi

NOB_VERSION=${Version}  # 不带B版本号，通过流水线设置的环境变量获取
echo "MS_IMAGE_TAG=${MS_IMAGE_TAG}"
echo "NOB_VERSION=${NOB_VERSION}"

BUILD_PATH="${BASE_PATH}/build/dockerfiles"
build_file=$(ls ${BUILD_PATH} | grep ".name" )

for h in ${build_file}; do
  sed -i "s#VERSION#${MS_IMAGE_TAG}#g" ${BUILD_PATH}/$h
done

build_docker_file=$(ls ${BUILD_PATH} | grep ".dockerfile" )

for h in ${build_docker_file}; do
  sed -i "s#pm-app-common:VERSION#pm-app-common:${MS_IMAGE_TAG}#g" ${BUILD_PATH}/$h
done

chart_file="${BASE_PATH}/build/helm/protect-manager/Chart.yaml"
values_file="${BASE_PATH}/build/helm/protect-manager/values.yaml"

system_base_dockerfile="${BASE_PATH}/build/dockerfiles/PM_System_Base_Service_opensource.dockerfile"

modify_version_files=(${chart_file} ${values_file} ${chart_dev_file} ${values_dev_file} ${system_base_dockerfile} \
                      ${system_base_dev_dockerfile})
for file in ${modify_version_files[*]}
do
  sed -i "s#\${NewAppVersion}#${NOB_VERSION}#g" "$file"
done
