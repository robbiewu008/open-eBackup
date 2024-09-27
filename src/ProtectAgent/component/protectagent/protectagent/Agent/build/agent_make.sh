#!/bin/sh
# This file is a part of the open-eBackup project.
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file, You can obtain one at
# http://mozilla.org/MPL/2.0/.
#
# Copyright (c) [2024] Huawei Technologies Co.,Ltd.
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.

CLEAN=0
CLEAN_ALL=0
FORTIFY=0
DATAPROCESS=0
NO_OPENSRC=0
AGENT=0
XBSA=0
CPPC=
CC=
cc=
CFLAGS=
cFLAGS=
OFLAGS=
DFLAGS=
OS_NAME=
NGINX_CPU_OPT=
NGINX_CPU_OPT_FLAG=0
#Linux version 2.6.29 support the file system freeze and thaw
FREEZE_VERSION=2629
#0 not support the freeze, 1 support
FREEZE_SUPPORT=0

#for module system test's coverage
MST_COVERAGE=0
COV_CFLAGS=
COV_OFLAGS=

# for publish private REST interfaces
REST_PUBLISH=0
HiTest_Flage=0
SANCLIENT_PLUIN=0

# for asan and tsan
ASAN=0
TSAN=0
ASAN_CFLAGS=
TSAN_CFLAGS=
ASAN_LDFLAGS=
TSAN_LDFLAGS=

sys=`uname -s`
sys_type=`uname -m`

if [ "$sys" = "Linux" ] || [ "$sys" = "AIX" ] || [ ""$sys"" = "SunOS" ]; then
    MAKE_JOB="-j 4"
else
    MAKE_JOB=""
fi

if [ "$sys" = "SunOS" ]
then
    AWK=nawk
else
    AWK=awk
fi
SYS_PLATFORM=`uname -p`

#gcc secure opt
NX_OPT="-Wl,-z,noexecstack"
SP_OPT="-fstack-protector-all --param ssp-buffer-size=4 -Wstack-protector"
RELRO_OPT="-Wl,-z,relro"
RPATH_OPT="-Wl,--disable-new-dtags" 
BIND_NOW_OPT="-Wl,-z,now"
PIE_OPT="-pie"
SYM_TABLE="-s"
IS_RELEASE=1
if [ ${IS_RELEASE} -eq 1 ]; then
    VERSION=""
else
    VERSION="-g"
fi

GenerateAgentUpdateVersion()
{
    if [ "$sys" = "Linux" ]; then
        currentDate=`date "+%Y-%m-%d %H:%M:%S"`
        AGENT_UPDATE_VERSION=`date -d "$currentDate" +%s`
        sed -i "s|AGENT_UPDATE_VERSION = .*;|AGENT_UPDATE_VERSION = ${AGENT_UPDATE_VERSION};|" ${AGENT_ROOT}/src/inc/common/AppVersion.h
    else
        AGENT_UPDATE_VERSION=`perl -e "print time"`
        TMP_FILE=${AGENT_ROOT}/src/inc/common/AppVersion.h.bak
        sed "s/AGENT_UPDATE_VERSION = .*;/AGENT_UPDATE_VERSION = ${AGENT_UPDATE_VERSION};/" ${AGENT_ROOT}/src/inc/common/AppVersion.h > ${TMP_FILE}
        mv ${TMP_FILE} ${AGENT_ROOT}/src/inc/common/AppVersion.h
    fi
}

AGENT_VERSION=`cat ${AGENT_ROOT}/src/inc/common/AppVersion.h | grep "mp_string AGENT_VERSION" | $AWK -F '"' '{print $2}'`
AGENT_BUILD_NUM=`cat ${AGENT_ROOT}/src/inc/common/AppVersion.h | grep "mp_string AGENT_BUILD_NUM" | $AWK -F '"' '{print $2}'`
GenerateAgentUpdateVersion


CompareSysVersion()
{
    SYS_VER=`uname -r | $AWK -F '-' '{print $1}' | $AWK -F '.' '{print $1$2$3}'`
    
    if [ ${SYS_VER} -lt ${FREEZE_VERSION} ]; then
        FREEZE_SUPPORT=0
    else
        FREEZE_SUPPORT=1
    fi
}

