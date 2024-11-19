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
package openbackup.dameng.protection.access.provider;

import lombok.AllArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import openbackup.dameng.protection.access.service.DamengService;
import openbackup.data.access.framework.agent.DataBaseAgentSelector;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Component;

import java.util.List;

/**
 * Dameng的插件选择
 *
 */
@Slf4j
@Component
@AllArgsConstructor
public class DamengAgentProvider extends DataBaseAgentSelector {
    private final DamengService damengService;

    @Override
    public List<Endpoint> getSelectedAgents(AgentSelectParam agentSelectParam) {
        if (ResourceSubTypeEnum.DAMENG_SINGLE_NODE.getType().equals(agentSelectParam.getResource().getSubType())) {
            return super.getSelectedAgents(agentSelectParam);
        }
        return damengService.getEndpointList(agentSelectParam.getResource().getUuid());
    }

    @Override
    public boolean applicable(AgentSelectParam agentSelectParam) {
        String subType = agentSelectParam.getResource().getSubType();
        return ResourceSubTypeEnum.DAMENG_SINGLE_NODE.getType().equals(subType)
            || ResourceSubTypeEnum.DAMENG_CLUSTER.getType().equals(subType);
    }
}
