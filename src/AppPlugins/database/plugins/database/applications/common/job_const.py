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



class JobNameConst:
    """
    任务名称常量类
    """
    # 根据主机信息查询集群信息
    QUERY_HOST_CLUSTER = "QueryHostCluster"
    # 根据应用信息查询集群信息
    QUERY_CLUSTER = "QueryCluster"
    # 检查生产环境信息
    CHECK_APPLICATION = "CheckApplication"
    # 列举环境信息
    LIST_APP_RESOURCE = "ListApplicationResource"
    # 列举环境信息V2
    LIST_APP_RESOURCE_V2 = "ListApplicationResourceV2"
    # 是否允许本地运行备份任务
    ALLOW_BACKUP_IN_LOCAL = "AllowBackupInLocalNode"
    ALLOW_BACKUP_SUBJOB_IN_LOCAL = "AllowBackupSubJobInLocalNode"
    # 查询应用权限信息
    QUERY_PERMISSION = "QueryJobPermission"
    # 检查备份任务类型
    CHECK_BACKUP_JOB_TYPE = "CheckBackupJobType"
    # 备份前置任务
    BACKUP_PRE = "BackupPrerequisite"
    # 备份生成子任务
    BACKUP_GEN_SUB_JOB = "BackupGenSubJob"
    # 备份任务
    BACKUP = "Backup"
    # 备份后置任务
    BACKUP_POST = "BackupPostJob"
    # 查询副本
    QUERY_COPY = "QueryBackupCopy"
    # 副本校验
    CHECK_COPY = "CheckCopy"
    # 删除副本
    DEL_COPY = "DelCopy"
    # 中止任务
    ABORT_JOB = "AbortJob"
    # 暂停任务
    PAUSE_JOB = "PauseJob"
    # 备份前置任务进度
    BACKUP_PER_PROGRESS = f"{BACKUP_PRE}_Progress"
    # 备份任务进度
    BACKUP_PROGRESS = f"{BACKUP}_Progress"
    # 备份后置任务进度
    BACKUP_POST_PROGRESS = f"{BACKUP_POST}_Progress"
    # 中止任务进度
    ABORT_JOB_PROGRESS = f"{ABORT_JOB}_Progress"
    # 删除副本任务进度
    DEL_COPY_PROGRESS = f"{DEL_COPY}_Progress"
    # 即时挂载任务
    LIVE_MOUNT = "Livemount"
    # 取消即时挂载任务
    CANCEL_LIVE_MOUNT = "CancelLivemount"
    # 是否允许本地运行恢复任务
    ALLOW_RESTORE_IN_LOCAL = "AllowRestoreInLocalNode"
    ALLOW_RESTORE_SUBJOB_IN_LOCAL = "AllowRestoreSubJobInLocalNode"
    # 恢复前置任务
    RESTORE_PRE = "RestorePrerequisite"
    # 恢复生成子任务
    RESTORE_GEN_SUB_JOB = "RestoreGenSubJob"
    # 恢复任务
    RESTORE = "Restore"
    # 恢复后置任务
    RESTORE_POST = "RestorePostJob"
    # 恢复前置任务进度
    RESTORE_PER_PROGRESS = f"{RESTORE_PRE}_Progress"
    # 恢复任务进度
    RESTORE_PROGRESS = f"{RESTORE}_Progress"
    # 恢复后置任务进度
    RESTORE_POST_PROGRESS = f"{RESTORE_POST}_Progress"
    DELIVER_TASK_STATUS = "DeliverTaskStatus"


class ParamKeyConst:
    """
    参数键常量类
    """
    JOB = "job"
    TYPE = "type"
    SUBTYPE = "subType"
    REPOSITORIES = "repositories"
    REPOSITORY_TYPE = "repositoryType"
    PATH = "path"
    ID = "id"
    PROTECT_OBJECT = "protectObject"
    PROTECT_ENV = "protectEnv"
    TARGET_OBJECT = "targetObject"
    TARGET_ENV = "targetEnv"
    EXT_INFO = "extendInfo"
    AUTH = "auth"
    NAME = "name"
    JOB_PARAM = "jobParam"
    BACKUP_TYPE = "backupType"
    SUB_JOB = "subJob"
    JOB_NAME = "jobName"
    JOB_ID = "jobId"
    SUB_JOB_ID = "subJobId"
    NODES = "nodes"
    ENDPOINT = "endpoint"
    RESTORE_TIMESTAMP = "restoreTimestamp"
    # 新位置或老位置恢复字段
    RESTORE_TARGET_LOCATION = "targetLocation"
    # 副本beginTime
    COPY_BEGIN_TIME = "beginTime"
    # 副本endTime
    COPY_END_TIME = "endTime"
    # 副本backupTime
    COPY_BAK_TIME = "backupTime"
    # 是否支持副本校验
    COPY_VERIFY_FILE = "copyVerifyFile"
    # 应用
    APPLICATION = "application"
