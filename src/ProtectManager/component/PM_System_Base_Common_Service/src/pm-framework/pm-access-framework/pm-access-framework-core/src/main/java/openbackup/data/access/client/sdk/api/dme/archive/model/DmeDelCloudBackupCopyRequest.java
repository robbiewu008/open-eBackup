package openbackup.data.access.client.sdk.api.dme.archive.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * dme delete cloud backup copy request
 *
 * @author g00500588
 * @since 2021/12/8
 */
@Data
public class DmeDelCloudBackupCopyRequest extends DmeDelCloudArchiveCopyRequest {
    @JsonProperty("ArchiveRemoteStorageType")
    private Integer archiveRemoteStorageType;
}
