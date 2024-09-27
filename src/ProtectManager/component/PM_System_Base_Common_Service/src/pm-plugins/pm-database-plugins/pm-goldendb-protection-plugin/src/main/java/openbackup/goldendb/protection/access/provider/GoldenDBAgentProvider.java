/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.goldendb.protection.access.provider;

import openbackup.data.access.framework.agent.DataBaseAgentSelector;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.goldendb.protection.access.service.GoldenDbService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.StreamUtil;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.LinkedList;
import java.util.List;
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
public class GoldenDBAgentProvider extends DataBaseAgentSelector {
    private final GoldenDbService goldenDbService;

    public GoldenDBAgentProvider(GoldenDbService goldenDbService) {
        this.goldenDbService = goldenDbService;
    }

    @Override
    public List<Endpoint> getSelectedAgents(AgentSelectParam agentSelectParam) {
        ProtectedResource backupResource = agentSelectParam.getResource();
        ProtectedEnvironment rootEnv = goldenDbService.getEnvironmentById(backupResource.getRootUuid());
        ProtectedResource resource = goldenDbService.getResourceById(backupResource.getUuid());
        List<Endpoint> result = new LinkedList<>();
        List<Endpoint> supplyAgent = getAgents(rootEnv);
        List<Endpoint> supplyAgent1 = getAgents(resource);
        result.addAll(supplyAgent1);
        result.addAll(supplyAgent);

        return result;
    }

    @Override
    public boolean applicable(AgentSelectParam agentSelectParam) {
        return ResourceSubTypeEnum.GOLDENDB_CLUSETER_INSTANCE.equalsSubType(
            agentSelectParam.getResource().getSubType());
    }

    private Endpoint getAgentEndpoint(ProtectedResource agentEnv) {
        return new Endpoint(agentEnv.getUuid(), agentEnv.getEndpoint(),
            Optional.ofNullable(agentEnv.getPort()).orElse(0));
    }

    private List<Endpoint> getAgents(ProtectedResource child) {
        List<ProtectedResource> resourceList = child.getDependencies().get(DatabaseConstants.AGENTS);
        return resourceList.stream()
            .flatMap(StreamUtil.match(ProtectedEnvironment.class))
            .map(this::getAgentEndpoint)
            .collect(Collectors.toList());
    }
}
