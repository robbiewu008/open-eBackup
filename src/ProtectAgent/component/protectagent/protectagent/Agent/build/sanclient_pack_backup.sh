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

# "-DDEBUG " just to debug Agent
# ADD_DEBUG+=" -DDEBUG "  

if [ ${AGENT_ROOT:-0} = 0 ]
then
    echo "please source env.csh first"
    exit 2
fi

. "${AGENT_ROOT}/build/agent_pack_common.sh"

PRODUCT_NAME="A8000"
DEPLOY_DIR="product/backup"
CONF_DIR="backup"
cd ${AGENT_ROOT}

DPP_BUILD_DIR="build/DppBuild"
DPP_BIN_DIR="DppBin"
DPP_PROCESS_BIN="dataprocess"
DPP_BUILD_SHELL="dataProcess.sh"
SQLITE_BIN="sqlite3"

LIB64_DIR="/usr/lib64"
COMMON_LIBSTDCPP="libstdc++.so.6"
OPEN_SRC_PACKET_DIR=${AGENT_ROOT}/open_src
LIB_STD_CPP_SO_PATH="${LIB64_DIR}/${COMMON_LIBSTDCPP}"
LIB_STD_CPP_SO=""
LIB_SECUREC_SO=""
sys=`uname -s`

# san pkg flag
SAN_PKG_FLAG=0

