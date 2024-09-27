#!/bin/bash
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
OUTPUT_CODE_DIR=${WORKSPACE}/output/code
if [ ! -d ${OUTPUT_CODE_DIR} ];then
    mkdir -p ${OUTPUT_CODE_DIR}
fi
rm -rf ${WORKSPACE}/code/Agent/open_src/SNMP/.git
rm -rf ${WORKSPACE}/code/Agent/open_src/boost/.git
rm -rf ${WORKSPACE}/code/Agent/open_src/boost/doc/
rm -rf ${WORKSPACE}/code/Agent/open_src/curl/.git
rm -rf ${WORKSPACE}/code/Agent/open_src/fcgi2/.git
rm -rf ${WORKSPACE}/code/Agent/open_src/gperftools/.git
rm -rf ${WORKSPACE}/code/Agent/open_src/jsoncpp_00.11.0/.git
rm -rf ${WORKSPACE}/code/Agent/open_src/libevent/.git
rm -rf ${WORKSPACE}/code/Agent/open_src/nginx/.git
rm -rf ${WORKSPACE}/code/Agent/open_src/openssl/.git
rm -rf ${WORKSPACE}/code/Agent/open_src/sqlite/.git
rm -rf ${WORKSPACE}/code/Agent/open_src/thrift/.git
rm -rf ${WORKSPACE}/code/Agent/open_src/tinyxml2/.git
rm -rf ${WORKSPACE}/code/Agent/open_src/zlib/.git
rm -rf ${WORKSPACE}/code/Agent/open_src/util-linux/.git
cd ${WORKSPACE}/code
zip -r Agent_code.zip Agent
cp -r Agent_code.zip ${OUTPUT_CODE_DIR}