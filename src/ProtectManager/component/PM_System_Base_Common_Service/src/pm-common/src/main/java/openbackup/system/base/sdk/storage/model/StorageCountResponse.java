package openbackup.system.base.sdk.storage.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

/**
 * The StorageCountResponse
 *
 * @author g30003063
 * @since 2022-08-11
 */
@Getter
@Setter
public class StorageCountResponse {
    @JsonProperty("COUNT")
    private Integer count;
}
