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
package openbackup.access.framework.resource.service;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.access.framework.resource.service.provider.UnifiedResourceConnectionChecker;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.common.constants.CommonErrorCode;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;

/**
 * UnifiedResourceConnectionChecker测试
 *
 */
public class UnifiedResourceConnectionCheckerTest {
    /**
     * 用例场景：agent check application结果转化
     * 前置条件：无
     * 检查点： 根据返回的值进行转化，覆盖errorMessage的各种场景。
     */
    @Test
    public void generate_check_result_from_agent() {
        AgentUnifiedService agentUnifiedService = Mockito.mock(AgentUnifiedService.class);
        ProtectedResource resource = new ProtectedResource();
        resource.setEnvironment(new ProtectedEnvironment());
        AgentBaseDto agentBaseDto1 = new AgentBaseDto();
        agentBaseDto1.setErrorCode("0");
        AgentBaseDto agentBaseDto2 = new AgentBaseDto();
        agentBaseDto2.setErrorCode("22");
        AgentBaseDto agentBaseDto3 = new AgentBaseDto();
        agentBaseDto3.setErrorCode("22");
        agentBaseDto3.setErrorMessage(
            "{\n" + "\"code\": 1677929219,\n" + "\"bodyErr\": \"\",\n" + "\"message\": \"" + "} ");
        AgentBaseDto agentBaseDto4 = new AgentBaseDto();
        agentBaseDto4.setErrorCode("22");
        agentBaseDto4.setErrorMessage("{\n" + "\"code\": 55555,\n" + "\"bodyErr\": \"66666\",\n" + "\"message\": \"" + "} ");
        AgentBaseDto agentBaseDto5 = new AgentBaseDto();
        agentBaseDto5.setErrorCode("22");
        agentBaseDto5.setErrorMessage(
            "{\n" + "\"code2\": 55555,\n" + "\"bodyErr2\": \"\",\n" + "\"message2\": \"" + "} ");
        AgentBaseDto agentBaseDto6 = new AgentBaseDto();
        agentBaseDto6.setErrorCode("22");
        agentBaseDto6.setErrorMessage("aaa");
        Mockito.when(agentUnifiedService.checkApplication(resource, resource.getEnvironment()))
            .thenReturn(agentBaseDto1)
            .thenReturn(agentBaseDto2)
            .thenReturn(agentBaseDto3)
            .thenReturn(agentBaseDto4)
            .thenReturn(agentBaseDto5)
            .thenReturn(agentBaseDto6);

        UnifiedResourceConnectionChecker checker = new UnifiedResourceConnectionChecker(
            Mockito.mock(ProtectedEnvironmentRetrievalsService.class), agentUnifiedService);
        CheckResult<Object> result1 = checker.generateCheckResult(resource);
        Assert.assertEquals(result1.getResults().getCode(), 0L);
        CheckResult<Object> result2 = checker.generateCheckResult(resource);
        Assert.assertEquals(result2.getResults().getCode(), CommonErrorCode.AGENT_NETWORK_ERROR);
        CheckResult<Object> result3 = checker.generateCheckResult(resource);
        Assert.assertEquals(result3.getResults().getCode(), CommonErrorCode.AGENT_NETWORK_ERROR);
        CheckResult<Object> result4 = checker.generateCheckResult(resource);
        Assert.assertEquals(result4.getResults().getCode(), 66666L);
        CheckResult<Object> result5 = checker.generateCheckResult(resource);
        Assert.assertEquals(result5.getResults().getCode(), CommonErrorCode.AGENT_NETWORK_ERROR);
        CheckResult<Object> result6 = checker.generateCheckResult(resource);
        Assert.assertEquals(result6.getResults().getCode(), CommonErrorCode.AGENT_NETWORK_ERROR);
    }
}
