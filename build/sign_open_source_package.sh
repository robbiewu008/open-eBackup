#!/bin/bash
set -e

if [ $# -ne 3 ]; then
    echo "Usage: "
    echo "    ./sign_open_source_package.sh <open-eBackup-package> <ROOT_KEY> <ROOT_CRT>"
    exit 1
fi

OPEN_EBACKUP_PACKAGE=$1
ROOT_KEY=$(realpath $2)
ROOT_CRT=$(realpath $3)

unzip $OPEN_EBACKUP_PACKAGE

MASTER_SERVER_PACKAGE=open-eBackup_*_MasterServer.tgz
MASTER_SERVER_CHART=open-eBackup_MasterServer_chart.tgz
MEDIA_SERVER_PACKAGE=open-eBackup_*_MediaServer.tgz
MEDIA_SERVER_CHART=open-eBackup_MediaServer_chart.tgz

mkdir -p master_server media_server
tar xvf $MASTER_SERVER_PACKAGE -C master_server
tar xvf $MEDIA_SERVER_PACKAGE -C media_server

cd master_server
mkdir -p image chart
tar xvf $MASTER_SERVER_PACKAGE -C image
tar xvf $MASTER_SERVER_CHART -C chart

cd image
find . -type f -exec sha256sum {} >> sha256sum_sync \;
openssl cms -sign -in sha256sum_sync -out sha256sum_sync.cms -inkey $ROOT_KEY -signer $ROOT_CRT -outform PEM
tar zcvf ../$MASTER_SERVER_PACKAGE *

cd ../chart
find . -type f -exec sha256sum {} >> sha256sum_sync \;
openssl cms -sign -in sha256sum_sync -out sha256sum_sync.cms -inkey $ROOT_KEY -signer $ROOT_CRT -outform PEM
tar zcvf ../$MASTER_SERVER_CHART *

cd ..
tar zcvf ../$MASTER_SERVER_PACKAGE *.tgz

cd ../media_server
mkdir -p image chart
tar xvf $MEDIA_SERVER_PACKAGE -C image
tar xvf $MEDIA_SERVER_CHART -C image

cd image
find . -type f -exec sha256sum {} >> sha256sum_sync \;
openssl cms -sign -in sha256sum_sync -out sha256sum_sync.cms -inkey $ROOT_KEY -signer $ROOT_CRT -outform PEM
tar zcvf ../$MEDIA_SERVER_PACKAGE *

cd ../chart
find . -type f -exec sha256sum {} >> sha256sum_sync \;
openssl cms -sign -in sha256sum_sync -out sha256sum_sync.cms -inkey $ROOT_KEY -signer $ROOT_CRT -outform PEM
tar zcvf ../$MEDIA_SERVER_CHART *

cd ..
tar zcvf ../$MEDIA_SERVER_PACKAGE *.tgz

cd ..
zip $OPEN_EBACKUP_PACKAGE $MASTER_SERVER_PACKAGE $MEDIA_SERVER_PACKAGE

rm -f $MASTER_SERVER_PACKAGE $MEDIA_SERVER_PACKAGE
rm -r master_server media_server

echo "Sign Ok"
