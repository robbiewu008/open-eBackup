package openbackup.kingbase.protection.access.service.impl;

import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.service.InstanceProtectionService;
import openbackup.kingbase.protection.access.service.KingBaseService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import org.springframework.stereotype.Service;

import java.util.Collections;
import java.util.List;
import java.util.stream.Collectors;

/**
 * KingBase服务
 *
 * @author lWX776769
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-22
 */
@Service
public class KingBaseServiceImpl implements KingBaseService {
    private final ResourceService resourceService;

    private final InstanceProtectionService instanceProtectionService;

    /**
     * 构造方法
     *
     * @param resourceService 资源服务类
     * @param instanceProtectionService 实例保护服务类
     */
    public KingBaseServiceImpl(ResourceService resourceService, InstanceProtectionService instanceProtectionService) {
        this.resourceService = resourceService;
        this.instanceProtectionService = instanceProtectionService;
    }

    @Override
    public ProtectedResource getResourceById(String resourceId) {
        return resourceService.getResourceById(resourceId)
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST,
                "Protected resource not exist. uuid: " + resourceId));
    }

    @Override
    public List<Endpoint> getAgentsByInstanceResource(ProtectedResource instanceResource) {
        List<TaskEnvironment> nodeList = getEnvNodesByInstanceResource(instanceResource);
        return nodeList.stream()
            .map(node -> new Endpoint(node.getUuid(), node.getEndpoint(), node.getPort()))
            .collect(Collectors.toList());
    }

    @Override
    public List<TaskEnvironment> getEnvNodesByInstanceResource(ProtectedResource instanceResource) {
        if (ResourceSubTypeEnum.KING_BASE_INSTANCE.equalsSubType(instanceResource.getSubType())) {
            return instanceProtectionService.extractEnvNodesBySingleInstance(instanceResource);
        }
        return instanceProtectionService.extractEnvNodesByClusterInstance(instanceResource);
    }

    @Override
    public List<TaskResource> getSubInstances(ProtectedResource instanceResource) {
        if (ResourceSubTypeEnum.KING_BASE_INSTANCE.equalsSubType(instanceResource.getSubType())) {
            return Collections.emptyList();
        }
        return instanceResource.getDependencies()
            .get(DatabaseConstants.CHILDREN)
            .stream()
            .map(childNode -> BeanTools.copy(childNode, TaskResource::new))
            .collect(Collectors.toList());
    }

    @Override
    public String getDeployType(String subType) {
        if (ResourceSubTypeEnum.KING_BASE_INSTANCE.equalsSubType(subType)) {
            return DatabaseDeployTypeEnum.SINGLE.getType();
        }
        return DatabaseDeployTypeEnum.AP.getType();
    }
}
