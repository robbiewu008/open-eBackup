package openbackup.data.access.client.sdk.api.framework.dee.model;

import lombok.Data;

/**
 * 安全一体机基于快照创建克隆请求参数
 *
 * @author w00574036
 * @since 2024-04-18
 * @version [OceanCyber 300 1.2.0]
 */
@Data
public class OcLiveMountCloneReq {
    /**
     * 请求id
     */
    private String requestId;

    /**
     * 任务id
     */
    private String taskId;

    /**
     * 存储设备id
     */
    private String deviceId;

    /**
     * 克隆文件系统名
     */
    private String name;

    /**
     * 基于文件系统的ID
     */
    private String parentFileSystemId;

    /**
     * 基于文件系统的名称
     */
    private String parentFileSystemName;

    /**
     * 父文件系统快照ID
     */
    private String parentSnapshotId;

    /**
     * 父文件系统快照名称
     */
    private String parentSnapshotName;

    /**
     * 租户Id
     */
    private String vstoreId;
}
