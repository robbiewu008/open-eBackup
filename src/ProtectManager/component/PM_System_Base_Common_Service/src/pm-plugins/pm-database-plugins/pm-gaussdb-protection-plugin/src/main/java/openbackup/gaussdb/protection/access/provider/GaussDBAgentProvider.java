package openbackup.gaussdb.protection.access.provider;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.data.access.framework.agent.DataBaseAgentSelector;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.gaussdb.protection.access.constant.GaussDBConstant;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.StreamUtil;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * 功能描述
 *
 * @author c00826511
 * @since 2023-07-28
 */
@Slf4j
@Component
public class GaussDBAgentProvider extends DataBaseAgentSelector {
    private final ProtectedEnvironmentRetrievalsService envRetrievalsService;

    private final ResourceService resourceService;

    /**
     * 构造器
     *
     * @param envRetrievalsService envRetrievalsService
     * @param resourceService resourceService
     */
    public GaussDBAgentProvider(ProtectedEnvironmentRetrievalsService envRetrievalsService,
        ResourceService resourceService) {
        this.envRetrievalsService = envRetrievalsService;
        this.resourceService = resourceService;
    }

    @Override
    public List<Endpoint> getSelectedAgents(AgentSelectParam agentSelectParam) {
        Optional<ProtectedResource> resourceOptional = resourceService.getResourceById(
            agentSelectParam.getResource().getRootUuid());
        if (!resourceOptional.isPresent()) {
            throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST);
        }
        ProtectedResource resource = resourceOptional.get();
        if (Objects.equals(agentSelectParam.getJobType(), JobTypeEnum.BACKUP.getValue())) {
            return buildAgentEndpointFromEnv(resource);
        } else if (Objects.equals(agentSelectParam.getJobType(), JobTypeEnum.RESOURCE_SCAN.getValue())) {
            return resource.getDependencies()
                .get(GaussDBConstant.GAUSSDB_AGENTS)
                .stream()
                .map(this::getAgentEndpoint)
                .collect(Collectors.toList());
        } else if (Objects.equals(agentSelectParam.getJobType(), JobTypeEnum.RESTORE.getValue())) {
            Map<ProtectedResource, List<ProtectedEnvironment>> protectedResourceMap = new HashMap<>();
            protectedResourceMap = envRetrievalsService.collectConnectableResources(resource.getUuid());
            return protectedResourceMap.values()
                .stream()
                .flatMap(List::stream)
                .map(this::getAgentEndpoint)
                .collect(Collectors.toList());
        } else {
            return super.getSelectedAgents(agentSelectParam);
        }
    }

    @Override
    public boolean applicable(AgentSelectParam agentSelectParam) {
        return ResourceSubTypeEnum.HCS_GAUSSDB_INSTANCE.equalsSubType(agentSelectParam.getResource().getSubType());
    }

    /**
     * 从环境中获取agent端口信息
     *
     * @param protectedEnvironment 环境信息
     * @return List<Endpoint>
     */
    private List<Endpoint> buildAgentEndpointFromEnv(ProtectedResource protectedEnvironment) {
        Map<String, List<ProtectedResource>> dependencies = protectedEnvironment.getDependencies();
        List<ProtectedResource> resourceList = dependencies.get(DatabaseConstants.AGENTS);
        return resourceList.stream()
            .flatMap(StreamUtil.match(ProtectedEnvironment.class))
            .map(this::getAgentEndpoint)
            .collect(Collectors.toList());
    }

    private Endpoint getAgentEndpoint(ProtectedResource agentEnv) {
        return new Endpoint(agentEnv.getUuid(), agentEnv.getEndpoint(),
            Optional.ofNullable(agentEnv.getPort()).orElse(0));
    }
}
