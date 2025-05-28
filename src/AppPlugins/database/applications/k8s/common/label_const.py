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

class LabelConst:
    COMMON = 'dpa.data.protection.io'
    COMMON_BACKUP = COMMON + ".backup"

    # 所有都会打上的label: 包括pod、快照等
    DPA_BACKUP_POD_COMMON_KEY = f'{COMMON_BACKUP}/tag'
    DPA_BACKUP_POD_COMMON_VALUE = "dpa"

    # 备份pod打上taskId， value未taskId
    DPA_BACKUP_POD_TASK_KEY = f'{COMMON_BACKUP}/task'

    # 白名单pod: whitelist  pvc迁移pod: datamove  检查逻辑端口pod: logiccheck
    DPA_BACKUP_POD_TYPE_KEY = f'{COMMON_BACKUP}/type'
    DPA_BACKUP_POD_TYPE_WHITE_LIST = "whitelist"
    DPA_BACKUP_POD_TYPE_DATA_MOVE = "datamove"
    DPA_BACKUP_POD_TYPE_LOGIC_CHECK = "logiccheck"

    # pod是备份还是恢复 backup or restore
    DPA_BACKUP_POD_ACTION_KEY = f'{COMMON_BACKUP}/action'
    DPA_BACKUP_POD_ACTION_BACKUP = "backup"
    DPA_BACKUP_POD_ACTION_RESTORE = "restore"

    # 备份pod打上node的label, value：nodeName
    DPA_BACKUP_POD_NODE_KEY = f'{COMMON_BACKUP}/node'

    # 备份pod打上, value: pvcName
    DPA_BACKUP_POD_PVC_KEY = f'{COMMON_BACKUP}/pvc'
    DPA_BACKUP_POD_PVC_SIZE_KEY = f'{COMMON_BACKUP}/size'
