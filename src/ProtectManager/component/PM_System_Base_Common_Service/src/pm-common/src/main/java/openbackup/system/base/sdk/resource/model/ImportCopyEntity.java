package openbackup.system.base.sdk.resource.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * import copy entity
 *
 * @author g00500588
 * @since 2021/9/16
 */
@Data
public class ImportCopyEntity extends ResourceEntity {
    @JsonProperty("sla_id")
    private String slaId;

    @JsonProperty("sla_name")
    private String slaName;

    @JsonProperty("sla_status")
    private String slaStatus;

    @JsonProperty("sla_compliance")
    private String slaCompliance;

    private String location;
}
