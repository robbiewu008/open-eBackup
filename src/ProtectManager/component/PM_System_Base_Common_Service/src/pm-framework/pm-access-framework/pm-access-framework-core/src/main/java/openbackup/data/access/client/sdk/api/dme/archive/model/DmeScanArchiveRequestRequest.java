package openbackup.data.access.client.sdk.api.dme.archive.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 扫描归档副本 请求体
 *
 * @author z30009433
 * @since 2020-12-31
 */
@Data
public class DmeScanArchiveRequestRequest {
    @JsonProperty("TaskID")
    String taskId;

    @JsonProperty("RequestID")
    String requestId;

    @JsonProperty("ArchiveStorage")
    ArchiveStorage archiveStorage;

    @JsonProperty("BackupStorage")
    BackupStorage backupStorage;
}
