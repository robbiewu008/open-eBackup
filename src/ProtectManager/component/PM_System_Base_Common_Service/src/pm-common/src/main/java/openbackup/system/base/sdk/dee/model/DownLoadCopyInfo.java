package openbackup.system.base.sdk.dee.model;

import lombok.Data;

import java.util.List;

/**
 * 副本相关的接口
 *
 * @author jwx701567
 * @version [OceanProtect A8000 1.1.0]
 * @since 2022-01-28
 */
@Data
public class DownLoadCopyInfo {
    private String resourceSubType;

    private String userId;

    private String uuid;

    private List<Snapshot> snapshots;

    /**
     * 设备Id
     */
    private String deviceId;

    /**
     * 存储单元id
     */
    private String storageId;
}
