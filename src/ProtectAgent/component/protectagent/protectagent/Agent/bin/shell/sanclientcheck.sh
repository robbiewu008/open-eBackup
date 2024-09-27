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

AGENT_ROOT_PATH=$1

. "${AGENT_ROOT_PATH}/sbin/agent_sbin_func.sh"

#for log
LOG_FILE_NAME="${LOG_PATH}/sanclientaction.log"

qla2xxx_status=`cat /sys/module/qla2xxx/parameters/qlini_mode`
if [ ${qla2xxx_status} != 'dual' ]; then
  Log "qla2xxx_status is not nomoral"
  exit ${ERROR_QLA2XXX_STAUTS}
fi

systemctl status targetcli
if [ $? -ne 0 ]; then
  Log "targetcli status is not nomoral"
  exit ${ERROR_TARGETCLI_STATUS}
fi

Log "check sanclient success"
exit 0