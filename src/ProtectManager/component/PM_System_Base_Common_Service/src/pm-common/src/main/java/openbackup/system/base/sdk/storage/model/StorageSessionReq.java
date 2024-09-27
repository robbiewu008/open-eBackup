package openbackup.system.base.sdk.storage.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * OceanStorageSessionReq
 *
 * @author p00511147
 * @since 2020-12-14
 */
@Data
public class StorageSessionReq {
    @JsonProperty("password")
    private String password;

    @JsonProperty("username")
    private String userName;

    @JsonProperty("scope")
    private int scope;
}
