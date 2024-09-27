#!/bin/bash
CURRENT_PATH=$1
version=$2
echo "client path is ${CURRENT_PATH}"
cd "${CURRENT_PATH}" || exit 1
sed -i "/Version/d" manifest.yml
echo "Version: $version" >> manifest.yml
pyinstaller --clean -F "${CURRENT_PATH}"/dpclient.py
mkdir "${CURRENT_PATH}"/dpclient
cp manifest.yml dpclient
mv "${CURRENT_PATH}"/dist/dpclient "${CURRENT_PATH}"/dpclient/dpclient
cp -r "${CURRENT_PATH}"/template "${CURRENT_PATH}"/dpclient
