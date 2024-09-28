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
package openbackup.tdsql.resources.access.provider;

import openbackup.data.access.framework.agent.DataBaseAgentSelector;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.StreamUtil;
import openbackup.tdsql.resources.access.service.TdsqlService;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.LinkedList;
import java.util.List;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * 功能描述
 *
 */
@Slf4j
@Component
public class TdsqlAgentProvider extends DataBaseAgentSelector {
    private final TdsqlService tdsqlService;

    public TdsqlAgentProvider(TdsqlService tdsqlService) {
        this.tdsqlService = tdsqlService;
    }

    @Override
    public List<Endpoint> getSelectedAgents(AgentSelectParam agentSelectParam) {
        ProtectedResource backupResource = agentSelectParam.getResource();
        ProtectedEnvironment rootEnv = tdsqlService.getEnvironmentById(backupResource.getRootUuid());
        ProtectedResource resource = tdsqlService.getResourceById(backupResource.getUuid());
        List<Endpoint> result = new LinkedList<>();
        List<Endpoint> supplyAgent = getAgents(rootEnv);
        List<Endpoint> supplyAgent1 = getAgents(resource);
        result.addAll(supplyAgent1);
        result.addAll(supplyAgent);
        return removeDuplicateAgent(result);
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

    private List<Endpoint> removeDuplicateAgent(List<Endpoint> agentList) {
        for (int head = 0; head < agentList.size(); head++) {
            Endpoint agentHead = agentList.get(head);
            for (int tail = agentList.size() - 1; tail > head; tail--) {
                Endpoint agentTail = agentList.get(tail);
                if (agentHead.getId().equals(agentTail.getId()) && agentHead.getIp().equals(agentTail.getIp())
                    && agentHead.getPort() == agentTail.getPort()) {
                    agentList.remove(tail);
                }
            }
        }
        return agentList;
    }
}
