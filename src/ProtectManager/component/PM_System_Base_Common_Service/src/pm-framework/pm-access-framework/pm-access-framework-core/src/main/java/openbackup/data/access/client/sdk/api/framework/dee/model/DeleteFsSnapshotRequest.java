package openbackup.data.access.client.sdk.api.framework.dee.model;

import lombok.Data;

/**
 * 删除快照请求参数
 * 支持Dorado/OceanProtect/Pacific设备
 *
 * @author q00564609
 * @since 2024-06-24
 * @version OceanCyber 300 1.2.0
 */
@Data
public class DeleteFsSnapshotRequest {
    /**
     * 请求id
     */
    private String requestId;

    /**
     * 任务id
     */
    private String taskId;

    /**
     * 快照名
     */
    private String snapshotName;

    /**
     * Dorado/OceanProtect设备：文件系统ID
     * Pacific设备：命名空间ID
     */
    private String filesystemId;

    /**
     * 存储设备ID
     */
    private String deviceId;

    /**
     * 租户ID
     */
    private String vstoreId;

    /**
     * 生成方式
     */
    private String generateBy;
}
