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
FUZZ_SUPPORT=0
#for module system test's coverage
MST_COVERAGE=0
COV_CFLAGS=
COV_OFLAGS=

sys=`uname -s`

#open source packages
OPEN_SRC_FCGI=fcgi*.tar.gz
OPEN_SRC_JSONCPP=jsoncpp*.tar.gz
OPEN_SRC_NGINX=nginx-release-1.15.8
OPEN_SRC_OPENSSL=openssl*.tar.gz
OPEN_SRC_SNMP=snmp++*.tar.gz
OPEN_SRC_SQLITE=sqlite*.zip
OPEN_SRC_TINYXML=tinyxml*.tar.gz
OPEN_SRC_CURL=curl*.tar.gz

OPEN_SRC_FCGI_DIR=fcgi
OPEN_SRC_JSONCPP_DIR=jsoncpp
OPEN_SRC_NGINX_DIR=nginx_tmp
OPEN_SRC_OPENSSL_DIR=openssl
OPEN_SRC_SQLITE_DIR=sqlite
OPEN_SRC_SNMP_DIR=snmp++
OPEN_SRC_TINYXML_DIR=tinyxml
OPEN_SRC_CURL_DIR=curl

AWK=
if [ $sys = "Linux" ]; then
    MAKE_JOB="-j 8"
else
    MAKE_JOB=""
fi

GTESTLIB_DIR= 
GTEST_DIR=${AGENT_ROOT}/test/stubTest/src/gtest/
 
#gcc secure opt
NX_OPT="-Wl,-z,noexecstack"
SP_OPT="-fstack-protector-all --param ssp-buffer-size=4 -Wstack-protector"
RELRO_OPT="-Wl,-z,relro,-z,now"
RPATH_OPT="-Wl,--disable-new-dtags" 

if [ $sys = "SunOS" ]
then
    AWK=nawk
else
    AWK=awk
fi

AGENT_VERSION=`cat ${AGENT_ROOT}/src/inc/common/AppVersion.h | grep "AGENT_VERSION = " | $AWK -F '"' '{print $2}'`
AGENT_BUILD_NUM=`cat ${AGENT_ROOT}/src/inc/common/AppVersion.h | grep "AGENT_BUILD_NUM =" | $AWK -F '"' '{print $2}'`
sysLinuxName=$(cat /etc/issue | grep 'Linx')


CompareSysVersion()
{
    SYS_VER=`uname -r | $AWK -F '-' '{print $1}' | $AWK -F '.' '{print $1$2$3}'`
    
    if [ ${SYS_VER} -lt ${FREEZE_VERSION} ]
    then
        FREEZE_SUPPORT=0
    else
        FREEZE_SUPPORT=1
    fi
}

unzip_open_src_packages()
{
    cd ${AGENT_ROOT}/open_src
    #fcgi
    if [ ! -d ${OPEN_SRC_FCGI_DIR} ]
    then
        gzip -cd ${OPEN_SRC_FCGI} | tar -xvf -
        UNZIPED_DIR_NAME=`basename ${OPEN_SRC_FCGI} .tar.gz`
        mv ${UNZIPED_DIR_NAME} ${OPEN_SRC_FCGI_DIR}
    fi
    #jsoncpp
    if [ ! -d ${OPEN_SRC_JSONCPP_DIR} ]
    then
        gzip -cd ${OPEN_SRC_JSONCPP} | tar -xvf -
        UNZIPED_DIR_NAME=`basename ${OPEN_SRC_JSONCPP} .tar.gz`
        mv ${UNZIPED_DIR_NAME} ${OPEN_SRC_JSONCPP_DIR}
    fi
    #nginx
    if [ ! -d ${OPEN_SRC_NGINX_DIR} ]
    then
        #gzip -cd ${OPEN_SRC_NGINX} | tar -xvf -
        #UNZIPED_DIR_NAME=`basename ${OPEN_SRC_NGINX} .tar.gz`
        unzip ${OPEN_SRC_NGINX}.zip
        mv ${OPEN_SRC_NGINX}/ ${OPEN_SRC_NGINX_DIR}      
    fi
    #openssl
    if [ ! -d ${OPEN_SRC_OPENSSL_DIR} ]
    then
        gzip -cd ${OPEN_SRC_OPENSSL} | tar -xvf -
        UNZIPED_DIR_NAME=`basename ${OPEN_SRC_OPENSSL} .tar.gz`
        mv ${UNZIPED_DIR_NAME} ${OPEN_SRC_OPENSSL_DIR}
    fi
    #snmp
    if [ ! -d ${OPEN_SRC_SNMP_DIR} ]
    then
        gzip -cd ${OPEN_SRC_SNMP} | tar -xvf -
        UNZIPED_DIR_NAME=`basename ${OPEN_SRC_SNMP} .tar.gz`
        mv ${UNZIPED_DIR_NAME}/ ${OPEN_SRC_SNMP_DIR}
    fi
    #sqlite
    if [ ! -d ${OPEN_SRC_SQLITE_DIR} ]
    then
        unzip ${OPEN_SRC_SQLITE}
        UNZIPED_DIR_NAME=`basename ${OPEN_SRC_SQLITE} .zip`
        mv ${UNZIPED_DIR_NAME} ${OPEN_SRC_SQLITE_DIR}
        mv ${AGENT_ROOT}/src/patch/sqlite/os_unix.c ${AGENT_ROOT}/open_src/sqlite/src/os_unix.c
        mv ${AGENT_ROOT}/src/patch/sqlite/rtree.c ${AGENT_ROOT}/open_src/sqlite/ext/rtree/rtree.c
    fi
    #tinyxml
    if [ ! -d ${OPEN_SRC_TINYXML_DIR} ]
    then
        gzip -cd ${OPEN_SRC_TINYXML} | tar -xvf -
        UNZIPED_DIR_NAME=tinyxml2-7.0.1
        mv ${UNZIPED_DIR_NAME}/ ${OPEN_SRC_TINYXML_DIR}
    fi
    #curl
    if [ ! -d ${OPEN_SRC_CURL_DIR} ]
    then
        gzip -cd ${OPEN_SRC_CURL} | tar -xvf -
        UNZIPED_DIR_NAME=curl-7.66.0
        mv ${UNZIPED_DIR_NAME}/ ${OPEN_SRC_CURL_DIR}
    fi
}

