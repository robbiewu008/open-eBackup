package openbackup.openstack.protection.access.keystone.dto;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

/**
 * endpoint请求体
 *
 * @author c30016231
 * @version [OceanProtect X8000 1.3.0]
 * @since 2023-01-30
 */
@Getter
@Setter
public class EndpointDto {
    @JsonProperty("interface")
    private String interfaces;

    @JsonProperty("region_id")
    private String regionId;

    private String url;

    @JsonProperty("service_id")
    private String serviceId;
}
