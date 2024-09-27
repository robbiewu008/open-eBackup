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
LLT=0
PLUGIN_SDK=0
OS_NAME=
MAKE_JOB=
OPEN_SRC_PACKET_DIR=${AGENT_ROOT}/open_src
OPEN_SRC_JSONCPP_DIR=jsoncpp
OPEN_SRC_TINYXML_DIR=tinyxml
# LLT
GTEST_DIR=${AGENT_ROOT}/test/stubTest/src/gtest/
AGENT_DEPLOYMENT_DIR="/opt/DataBackup/ProtectClient/ProtectClient-E/bin"

#Linux version 2.6.29 support the file system freeze and thaw
FREEZE_VERSION=2629
#0 not support the freeze, 1 support
FREEZE_SUPPORT=0

sys=`uname -s`
if [ "$sys" = "SunOS" ]
then
    AWK=nawk
else
    AWK=awk
fi

SYS_PLATFORM=`uname -p`

if [ $sys = "AIX" ]; then
    CPPC=g++
elif [ $sys = "SunOS" ]; then
    CPPC=g++
elif [ $sys = "Linux" ]; then
    CPPC=g++
elif [ $sys = "HP-UX" ]; then
    CPPC=aCC
fi

# Special treatment: solve EulerOS ccache to accelerate compilation
if [ "${SYS_PLATFORM}" = "aarch64" ] || [ "${SYS_PLATFORM}" = "aarch32" ]; then
    env | grep -w CC | grep ccache 1>/dev/null 2>&1
    if [ $? -eq 0 ]; then
        export CC=/usr/bin/gcc
    fi

    env | grep -w CXX | grep ccache 1>/dev/null 2>&1
    if [ $? -eq 0 ]; then
        export CXX=/usr/bin/g++
    fi
fi

compile_gtest()
{
    cd ${AGENT_ROOT}/test/stubTest/obj/
    echo ${CPPC} -fno-access-control -fPIC -I${GTEST_DIR}/include -I${GTEST_DIR} -c ${GTEST_DIR}/src/gtest-all.cc 
    ${CPPC} -fno-access-control -fPIC -I${GTEST_DIR}/include -I${GTEST_DIR} -c ${GTEST_DIR}/src/gtest-all.cc 
    ar -rv libgtest.a gtest-all.o
    mkdir gtestlib
    mv libgtest.a gtestlib/
    cp -r ${GTEST_DIR}/include gtestlib/
    rm gtest-all.o
    echo "#########################################################"
    echo "   Make gtest succ."
    echo "#########################################################"
}

obtain_os_name()
{
    sysLinuxName=`cat /etc/issue | grep 'Linx'`
    if [ -f /etc/SuSE-release ]; then
        OS_NAME=SUSE
    elif [ -f /etc/isoft-release ]; then
        OS_NAME=ISOFT
    elif [ -f /etc/redhat-release ]; then
        OS_NAME=REDHAT
    elif [ -f /etc/euleros-release ]; then
        OS_NAME=EulerOS
    elif [ "${sysLinuxName}" != "" ]; then
        OS_NAME=ROCKY
    elif [ -z "${sysLinuxName}" ]; then
        sysLinuxName_tmp=`cat /etc/issue | grep 'Rocky'`
        if [ "${sysLinuxName_tmp}" != "" ]; then
            OS_NAME=ROCKY
        fi
    fi
    export OS_NAME=${OS_NAME}
}

CompareSysVersion()
{
    SYS_VER=`uname -r | $AWK -F '-' '{print $1}' | $AWK -F '.' '{print $1$2$3}'`
    
    if [ ${SYS_VER} -lt ${FREEZE_VERSION} ]; then
        FREEZE_SUPPORT=0
    else
        FREEZE_SUPPORT=1
        export FREEZE_SUPPORT=${FREEZE_SUPPORT}
    fi

}

