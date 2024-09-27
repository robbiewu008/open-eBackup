package openbackup.postgre.protection.access.service.impl;

import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.service.InstanceProtectionService;
import openbackup.postgre.protection.access.service.PostgreInstanceService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Service;

import java.util.List;
import java.util.stream.Collectors;

/**
 * postgre实例服务
 *
 * @author lWX776769
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-28
 */
@Service
public class PostgreInstanceServiceImpl implements PostgreInstanceService {
    private final ResourceService resourceService;

    private final InstanceProtectionService instanceProtectionService;

    public PostgreInstanceServiceImpl(ResourceService resourceService,
        InstanceProtectionService instanceProtectionService) {
        this.resourceService = resourceService;
        this.instanceProtectionService = instanceProtectionService;
    }

    @Override
    public ProtectedResource getResourceById(String resourceId) {
        return resourceService.getResourceById(resourceId)
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST,
                "Protected resource not exist."));
    }

    @Override
    public List<TaskEnvironment> getEnvNodesByInstanceResource(ProtectedResource instanceResource) {
        if (ResourceSubTypeEnum.POSTGRE_INSTANCE.equalsSubType(instanceResource.getSubType())) {
            return instanceProtectionService.extractEnvNodesBySingleInstance(instanceResource);
        }
        return instanceProtectionService.extractEnvNodesByClusterInstance(instanceResource);
    }

    @Override
    public List<Endpoint> getAgentsByInstanceResource(ProtectedResource instanceResource) {
        List<TaskEnvironment> nodeList = getEnvNodesByInstanceResource(instanceResource);
        return nodeList.stream()
            .map(node -> new Endpoint(node.getUuid(), node.getEndpoint(), node.getPort()))
            .collect(Collectors.toList());
    }
}