delete_open_src_unzipped_files()
{
    cd ${AGENT_ROOT}/open_src

    #fcgi
    UNZIPED_DIR_NAME=`basename ${OPEN_SRC_FCGI} .tar.gz`
    rm -rf ${UNZIPED_DIR_NAME}
    rm -rf ${OPEN_SRC_FCGI_DIR}
    #jsoncpp
    UNZIPED_DIR_NAME=`basename ${OPEN_SRC_JSONCPP} .tar.gz`
    rm -rf ${UNZIPED_DIR_NAME}
    rm -rf ${OPEN_SRC_JSONCPP_DIR}
    #nginx
    #UNZIPED_DIR_NAME=`basename ${OPEN_SRC_NGINX} .tar.gz`
    rm -rf ${OPEN_SRC_NGINX}
    rm -rf ${OPEN_SRC_NGINX_DIR}
    #openssl
    UNZIPED_DIR_NAME=`basename ${OPEN_SRC_OPENSSL} .tar.gz`
    rm -rf ${UNZIPED_DIR_NAME}
    rm -rf ${OPEN_SRC_OPENSSL_DIR}
    #snmp
    rm -rf ${OPEN_SRC_SNMP_DIR}
    #sqlite
    UNZIPED_DIR_NAME=`basename ${OPEN_SRC_SQLITE} .zip`
    rm -rf ${UNZIPED_DIR_NAME}
    rm -rf ${OPEN_SRC_SQLITE_DIR}
    #tinyxml
    rm -rf ${OPEN_SRC_TINYXML_DIR}
    #curl
    rm -rf ${OPEN_SRC_CURL_DIR}
}

create_dir()
{
    #create plugins dir
    mkdir -p ${AGENT_ROOT}/bin/plugins
    #create objs dir
    mkdir -p ${AGENT_ROOT}/obj
    mkdir -p ${AGENT_ROOT}/obj/agent
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
    mkdir -p ${AGENT_ROOT}/obj/array
    mkdir -p ${AGENT_ROOT}/obj/common
    mkdir -p ${AGENT_ROOT}/obj/securecom
    mkdir -p ${AGENT_ROOT}/obj/device
    mkdir -p ${AGENT_ROOT}/obj/host
    mkdir -p ${AGENT_ROOT}/obj/message/rest
    mkdir -p ${AGENT_ROOT}/obj/message/tcp
    mkdir -p ${AGENT_ROOT}/obj/message/tcpssl
    mkdir -p ${AGENT_ROOT}/obj/message/archivestream    
    mkdir -p ${AGENT_ROOT}/obj/pluginfx
    mkdir -p ${AGENT_ROOT}/obj/plugins
    mkdir -p ${AGENT_ROOT}/obj/plugins/device
    mkdir -p ${AGENT_ROOT}/obj/plugins/host
    mkdir -p ${AGENT_ROOT}/obj/plugins/oracle
    mkdir -p ${AGENT_ROOT}/obj/plugins/restore
    mkdir -p ${AGENT_ROOT}/obj/plugins/app
    mkdir -p ${AGENT_ROOT}/obj/plugins/oraclenative
    mkdir -p ${AGENT_ROOT}/obj/plugins/appprotect
    mkdir -p ${AGENT_ROOT}/obj/plugins/dws
    mkdir -p ${AGENT_ROOT}/obj/rest
    mkdir -p ${AGENT_ROOT}/obj/tools
    mkdir -p ${AGENT_ROOT}/obj/securec
    mkdir -p ${AGENT_ROOT}/obj/sqlite
    mkdir -p ${AGENT_ROOT}/obj/json
    mkdir -p ${AGENT_ROOT}/obj/tinyxml
    mkdir -p ${AGENT_ROOT}/obj/alarm
    mkdir -p ${AGENT_ROOT}/obj/taskmanager
    mkdir -p ${AGENT_ROOT}/obj/taskmanager/externaljob
    mkdir -p ${AGENT_ROOT}/obj/taskmanager/filter
    mkdir -p ${AGENT_ROOT}/obj/servicecenter/servicefactory
    mkdir -p ${AGENT_ROOT}/obj/servicecenter/thriftservice
    mkdir -p ${AGENT_ROOT}/obj/servicecenter/certifcateservice
    mkdir -p ${AGENT_ROOT}/obj/servicecenter/services/device
    mkdir -p ${AGENT_ROOT}/obj/servicecenter/services/jobservice
    mkdir -p ${AGENT_ROOT}/obj/servicecenter/messageservice
    mkdir -p ${AGENT_ROOT}/obj/servicecenter/timerservice
    mkdir -p ${AGENT_ROOT}/obj/apps/appprotect/
    mkdir -p ${AGENT_ROOT}/obj/apps/appprotect/plugininterface
    mkdir -p ${AGENT_ROOT}/obj/XBSACom
    mkdir -p ${AGENT_ROOT}/obj/XBSAClient
    mkdir -p ${AGENT_ROOT}/obj/curlclient
    mkdir -p ${AGENT_ROOT}/obj/apps/dws/XBSAServer
    mkdir -p ${AGENT_ROOT}/obj/dataprocess/datamessage
    mkdir -p ${AGENT_ROOT}/obj/dataprocess/dataconfig
    mkdir -p ${AGENT_ROOT}/obj/dataprocess/datapath
    mkdir -p ${AGENT_ROOT}/obj/dataprocess/datareadwrite
    mkdir -p ${AGENT_ROOT}/obj/dataprocess/vmwarenative
    mkdir -p ${AGENT_ROOT}/obj/dataprocess/jobqosmanager
    mkdir -p ${AGENT_ROOT}/obj/dataprocess/ioscheduler

    # create test obj dir
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/agent
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/rootexec
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/crypto
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/scriptsign
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/datamigration
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/monitor
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/xmlcfg
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/agentcli
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/getinput
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/apps/oracle
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/apps/restore
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/apps/app
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/apps/oraclenative
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/array
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/common
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/device
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/host
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/message/rest
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/message/tcp
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/pluginfx
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/plugins
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/plugins/device
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/plugins/cluster
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/plugins/host
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/plugins/oracle
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/plugins/restore
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/plugins/app
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/plugins/oraclenative
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/plugins/appprotect
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/plugins/dws
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/rest
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/tools
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/securec
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/sqlite
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/json
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/tinyxml
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/alarm
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/taskmanager
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/taskmanager/externaljob
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/taskmanager/filter
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/servicecenter/servicefactory
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/servicecenter/thriftservice
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/servicecenter/certifcateservice
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/servicecenter/services/device
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/servicecenter/services/jobservice
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/servicecenter/timerservice
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/apps/appprotect/
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/apps/appprotect/plugininterface
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/apps/dws/XBSAServer
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/apps/xbsa
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/XBSAClient
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/XBSACom
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/cunitpub
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/tools/monitor/
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/tcp
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/curlhttpclient
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/dataprocess
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/dataprocess/datamessage
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/dataprocess/dataconfig
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/dataprocess/datapath
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/dataprocess/datareadwrite
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/dataprocess/vmwarenative
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/dataprocess/jobqosmanager
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/dataprocess/ioscheduler
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/securecom
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/servicecenter/certificateservice/
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/servicecenter/messageservice
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/curlclient
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/message/tcpssl    
    mkdir -p ${AGENT_ROOT}/test/stubTest/obj/message/archivestream
}

