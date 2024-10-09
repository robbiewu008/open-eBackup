#!/bin/sh
set +x
# compile script
SYS_NAME=`uname -s`
BASEDIR=""
if [ "${SYS_NAME}" = "AIX" ]; then
    BASEDIR="$(cd "$(dirname $0)" && pwd)"
else
    BASEDIR=$(cd $(dirname ${BASH_SOURCE[0]}); pwd)
fi

script_dest_path=$1
conf_dest_path=$2

cp -rp "${BASEDIR}/../../applications/." $script_dest_path
cp -rp "${BASEDIR}/../conf/." $conf_dest_path

#make any user can build package and remove file
chmod -R 777 "$script_dest_path"
rm -rf "$script_dest_path/build"
rm -rf "$script_dest_path/conf"
rm -rf "$script_dest_path/test"

#make install package mini permission
chmod -R 550 "$script_dest_path"
