#!/bin/bash

SCRIPT_PATH=$(dirname ${BASH_SOURCE[0]})
SCRIPT_NAME="${BASH_SOURCE[0]##*/}"
OPEN_SRC_PACKET_DIR=$(cd ${SCRIPT_PATH}/..;pwd)
OPEN_SRC_MODULE=thrift
OPEN_SRC_OPENSSL_MODULE=openssl
OPENSSL_INSTALL_PATH=${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_OPENSSL_MODULE}_rel
OPEN_SRC_BOOST_MODULE=boost
BOOST_INSTALL_PATH=${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_BOOST_MODULE}_rel

OPEN_SRC_THRIFT_MODULE=thrift
THRIFT_INSTALL_PATH=${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_THRIFT_MODULE}_rel

OPEN_SRC_LIBEVENT_MODULE=libevent
LIBEVENT_INSTALL_PATH=${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_LIBEVENT_MODULE}_rel

OPEN_SRC_ZLIB_MODULE=zlib
ZLIB_INSTALL_PATH=${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_ZLIB_MODULE}_rel

function make_thrift()
{
    echo "INFO: Begin to compile thrift."
    if [ -f "${THRIFT_INSTALL_PATH}/lib/libthrift.a" ]; then
        echo "INFO: thrift library exists"
        return 0
    fi
    cd ${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_MODULE}/${OPEN_SRC_THRIFT_MODULE}
    sed -i 's/(c >= 0) && //' ${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_MODULE}/${OPEN_SRC_THRIFT_MODULE}/compiler/cpp/src/thrift/generate/t_delphi_generator.cc
    find ./ -name "*" | xargs dos2unix 2>/dev/null
    if [ ! -f "bootstrap.sh" ]; then
        echo "ERROR: cannot find bootstrap.sh"
        exit 1
    fi
    sh bootstrap.sh >> ./build_make.log 2>&1
    main_result=$?
    if [ ${main_result} != 0 ]; then
        echo "#########################################################"
        echo "ERROR:    thrift bootstrap failed."
        echo "#########################################################"
        exit ${main_result}
    fi

    THRIFT_OPT="--disable-shared --disable-tests --prefix=${THRIFT_INSTALL_PATH} --with-cpp --with-boost=${BOOST_INSTALL_PATH} \
        --with-libevent=${LIBEVENT_INSTALL_PATH} --without-python --with-openssl=${OPENSSL_INSTALL_PATH} --without-java \
        --with-zlib=${ZLIB_INSTALL_PATH} --without-go --without-py3 --without-c_glib --without-nodejs --without-php"

    local sysPlatform=$(arch)
    if [ "X${sysPlatform}" != "Xaarch64" ]; then
        export CFLAGS="-g -w -pipe -fpic -fstack-protector-strong -m64  -I${LIBEVENT_INSTALL_PATH}/include"
        export CXXFLAGS="-g -w -pipe -fpic -fstack-protector-strong -m64  -I${LIBEVENT_INSTALL_PATH}/include"
    else
        export CFLAGS="-g -w -pipe -fpic -fstack-protector-strong -I${LIBEVENT_INSTALL_PATH}/include"
        export CXXFLAGS="-g -w -pipe -fpic -fstack-protector-strong -I${LIBEVENT_INSTALL_PATH}/include"
    fi

    cd ${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_MODULE}/${OPEN_SRC_THRIFT_MODULE}
    if [ ! -f "configure" ]; then
        echo "ERROR: cannot find configure"
        exit 1
    fi
    chmod u+x ./configure
    ./configure ${THRIFT_OPT} CFLAGS="${CFLAGS}" CXXFLAGS="${CXXFLAGS}" >> ./build_make.log 2>&1
    main_result=$?
    if [ ${main_result} != "0" ]; then
        echo "ERROR:    thrift configure failed."
        exit ${main_result}
    fi

    echo "compiling thrift, log is in build_make.log..."
    make -j4 $1 >> ./build_make.log 2>&1
    main_result=$?
    if [ ${main_result} != "0" ]; then
        echo "ERROR:    Make thrift failed."
        exit ${main_result}
    fi

    make install
    main_result=$?
    if [ ${main_result} -ne "0" ]; then
        echo "ERROR:    Make install thrift failed."
        exit ${main_result}
    fi

    echo "INFO:  Make thrift succ."
}

make_thrift