LLT_thrift_cpp()
{
    echo "generate thrift code"
    THRIFT_BIN_DIR=${AGENT_ROOT}/open_src/thrift/.libs/bin
    if [ ! -d "${THRIFT_BIN_DIR}" ];then
        echo "cannot find bin path"
        exit 1
    fi

    if [ ! -f "${THRIFT_BIN_DIR}/thrift" ];then
        echo "thrift binary not exists"
        exit 1
    fi
    cd "${THRIFT_BIN_DIR}"
    rm -rf "${DEFAULT_DIR}"/*.h
    rm -rf "${DEFAULT_DIR}"/*.cpp
    rm -rf "${THRIFT_BIN_DIR}"/*.thrift
    cp -f ${AGENT_ROOT}/test/stubTest/src/src/servicecenter/thrift/test.thrift "${THRIFT_BIN_DIR}"
    DEFAULT_DIR=gen-cpp
    if [ -d "${DEFAULT_DIR}" ]; then
        rm -rf ${DEFAULT_DIR}
    fi
    echo "##############thrift -r -gen cpp test.thrift###########"
    ./thrift -r -gen cpp test.thrift
    main_result=$?
    if [ ${main_result} -ne 0 ]; then
        echo "#########################################################"
        echo "   generate thrift cpp failed."
        echo "#########################################################"
        exit ${main_result}
    fi

    if [ ! -d "${DEFAULT_DIR}" ]; then
        echo "#########################################################"
        echo "   generate thrift cpp failed."
        echo "#########################################################"
        exit 1
    fi

    FILENAME=`ls ${DEFAULT_DIR}/*skeleton.cpp`
    echo ${FILENAME}
    rm -f ${FILENAME}
    cp -f ${DEFAULT_DIR}/*.h  ${AGENT_ROOT}/test/stubTest/src/src/servicecenter/thriftservice
    cp -f ${DEFAULT_DIR}/*.cpp ${AGENT_ROOT}/test/stubTest/src/src/servicecenter/thriftservice
    echo "#########################################################"
    echo "   generate thrift cpp succ."
    echo "#########################################################"
}

# cmake compile
cmakeall()
{
    [ -f "${AGENT_ROOT}/build-cmake" ] && rm -rf "${AGENT_ROOT}/build-cmake"
    if [ "$1" = "clean" ]; then
        exit 0
    fi

    # build module
    BUILD_MODULE=""
    if [ "$1" = "dp" ]; then
        BUILD_MODULE="-DDP=ON"
    elif [ "$1" = "xbsa" ]; then
        BUILD_MODULE="-DXBSA=ON"
    elif [ "$1" = "agent" ] || [ "$2" = "agent" ]; then
        BUILD_MODULE="-DAGENT=ON"
    elif [ "$1" = "LLT" ]; then
        BUILD_MODULE="-DLLT=ON"
    elif [ "$1" = "rest_publish" ]; then
        BUILD_MODULE="-DREST_PUBLISH=ON"
    elif [ "$1" = "PLUGIN_SDK" ]; then
        BUILD_MODULE="-DPLUGIN_SDK=ON"
    fi

    echo "#######################################################################################################"
    echo "Begin cmake compile."
    echo "#######################################################################################################"
    echo 

    mkdir -p "${AGENT_ROOT}/build-cmake"
    cd "${AGENT_ROOT}/build-cmake"
    if [ -z ${BUILD_MODULE} ]; then
        cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=YES -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DCMAKE_C_COMPILER=$(which gcc) -DCMAKE_CXX_COMPILER=$(which g++) ..
    else
        cmake ${BUILD_MODULE} -DCMAKE_EXPORT_COMPILE_COMMANDS=YES -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DCMAKE_C_COMPILER=$(which gcc) -DCMAKE_CXX_COMPILER=$(which g++) ..
    fi

    if [ $? -ne 0 ]; then
        echo "cmake error."
        exit 1
    fi

    if [ "${sys}" = "AIX" ]; then
        export OBJECT_MODE=64
    fi

    make ${MAKE_JOB}
    if [ $? -ne 0 ]; then
        echo "make error."
        exit 1
    fi

    echo
    echo "#######################################################################################################"
    echo "Compile success."
    echo "#######################################################################################################"

    if [ "$1" = "PLUGIN_SDK" ]; then
        gen_plugin_sdk_package
    else
        # copy generate lib
        copy_generate_object
    fi
    return 0
}

generate_agent_update_version()
{
    if [ $sys = "Linux" ]; then
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

obtain_version()
{
    AGENT_BUILD_NUM=`cat ${AGENT_ROOT}/src/inc/common/AppVersion.h | grep "mp_string AGENT_BUILD_NUM" | $AWK -F '"' '{print $2}'`
    export AGENT_BUILD_NUM=${AGENT_BUILD_NUM}

    generate_agent_update_version
}

# clean shell
rm_shell()
{
    SHELL_FILES=`ls ${AGENT_ROOT}/bin/*.sh 2>/dev/null`
    if [ "${SHELL_FILES}" != "" ]; then
        rm -f ${AGENT_ROOT}/bin/*.sh
    fi

    if [ -d "${AGENT_ROOT}/bin/thirdparty" ]; then
        rm -rf ${AGENT_ROOT}/bin/thirdparty
    fi
}

copy_shell()
{
    rm_shell

    cp ${AGENT_ROOT}/bin/shell/*.sh ${AGENT_ROOT}/bin
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

# generate plugin sdk package
gen_plugin_sdk_package()
{
    if [ "${sys}" = "Linux" ] || [ "${sys}" = "AIX" ] || [ "${sys}" = "SunOS" ]; then
        rm -rf ${AGENT_ROOT}/plugin_sdk.tar.gz
        rm -rf ${AGENT_ROOT}/plugin_sdk
        mkdir -p ${AGENT_ROOT}/plugin_sdk
        mkdir -p ${AGENT_ROOT}/plugin_sdk/lib
        mkdir -p ${AGENT_ROOT}/plugin_sdk/include

        plugin_sdk_file="${AGENT_ROOT}/bin/libpluginsdk.so"
        if [ -f "${AGENT_ROOT}/bin/libpluginsdk.so" ]; then
            plugin_sdk_file="${AGENT_ROOT}/bin/libpluginsdk.so"
        elif [ -f "${AGENT_ROOT}/build-cmake/libpluginsdk.so" ]; then
            plugin_sdk_file="${AGENT_ROOT}/build-cmake/libpluginsdk.so"
        else
            echo "The libpluginsdk cannot be found."
            exit 1
        fi
        cp ${plugin_sdk_file} ${AGENT_ROOT}/plugin_sdk/lib


        PLUGINS_SDK_INCLUDE_LISTS="src/inc/common,                                  \
                                   src/inc/securecom,                               \
                                   src/inc/message/archivestream,message            \
                                   src/inc/message/tcp,message                      \
                                   src/inc/pluginfx/ExternalPluginSDK.h    \
                                   src/inc/pluginfx/com_huawei_oceanprotect_sdk_agent_plugin_lib_security_ability_definition_SecurityUtil.h    \
                                   src/inc/pluginfx/com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility.h,pluginfx    \
                                "
        for item in ${PLUGINS_SDK_INCLUDE_LISTS}; do
            srcFolder=`echo $item | ${AWK} -F "," '{print $1}'`
            dstFolder=`echo $item | ${AWK} -F "," '{print $2}'`
            [ ! -d "${AGENT_ROOT}/plugin_sdk/include/${dstFolder}" ] && mkdir -p ${AGENT_ROOT}/plugin_sdk/include/${dstFolder}
            cp -rf ${AGENT_ROOT}/${srcFolder} ${AGENT_ROOT}/plugin_sdk/include/${dstFolder}
        done

        cd ${AGENT_ROOT}/plugin_sdk
        tar cvf plugin_sdk.tar *
        gzip plugin_sdk.tar
        mv plugin_sdk.tar.gz ${AGENT_ROOT}
    fi 
}

copy_generate_object()
{
    if [ `uname -s` = "Linux" ]; then
        mkdir -p ${AGENT_ROOT}/bin/plugins
        # plugins library
        PLUGINS_LIB_LISTS="libapp-${AGENT_BUILD_NUM}.so        \
                        libappprotect-${AGENT_BUILD_NUM}.so    \
                        libcluster-${AGENT_BUILD_NUM}.so       \
                        libdevice-${AGENT_BUILD_NUM}.so        \
                        libdws-${AGENT_BUILD_NUM}.so           \
                        libhost-${AGENT_BUILD_NUM}.so          \
                        liboracle-${AGENT_BUILD_NUM}.so        \
                        liboraclenative-${AGENT_BUILD_NUM}.so  \
                        librestore-${AGENT_BUILD_NUM}.so       \
                        libvmwarenative-${AGENT_BUILD_NUM}.so  \
                        libxbsa64.so                           \
                        "
        for FILE in ${PLUGINS_LIB_LISTS}; do
            [ -f "${AGENT_ROOT}/bin/${FILE}" ] && mv ${AGENT_ROOT}/bin/${FILE} ${AGENT_ROOT}/bin/plugins
        done
    elif [ "${sys}" = "AIX" ]; then
        mkdir -p ${AGENT_ROOT}/bin/plugins

        # plugins library
        PLUGINS_LIB_LISTS="libapp-${AGENT_BUILD_NUM}.so         \
                            libhost-${AGENT_BUILD_NUM}.so           \
                            libappprotect-${AGENT_BUILD_NUM}.so     \
                            libdws-${AGENT_BUILD_NUM}.so            \
                            libxbsa64.so                            \
                            libxbsa64iif.so                         \
                            "

        PLUGINS_STATIC_LIB_LISTS="libapp-${AGENT_BUILD_NUM}.a           \
                                libhost-${AGENT_BUILD_NUM}.a            \
                                libappprotect-${AGENT_BUILD_NUM}.a      \
                                libdws-${AGENT_BUILD_NUM}.a             \
                                libxbsa64.a                             \
                                libxbsa64iif.a                          \
                                "

        LIB_BIN_LISTS="         \
            agentcli            \
            crypto              \
            monitor             \
            rdagent             \
            rootexec            \
            scriptsign          \
            xmlcfg              \
            getinput            \
            libcommon.a         \
            libsecurecom.a      \
        "
        for FILE in ${PLUGINS_LIB_LISTS}; do
            [ -f "${AGENT_ROOT}/build-cmake/${FILE}" ] && mv ${AGENT_ROOT}/build-cmake/${FILE} ${AGENT_ROOT}/bin/plugins
        done

        for STATIC_FILE in ${PLUGINS_STATIC_LIB_LISTS}; do
            [ -f "${AGENT_ROOT}/bin/${STATIC_FILE}" ] && mv ${AGENT_ROOT}/bin/${STATIC_FILE} ${AGENT_ROOT}/bin/plugins
        done

        for FILE in ${LIB_BIN_LISTS}; do
            [ -f "${AGENT_ROOT}/build-cmake/${FILE}" ] && mv ${AGENT_ROOT}/build-cmake/${FILE} ${AGENT_ROOT}/bin/
        done

        cp -f "${AIX_STDCPP_PATH}" "${AGENT_ROOT}/bin"
        cp -f "${AIX_GCC_S_PATH}" "${AGENT_ROOT}/bin"
        cp -f "${AGENT_ROOT}/open_src/libevent/.libs/lib/libevent.a" "${AGENT_ROOT}/bin"
    elif [ "${sys}" = "SunOS" ]; then
        mkdir -p ${AGENT_ROOT}/bin/plugins
        # plugins library
        PLUGINS_LIB_LISTS="libapp-${AGENT_BUILD_NUM}.so        \
                        libappprotect-${AGENT_BUILD_NUM}.so    \
                        libdevice-${AGENT_BUILD_NUM}.so        \
                        libdws-${AGENT_BUILD_NUM}.so           \
                        libhost-${AGENT_BUILD_NUM}.so          \
                        liboracle-${AGENT_BUILD_NUM}.so        \
                        liboraclenative-${AGENT_BUILD_NUM}.so  \
                        libxbsa64.so                           \
                        "
        for FILE in ${PLUGINS_LIB_LISTS}; do
            [ -f "${AGENT_ROOT}/bin/${FILE}" ] && mv ${AGENT_ROOT}/bin/${FILE} ${AGENT_ROOT}/bin/plugins
        done
        cp -f "${STDCPP_LIB_PATH}" "${AGENT_ROOT}/bin"
        cp -f "${GCC_S_LIB_PATH}" "${AGENT_ROOT}/bin"
        cp -f "${SSP_LIB_PATH}" "${AGENT_ROOT}/bin"
    fi
}

dos2unix_conf_files()
{
    file_list=`ls ${AGENT_ROOT}/conf`
    for FILE_NAME in ${file_list}; do
        if [ "${FILE_NAME}" = "kmc_config.txt" ] || [ "${FILE_NAME}" = "kmc_config_bak.txt" ] || [ "${FILE_NAME}" = "kmc_store.txt" ] || [ "${FILE_NAME}" = "kmc_store_bak.txt" ] || [ "${FILE_NAME}" = "kmc" ]; then
            continue
        fi

        CURR_FILE=${AGENT_ROOT}/conf/${FILE_NAME}
        BAK_CURR_FILE=${CURR_FILE}.bak
        cat ${CURR_FILE} | tr -d '\r' > ${BAK_CURR_FILE}
        
        cp ${BAK_CURR_FILE} ${CURR_FILE}
        rm ${BAK_CURR_FILE}
    done
}

aix_copy_stdcpp_lib()
{
    if [ "${sys}" = "AIX" ]; then
        if [ ! -f "${AIX_STDCPP_PATH}" ]; then
            echo "The path[libstdc++.a] is wrong, please reconfigure [AIX_STDCPP_PATH] in the env.sh. and source env.sh."
            exit 1
        fi

        if [ ! -f "${AIX_GCC_S_PATH}" ]; then
            echo "The path[libgcc_s.a] is wrong, please reconfigure [AIX_GCC_S_PATH] in the env.sh. and source env.sh."
            exit 1
        fi

        # create agent deployment dir
        mkdir -p ${AGENT_DEPLOYMENT_DIR}
        cp -r "${AIX_STDCPP_PATH}" "${AGENT_DEPLOYMENT_DIR}"
        cp -r "${AIX_GCC_S_PATH}" "${AGENT_DEPLOYMENT_DIR}"
    else
        if [ -z "${STDCPP_LIB_PATH}" ] && [ -z "${GCC_S_LIB_PATH}" ]; then
            return
        fi

        mkdir -p ${AGENT_DEPLOYMENT_DIR}
        # STDCPP_LIB_PATH
        if [ -n "${STDCPP_LIB_PATH}" ]; then
            if [ ! -f "${STDCPP_LIB_PATH}" ]; then
                echo "The path[libstdc++.a] is wrong, please reconfigure [STDCPP_LIB_PATH] in the env.sh. and source env.sh."
                exit 1
            fi
            cp -r "${STDCPP_LIB_PATH}" "${AGENT_DEPLOYMENT_DIR}"
        fi
        
        # STDCPP_LIB_PATH
        if [ -n "${GCC_S_LIB_PATH}" ]; then
            if [ ! -f "${GCC_S_LIB_PATH}" ]; then
                echo "The path[libstdc++.a] is wrong, please reconfigure [GCC_S_LIB_PATH] in the env.sh. and source env.sh."
                exit 1
            fi
            cp -r "${GCC_S_LIB_PATH}" "${AGENT_DEPLOYMENT_DIR}"
        fi
    fi
}

modify_appversion()
{
    COMPILE_TIME=`date`
    sed "9s/compile/$COMPILE_TIME/1" ${AGENT_ROOT}/src/inc/common/AppVersion.h >  ${AGENT_ROOT}/src/inc/common/AppVersion.h.bak
    rm -rf ${AGENT_ROOT}/src/inc/common/AppVersion.h
    mv ${AGENT_ROOT}/src/inc/common/AppVersion.h.bak ${AGENT_ROOT}/src/inc/common/AppVersion.h
}

param_init()
{
    if [ "$sys" = "Linux" ] || [ "$sys" = "AIX" ] || [ "$sys" = "SunOS" ]; then
        MAKE_JOB="-j 4"
    else
        MAKE_JOB=""
    fi

    if [ $# -ne 0 ]; then
        if [ "$1" = "clean" ]; then
            if [ $# -eq 2 ]; then
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
        elif [ "$1" = "rest_publish" ]; then
            REST_PUBLISH=1
        elif [ "$1" = "LLT" ]; then
            LLT=1
        elif [ "$1" = "no_opensrc" ]; then
            NO_OPENSRC=1
        elif [ "$1" = "sdk" ]; then
            PLUGIN_SDK=1
        elif [ "$1" = "sdk_no_opensrc" ]; then
            PLUGIN_SDK=1
            NO_OPENSRC=1
        elif [ "$1" = "help" ]; then
            PrintHelp
            exit 0
        else
            echo "Invalid make option, support clean or fortify only, but input option is [$1]."
            exit 2
        fi
    fi

    if [ ${CLEAN_ALL} -ne 1 ] && [ ${NO_OPENSRC} -ne 1 ]; then
        ${AGENT_ROOT}/build/agent_make_opensrc.sh no_opensrc
        if [ $? -ne 0 ]; then
            echo "make open_src failed."
            exit 1
        fi
    fi

    # obtain agent version
    obtain_version

    # obtain os name
    obtain_os_name
}

PrintHelp()
{
    CurFile=`basename $0`
    echo "Usage: $CurFile [option]"
    echo "Options:"
    echo "  clean           Clean compiled files."
    echo "  fortify         Compile all file with forify."
    echo "  mst_coverage    Compile all file with mst_coverage."
    echo "  dp              Compile dataprocess binary file."
    echo "  agent           Compile DataBackup ProtectAgent all file."
    echo "  xbsa            Compile dws xbas library file."
    echo "  rest_publish    Compile OceanStorBCManager ProtectAgent all file."
    echo "  LLT             Compile ProtectAgent all llt binary file."
    echo "  no_opensrc      Please reference help with agent_make_opensrc.sh."
    echo "  sdk             Build external plugin SDK(Software Development Kit) package."
    echo "  help            Display this information."
    echo "  [none]          Compile OceanStorBCManager ProtectAgent all file."
    echo ""
    echo "For bug reporting instructions, please call 120, thanks."
}

process_enter()
{
    if [ ${AGENT_ROOT:-0} = 0 ]; then
        echo "Please source env.csh first."
        exit 2
    fi

    aix_copy_stdcpp_lib

    dos2unix_conf_files

    MAKE_OPTION=
    MAKE_OPTION_AGENT="agent"
    # don't clean third part objs
    if [ ${CLEAN} -eq 1 ]; then
        cmakeall "clean"
    elif [ ${DATAPROCESS} -eq 1 ]; then
        cmakeall "dp"
    # compile xbsa
    elif [ ${XBSA} -eq 1 ]; then
        cmakeall "xbsa"
    # compile agent
    elif [ ${AGENT} -eq 1 ]; then
        if [ ${CLEAN_ALL} -eq 1 ]; then
            MAKE_OPTION="clean"
            MAKE_OPTION_AGENT=
            rm_shell
        else
            copy_shell
        fi

        ${AGENT_ROOT}/build/agent_make_opensrc.sh no_opensrc ${MAKE_OPTION}
        if [ $? -ne 0 ]; then
            echo "make open_src failed."
            exit 1
        fi
        modify_appversion

        cmakeall ${MAKE_OPTION} ${MAKE_OPTION_AGENT}
    elif [ ${LLT} -eq 1 ]; then
        ${AGENT_ROOT}/build/agent_make_opensrc.sh no_opensrc ${MAKE_OPTION}
        if [ $? -ne 0 ]; then
            echo "make open_src failed."
            exit 1
        fi
        LLT_thrift_cpp
        compile_gtest # LLT compile
        modify_appversion
        cmakeall "LLT" ${MAKE_OPTION}
    elif [ ${PLUGIN_SDK} -eq 1 ]; then
        ${AGENT_ROOT}/build/agent_make_opensrc.sh no_opensrc ${MAKE_OPTION}
        if [ $? -ne 0 ]; then
            echo "make open_src failed."
            exit 1
        fi

        cmakeall "PLUGIN_SDK"
    # compile all
    else        
        if [ ${CLEAN_ALL} -eq 1 ]; then
            MAKE_OPTION="clean"
            rm_shell
        else
            copy_shell
        fi
        ${AGENT_ROOT}/build/agent_make_opensrc.sh no_opensrc ${MAKE_OPTION}
        if [ $? -ne 0 ]; then
            echo "make open_src failed."
            exit 1
        fi
        modify_appversion
        cmakeall ${MAKE_OPTION}
    fi
}

main()
{
    param_init $*

    process_enter

    return $?
}

main $*
exit $?
