#!/bin/bash
BASE_PATH="$(
        cd "$(dirname "$BASH_SOURCE")/../../"
        pwd
    )"

if [ -z "${MS_IMAGE_TAG}" ]; then
    echo "MS_IMAGE_TAG does not exist."
    exit 1
fi

NOB_VERSION=$(echo "$MS_IMAGE_TAG" | grep -oE '(\w+.){2}(\w+)' | head -n 1)
echo "MS_IMAGE_TAG=${MS_IMAGE_TAG}"
echo "NOB_VERSION=${NOB_VERSION}"

BUILD_PATH="${BASE_PATH}/build/dockerfiles"
BUILD_DEV_PATH="${BASE_PATH}/build-dev/dockerfiles"
build_file=$(ls ${BUILD_PATH} | grep ".name" )
build_dev_file=$(ls ${BUILD_DEV_PATH} | grep ".name" )

for h in ${build_file}; do
  sed -i "s#VERSION#${MS_IMAGE_TAG}#g" ${BUILD_PATH}/$h
done

for h in ${build_file}; do
  sed -i "s#VERSION#${MS_IMAGE_TAG}#g" ${BUILD_DEV_PATH}/$h
done

build_docker_file=$(ls ${BUILD_PATH} | grep ".dockerfile" )
build_dev_docker_file=$(ls ${BUILD_DEV_PATH} | grep ".dockerfile" )

for h in ${build_docker_file}; do
  sed -i "s#pm-app-common:VERSION#pm-app-common:${MS_IMAGE_TAG}#g" ${BUILD_PATH}/$h
done

for h in ${build_dev_docker_file}; do
  sed -i "s#pm-app-common:VERSION#pm-app-common:${MS_IMAGE_TAG}#g" ${BUILD_DEV_PATH}/$h
done

chart_file="${BASE_PATH}/build/helm/protect-manager/Chart.yaml"
values_file="${BASE_PATH}/build/helm/protect-manager/values.yaml"

chart_dev_file="${BASE_PATH}/build-dev/helm-dev/protect-manager/Chart.yaml"
values_dev_file="${BASE_PATH}/build-dev/helm-dev/protect-manager/values.yaml"

system_base_dockerfile="${BASE_PATH}/build/dockerfiles/PM_System_Base_Service.dockerfile"
system_base_dev_dockerfile="${BASE_PATH}/build-dev/dockerfiles/PM_System_Base_Service.dockerfile"

modify_version_files=(${chart_file} ${values_file} ${chart_dev_file} ${values_dev_file} ${system_base_dockerfile} \
                      ${system_base_dev_dockerfile})
for file in ${modify_version_files[*]}
do
  sed -i "s#\${NewAppVersion}#${NOB_VERSION}#g" "$file"
done
