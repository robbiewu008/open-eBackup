package openbackup.openstack.protection.access.dto;

import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import lombok.Getter;
import lombok.Setter;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * 资源扫描参数
 *
 * @author c30016231
 * @version [OceanProtect X8000 1.3.0]
 * @since 2022-12-29
 */
@Getter
@Setter
public class ResourceScanParam {
    private ProtectedEnvironment environment;

    private Endpoint endpoint;

    private List<ProtectedResource> projectResources;

    private List<ProtectedResource> domainResources;

    private Map<String, Integer> domainProjectCountMap = new HashMap<>();

    private Map<String, Authentication> domainAuthMap = new HashMap<>();
}