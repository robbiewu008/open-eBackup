#!/bin/bash
CURRENT_PATH=$1
version=$2
echo "server!!!${CURRENT_PATH}"
SETUP_PATH=${CURRENT_PATH}/../setup
cd "${CURRENT_PATH}" || exit 1
sed -i "/Version/d" manifest.yml
echo "Version: $version" >> manifest.yml
pyinstaller --clean -F "${CURRENT_PATH}"/__main__.py
mkdir -p "${CURRENT_PATH}"/setup/bin
mv "${CURRENT_PATH}"/dist/__main__ "${CURRENT_PATH}"/setup/bin/dpserver

# 添加版本文件
cp manifest.yml setup
# bin 目录下还需要加入verify_tool
artget pull -d down_verify_tool.xml -ap "${CURRENT_PATH}/setup/bin" -user ${cmc_user} -pwd ${cmc_pwd}
artget pull -d down_kmc_tool.xml -ap "${CURRENT_PATH}/setup/kmc" -user ${cmc_user} -pwd ${cmc_pwd}
chmod 550 ${CURRENT_PATH}/setup/bin/verify_tool

mkdir -p "${CURRENT_PATH}"/setup/temp
tar zxvf ${CURRENT_PATH}/setup/kmc/platform.tar.gz -C ${CURRENT_PATH}/setup/temp KMCv3_infra_rel/lib/libkmcv3.so
mv ${CURRENT_PATH}/setup/temp/KMCv3_infra_rel/lib/libkmcv3.so  ${CURRENT_PATH}/setup/kmc
rm -rf "${CURRENT_PATH}"/setup/temp
rm -rf ${CURRENT_PATH}/setup/kmc/platform.tar.gz

chmod 550 -R ${CURRENT_PATH}/setup/kmc
#cp verify_tool "${CURRENT_PATH}"/setup/bin/verify_tool
cp -rfT "${SETUP_PATH}" "${CURRENT_PATH}"/setup

