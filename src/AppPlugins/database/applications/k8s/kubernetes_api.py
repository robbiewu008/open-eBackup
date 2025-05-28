import sys

from common.process_handle import process_handle
from k8s.services.resource.resource_api import ResourceApi
from k8s.services.backup.backup_api import BackupApi
from k8s.services.restore.restore_api import RestoreApi
from k8s.logger import log


'''
功能映射字典
'''
fun = {
    'CheckApplication': ResourceApi.check_application,
    'ListApplicationResourceV2': ResourceApi.list_application_resource,
    'QueryCluster': ResourceApi.query_cluster,

    'AllowBackupInLocalNode': BackupApi.allow_backup_in_local_node,
    'AllowBackupSubJobInLocalNode': BackupApi.allow_backup_in_local_node,
    'CheckBackupJobType': BackupApi.check_backup_job_type,
    'QueryJobPermission': BackupApi.query_job_permission,
    'BackupPrerequisite': BackupApi.backup_prerequisite,
    'BackupPrerequisiteProgress': BackupApi.backup_prerequisite_progress,
    'BackupGenSubJob': BackupApi.backup_gen_sub_job,
    'Backup': BackupApi.backup,
    'BackupPostJob': BackupApi.backup_post_job,
    'BackupPostJobProgress': BackupApi.backup_post_job_progress,
    'AbortJob': BackupApi.abort_job,
    'AllowRestoreInLocalNode': RestoreApi.allow_restore_in_local_node,
    'AllowRestoreSubJobInLocalNode': RestoreApi.allow_restore_in_local_node,
    'RestorePrerequisite': RestoreApi.restore_prerequisite,
    'RestorePrerequisiteProgress': RestoreApi.restore_prerequisite_progress,
    'RestoreGenSubJob': RestoreApi.restore_gen_sub_job,
    'Restore': RestoreApi.restore,
    'RestorePostJob': RestoreApi.restore_post_job,
    'RestorePostJobProgress': RestoreApi.restore_post_job_progress
}

# K8S Plugin API 入口
if __name__ == '__main__':
    """
    功能描述:执行K8S插件主程序
    参数:
    @1: 请求类型
    @2: 请求的ID
    @3: 主任务ID
    @4: 子任务ID
    输入:
    标准输入数据为外部程序通过管道向插件程序注入的各项参数数据（可能携带敏感数据）
    样例:
    python3 k8s_plugins.py Backup 1657539915784 2e0c6c0b faa3e2c0
    """
    # 打印日志
    log.info('Enter K8S Plugin process')
    process_handle(log, sys.argv, sys.stdin, fun)
    log.info('Exit  K8S Plugin process')
