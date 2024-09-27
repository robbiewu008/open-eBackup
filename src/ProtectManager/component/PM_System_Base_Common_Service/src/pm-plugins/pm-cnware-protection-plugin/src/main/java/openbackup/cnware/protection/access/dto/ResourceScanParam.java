package openbackup.cnware.protection.access.dto;

import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;

import lombok.Getter;
import lombok.Setter;

/**
 * 资源扫描Param
 *
 * @author z30047175
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2023-12-18
 */
@Setter
@Getter
public class ResourceScanParam {
    private ProtectedEnvironment environment;

    private Endpoint endpoint;
}
