#!/bin/sh
#
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
#
export CURRENT_DIR="$(cd "$(dirname "$BASH_SOURCE")";pwd)"
source "$CURRENT_DIR/env.sh"

export MS_BASE_DIR=${EBACKUP_FRAMEWORK_DIR}

compile_proto()
{
    echo "#########################################################"
    echo "   Start to compile protocol "
    echo "#########################################################"
    local LD_TMP=${LD_LIBRARY_PATH}
    export LD_LIBRARY_PATH=${LD_TMP}:${THIRD_PARTY_DIR}/protobuf_rel/lib
    PROTOC=${THIRD_PARTY_DIR}/protobuf_rel/bin/protoc
    rm -rf ${MS_BASE_DIR}/var/protocol
    mkdir -p ${MS_BASE_DIR}/var/protocol 
    cd ${MS_BASE_DIR}/src/protocol
    for PROTO_FILE in `ls .`
    do
        CURR_PROTO_NAME=${PROTO_FILE%.*}
        CURR_PROTO_SUFFIX=${PROTO_FILE#*.}

        if [ "${CURR_PROTO_SUFFIX}" != "proto" ]
        then
            continue
        fi
        CURR_PROTO_HEAD_FILE=${CURR_PROTO_NAME}.pb.h
        if [ ! -f ${MS_BASE_DIR}/var/protocol/${CURR_PROTO_HEAD_FILE} ]
        then
            ${PROTOC} ./${PROTO_FILE} --cpp_out=${MS_BASE_DIR}/var/protocol 
            if [ $? -ne 0 ];then
                echo "Make protocol ${PROTO_FILE} failed"
                exit 1
            fi
        fi
    done
    sed --in-place 's/PROTOBUF_FINAL //' ${MS_BASE_DIR}/var/protocol/*
    if [ $? -ne 0 ];then
        echo "sed PROTOBUF_FINAL failed"
        exit 1
    fi
    sed -i 's/ \bfinal\b /\/*final*\//' ${MS_BASE_DIR}/var/protocol/*
    if [ $? -ne 0 ];then
        echo "sed final failed"
        exit 1
    fi
    export LD_LIBRARY_PATH=${LD_TMP}

    cd - >/dev/null
    
    echo "#########################################################"
    echo "   Compile protocol success "
    echo "#########################################################"
}

copy_header_files()
{
    cp `find $CURRENT_DIR/../var/ -name "*.h"` $CURRENT_DIR/../src/protocol/
    if [[ $? != 0 ]]
    then
        echo "copy protocol header failed"
        exit 1
    fi
}

compile_proto
copy_header_files



