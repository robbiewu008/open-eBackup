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
set +x

NGINX_ROOT_PATH="${AGENT_ROOT}/nginx/"
NGINX_CONF_PATH="${AGENT_ROOT}/nginx/conf/nginx.conf"

LOG_USER=$LOGNAME
if [ "root" = "${LOG_USER}" ]; then
    echo "You can not execute this script with user root."
    exit 1
fi

main()
{
    if [ ! -d "${NGINX_ROOT_PATH}" ]; then
        return 1
    fi

    cd "${NGINX_ROOT_PATH}"
    ${NGINX_ROOT_PATH}/rdnginx -c ${NGINX_CONF_PATH} -s reload
    return 0
}

main
exit $?