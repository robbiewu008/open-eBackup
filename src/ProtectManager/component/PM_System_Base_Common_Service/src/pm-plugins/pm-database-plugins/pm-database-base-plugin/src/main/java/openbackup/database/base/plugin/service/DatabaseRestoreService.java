package openbackup.database.base.plugin.service;

/**
 * 数据库恢复服务
 *
 * @author lWX776769
 * @version [DataBackup 1.3.0]
 * @since 2023-02-08
 */
public interface DatabaseRestoreService {
    /**
     * 检查恢复的部署操作系统
     *
     * @param sourceDeployOs 源端部署系统
     * @param targetDeployOs 目标端部署系统
     */
    void checkDeployOperatingSystem(String sourceDeployOs, String targetDeployOs);

    /**
     * 检验恢复的资源类型
     *
     * @param sourceSubType 源端资源类型
     * @param targetSubType 目标端资源类型
     */
    void checkResourceSubType(String sourceSubType, String targetSubType);

    /**
     * 检验恢复的集群类型
     *
     * @param sourceClusterType 源端集群类型
     * @param targetClusterType 目标端集群类型
     */
    void checkClusterType(String sourceClusterType, String targetClusterType);

    /**
     * 检验version是否一致
     *
     * @param sourceVersion 源端版本
     * @param targetVersion 目标端版本
     */
    void checkVersion(String sourceVersion, String targetVersion);
}
