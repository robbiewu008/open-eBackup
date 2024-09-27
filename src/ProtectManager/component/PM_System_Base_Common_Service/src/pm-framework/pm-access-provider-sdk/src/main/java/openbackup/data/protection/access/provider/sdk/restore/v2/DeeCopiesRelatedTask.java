package openbackup.data.protection.access.provider.sdk.restore.v2;

import lombok.Data;

/**
 * 实时侦测安全快照相关任务实体类
 *
 * @author f00809938
 * @since 2023-06-01
 * @version OceanCyber 300 1.1.0
 **/
@Data
public class DeeCopiesRelatedTask {
    /**
     * 任务id
     */
    private String taskId;

    /**
     * 请求id
     */
    private String requestId;

    /**
     * 副本id
     */
    private String snapshotId;

    /**
     * 副本名称
     */
    private String snapshotName;

    /**
     * 文件系统id(DM)
     */
    private String filesystemId;

    /**
     * 文件系统名称
     */
    private String filesystemName;

    /**
     * 租户id
     */
    private String vstoreId;

    /**
     * 租户名称
     */
    private String vstoreName;

    /**
     * 设备id
     */
    private String deviceId;

    /**
     * 设备名称
     */
    private String deviceName;

    /**
     * 过期时间 单位天 永久保留不传
     */
    private Long retentionDay;

    /**
     * 资源ID(PM)
     */
    private String resourceId;

    /**
     * 类型
     */
    private String subType;
}
