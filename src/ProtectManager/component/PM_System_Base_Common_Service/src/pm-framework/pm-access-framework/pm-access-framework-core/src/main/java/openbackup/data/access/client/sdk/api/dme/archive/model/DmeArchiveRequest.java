package openbackup.data.access.client.sdk.api.dme.archive.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.Map;

/**
 * 归档副本请求
 *
 * @author y00490893
 * @since 2020-12-16
 */
@Data
public class DmeArchiveRequest {
    @JsonProperty("RequestId")
    private String requestId;

    @JsonProperty("PolicyID")
    private String policyId;

    @JsonProperty("Name")
    private String name;

    @JsonProperty("TaskID")
    private String taskId;

    @JsonProperty("ArchiveCopyId")
    private String archiveCopyId;

    @JsonProperty("ChainId")
    private String chainId;

    @JsonProperty("ArchiveStorage")
    private Map archiveStorage;

    @JsonProperty("OriginBackupCopyID")
    private String originBackupCopyId;

    @JsonProperty("ApplicationType")
    private int applicationType;

    @JsonProperty("BackupStorage")
    private DmeBackupStorage backupStorage;

    @JsonProperty("Qos")
    private int qos;

    @JsonProperty("LocalESN")
    private String localEsn;

    private int driverCount;
}
