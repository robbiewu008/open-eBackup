package openbackup.data.protection.access.provider.sdk.backup.v2;

import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

/**
 * 备份特性功能服务
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/5/27
 */
public interface BackupFeatureService {
    /**
     * 是否支持数据备份与日志备份并行备份
     *
     * @param resourceId 资源ID
     * @return 是否支持并行
     */
    boolean isSupportDataAndLogParallelBackup(String resourceId);

    /**
     * 是否支持数据备份与日志备份并行备份
     *
     * @param resource 资源
     * @return 是否支持并行
     */
    boolean isSupportDataAndLogParallelBackup(ProtectedResource resource);

    /**
     * 转换备份类型
     *
     * @param backupType 备份类型
     * @param resourceId 资源ID
     * @return 转换后的备份类型
     */
    BackupTypeConstants transferBackupType(BackupTypeConstants backupType, String resourceId);

    /**
     * 转换备份类型
     *
     * @param backupType 备份类型
     * @param resource 资源
     * @return 转换后的备份类型
     */
    BackupTypeConstants transferBackupType(BackupTypeConstants backupType, ProtectedResource resource);
}
