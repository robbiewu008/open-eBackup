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

import sys

from common.process_handle import process_handle
from tdsql.handle.backup.backup_ability import BackupAbility
from tdsql.handle.livemount.tdsql_livemount_service import TdsqlLivemountService
from tdsql.handle.job.job_ability import JobAbility
from tdsql.handle.resource.resource_ability import ResourceAbility
from tdsql.handle.restore.restore_ability import RestoreAbility
from tdsql.logger import log

fun = {
    'CheckApplication': ResourceAbility.check_application,
    'ListApplicationResourceV2': ResourceAbility.list_application_v2,
    'QueryCluster': ResourceAbility.query_cluster,
    'FinalizeClear': ResourceAbility.finalize_clear,

    'QueryBackupCopy': BackupAbility.query_backup_copy,
    'AllowBackupInLocalNode': BackupAbility.allow_backup_in_local_node,
    'CheckBackupJobType': BackupAbility.check_backup_job_type,
    'BackupPrerequisite': BackupAbility.backup_prerequisite,
    'BackupPrerequisiteProgress': BackupAbility.backup_prerequisite_progress,
    'BackupGenSubJob': BackupAbility.backup_gen_sub_job,
    'Backup': BackupAbility.backup,
    'BackupPostJob': BackupAbility.backup_post_job,
    'BackupPostJobProgress': BackupAbility.backup_post_job_progress,

    'QueryJobPermission': JobAbility.query_job_permission,
    'AbortJob': JobAbility.abort_job,
    'PauseJob': JobAbility.pause_job,

    'AllowRestoreInLocalNode': RestoreAbility.allow_restore_in_local_node,
    'RestorePrerequisite': RestoreAbility.restore_prerequisite,
    'RestorePrerequisiteProgress': RestoreAbility.restore_prerequisite_progress,
    'RestoreGenSubJob': RestoreAbility.restore_gen_sub_job,
    'Restore': RestoreAbility.restore,
    'RestorePostJob': RestoreAbility.restore_post_job,
    'RestorePostJobProgress': RestoreAbility.restore_post_job_progress,

    'LiveMount': TdsqlLivemountService.live_mount,
    'CancelLiveMount': TdsqlLivemountService.cancel_live_mount
}
# TDSQL Plugin API 入口
if __name__ == '__main__':
    """
    功能描述:执行TDSQL插件主程序
    参数:
    @1: 请求类型
    @2: 请求的ID
    @3: 主任务ID
    @4: 子任务ID
    输入:
    标准输入数据为外部程序通过管道向插件程序注入的各项参数数据（可能携带敏感数据）
    样例:
    python3 tdsql_api.py Backup 1657539915784 2e0c6c0b faa3e2c0
    """
    # 打印日志
    log.info('Enter TDSQL Plugin process')
    process_handle(log, sys.argv, sys.stdin, fun)
    log.info('Exit  TDSQL Plugin process')
