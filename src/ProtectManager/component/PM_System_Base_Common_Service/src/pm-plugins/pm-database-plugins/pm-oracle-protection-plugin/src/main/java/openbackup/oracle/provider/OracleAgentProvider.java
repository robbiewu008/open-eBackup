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
package openbackup.oracle.provider;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.agent.DataBaseAgentSelector;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.oracle.service.OracleBaseService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.List;
import java.util.stream.Collectors;

/**
 * oracle的插件选择
 *
 */
@Slf4j
@Component
public class OracleAgentProvider extends DataBaseAgentSelector {
    private final OracleBaseService oracleBaseService;

    public OracleAgentProvider(OracleBaseService oracleBaseService) {
        this.oracleBaseService = oracleBaseService;
    }

    @Override
    public List<Endpoint> getSelectedAgents(AgentSelectParam agentSelectParam) {
        String parentUuid = agentSelectParam.getResource().getParentUuid();
        if (ResourceSubTypeEnum.ORACLE_PDB.getType().equals(agentSelectParam.getResource().getSubType())) {
            ProtectedResource parentResource = oracleBaseService.getResource(parentUuid);
            return getSelectedAgentsForSingleAndCluster(parentResource.getSubType(), parentResource.getParentUuid());
        } else {
            return getSelectedAgentsForSingleAndCluster(agentSelectParam.getResource().getSubType(), parentUuid);
        }
    }

    private List<Endpoint> getSelectedAgentsForSingleAndCluster(String subType, String parentUuid) {
        // 如果是数据库资源，根据数据库对应的单实例对应的Agent信息，设置到备份对象中
        if (ResourceSubTypeEnum.ORACLE.getType().equals(subType)) {
            // 获取单实例对应的Agent信息
            ProtectedEnvironment agentEnv = oracleBaseService.getAgentBySingleInstanceUuid(parentUuid);
            // 将Agent信息，放置到备份对象中
            Endpoint agentEndpoint = oracleBaseService.getAgentEndpoint(agentEnv);
            List<Endpoint> endpointList = new ArrayList<>();
            endpointList.add(agentEndpoint);
            return endpointList;
        }
        // 针对集群，设置Agents信息
        List<Endpoint> endpointList = new ArrayList<>();
        // 从dependency里，获取集群实例下面的所有子实例
        List<ProtectedEnvironment> agents =
                oracleBaseService
                        .getEnvironmentById(parentUuid)
                        .getDependencies()
                        .get(DatabaseConstants.AGENTS)
                        .stream()
                        .filter(resource -> resource instanceof ProtectedEnvironment)
                        .map(resource -> (ProtectedEnvironment) resource)
                        .collect(Collectors.toList());
        for (ProtectedEnvironment agent : agents) {
            endpointList.add(oracleBaseService.getAgentEndpoint(agent));
        }
        return endpointList;
    }

    @Override
    public boolean applicable(AgentSelectParam agentSelectParam) {
        return ResourceSubTypeEnum.ORACLE.equalsSubType(agentSelectParam.getResource().getSubType())
                || ResourceSubTypeEnum.ORACLE_CLUSTER.equalsSubType(agentSelectParam.getResource().getSubType())
                || ResourceSubTypeEnum.ORACLE_PDB.equalsSubType(agentSelectParam.getResource().getSubType());
    }
}
