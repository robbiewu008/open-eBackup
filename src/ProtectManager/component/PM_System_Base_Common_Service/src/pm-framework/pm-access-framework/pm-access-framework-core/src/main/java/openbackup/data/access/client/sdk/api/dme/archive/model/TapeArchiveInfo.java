package openbackup.data.access.client.sdk.api.dme.archive.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 介质集信息
 *
 * @author z00633516
 * @since 2022-01-29
 */
@Data
public class TapeArchiveInfo {
    @JsonProperty("TapeSetId")
    private String mediaSetId;
}
