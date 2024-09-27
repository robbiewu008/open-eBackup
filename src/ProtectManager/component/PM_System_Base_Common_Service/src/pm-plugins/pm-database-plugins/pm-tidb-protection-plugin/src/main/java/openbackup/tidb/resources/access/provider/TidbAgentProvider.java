/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.tidb.resources.access.provider;

import openbackup.data.access.framework.agent.DataBaseAgentSelector;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.tidb.resources.access.service.TidbService;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.List;

/**
 * 功能描述
 *
 * @author c00826511
 * @since 2023-07-28
 */
@Slf4j
@Component
public class TidbAgentProvider extends DataBaseAgentSelector {
    private final TidbService tidbService;

    public TidbAgentProvider(TidbService tidbService) {
        this.tidbService = tidbService;
    }

    @Override
    public List<Endpoint> getSelectedAgents(AgentSelectParam agentSelectParam) {
        String clusterUuid = agentSelectParam.getResource().getRootUuid();
        ProtectedResource protectedResource = tidbService.getResourceByCondition(clusterUuid);
        return tidbService.getTaskEndpoint(protectedResource);
    }

    @Override
    public boolean applicable(AgentSelectParam agentSelectParam) {
        return ResourceSubTypeEnum.TIDB_CLUSTER.getType().equals(agentSelectParam.getResource().getSubType())
            || ResourceSubTypeEnum.TIDB_DATABASE.getType().equals(agentSelectParam.getResource().getSubType())
            || ResourceSubTypeEnum.TIDB_TABLE.getType().equals(agentSelectParam.getResource().getSubType());
    }
}