rm_shell()
{
    SHELL_FILES=`ls ${AGENT_ROOT}/bin/*.sh 2>/dev/null`
    if [ "${SHELL_FILES}" != "" ]
    then
        rm ${AGENT_ROOT}/bin/*.sh
    fi

    if [ -d ${AGENT_ROOT}/bin/thirdparty ]
    then
        rm -rf ${AGENT_ROOT}/bin/thirdparty
    fi
}

copy_shell()
{
    rm_shell

    cp ${AGENT_ROOT}/bin/shell/*.sh ${AGENT_ROOT}/bin
    chmod 0755 ${AGENT_ROOT}/bin/*.sh

    #create thirdpatry directory
    if [ ! -d ${AGENT_ROOT}/bin/thirdparty/sample ]
    then
        mkdir -p ${AGENT_ROOT}/bin/thirdparty/sample
    fi

    #copy thirdparty files
    FILE_COUNT=`ls ${AGENT_ROOT}/bin/shell/thirdparty | wc -l`
    if [ ${FILE_COUNT} != 0 ]
    then
        cp -rf ${AGENT_ROOT}/bin/shell/thirdparty/* ${AGENT_ROOT}/bin/thirdparty
        chmod -R 0755 ${AGENT_ROOT}/bin/thirdparty/*
    fi
}

dos2unix_conf_files()
{
    QUERY_FILE_NAME=`ls ${AGENT_ROOT}/conf`
    for FILE_NAME in ${QUERY_FILE_NAME}
    do
        if [ "${FILE_NAME}" = "kmc_config.txt" ] || [ "${FILE_NAME}" = "kmc_config_bak.txt" ] || [ "${FILE_NAME}" = "kmc_store.txt" ] || [ "${FILE_NAME}" = "kmc_store_bak.txt" ] || [ "${FILE_NAME}" = "kmc" ]
        then
            continue
        fi
        
        CURR_FILE=${AGENT_ROOT}/conf/${FILE_NAME}
        BAK_CURR_FILE=${CURR_FILE}.bak
        cat ${CURR_FILE} | tr -d '\r' > ${BAK_CURR_FILE}
        
        cp ${BAK_CURR_FILE} ${CURR_FILE}
        rm ${BAK_CURR_FILE}
    done
}

make_snmp()
{
    echo "Begin to compile snmp."
    if [ "$1" = "clean" ]
    then
        rm -rf ${AGENT_ROOT}/open_src/snmp/snmp++/src/.libs
        return
    fi
    
    if [ -f "${AGENT_ROOT}/open_src/snmp++/src/.libs/libsnmp++.a" ]
    then
        echo "#########################################################"
        echo "   Compile snmp succ."
        echo "#########################################################"
        return
    fi

    OLD_CMD_CC=${CC}
    cd ${AGENT_ROOT}/open_src/snmp++
    if [ $sys = "AIX" ]
    then
        export CXXFLAGS="-gdwarf-2 -q64 -G"
        export OBJECT_MODE=64
    elif [ $sys = "HP-UX" ]
    then
        export CXXFLAGS="-gdwarf-2 +DD64 -w -AA -D_REENTRANT -DHP_UX_IA -DHP_UX -mt -b +z"
    elif [ $sys = "SunOS" ]
    then
        export CXXFLAGS="-gdwarf-2 -m64 -mt -xcode=pic32 -gdwarf-2 -PIC"
        export CXX="CC"
        export CC="cc"
    else
        export CXXFLAGS="-pipe -fpic"
    fi
    
    export ssl_CFLAGS="-I${AGENT_ROOT}/open_src/openssl/include"
    export ssl_LIBS="-lssl -lcrypto"
    export LDFLAGS="-L${AGENT_ROOT}/open_src/openssl"
    ./configure --with-ssl --disable-logging --disable-namespace --enable-shared=no
    
    if [ $sys = "AIX" ]
    then
        # In AIX, use PRIu64 need to define __64BIT__ or _LONG_LONG,
        # now do not know define what, so undefine HAVE_INTTYPES_H to go to old logical branch.
        # check it in the file '/usr/include/inttypes.h' in the AIX
        echo "#undef HAVE_INTTYPES_H" >> config.h
        
        # In AIX, compile failed in auth_priv.cpp, need to replace public Hasher to public AuthSHABase::Hasher
        TMP_FILE=${AGENT_ROOT}/open_src/snmp++/auth_test.cpp
        sed -g 's/public\ Hasher/public\ AuthSHABase::Hasher/' ${AGENT_ROOT}/open_src/snmp++/src/auth_priv.cpp > ${TMP_FILE}
        mv ${TMP_FILE} ${AGENT_ROOT}/open_src/snmp++/src/auth_priv.cpp
        
        # UdpAddress not define override function "operator=(const char *address)"
        # In AIX, compile failed, but IpAddress define contructor function "IpAddress::IpAddress(const char *inaddr)"
        # use contructor to resovle this, but do not know why can be compile failed.
        TMP_FILE=${AGENT_ROOT}/open_src/snmp++/uxsnmp_test.cpp
        sed -g 's/fromaddress\ =\ tmp_buffer;/IpAddress\ ipaddr\ =\ tmp_buffer;fromaddress = ipaddr;/' src/uxsnmp.cpp > ${TMP_FILE}
        mv ${TMP_FILE} ${AGENT_ROOT}/open_src/snmp++/src/uxsnmp.cpp
        
        sed -g 's/fromaddress\ =\ inet_ntoa(((sockaddr_in\&)from_addr)\.sin_addr);/IpAddress\ ipaddr\ =\ inet_ntoa(((sockaddr_in\&)from_addr)\.sin_addr);fromaddress\ =\ ipaddr;/' src/uxsnmp.cpp > ${TMP_FILE}
        mv ${TMP_FILE} ${AGENT_ROOT}/open_src/snmp++/src/uxsnmp.cpp
    elif [ $sys = "HP-UX" ]
    then
        # In HP-UX, the function clock_gettime not support parameter CLOCK_MONOTONIC, 
        # so undefine HAVE_CLOCK_GETTIME to go to old logical branch.
        # check it by `man clock_gettime`
        echo "#undef HAVE_CLOCK_GETTIME" >> config.h
    fi
    
    # start to make
    make ${MAKE_JOB}
    
    main_result=$?
    export CC="${OLD_CMD_CC}"
    if [ ${main_result} != 0 ]
    then
        echo "#########################################################"
        echo "   Compile snmp failed."
        echo "#########################################################"
        
        exit ${main_result}
    fi
    
    if [ ! -f "${AGENT_ROOT}/open_src/snmp++/src/.libs/libsnmp++.a" ]
    then
        echo "#########################################################"
        echo "   Compile snmp failed, libsnmp++.a is not exists."
        echo "#########################################################"
        
        exit 1
    fi
    
    echo "#########################################################"
    echo "   Compile snmp succ."
    echo "#########################################################"
}

make_fcgi()
{
    if [ "$1" = "clean" ]
    then
        return
    fi

    cd ${AGENT_ROOT}/open_src/fcgi
    if [ -f "libfcgi/.libs/libfcgi.a" ]
    then 
        return
    fi
    
    cd ${AGENT_ROOT}/open_src/fcgi
    ./autogen.sh
    ./configure --disable-shared "$FCGI_CP_OPT"   
  
    if [ $sys = "Linux" ]
    then
        sed '34i #include <cstdio>' -i ${AGENT_ROOT}/open_src/fcgi/include/fcgio.h
    fi
    make $MAKE_JOB $1
    main_result=$?
    echo "make_fcgi result: ${main_result}"
    if [ ${main_result} != 0 ]
    then
        echo "#########################################################"
        echo "   Make fcgi failed."
        echo "#########################################################"

        exit ${main_result}
    fi
   
    cp ${AGENT_ROOT}/open_src/fcgi/fcgi_config.h ${AGENT_ROOT}/open_src/fcgi/include/fcgi_config.h
    echo "#########################################################"
    echo "   Make fcgi succ."
    echo "#########################################################"
}

make_curl()
{
    if [ $sys != "Linux" ]
    then
        return
    fi

    if [ "$1" = "clean" ]
    then
        rm -rf ${AGENT_ROOT}/open_src/curl/lib/.libs/
        return 
    fi

    cd ${AGENT_ROOT}/open_src/curl
    if [ -f "lib/.libs/libcurl.a" ]
    then
        return
    fi

    env PKG_CONFIG_PATH=${AGENT_ROOT}/open_src/openssl/.openssl/lib/pkgconfig
    ./configure --prefix=${AGENT_ROOT}/open_src/curl --with-ssl=${AGENT_ROOT}/open_src/openssl --disable-ldap --disable-shared --without-zlib
    
    make ${MAKE_JOB}
    main_result=$?
    if [ ${main_result} != 0 ]
    then
        echo "#########################################################"
        echo "   Make curl failed."
        echo "#########################################################"

        exit ${main_result}
    fi
     
    echo "#########################################################"
    echo "   Make curl succ."
    echo "#########################################################"
}


#Generate sqlite3.c file
make_sqlite3_file()
{
    if [ "$1" = "clean" ]
    then
        return
    fi
    
    cd ${AGENT_ROOT}/open_src/sqlite
    if [ -f "sqlite3.c" ] && [ -f "sqlite3.h" ]
    then
        return
    fi

    chmod +x ./configure
    ./configure
    make $MAKE_JOB sqlite3.c $1
    
    main_result=$?
    if [ ${main_result} != 0 ]
    then
        echo "#########################################################"
        echo "   Make sqlite3.c failed."
        echo "#########################################################"
        
        exit ${main_result}
    fi

    echo "#########################################################"
    echo "   Make sqlite3.c succ."
    echo "#########################################################"
}

prepare_nginx()
{
    # To solve problem in HP-UX,  #error ngx_atomic_cmp_set() is not defined!
    # ngx_rwlock.c is a new file in nginx 1.10.1, It hasn't influence on now version
    n=0
    while IFS= read -r line
    do
        n=`expr $n + 1`
        if [ "$n" = "11" ]
        then
            echo '#ifdef HP_UX_IA'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/core/ngx_rwlock.c1
            echo '#define NGX_HAVE_ATOMIC_OPS 1'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/core/ngx_rwlock.c1
            echo '#endif'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/core/ngx_rwlock.c1
        fi
        echo "$line">> ${AGENT_ROOT}/open_src/nginx_tmp/src/core/ngx_rwlock.c1
    done < ${AGENT_ROOT}/open_src/nginx_tmp/src/core/ngx_rwlock.c
    mv ${AGENT_ROOT}/open_src/nginx_tmp/src/core/ngx_rwlock.c1 ${AGENT_ROOT}/open_src/nginx_tmp/src/core/ngx_rwlock.c
}
make_nginx()
{
    if [ "$1" = "clean" ]
    then
        rm -rf ${AGENT_ROOT}/open_src/nginx
        rm -rf ${AGENT_ROOT}/bin/nginx
        return
    fi

    if [ -f "${AGENT_ROOT}/open_src/nginx/nginx" ]
    then
        return
    fi

    OLD_CFLAGS=${CFLAGS}
    if [ $sys = "AIX" ]
    then
        export OBJECT_MODE=64
    elif [ $sys = "HP-UX" ]
    then
        NGINX_CFG_OPT="--with-cc-opt=-Agcc"
        #to resolve download log failed when the size of log is over 32K
        #HP os,sed ÃüÁî¸ñÊ½ sed '2i\
        ###                         test' xxx.txt
        ROW_NUM_RET=`sed -n "/location/=" "${AGENT_ROOT}/conf/nginx.conf"`
        sed ${ROW_NUM_RET}'i\
        fastcgi_keep_conn on;' ${AGENT_ROOT}/conf/nginx.conf > ${AGENT_ROOT}/conf/nginx.conf.bak
        mv ${AGENT_ROOT}/conf/nginx.conf.bak ${AGENT_ROOT}/conf/nginx.conf
        sed ${ROW_NUM_RET}'i\
        fastcgi_buffers 6 8k;' ${AGENT_ROOT}/conf/nginx.conf > ${AGENT_ROOT}/conf/nginx.conf.bak
        mv ${AGENT_ROOT}/conf/nginx.conf.bak ${AGENT_ROOT}/conf/nginx.conf            
   
	    # HP-UX 11v2 nginx will close connection in ngx_event_accept.c(line 295, c->addr_text.len is zero), because the sa_family is zero after accept(line 82) 
        # do not known cause, in function ngx_event_accept change "socklen_t  socklen" to "int socklen"
        OS_VERSION=`uname -a | awk '{print $3}' | awk -F "." '{print $2"."$3}'`
        #HP-UX 11v2
        if [ "${OS_VERSION}" = "11.23" ]
        then
            export CFLAGS="${CFLAGS} -DHP_UX_11V2"
        fi
    elif [ $sys = "SunOS" ]
    then
        NGINX_CFG_OPT=""    
        CFLAGS=""
    fi
    
    echo "#########################################################"
    echo "   Start to make Nginx."
    echo "#########################################################"

    cd ${AGENT_ROOT}/open_src/nginx_tmp/
    if [  ${NGINX_CPU_OPT_FLAG} -eq 1 ]
    then
        export CFLAGS="${CFLAGS} -Werror"
    fi
    if [ $sys = "SunOS" ]
    then
        ./configure --prefix=../nginx --sbin-path=../nginx --without-http_rewrite_module --without-http_gzip_module --with-http_ssl_module --with-openssl=../openssl --with-cpu-opt=sparc64 --with-cc-opt="-xmemalign -DWSEC_ERR_CODE_BASE=0"
	else
        ./configure --prefix=../nginx --sbin-path=../nginx --without-http_rewrite_module --without-http_gzip_module --with-http_ssl_module --with-openssl=../openssl $NGINX_CFG_OPT
    fi
    
    if [ $sys = "HP-UX" ]
    then
        export LPATH=/usr/lib/hpux64/
    fi

    OPENSSL_DEL_BEGIN_LINE=`cat ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile | grep -n '../openssl/.openssl/include/openssl/ssl.h:'| $AWK -F ":" '{print $1}'`
    OPENSSL_DEL_END_LINE=`expr $OPENSSL_DEL_BEGIN_LINE + 5`
    sed $OPENSSL_DEL_BEGIN_LINE,${OPENSSL_DEL_END_LINE}d ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile >${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile.bak
    rm -rf ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile
    mv ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile.bak ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile
    
    sed -n '/-I ..\/openssl\/.openssl\/include/=' ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile > ${AGENT_ROOT}/open_src/nginx_tmp/objs/Row
    ROW_NUM_RET=`sed -n '2p' ${AGENT_ROOT}/open_src/nginx_tmp/objs/Row`
    if [ $sys = "Linux" -o $sys = "SunOS" ]
    then
        SED_CMD=$ROW_NUM_RET's/\\/-I ..\/..\/platform\/kmc\/include -I ..\/..\/platform\/kmc\/src\/sdp -I ..\/..\/platform\/securec\/include \\/1'
    else
        SED_CMD=$ROW_NUM_RET's/\\\\/-I ..\/..\/platform\/kmc\/include -I ..\/..\/platform\/kmc\/src\/sdp -I ..\/..\/platform\/securec\/include \\\\/1'
    fi
    echo $SED_CMD > ${AGENT_ROOT}/open_src/nginx_tmp/objs/Cmd
    sed -f ${AGENT_ROOT}/open_src/nginx_tmp/objs/Cmd ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile > ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile.bak
    rm -rf ${AGENT_ROOT}/open_src/nginx_tmp/objs/Row
    rm -rf ${AGENT_ROOT}/open_src/nginx_tmp/objs/Cmd
    rm -rf ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile
    mv ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile.bak ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile
    
    sed -n '/..\/openssl\/.openssl\/include\/openssl\/ssl.h/=' ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile > ${AGENT_ROOT}/open_src/nginx_tmp/objs/Row
    ROW_NUM_RET=`sed -n '1p' ${AGENT_ROOT}/open_src/nginx_tmp/objs/Row`
    if [ $sys = "Linux" -o $sys = "SunOS" ]
    then
        SED_CMD=$ROW_NUM_RET's/\\/..\/..\/platform\/kmc\/include\/wsecv2_type.h ..\/..\/platform\/kmc\/include\/kmcv2_itf.h ..\/..\/platform\/kmc\/src\/sdp\/sdpv2_itf.h ..\/..\/platform\/securec\/include\/securec.h \\/1'
    else
        SED_CMD=$ROW_NUM_RET's/\\\\/..\/..\/platform\/kmc\/include\/wsecv2_type.h ..\/..\/platform\/kmc\/include\/kmcv2_itf.h ..\/..\/platform\/kmc\/src\/sdp\/sdpv2_itf.h ..\/..\/platform\/securec\/include\/securec.h \\\\/1'
    fi
    echo $SED_CMD > ${AGENT_ROOT}/open_src/nginx_tmp/objs/Cmd
    sed -f ${AGENT_ROOT}/open_src/nginx_tmp/objs/Cmd ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile > ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile.bak
    rm -rf ${AGENT_ROOT}/open_src/nginx_tmp/objs/Row
    rm -rf ${AGENT_ROOT}/open_src/nginx_tmp/objs/Cmd
    rm -rf ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile
    mv ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile.bak ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile
    
    sed -n '/libcrypto.a/=' ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile > ${AGENT_ROOT}/open_src/nginx_tmp/objs/Row
    ROW_NUM_RET=`sed -n '1p' ${AGENT_ROOT}/open_src/nginx_tmp/objs/Row`
    if [ $sys = "SunOS" ] 
    then 
        SED_CMD=$ROW_NUM_RET's/libcrypto.a/libcrypto.a -lm ..\/..\/platform\/kmc\/lib\/libKMC.a/1'
    else
        SED_CMD=$ROW_NUM_RET's/libcrypto.a/libcrypto.a ..\/..\/platform\/kmc\/lib\/libKMC.a/1'
    fi
    echo $SED_CMD > ${AGENT_ROOT}/open_src/nginx_tmp/objs/Cmd
    sed -f ${AGENT_ROOT}/open_src/nginx_tmp/objs/Cmd ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile > ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile.bak
    rm -rf ${AGENT_ROOT}/open_src/nginx_tmp/objs/Row
    rm -rf ${AGENT_ROOT}/open_src/nginx_tmp/objs/Cmd
    rm -rf ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile
    mv ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile.bak ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile
    
    if [ $sys = "SunOS" ] 
    then
        ROW_NUM_RET=`expr $ROW_NUM_RET + 1`
        SED_CMD=$ROW_NUM_RET's/-m64/-m64 -xmemalign/1'
        echo $SED_CMD > ${AGENT_ROOT}/open_src/nginx_tmp/objs/Cmd
        sed -f ${AGENT_ROOT}/open_src/nginx_tmp/objs/Cmd ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile > ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile.bak
        rm -rf ${AGENT_ROOT}/open_src/nginx_tmp/objs/Cmd
        rm -rf ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile
        mv ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile.bak ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile
    fi
    
    sed -n '/src\/event\/ngx_event_openssl.c/=' ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile > ${AGENT_ROOT}/open_src/nginx_tmp/objs/Row
    ROW_NUM_RET=`sed -n '1p' ${AGENT_ROOT}/open_src/nginx_tmp/objs/Row`
    ROW_NUM_RET=`expr $ROW_NUM_RET + 1`
    SED_CMD=$ROW_NUM_RET's/$(CORE_INCS)/$(CORE_INCS) $(HTTP_INCS)/1'
    echo $SED_CMD > ${AGENT_ROOT}/open_src/nginx_tmp/objs/Cmd
    sed -f ${AGENT_ROOT}/open_src/nginx_tmp/objs/Cmd ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile > ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile.bak
    rm -rf ${AGENT_ROOT}/open_src/nginx_tmp/objs/Row
    rm -rf ${AGENT_ROOT}/open_src/nginx_tmp/objs/Cmd
    rm -rf ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile
    mv ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile.bak ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile
    
    make ${MAKE_JOB} $1
    main_result=$?
    if [ ${main_result} != 0 ]
    then
        echo "#########################################################"
        echo "   Make nginx failed."
        echo "#########################################################"

        exit ${main_result}
    fi

    make install
    main_result=$?
    if [ ${main_result} != 0 ]
    then
        echo "#########################################################"
        echo "   Make install nginx failed."
        echo "#########################################################"

        exit ${main_result}
    fi

    cp -rf ${AGENT_ROOT}/open_src/nginx ${AGENT_ROOT}/bin    
    rm -rf ${AGENT_ROOT}/bin/nginx/conf/*
    cp ${AGENT_ROOT}/conf/nginx.conf ${AGENT_ROOT}/bin/nginx/conf
    cp ${AGENT_ROOT}/conf/fastcgi_params ${AGENT_ROOT}/bin/nginx/conf
    cp ${AGENT_ROOT}/conf/server.crt ${AGENT_ROOT}/bin/nginx/conf
    cp ${AGENT_ROOT}/conf/server.key ${AGENT_ROOT}/bin/nginx/conf
    cp ${AGENT_ROOT}/conf/bcmagentca.crt ${AGENT_ROOT}/bin/nginx/conf
    #rename nginx to rdnginx
    mv ${AGENT_ROOT}/bin/nginx/nginx ${AGENT_ROOT}/bin/nginx/rdnginx
    
    #AIX5.3,nginx cannot running if "ssl_session_cache    shared:SSL:1m;" not comment,
    #now, do not known why, after get the really reason modify this
    if [ "$sys" = "AIX" ]
    then
        OS_VERSION=`oslevel | awk -F "." '{print $1$2}'`
        if [ "${OS_VERSION}" = "53" ]
        then
            sed "s/ssl_session_cache/#ssl_session_cache/" ${AGENT_ROOT}/bin/nginx/conf/nginx.conf > ${AGENT_ROOT}/bin/nginx/conf/nginx.conf.bk
            mv ${AGENT_ROOT}/bin/nginx/conf/nginx.conf.bk ${AGENT_ROOT}/bin/nginx/conf/nginx.conf
        fi
    fi
    
    export CFLAGS=${OLD_CFLAGS}
    echo "#########################################################"
    echo "   Make nginx succ."
    echo "#########################################################"
}

make_gtest()
{
    cd ${AGENT_ROOT}/test/stubTest/obj/
    echo ${CPPC} -fno-access-control -I${GTEST_DIR}/include -I${GTEST_DIR} -c ${GTEST_DIR}/src/gtest-all.cc 
	${CPPC} -fno-access-control -I${GTEST_DIR}/include -I${GTEST_DIR} -c ${GTEST_DIR}/src/gtest-all.cc 
	ar -rv libgtest.a gtest-all.o
	mkdir gtestlib
	mv libgtest.a gtestlib/
	cp -r ${GTEST_DIR}/include gtestlib/
	rm gtest-all.o
    echo "#########################################################"
    echo "   Make gtest succ."
    echo "#########################################################"
}

make_fuzz()
{
    if [ -f "${AGENT_ROOT}/test/stubTest/src/secodefuzz/OpenSource/test/test1/libSecodefuzz.a" ]; then
        return
    fi
    cd ${AGENT_ROOT}/test/stubTest/src/secodefuzz/OpenSource/test/test1
    make
    echo "#########################################################"
    echo "   Make fuzz succ."
    echo "#########################################################"
}

make_init()
{
    if [ $# != 0 ]
    then
        if [ "$1" = "coverage" ]; then
            GCOV_COMPILE_FLAG="-fprofile-arcs -ftest-coverage -fdump-rtl-expand"
            GCOV_LINK_FLAG="-lgcov -luuid"
            MST_COVERAGE=1
            export GCOV_COMPILE_FLAG GCOV_LINK_FLAG
        elif [ "$1" = "fuzz" ]; then
            FUZZ_CFLAGS="-g -O0 -ftest-coverage -fprofile-arcs -fdump-rtl-expand -fsanitize=address -fsanitize=undefined -fsanitize-coverage=trace-pc"
            FUZZ_LDFLAGS="-fsanitize=address -fsanitize=undefined -static-libasan -static-libubsan"
            FUZZ_SUPPORT=1
        else
            echo "Please use 'sh test_make.sh [coverage] or [fuzz]'"
			exit 2
        fi
	else
        GCOV_COMPILE_FLAG=""
        GCOV_LINK_FLAG=""
        export GCOV_COMPILE_FLAG GCOV_LINK_FLAG
    fi

    if [ $sys = "AIX" ]
    then
        CPPC=xlC_r
        CC=xlC_r
        cc=xlC_r
        OS_VERSION=`oslevel | awk -F "." '{print $1$2}'`
        if [ "${OS_VERSION}" = "53" ]
        then
            CFLAGS="-gdwarf-2 -q64 -DSTDCXX_98_HEADERS -DAIX -DAIX53 $KMC_OPT"
        else
            CFLAGS="-gdwarf-2 -q64 -DSTDCXX_98_HEADERS -DAIX $KMC_OPT"
        fi
        cFLAGS=${CFLAGS}
        OFLAGS="-gdwarf -q64 -brtl -bexpall"
        oFLAGS=${OFLAGS}
        DFLAGS="-gdwarf -q64 -qmkshrobj -G"
        dFLAGS=${DFLAGS}
    elif [ $sys = "HP-UX" ]
    then
        CPPC=aCC
        CC=aCC
        #for sqlite and securec
        cc=cc
        CFLAGS="-gdwarf-2 -w -AA +DD64 -D_REENTRANT -DHP_UX_IA -DHP_UX -mt $KMC_OPT"
        cFLAGS="-gdwarf-2 -w +DD64 -D_REENTRANT -DHP_UX_IA -DHP_UX -mt"
        CXXFLAGS="-gdwarf-2 +DD64 -w -AA -D_REENTRANT -DHP_UX_IA -DHP_UX -mt -b +z"
        oFLAGS="-gdwarf-2 +DD64 -ldl"
        OFLAGS="-gdwarf-2 +DD64 -ldl -ldcekt"
        dFLAGS="-gdwarf-2 +DD64 -b"
        DFLAGS="-gdwarf-2 +DD64 -b -ldcekt"
    elif [ $sys = "Linux" ]
    then
        sysLinuxName=`cat /etc/issue | grep 'Linx'`
        if [ -f /etc/SuSE-release ]
        then
            OS_NAME=SUSE
        elif [ -f /etc/isoft-release ]
        then
            OS_NAME=ISOFT
        elif [ -f /etc/redhat-release ]
        then
            OS_NAME=REDHAT
        elif [ "${sysLinuxName}" != "" ]
        then
            OS_NAME=ROCKY
        elif [ -z "${sysLinuxName}" ]
        then
            sysLinuxName_tmp=`cat /etc/issue | grep 'Rocky'`
            if [ "${sysLinuxName_tmp}" != "" ]
            then
                OS_NAME=ROCKY
                NGINX_CPU_OPT_FLAG=1
            fi
        fi

        if [ ${FORTIFY} -eq 1 ]
        then
            CPPC="sourceanalyzer -b rdagent g++"
            CC="sourceanalyzer -b rdagent gcc"
            cc="sourceanalyzer -b rdagent gcc"
        else
            CPPC="g++"
            CC="gcc"
            cc="gcc"
            if [ ${MST_COVERAGE} -eq 1 ]
            then
                COV_CFLAGS="-gdwarf-2 -fprofile-arcs -ftest-coverage"
                COV_OFLAGS="-gdwarf-2 -ftest-coverage -fprofile-arcs -lgcov"
            fi
        fi
        
        CompareSysVersion
        
        # add define STDCXX_98_HEADERS, support for snmp++ 3.3.7
        if [ ${FREEZE_SUPPORT} -eq 1 ]
        then
            CFLAGS="-gdwarf-2 -pipe -m64 -fpic -DLINUX -DFRAME_SIGN -DLIN_FRE_SUPP -DSTDCXX_98_HEADERS -D${OS_NAME} $KMC_OPT $FUZZ_CFLAGS -fno-access-control"
        else
            CFLAGS="-gdwarf-2 -pipe -m64 -fpic -DLINUX -DFRAME_SIGN -DSTDCXX_98_HEADERS -D${OS_NAME} $KMC_OPT $FUZZ_CFLAGS -fno-access-control"
        fi
        
        cFLAGS=${CFLAGS}
        OFLAGS="-gdwarf-2 -m64 -luuid $NX_OPT $SP_OPT $RELRO_OPT $RPATH_OPT $FUZZ_LDFLAGS"
        oFLAGS="-gdwarf-2 -m64 $NX_OPT $SP_OPT $RELRO_OPT $RPATH_OPT $FUZZ_LDFLAGS"
        DFLAGS="-gdwarf-2 -m64 -shared -luuid $FUZZ_LDFLAGS"
        dFLAGS="-gdwarf-2 -m64 -shared $FUZZ_LDFLAGS"
    elif [ $sys = "SunOS" ]
    then
        CPPC=CC
        CC=cc
        #for sqlite and securec
        cc=cc
        CFLAGS="-xmemalign -m64 -DSTDCXX_98_HEADERS -D_REENTRANT -mt -gdwarf-2 -xcode=pic32 -DSOLARIS $KMC_OPT"
        CXXFLAGS="" 
        cFLAGS=$CFLAGS
        OFLAGS="-gdwarf-2 -xmemalign -pg -m64 -lsocket -lnsl -luuid"
        oFLAGS=${OFLAGS}
        DFLAGS="-gdwarf-2 -xmemalign -m64 -shared -lsocket -lnsl -luuid"
        dFLAGS=${DFLAGS}
    else
        echo "Unsupported OS"
        exit 0
    fi
}

make_openssl()
{
    OPENSSL_INSTALL_PATH=${AGENT_ROOT}/open_src/openssl/.openssl
    #the openssl 1.0.2 version need compile with dynamic file for solaris, so modify option "no-shared" to "shared"
    if [ $sys != "SunOS" ]
    then
        OPENSSL_OPT="--prefix=$OPENSSL_INSTALL_PATH no-shared no-threads"
    else
        OPENSSL_OPT="--prefix=$OPENSSL_INSTALL_PATH no-shared no-threads no-asm"
    fi
	
    if [ "$1" = "clean" ]
    then
        echo "clean openssl"
    elif [ ! -f "$OPENSSL_INSTALL_PATH/lib/libssl.a" ] || [ ! -f "$OPENSSL_INSTALL_PATH/lib/libcrypto.a" ]
    then
        cd ${AGENT_ROOT}/open_src/openssl
        chmod +x ./config
        chmod +x ./Configure
        `ls | xargs touch`
        if [ $sys = "AIX" ]
        then
            ./Configure aix64-cc $OPENSSL_OPT
        elif [ $sys = "HP-UX" ]
        then
            ./Configure hpux64-ia64-cc $OPENSSL_OPT
        elif [ $sys = "SunOS" ]
        then
            sed '275s/-xarch=v9/-m64/g' ${AGENT_ROOT}/open_src/openssl/Configure > ${AGENT_ROOT}/open_src/openssl/Configure.bak
            rm -rf ${AGENT_ROOT}/open_src/openssl/Configure
            mv ${AGENT_ROOT}/open_src/openssl/Configure.bak ${AGENT_ROOT}/open_src/openssl/Configure
            chmod 777 ${AGENT_ROOT}/open_src/openssl/Configure
            echo >>${AGENT_ROOT}/open_src/openssl/Makefile
            ./Configure solaris64-sparcv9-cc -xcode=pic32 $OPENSSL_OPT
        else
            ./config -fPIC $OPENSSL_OPT
        fi
        make $MAKE_JOB $1
        main_result=$?
        if [ ${main_result} != 0 ]
        then
            echo "#########################################################"
            echo "   Compile openssl failed."
            echo "#########################################################"
            exit ${main_result}
        fi
        make install
        main_result=$?
        if [ ${main_result} != 0 ]
        then
            echo "#########################################################"
            echo "   Compile openssl failed."
            echo "#########################################################"
            exit ${main_result}
        fi
    fi
    cp "${AGENT_ROOT}/open_src/openssl/.openssl/bin/openssl" "${AGENT_ROOT}/bin"
    cp "${AGENT_ROOT}/open_src/openssl/apps/openssl.cnf" "${AGENT_ROOT}/conf"
    echo "#########################################################"
    echo "   Compile openssl succ."
    echo "#########################################################"
}

generate_thrift_cpp()
{
    if [ "$1" = "clean" ]
    then
        return
    fi

    echo "generate thrift code"
    THRIFT_BIN_DIR=${AGENT_ROOT}/open_src/thrift/.libs/bin
    if [ ! -d "${THRIFT_BIN_DIR}" ];then
        echo "cannot find bin path"
        return
    fi

    if [ ! -f "${THRIFT_BIN_DIR}/thrift" ];then
        echo "thrift binary not exists"
        return
    fi
    echo ${THRIFT_BIN_DIR}
    cd ${THRIFT_BIN_DIR}
    rm -rf ${DEFAULT_DIR}/*.h
    rm -rf ${DEFAULT_DIR}/*.cpp
    rm -rf ./*.thrift
    cp -f ${AGENT_ROOT}/test/stubTest/src/src/servicecenter/thrift/test.thrift ./
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
        # ?skeleton.cpp???skeleton.cpp.bak??????????
    FILENAME=`ls ${DEFAULT_DIR}/*skeleton.cpp`
    echo ${FILENAME}
    rm -f ${FILENAME}
    cp -f ${DEFAULT_DIR}/*.h  ${AGENT_ROOT}/test/stubTest/src/src/servicecenter/thriftservice
    cp -f ${DEFAULT_DIR}/*.cpp ${AGENT_ROOT}/test/stubTest/src/src/servicecenter/thriftservice
    echo "#########################################################"
    echo "   generate thrift cpp succ."
    echo "#########################################################"
}

main_enter()
{
    if [ ${AGENT_ROOT:-0} = 0 ]
    then
        echo "Please source env.csh first"
        exit 2
    else
        create_dir
    fi

    dos2unix_conf_files

    MAKE_OPTION=
    #don't clean third part objs
    if [ ${CLEAN} -eq 1 ]
    then
        MAKE_OPTION="clean"
        make $MAKE_JOB -f ${AGENT_ROOT}/test/stubTest/build/makefile ${MAKE_OPTION}
    else
        if [ ${CLEAN_ALL} -eq 1 ]
        then
            MAKE_OPTION="clean"
            rm_shell
        else
            copy_shell
        fi

        make_gtest

        generate_thrift_cpp

        COMPILE_TIME=`date`
        sed "9s/compile/$COMPILE_TIME/1" ${AGENT_ROOT}/src/inc/common/AppVersion.h >  ${AGENT_ROOT}/src/inc/common/AppVersion.h.bak
        rm -rf ${AGENT_ROOT}/src/inc/common/AppVersion.h
        mv ${AGENT_ROOT}/src/inc/common/AppVersion.h.bak ${AGENT_ROOT}/src/inc/common/AppVersion.h
        echo "began to compile makefile..."       
        
        if [ ${FUZZ_SUPPORT} -eq 1 ]; then
            FUZZ_CFLAGS+=" -DFUZZ"
            FUZZ_LDFLAGS+=" -DFUZZ"
            make_fuzz
            make $MAKE_JOB -f ${AGENT_ROOT}/test/stubTest/build/makefile fuzz
        else
            make $MAKE_JOB -f ${AGENT_ROOT}/test/stubTest/build/makefile ${MAKE_OPTION}
        fi
    fi
}

compile_gcov_out()
{
    if [ "$MST_COVERAGE" = "1" ]
    then 
        ${CC} -shared -fPIC ${AGENT_ROOT}/test/stubTest/auto/src/gcov_out.c -o ${AGENT_ROOT}/obj/gcov_out.so
    fi
}

echo "#########################################################"
echo "   Copyright (C), 2013-2014, Huawei Tech. Co., Ltd."
echo "   Start to compile Agent "
echo "#########################################################"
StartTime=`date '+%Y-%m-%d %H:%M:%S'`

make_init $*

if [ $sys = "SunOS" ]
then
    sed 's/-lsnmp++/-lsnmp++ -lresolv/g' makefile > makefile.bak
    mv makefile.bak makefile
fi

export CPPC CC cc CFLAGS cFLAGS CXXFLAGS  OFLAGS oFLAGS DFLAGS dFLAGS AGENT_BUILD_NUM
export COV_CFLAGS COV_OFLAGS

main_enter

main_result=$?

if [ ${main_result} != 0 ]
then
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
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${AGENT_ROOT}/bin

exit $main_result

