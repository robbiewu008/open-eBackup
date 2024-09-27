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

find ${AGENT_ROOT} -name "*.gcno" | xargs rm -f
find ${AGENT_ROOT} -name "*.gcda" | xargs rm -f
find ${AGENT_ROOT} -name "*.o" | xargs rm -f
find ${AGENT_ROOT} -name "*.d" | xargs rm -f
find ${AGENT_ROOT} -name "*.expand" | xargs rm -f

if [ "$1" = "all" ]; then
    rm -rf ${AGENT_ROOT}/open_src/fcgi
    rm -rf ${AGENT_ROOT}/open_src/openssl
    rm -rf ${AGENT_ROOT}/open_src/jsoncpp
    rm -rf ${AGENT_ROOT}/open_src/snmp++
    rm -rf ${AGENT_ROOT}/open_src/nginx
    rm -rf ${AGENT_ROOT}/open_src/nginx_tmp
    rm -rf ${AGENT_ROOT}/open_src/sqlite
    rm -rf ${AGENT_ROOT}/open_src/tinyxml
    rm -rf ${AGENT_ROOT}/open_src/objs_ngx
fi