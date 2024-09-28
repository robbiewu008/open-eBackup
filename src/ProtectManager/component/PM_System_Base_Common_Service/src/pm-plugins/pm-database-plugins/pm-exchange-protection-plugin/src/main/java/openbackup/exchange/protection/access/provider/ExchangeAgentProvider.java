/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
package openbackup.exchange.protection.access.provider;

import openbackup.data.access.framework.agent.DataBaseAgentSelector;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.exchange.protection.access.service.ExchangeService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Optional;

/**
 * Exchange Agent Provider
 *
 */
@Slf4j
@Component
public class ExchangeAgentProvider extends DataBaseAgentSelector {
    private final ResourceService resourceService;

    private final ExchangeService exchangeService;

    public ExchangeAgentProvider(ResourceService resourceService, ExchangeService exchangeService) {
        this.resourceService = resourceService;
        this.exchangeService = exchangeService;
    }

    @Override
    public boolean applicable(AgentSelectParam agentSelectParam) {
        String subType = agentSelectParam.getResource().getSubType();
        return Arrays.asList(ResourceSubTypeEnum.EXCHANGE_MAILBOX.getType(),
            ResourceSubTypeEnum.EXCHANGE_DATABASE.getType(), ResourceSubTypeEnum.EXCHANGE_GROUP.getType(),
            ResourceSubTypeEnum.EXCHANGE_SINGLE_NODE.getType()).contains(subType);
    }

    @Override
    public List<Endpoint> getSelectedAgents(AgentSelectParam agentSelectParam) {
        List<Endpoint> agents = new ArrayList<>();
        String subType = agentSelectParam.getResource().getSubType();
        String jobType = agentSelectParam.getJobType();
        if (ResourceSubTypeEnum.EXCHANGE_GROUP.equalsSubType(subType)
            || ResourceSubTypeEnum.EXCHANGE_SINGLE_NODE.equalsSubType(subType)) {
            getAgentsByExtendInfo(agentSelectParam.getResource().getExtendInfoByKey("agentUuid"), agents);
        }
        if (ResourceSubTypeEnum.EXCHANGE_DATABASE.equalsSubType(subType)
            || ResourceSubTypeEnum.EXCHANGE_MAILBOX.equalsSubType(subType)) {
            ProtectedEnvironment environment = exchangeService.getEnvironmentById(
                agentSelectParam.getResource().getEnvironment().getUuid());
            getAgentsByExtendInfo(environment.getExtendInfoByKey("agentUuid"), agents);
        }
        log.info("exchange resource: {}, task type: {}, get agent: {}", agentSelectParam.getResource().getUuid(),
            jobType, agents);
        return agents;
    }

    private void getAgentsByExtendInfo(String agentsStr, List<Endpoint> agents) {
        String[] agentUuids = agentsStr.split(";");
        Arrays.stream(agentUuids).forEach(uuid -> {
            ProtectedResource protectedResource = resourceService.getResourceById(uuid).orElseThrow(
                () -> new LegoCheckedException(CommonErrorCode.AGENT_NOT_EXIST, "No agent found"));
            agents.add(getAgentEndpoint(protectedResource));
        });
    }

    private Endpoint getAgentEndpoint(ProtectedResource agentEnv) {
        return new Endpoint(agentEnv.getUuid(), agentEnv.getEndpoint(),
            Optional.ofNullable(agentEnv.getPort()).orElse(0));
    }
}
