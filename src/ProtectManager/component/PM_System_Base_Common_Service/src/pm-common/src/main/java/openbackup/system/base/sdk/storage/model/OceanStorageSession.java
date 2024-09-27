package openbackup.system.base.sdk.storage.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 存储信息
 *
 * @author p00511147
 * @since 2020-12-14
 */
@Data
public class OceanStorageSession {
    @JsonProperty("passphrase")
    private String passphrase;
}
