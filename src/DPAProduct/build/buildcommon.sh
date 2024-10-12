export G_FIST_BUILD_DOCKERS="dme_3rd.dockerfile dme_nginx.dockerfile om.dockerfile dee_common.dockerfile PM_App_Common_Lib.dockerfile"
mkdir -p "$(dirname "$BASH_SOURCE")/../pkg"
export G_BASE_DIR="$(cd "$(dirname "$BASH_SOURCE")/../";pwd)"
# G_VERSION=$(cat ${G_BASE_DIR}/build/version | grep "G_VERSION" | awk -F "\"" '{print $2}')
INTERNAL_VERSION=${INTERNAL_VERSION}
PATCH_VERSION=${PATCH_VERSION}
if [ "X${INTERNAL_VERSION}" == "X" ];then
    echo "please check parm INTERNAL_VERSION"
    exit 1
fi

if [ "X${PATCH_VERSION}" == "X" ];then
    PATCH_VERSION=$(cat ${G_BASE_DIR}/build/version | grep "PATCH_VERSION" | awk -F "\"" '{print $2}')
fi
if [ "${BUILD_PKG_TYPE}" == "OceanCyber" ];then
    PKG_PATH_NAME=${BUILD_PKG_TYPE}
    PBI_NAME_LAST=${PBI_NAME_CY}
    G_VERSION=${CY_VERSION}
    PRODUCT=${BUILD_PKG_TYPE}
    OFFERING="OceanCyber 300"
    LAST_MS_TAG=${MS_IMAGE_TAG_CY}
    PRODUCT_CODE_LAST=${PRODUCT_CODE_CY}
    OS_CODE_LAST=${OS_CODE_CY}
    PLATFORM_CODE_LAST=${PLATFORM_CODE_CY}
    PRODUCT_IMAGE_PATH="oceancyber"
elif [ "${BUILD_PKG_TYPE}" == "OpenSource" ];then
    PKG_PATH_NAME=open-eBackup
    PBI_NAME_LAST=${PBI_NAME}
    G_VERSION=${Version}
    PRODUCT=open-eBackup
    OFFERING="OceanProtect DataBackup"
    LAST_MS_TAG=${MS_IMAGE_TAG}
else
    PKG_PATH_NAME=DataProtect
    PBI_NAME_LAST=${PBI_NAME}
    G_VERSION=${Version}
    PRODUCT=OceanProtect_DataProtect
    OFFERING="OceanProtect DataBackup"
    LAST_MS_TAG=${MS_IMAGE_TAG}
    PRODUCT_CODE_LAST=${PRODUCT_CODE}
fi

arch_type=$(uname -m)
if [ "$arch_type" == "aarch64" ]; then
    ARCH="ARM_64"
else
    ARCH="X86_64"
fi

SIGN_TOOL_PATH=${SIGNATURE_HOME}

#基于哪个镜像tag号做补丁
initial_image_version=$(cat ${G_BASE_DIR}/CI/patch/initial_image_version)
#补丁镜像tag号，流水线传入
patch_image_version=${MS_IMAGE_TAG}
#数字版本号，型式：8000xxxyyy，流水线传入
digitalVersion=${digitalVersion}
