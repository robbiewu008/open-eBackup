#!/bin/sh
 
MAKE_JOB="-j4"
 
 
if [ "$1" = "" ] && [ -f "${AGENT_ROOT}/platform/kmcv1/lib/libKMC.a" ]
then
    exit 0
fi
 
if [ "$1" = "clean" ]
then
    cd ${AGENT_ROOT}/platform/kmcv1
    make $MAKE_JOB -f makefile $1
    exit 0
fi
 
if [ ! -d "${AGENT_ROOT}/platform/kmcv1/lib/" ]; then
    mkdir -p "${AGENT_ROOT}/platform/kmcv1/lib/"
fi
 
cd ${AGENT_ROOT}/platform/kmcv1
cp -rf ${AGENT_ROOT}/build/kmcv1/makefile ./
cp -rf ${AGENT_ROOT}/build/kmcv1/makefile_imp ./
 
cp -rf ${AGENT_ROOT}/build/kmcv1/conf.config ./
 
if [ -f "/etc/SuSE-release" ];then
    cat /etc/SuSE-release | grep "VERSION = 10"
    if [ $? -eq 0 ]; then
        # gcc isn't support this parameter, so delete it
        sed -i "/-Warray-bounds/d" conf.config
        sed -i "/-Wstrict-overflow=1/d" conf.config
    fi
fi
 
platform_type=`uname -m`
if [ $platform_type = "aarch64" ]; then
    sed -i "s/PLATFOMR_ARM = n/PLATFOMR_ARM = y/g" conf.config
fi
gmake $MAKE_JOB -f makefile "SDP"
if [ "$?" != "0" ]; then
    echo "\033[31mCompile libKMC.a failed.\033[0m"
    exit 1
fi