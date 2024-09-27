package openbackup.system.base.sdk.storage.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 文件系统快照信息
 *
 * @author nwx1077006
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-10
 */
@Data
public class StorageFileSystemSnapshotBo {
    @JsonProperty("ID")
    private String id;

    @JsonProperty("NAME")
    private String name;

    // 是否是安全快照
    private Boolean isSecuritySnap;

    // 是否在保护期内
    private Boolean isInProtectionPeriod;
}
