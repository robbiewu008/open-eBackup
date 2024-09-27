package openbackup.system.base.sdk.storage.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * pacific 文件系统快照模型
 *
 * @author j00619968
 * @since 2023-04-17
 */
@Data
public class PacificFileSystemSnapshotBo {
    @JsonProperty("id")
    private String id;

    @JsonProperty("name")
    private String name;

    /**
     * 是否是安全快照, 0 不是, 1 是
     */
    @JsonProperty("is_security_snap")
    private Integer isSecuritySnap;

    /**
     * 过期时间, 时间戳, 秒
     */
    @JsonProperty("expired_date")
    private Long expiredDate;
}
