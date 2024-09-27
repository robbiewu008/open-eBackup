/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.gaussdbt.protection.access.provider.agent;

import openbackup.data.access.framework.agent.DataBaseAgentSelector;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.gaussdbt.protection.access.provider.service.GaussDBTSingleService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Component;

import java.util.List;

/**
 * GaussDBT单机资源查询agent主机的provider
 *
 * @author lWX776769
 * @version [DataBackup 1.5.0]
 * @since 2023/8/29
 */
@Component
public class GaussDBTSingleAgentProvider extends DataBaseAgentSelector {
    private final GaussDBTSingleService gaussDBTSingleService;

    public GaussDBTSingleAgentProvider(GaussDBTSingleService gaussDBTSingleService) {
        this.gaussDBTSingleService = gaussDBTSingleService;
    }

    @Override
    public List<Endpoint> getSelectedAgents(AgentSelectParam agentSelectParam) {
        return gaussDBTSingleService.getAgents(agentSelectParam.getResource());
    }

    @Override
    public boolean applicable(AgentSelectParam agentSelectParam) {
        return ResourceSubTypeEnum.GAUSSDBT_SINGLE.equalsSubType(agentSelectParam.getResource().getSubType());
    }
}
