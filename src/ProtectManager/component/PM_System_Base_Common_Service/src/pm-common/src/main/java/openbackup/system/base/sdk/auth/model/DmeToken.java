package openbackup.system.base.sdk.auth.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * DME token
 *
 * @author z30062305
 * @since 2023-07-027
 */
@Setter
@Getter
public class DmeToken {
    private String tokenStr;

    private List<String> methods;

    @JsonProperty("expires_at")
    private String expiresAt;

    private String issuedAt;

    private TokenUser user;

    private Domain domain;

    private List<CataLog> catalog;

    private List<Role> roles;

    private Project project;
}
