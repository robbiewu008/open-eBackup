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
package openbackup.opengauss.resources.access.provider;

import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import openbackup.opengauss.resources.access.service.OpenGaussAgentService;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.HashMap;

/**
 * {@link OpenGaussAgentProvider 测试类}
 *
 */
public class OpenGaussAgentProviderTest {
    private final OpenGaussAgentService openGaussAgentService = PowerMockito.mock(OpenGaussAgentService.class);
    private final OpenGaussAgentProvider agentProvider = new OpenGaussAgentProvider(openGaussAgentService);

    /**
     * 用例场景：填充agent
     * 前置条件：资源类型为OpenGauss-instance
     * 检查点：填充成功
     */
    @Test
    public void select_agent_success() {
        AgentSelectParam agentSelectParam = getAgentSelectParam();
        PowerMockito.when(openGaussAgentService.getAgentEndpoint(anyString())).thenReturn(new ArrayList<>());
        Assert.assertNotNull(agentProvider.getSelectedAgents(agentSelectParam));
    }

    /**
     * 用例场景：过滤OpenGauss类型
     * 前置条件：资源类型为OpenGauss-instance
     * 检查点：类过滤检查返回成功
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(agentProvider.applicable(getAgentSelectParam()));
    }

    private AgentSelectParam getAgentSelectParam() {
        ProtectedResource resource = new ProtectedResource();
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setUuid("env-uuid");
        resource.setEnvironment(environment);
        resource.setSubType(ResourceSubTypeEnum.OPENGAUSS_INSTANCE.getType());
        return AgentSelectParam.builder()
            .resource(resource)
            .jobType(JobTypeEnum.BACKUP.getValue())
            .parameters(new HashMap<>())
            .build();
    }
}