create_dir()
{
    #create plugins dir
    mkdir -p ${AGENT_ROOT}/bin/plugins
    #create objs dir
    mkdir -p ${AGENT_ROOT}/obj
    mkdir -p ${AGENT_ROOT}/obj/afs
    mkdir -p ${AGENT_ROOT}/obj/agent
    mkdir -p ${AGENT_ROOT}/obj/rdbak
    mkdir -p ${AGENT_ROOT}/obj/rootexec
    mkdir -p ${AGENT_ROOT}/obj/crypto
    mkdir -p ${AGENT_ROOT}/obj/scriptsign
    mkdir -p ${AGENT_ROOT}/obj/datamigration
    mkdir -p ${AGENT_ROOT}/obj/monitor
    mkdir -p ${AGENT_ROOT}/obj/xmlcfg
    mkdir -p ${AGENT_ROOT}/obj/agentcli
    mkdir -p ${AGENT_ROOT}/obj/getinput
    mkdir -p ${AGENT_ROOT}/obj/apps/oracle
    mkdir -p ${AGENT_ROOT}/obj/apps/restore
    mkdir -p ${AGENT_ROOT}/obj/apps/app
    mkdir -p ${AGENT_ROOT}/obj/apps/oraclenative
    mkdir -p ${AGENT_ROOT}/obj/apps/vmwarenative
    mkdir -p ${AGENT_ROOT}/obj/array
    mkdir -p ${AGENT_ROOT}/obj/common
    mkdir -p ${AGENT_ROOT}/obj/securecom
    mkdir -p ${AGENT_ROOT}/obj/device
    mkdir -p ${AGENT_ROOT}/obj/host
    mkdir -p ${AGENT_ROOT}/obj/mobility
    mkdir -p ${AGENT_ROOT}/obj/pluginfx
    mkdir -p ${AGENT_ROOT}/obj/plugins
    mkdir -p ${AGENT_ROOT}/obj/plugins/device
    mkdir -p ${AGENT_ROOT}/obj/plugins/host
    mkdir -p ${AGENT_ROOT}/obj/plugins/oracle
    mkdir -p ${AGENT_ROOT}/obj/plugins/restore
    mkdir -p ${AGENT_ROOT}/obj/plugins/app
    mkdir -p ${AGENT_ROOT}/obj/plugins/mobility
    mkdir -p ${AGENT_ROOT}/obj/plugins/dppserver
    mkdir -p ${AGENT_ROOT}/obj/plugins/oraclenative
    mkdir -p ${AGENT_ROOT}/obj/plugins/vmwarenative
    mkdir -p ${AGENT_ROOT}/obj/plugins/appprotect
    mkdir -p ${AGENT_ROOT}/obj/message
    mkdir -p ${AGENT_ROOT}/obj/message/tcp
    mkdir -p ${AGENT_ROOT}/obj/message/archivestream
    mkdir -p ${AGENT_ROOT}/obj/message/tcpssl
    mkdir -p ${AGENT_ROOT}/obj/message/rest
    mkdir -p ${AGENT_ROOT}/obj/curlclient
    mkdir -p ${AGENT_ROOT}/obj/tools
    mkdir -p ${AGENT_ROOT}/obj/securec
    mkdir -p ${AGENT_ROOT}/obj/sqlite
    mkdir -p ${AGENT_ROOT}/obj/json
    mkdir -p ${AGENT_ROOT}/obj/tinyxml
    mkdir -p ${AGENT_ROOT}/obj/alarm
    mkdir -p ${AGENT_ROOT}/obj/taskmanager
    mkdir -p ${AGENT_ROOT}/obj/taskmanager/externaljob
    mkdir -p ${AGENT_ROOT}/obj/taskmanager/filter
    mkdir -p ${AGENT_ROOT}/obj/upgradeKernel
    mkdir -p ${AGENT_ROOT}/obj/dataprocess
    mkdir -p ${AGENT_ROOT}/obj/dataprocess/dataconfig
    mkdir -p ${AGENT_ROOT}/obj/dataprocess/datapath
    mkdir -p ${AGENT_ROOT}/obj/dataprocess/datamessage
    mkdir -p ${AGENT_ROOT}/obj/dataprocess/datareadwrite
    mkdir -p ${AGENT_ROOT}/obj/dataprocess/vmwarenative
    mkdir -p ${AGENT_ROOT}/obj/dataprocess/jobqosmanager
    mkdir -p ${AGENT_ROOT}/obj/dataprocess/ioscheduler
    mkdir -p ${AGENT_ROOT}/obj/dataprocess/ioscheduler/libvmfs
    mkdir -p ${AGENT_ROOT}/obj/dataprocess/ioscheduler/libvmfs6
    mkdir -p ${AGENT_ROOT}/obj/apps/dws/XBSAServer
    mkdir -p ${AGENT_ROOT}/obj/plugins/dws
    mkdir -p ${AGENT_ROOT}/obj/XBSAClient
    mkdir -p ${AGENT_ROOT}/obj/IIFXBSAClient
    mkdir -p ${AGENT_ROOT}/obj/HCSXBSAClient
    mkdir -p ${AGENT_ROOT}/obj/TPOPSXBSAClient
    mkdir -p ${AGENT_ROOT}/obj/XBSACom
    mkdir -p ${AGENT_ROOT}/obj/XBSACOMM
    mkdir -p ${AGENT_ROOT}/obj/servicecenter/servicefactory
    mkdir -p ${AGENT_ROOT}/obj/servicecenter/thriftservice
    mkdir -p ${AGENT_ROOT}/obj/apps/appprotect/plugininterface
    mkdir -p ${AGENT_ROOT}/obj/servicecenter/certificateservice
    mkdir -p ${AGENT_ROOT}/obj/servicecenter/messageservice
    mkdir -p ${AGENT_ROOT}/obj/servicecenter/services/device
    mkdir -p ${AGENT_ROOT}/obj/servicecenter/services/jobservice
    mkdir -p ${AGENT_ROOT}/obj/servicecenter/timerservice 
}

