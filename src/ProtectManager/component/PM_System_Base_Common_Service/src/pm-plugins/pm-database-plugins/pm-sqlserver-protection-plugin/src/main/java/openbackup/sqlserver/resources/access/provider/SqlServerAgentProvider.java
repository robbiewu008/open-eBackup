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
package openbackup.sqlserver.resources.access.provider;

import lombok.AllArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.agent.DataBaseAgentSelector;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.sqlserver.protection.service.SqlServerBaseService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.List;

/**
 * SqlServer的agent配置
 *
 */
@Slf4j
@Component
@AllArgsConstructor
public class SqlServerAgentProvider extends DataBaseAgentSelector {
    private final SqlServerBaseService sqlServerBaseService;

    @Override
    public List<Endpoint> getSelectedAgents(AgentSelectParam agentSelectParam) {
        return sqlServerBaseService.convertNodeListToAgents(agentSelectParam.getResource().getUuid());
    }

    @Override
    public boolean applicable(AgentSelectParam agentSelectParam) {
        String subType = agentSelectParam.getResource().getSubType();
        return Arrays.asList(ResourceSubTypeEnum.SQL_SERVER_INSTANCE.getType(),
            ResourceSubTypeEnum.SQL_SERVER_DATABASE.getType(), ResourceSubTypeEnum.SQL_SERVER_ALWAYS_ON.getType(),
            ResourceSubTypeEnum.SQL_SERVER_CLUSTER_INSTANCE.getType()).contains(subType);
    }
}
