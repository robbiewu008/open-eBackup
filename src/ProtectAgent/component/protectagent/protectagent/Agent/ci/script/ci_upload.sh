#!/bin/bash
set -ex

CUR_PATH=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
BASE_PATH=${CUR_PATH}/../../../
SIGN_TOOL_PATH=${SIGNATURE_HOME}
OPENSOURCE_REPOSITORY_DIR=${CLOUD_BUILD_WORKSPACE}/open-source-obligation

if [ -z ${branch} ]; then
    echo "Please specify build branch!"
    exit 1
fi

if [ -z "${componentVersion}" ]; then
    componentVersion="1.1.0"
fi
echo "Component Version:${componentVersion}"

if [ -z "$BUILD_PKG_TYPE" ]; then
    BUILD_PKG_TYPE=$1
fi
echo BUILD_PKG_TYPE=${BUILD_PKG_TYPE}

# 拷贝自研软件包
if [ "$BUILD_PKG_TYPE" = "OceanCyber" ]; then
    find ${BASE_PATH}/temp/ -maxdepth 1 -name "protectclient*Linux*.tar.xz"  -exec cp -f "{}" "${BASE_PATH}/final_pkg/ProtectClient-e" \;
else
    find ${BASE_PATH}/temp/ -maxdepth 1 -name "protectclient*Linux*.tar.xz"  -exec cp -f "{}" "${BASE_PATH}/final_pkg/ProtectClient-e" \;
    find ${BASE_PATH}/temp/ -maxdepth 1 -name "protectclient*.tar.gz"  -exec cp -f "{}" "${BASE_PATH}/final_pkg/ProtectClient-e" \;
    find ${BASE_PATH}/temp/ -maxdepth 1 \( -name "protectclient*.tar.xz" -a ! -name "protectclient*Linux*.tar.xz" \) -exec cp -f "{}" "${BASE_PATH}/final_pkg/ProtectClient-e" \;
    find ${BASE_PATH}/temp/ -maxdepth 1 -name "sanclient*.tar.xz" -exec cp -f "{}" "${BASE_PATH}/final_pkg/ProtectClient-e" \;
    find ${BASE_PATH}/temp/ -maxdepth 1 -name "protectclient*Windows*.zip" -exec cp -f "{}" "${BASE_PATH}/final_pkg/ProtectClient-e" \;
fi
# 复制文件，打包到ZIP
cp -rf ${BASE_PATH}/Agent/ci/script/package.json ${BASE_PATH}/

# 修改package.json 版本号
sh ${BASE_PATH}/Agent/ci/script/set_package_version.sh ${BASE_PATH}/package.json  ${MS_IMAGE_TAG} ${Version}

#sha256 files
mv ${BASE_PATH}/package.json ${BASE_PATH}/final_pkg/
cd ${BASE_PATH}/final_pkg/ && find -type f | xargs sha256sum >../sha256sum_sync
mv ../sha256sum_sync .
echo "agent files sha256 finish"
#sign_pkgs
if [ "$BUILD_PKG_TYPE" = "OpenSource" ]; then
    echo "Do not sign."
else
    SHA_PATH="${BASE_PATH}/final_pkg/"
    cp ${BASE_PATH}/Agent/ci/signconf.xml ${SHA_PATH}/..
    sed -i "s#{pkg_dir}#${SHA_PATH}#g" ${SHA_PATH}/../signconf.xml
    cd ${SIGN_TOOL_PATH}
    java -jar signature.jar ${SHA_PATH}/../signconf.xml
    if [ $? -ne 0 ]; then
        echo "${SHA_PATH} files Signing failed."
        exit 1
    fi
    echo "${SHA_PATH} sign finished."
fi

