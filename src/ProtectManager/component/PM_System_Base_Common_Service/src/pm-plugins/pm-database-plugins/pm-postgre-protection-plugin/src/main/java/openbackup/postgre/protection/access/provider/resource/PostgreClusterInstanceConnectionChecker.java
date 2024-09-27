package openbackup.postgre.protection.access.provider.resource;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.access.framework.resource.service.provider.UnifiedResourceConnectionChecker;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.service.InstanceResourceService;
import openbackup.postgre.protection.access.common.PostgreConstants;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;

/**
 * PostgreClusterInstanceConnectionChecker连接检查
 *
 * @author x30028756
 * @version [OceanProtect X8000 1.6.0]
 * @since 2024-04-25
 */
@Slf4j
@Component
public class PostgreClusterInstanceConnectionChecker extends UnifiedResourceConnectionChecker {
    private final ProtectedEnvironmentService protectedEnvironmentService;

    private final InstanceResourceService instanceResourceService;

    /**
     * 有参构造
     *
     * @param environmentRetrievalsService 环境服务
     * @param agentUnifiedService agent接口实现
     * @param protectedEnvironmentService 环境服务
     * @param instanceResourceService 实例资源服务
     */
    public PostgreClusterInstanceConnectionChecker(ProtectedEnvironmentRetrievalsService environmentRetrievalsService,
        AgentUnifiedService agentUnifiedService, ProtectedEnvironmentService protectedEnvironmentService,
        InstanceResourceService instanceResourceService) {
        super(environmentRetrievalsService, agentUnifiedService);
        this.protectedEnvironmentService = protectedEnvironmentService;
        this.instanceResourceService = instanceResourceService;
    }

    @Override
    public Map<ProtectedResource, List<ProtectedEnvironment>> collectConnectableResources(ProtectedResource resource) {
        log.info("Start collecting connectable resources.");
        resource.setEnvironment(protectedEnvironmentService.getEnvironmentById(resource.getParentUuid()));
        resource.getExtendInfo().put(DatabaseConstants.VIRTUAL_IP, resource.getEnvironment().getEndpoint());
        try {
            instanceResourceService.setClusterInstanceNodeRole(resource);
        } catch (LegoCheckedException | NullPointerException e) {
            log.error("Collecting postgre cluster instance check error", ExceptionUtil.getErrorMessage(e));
            throw new LegoCheckedException(CommonErrorCode.RESOURCE_LINK_STATUS_OFFLINE,
                "target environment is offline.");
        }
        List<ProtectedResource> children = resource.getDependencies().get(DatabaseConstants.CHILDREN);
        // 如果没有主节点直接报错
        Optional<ProtectedResource> childOptional = children.stream()
            .filter(
                item -> StringUtils.equals(PostgreConstants.PRIMARY, item.getExtendInfo().get(DatabaseConstants.ROLE)))
            .findFirst();
        if (!childOptional.isPresent()) {
            throw new LegoCheckedException(CommonErrorCode.RESOURCE_LINK_STATUS_OFFLINE,
                "target environment is offline.");
        }
        return super.collectConnectableResources(resource);
    }

    @Override
    public boolean applicable(ProtectedResource object) {
        return Objects.nonNull(object) && ResourceSubTypeEnum.POSTGRE_CLUSTER_INSTANCE.getType()
            .equals(object.getSubType());
    }
}