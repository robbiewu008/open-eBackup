package openbackup.openstack.protection.access.keystone.dto;

import lombok.Data;

/**
 * service请求参数结构体
 *
 * @author x30038064
 * @version [OceanProtect X8000 1.3.0]
 * @since 2022-12-26
 */
@Data
public class ServiceRequestDto {
    private ServiceDto service;
}
