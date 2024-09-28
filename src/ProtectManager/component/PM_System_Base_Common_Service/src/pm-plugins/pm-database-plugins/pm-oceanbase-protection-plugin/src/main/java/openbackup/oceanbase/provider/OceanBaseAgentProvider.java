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
package openbackup.oceanbase.provider;

import openbackup.data.access.framework.agent.DataBaseAgentSelector;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.oceanbase.common.util.OceanBaseUtils;
import openbackup.oceanbase.service.OceanBaseService;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.List;
import java.util.Objects;

/**
 * 功能描述
 *
 */
@Slf4j
@Component
public class OceanBaseAgentProvider extends DataBaseAgentSelector {
    private final OceanBaseService oceanBaseService;

    public OceanBaseAgentProvider(OceanBaseService oceanBaseService) {
        this.oceanBaseService = oceanBaseService;
    }

    @Override
    public List<Endpoint> getSelectedAgents(AgentSelectParam agentSelectParam) {
        if (Objects.equals(agentSelectParam.getJobType(), JobTypeEnum.BACKUP.getValue())) {
            ProtectedResource resource = agentSelectParam.getResource();
            ProtectedEnvironment clusterEnv = oceanBaseService.getEnvironmentById(resource.getRootUuid());
            return OceanBaseUtils.supplyAgent(clusterEnv, oceanBaseService);
        } else if (Objects.equals(agentSelectParam.getJobType(), JobTypeEnum.RESTORE.getValue())) {
            ProtectedResource resource = agentSelectParam.getResource();
            ProtectedEnvironment clusterEnv = oceanBaseService.getEnvironmentById(resource.getRootUuid());
            return OceanBaseUtils.supplyAgentWithSingleClient(clusterEnv, oceanBaseService);
        } else {
            return super.getSelectedAgents(agentSelectParam);
        }
    }

    @Override
    public boolean applicable(AgentSelectParam agentSelectParam) {
        return ResourceSubTypeEnum.OCEAN_BASE_CLUSTER.getType().equals(agentSelectParam.getResource().getSubType())
            || ResourceSubTypeEnum.OCEAN_BASE_TENANT.getType().equals(agentSelectParam.getResource().getSubType());
    }
}
