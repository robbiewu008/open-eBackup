#!/bin/sh

MAKE_JOB="-j4"


if [ "$1" = "" ] && [ -f "${AGENT_ROOT}/platform/kmc/lib/libKMC.a" ]
then
    exit 0
fi

if [ "$1" = "clean" ]
then
    cd ${AGENT_ROOT}/platform/kmc
    make $MAKE_JOB -f makefile $1
    cd -
    sh ${AGENT_ROOT}/build/securec/securec_make.sh $1
    exit 0
fi

if [ ! -d "${AGENT_ROOT}/platform/kmc/lib/" ]; then
    mkdir -p "${AGENT_ROOT}/platform/kmc/lib/"
fi
# compile libsecurec.a
sh ${AGENT_ROOT}/build/securec/securec_make.sh
if [ "$?" != "0" ]
then
    echo "\033[31mCompile libsecurec.a failed.\033[0m"
    exit 1
fi

cd ${AGENT_ROOT}/platform/kmc
cp ${AGENT_ROOT}/build/kmc/makefile ./
cp ${AGENT_ROOT}/build/kmc/makefile_imp ./

if [ "`uname`" = "AIX" ]; then
    cp ${AGENT_ROOT}/build/kmc/conf.config_AIX ./conf.config
elif [ "`uname`" = "SunOS" ]; then
    cp ${AGENT_ROOT}/build/kmc/conf.config_SunOS ./conf.config
elif [ "`uname`" = "HP-UX" ]; then
    cp ${AGENT_ROOT}/build/kmc/conf.config_HPUX ./conf.config
else
    cp ${AGENT_ROOT}/build/kmc/conf.config ./

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
fi
gmake $MAKE_JOB -f makefile "SDP"
if [ "$?" != "0" ]
then
    echo "\033[31mCompile libKMC.a failed.\033[0m"
    exit 1
fi
