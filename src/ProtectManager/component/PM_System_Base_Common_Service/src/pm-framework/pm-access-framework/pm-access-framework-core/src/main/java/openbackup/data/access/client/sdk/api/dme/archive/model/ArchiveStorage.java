package openbackup.data.access.client.sdk.api.dme.archive.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 归档存储信息
 *
 * @author z30009433
 * @since 2020-12-31
 */
@Data
public class ArchiveStorage {
    @JsonProperty("Type")
    int type;

    @JsonProperty("Cloud")
    CloudArchiveInfo cloud;

    @JsonProperty("Tape")
    private TapeArchiveInfo tapeArchiveInfo;
}
