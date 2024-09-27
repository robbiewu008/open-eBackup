package openbackup.data.protection.access.provider.sdk.restore;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * host entity
 *
 * @author y30000858
 * @since 2020-09-19
 */
@Data
public class Source {
    @JsonProperty("source_name")
    private String sourceName;
    @JsonProperty("source_location")
    private String sourceLocation;
}
