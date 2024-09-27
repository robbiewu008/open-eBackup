package openbackup.data.protection.access.provider.sdk.copy;

import com.fasterxml.jackson.annotation.JsonProperty;
import com.fasterxml.jackson.databind.PropertyNamingStrategy;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Getter;
import lombok.Setter;

import java.util.Date;

/**
 * Copy Info
 *
 * @author l00272247
 * @since 2020-09-23
 */
@Getter
@Setter
@JsonNaming(PropertyNamingStrategy.SnakeCaseStrategy.class)
public class CopyInfoBo extends CopyResourceBase {
    @JsonProperty("uuid")
    private String uuid;

    @JsonProperty("chain_id")
    private String chainId;

    @JsonProperty("timestamp")
    private String timestamp;

    @JsonProperty("display_timestamp")
    private String displayTimestamp;

    @JsonProperty("deletable")
    private boolean isDeletable;

    @JsonProperty("status")
    private String status;

    @JsonProperty("location")
    private String location;

    @JsonProperty("backup_type")
    private int backupType; // 1：完全备份 2：增量备份 3：差异备份 4：日志备份

    @JsonProperty("generated_by")
    private String generatedBy;

    @JsonProperty("generated_time")
    private String generatedTime;

    @JsonProperty("origin_copy_time_stamp")
    private String originCopyTimeStamp;

    @JsonProperty("features")
    private int features;

    @JsonProperty("indexed")
    private String indexed;

    @JsonProperty("generation")
    private int generation;

    @JsonProperty("parent_copy_uuid")
    private String parentCopyUuid;

    @JsonProperty("retention_type")
    private int retentionType;

    @JsonProperty("retention_duration")
    private int retentionDuration;

    @JsonProperty("duration_unit")
    private String durationUnit;

    @JsonProperty("expiration_time")
    private Date expirationTime;

    @JsonProperty("properties")
    private String properties;

    @JsonProperty("sla_name")
    private String slaName;

    @JsonProperty("sla_properties")
    private String slaProperties;

    @JsonProperty("job_type")
    private String jobType;

    @JsonProperty("user_id")
    private String userId;

    @JsonProperty("is_archived")
    private Boolean isArchived;

    @JsonProperty("is_replicated")
    private Boolean isReplicated;

    @JsonProperty("name")
    private String name;

    @JsonProperty("storage_id")
    private String storageId;

    @JsonProperty("source_copy_type")
    private int sourceCopyType;

    @JsonProperty("device_esn")
    private String deviceEsn;

    @JsonProperty("pool_id")
    private String poolId;

    @JsonProperty("storage_unit_id")
    private String storageUnitId;

    /**
     * 原始副本备份id多集群场景复制场景使用
     */
    @JsonProperty("origin_backup_id")
    private String originBackupId;

    /**
     * 是否为存储快照
     */
    @JsonProperty("storage_snapshot_flag")
    private Boolean isStorageSnapshot;

    /**
     * 扩展类型
     */
    @JsonProperty("extend_type")
    private String extendType;
}
