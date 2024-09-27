package openbackup.openstack.protection.access.keystone.dto;

import lombok.Getter;
import lombok.Setter;

/**
 * 创建endpoint请求体
 *
 * @author c30016231
 * @version [OceanProtect X8000 1.3.0]
 * @since 2023-01-30
 */
@Getter
@Setter
public class EndpointRequestDto {
    private EndpointDto endpoint;
}
