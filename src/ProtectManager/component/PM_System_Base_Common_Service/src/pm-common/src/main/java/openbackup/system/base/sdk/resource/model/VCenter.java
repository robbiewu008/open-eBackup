package openbackup.system.base.sdk.resource.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * vCenter信息
 *
 * @author t00482481
 * @since 2020-11-04
 */
@Data
public class VCenter extends ResourceEntity {
    @JsonProperty("user_name")
    private String userName;

    @JsonProperty("password")
    private String password;

    @JsonProperty("endpoint")
    private String endpoint;

    @JsonProperty("port")
    private int port;

    @JsonProperty("link_status")
    private Integer linkStatus;

    @JsonProperty("cert_name")
    private String certName;
}