rm_shell()
{
    SHELL_FILES=`ls ${AGENT_ROOT}/bin/*.sh 2>/dev/null`
    if [ "${SHELL_FILES}" != "" ]; then
        rm ${AGENT_ROOT}/bin/*.sh
    fi

    if [ -d "${AGENT_ROOT}/bin/thirdparty" ]; then
        rm -rf ${AGENT_ROOT}/bin/thirdparty
    fi
}

copy_shell()
{
    rm_shell

    cp ${AGENT_ROOT}/bin/shell/*.sh ${AGENT_ROOT}/bin
    if [ "${OCEAN_MOBILITY}" = "1" ]; then
        cp ${AGENT_ROOT}/bin/shell/install_iomirror/*.sh ${AGENT_ROOT}/bin
    fi
    chmod 0755 ${AGENT_ROOT}/bin/*.sh

    #create thirdpatry directory
    if [ ! -d ${AGENT_ROOT}/bin/thirdparty/sample ]; then
        mkdir -p ${AGENT_ROOT}/bin/thirdparty/sample
    fi

    #copy thirdparty files
    FILE_COUNT=`ls ${AGENT_ROOT}/bin/shell/thirdparty | wc -l`
    if [ ${FILE_COUNT} != "0" ]; then
        cp -rf ${AGENT_ROOT}/bin/shell/thirdparty/* ${AGENT_ROOT}/bin/thirdparty
        chmod -R 0755 ${AGENT_ROOT}/bin/thirdparty/*
    fi
}

Dos2Unix()
{
    filePath=$1
    which dos2unix 1>/dev/null 2>&1
    if [ $? -eq 0 ]; then
        dos2unix "${filePath}"
    else
        filePath_bak=${filePath}.bak
        cat ${filePath} | tr -d '\r' > ${filePath_bak}
        mv ${filePath_bak} ${filePath}
    fi
}

dos2unix_src()
{
    for filePath in ${AGENT_ROOT}/conf/*; do
        fileName=`basename ${filePath}`
        if [ "${fileName}" = "kmc_config.txt" ] || [ "${fileName}" = "kmc_config_bak.txt" ] || \
        [ "${fileName}" = "kmc_store.txt" ] || [ "${fileName}" = "kmc_store_bak.txt" ] || [ "${fileName}" = "kmc" ]; then
            continue
        fi

        Dos2Unix ${AGENT_ROOT}/conf/${fileName} >> /dev/null
    done

    for filePath in ${AGENT_ROOT}/open_src/patch/*; do
        Dos2Unix ${filePath} >> /dev/null
    done
}

patch_to_curl()
{
    sed '2466s/conn,/conn, isproxy,/' ./lib/vtls/openssl.c > ./lib/vtls/openssl.c.bak && cat ./lib/vtls/openssl.c.bak > ./lib/vtls/openssl.c && rm ./lib/vtls/openssl.c.bak
    if [ $? -ne 0 ]; then
        return 1
    fi
}

set_san_flags()
{
    if [ ${ASAN} -eq 1 ]; then
        ASAN_CFLAGS="-fsanitize=address -fsanitize-recover=address -fno-stack-protector -fno-omit-frame-pointer -fno-var-tracking -g1"
        ASAN_LDFLAGS="-fsanitize=address -static-libasan"
    fi
    if [ ${TSAN} -eq 1 ]; then
        TSAN_CFLAGS="-DENABLE_TSAN -fsanitize=thread -fno-omit-frame-pointer -Wno-unused"
        TSAN_LDFLAGS="-fsanitize=thread"
    fi
}

set_hitest_env()
{
    export isOverlappedCompile=0
    export PlatformToken=BOARD
    export gcovmode=0
    export TimerPolicy=0         #是否开启线程定时器采集覆盖率数据, 1: 开启, 0: 不开启       
    export TimeInterval=60       #线程定时器，各隔多少秒采集一次数据，60表示60秒       
    export SignalPolicy=1        #是否开启发信号采集覆盖率，1: 开启, 0: 不开启      
    export SignalNUM=34          #kill -34 pid 采覆盖率， kill -44 pid 重置覆盖率       
    export lltwrapper_cfg=0      # 0: 普通模式，4: 无OS极简模式(单模块), 5: 无OS通用模式
    export HITEST_AGENT_INSIDE=1 # 1: 使用内嵌agent.o, 0: 不使用内嵌agent.o
    export USE_HLLT_COVERAGE=1
    export USE_HLLT_TESTCASE=0
    export simplemode=0    # 0: 非精简模式, 1: 精简模式
    export ncs_coverage_stub_mold=1       # 0: 非计数模式, 1: 计数模式
    export hitest_disable_cfg=0           # 是否需要导出CFG控制流图, 默认导出 0:导出  1:不导出
    export hitest_disable_dfg=1           # 是否需要导出污点数据, 默认不导出 0:导出  1:不导出 
    export lltcovRootpath=/opt/covdata      #(测试执行环境中，覆盖率数据保存的根目录)
    export PATH=${PATH}:/opt/HiTest        #(添加工具包到PATH环境变量)
    export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/opt/HiTest   #(添加工具包到LD_LIBRARY_PATH环境变量)
}

make_init()
{
    if [ $# != 0 ]; then
        if [ "$1" = "clean" ]; then
            if [ $# = 2 ]; then
                if [ "$2" = "all" ]; then
                    CLEAN_ALL=1
                else
                    echo "Invalid make option, 'make clean [all]'."
                    exit 2
                fi
            else
                CLEAN=1
            fi
        elif [ "$1" = "fortify" ]; then
            FORTIFY=1
        elif [ "$1" = "mst_coverage" ]; then 
            MST_COVERAGE=1
        elif [ "$1" = "dp" ]; then
            DATAPROCESS=1
        elif [ "$1" = "agent" ]; then
            AGENT=1
        elif [ "$1" = "xbsa" ]; then
            XBSA=1
        elif [ "$1" = "rest_publish" ];then
            REST_PUBLISH=1
        elif [ "$1" = "asan" ]; then
            ASAN=1
        elif [ "$1" = "tsan" ]; then
            TSAN=1
        elif [ "$1" = "hitest" ]; then
            HiTest_Flage=1
        elif [ "$1" = "sanclient" ]; then
            SANCLIENT_PLUIN=1
        else
            echo "Invalid make option, support clean or fortify only, but input option is [$1]."
            exit 2
        fi

        if [ "$2" = "no_opensrc" ]; then
            NO_OPENSRC=1
        elif [ "$2" = "asan" ]; then
            ASAN=1
        elif [ "$2" = "tsan" ]; then
            TSAN=1
        elif [ "$2" = "hitest" ]; then
            HiTest_Flage=1
        fi
    fi

    # set_san_flags
    set_san_flags

    if [ $sys = "AIX" ]; then
        CPPC=g++
        CPP=g++
        CC=gcc
        cc=gcc
        OS_VERSION=`oslevel | awk -F "." '{print $1$2}'`
        CFLAGS="${VERSION} -Wl,-bbigtoc -DSTDCXX_98_HEADERS -DAIX $KMC_OPT"
        cFLAGS=${CFLAGS}
        OFLAGS="${VERSION} -Wl,-bbigtoc -brtl -bexpall"
        oFLAGS=${OFLAGS}
        DFLAGS="${VERSION} -Wl,-bbigtoc"
        dFLAGS=${DFLAGS}
        ARNew="ar -X64 -v -r"
        STFLAGS=""
        DYFLAGS=""
        STLIBS=""
    elif [ $sys = "HP-UX" ]; then
        CPPC=aCC
        CC=aCC
        #for sqlite and securec
        cc=cc
        CFLAGS="${VERSION} -w -AA +DD64 -D_REENTRANT -DHP_UX_IA -DHP_UX -mt $KMC_OPT $ASAN_LDFLAGS $TSAN_LDFLAGS"
        cFLAGS="${VERSION} -w +DD64 -D_REENTRANT -DHP_UX_IA -DHP_UX -mt $ASAN_LDFLAGS $TSAN_LDFLAGS"
        CXXFLAGS="${VERSION} +DD64 -w -AA -D_REENTRANT -DHP_UX_IA -DHP_UX -mt -b +z"
        oFLAGS="${VERSION} +DD64 -ldl $ASAN_LDFLAGS $TSAN_LDFLAGS"
        OFLAGS="${VERSION} +DD64 -ldl -ldcekt $ASAN_LDFLAGS $TSAN_LDFLAGS"
        dFLAGS="${VERSION} +DD64 -b $ASAN_LDFLAGS $TSAN_LDFLAGS"
        DFLAGS="${VERSION} +DD64 -b -ldcekt $ASAN_LDFLAGS $TSAN_LDFLAGS"
        ARNew="ar rc"
        STFLAGS=""
        DYFLAGS=""
        STLIBS=""
    elif [ $sys = "Linux" ]; then
        sysLinuxName=`cat /etc/issue | grep 'Linx'`
        if [ -f /etc/SuSE-release ]; then
            OS_NAME=SUSE
        elif [ -f /etc/isoft-release ]; then
            OS_NAME=ISOFT
        elif [ -f /etc/redhat-release ]; then
            OS_NAME=REDHAT
        elif [ -f /etc/euleros-release ]; then
            OS_NAME=EULER
        elif [ "${sysLinuxName}" != "" ]; then
            OS_NAME=ROCKY
        elif [ -z "${sysLinuxName}" ]; then
            sysLinuxName_tmp=`cat /etc/issue | grep 'Rocky'`
            if [ "${sysLinuxName_tmp}" != "" ]; then
                OS_NAME=ROCKY
                NGINX_CPU_OPT_FLAG=1
            fi
        fi

        if [ ${FORTIFY} -eq 1 ]; then
            CPPC="sourceanalyzer -b rdagent g++"
            CC="sourceanalyzer -b rdagent gcc"
            cc="sourceanalyzer -b rdagent gcc"
        elif [ ${HiTest_Flage} -eq 1 ]; then
            CPPC="hitestwrapper g++"
            CC="hitestwrapper gcc"
            cc="hitestwrapper gcc"
            set_hitest_env
        else
            CPPC="g++"
            CC="gcc"
            cc="gcc"
            if [ ${MST_COVERAGE} -eq 1 ]; then
                COV_CFLAGS="-fprofile-arcs -ftest-coverage"
                COV_OFLAGS="-ftest-coverage -fprofile-arcs -lgcov"
            fi
        fi
        
        CompareSysVersion
        
        # add define STDCXX_98_HEADERS, support for snmp++ 3.3.7
        if [ ${FREEZE_SUPPORT} -eq 1 ]; then
            CFLAGS="${VERSION} -pipe -fpic -DLINUX -DFRAME_SIGN -DLIN_FRE_SUPP -D${OS_NAME} -DSTDCXX_98_HEADERS $KMC_OPT ${EXTERNAL_DEBUG_FLAG} -Wno-deprecated-declarations -fstack-protector-all $ASAN_CFLAGS $TSAN_CFLAGS"
        else
            CFLAGS="${VERSION} -pipe -fpic -DLINUX -DFRAME_SIGN -D${OS_NAME} -DSTDCXX_98_HEADERS $KMC_OPT ${EXTERNAL_DEBUG_FLAG} -Wno-deprecated-declarations -fstack-protector-all $ASAN_CFLAGS $TSAN_CFLAGS"
        fi

        if [ $REST_PUBLISH -eq 1 ]; then
            CFLAGS+=" -DREST_PUBLISH"
        fi

        cFLAGS=${CFLAGS}
        OFLAGS="${VERSION} -rdynamic ${SYM_TABLE} $NX_OPT $SP_OPT $RELRO_OPT $RPATH_OPT $PIE_OPT $BIND_NOW_OPT $ASAN_LDFLAGS $TSAN_LDFLAGS"
        oFLAGS="${VERSION} -rdynamic ${SYM_TABLE} $NX_OPT $SP_OPT $RELRO_OPT $RPATH_OPT $PIE_OPT $BIND_NOW_OPT $ASAN_LDFLAGS $TSAN_LDFLAGS"
        DFLAGS="${VERSION} -rdynamic -shared -fPIC ${SYM_TABLE} $NX_OPT $SP_OPT $RELRO_OPT $RPATH_OPT $BIND_NOW_OPT $ASAN_LDFLAGS $TSAN_LDFLAGS" 
        dFLAGS="${VERSION} -rdynamic -shared -fPIC ${SYM_TABLE} $NX_OPT $SP_OPT $RELRO_OPT $RPATH_OPT $BIND_NOW_OPT $ASAN_LDFLAGS $TSAN_LDFLAGS"
        if [ ${SYS_PLATFORM} != "aarch64" ]; then
            CFLAGS+=" -m64"
            cFLAGS+=" -m64"
            OFLAGS+=" -m64"
            oFLAGS+=" -m64"
            DFLAGS+=" -m64"
            dFLAGS+=" -m64"
        fi
        ARNew="ar rcs"
        STFLAGS="-Wl,-Bstatic"
        DYFLAGS="-Wl,-Bdynamic"
        STLIBS="-luuid"
        if [ $SANCLIENT_PLUIN -eq 1 ]; then
            CFLAGS+=" -DSANCLIENT_AGENT"
            DFLAGS+=" -DSANCLIENT_AGENT"
        fi
    elif [ "$sys" = "SunOS" ]; then
        CPPC="g++"
        CC="gcc"
        cc="gcc"
        #for sqlite and securec
        CFLAGS=" -mcpu=v9 -D_LARGEFILE64_SOURCE=1 -DHAVE_HIDDEN  -DPIC -DSOLARIS -DSTDCXX_98_HEADERS $KMC_OPT ${EXTERNAL_DEBUG_FLAG} -Wno-deprecated-declarations -fstack-protector-all  -nostdlib -m64"
        CXXFLAGS="-mcpu=v9 -D_LARGEFILE64_SOURCE=1 -DHAVE_HIDDEN  -DPIC -DSOLARIS -DSTDCXX_98_HEADERS $KMC_OPT ${EXTERNAL_DEBUG_FLAG} -Wno-deprecated-declarations -fstack-protector-all -nostdlib -m64"
        cFLAGS=$CFLAGS
        OFLAGS=" -m64 -lsocket -lnsl"
        oFLAGS=${OFLAGS}
        DFLAGS=" -m64 -shared -lsocket -lnsl"
        dFLAGS=${DFLAGS}
        ARNew="ar rc"
        STFLAGS=""
        DYFLAGS=""
        STLIBS="-luuid"
    else
        echo "Unsupported OS"
        exit 0
    fi
}

modify_appversion()
{
    COMPILE_TIME=`date`
    sed "9s/compile/$COMPILE_TIME/1" ${AGENT_ROOT}/src/inc/common/AppVersion.h >  ${AGENT_ROOT}/src/inc/common/AppVersion.h.bak
    rm -rf ${AGENT_ROOT}/src/inc/common/AppVersion.h
    mv ${AGENT_ROOT}/src/inc/common/AppVersion.h.bak ${AGENT_ROOT}/src/inc/common/AppVersion.h
}

main_enter()
{
    if [ ${AGENT_ROOT:-0} = 0 ]; then
        echo "Please source env.csh first"
        exit 2
    else
        create_dir
    fi

    dos2unix_src

    MAKE_OPTION=
    MAKE_OPTION_AGENT="agent"
    #don't clean third part objs
    if [ ${CLEAN} -eq 1 ]; then
        gmake $MAKE_JOB -f ${AGENT_ROOT}/build/makefile "clean"
    elif [ ${DATAPROCESS} -eq 1 ]; then
        make $MAKE_JOB -f ${AGENT_ROOT}/build/makefile "dp"
    # compile xbsa
    elif [ ${XBSA} -eq 1 ]; then
        make $MAKE_JOB -f ${AGENT_ROOT}/build/makefile "xbsa"
    # compile agent
    elif [ ${AGENT} -eq 1 ] || [ ${SANCLIENT_PLUIN} -eq 1 ]; then
        if [ ${CLEAN_ALL} -eq 1 ]; then
            MAKE_OPTION="clean"
            MAKE_OPTION_AGENT=
            rm_shell
        else
            copy_shell
        fi

        ${AGENT_ROOT}/build/agent_make_opensrc.sh no_opensrc ${MAKE_OPTION}
        if [ $? -ne 0 ]; then
            echo "make open_src failed!"
            exit 1
        fi
        modify_appversion

        CFLAGS=$CFLAGS" -Wl,--whole-archive"
        export CFLAGS
        make $MAKE_JOB -f ${AGENT_ROOT}/build/makefile ${MAKE_OPTION} ${MAKE_OPTION_AGENT}
    # compile all
    else
        if [ ${CLEAN_ALL} -eq 1 ]; then
            MAKE_OPTION="clean"
            rm_shell
        else
            copy_shell
        fi

        if [ $? -ne 0 ]; then
            echo "make open_src failed!"
            exit 1
        fi
        modify_appversion
        CFLAGS=$CFLAGS" -Wl,--whole-archive"
        export CFLAGS

        if [ "$sys" = "HP-UX" ]; then
            # patch to jsoncpp 0.10.7
            awk -v n=46 -v s="#ifdef isfinite" 'NR == n {print s} {print}' ${AGENT_ROOT}/open_src/jsoncpp/src/lib_json/json_writer.cpp > ${AGENT_ROOT}/open_src/jsoncpp/src/lib_json/json_writer.cpp1
            mv ${AGENT_ROOT}/open_src/jsoncpp/src/lib_json/json_writer.cpp1 ${AGENT_ROOT}/open_src/jsoncpp/src/lib_json/json_writer.cpp
            awk -v n=47 -v s="#undef isfinite" 'NR == n {print s} {print}' ${AGENT_ROOT}/open_src/jsoncpp/src/lib_json/json_writer.cpp > ${AGENT_ROOT}/open_src/jsoncpp/src/lib_json/json_writer.cpp1            
            mv ${AGENT_ROOT}/open_src/jsoncpp/src/lib_json/json_writer.cpp1 ${AGENT_ROOT}/open_src/jsoncpp/src/lib_json/json_writer.cpp
            awk -v n=48 -v s="#endif" 'NR == n {print s} {print}' ${AGENT_ROOT}/open_src/jsoncpp/src/lib_json/json_writer.cpp > ${AGENT_ROOT}/open_src/jsoncpp/src/lib_json/json_writer.cpp1            
            mv ${AGENT_ROOT}/open_src/jsoncpp/src/lib_json/json_writer.cpp1 ${AGENT_ROOT}/open_src/jsoncpp/src/lib_json/json_writer.cpp
            awk -v n=49 -v s="#include <math.h>" 'NR == n {print s} {print}' ${AGENT_ROOT}/open_src/jsoncpp/src/lib_json/json_writer.cpp > ${AGENT_ROOT}/open_src/jsoncpp/src/lib_json/json_writer.cpp1            
            mv ${AGENT_ROOT}/open_src/jsoncpp/src/lib_json/json_writer.cpp1 ${AGENT_ROOT}/open_src/jsoncpp/src/lib_json/json_writer.cpp
        fi
        make $MAKE_JOB -f ${AGENT_ROOT}/build/makefile ${MAKE_OPTION}
    fi
}

compile_gcov_out()
{
    if [ "$MST_COVERAGE" = "1" ]; then 
        ${CC} -shared -fPIC ${AGENT_ROOT}/test/stubTest/auto/src/gcov_out.c -o ${AGENT_ROOT}/obj/gcov_out.so
    fi
}

Create_python_executalbe_file()
{
    if [ $sys != "Linux" ]; then
        return 0
    fi
    pyinstaller -v
    if [ $? -ne 0 ]; then
        echo "Not install pyinstaller."
        exit 1
    fi

    if ls ${AGENT_ROOT}/open_src/zlib/.libs/lib/ | grep "^libz.so.1."; then
        cd ${AGENT_ROOT}/open_src/zlib/.libs/lib/
        zlib_name=`ls libz.so.1.*`
        cp -rf ${AGENT_ROOT}/open_src/zlib/.libs/lib/${zlib_name} /usr/lib64/
        cd /usr/lib64/
        ln -snf ${zlib_name} libz.so.1
        if [ ! -h "/lib64" ]; then
            cp -rf ${AGENT_ROOT}/open_src/zlib/.libs/lib/${zlib_name} /lib64/
            cd /lib64/
            ln -snf ${zlib_name} libz.so.1
        fi
    fi
    cd ${AGENT_ROOT}/bin/install/Linux/ProtectClient-E
    pyinstaller -F update_json_file.py
    if [ ! -f ${AGENT_ROOT}/bin/install/Linux/ProtectClient-E/dist/update_json_file ]; then
        echo "Not Create update_json_file execute file."
        exit 1
    fi
    echo "Create update_json_file execute file success."

    if [ $sys = "Linux" ]; then
        cd ${AGENT_ROOT}/bin/shell
        pyinstaller -F CreateDataturbolink.py
        if [ ! -f ${AGENT_ROOT}/bin/shell/dist/CreateDataturbolink ]; then
            echo "Not Create Dataturbo link execute file."
            exit 1
        fi
        echo "Create Dataturbo link execute file success."
    fi
}

echo "#########################################################"
echo "   Copyright (C), 2013-2014, Huawei Tech. Co., Ltd."
echo "   Start to compile Agent "
echo "#########################################################"
StartTime=`date '+%Y-%m-%d %H:%M:%S'`

make_init $*

if [ "$sys" = "SunOS" ]; then
    sed 's/-lsnmp++/-lsnmp++ -lresolv/g' makefile > makefile.bak
    mv makefile.bak makefile
fi

if [ ${CLEAN_ALL} -ne 1 ] && [ ${NO_OPENSRC} -ne 1 ]; then
    ${AGENT_ROOT}/build/agent_make_opensrc.sh no_opensrc
    if [ $? -ne 0 ]; then
        echo "make open_src failed!"
        exit 1
    fi
fi

export CPPC CC cc CFLAGS cFLAGS CXXFLAGS  OFLAGS oFLAGS DFLAGS dFLAGS AGENT_BUILD_NUM ARNew STFLAGS DYFLAGS STLIBS
export COV_CFLAGS COV_OFLAGS

touch build_make.log

main_enter

main_result=$?

Create_python_executalbe_file

compile_gcov_out

if [ ${main_result} != 0 ]; then
    echo "#########################################################"
    echo "   Compile Agent failed."
    echo "#########################################################"

    exit ${main_result}
fi

EndTime=`date '+%Y-%m-%d %H:%M:%S'`
echo "#########################################################"
echo "   Compile Agent completed."
echo "   begin at ${StartTime}"
echo "   end   at ${EndTime}"
echo "#########################################################"

exit $main_result
