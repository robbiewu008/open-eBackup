package openbackup.system.base.sdk.auth.model;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;

import lombok.Data;

/**
 * Role
 *
 * @author y30021475
 * @since 2023-07-08
 */
@Data
@JsonIgnoreProperties(ignoreUnknown = true)
public class Role {
    private String name;
    private String id;
}
