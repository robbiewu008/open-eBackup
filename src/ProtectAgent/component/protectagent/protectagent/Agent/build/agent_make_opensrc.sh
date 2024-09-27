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
if [ "$sys" = "SunOS" ]; then
    AWK=nawk
else
    AWK=awk
fi
SYS_PLATFORM=`uname -p`

if [ "$sys" = "Linux" ] || [ "$sys" = "AIX" ] || [ "$sys" = "SunOS" ]; then
    MAKE_JOB="-j4"
else
    MAKE_JOB=""
fi
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
REST_PUBLISH=0

#open source dir
OPEN_SRC_PACKET_DIR=${AGENT_ROOT}/open_src
OPEN_SRC_FCGI_DIR=fcgi
OPEN_SRC_JSONCPP_DIR=jsoncpp
OPEN_SRC_NGINX_DIR=nginx
OPEN_SRC_OPENSSL_DIR=openssl
OPEN_SRC_SQLITE_DIR=sqlite
OPEN_SRC_SNMP_DIR=snmp++
OPEN_SRC_TINYXML_DIR=tinyxml
OPEN_SRC_CURL_DIR=curl
OPEN_SRC_UTIL_LINUX_DIR=util-linux
OPEN_SRC_GPERFTOOLS=gperftools
OPEN_SRC_BOOST_DIR=boost
OPEN_SRC_THRIFT_DIR=thrift
OPEN_SRC_LIBEVENT_DIR=libevent
OPEN_SRC_ZLIB_DIR=zlib

#gcc secure opt
NX_OPT="-Wl,-z,noexecstack"
SP_OPT="-fstack-protector-strong --param ssp-buffer-size=4 -Wstack-protector"
RELRO_OPT="-Wl,-z,relro"
RPATH_OPT="-Wl,--disable-new-dtags"
BIND_NOW_OPT="-Wl,-z,now"
PIE_OPT="-fPIE -pie"
FS_OPT="-D_FORTIFY_SOURCE=2 -O2"


#Generate sqlite3.c file
make_sqlite3_file()
{
    if [ "$1" = "clean" ]; then
        return
    fi
    
    if [ -f "${AGENT_ROOT}/bin/sqlite3" ]; then
        return
    fi

    cd ${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_SQLITE_DIR}
    if [ -f "configure" ]; then
        chmod +x configure
        ./configure >> ./build_make.log 2>&1
    fi

    if [ -f "Makefile" ]; then
        make shell.c sqlite3.c 
    fi

    if [ ! -f "sqlite3.c" ] || [ ! -f "sqlite3.h" ] || [ ! -f "shell.c" ]; then
        echo "Compile sqlite3 failed for incomplete files."
        return
    fi

    echo "compiling sqlite3, log is in build_make.log..."
    if [ $sys = "Linux" ]; then
        ${CC} $NX_OPT $SP_OPT $RELRO_OPT $RPATH_OPT $BIND_NOW_OPT ${PIE_OPT} ${FS_OPT} -s shell.c sqlite3.c -lpthread -ldl -o ${AGENT_ROOT}/bin/sqlite3 >> ./build_make.log
    elif [ "$sys" = "SunOS" ]; then
        ${CC} -m64 -s shell.c sqlite3.c -lpthread -ldl -o ${AGENT_ROOT}/bin/sqlite3 >> ./build_make.log 2>&1
    else
        ${CC} -maix64 -s shell.c sqlite3.c -lpthread -ldl -o ${AGENT_ROOT}/bin/sqlite3 >> ./build_make.log 2>&1
    fi
    main_result=$?
    if [ ${main_result} != "0" ]; then
        echo "#########################################################"
        echo "   Make sqlite3.c failed."
        echo "#########################################################"
        exit ${main_result}
    fi

    echo "#########################################################"
    echo "   Make sqlite3.c succ."
    echo "#########################################################"
}

make_openssl()
{
    echo "Begin to compile openssl."

    OPENSSL_INSTALL_PATH=${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_OPENSSL_DIR}/.openssl
    #the openssl 1.0.2 version need compile with dynamic file for solaris, so modify option "no-shared" to "shared"
    if [ "$sys" != "SunOS" ]; then
        OPENSSL_OPT="--prefix=$OPENSSL_INSTALL_PATH no-shared no-tests"
    else
        OPENSSL_OPT="--prefix=$OPENSSL_INSTALL_PATH no-shared no-tests no-asm"
    fi
	
    LIBCRYPTO_NAME="libcrypto.a"
    LIBSSL_NAME="libssl.a"

    if [ "$1" = "clean" ]; then
        [ -d "${OPENSSL_INSTALL_PATH}" ] && rm -rf "${OPENSSL_INSTALL_PATH}"
        echo "clean openssl success."
        exit 0
    elif [ ! -f "$OPENSSL_INSTALL_PATH/lib/libssl.a" ] || [ ! -f "$OPENSSL_INSTALL_PATH/lib/libcrypto.a" ]; then
        cd ${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_OPENSSL_DIR}
        chmod +x ./config
        chmod +x ./Configure
        `ls | xargs touch`
        if [ $sys = "AIX" ]; then
            ./Configure aix64-gcc $OPENSSL_OPT >> ./build_make.log 2>&1
        elif [ $sys = "HP-UX" ]; then
            ./Configure hpux64-ia64-cc $OPENSSL_OPT >> ./build_make.log 2>&1
        elif [ "$sys" = "SunOS" ]; then
            # resolve issue follow https://github.com/openssl/openssl/issues/6333
            ./Configure solaris64-sparcv9-cc -xcode=pic32 $OPENSSL_OPT
            sed 's/CNF_EX_LIBS=.*/& -lpthread -lrt/g' ${AGENT_ROOT}/open_src/openssl/Makefile > ${AGENT_ROOT}/open_src/openssl/Makefile.bk
            mv ${AGENT_ROOT}/open_src/openssl/Makefile.bk ${AGENT_ROOT}/open_src/openssl/Makefile
        else
            ./config -fPIC $RELRO_OPT $OPENSSL_OPT $BIND_NOW_OPT -fstack-protector-all  -Wl,-z,noexecstack -pie -s >> ./build_make.log 2>&1
        fi
        #before comiling openssl,patch it on solaris to fix sk_num_null issue
        if [ "$sys" = "SunOS" ]; then
            #check if the .patch file exists
            if [ ! -f "${AGENT_ROOT}/open_src/patch/openssl_lhash.patch" ] || [ ! -f "${AGENT_ROOT}/open_src/patch/openssl_safestack.patch" ]; then
                echo "#########################################################"
                echo "Can't find the patch files on Solaris to fix sk_num_null!"
                echo "#########################################################"
            else
                gpatch -p2 ${AGENT_ROOT}/open_src/openssl/include/openssl/lhash.h < ${AGENT_ROOT}/open_src/patch/openssl_lhash.patch
                gpatch -p2 ${AGENT_ROOT}/open_src/openssl/include/openssl/safestack.h < ${AGENT_ROOT}/open_src/patch/openssl_safestack.patch
            fi
        fi

        echo "compiling openssl, log is in build_make.log..."
        make $MAKE_JOB $1 >> ./build_make.log 2>&1
        main_result=$?
        if [ ${main_result} != 0 ]; then
            echo "#########################################################"
            echo "   Compile openssl failed."
            echo "#########################################################"
            exit ${main_result}
        fi
        make install_sw
        make install_ssldirs
        if [ ! -f "$OPENSSL_INSTALL_PATH/lib/${LIBCRYPTO_NAME}" ] || [ ! -f "$OPENSSL_INSTALL_PATH/lib/${LIBSSL_NAME}" ] || [ ! -f "$OPENSSL_INSTALL_PATH/bin/openssl" ]; then
            echo "#########################################################"
            echo "   Compile openssl failed."
            echo "#########################################################"
            exit 1
        fi
        main_result=$?
        if [ ${main_result} != 0 ]; then
            echo "#########################################################"
            echo "   make install openssl failed."
            echo "#########################################################"
            exit ${main_result}
        fi
    fi
    cp ${AGENT_ROOT}/open_src/openssl/.openssl/bin/openssl "${AGENT_ROOT}/bin"
    cp ${AGENT_ROOT}/open_src/openssl/apps/openssl.cnf "${AGENT_ROOT}/conf"
    echo "#########################################################"
    echo "   Compile openssl succ."
    echo "#########################################################"
}

