package openbackup.openstack.protection.access.keystone.dto;

import lombok.Data;

/**
 * Auth常量类
 *
 * @author x30038064
 * @version [OceanProtect X8000 1.3.0]
 * @since 2022-12-24
 */
@Data
public class AuthDto {
    private IdentityDto identity;
    private ScopeDto scope;
}