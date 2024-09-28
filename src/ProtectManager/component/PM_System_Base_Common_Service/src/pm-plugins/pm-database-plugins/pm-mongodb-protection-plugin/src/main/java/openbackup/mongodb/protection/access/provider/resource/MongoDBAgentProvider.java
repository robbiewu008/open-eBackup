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
package openbackup.mongodb.protection.access.provider.resource;

import openbackup.data.access.framework.agent.DataBaseAgentSelector;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * MongoDB的插件选择
 *
 */
@Slf4j
@Component
public class MongoDBAgentProvider extends DataBaseAgentSelector {
    @Override
    public List<Endpoint> getSelectedAgents(AgentSelectParam agentSelectParam) {
        List<Endpoint> agents = super.getSelectedAgents(agentSelectParam);
        Set<String> uuid = new HashSet<>();
        return agents.stream().filter(endpoint -> uuid.add(endpoint.getId())).collect(Collectors.toList());
    }

    @Override
    public boolean applicable(AgentSelectParam agentSelectParam) {
        String subType = agentSelectParam.getResource().getSubType();
        return Arrays.asList(ResourceSubTypeEnum.MONGODB_CLUSTER.getType(),
            ResourceSubTypeEnum.MONGODB_SINGLE.getType()).contains(subType);
    }
}
