#!/bin/bash

SCRIPT_PATH=$(dirname ${BASH_SOURCE[0]})
SCRIPT_NAME="${BASH_SOURCE[0]##*/}"
OPEN_SRC_PACKET_DIR=$(cd ${SCRIPT_PATH}/..;pwd)
OPEN_SRC_MODULE=boost
OPEN_SRC_BOOST_DIR=boost
BOOST_INSTALL_PATH=${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_MODULE}_rel

# 安装thrift之前，检查是否boost库是否已经安装
function make_boost()
{
    BOOST_VAR=$(ls ${BOOST_INSTALL_PATH}/lib/*${OPEN_SRC_BOOST_DIR}*.so 2>/dev/null)
    if [ -n "${BOOST_VAR}" ]; then
        echo "INFO: ${OPEN_SRC_BOOST_DIR} library alreay exists"
        return 0
    fi

    # install boost
    echo "INFO: Begin to install boost."
    cd ${OPEN_SRC_PACKET_DIR}/${OPEN_SRC_MODULE}/${OPEN_SRC_BOOST_DIR}
    if [ ! -f "bootstrap.sh" ]; then
        echo "ERROR: Failed to find booststrap.sh"
        return 1
    fi
    find -name *.sh | xargs chmod u+x  2>/dev/null
    sh bootstrap.sh --with-libraries=chrono,date_time,filesystem,log,regex,serialization,system,thread --with-toolset=gcc
    if [ $? -ne 0 ]; then
        echo "ERROR: bootstrap.sh execute fail"
        return 1
    fi

    if [ ! -f "b2" ]; then
        echo "ERROR: Failed to find binary b2"
        return 1
    fi
    ./b2 toolset=gcc cxxflags="-fstack-protector-strong" cflags="-fstack-protector-strong" >> ./build_make.log 2>&1

    ./b2 install --prefix=${BOOST_INSTALL_PATH}
    if [ $? -ne 0 ]; then
        echo "ERROR: install b2 fail for boost"
        return 1
    fi
    echo "INFO: install boost succ."
}

make_boost


