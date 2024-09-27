package openbackup.system.base.sdk.accesspoint.model;

import lombok.Data;

/**
 * 标准备份卷初始信息
 *
 * @author s00574739
 * @since 2021-07-05
 */
@Data
public class StandardBackupVolInitInfo {
    private String volName;

    private String nodeId;
}
