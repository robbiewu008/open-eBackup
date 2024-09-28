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
package openbackup.postgre.protection.access.provider.agent;

import openbackup.data.access.framework.agent.DataBaseAgentSelector;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.postgre.protection.access.service.PostgreInstanceService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.List;

/**
 * Postgre资源查询agent主机的provider
 *
 */
@Component
public class PostgreAgentProvider extends DataBaseAgentSelector {
    private final PostgreInstanceService postgreInstanceService;

    public PostgreAgentProvider(PostgreInstanceService postgreInstanceService) {
        this.postgreInstanceService = postgreInstanceService;
    }

    @Override
    public List<Endpoint> getSelectedAgents(AgentSelectParam agentSelectParam) {
        return postgreInstanceService.getAgentsByInstanceResource(agentSelectParam.getResource());
    }

    @Override
    public boolean applicable(AgentSelectParam agentSelectParam) {
        return Arrays.asList(ResourceSubTypeEnum.POSTGRE_INSTANCE.getType(),
            ResourceSubTypeEnum.POSTGRE_CLUSTER_INSTANCE.getType())
            .contains(agentSelectParam.getResource().getSubType());
    }
}
