package openbackup.system.base.sdk.storage.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

/**
 * 本地存储文件系统BO
 *
 * @author g30003063
 * @since 2021/12/14
 */
@Getter
@Setter
public class StorageFileSystemBo {
    /**
     * ID
     */
    @JsonProperty("ID")
    private String id;

    /**
     * 名称
     */
    @JsonProperty("NAME")
    private String name;

    /**
     * 远程复制ID列表
     */
    @JsonProperty("REMOTEREPLICATIONIDS")
    private String remoteReplicationIds;

    /**
     * 双活PairID列表
     */
    @JsonProperty("HYPERMETROPAIRIDS")
    private String hyperMetroPairIds;


    /**
     * NAS协议支持多种安全模式
     * <p>
     * 1：Native安全模式
     * 2：NTFS安全模式
     * 3：UNIX安全模式
     */
    @JsonProperty("securityStyle")
    private String securityStyle;

    /**
     * 文件系统类型
     * <p>
     * 0：普通文件系统
     * 1：WORM文件系统
     */
    @JsonProperty("SUBTYPE")
    private String subType;

    /**
     * 是否是克隆文件系统
     */
    @JsonProperty("ISCLONEFS")
    private boolean isCloneFs;
}
