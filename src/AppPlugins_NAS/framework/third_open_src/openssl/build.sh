#!/bin/bash

SCRIPT_PATH=$(dirname ${BASH_SOURCE[0]})
SCRIPT_NAME="${BASH_SOURCE[0]##*/}"
OPEN_SRC_PACKET_DIR=$(cd ${SCRIPT_PATH}/..;pwd)
OPEN_SRC_MODULE=openssl
OPEN_SRC_OPENSSL_DIR=openssl
OPENSSL_INSTALL_PATH=${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_MODULE}_rel

function make_openssl()
{
    #the openssl 1.0.2 version need compile with dynamic file for solaris, so modify option "no-shared" to "shared"
    if [ "X$sys" != "XSunOS" ]; then
        OPENSSL_OPT="--prefix=$OPENSSL_INSTALL_PATH no-shared no-tests"
    else
        OPENSSL_OPT="--prefix=$OPENSSL_INSTALL_PATH no-shared no-tests no-asm"
    fi

    LIBCRYPTO_NAME="libcrypto.a"
    LIBSSL_NAME="libssl.a"
    #gcc secure opt
    RELRO_OPT="-Wl,-z,relro"
    BIND_NOW_OPT="-Wl,-z,now"
    if [ -f "${OPENSSL_INSTALL_PATH}/lib/libssl.a" ] && [ -f "${OPENSSL_INSTALL_PATH}/lib/libcrypto.a" ]; then
        echo "INFO: openssl libs already exist"
        return 0
    fi

    export CFLAGS="-fstack-protector-strong"
    export CXXFLAGS="-fstack-protector-strong"
    cd ${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_MODULE}/${OPEN_SRC_OPENSSL_DIR}
    chmod u+x config
    chmod u+x Configure
    ./config -fPIC $RELRO_OPT $OPENSSL_OPT $BIND_NOW_OPT -fstack-protector-strong  -Wl,-z,relro,-z,now,-z,noexecstack -pie >> ./build_make.log 2>&1
    echo "compiling openssl, log is in build_make.log..."
    make $MAKE_JOB $1 >> ./build_make.log 2>&1
    local main_result=$?
    if [ ${main_result} -ne 0 ]; then
        echo "ERROR:   Compile openssl failed."
        exit ${main_result}
    fi
    make install_sw
    make install_ssldirs
    main_result=$?
    if [ ! -f "$OPENSSL_INSTALL_PATH/lib/${LIBCRYPTO_NAME}" ] \
        || [ ! -f "$OPENSSL_INSTALL_PATH/lib/${LIBSSL_NAME}" ] \
        || [ ! -f "$OPENSSL_INSTALL_PATH/bin/openssl" ]; then
        echo "ERROR:   Compile openssl failed."
        exit 1
    fi

    if [ ${main_result} != 0 ]; then
        echo "ERROR:   make install openssl failed."
        exit ${main_result}
    fi

#    cp ${OPENSSL_INSTALL_PATH}/bin/openssl "${NAS_ROOT_DIR}/bin"
#    cp ${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_OPENSSL_DIR}/apps/openssl.cnf "${NAS_ROOT_DIR}/conf"
    echo "   Compile openssl succussfully."
}

make_openssl


