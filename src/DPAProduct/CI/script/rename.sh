#!/bin/bash
set -x
readonly LOCAL_PATH="$(
  cd "$(dirname "$0")"
  pwd
)"
PACKAGE_NAME="OceanCyber_${CY_VERSION}_EulerOS_ARM_64"

function make_package() {
  ISO_NAME=$(ls ${PACKAGE_NAME} | grep -i "aarch64.tgz$" | awk -F '.tgz' '{print $1}')
  if [ "${ISO_NAME}X" == "X" ]; then
    echo "[Line:${LINENO}]iso not exist"
    return 1
  fi
  cd ${PACKAGE_NAME}
  tar -zxvf "${ISO_NAME}".tgz
  mkdir -p iso
  sudo su root -c "cd ${LOCAL_PATH}/${PACKAGE_NAME};mount -o loop *.iso iso/"
  if [ $? -ne 0 ]; then
    echo "[Line:${LINENO}]mount iso failed."
    return 1
  fi
  sudo cp -rf iso/* .
  sudo mv images/pxeboot/initrd.img images/pxeboot/${OS_CODE_CY}_filesys
  sudo mv images/pxeboot/vmlinuz images/pxeboot/${OS_CODE_CY}_kernel
  cd ${LOCAL_PATH}/${PACKAGE_NAME}
  sudo su root -c "cd ${LOCAL_PATH}/${PACKAGE_NAME};umount iso/"
  sudo rm -rf iso/
  sudo rm -rf OceanCyber_EulerOS*.iso
  sudo rm -rf OceanCyber_EulerOS*.iso.*
  sudo rm -rf "${ISO_NAME}".tgz
  sudo rm -rf crldata.crl
  sudo rm -rf sha256sum_sync
  sudo rm -rf sha256sum_sync.cms
  sudo chown root:root -R ./*
  sudo su root -c "export CLOUD_BUILD_RECORD_ID=${CLOUD_BUILD_RECORD_ID};export CLOUD_BUILD_SERVER_URL=${CLOUD_BUILD_SERVER_URL};cd ${LOCAL_PATH}/${PACKAGE_NAME} && find ./ | xargs sha256sum >> ../sha256sum_sync;mv ../sha256sum_sync .;chmod 644 sha256sum_sync"
  echo "${LOCAL_PATH}/${PACKAGE_NAME} files sha256 finish"
  if [ $? -ne 0 ]; then
    echo "[Line:${LINENO}]make allsha256 failed."
    return 1
  fi
  #sign_pkgs
  sudo cp ${WORKSPACE}/DPAProduct/CI/conf/signconf.xml ../
  sudo sed -i "s#{pkg_dir}#${LOCAL_PATH}/${PACKAGE_NAME}#g"  ../signconf.xml
  cd ${SIGNATURE_HOME}
  sudo su root -c "export CLOUD_BUILD_RECORD_ID=${CLOUD_BUILD_RECORD_ID};export CLOUD_BUILD_SERVER_URL=${CLOUD_BUILD_SERVER_URL};java -jar signature.jar ${LOCAL_PATH}/${PACKAGE_NAME}/../signconf.xml"
  if [ $? -ne 0 ]; then
      echo "${LOCAL_PATH}/${PACKAGE_NAME} files Signing failed."
      exit 1
  fi
  sudo chmod 644 ${LOCAL_PATH}/${PACKAGE_NAME}/crldata.crl
  sudo chmod 644 ${LOCAL_PATH}/${PACKAGE_NAME}/sha256sum_sync.cms
  echo "${LOCAL_PATH}/${PACKAGE_NAME} sign finished."
  touch Level2ISOSoftwareName.txt
  echo "${ISO_NAME}" >Level2ISOSoftwareName.txt
  PACKAGE_NAME="OceanCyber_${CY_VERSION}_EulerOS_ARM_64"
  sudo chown root:root -R Level2ISOSoftwareName.txt
  mkdir -p ${LOCAL_PATH}/../OceanCyber/final/${OS_CODE_CY}
  sudo su root -c "cd ${LOCAL_PATH}/${PACKAGE_NAME}; zip -r ${LOCAL_PATH}/../OceanCyber/final/${OS_CODE_CY}/${PACKAGE_NAME}.zip ./*"
}
function main() {
  VERSION=$1
  if [ "X${VERSION}" == "X" ]; then
    echo "[Line:${LINENO}]VERSION is empty. build ${PACKAGE_NAME} error"
    return 1
  fi
  echo "[Line:${LINENO}]VERSION is ${VERSION}."
  PACKAGE_NAME=${PACKAGE_NAME}
  cd ${LOCAL_PATH}
  mkdir -p ${LOCAL_PATH}/${PACKAGE_NAME}
  make_package
  ret=$?
  if [ $ret -ne 0 ]; then
    echo "[Line:${LINENO}]zip ${PACKAGE_NAME}.zip failed."
    return $ret
  fi
  echo "[Line:${LINENO}]build ${PACKAGE_NAME} success."
  return 0
}
main $@