cd ${BASE_PATH}/final_pkg
zip -r DataProtect_${Version}_client.zip ./*

#上传CMC组件
function upload_artifact() {
    cd ${BASE_PATH}/Agent/ci/LCRP/conf
    if [ "${BUILD_TYPE}" == "ASAN" ];then
        PKG_TYPE=mspkg_asan
    else
        PKG_TYPE=mspkg
    fi
    if [ "$BUILD_PKG_TYPE" = "OceanCyber" ]; then
        artget push -d OceanCyber_pkg_from_cmc.xml -p "{'componentVersion':'${componentVersion}','AGENT_BRANCH':'${branch}','Version':'${Version}', 'PKG_TYPE':'${PKG_TYPE}'}" -ap ${BASE_PATH}/final_pkg -user ${cmc_user} -pwd ${cmc_pwd}
    elif [ "$BUILD_PKG_TYPE" = "OpenSource" ]; then
    mkdir -p ${OPENSOURCE_REPOSITORY_DIR}/${PKG_TYPE}/linux/
        cp ${BASE_PATH}/final_pkg/DataProtect_${Version}_client*.zip ${OPENSOURCE_REPOSITORY_DIR}/${PKG_TYPE}/linux/
    else
        artget push -d pkg_from_cmc.xml -p "{'componentVersion':'${componentVersion}','AGENT_BRANCH':'${branch}','Version':'${Version}', 'PKG_TYPE':'${PKG_TYPE}'}" -ap ${BASE_PATH}/final_pkg -user ${cmc_user} -pwd ${cmc_pwd}
    fi
    
    if [ $? -ne 0 ]; then
        echo "Upload artifact to CMC error"
        exit 1
    fi
}
#上传B版本
function upload_Bversion() {
    cd ${BASE_PATH}/Agent/ci/LCRP/conf
    artget push -d bversion.xml -p "{'PBI_NAME':'${PBI_NAME}', 'IS_SNAPSHOT':'${IS_SNAPSHOT}', 'Version':'${Version}'}" -ap ${BASE_PATH}/final_pkg -user ${cmc_user} -pwd ${cmc_pwd}
    if [ $? -ne 0 ]; then
        echo "Upload artifact to ${PBI_NAME} error"
        exit 1
    fi
}
# 上传到大包组件版本
function upload_Component_Version() {
    if [ "$BUILD_PKG_TYPE" = "OceanCyber" ]; then
        XMLFILE=OceanCyber_pkg_from_cmc.xml
    else
        XMLFILE=pkg_from_cmc.xml
    fi
    componentVersion=${MS_IMAGE_TAG}
    Version=${Version}
    componentName=${Version}_A8000_ocean_storage
    cd ${BASE_PATH}/Agent/ci/LCRP/conf
    sed -i '/<dependencies\>/,/<\/dependencies>/d' ${XMLFILE}
    sed -i 's#<componentName>.*#<componentName>${componentName}</componentName>#g' ${XMLFILE}
    sed -i 's#<dest>.*#<dest>.</dest>#g' ${XMLFILE}
    if [ "$BUILD_PKG_TYPE" = "OceanCyber" ]; then
        artget push -d OceanCyber_pkg_from_cmc.xml -p "{'componentVersion':'${componentVersion}','componentName':'${componentName}','Version':'${Version}'}" -ap ${BASE_PATH}/final_pkg -user ${cmc_user} -pwd ${cmc_pwd}
    else
        artget push -d pkg_from_cmc.xml -p "{'componentVersion':'${componentVersion}','componentName':'${componentName}','Version':'${Version}'}" -ap ${BASE_PATH}/final_pkg -user ${cmc_user} -pwd ${cmc_pwd}
    fi
}

upload_artifact
if [ "${BUILD_TYPE}" == "ASAN" ];then
   cd ${BASE_PATH}/final_pkg
   mv DataProtect_${Version}_client.zip DataProtect_${Version}_client_asan.zip
fi

# hwp7s后缀签名
if [ "$BUILD_PKG_TYPE" = "OpenSource" ]; then
    echo "Do not sign."
else
    ed -i "s#{fileset_path}#${BASE_PATH}/final_pkg#g" ${BASE_PATH}/Agent/ci/product_signconf.xml
    sed -i "s#{pkg_dir}#${SHA_PATH}#g" ${BASE_PATH}/Agent/ci/product_signconf.xml
    cd ${SIGN_TOOL_PATH}
    java -jar signature.jar ${BASE_PATH}/Agent/ci/product_signconf.xml
fi

if [ "${IS_ComponentVersion}" == "N" ] && [ "$BUILD_PKG_TYPE" != "OceanCyber" ] && [ "$BUILD_PKG_TYPE" != "OpenSource" ];then
    upload_Bversion
    if [ $? -ne 0 ]; then
        echo "Upload artifact to ${PBI_NAME} error"
        exit 1
    fi
elif [ "${IS_ComponentVersion}" == "Y" ] && [ "$BUILD_PKG_TYPE" != "OpenSource" ];then
    upload_Component_Version
    if [ $? -ne 0 ]; then
        echo "Upload artifact to ${Version}_A8000_ocean_storage error"
        exit 1
    fi
else
    echo "Do not need upload to cmc"
    exit 0
fi