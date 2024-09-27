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
package openbackup.clickhouse.plugin.provider;

import openbackup.data.access.framework.agent.DataBaseAgentSelector;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;

/**
 * 功能描述
 *
 * @author c00826511
 * @since 2023-07-28
 */
@Slf4j
@Component
public class ClickHouseAgentProvider extends DataBaseAgentSelector {
    private final ResourceService resourceService;

    public ClickHouseAgentProvider(ResourceService resourceService) {
        this.resourceService = resourceService;
    }

    @Override
    public List<Endpoint> getSelectedAgents(AgentSelectParam agentSelectParam) {
        if (Objects.equals(agentSelectParam.getJobType(), JobTypeEnum.BACKUP.getValue())) {
            return getAgentsByRootUuid(agentSelectParam);
        } else if (Objects.equals(agentSelectParam.getJobType(), JobTypeEnum.RESTORE.getValue())) {
            return getAgents(agentSelectParam);
        } else if (Objects.equals(agentSelectParam.getJobType(), JobTypeEnum.RESOURCE_SCAN.getValue())) {
            ProtectedResource child = agentSelectParam.getResource();
            return getAgentsOfChild(child);
        } else {
            return super.getSelectedAgents(agentSelectParam);
        }
    }

    @Override
    public boolean applicable(AgentSelectParam agentSelectParam) {
        return ResourceSubTypeEnum.CLICK_HOUSE.equalsSubType(agentSelectParam.getResource().getSubType());
    }

    private Endpoint getAgentEndpoint(ProtectedResource agentEnv) {
        return new Endpoint(agentEnv.getUuid(), agentEnv.getEndpoint(),
            Optional.ofNullable(agentEnv.getPort()).orElse(0));
    }

    private List<Endpoint> getAgentsOfChild(ProtectedResource child) {
        List<ProtectedResource> agents = child.getDependencies().get(DatabaseConstants.AGENTS);
        // 针对集群，设置Agents信息
        List<Endpoint> endpointList = new ArrayList<>();
        for (ProtectedResource agent : agents) {
            endpointList.add(getAgentEndpoint(agent));
        }

        return endpointList;
    }

    private List<Endpoint> getAgents(AgentSelectParam agentSelectParam) {
        ProtectedResource clusterResource = agentSelectParam.getResource();
        Map<String, Endpoint> agents = getEndpointMap(clusterResource);
        return new ArrayList<>(agents.values());
    }

    private static Map<String, Endpoint> getEndpointMap(ProtectedResource clusterResource) {
        List<ProtectedResource> nodeResources = clusterResource.getDependencies().get(ResourceConstants.CHILDREN);

        // 遍历节点添加Agent，根据uuid去重
        Map<String, Endpoint> agents = new HashMap<>();
        for (ProtectedResource child : nodeResources) {
            ProtectedEnvironment env = child.getDependencies()
                .get(DatabaseConstants.AGENTS)
                .stream()
                .filter(resource -> resource instanceof ProtectedEnvironment)
                .map(resource -> (ProtectedEnvironment) resource)
                .findFirst()
                .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "No agent online"));
            // 去除相同Agent
            if (!agents.containsKey(env.getUuid())) {
                agents.put(env.getUuid(), new Endpoint(env.getUuid(), env.getEndpoint(), env.getPort()));
            }
        }
        return agents;
    }

    private List<Endpoint> getAgentsByRootUuid(AgentSelectParam agentSelectParam) {
        String rootUuid = agentSelectParam.getResource().getRootUuid();
        ProtectedResource clusterResource = resourceService.getResourceById(rootUuid)
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST));

        Map<String, Endpoint> agents = getEndpointMap(clusterResource);
        return new ArrayList<>(agents.values());
    }
}
