package openbackup.system.base.sdk.resource.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

/**
 * network信息
 *
 * @author t00482481
 * @since 2020-11-04
 */
@Getter
@Setter
public class Network extends ResourceEntity {
    @JsonProperty("vcenter_ip")
    private String vCenterIP;

    @JsonProperty("env_ip")
    private String envIp;

    @JsonProperty("mo_id")
    private String moId;

    @JsonProperty("alias_type")
    private String aliasType;

    @JsonProperty("alias_value")
    private String aliasValue;

    @JsonProperty("capacity")
    private String capacity;

    @JsonProperty("free_space")
    private String freeSpace;

    @JsonProperty("uncommitted")
    private String uncommitted;

    @JsonProperty("children")
    private String children;

    @JsonProperty("is_template")
    private String isTemplate;
}
