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

sys=`uname -s`
if [ "$sys" = "SunOS" ]
then
    AWK=nawk
else
    AWK=awk
fi

LD_LIBRARY_PATH=${AGENT_ROOT}/bin;export LD_LIBRARY_PATH
SANCLIENT_PACKAGE_NAME="sanclient-Linux-x86_64"

getLinuxOsName()
{
    if [ -f /etc/oracle-release ] #oracle linux 
    then
        num=`ls ${AGENT_ROOT}/bin/CustomScripts/Linux/OEL6/ |wc -l `
        if [ $num -gt 0 ]
        then
            cp ${AGENT_ROOT}/bin/CustomScripts/Linux/OEL6/* ${AGENT_ROOT}/bin/shell/thirdparty/
        fi
        OS_NAME=OL`cat /etc/oracle-release| $AWK -F '.' '{print $1}' | $AWK '{print $NF}'`
    elif [ -f /etc/SuSE-release ] #SuSe10/11/12
    then
        OS_VERSION=`cat /etc/SuSE-release | $AWK '$1 == "VERSION" {print $3}'`
        if [ $OS_VERSION = "10" ]
        then
            num=`ls ${AGENT_ROOT}/bin/CustomScripts/Linux/Suse10/ |wc -l `
            if [ $num -gt 0 ]
            then
                cp ${AGENT_ROOT}/bin/CustomScripts/Linux/Suse10/* ${AGENT_ROOT}/bin/shell/thirdparty/
            fi
        elif [ $OS_VERSION = "11" ]
        then
            num=`ls ${AGENT_ROOT}/bin/CustomScripts/Linux/Suse11/ |wc -l `
            if [ $num -gt 0 ]
            then
                cp ${AGENT_ROOT}/bin/CustomScripts/Linux/Suse11/* ${AGENT_ROOT}/bin/shell/thirdparty/
            fi
        elif [ $OS_VERSION = "12" ]
        then
            num=`ls ${AGENT_ROOT}/bin/CustomScripts/Linux/Suse12/ |wc -l `
            if [ $num -gt 0 ]
            then
                cp ${AGENT_ROOT}/bin/CustomScripts/Linux/Suse12/* ${AGENT_ROOT}/bin/shell/thirdparty/
            fi
        fi            
        OS_NAME=SuSE${OS_VERSION}
    elif [ -f /etc/isoft-release ] #isoft
    then
        num=`ls ${AGENT_ROOT}/bin/CustomScripts/Linux/iSoft3/ |wc -l `
        if [ $num -gt 0 ]
        then
            cp ${AGENT_ROOT}/bin/CustomScripts/Linux/iSoft3/* ${AGENT_ROOT}/bin/shell/thirdparty/
        fi
        OS_VERSION=`cat /etc/isoft-release | awk -F '.' '{print $1}' | awk '{print $NF}'`
        OS_NAME=iSoft${OS_VERSION}
    elif [ -f /etc/redhat-release ] #Redhat5/6/7
    then
        OS_VERSION=`cat /etc/redhat-release | $AWK -F '.' '{print $1}' | $AWK '{print $NF}'`
        if [ $OS_VERSION = "6" ]
        then
            num=`ls ${AGENT_ROOT}/bin/CustomScripts/Linux/Redhat6/ |wc -l `
            if [ $num -gt 0 ]
            then
                cp ${AGENT_ROOT}/bin/CustomScripts/Linux/Redhat6/* ${AGENT_ROOT}/bin/shell/thirdparty/
            fi
        elif [ $OS_VERSION = "7" ]
        then
            num=`ls ${AGENT_ROOT}/bin/CustomScripts/Linux/Redhat7/ |wc -l `
            if [ $num -gt 0 ]
            then
                cp ${AGENT_ROOT}/bin/CustomScripts/Linux/Redhat7/* ${AGENT_ROOT}/bin/shell/thirdparty/
            fi
        fi  
        OS_TYPE=`cat /etc/redhat-release | $AWK -F' ' '{print $1}'`
        if [ "EulerOS" == "${OS_TYPE}" ]
        then
            OS_NAME=EulerOS
        elif [ "CentOS" == "${OS_TYPE}" ]
        then
            OS_NAME=CentOS
        else
            OS_NAME=RedHat
        fi
        #${OS_VERSION}
        OS_NAME="Linux"
    elif [ -f /etc/debian_version ] #rocky6
    then
        num=`ls ${AGENT_ROOT}/bin/CustomScripts/Linux/Rocky6/ |wc -l `
        if [ $num -gt 0 ]
        then
            cp ${AGENT_ROOT}/bin/CustomScripts/Linux/Rocky6/* ${AGENT_ROOT}/bin/shell/thirdparty/
        fi
        OS_NAME=Rocky`cat /etc/debian_version|$AWK -F '.' '{print $1}'`
    elif [ -f /etc/linx-release ] #rocky4
    then
        OS_NAME=Rocky`cat /etc/linx-release|awk '{print $2}'|awk -F '.' '{print $1}'`
    fi
    OS_NAME="Linux"
    echo "$OS_NAME";
}

AGENT_PACK=${HOME}/AGENT_PACK_TEMP
AGENT_VERSION=
AGENT_UPDATE_VERSION=
AGENT_PACKAGE_VERSION=
BASE_NAME=
PACK_BASE_NAME=
BIN_NAME_BASE=
SRC_NAME_BASE=
DATE_STR=
OS_NAME=
FLAG_BIT="x86_64"

pack_files_src="           \
    bin                    \
    build                  \
    ci                     \
    conf                   \
    open_src               \
    platform               \
    selfdevelop            \
    src                    \
    test                   \
    third_party_groupware  \
    vsprj
  "

pack_files_bin_common_shell="        \
    bin/agent_bin_func.sh            \
    bin/agent_reload_nginx.sh        \
    bin/agent_start.sh               \
    bin/agent_stop.sh                \
    bin/agent_upgrade_sqlite.sh      \
    sbin/firewall_tools.sh           \
    sbin/packlog.sh                  \
    sbin/procmonitor.sh              \
"

pack_files_bin="                     \
    nginx                            \
    bin/rdagent                      \
    bin/monitor                      \
    bin/agentcli                     \
    bin/openssl                      \
    bin/sanclient                    \
    bin/lib*.so                      \
    conf/kmc/agentcli*.txt           \
    conf/agent_cfg.xml               \
    conf/alarm_info.xml              \
    conf/pluginmgr.xml               \
    conf/vddk.cfg                    \
    conf/kmc_config.txt              \
    conf/kmc_store.txt               \
    conf/script.sig                  \
    conf/version                     \
    conf/openssl.cnf                 \
    conf/vddk.cfg                    \
    conf/package.json                \
    log                              \
    slog                             \
    tmp                              \
    stmp                             \
    lib                             \
    *.doc                            \
    db/AgentDB.db                    \
    db/DwsDB.db                      \
    db/upgrade                       \
" 
pack_files_bin="${pack_files_bin} ${pack_files_bin_common_shell}"

pack_files_sbin="
    sbin/agent_install.sh              \
    sbin/agent_uninstall.sh            \
    sbin/agent_upgrade.sh              \
    sbin/agent_sbin_func.sh            \
    sbin/initiator.sh                  \
    sbin/linkiscsitarget.sh            \
    sbin/rootexec                      \
    sbin/dataprocess                   \
    sbin/scandisk.sh                   \
    sbin/getinput                      \
    sbin/umountnasmedia.sh             \
    sbin/preparenasmedia.sh            \
    sbin/preparedataturbomedia.sh      \
    sbin/procmonitor.sh                \
    sbin/gethostos.sh                  \
    sbin/packlog.sh                    \
    sbin/updateCert.sh                 \
    sbin/crl_update.sh                 \
    sbin/thirdparty                    \
    sbin/xmlcfg                        \
    sbin/modify_caller.sh              \
    sbin/modify_check.sh               \
    sbin/modify_prepare.sh             \
    sbin/upgrade_caller.sh             \
    sbin/upgrade_check.sh              \
    sbin/upgrade_prepare.sh            \
    sbin/push_update_cert.sh           \
    sbin/mountnasfilesystem.sh         \
    sbin/mountfileiosystem.sh          \
    sbin/mountdataturbofilesystem.sh   \
    sbin/umountnasfilesystem.sh        \
    sbin/clearmountpoint.sh            \
    sbin/setcgroup.sh                  \
    sbin/CreateDataturbolink           \
    sbin/vmfs_check_tool.sh            \
    sbin/vmfs_mount.sh                 \
    sbin/vmfs_umount.sh                \
    sbin/update_json_file              \
    sbin/config_dpc_flow_control.sh         \
    sbin/config_dpc_policy_route.sh         \
    "

pack_files_sanclient_bin="
    bin/sanclient                      \
    "

pack_files_sanclient_sbin="
    sbin/sanclientaction.sh            \
    sbin/sanclientactioniscsi.sh       \
    sbin/sanclientcopylogmeta.sh       \
    sbin/sanclientcheck.sh             \
    sbin/sanclientclear.sh             \
    "

if [ "$1" = "sanclient" ];then
    pack_files_bin="${pack_files_bin} ${pack_files_sanclient_bin}"

    pack_files_sbin="${pack_files_sbin} ${pack_files_sanclient_sbin}"
fi

if [ -f ${AGENT_ROOT}/conf/svn ]
then 
    pack_files_bin="$pack_files_bin conf/svn"
fi

if [ ${AGENT_ROOT:-0} = 0 ]
then
    echo "please source env.csh first"
    exit 2
fi

sys=`uname -s`
if [ $sys = "AIX" ]
then
    cp ${AGENT_ROOT}/bin/CustomScripts/Unix/*.sh ${AGENT_ROOT}/bin/shell/thirdparty/ >/dev/null 2>&1
    cp ${AGENT_ROOT}/bin/CustomScripts/Unix/AIX6/* ${AGENT_ROOT}/bin/shell/thirdparty/ >/dev/null 2>&1
    OS_NAME="AIX"
    OS_VERSION=`oslevel | awk -F "." '{print $1$2}'`
    if [ "${OS_VERSION}" = "53" ]
    then
        OS_NAME="AIX53"
    fi
    FLAG_BIT="ppc_64"
elif [ $sys = "Linux" ]
then
    cp ${AGENT_ROOT}/bin/CustomScripts/Linux/*.sh ${AGENT_ROOT}/bin/shell/thirdparty/ >/dev/null 2>&1
    LinuxOsName=`getLinuxOsName`
    OS_NAME=${LinuxOsName}
    FLAG_BIT=`uname -m`
elif [ $sys = "HP-UX" ]
then
    cp ${AGENT_ROOT}/bin/CustomScripts/Unix/*.sh ${AGENT_ROOT}/bin/shell/thirdparty/ >/dev/null 2>&1
    cp ${AGENT_ROOT}/bin/CustomScripts/Unix/HP1131/* ${AGENT_ROOT}/bin/shell/thirdparty/ >/dev/null 2>&1
    OS_VERSION=`uname -a | awk '{print $3}' | awk -F "." '{print $2"."$3}'`
    OS_NAME="HP-UX_$OS_VERSION"
    FLAG_BIT="ia_64"
elif [ "$sys" = "SunOS" ]
then
    cp ${AGENT_ROOT}/bin/CustomScripts/Unix/*.sh ${AGENT_ROOT}/bin/shell/thirdparty/ >/dev/null 2>&1
    cp ${AGENT_ROOT}/bin/CustomScripts/Unix/Solaris/* ${AGENT_ROOT}/bin/shell/thirdparty/ >/dev/null 2>&1
    OS_NAME="SunOS"
    FLAG_BIT="sparc_64"
else
    echo "Unsupported OS"
    exit 1
fi

AGENT_VERSION=""
AGENT_BUILD_NUM=""
AGENT_PACKAGE_VERSION=""

ConfigPackageVersion()
{
    if [ "${PRODUCT_NAME}" != "A8000" ]
    then 
        BASE_NAME="DataBackup ProtectAgent ${AGENT_VERSION}_Agent"
        PACK_BASE_NAME="DataBackup ProtectAgent ${AGENT_PACKAGE_VERSION}_Agent"
    else
        BASE_NAME="protectclient"
        PACK_BASE_NAME="protectclient"
    fi 

    if [ "eBackup" = "$1" ];then
        BASE_NAME="Cloud Server Backup ${AGENT_VERSION}_Agent"
        PACK_BASE_NAME="Cloud Server Backup ${AGENT_PACKAGE_VERSION}_Agent"
        #delete parameter "eBackup"
        shift 1
    fi

    BIN_NAME_BASE="${BASE_NAME}-${OS_NAME}-${FLAG_BIT}"
    SRC_NAME_BASE="${BASE_NAME}-SRC-"
    arch_name=`arch`
    if [ $sys = "Linux" ]; then
        cat /etc/system-release | grep CentOS
        if [ $? -ne 1 ] && [ $arch_name = "aarch64" ]; then
            BIN_NAME_BASE="${BASE_NAME}-${OS_NAME}-Centos-${FLAG_BIT}"
        fi
    fi
    DATE_STR=`date +%y-%m-%d`
}

# step2: make clean old objs before pack
AgentCommonClean()
{
    #clean obj files before pack src files
    if [ "$sys" = "SunOS" ]
    then
        bash ${AGENT_ROOT}/build/agent_make.sh clean
    else
        ${AGENT_ROOT}/build/agent_make.sh clean
    fi
}

#step3: prepare directory and make MAKEFILE
AgentCompile()
{
    #del svn files
    find ${AGENT_ROOT} -name ".svn" |xargs rm -rf 2>/dev/null
    find ${AGENT_ROOT} -name ".gitignore" |xargs rm -rf 2>/dev/null
    find ${AGENT_ROOT} -name ".gitkeep" |xargs rm -rf 2>/dev/null

    #pack src files
    echo "#########################################################"
    echo "   Copyright (C), 2013-2020, Huawei Tech. Co., Ltd."
    echo "   Start to pack src files"
    echo "#########################################################"
    StartTime=`date '+%Y-%m-%d %H:%M:%S'`

    if [ ! -d ${AGENT_PACK} ]
    then
        mkdir ${AGENT_PACK}
    fi

    #prepare log and tmp dir
    if [ ! -d ${AGENT_ROOT}/log ]
    then
        mkdir ${AGENT_ROOT}/log
    fi
    rm -rf ${AGENT_ROOT}/log/*

    if [ ! -d ${AGENT_ROOT}/slog ]
    then
        mkdir ${AGENT_ROOT}/slog
    fi
    rm -rf ${AGENT_ROOT}/slog/*

    if [ ! -d ${AGENT_ROOT}/tmp ]
    then
        mkdir ${AGENT_ROOT}/tmp
    fi
    rm -rf ${AGENT_ROOT}/tmp/*

    if [ ! -d ${AGENT_ROOT}/stmp ]
    then
        mkdir ${AGENT_ROOT}/stmp
    fi
    rm -rf ${AGENT_ROOT}/stmp/*

    if [ ! -d ${AGENT_ROOT}/lib ]; then
        mkdir ${AGENT_ROOT}/lib
    fi
    rm -rf ${AGENT_ROOT}/lib/*

    #build src package
    cd ${AGENT_ROOT}

    EndTime=`date '+%Y-%m-%d %H:%M:%S'`
    echo "#########################################################"
    echo "   Pack src files completed."
    echo "   begin at ${StartTime}"
    echo "   end   at ${EndTime}"
    echo "#########################################################"

    EXTERNAL_DEBUG_FLAG=${ADD_DEBUG}; export EXTERNAL_DEBUG_FLAG

    if [ ${BUILD_CMAKE} = "OFF" ]; then
        #make agent
        if [ "$sys" = "SunOS" ]; then
            bash ${AGENT_ROOT}/build/agent_make.sh $*
        elif [ $sys = "Linux" ]; then
            if [ "$1" = "sanclient" -o "$2" = "sanclient" ];then
                # compile sanclient
                ${AGENT_ROOT}/build/agent_make.sh sanclient $*
                if [ $? != 0 ];then
                    echo "#########################################################"
                    echo "   sanclient src files Compile Faild."
                    echo "   begin at ${StartTime}"
                    echo "   end   at ${EndTime}"
                    echo "#########################################################"
                    exit 1
                fi
            else
                ${AGENT_ROOT}/build/agent_make.sh agent $*
                if [ $? != 0 ];then
                    echo "#########################################################"
                    echo "   linux agent src files Compile Faild."
                    echo "   begin at ${StartTime}"
                    echo "   end   at ${EndTime}"
                    echo "#########################################################"
                    exit 1
                fi
            
                #step3.1 Compile Dpp Process
                ${AGENT_ROOT}/build/agent_make.sh dp $*
                if [ $? != 0 ];then
                    echo "#########################################################"
                    echo "   Dataprocess src files Compile Faild."
                    echo "   begin at ${StartTime}"
                    echo "   end   at ${EndTime}"
                    echo "#########################################################"
                    exit 1
                fi

                # compile xbsa
                ${AGENT_ROOT}/build/agent_make.sh xbsa $*
                if [ $? != 0 ];then
                    echo "#########################################################"
                    echo "   xbsa src files Compile Faild."
                    echo "   begin at ${StartTime}"
                    echo "   end   at ${EndTime}"
                    echo "#########################################################"
                    exit 1
                fi
            fi
        else
            ${AGENT_ROOT}/build/agent_make.sh $*
        fi
    elif [ ${BUILD_CMAKE} = "ON" ]; then
        # 流水线CI登录后环境变量缺失，需要切root用户执行cmake编译脚本
        su - root -c ". ${AGENT_ROOT}/build/env.sh; .${AGENT_ROOT}/build/agent_make_cmake.sh $*"
    else
        echo "env.sh file field error.[BUILD_CMAKE=ON/OFF]"
        exit 1
    fi

    if [ $? != 0 ]; then
        echo "#########################################################"
        echo "   Agent src files Compile Faild."
        echo "   begin at ${StartTime}"
        echo "   end   at ${EndTime}"
        echo "#########################################################"
        exit 1
    fi
    echo ""
}

#step4 : config plugins version
ConfigPluginsVersion()
{
    #mk version file
    if [ -f "${AGENT_ROOT}/conf/version" ]; then
        rm -rf "${AGENT_ROOT}/conf/version"
    fi

    echo ${AGENT_VERSION}>${AGENT_ROOT}/conf/version
    echo ${AGENT_BUILD_NUM}>>${AGENT_ROOT}/conf/version
    AGENT_UPDATE_VERSION=`cat ${AGENT_ROOT}/src/inc/common/AppVersion.h | grep "AGENT_UPDATE_VERSION = " | $AWK -F '= ' '{print $2}' | $AWK -F ';' '{print $1}'`
    echo ${AGENT_UPDATE_VERSION}>>${AGENT_ROOT}/conf/version

    #modify plugin.xml
    OLD_PLUG_VER=`cat ${AGENT_ROOT}/conf/pluginmgr.xml | grep '" service="' | $AWK -F '" service="' '{print $1}' |  $AWK -F 'version="' '{print $2}' | sed -n 1p`
    mv ${AGENT_ROOT}/conf/pluginmgr.xml ${AGENT_ROOT}/conf/pluginmgr.xml.bak
    sed "s/${OLD_PLUG_VER}/${AGENT_BUILD_NUM}/g"  ${AGENT_ROOT}/conf/pluginmgr.xml.bak > ${AGENT_ROOT}/conf/pluginmgr.xml

    # copy and update package.json version
    cp -rf ${AGENT_ROOT}/ci/script/package.json ${AGENT_ROOT}/conf
    sh ${AGENT_ROOT}/ci/script/set_package_version.sh ${AGENT_ROOT}/conf/package.json  ${MS_IMAGE_TAG} ${Version}
}

#step5: begin to config snmp, nginx
PrepareAgentPackage()
{
    ${AGENT_ROOT}/bin/scriptsign
    rm -f ${AGENT_ROOT}/log/scriptsign.log

    #pack bin files
    echo "#########################################################"
    echo "   Copyright (C), 2013-2017, Huawei Tech. Co., Ltd."
    echo "   Start to pack bin files"
    echo "#########################################################"
    StartTime=`date '+%Y-%m-%d %H:%M:%S'`


    if [ ! -d ${AGENT_ROOT}/db ]
    then
        mkdir -p ${AGENT_ROOT}/db
        chmod 755 ${AGENT_ROOT}/db
        echo "mkdir ${AGENT_ROOT}/db"
    fi

    if [ ! -d "${AGENT_ROOT}/selfdevelop" ]; then
        mkdir -p ${AGENT_ROOT}/selfdevelop
        chmod 755 ${AGENT_ROOT}/selfdevelop
        echo "mkdir ${AGENT_ROOT}/selfdevelop"
    fi

    if [ ! -f "${AGENT_ROOT}/db/AgentDB.db" ]; then
        mkdir -p ${AGENT_ROOT}/db
        chmod 755 ${AGENT_ROOT}/db

        sqlite3 ${AGENT_ROOT}/selfdevelop/AgentDB.db "`cat ${AGENT_ROOT}/build/create_table.sql`"
        if [ $? -ne 0 ]; then
            echo "Create agent DB failed."
            exit 1
        fi
        cp ${AGENT_ROOT}/selfdevelop/AgentDB.db ${AGENT_ROOT}/db
        echo "Agent db created successfully."
    fi

    if [ ! -f "${AGENT_ROOT}/db/DwsDB.db" ]; then
        if [ "$1" != "sanclient" ]; then
            sqlite3 ${AGENT_ROOT}/selfdevelop/DwsDB.db "`cat ${AGENT_ROOT}/build/create_dws_table.sql`"
            if [ $? -ne 0 ]; then
                echo "Create dws DB failed."
                exit 1
            fi
            cp ${AGENT_ROOT}/selfdevelop/DwsDB.db ${AGENT_ROOT}/db
        fi
        cp -rf "${AGENT_ROOT}/bin/install/Linux/ProtectClient-E/upgrade" "${AGENT_ROOT}/db"
        echo "dws db created successfully."
    fi

    cp "${AGENT_ROOT}/build/copyRight/Open Source Software Notice.doc" ${AGENT_ROOT}/

    #initialize snmp config
    
    # kmc generate passwd
    # NGINX_CONFIG=`${AGENT_ROOT}/bin/crypto -a 0 -i $INPUT_PATH`

    # snmp private_password encrypted by KMC
    SNMP_PRIVATE_PASSWD="00000001000000010000000000000005000000010000000153d515e41001697d5474c6f329a671cb3ba511f45c21ec57572163012c0c354e0000002000000000000000006b181ea5dc07d3f3646c8d17bddbca2966483054258e4b33d07b77af771e145e0000000100000000000008040000000100000001e5aa69c27edd0c2181b9898d1f3de54c00000000000000001ef5368807eda14b417b61330ded694e63798d17e9de90ab5a1374ecabce8f89"
    ${AGENT_ROOT}/sbin/xmlcfg write SNMP private_password "$SNMP_PRIVATE_PASSWD"
    
    # snmp auth_password encrypted by KMC
    SNMP_AUTH_PASSWD="000000010000000100000000000000050000000100000001c1b831a0fbb4e9f7bcdf5f9f4903357dfd6306be5b3c74eba3ac2d5081a2b0d2000000200000000000000000da7037e7e90938720c84c633cb6e1d231c47907a657f5c1f646755fa695310ed000000010000000000000804000000010000000198e0022f6e462a60bac5c6f5ef9d1e040000000000000000a17ab32d8a80c0e64782ed86bb9039fcac47d166699e01e7a643f910eb32ae77"
    ${AGENT_ROOT}/sbin/xmlcfg write SNMP auth_password "$SNMP_AUTH_PASSWD"

    # nginx ssl key passwd of Application consistency
    NGINX_SSL_KEY_PASSWD="000000010000000100000000000000050000000100000001b343ffca851b6d034c6bb7d445fd4af708a3ee07194e9743b540044239695b2b000000200000000000000000078c3a332e83382aae523e3a0c0b69bab0624250a72a5cb1655cab72e2f607860000000100000000000008040000000100000001410d4a21dd845af48ffb5e0ef920aece00000000000000001f63687345f329caacd70fc94b202ee87e4d5bd85385d0e971cafe26b31f841b"
    ${AGENT_ROOT}/sbin/xmlcfg write Monitor nginx ssl_key_password "$NGINX_SSL_KEY_PASSWD"

    rm -f ${AGENT_ROOT}/log/*

    # delete unncessary files for nginx
    if [ -d ${AGENT_ROOT}/bin/nginx/html ]; then 
        rm -rf ${AGENT_ROOT}/bin/nginx/html
    fi
}

#step6: pack Agent, should firstly set pack_files_bin
AgentPack()
{
    echo "AgentPack()"
    if [ $# -eq 1 ] && [ $1 = "san" ]; then
        pack_files_sbin="$pack_files_sbin sbin/agent_install_san.sh"
    fi
    #build bin package
    cd ${AGENT_PACK}
    if [ "$1" = "sanclient" ];then
        rm -f "${SANCLIENT_PACKAGE_NAME}.tar" "${SANCLIENT_PACKAGE_NAME}.tar.xz"
    else
        rm -f "${BIN_NAME_BASE}.tar" "${BIN_NAME_BASE}.tar.xz"
    fi
    cd ${AGENT_ROOT}
    mv ${AGENT_ROOT}/bin/nginx ${AGENT_ROOT}
    if [ ${sys} = "AIX" ]; then
        tar cvf ${AGENT_PACK}/${BIN_NAME_BASE}.tar ${pack_files_bin} ${pack_files_sbin}
        xz -ve ${AGENT_PACK}/${BIN_NAME_BASE}.tar
    elif [ "$1" = "sanclient" ]; then
        export XZ_OPT="-v -T 0 -9e"
        tar cJf "${AGENT_PACK}/${SANCLIENT_PACKAGE_NAME}.tar.xz" ${pack_files_bin} ${pack_files_sbin}
    elif [ ${sys} = "SunOS" ]; then
        tar cvf ${AGENT_PACK}/${BIN_NAME_BASE}.tar ${pack_files_bin} ${pack_files_sbin}
        gzip ${AGENT_PACK}/${BIN_NAME_BASE}.tar
    else
        export XZ_OPT="-v -T 0 -9e"
        tar cJf "${AGENT_PACK}/${BIN_NAME_BASE}.tar.xz" ${pack_files_bin} ${pack_files_sbin}
    fi

    rm -rf ${AGENT_ROOT}/db
    EndTime=`date '+%Y-%m-%d %H:%M:%S'`
    echo "#########################################################"
    echo "   Pack bin files completed."
    echo "   begin at ${StartTime}"
    echo "   end   at ${EndTime}"
    echo "#########################################################"
}
