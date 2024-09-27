package openbackup.data.access.client.sdk.api.dme.archive.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * Dorado存储信息
 *
 * @author y00490893
 * @since 2020-12-19
 */
@Data
public class DmeBackupStorage {
    @JsonProperty("Type")
    private int type = 1;

    @JsonProperty("IP")
    private String ip;

    @JsonProperty("Port")
    private int port;

    @JsonProperty("Username")
    private String username;

    @JsonProperty("Password")
    private String password;

    @JsonProperty("Certificate")
    private String certificate;
}