make_kmc()
{
    echo "Begin to compile kmc."
    cp -rf ${AGENT_ROOT}/platform/kmc ${AGENT_ROOT}/platform/kmcv1

    [ ! -f "${AGENT_ROOT}/platform/kmc/kmc_make.sh" ] && cp "${AGENT_ROOT}/build/kmc/kmc_make.sh" "${AGENT_ROOT}/platform/kmc/kmc_make.sh"
    [ ! -f "${AGENT_ROOT}/platform/kmc/makefile_imp" ] && cp "${AGENT_ROOT}/build/kmc/makefile_imp" "${AGENT_ROOT}/platform/kmc/makefile_imp"
    sh ${AGENT_ROOT}/platform/kmc/kmc_make.sh $1
    main_result=$?
    if [ ${main_result} != 0 ]; then
        echo "#########################################################"
        echo "   Compile kmc failed."
        echo "#########################################################"
        exit ${main_result}
    fi
    if [ "${sys}" = "Linux" ]; then
        cp -rf "${AGENT_ROOT}/build/kmcv1/kmc_make.sh" "${AGENT_ROOT}/platform/kmcv1/kmc_make.sh"
        cp -rf "${AGENT_ROOT}/build/kmcv1/makefile_imp" "${AGENT_ROOT}/platform/kmcv1/makefile_imp"
        sh ${AGENT_ROOT}/platform/kmcv1/kmc_make.sh $1
        main_result=$?
        if [ ${main_result} != 0 ]; then
            echo "#########################################################"
            echo "   Compile kmcv1 failed."
            echo "#########################################################"
            exit ${main_result}
        fi
    fi

    echo "#########################################################"
    echo "   Compile kmc succ."
    echo "#########################################################"
}

make_uuid()
{
    if [ "$1" = "clean" ]; then
        rm -rf ${AGENT_ROOT}/open_src/$OPEN_SRC_UTIL_LINUX_DIR
        rm -rf ${AGENT_ROOT}/open_src/libuuid
        return
    fi

    echo "Begin to compile libuuid."
    if [ "$sys" = "AIX" ] || [ "$sys" = "SunOS" ]; then
        echo "The current system(AIX) does not support tcmalloc."
        return
    fi

    DEST_INSTALL_DIR=${AGENT_ROOT}/open_src/libuuid

    if [ -f "${DEST_INSTALL_DIR}/lib/libuuid.a" ]; then
        return
    fi

    /bin/rm -rf ${DEST_INSTALL_DIR}
    if [ ! -d "${AGENT_ROOT}/open_src/$OPEN_SRC_UTIL_LINUX_DIR" ]; then
        echo "no libuuid src compile failed"
        exit 1;
    fi
    cd ${AGENT_ROOT}/open_src/$OPEN_SRC_UTIL_LINUX_DIR
    OLD_CFLAGS="${CFLAGS}"
    OLD_CPPFLAGS="${CPPFLAGS}"
    export CPPFLAGS="-fPIC -fstack-protector-strong -Wl,-z,now"
    export CFLAGS="-fPIC -fstack-protector-strong -Wl,-z,now"
    ./autogen.sh >> ./build_make.log 2>&1
    if [ $? -ne 0 ]; then
        echo "#########################################################"
        echo "   make libuuid failed."
        echo "#########################################################"
        exit $?
    fi
    ./configure --disable-all-programs --enable-libuuid --with-bashcompletiondir=${DEST_INSTALL_DIR} --prefix=${DEST_INSTALL_DIR} >> ./build_make.log 2>&1
    echo "#########################################################"
    echo "   make libuuid succ."
    echo "#########################################################"
    make $MAKE_JOB $1 >> ./build_make.log 2>&1
    main_result=$?
    if [ ${main_result} != 0 ]; then
        echo "#########################################################"
        echo "   Compile libuuid failed."
        echo "#########################################################"
        exit 1;
    fi
    make install
    main_result=$?
    if [ ${main_result} != 0 ]; then
        echo "#########################################################"
        echo "    make install libuuid failed."
        echo "#########################################################"
        exit 1;
    fi
    echo "#########################################################"
    echo "   Compile libuuid succ."
    echo "#########################################################"
    CPPFLAGS=${OLD_CPPFLAGS}
    export CPPFLAGS
    CFLAGS=${OLD_CFLAGS}
    export CFLAGS
}

