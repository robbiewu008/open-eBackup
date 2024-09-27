generate_thrift_cpp()
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
{
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
    cp -f ${HLT_ROOT}/thrifttest/thrift/test.thrift ./
    DEFAULT_DIR=gen-cpp
    if [ -d "&{DEFAULT_DIR}" ];then
        rm -rf &{DEFAULT_DIR}
    fi
    echo "##############thrift -r -gen cpp test.thrift###########"
    ./thrift -r -gen cpp test.thrift
    main_result=$?
    if [ ${main_result} -ne 0 ];then
        echo "#########################################################"
        echo "   generate thrift cpp failed."
        echo "#########################################################"
        exit ${main_result}
    fi

    if [ ! -d "${DEFAULT_DIR}" ];then
        echo "#########################################################"
        echo "   generate thrift cpp failed."
        echo "#########################################################"
        exit 1
    fi
    FILENAME=`ls ${DEFAULT_DIR}/*skeleton.cpp`
    echo ${FILENAME}
    rm -f ${FILENAME}
    cp -f ${DEFAULT_DIR}/*.h  ${HLT_ROOT}/thrifttest/server/
    cp -f ${DEFAULT_DIR}/*.cpp ${HLT_ROOT}/thrifttest/server/
    cp -f ${DEFAULT_DIR}/*.h  ${HLT_ROOT}/thrifttest/client/
    cp -f ${DEFAULT_DIR}/*.cpp ${HLT_ROOT}/thrifttest/client/
    echo "#########################################################"
    echo "   generate thrift cpp succ."
    echo "#########################################################"
}

create_dir()
{
    mkdir -p ${HLT_ROOT}/obj
    mkdir -p ${HLT_ROOT}/bin
    mkdir -p ${HLT_ROOT}/obj/thrifttest/client
    mkdir -p ${HLT_ROOT}/obj/thrifttest/server
    mkdir -p ${HLT_ROOT}/obj/servicecenter/servicefactory
    mkdir -p ${HLT_ROOT}/obj/common
    mkdir -p ${HLT_ROOT}/obj/tinyxml
    mkdir -p ${HLT_ROOT}/obj/json
    mkdir -p ${HLT_ROOT}/obj/sqlite
    mkdir -p ${HLT_ROOT}/obj/securec
    mkdir -p ${HLT_ROOT}/obj/curlclient
    
}

main_enter()
{
    create_dir
    generate_thrift_cpp
}

echo "#########################################################"
echo "   Copyright (C), 2013-2014, Huawei Tech. Co., Ltd."
echo "   Start to compile Agent "
echo "#########################################################"
StartTime=`date '+%Y-%m-%d %H:%M:%S'`

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
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${AGENT_ROOT}/test/stubTest/hlt

exit $main_result