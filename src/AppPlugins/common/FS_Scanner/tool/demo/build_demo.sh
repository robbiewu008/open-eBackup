#!/bin/bash
echo "ARGS = $@"
SCRIPT_PATH=$(cd $(dirname $0); pwd)
echo "${SCRIPT_PATH}"
ROOT_PATH=${SCRIPT_PATH}/../..
BUILD_PATH=${ROOT_PATH}/build
MODULE_ROOT_PATH=${ROOT_PATH}/Module
RELEASE_PATH=${SCRIPT_PATH}/bin
source ${BUILD_PATH}/common/branch.sh

function compile_scanner()
{
    if [ ! -f ${ROOT_PATH}/ext_pkg/Module_rel.tar.gz ]; then
        sh ${BUILD_PATH}/download_module_from_cmc.sh ${MODULE_BRANCH}
        if [ $? -ne 0 ]; then
            echo "ERROR: download module failed"
            exit 1
        fi
    fi

    mkdir -p "${ROOT_PATH}/build-cmake-file"
    cd "${ROOT_PATH}/build-cmake-file"
    COMPILE_PARA=" ${COMPILE_PARA} -D FILE=ON $@ -D MODULE_ROOT_DIR=${MODULE_ROOT_PATH} "
    echo "INFO: cmake ${COMPILE_PARA} .."
    cmake ${COMPILE_PARA} ..
    numProc=$(cat /proc/cpuinfo | grep processor | wc -l)
    make -j${numProc}
    if [ $? -ne 0 ]; then
        echo "ERROR: make error"
        exit 1
    fi

    echo "INFO: Compile backup successfully"
    return 0
}

compile_scanner

echo "scanner root path is ${ROOT_PATH}"
rm -rf ${RELEASE_PATH}
mkdir -p $RELEASE_PATH
echo "copy scanner demo dependency to ${RELEASE_PATH}"
find $ROOT_PATH -name "*.so*" | xargs -i cp {} $RELEASE_PATH

export LD_LIBRARY_PATH=$RELEASE_PATH

mkdir -p ${SCRIPT_PATH}/build-cmake
cd ${SCRIPT_PATH}/build-cmake
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DFILE=ON $@ .. && make -j16
if [ $? -ne 0 ];then
    echo "make scanner demo error"
    exit 1
fi

find $RELEASE_PATH | xargs -i chmod 550 {}
echo "Done."

if [ "X$1" == "Xrun" ];then
# start
$RELEASE_PATH/ScannerDemo
elif [ "X$1" == "Xgdb" ];then
gdb $RELEASE_PATH/ScannerDemo
fi

exit 0