# 安装thrift之前，检查是否boost库是否已经安装
make_boost()
{
    BOOST_INSTALL_PATH=${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_BOOST_DIR}/.libs
    BOOST_SHARED_VAR=`ls ${BOOST_INSTALL_PATH}/lib/libboost*.so 2>/dev/null`
    BOOST_STATIC_VAR=`ls ${BOOST_INSTALL_PATH}/lib/libboost*.a 2>/dev/null`
    if [ -n "${BOOST_SHARED_VAR}" ] || [ -n "${BOOST_STATIC_VAR}" ]; then
        echo "boost library exists."
        return
    fi

    # install boost
    echo "Begin to compile boost."
    cd ${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_BOOST_DIR}

    if [ "${sys}" = "AIX" ]; then
        export CXX=gcc
        export CFLAGS="-w -pipe -maix64 -pthread -fpic"
        export CXXFLAGS="-maix64 -pthread -Wno-attributes -lstdc++"
        sh ./tools/build/src/engine/build.sh cxx
        if [ $? -ne 0 ]; then
            echo "Failed to run the boost build.sh script."
            exit 1
        fi
        echo "The build.sh script of boost is successfully executed."
        cp "${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_BOOST_DIR}/tools/build/src/engine/b2" "${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_BOOST_DIR}"
    fi


    if [ -f "bootstrap.sh" ]; then
        chmod +x bootstrap.sh
        if [ "${sys}" = "AIX" ]; then
            ./bootstrap.sh --with-bjam=${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_BOOST_DIR}/tools/build/src/engine/b2 --with-libraries=chrono,date_time,filesystem,log,regex,serialization,system,thread -with-toolset=gcc
        else
            sh bootstrap.sh --with-libraries=chrono,date_time,filesystem,log,regex,serialization,system,thread --with-toolset=gcc
        fi

        if [ $? -ne 0 ]; then
            echo "bootstrap.sh execute fail"
            exit 1
        fi
    else
        echo "cannot find booststrap.sh"
        exit 1
    fi

    if [ -f "b2" ]; then
        if [ "${sys}" = "AIX" ]; then
            ${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_BOOST_DIR}/tools/build/src/engine/b2 --build-dir=./build-tmp --stagedir=./libs-tmp address-model=64 link=static >> ./build_make.log 2>&1
            if [ $? -ne 0 ]; then
                echo "Failed to execute b2."
                exit 1
            fi
        else
            ./b2 toolset=gcc >> ./build_make.log 2>&1
        fi
    else
        echo "cannot find binary b2"
        exit 1
    fi

    if [ "${sys}" = "AIX" ]; then
        if [ ! -d "${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_BOOST_DIR}/.libs" ]; then
            mkdir "${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_BOOST_DIR}/.libs"
        fi

        if [ ! -d "${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_BOOST_DIR}/.libs/include" ]; then
            mkdir "${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_BOOST_DIR}/.libs/include"
        fi
        rm -rf "${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_BOOST_DIR}/.libs/include"/*
        cp -rf "${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_BOOST_DIR}/boost" "${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_BOOST_DIR}/.libs/include"

        if [ ! -d "${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_BOOST_DIR}/.libs/lib" ]; then
            mkdir "${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_BOOST_DIR}/.libs/lib"
        fi
        rm -rf "${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_BOOST_DIR}/.libs/lib"/libboost*.a
        cp -rf "${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_BOOST_DIR}/libs-tmp/lib"/libboost*.a "${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_BOOST_DIR}/.libs/lib"
        rm -rf "${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_BOOST_DIR}/.libs/lib"/libboost*.so
        cp -rf "${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_BOOST_DIR}/libs-tmp/lib"/libboost*.so "${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_BOOST_DIR}/.libs/lib"
    else
        ./b2 install --prefix=${BOOST_INSTALL_PATH}
    fi

    echo "#########################################################"
    echo "   install boost succ."
    echo "#########################################################"
}

make_libevent()
{
    # install libevent
    LIBEVENT_INSTALL_PATH=${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_LIBEVENT_DIR}/.libs
    if [ $sys = "AIX" ]; then
        LIBEVENT_VAR=`ls ${LIBEVENT_INSTALL_PATH}/lib/*event*.a`
    else
        LIBEVENT_VAR=`ls ${LIBEVENT_INSTALL_PATH}/lib/*event*.so`
    fi
    if [ -n "${LIBEVENT_VAR}" ]; then
        echo "libevent library exists"
        return
    fi

    echo "Begin to compile libevent."
    cd ${AGENT_ROOT}/open_src/libevent
    if [ -f "autogen.sh" ]; then
        sh autogen.sh
    else
        echo "autogen.sh is not exist"
        return
    fi

    OLD_CMD_CC=${CC}
    OLD_CFLAGS="${CFLAGS}"

    if [ "${sys}" == "AIX" ]; then
        export CFLAGS="-w -pipe -maix64 -fpic"
        export CXXFLAGS="-w -pipe -maix64 -fpic"
    elif [ ${SYS_PLATFORM} != "aarch64" ]; then
        export CFLAGS="-w -pipe -m64 -fpic"
        export CXXFLAGS="-w -pipe -m64 -fpic"
    else
        export CFLAGS="-w -pipe -fpic"
        export CXXFLAGS="-w -pipe -fpic"
    fi

    if [ -f "configure" ]; then
        ./configure --prefix=${LIBEVENT_INSTALL_PATH} --disable-openssl >> ./build_make.log 2>&1
    else
        echo "configure is not exist"
        export CFLAGS="${OLD_CFLAGS}"
        return
    fi

    echo "compiling libevent, log is in build_make.log..."
    make $MAKE_JOB $1 >> ./build_make.log 2>&1
    main_result=$?
    if [ ${main_result} != "0" ]; then
        echo "#########################################################"
        echo "   Make libevent failed."
        echo "#########################################################"
        exit ${main_result}
    fi
    export CC="${OLD_CMD_CC}"
    export CFLAGS="${OLD_CFLAGS}"

    make install
    main_result=$?
    if [ ${main_result} != "0" ]; then
        echo "#########################################################"
        echo "   Make install libevent failed."
        echo "#########################################################"
        exit ${main_result}
    fi

    echo "#########################################################"
    echo "   Make libevent succ."
    echo "#########################################################"
}

make_zlib()
{
    # install zlib
    ZLIB_INSTALL_PATH=${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_ZLIB_DIR}/.libs
    if [ $sys = "AIX" ]; then
        ZLIB_VAR=`ls ${ZLIB_INSTALL_PATH}/lib/*z*.a`
    else
        ZLIB_VAR=`ls ${ZLIB_INSTALL_PATH}/lib/*z*.so`
    fi
    if [ -n "${ZLIB_VAR}" ]; then
        echo "zlib library exists"
        return
    fi

    OLD_CMD_CC=${CC}
    OLD_CFLAGS="${CFLAGS}"

    if [ "${sys}" == "AIX" ]; then
        export CFLAGS='-w -pipe -maix64 -fpic'
        export CXXFLAGS="-w -pipe -maix64 -fpic"
    elif [ ${SYS_PLATFORM} != "aarch64" ]; then
        export CFLAGS="-w -pipe -m64 -fpic"
        export CXXFLAGS="-w -pipe -m64 -fpic"
    else
        export CFLAGS="-w -pipe -fpic"
        export CXXFLAGS="-w -pipe -fpic"
    fi

    echo "Begin to install zlib."
    echo "OBJECT_MODE=$OBJECT_MODE,CC=$CC,CFLAGS=$CFLAGS"
    cd ${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_ZLIB_DIR}
    ./configure --prefix=${ZLIB_INSTALL_PATH} >> ./build_make.log 2>&1
    if [ $? -ne 0 ]; then
        echo "Failed to configure Zlib."
        exit 1
    fi

    echo "compiling zlib, log is in build_make.log..."
    make $MAKE_JOB $1 >> ./build_make.log 2>&1
    main_result=$?
    if [ ${main_result} != "0" ]; then
        echo "#########################################################"
        echo "   Make zlib failed."
        echo "#########################################################"
        exit ${main_result}
    fi
    export CC="${OLD_CMD_CC}"
    export CFLAGS="${OLD_CFLAGS}"

    make install
    main_result=$?
    if [ ${main_result} != "0" ]; then
        echo "#########################################################"
        echo "   Make install zlib failed."
        echo "#########################################################"
        exit ${main_result}
    fi

    echo "#########################################################"
    echo "   Make zlib succ."
    echo "#########################################################"
}

prepare_thrift()
{
    if [ "$1" = "clean" ]; then
        return
    fi
    make_boost
    make_libevent
    make_zlib
}

make_thrift_aix()
{
    cd "${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_THRIFT_DIR}"
    mkdir build-cmake
    cd "${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_THRIFT_DIR}/build-cmake"

    LIBEVENT_INSTALL_PATH=${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_LIBEVENT_DIR}/.libs
    BOOST_INSTALL_PATH=${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_BOOST_DIR}/.libs
    ZLIB_INSTALL_PATH=${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_ZLIB_DIR}/.libs
    OPENSSL_INSTALL_CRYPTO_PATH=${OPEN_SRC_PACKET_DIR}/openssl/.openssl/lib/libcrypto.a
    OPENSSL_INSTALL_SSL_PATH=${OPEN_SRC_PACKET_DIR}/openssl/.openssl/lib/libssl.a
    OPENSSL_INCLUDE_PATH=${OPEN_SRC_PACKET_DIR}/openssl/.openssl/include
    export AR="ar -X64"
    export OBJECT_MODE=64
    export CFLAGS='-w -pipe -maix64 -fpic -pthread -mminimal-toc'
    export CXXFLAGS='-w -pipe -maix64 -fpic -pthread -mminimal-toc'

    cmake ..  -DCMAKE_INSTALL_PREFIX=../.libs -DWITH_AS3=OFF -DWITH_C_GLIB=OFF -DWITH_JAVA=OFF -DWITH_JAVASCRIPT=OFF -DWITH_NODEJS=OFF -DWITH_PYTHON=OFF -DWITH_HASKELL=OFF -DBUILD_TESTING=OFF -DWITH_QT5=OFF -DOPENSSL_CRYPTO_LIBRARY=${OPENSSL_INSTALL_CRYPTO_PATH} -DOPENSSL_INCLUDE_DIR=${OPENSSL_INCLUDE_PATH} -DOPENSSL_SSL_LIBRARY=${OPENSSL_INSTALL_SSL_PATH} -DBOOST_ROOT=${BOOST_INSTALL_PATH} -DLIBEVENT_ROOT=${LIBEVENT_INSTALL_PATH} -DZLIB_ROOT=${ZLIB_INSTALL_PATH} -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ >> ./build_make.log 2>&1
    if [ $? -ne 0 ]; then
        echo "CMake failed to build thrift."
        exit 1
    fi
    flags_list=`find ./ -name flags.make`

    for item in $flags_list; do
        sed 's/-isystem /-I/g' $item  >$item.bk
        mv $item.bk $item
    done

    make -j4 >> ./build_make.log 2>&1
    if [ $? -ne 0 ]; then
        echo "Failed to compile thrift."
        exit 1
    fi

    make install >> ./build_make.log 2>&1
    if [ $? -ne 0 ]; then
        echo "Failed to install thrift."
        exit 1
    fi
}

make_thrift()
{
    if [ "$1" = "clean" ]; then
        return
    fi

    echo "Begin to compile thrift."
    THRIFT_INSTALL_PATH=${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_THRIFT_DIR}/.libs
    if [ -f "${THRIFT_INSTALL_PATH}/lib/libthrift.a" ]; then
        echo "thrift library exists"
        return
    fi
    cd ${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_THRIFT_DIR}
    if [ "${sys}" == "AIX" ]; then
        make_thrift_aix
        return
    fi

    find ./ -name "*" | xargs dos2unix
    if [ -f "bootstrap.sh" ]; then
        sh bootstrap.sh >> ./build_make.log 2>&1
        main_result=$?
        if [ ${main_result} != 0 ]; then
            echo "#########################################################"
            echo "   thrift bootstrap failed."
            echo "#########################################################"
            exit ${main_result}
        fi
    else
        echo "cannot find bootstrap.sh"
        exit 1
    fi

    OLD_CMD_CC=${CC}
    OLD_CFLAGS="${CFLAGS}"

    OPENSSL_INSTALL_PATH=${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_OPENSSL_DIR}/.openssl
    LIBEVENT_INSTALL_PATH=${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_LIBEVENT_DIR}/.libs
    BOOST_INSTALL_PATH=${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_BOOST_DIR}/.libs
    ZLIB_INSTALL_PATH=${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_ZLIB_DIR}/.libs

    if [ ${SYS_PLATFORM} != "aarch64" ]; then
        export CFLAGS="-w -pipe -m64 -fpic -I${LIBEVENT_INSTALL_PATH}/include"
        export CXXFLAGS="-w -pipe -m64 -fpic -I${LIBEVENT_INSTALL_PATH}/include"
    else
        export CFLAGS="-w -pipe -fpic -I${LIBEVENT_INSTALL_PATH}/include"
        export CXXFLAGS="-w -pipe -fpic -I${LIBEVENT_INSTALL_PATH}/include"   
    fi
    THRIFT_OPT="--disable-shared --disable-tests --prefix=${THRIFT_INSTALL_PATH} --with-cpp --with-boost=${BOOST_INSTALL_PATH} \
        --with-libevent=${LIBEVENT_INSTALL_PATH} --without-python --with-openssl=${OPENSSL_INSTALL_PATH} --without-java \
        --with-zlib=${ZLIB_INSTALL_PATH} --without-go --without-py3 --without-c_glib --without-nodejs"
    export LDFLAGS="-L${LIBEVENT_INSTALL_PATH}/lib"

    if [ -f "configure" ]; then
        ./configure ${THRIFT_OPT} >> ./build_make.log 2>&1
        main_result=$?
        if [ ${main_result} != "0" ]; then
            echo "#########################################################"
            echo "   thrift configure failed."
            echo "#########################################################"
            exit ${main_result}
        fi
    else
        echo "cannot find configure"
        exit 1
    fi

    echo "compiling thrift, log is in build_make.log..."
    make $MAKE_JOB $1 >> ./build_make.log 2>&1
    main_result=$?
    if [ ${main_result} != "0" ]; then
        echo "#########################################################"
        echo "   Make thrift failed."
        echo "#########################################################"
        exit ${main_result}
    fi

    export CC="${OLD_CMD_CC}"
    export CFLAGS="${OLD_CFLAGS}"
    make install
    main_result=$?
    if [ ${main_result} != "0" ]; then
        echo "#########################################################"
        echo "   Make install thrift failed."
        echo "#########################################################"
        exit ${main_result}
    fi

    echo "#########################################################"
    echo "   Make thrift succ."
    echo "#########################################################"
}

generate_thrift_cpp()
{
    if [ "$1" = "clean" ]; then
        return
    fi

    echo "generate thrift code"
    THRIFT_BIN_DIR=${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_THRIFT_DIR}/.libs/bin
    if [ ! -d "${THRIFT_BIN_DIR}" ]; then
        echo "Thrift bin path not exists."
        exit 1
    fi

    if [ ! -f "${THRIFT_BIN_DIR}/thrift" ]; then
        echo "Thrift binary not exists"
        exit 1
    fi
    chmod 777 ${THRIFT_BIN_DIR}/thrift

    cd ${THRIFT_BIN_DIR}
    cp -f ${AGENT_ROOT}/src/src/apps/dws/XBSAServer/xbsa.thrift .
    DEFAULT_DIR=gen-cpp
    if [ -d "${DEFAULT_DIR}" ]; then
        rm -rf ${DEFAULT_DIR}
    fi

    ./thrift -r -gen cpp xbsa.thrift >> ./build_make.log 2>&1
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

    # 将skeleton.cpp改名为skeleton.cpp.bak，不拷贝到源代码目录
    FILENAME=`ls ${DEFAULT_DIR}/*skeleton.cpp`
    NEW_FILE_NAME=$(echo ${DEFAULT_DIR}/$(basename ${FILENAME}).bak)
    mv  ${FILENAME} ${NEW_FILE_NAME}
    # 拷贝到server端
    cp -f ${DEFAULT_DIR}/*.h  ${AGENT_ROOT}/src/inc/apps/dws/XBSAServer
    cp -f ${DEFAULT_DIR}/*.cpp ${AGENT_ROOT}/src/src/apps/dws/XBSAServer
    # 拷贝到公共
    cp -f ${DEFAULT_DIR}/*.h  ${AGENT_ROOT}/src/inc/xbsaclientcomm
    cp -f ${DEFAULT_DIR}/*.cpp ${AGENT_ROOT}/src/src/apps/xbsaclientcomm

    # Agent框架thrift
    rm -rf ${AGENT_ROOT}/src/src/servicecenter/thrift/${DEFAULT_DIR}
    THRIFT_SERVICE_DIR=${AGENT_ROOT}/src/src/servicecenter/thrift
    THRIFT_SERVICE_OUTPUT=${AGENT_ROOT}/src/src/servicecenter/thrift/${DEFAULT_DIR}

    ${THRIFT_BIN_DIR}/thrift -o ${THRIFT_SERVICE_DIR} -r -gen cpp ${THRIFT_SERVICE_DIR}/ApplicationProtectBaseDataType.thrift
    main_result=$?
    if [ ${main_result} -ne 0 ];then
        echo "AppProtectPlugin.thrift gen fail"
        exit ${main_result}
    fi

    ${THRIFT_BIN_DIR}/thrift -o ${THRIFT_SERVICE_DIR} -r -gen cpp ${THRIFT_SERVICE_DIR}/ApplicationProtectFramework.thrift
    main_result=$?
    if [ ${main_result} -ne 0 ];then
        echo "AppProtectService.thrift gen fail"
        exit ${main_result}
    fi

    ${THRIFT_BIN_DIR}/thrift -o ${THRIFT_SERVICE_DIR} -r -gen cpp ${THRIFT_SERVICE_DIR}/ApplicationProtectPlugin.thrift
    main_result=$?
    if [ ${main_result} -ne 0 ];then
        echo "AppProtectComm.thrift gen fail"
        exit ${main_result}
    fi

    # 将skeleton.cpp改名为skeleton.cpp.bak，不拷贝到源代码目录
    FILENAMES=`ls ${THRIFT_SERVICE_OUTPUT}/*skeleton.cpp`
    for FILENAME in $FILENAMES;
    do
        rm -f "${FILENAME}"
    done

    # 拷贝到server端
    cp -f ${THRIFT_SERVICE_OUTPUT}/*.h  ${AGENT_ROOT}/src/inc/apps/appprotect/plugininterface
    cp -f ${THRIFT_SERVICE_OUTPUT}/*.cpp ${AGENT_ROOT}/src/src/apps/appprotect/plugininterface
    echo "#########################################################"
    echo "   generate frame thrift cpp succ."
    echo "#########################################################"

}

make_tcmalloc()
{
    echo "Begin to compile tcmalloc."
    if [ "$sys" = "AIX" ] || [ "$sys" = "SunOS" ]; then
        echo "The current system(AIX) does not support tcmalloc."
        return
    fi

    if [ "$1" = "clean" ]; then
        rm -rf ${AGENT_ROOT}/open_src/gperftools/tcmalloc
        return
    fi

    if [ -f "${AGENT_ROOT}/open_src/gperftools/tcmalloc/lib/libtcmalloc.a" ]; then 
        return
    fi
    OLD_CMD_CC=${CC}
    OLD_CFLAGS="${CFLAGS}"
    if [ ${SYS_PLATFORM} != "aarch64" ]; then
        export CXXFLAGS="-pipe -m64 -fpic"
    else
        CXXFLAGS="-pipe -fpic"
        export CXXFLAGS
    fi
    
    mkdir ${AGENT_ROOT}/open_src/gperftools/tcmalloc
    cd ${AGENT_ROOT}/open_src/gperftools
    ./autogen.sh >> ./build_make.log 2>&1
    ./configure --prefix=${AGENT_ROOT}/open_src/gperftools/tcmalloc --enable-frame-pointers
    make $MAKE_JOB $1 >> ./build_make.log 2>&1
    main_result=$?

    CC="${OLD_CMD_CC}"
    export CC
    CFLAGS="${OLD_CFLAGS}"
    export CFLAGS
    if [ ${main_result} != "0" ]; then
        echo "#########################################################"
        echo "   Compile tcmalloc failed."
        echo "#########################################################"
        exit ${main_result}
    fi
    make install
    main_result=$?
    if [ ${main_result} != "0" ]; then
        echo "#########################################################"
        echo "   make install tcmalloc failed."
        echo "#########################################################"
        exit ${main_result}
    fi

    echo "#########################################################"
    echo "   Compile tcmalloc succ."
    echo "#########################################################"
}

copy_nginx()
{
    cp ${AGENT_ROOT}/conf/backup/nginx.conf ${AGENT_ROOT}/conf
    cp -rf ${AGENT_ROOT}/open_src/nginx ${AGENT_ROOT}/bin    
    rm -rf ${AGENT_ROOT}/bin/nginx/conf/*
    cp ${AGENT_ROOT}/conf/nginx.conf ${AGENT_ROOT}/bin/nginx/conf
    cp ${AGENT_ROOT}/conf/fastcgi_params ${AGENT_ROOT}/bin/nginx/conf
    cp ${AGENT_ROOT}/conf/server.crt ${AGENT_ROOT}/bin/nginx/conf/server.pem
    cp ${AGENT_ROOT}/conf/server.key ${AGENT_ROOT}/bin/nginx/conf/server.key 
    cp ${AGENT_ROOT}/conf/bcmagentca.crt ${AGENT_ROOT}/bin/nginx/conf/pmca.pem
    #rename nginx to rdnginx
    mv ${AGENT_ROOT}/bin/nginx/nginx ${AGENT_ROOT}/bin/nginx/rdnginx
}

make_nginx()
{
    echo "Begin to compile nginx."
    if [ "$1" = "clean" ]; then
        rm -rf "${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_NGINX_DIR}"
        rm -rf "${AGENT_ROOT}/bin/nginx"
        mv ${OPEN_SRC_PACKET_DIR}/nginx_backup ${OPEN_SRC_PACKET_DIR}/nginx # restore nginx file
        return
    fi
    if [ -f "${AGENT_ROOT}/open_src/nginx/nginx" ]; then
        echo "The [nginx] has been compiled successfully and no more compilation."
        return
    fi

    cp ${AGENT_ROOT}/conf/backup/nginx.conf ${AGENT_ROOT}/conf
    if [ -f "${AGENT_ROOT}/open_src/nginx/nginx" ]; then
        echo "nginx have exists"
        return
    fi

    OLD_CFLAGS=${CFLAGS}
    if [ $sys = "AIX" ]; then
        export OBJECT_MODE=32
        export CFLAGS="-Wl,-b64 -maix64"
    elif [ $sys = "HP-UX" ]; then
        NGINX_CFG_OPT="--with-cc-opt=-Agcc"
        #to resolve download log failed when the size of log is over 32K
        #HP os,sed 命令格式 sed '2i\
        ###                         test' xxx.txt
        ROW_NUM_RET=`sed -n "/location/=" "${AGENT_ROOT}/conf/nginx.conf"`
        sed ${ROW_NUM_RET}'i\
        fastcgi_keep_conn on;' ${AGENT_ROOT}/conf/nginx.conf > ${AGENT_ROOT}/conf/nginx.conf.bak
        mv ${AGENT_ROOT}/conf/nginx.conf.bak ${AGENT_ROOT}/conf/nginx.conf
        sed ${ROW_NUM_RET}'i\
        fastcgi_buffers 6 8k;' ${AGENT_ROOT}/conf/nginx.conf > ${AGENT_ROOT}/conf/nginx.conf.bak
        mv ${AGENT_ROOT}/conf/nginx.conf.bak ${AGENT_ROOT}/conf/nginx.conf            
    elif [ "$sys" = "SunOS" ]; then
        NGINX_CFG_OPT=""    
        CFLAGS=""
    fi

    cd ${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_NGINX_DIR}
    if [  ${NGINX_CPU_OPT_FLAG} -eq 1 ]; then
        CFLAGS="${CFLAGS} -Werror"
        export CFLAGS
    fi

    cd ${AGENT_ROOT}/open_src/nginx_tmp
    chmod +x ./auto/configure
    if [ "$sys" = "SunOS" ]; then
        ./auto/configure --prefix=../nginx --sbin-path=../nginx --without-http_rewrite_module --without-http_gzip_module --with-http_ssl_module --with-openssl=../openssl --with-cpu-opt=sparc64 --with-cc-opt="-xmemalign -DWSEC_ERR_CODE_BASE=0 -errwarn=%none" --with-openssl-opt="no-shared no-threads no-asm no-tests" --without-http_upstream_zone_module --without-stream_upstream_zone_module >> ./build_make.log 2>&1
    elif [ $sys = "AIX" ]; then
        ./auto/configure --prefix=../nginx --sbin-path=../nginx --without-http_rewrite_module --without-http_gzip_module --with-http_ssl_module --with-openssl=../openssl --with-openssl-opt="no-shared no-threads no-tests" --without-http_upstream_zone_module --without-stream_upstream_zone_module --with-ld-opt="-maix64 -lpthread" --with-cc-opt="-maix64" >> ./build_make.log 2>&1
    elif [ $sys = "HP-UX" ]; then
        ./auto/configure --prefix=../nginx --sbin-path=../nginx --without-http_rewrite_module --without-http_gzip_module --with-ld-opt="${AGENT_ROOT}/open_src/openssl/.openssl/lib/libssl.a ${AGENT_ROOT}/open_src/openssl/.openssl/lib/libcrypto.a" $NGINX_CFG_OPT >> ./build_make.log 2>&1
    else
        ./auto/configure --prefix=../nginx --sbin-path=../nginx --without-http_rewrite_module --without-http_gzip_module $NGINX_CFG_OPT --with-http_ssl_module --with-cc-opt="-fstack-protector-all" --with-ld-opt="$BIND_NOW_OPT $RELRO_OPT -s -fPIE -pie" --with-openssl=../openssl --with-openssl-opt="no-shared no-tests" --without-http_upstream_zone_module --without-stream_upstream_zone_module >> ./build_make.log 3>&1
    fi

    # 删除编译openssl的过程，减少编译时间，同时HP下设置Agcc后，openssl会编译失败
    bNum=`grep -n "../openssl/.openssl/include/openssl/ssl.h:" ./objs/Makefile | awk -F: '{print $1}'`
    eNum=`grep -n " install_sw " ./objs/Makefile | awk -F: '{print $1}'`
    echo "bNum=$bNum,eNum=$eNum"

    bNum=`expr $bNum + 2`
    index=$bNum
    while [ ${bNum} -le ${eNum} ]; do
        if [ -z "${bNum}" ] || [ -z "$eNum" ]; then
            break
        fi
        sed "${index}d" ./objs/Makefile > ./objs/Makefile.bk
        mv ./objs/Makefile.bk ./objs/Makefile
        bNum=`expr $bNum + 1`
    done
    if [ $sys = "HP-UX" ]; then
        export LPATH=/usr/lib/hpux64/
    fi
    
    echo "compiling nginx, log is in build_make.log..."
    make ${MAKE_JOB} $1 >> ./build_make.log 2>&1
    main_result=$?
    if [ ${main_result} != "0" ]; then
        echo "#########################################################"
        echo "   Make nginx failed."
        echo "#########################################################"
        exit ${main_result}
    fi

    make install
    main_result=$?
    if [ ${main_result} != "0" ]; then
        echo "#########################################################"
        echo "   Make install nginx failed."
        echo "#########################################################"
        exit ${main_result}
    fi

    cp -rf ${AGENT_ROOT}/open_src/nginx ${AGENT_ROOT}/bin    
    rm -rf ${AGENT_ROOT}/bin/nginx/conf/*
    cp ${AGENT_ROOT}/conf/nginx.conf ${AGENT_ROOT}/bin/nginx/conf
    cp ${AGENT_ROOT}/conf/fastcgi_params ${AGENT_ROOT}/bin/nginx/conf
    cp ${AGENT_ROOT}/conf/server.crt ${AGENT_ROOT}/bin/nginx/conf/server.pem
    cp ${AGENT_ROOT}/conf/server.key ${AGENT_ROOT}/bin/nginx/conf/server.key 
    cp ${AGENT_ROOT}/conf/bcmagentca.crt ${AGENT_ROOT}/bin/nginx/conf/pmca.pem
    #rename nginx to rdnginx
    mv ${AGENT_ROOT}/bin/nginx/nginx ${AGENT_ROOT}/bin/nginx/rdnginx
    
    CFLAGS=${OLD_CFLAGS}
    export CFLAGS
    echo "#########################################################"
    echo "   Make nginx succ."
    echo "#########################################################"
}

make_snmp()
{
    echo "Begin to compile snmp."
    if [ "$1" = "clean" ]; then
        rm -rf "${OPEN_SRC_PACKET_DIR}/snmp++/src/.libs"
        return
    fi
    
    if [ -f "${OPEN_SRC_PACKET_DIR}/snmp++/src/.libs/libsnmp++.a" ]; then 
        echo "The [snmp] has been compiled successfully and no more compilation."
        return
    fi

    OLD_CMD_CC=${CC}
    OLD_CFLAGS="${CFLAGS}"
    cd ${AGENT_ROOT}/open_src/snmp++
    if [ $sys = "AIX" ]; then
        export CXXFLAGS="-Wl,-b64 -maix64 -D__STDC_FORMAT_MACROS -DSTDCXX_98_HEADERS"
        export OBJECT_MODE=64
        export CFLAGS="-maix64 -D__STDC_FORMAT_MACROS -DSTDCXX_98_HEADERS -DAIX $KMC_OPT"
    elif [ $sys = "HP-UX" ]; then
        export CXXFLAGS="+DD64 -w -AA -D_REENTRANT -DHP_UX_IA -DHP_UX -mt -b +z"
    elif [ "$sys" = "SunOS" ]; then
        CXXFLAGS="-m64 -mt -xcode=pic32 -G -PIC"
        export CXXFLAGS
        CXX="CC"
        export CXX
        CC="cc"
        export CC
    else
        if [ ${SYS_PLATFORM} != "aarch64" ]; then
            export CXXFLAGS="-pipe -m64 -fpic"
        else
            export CXXFLAGS="-pipe -fpic"
        fi        
    fi
    
    ssl_CFLAGS="-I${AGENT_ROOT}/open_src/openssl/include"
    export ssl_CFLAGS
    ssl_LIBS="-lssl -lcrypto"
    export ssl_LIBS
    LDFLAGS="-L${AGENT_ROOT}/open_src/openssl"
    export LDFLAGS
    chmod +x configure
    if [ ${SYS_PLATFORM} != "aarch64" ]; then
        ./configure --with-ssl --disable-logging --disable-namespace --enable-shared=no
    else
        ./configure --with-ssl --disable-logging --disable-namespace --enable-shared=no --host=arm-linux --target=arm-linux --build=i686-linux
    fi 
    
    if [ $sys = "HP-UX" ]; then
        #patch snmp 3.3.9
        # In HP-UX, the function clock_gettime not support parameter CLOCK_MONOTONIC, 
        # so undefine HAVE_CLOCK_GETTIME to go to old logical branch.
        # check it by `man clock_gettime`
        echo "#undef HAVE_CLOCK_GETTIME" >> config.h
    fi
    
    # start to make
    echo "compiling snmp, log is in build_make.log..."
    make ${MAKE_JOB} >> ./build_make.log 2>&1
    
    main_result=$?
    CC="${OLD_CMD_CC}"
    export CC
    CFLAGS="${OLD_CFLAGS}"
    export CFLAGS
    if [ ${main_result} != "0" ]; then
        echo "#########################################################"
        echo "   Compile snmp failed."
        echo "#########################################################"
        
        exit ${main_result}
    fi
    
    if [ ! -f "${AGENT_ROOT}/open_src/snmp++/src/.libs/libsnmp++.a" ]; then
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
    echo "Begin to compile fcgi."
    if [ "$1" = "clean" ]; then
        return
    fi

    cd ${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_FCGI_DIR}
    if [ -f "libfcgi/.libs/libfcgi.a" ]; then 
        echo "The [fcgi] has been compiled successfully and no more compilation."
        return
    fi

    OLD_CFLAGS="${CFLAGS}"
    echo "cd ${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_FCGI_DIR}" >> ./build_make.log
    if [ "$sys" = "SunOS" ]; then
        CXXFLAGS="-m64 -mt -xcode=pic32 -G -PIC"
        export CXXFLAGS
        CXX="CC"
        export CXX
        CC="cc"
        export CC
        FCGI_CP_OPT="CFLAGS=-pg -xcode=pic32 -m64"
    fi
    if [ $sys = "AIX" ]; then 
        export OBJECT_MODE=64
        export CXXFLAGS="-maix64"
    fi

    chmod +x ./autogen.sh
    ./autogen.sh >> ./build_make.log 2>&1
    chmod +x ./configure
    if [ $sys = "HP-UX" ]; then
        export oFLAGS="-G +DD64 -ldl"
        export OFLAGS="-G +DD64 -ldl -ldcekt"
        ./configure >> ./build_make.log 2>&1
    else
        ./configure --disable-shared "$FCGI_CP_OPT" >> ./build_make.log 2>&1
    fi     
    ###AIX HP SunOS 编译是需要修改configure and libfcgi 目录下的Makefile文件
    if [ $sys = "HP-UX" ]; then 
        #patch fcgi 2.4.2
        ##modify configure 
        sed "s/AM_CPPFLAGS = -I\$(top_srcdir)\/include -W -Wall -pedantic/AM_CPPFLAGS = -I\$(top_srcdir)\/include -pedantic/g" libfcgi/Makefile >libfcgi/Makefile.bak
        rm libfcgi/Makefile
        mv libfcgi/Makefile.bak libfcgi/Makefile
        ###change Makefile.am
        sed "s/AM_CPPFLAGS = -I\$(top_srcdir)\/include -W -Wall -pedantic/AM_CPPFLAGS = -I\$(top_srcdir)\/include -pedantic/g" libfcgi/Makefile.am >libfcgi/Makefile.am.bak
        rm libfcgi/Makefile.am
        mv libfcgi/Makefile.am.bak libfcgi/Makefile.am
        ####change Makefile.in
        sed "s/AM_CPPFLAGS = -I\$(top_srcdir)\/include -W -Wall -pedantic/AM_CPPFLAGS = -I\$(top_srcdir)\/include -pedantic/g" libfcgi/Makefile.in >libfcgi/Makefile.in.bak
        rm libfcgi/Makefile.in
        mv libfcgi/Makefile.in.bak libfcgi/Makefile.in 
    elif [ "$sys" = "SunOS" ]; then
        #patch fcgi 2.4.2
        ##modify libfcgi/Makefile
        SED_CMD="s/AM_CPPFLAGS = -I\$(top_srcdir)\/include -W -Wall -pedantic/AM_CPPFLAGS = -I\$(top_srcdir)\/include -pedantic/g"
        echo $SED_CMD > ${AGENT_ROOT}/open_src/fcgi/libfcgi/cmd
        sed -f ${AGENT_ROOT}/open_src/fcgi/libfcgi/cmd ${AGENT_ROOT}/open_src/fcgi/libfcgi/Makefile > ${AGENT_ROOT}/open_src/fcgi/libfcgi/Makefile.bak
        rm -rf ${AGENT_ROOT}/open_src/fcgi/libfcgi/Makefile
        mv ${AGENT_ROOT}/open_src/fcgi/libfcgi/Makefile.bak ${AGENT_ROOT}/open_src/fcgi/libfcgi/Makefile
        ###modify Makefile.am
        sed -f ${AGENT_ROOT}/open_src/fcgi/libfcgi/cmd ${AGENT_ROOT}/open_src/fcgi/libfcgi/Makefile.am > ${AGENT_ROOT}/open_src/fcgi/libfcgi/Makefile.am.bak
        rm -rf ${AGENT_ROOT}/open_src/fcgi/libfcgi/Makefile.am
        mv ${AGENT_ROOT}/open_src/fcgi/libfcgi/Makefile.am.bak ${AGENT_ROOT}/open_src/fcgi/libfcgi/Makefile.am
        ###modify Makefile.in
        sed -f ${AGENT_ROOT}/open_src/fcgi/libfcgi/cmd ${AGENT_ROOT}/open_src/fcgi/libfcgi/Makefile.in > ${AGENT_ROOT}/open_src/fcgi/libfcgi/Makefile.in.bak
        rm -rf ${AGENT_ROOT}/open_src/fcgi/libfcgi/Makefile.in
        mv ${AGENT_ROOT}/open_src/fcgi/libfcgi/Makefile.in.bak ${AGENT_ROOT}/open_src/fcgi/libfcgi/Makefile.in
        rm -rf ${AGENT_ROOT}/open_src/fcgi/libfcgi/cmd
    fi

    if [ $sys = "Linux" ]; then
        sed '34i #include <cstdio>' -i ${AGENT_ROOT}/open_src/fcgi/include/fcgio.h
    fi

    echo "compiling fcig, log is in build_make.log..."
    make $MAKE_JOB $1 >> ./build_make.log 2>&1
    main_result=$?
    if [ ${main_result} != "0" ]; then
        echo "#########################################################"
        echo "   Make fcgi failed."
        echo "#########################################################"
        exit ${main_result}
    fi

    CFLAGS="${OLD_CFLAGS}"
    export CFLAGS
    if [ $sys = "HP-UX" ]; then
        # HP-UX下需要这个动态库
        cp ${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_FCGI_DIR}/libfcgi/.libs/libfcgi.so.0.0 ${AGENT_ROOT}/bin/libfcgi.so.0
    fi

    cp ${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_FCGI_DIR}/fcgi_config.h ${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_FCGI_DIR}/include/fcgi_config.h
    
    echo "#########################################################"
    echo "   Make fcgi succ."
    echo "#########################################################"
}

make_curl()
{
    echo "Begin to compile curl."
    if [ "$1" = "clean" ]; then
        rm -rf "${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_CURL_DIR}/lib/.libs/"
        return 
    fi

    OLD_CFLAGS="${CFLAGS}"
    cd ${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_CURL_DIR}
    if [ -f "lib/.libs/libcurl.a" ]; then
        echo "The [curl] has been compiled successfully and no more compilation."
        return
    fi

    if [ $sys = "AIX" ]; then
        export OBJECT_MODE=64
        export CXXFLAGS="-maix64"
    fi
    cd ${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_CURL_DIR}
    aclocal -I m4
    autoreconf -ivf
    automake -a -c
    env PKG_CONFIG_PATH=${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_OPENSSL_DIR}/.openssl/lib/pkgconfig
    ./configure --prefix=${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_CURL_DIR} --with-ssl=${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_OPENSSL_DIR}/.openssl --disable-ldap --disable-shared --without-zlib
    
    make ${MAKE_JOB}
    main_result=$?
    if [ ${main_result} != "0" ]; then
        echo "#########################################################"
        echo "   Make curl failed."
        echo "#########################################################"

        if [ ! -f "${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_CURL_DIR}/lib/.libs/libcurl.a" ]; then
            exit ${main_result}
        fi
    fi

    CFLAGS="${OLD_CFLAGS}"
    export CFLAGS
    echo "#########################################################"
    echo "   Make curl succ."
    echo "#########################################################"
}


make_opensrc()
{
    make_openssl $1
    make_kmc $1
    make_uuid $1
    prepare_thrift $1
    make_thrift $1
    generate_thrift_cpp $1
    make_tcmalloc $1
    make_nginx $1
    copy_nginx
    make_snmp $1
    make_fcgi $1
    # json_cpp patch in AIX\HP_UX\Solaris
    # create by diff -uNr src/lib_json/json_writer.cpp fix/lib_json/json_writer.cpp in linux
    version=`cat ${AGENT_ROOT}/open_src/jsoncpp/include/json/version.h |grep JSONCPP_VERSION_STRING |awk -F ' ' '{print $NF}' |sed 's/\"//g' |tr -cd "[:print:]\n"`
    if [ "${version}" = "00.11.0" ]; then
        if [ -f "${AGENT_ROOT}/open_src/patch/json_cpp_reader_h_parse.patch" ]; then
            #patch jsoncpp 0.11.0
            cd ${AGENT_ROOT}/open_src/patch
            if [ $sys = "Linux" ]; then
                patch -N ${AGENT_ROOT}/open_src/jsoncpp/include/json/reader.h < json_cpp_reader_h_parse.patch
            fi
        fi
        
        if [ -f "${AGENT_ROOT}/open_src/patch/json_cpp_reader_cpp_parse.patch" ]; then
            #patch jsoncpp 0.11.0
            cd ${AGENT_ROOT}/open_src/patch
            if [ $sys = "Linux" ]; then
                patch -N ${AGENT_ROOT}/open_src/jsoncpp/src/lib_json/json_reader.cpp < json_cpp_reader_cpp_parse.patch
            fi
        fi
    fi
    make_curl $1
}


CompareSysVersion()
{
    SYS_VER=`uname -r | $AWK -F '-' '{print $1}' | $AWK -F '.' '{print $1$2$3}'`
    
    if [ ${SYS_VER} -lt ${FREEZE_VERSION} ]; then
        FREEZE_SUPPORT=0
    else
        FREEZE_SUPPORT=1
    fi
}

init_env()
{
    mkdir -p ${AGENT_ROOT}/obj

    if [ $sys = "AIX" ]; then
        CPPC=g++
        CPP=g++
        CC=gcc
        cc=gcc
        OS_VERSION=`oslevel | awk -F "." '{print $1$2}'`
        CFLAGS="-maix64 -DSTDCXX_98_HEADERS -DAIX $KMC_OPT"
        cFLAGS=${CFLAGS}
        OFLAGS="-maix64"
        oFLAGS=${OFLAGS}
        DFLAGS="-maix64"
        dFLAGS=${DFLAGS}
        ARNew="ar -X64 -v -r"
        STFLAGS=""
        DYFLAGS=""
        STLIBS=""
        export OBJECT_MODE=64
    elif [ $sys = "HP-UX" ]; then
        CPPC=aCC
        CC=aCC
        #for sqlite and securec
        cc=cc
        CFLAGS="-w -AA +DD64 -D_REENTRANT -DHP_UX_IA -DHP_UX -mt $KMC_OPT"
        cFLAGS="-w +DD64 -D_REENTRANT -DHP_UX_IA -DHP_UX -mt"
        CXXFLAGS="+DD64 -w -AA -D_REENTRANT -DHP_UX_IA -DHP_UX -mt -b +z"
        oFLAGS="+DD64 -ldl"
        OFLAGS="+DD64 -ldl -ldcekt"
        dFLAGS="+DD64 -b"
        DFLAGS="+DD64 -b -ldcekt"
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
            OS_NAME=REDHAT
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
            CFLAGS="-pipe -fpic -DLINUX -DLIN_FRE_SUPP -D${OS_NAME} -DSTDCXX_98_HEADERS $KMC_OPT ${EXTERNAL_DEBUG_FLAG} -Wno-deprecated-declarations -fstack-protector-all"
        else
            CFLAGS="-pipe -fpic -DLINUX -D${OS_NAME} -DSTDCXX_98_HEADERS $KMC_OPT ${EXTERNAL_DEBUG_FLAG} -Wno-deprecated-declarations -fstack-protector-all"
        fi

        if [ $REST_PUBLISH -eq 1 ]; then
            CFLAGS+=" -DREST_PUBLISH"
        fi

        cFLAGS=${CFLAGS}
        OFLAGS="-rdynamic -s $NX_OPT $SP_OPT $RELRO_OPT $RPATH_OPT $PIE_OPT $BIND_NOW_OPT"
        oFLAGS="-rdynamic -s $NX_OPT $SP_OPT $RELRO_OPT $RPATH_OPT $PIE_OPT $BIND_NOW_OPT"
        DFLAGS="-rdynamic -shared -fPIC -s $NX_OPT $SP_OPT $RELRO_OPT $RPATH_OPT $BIND_NOW_OPT" 
        dFLAGS="-rdynamic -shared -fPIC -s $NX_OPT $SP_OPT $RELRO_OPT $RPATH_OPT $BIND_NOW_OPT"
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
    elif [ "$sys" = "SunOS" ]; then
        CPPC="g++"
        CC="gcc"
        cc="gcc"
        #for sqlite and securec
        cc=cc
        CFLAGS="-pipe -fpic -DSOLARIS -DSTDCXX_98_HEADERS $KMC_OPT ${EXTERNAL_DEBUG_FLAG} -nostdlib"
        CXXFLAGS="" 
        cFLAGS=$CFLAGS
        OFLAGS=" -m64 -lsocket"
        oFLAGS=${OFLAGS}
        DFLAGS=" -m64 -shared -lsocket"
        dFLAGS=${DFLAGS}
        ARNew="ar rc"
        STFLAGS=""
        DYFLAGS=""
        STLIBS="-luuid"
    else
        echo "Unsupported OS"
        exit 0
    fi

    export CPPC CC cc CFLAGS cFLAGS CXXFLAGS  OFLAGS oFLAGS DFLAGS dFLAGS AGENT_BUILD_NUM ARNew STFLAGS DYFLAGS STLIBS
    export COV_CFLAGS COV_OFLAGS
}

check_open_src_packages()
{
    echo "Checking the open source package. Please wait..."
    if [ ! -d "${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_JSONCPP_DIR}" ]; then
        if [ ! -d "${OPEN_SRC_PACKET_DIR}/jsoncpp_00.11.0" ]; then
            echo "OpenSrc package[jsoncpp_00.11.0] is not exist."
            exit 1
        fi
        cp -rp ${OPEN_SRC_PACKET_DIR}/jsoncpp_00.11.0 ${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_JSONCPP_DIR}
    fi
    
    if [ ! -d "${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_FCGI_DIR}" ]; then
        if [ ! -d "${OPEN_SRC_PACKET_DIR}/fcgi2" ]; then
            echo "OpenSrc package[${OPEN_SRC_PACKET_DIR}/fcgi2] is not exist."
            exit 1
        fi
        cp -rp ${OPEN_SRC_PACKET_DIR}/fcgi2 ${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_FCGI_DIR}
    fi

    if [ ! -d "${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_NGINX_DIR}" ]; then
        echo "OpenSrc package[nginx] is not exist."
        exit 1
    fi

    if [ ! -d "${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_SQLITE_DIR}" ]; then
        echo "OpenSrc package[${OPEN_SRC_SQLITE_DIR}] is not exist."
        exit 1
    fi

    if [ ! -d "${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_SNMP_DIR}" ]; then
        if [ ! -d "${OPEN_SRC_PACKET_DIR}/SNMP" ]; then
            echo "OpenSrc package[SNMP] is not exist."
            exit 1
        fi
        cp -rp ${OPEN_SRC_PACKET_DIR}/SNMP ${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_SNMP_DIR}
    fi

    if [ ! -d "${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_TINYXML_DIR}" ]; then
        if [ ! -d "${OPEN_SRC_PACKET_DIR}/tinyxml2" ]; then
            echo "OpenSrc package[tinyxml2] is not exist."
            exit 1
        fi
        cp -rp ${OPEN_SRC_PACKET_DIR}/tinyxml2 ${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_TINYXML_DIR}
    fi

    if [ ! -d "${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_CURL_DIR}" ]; then
        echo "OpenSrc package[${OPEN_SRC_CURL_DIR}] is not exist."
        exit 1
    fi

    if [ "$sys" = "Linux" ]; then
        if [ ! -d "${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_UTIL_LINUX_DIR}" ]; then
            echo "OpenSrc package[${OPEN_SRC_UTIL_LINUX_DIR}] is not exist."
            exit 1
        fi

        if [ ! -d "${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_GPERFTOOLS}" ]; then
            echo "OpenSrc package[${OPEN_SRC_GPERFTOOLS}] is not exist."
            exit 1
        fi
    fi

    if [ ! -d "${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_BOOST_DIR}" ]; then
        echo "OpenSrc package[${OPEN_SRC_BOOST_DIR}] is not exist."
        exit 1
    fi

    if [ ! -d "${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_THRIFT_DIR}" ]; then
        echo "Open src package[${OPEN_SRC_THRIFT_DIR}] is not exist."
        exit 1
    fi

    if [ ! -d "${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_LIBEVENT_DIR}" ]; then
        echo "Open src package[${OPEN_SRC_LIBEVENT_DIR}] is not exist."
        exit 1
    fi

    if [ ! -d "${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_ZLIB_DIR}" ]; then
        echo "Open src package[${OPEN_SRC_ZLIB_DIR}] is not exist."
        exit 1
    fi

    chmod -R 755 ${OPEN_SRC_PACKET_DIR}
}

PrintHelp()
{
    CurFile=`basename $0`
    echo "Usage: $CurFile [option]"
    echo "  preEnv          Check whether the {AGENT_ROOT}/oper_src packages are complete."
    echo "  afterEnv        Compile all open source packages."
    echo "  no_opensrc      Generate thrift code usign thrift file."
    echo "  [none]          Execute preEnv,afterEnv one by one."
    echo ""
    echo "For bug reporting instructions, please call 120, thanks."
}

init_env

if [ $# != 0 ]; then
    if [ "$1" = "preEnv" ]; then
        check_open_src_packages
        make_sqlite3_file
    elif [ "$1" = "afterEnv" ]; then
        make_opensrc $2
    elif [ "$1" = "no_opensrc" ]; then
        make_kmc $2
        generate_thrift_cpp $2
    elif [ "$1" = "help" ]; then
        PrintHelp
    else
        echo "Unsupported option"
        exit 0
    fi
else
    check_open_src_packages
    make_sqlite3_file
    make_opensrc
fi

exit 0
