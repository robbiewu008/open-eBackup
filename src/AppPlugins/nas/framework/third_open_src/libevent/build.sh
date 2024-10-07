#!/bin/bash

SCRIPT_PATH=$(dirname ${BASH_SOURCE[0]})
SCRIPT_NAME="${BASH_SOURCE[0]##*/}"
OPEN_SRC_PACKET_DIR=$(cd ${SCRIPT_PATH}/..;pwd)
OPEN_SRC_MODULE=libevent
OPEN_SRC_LIBEVENT_DIR=libevent
LIBEVENT_INSTALL_PATH=${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_MODULE}_rel

function make_libevent()
{
    rm -rf "$LIBEVENT_INSTALL_PATH"

    # install libevent
    if [ -n "${LIBEVENT_VAR}" ]; then
        echo "INFO: libevent library exists"
        return 0
    fi

    echo "INFO: Begin to install libevent."
    cd ${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_MODULE}/${OPEN_SRC_LIBEVENT_DIR}
    if [ ! -f "autogen.sh" ]; then
        echo "ERROR:  autogen.sh is not exist"
        return 1
    fi
    sh autogen.sh

    if [ ! -f "configure" ]; then
        echo "ERROR:  configure is not exist"
        return 1
    fi
    export CFLAGS="-fPIC -fstack-protector-strong -Wl,-z,relro,-z,now,-z,noexecstack"
    export CXXFLAGS="-fPIC -fstack-protector-strong -Wl,-z,relro,-z,now,-z,noexecstack"
    ./configure --prefix=${LIBEVENT_INSTALL_PATH}   \
                --disable-openssl                   \
                CFLAGS="${CFLAGS}"                  \
                CXXFLAGS="${CXXFLAGS}"              >> ./build_make.log 2>&1

    echo "INFO: compiling libevent, log is in build_make.log..."
    make $MAKE_JOB $1 >> ./build_make.log 2>&1
    local main_result=$?
    if [ ${main_result} != "0" ]; then
        echo "ERROR: Make libevent failed."
        exit ${main_result}
    fi

    make install
    main_result=$?
    if [ ${main_result} != "0" ]; then
        echo "ERROR: Make install libevent failed."
        exit ${main_result}
    fi

    echo "   Make libevent succ."
}

make_libevent


