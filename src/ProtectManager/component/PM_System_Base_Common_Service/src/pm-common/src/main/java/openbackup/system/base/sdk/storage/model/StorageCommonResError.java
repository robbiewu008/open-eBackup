package openbackup.system.base.sdk.storage.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * DME Response Error
 *
 * @author l00272247
 * @since 2020-04-03
 */
@Data
public class StorageCommonResError {
    @JsonProperty("code")
    private long code;

    @JsonProperty("description")
    private String description;
}
