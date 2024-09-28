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
package openbackup.tidb.resources.access.provider;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.tidb.resources.access.service.TidbService;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;

/**
 * 功能描述
 *
 */
@RunWith(PowerMockRunner.class)
public class TidbAgentProviderTest {
    @Mock
    private TidbService tidbService;

    private TidbAgentProvider tidbAgentProvider;

    @Before
    public void init() {
        tidbAgentProvider = new TidbAgentProvider(tidbService);
    }

    /**
     * 用例场景：备份场景查询agent成功
     * 前置条件：无
     * 检查点: 识别成功
     */
    @Test
    public void backup_select_success() {
        ProtectedResource resource = new ProtectedResource();
        resource.setRootUuid("1123");
        PowerMockito.when(tidbService.getResourceByCondition(anyString())).thenReturn(new ProtectedResource());
        PowerMockito.when(tidbService.getTaskEndpoint(any())).thenReturn(new ArrayList<>());

        AgentSelectParam agentSelectParam = AgentSelectParam.builder()
            .resource(resource)
            .jobType(JobTypeEnum.BACKUP.getValue())
            .build();

        tidbAgentProvider.getSelectedAgents(agentSelectParam);
        Mockito.verify(tidbService).getResourceByCondition(anyString());
        Mockito.verify(tidbService).getTaskEndpoint(any());
    }

    /**
     * 用例场景：过滤
     * 前置条件：无
     * 检查点: 识别成功
     */
    @Test
    public void applicable_success() {
        ProtectedResource nodeResource1 = new ProtectedResource();
        nodeResource1.setSubType(ResourceSubTypeEnum.TIDB_TABLE.getType());
        AgentSelectParam agentSelectParam = AgentSelectParam.builder().resource(nodeResource1).build();
        boolean applicable = tidbAgentProvider.applicable(agentSelectParam);
        Assert.assertTrue(applicable);
    }
}