AddPackFilesBin()
{
    echo "AddPackFilesBin()"
    cp -r ${AGENT_ROOT}/conf/${CONF_DIR}/pluginmgr.xml   ${AGENT_ROOT}/conf/pluginmgr.xml
    cp -r ${AGENT_ROOT}/conf/${CONF_DIR}/vddk.cfg   ${AGENT_ROOT}/conf/vddk.cfg
    cp -r ${AGENT_ROOT}/${DPP_BUILD_DIR}/${DPP_BIN_DIR}/${DPP_PROCESS_BIN}  ${AGENT_ROOT}/bin/${DPP_PROCESS_BIN}

    if [ $# -eq 1 ] && [ $1 = "san" ]; then
        cp -r ${AGENT_ROOT}/bin/install/Linux/ProtectClient-E/agent_install_san.sh   ${AGENT_ROOT}/bin/agent_install_san.sh
    fi
    cp -r ${AGENT_ROOT}/bin/install/Linux/ProtectClient-E/agent_install.sh   ${AGENT_ROOT}/bin/agent_install.sh
    cp -r ${AGENT_ROOT}/bin/install/Linux/ProtectClient-E/agent_uninstall.sh   ${AGENT_ROOT}/bin/agent_uninstall.sh
    cp -r ${AGENT_ROOT}/bin/install/Linux/ProtectClient-E/agent_stop.sh   ${AGENT_ROOT}/bin/agent_stop.sh
    cp -r ${AGENT_ROOT}/bin/install/Linux/ProtectClient-E/agent_start.sh  ${AGENT_ROOT}/bin/agent_start.sh
    cp -r ${AGENT_ROOT}/bin/install/Linux/ProtectClient-E/agent_upgrade.sh  ${AGENT_ROOT}/bin/agent_upgrade.sh
    cp -r ${AGENT_ROOT}/bin/install/Linux/ProtectClient-E/agent_upgrade_sqlite.sh  ${AGENT_ROOT}/bin/agent_upgrade_sqlite.sh
    cp -r ${AGENT_ROOT}/bin/install/updateCert.sh  ${AGENT_ROOT}/bin/updateCert.sh
    cp -r ${AGENT_ROOT}/bin/install/crl_update.sh  ${AGENT_ROOT}/bin/crl_update.sh

    rm -rf ${AGENT_ROOT}/bin/thirdparty/*
    #add agent driver bin files
    pack_files_bin="${pack_files_bin} bin/plugins/libxbsa64* bin/plugins/libdws* bin/plugins/libhost* bin/plugins/libvmwarenative* bin/plugins/liboracle* bin/plugins/libapp* bin/${DPP_PROCESS_BIN} bin/${SQLITE_BIN}"
    pack_files_bin="${pack_files_bin} bin/${LIB_STD_CPP_SO}"
    if [ -f "${AGENT_ROOT}/bin/${LIB_CRYPT_SO}" ]; then
        pack_files_bin="${pack_files_bin} bin/${LIB_CRYPT_SO}"
    fi
    if [ -f "${AGENT_ROOT}/bin/${LIB_SECUREC_SO}" ]; then
        pack_files_bin="${pack_files_bin} bin/${LIB_SECUREC_SO}"
    fi
}

PackLibStdCppSo()
{
    LIB_STD_CPP_SO=`ls ${OPEN_SRC_PACKET_DIR}/libstdc++.so.6*`
    if [ "${LIB_STD_CPP_SO}" = "" ];then
        echo "${LIB_STD_CPP_SO} : null"
        exit 1
    fi
    /bin/cp -rf "${LIB_STD_CPP_SO}" "${AGENT_ROOT}/bin"
    echo "compile Agent with libstdc++ file ${LIB_STD_CPP_SO}"
    LIB_STD_CPP_SO=`basename ${LIB_STD_CPP_SO}`
}

PackLibcryptSO()
{
    LIB_CRYPT_SO=`ls ${OPEN_SRC_PACKET_DIR}/libcrypt.so*`
    if [ -f "${LIB_CRYPT_SO}" ]; then
        /bin/cp -rf "${LIB_CRYPT_SO}" "${AGENT_ROOT}/bin"
        LIB_CRYPT_SO=`basename ${LIB_CRYPT_SO}`
    fi
}

RootCopyToSbin()
{
    echo "RootCopyToSbin()"
    #prepare sbin dir
    if [ ! -d ${AGENT_ROOT}/sbin ]
    then
        mkdir ${AGENT_ROOT}/sbin
    fi

    # Copy the root permission script to the sbin directory
    ROOTSCRIPT_LIST_SBIN="agent_install.sh              \
                          agent_uninstall.sh            \
                          agent_upgrade.sh              \
                          agent_bin_func.sh             \
                          agent_sbin_func.sh            \
                          firewall_tools.sh             \
                          initiator.sh                  \
                          linkiscsitarget.sh            \
                          rootexec                      \
                          dataprocess                   \
                          getinput                      \
                          scandisk.sh                   \
                          umountnasmedia.sh             \
                          preparenasmedia.sh            \
                          preparedataturbomedia.sh      \
                          procmonitor.sh                \
                          gethostos.sh                  \
                          packlog.sh                    \
                          updateCert.sh                 \
                          crl_update.sh                 \
                          xmlcfg                        \
                          upgrade_caller.sh             \
                          upgrade_check.sh              \
                          upgrade_prepare.sh            \
                          mountnasfilesystem.sh         \
                          mountdataturbofilesystem.sh   \
                          umountnasfilesystem.sh        \
                          clearmountpoint.sh            \
                          setcgroup.sh            \
                          sanclientaction.sh            \
                          sanclientactioniscsi.sh       \
                          sanclientcopylogmeta.sh       \
                          sanclientcheck.sh             \
                          sanclientclear.sh             \
                          "
    for SCRIPTFILE in ${ROOTSCRIPT_LIST_SBIN}; do
        [ -f "${AGENT_ROOT}/bin/${SCRIPTFILE}" ] && cp -r ${AGENT_ROOT}/bin/${SCRIPTFILE} ${AGENT_ROOT}/sbin
    done
    
    # copy thirdparty
    [ -d "${AGENT_ROOT}/bin/thirdparty" ] && cp -r ${AGENT_ROOT}/bin/thirdparty ${AGENT_ROOT}/sbin

    #copy create dataturbo link file
    [ -f "${AGENT_ROOT}/bin/shell/dist/CreateDataturbolink" ] && cp -r ${AGENT_ROOT}/bin/shell/dist/CreateDataturbolink ${AGENT_ROOT}/sbin
    
}

#copy agent_cfg.xml to conf
[ -f "${AGENT_ROOT}/conf/backup/agent_cfg.xml" ] && cp -rf ${AGENT_ROOT}/conf/backup/agent_cfg.xml ${AGENT_ROOT}/conf
[ -f "${AGENT_ROOT}/conf/backup/alarm_info.xml" ] && cp -rf ${AGENT_ROOT}/conf/backup/alarm_info.xml ${AGENT_ROOT}/conf

AGENT_VERSION=`cat ${AGENT_ROOT}/src/inc/common/AppVersion.h | grep "AGENT_HDRS_VERSION = " | $AWK -F '"' '{print $2}'`
AGENT_BUILD_NUM=`cat ${AGENT_ROOT}/src/inc/common/AppVersion.h | grep "AGENT_BUILD_NUM = " | $AWK -F '"' '{print $2}'`
AGENT_PACKAGE_VERSION=`cat ${AGENT_ROOT}/src/inc/common/AppVersion.h | grep "AGENT_PACKAGE_VERSION = " | $AWK -F '"' '{print $2}'`

#step 1 of agent common pack
ConfigPackageVersion


#step 2 of agent common pack
if [ "`uname -s`" != "SunOS" ]; then
    AgentCommonClean
fi

#step 1.2 copy compile VM's libstadc+++.so.6.0.*, thus it can be packed to Agent
if [ "`uname -s`" = "Linux" ]; then
    PackLibStdCppSo
    PackLibcryptSO
fi

#step 3 of agent common pack
if [ "$1" = "no_opensrc" ] ; then
    AgentCompile no_opensrc sanclient
else 
    AgentCompile sanclient
fi

#step 4 of agent common pack
ConfigPluginsVersion

#step 5 of agent common pack
PrepareAgentPackage sanclient
#step 5.1: add appbackup files into package
AddPackFilesBin

# add root permission to sbin directory
RootCopyToSbin

#step 6 of agent common pack
AgentPack sanclient

exit 0
