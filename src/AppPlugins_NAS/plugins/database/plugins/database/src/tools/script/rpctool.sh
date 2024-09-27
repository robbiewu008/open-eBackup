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
set +x

PLUGIN_PATH="$(
    cd "$(dirname "$BASH_SOURCE")/../"
    pwd
)"
AGENT_BIN_PATH=${PLUGIN_PATH}/../../ProtectClient-E/bin
THRIFT_SERVER_PORT_FILE=${PLUGIN_PATH}/../../ProtectClient-E/tmp/thriftserverport
GENERALDB_LOG_PATH=${PLUGIN_PATH}/../../ProtectClient-E/slog/Plugins/GeneralDBPlugin

export LD_LIBRARY_PATH=${AGENT_BIN_PATH}:${PLUGIN_PATH}/lib/service:${PLUGIN_PATH}/lib/ext:${PLUGIN_PATH}/lib/dme/3rd:${PLUGIN_PATH}/lib/dme/platform:${PLUGIN_PATH}/lib/dme:${PLUGIN_PATH}/lib/3rd:${PLUGIN_PATH}/lib:${PLUGIN_PATH}/lib/agent_sdk:${LD_LIBRARY_PATH}

if [ $# -lt 3 ]; then
    echo "The number of parameters is less than 3."
    exit 1
fi

[ ! -d "${GENERALDB_LOG_PATH}" ] && mkdir -p "${GENERALDB_LOG_PATH}"

if [ ! -f "${THRIFT_SERVER_PORT_FILE}" ]; then
    echo "Agent thrift server port file not exist."
    exit 1
fi
THRIFT_SERVER_PORT=`cat ${THRIFT_SERVER_PORT_FILE}`

${PLUGIN_PATH}/bin/dbrpctool $1 $2 $3 ${GENERALDB_LOG_PATH} ${THRIFT_SERVER_PORT}
if [ $? -ne 0 ]; then
    echo "Call dbrpccall filed."
    exit 1
fi

echo "Call dbrpccall success."
exit 0