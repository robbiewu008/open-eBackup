#!/bin/bash

if [ -z ${LCRP_HOME} ]; then
  echo "Cant't find LCRP_HOME env, please config LCRP tool first"
  exit 1
fi

PRODUCT=$1
CODE_BRANCH=$2
PKG=$3
COMPONENT_TYPE=$4

if [ -z "${componentVersion}" ]; then
  componentVersion="1.1.0"
fi

echo "Product name ${PRODUCT}"
echo "Use branch ${CODE_BRANCH}"
echo "Component type ${COMPONENT_TYPE}"
echo "Component Version:${componentVersion}"

if [ -z "${PRODUCT}" -o -z "${CODE_BRANCH}" -o -z "${COMPONENT_TYPE}" ]; then
  echo "Some variable is empry, please check"
  exit 1
fi

initEnv() {
  BASE_PATH="$(
    cd "$(dirname "$BASH_SOURCE")"
    pwd
  )"

  local arch_type=$(uname -m)
  if [ "$arch_type" == "aarch64" ]; then
    ARCH="euler-arm"
  else
    ARCH="euler-x86"
  fi
}

download_artifact() {

  artget pull -d pkg_from_cmc.xml -p "{'componentVersion':'${componentVersion}','PRODUCT':'${PRODUCT}', 'CODE_BRANCH':'${CODE_BRANCH}','COMPONENT_TYPE':'${COMPONENT_TYPE}', 'ARCH':'${ARCH}', 'PKG': '${PKG}'}" -ap ${BASE_PATH} -user ${cmc_user} -pwd ${cmc_pwd}
  if [ $? -ne 0 ]; then
    echo "Download artifact from cmc error"
    exit 1
  fi

  echo "After download ${BASE_PATH}/pkg/${COMPONENT_TYPE}:"
  ls -l ${BASE_PATH}
}

main() {
  initEnv
  download_artifact
}

echo "#########################################################"
echo "   Begin down ${CODE_BRANCH}/${PRODUCT}/${COMPONENT_TYPE} pkg from cmc"
echo "#########################################################"

main

echo "#########################################################"
echo "   Success down ${CODE_BRANCH}/${PRODUCT}/${COMPONENT_TYPE} pkg from cmc"
echo "#########################################################"
