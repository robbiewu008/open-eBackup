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
package openbackup.openstack.protection.access.provider;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.CheckReport;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;
import org.mockito.Mockito;

import java.util.HashMap;
import java.util.List;

import static org.mockito.ArgumentMatchers.anyList;

/**
 * 功能描述: test OpenstackConnectionChecker
 *
 */
public class OpenstackConnectionCheckerTest {
    private static OpenstackConnectionChecker openstackConnectionChecker;
    private static final ResourceService resourceService = Mockito.mock(ResourceService.class);
    private static final AgentUnifiedService agentUnifiedService = Mockito.mock(AgentUnifiedService.class);
    private static final ProtectedEnvironmentRetrievalsService protectedEnvironmentRetrievalsService = Mockito.mock(
        ProtectedEnvironmentRetrievalsService.class);

    @BeforeClass
    public static void init() {
        openstackConnectionChecker = new OpenstackConnectionChecker(protectedEnvironmentRetrievalsService,
            agentUnifiedService, resourceService);
    }

    /**
     * 测试场景：applicable匹配成功 <br/>
     * 前置条件：传入资源信息subtype类型为OpenStackContainer <br/>
     * 检查点：返回True
     */
    @Test
    public void test_applicable_success() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(ResourceSubTypeEnum.OPENSTACK_CONTAINER.getType());
        boolean applicable = openstackConnectionChecker.applicable(protectedResource);
        Assert.assertTrue(applicable);
    }

    /**
     * 测试场景：校验生成连通性检查结果成功 <br/>
     * 前置条件：参数补充正确 <br/>
     * 检查点：连通性检查结果有返回且无异常
     */
    @Test
    @Ignore
    public void test_generate_check_result_success() {
        AgentBaseDto agentBaseDto1 = new AgentBaseDto();
        agentBaseDto1.setErrorCode(String.valueOf(ActionResult.SUCCESS_CODE));
        ProtectedResource resource = mockProtectedResource();
        Mockito.when(agentUnifiedService.checkApplication(resource, resource.getEnvironment()))
            .thenReturn(agentBaseDto1);
        Mockito.doNothing().when(resourceService).updateSourceDirectly(anyList());
        CheckResult<Object> checkResult = openstackConnectionChecker.generateCheckResult(resource);
        Assert.assertEquals(ActionResult.SUCCESS_CODE, checkResult.getResults().getCode());
    }

    /**
     * 测试场景：校验生成连通性检查结果失败 <br/>
     * 前置条件：检查结果为失败 <br/>
     * 检查点：连通性检查结果有返回且返回码为错误码
     */
    @Test
    @Ignore
    public void test_generate_check_result_fail() {
        AgentBaseDto agentBaseDto1 = new AgentBaseDto();
        agentBaseDto1.setErrorCode("-1");
        ProtectedResource resource = mockProtectedResource();
        Mockito.when(agentUnifiedService.checkApplication(resource, resource.getEnvironment()))
            .thenReturn(agentBaseDto1);
        Mockito.doNothing().when(resourceService).updateSourceDirectly(anyList());
        CheckResult<Object> checkResult = openstackConnectionChecker.generateCheckResult(resource);
        Assert.assertEquals(CommonErrorCode.AGENT_NETWORK_ERROR, checkResult.getResults().getCode());
    }

    /**
     * 测试场景：检查当环境离线后生成的结果 <br/>
     * 前置条件：检查结果为失败 <br/>
     * 检查点：连通性检查结果有返回且返回码为错误码
     */
    @Test
    @Ignore
    public void test_generate_check_result_when_agent_is_offline() {
        AgentBaseDto agentBaseDto = new AgentBaseDto();
        agentBaseDto.setErrorCode(String.valueOf(CommonErrorCode.AGENT_NETWORK_ERROR));
        ProtectedResource resource = mockProtectedResource();
        ProtectedEnvironment environment = MockFactory.mockEnvironment();
        environment.setLinkStatus(LinkStatusEnum.OFFLINE.getStatus().toString());
        resource.setEnvironment(environment);
        CheckResult<Object> checkResult = openstackConnectionChecker.generateCheckResult(resource);
        Assert.assertEquals(CommonErrorCode.AGENT_NETWORK_ERROR, checkResult.getResults().getCode());
    }

    /**
     * 测试场景：校验连通性报告成功 <br/>
     * 前置条件：连通性检查报告中有成功的结果 <br/>
     * 检查点：校验连通性报告有返回且无异常
     */
    @Test
    public void test_collect_action_results_success() {
        List<CheckReport<Object>> checkReports = MockFactory.buildCheckReport();
        List<ActionResult> resultList = openstackConnectionChecker.collectActionResults(checkReports, new HashMap<>());
        Assert.assertEquals(1, resultList.size());
    }

    private ProtectedResource mockProtectedResource() {
        ProtectedResource resource = new ProtectedResource();
        resource.setEnvironment(new ProtectedEnvironment());
        resource.setUuid("envId");
        return resource;
    }
}
