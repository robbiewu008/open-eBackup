#!/bin/bash
export PYTHONHOME=$PYTHON_HOME
BASE_PATH=$(
  cd $(dirname $0)
  pwd
)
echo ${BASE_PATH}
version=$1
echo ${version}
SERVER_BUILD_PATH=${BASE_PATH}/server
CLIENT_BUILD_PATH=${BASE_PATH}/client
FINAL_PKGPATH="${BASE_PATH}"/../../final_pkg
SIGN_TOOL_PATH=${SIGNATURE_HOME}
SIGN_FILE_PATH=${BASE_PATH}/../../../../DPAProduct/
pip3 install -r "${BASE_PATH}"/requirements.txt
echo "start"
function sign_pkgs() {
  #sha256 files
  SHA_PATH=$1
  cd ${SHA_PATH} && find ./ | xargs sha256sum >>../sha256sum_sync
  mv ../sha256sum_sync .
  chmod 644 sha256sum_sync
  echo "${SHA_PATH} files sha256 finish"

  #sign_pkgs
  cp ${SIGN_FILE_PATH}/CI/conf/signconf.xml ${SHA_PATH}/../signconf.xml
  sed -i "s#{pkg_dir}#${SHA_PATH}#g" ${SHA_PATH}/../signconf.xml
  cd ${SIGN_TOOL_PATH}
  java -jar signature.jar ${SHA_PATH}/../signconf.xml
  if [ $? -ne 0 ]; then
    echo "${SHA_PATH} files Signing failed."
    exit 1
  fi
  chmod 644 ${SHA_PATH}/crldata.crl
  chmod 644 ${SHA_PATH}/sha256sum_sync.cms
  echo "${SHA_PATH} sign finished."
  return 0
}

sh "${SERVER_BUILD_PATH}"/build.sh "${SERVER_BUILD_PATH}" "${version}"

sign_pkgs ${SERVER_BUILD_PATH}/setup
cd ${SERVER_BUILD_PATH}/setup
tar zcvf ../dpserver.tar.gz .

echo "server done"
sh "${CLIENT_BUILD_PATH}"/build.sh "${CLIENT_BUILD_PATH}" "${version}"
echo "client done"

sign_pkgs ${CLIENT_BUILD_PATH}/dpclient
cd ${CLIENT_BUILD_PATH}
tar zcvf dpclient_linux.tar.gz dpclient


function product_sign() {
  # hwp7s后缀签名
  umask 0022
  local file_path=$1
  local all_product_files=$(cd ${file_path} && ls)
  for product_file in ${all_product_files}; do
    local path=$file_path/${product_file}
    if [ -d ${path} ]; then
      product_sign $path
    else
      cp -f ${SIGN_FILE_PATH}/CI/conf/product_signconf.xml ${FINAL_PKGPATH}/product_signconf.xml
      sed -i "s#{fileset_path}#${file_path}#g;s#{filename}#${product_file}#g" ${FINAL_PKGPATH}/product_signconf.xml
      cd ${SIGN_TOOL_PATH}
      java -jar signature.jar ${FINAL_PKGPATH}/product_signconf.xml
      if [ $? -ne 0 ]; then
        echo "${product_file} Product Signing failed."
        exit 1
      fi
    fi
  done
}

mkdir "${BASE_PATH}"/dpdeploy
mkdir ${FINAL_PKGPATH}
DPDEPLOY_PATH="$BASE_PATH"/dpdeploy
cp "${SERVER_BUILD_PATH}"/dpserver.tar.gz ${FINAL_PKGPATH}
cp "${CLIENT_BUILD_PATH}"/dpclient_linux.tar.gz ${FINAL_PKGPATH}

cd "${FINAL_PKGPATH}"
chmod o+rx -R ${FINAL_PKGPATH}
product_sign ${FINAL_PKGPATH}
rm -rf ${FINAL_PKGPATH}/product_signconf.xml
artget push -d ${BASE_PATH}/../../conf/dpserver_to_cmc.xml -p "{'componentVersion':'${componentVersion}','CODE_BRANCH':'${branch}'}" -ap ${FINAL_PKGPATH}/ -user ${cmc_user} -pwd ${cmc_pwd}