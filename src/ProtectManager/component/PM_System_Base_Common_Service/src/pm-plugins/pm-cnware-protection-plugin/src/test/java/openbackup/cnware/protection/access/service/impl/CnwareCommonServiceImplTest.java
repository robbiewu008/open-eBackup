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
package openbackup.cnware.protection.access.service.impl;

import static org.mockito.ArgumentMatchers.any;

import openbackup.cnware.protection.access.mock.CnwareMockUtil;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;

import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.json.JsonUtil;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;
import java.util.List;
import java.util.Optional;

/**
 * 功能描述
 *
 */
@RunWith(PowerMockRunner.class)
public class CnwareCommonServiceImplTest {
    private CnwareCommonServiceImpl cnwareCommonServiceTest;
    @Mock
    private AgentUnifiedService mockAgentUnifiedService;
    @Mock
    private ResourceService mockResourceService;

    @Before
    public void setUp() {
        cnwareCommonServiceTest = new CnwareCommonServiceImpl(mockAgentUnifiedService, mockResourceService);
    }

    /**
     * 用例场景：检查环境域名信息
     * 前置条件：域名中含有非法字符
     * 检查点：若域名中含有非法字符则应抛出异常
     */
    @Test
    public void test_check_envrionment_name_should_throw_exception() {
        Assert.assertThrows(LegoCheckedException.class, () -> cnwareCommonServiceTest.checkEnvName("dsaf%daf"));
    }

    /**
     * 用例场景：查询集群信息
     * 前置条件：无
     * 检查点：未能查询到集群信息，返回空值
     */
    @Test
    public void test_query_cluster_info_should_return_null() {
        PowerMockito.when(mockAgentUnifiedService.getClusterInfo(any(), any())).thenReturn(null);
        AppEnvResponse queryClusterInfo = cnwareCommonServiceTest.queryClusterInfo(
            CnwareMockUtil.mockScanEnvironment(), CnwareMockUtil.mockEnvironment());
        Assert.assertNull(queryClusterInfo);
    }

    /**
     * 用例场景：根据资源uuid查询环境信息
     * 前置条件：无
     * 检查点：返回查询到的环境信息
     */
    @Test
    public void test_get_environment_by_id_should_return_environment() {
        PowerMockito.when(mockResourceService.getBasicResourceById(any())).thenReturn(
            Optional.of(CnwareMockUtil.mockEnvironment()));
        ProtectedEnvironment protectedEnvironment = cnwareCommonServiceTest.getEnvironmentById("123");
        Assert.assertEquals(protectedEnvironment.getEndpoint(), "8.40.162.56");
    }

    /**
     * 用例场景：检查agent资源连通性
     * 前置条件：无
     * 检查点：返回错误信息，抛出异常
     */
    @Test
    public void test_check_connectivity_should_throw_exception_when_get_error() {
        ActionResult actionResult = new ActionResult();
        actionResult.setCode(ActionResult.SUCCESS_CODE);
        actionResult.setMessage("SUCCESS_MSG");
        List<ProtectedEnvironment> agentEnvList = new ArrayList<>();
        agentEnvList.add(CnwareMockUtil.mockScanEnvironment());
        AgentBaseDto agentBaseDto = new AgentBaseDto();
        agentBaseDto.setErrorCode("1");
        agentBaseDto.setErrorMessage(JsonUtil.json(actionResult));
        agentBaseDto.isAgentBaseDtoReturnSuccess();
        PowerMockito.when(mockAgentUnifiedService.checkApplication(any(), any())).thenReturn(agentBaseDto);
        Assert.assertThrows(NumberFormatException.class,
            () -> cnwareCommonServiceTest.checkConnectivity(CnwareMockUtil.mockEnvironment(), agentEnvList));
    }

    /**
     * 用例场景：检查agent资源连通性
     * 前置条件：无
     * 检查点：连通性校验成功
     */
    @Test
    public void test_check_connectivity_should_return_when_success() {
        List<ProtectedEnvironment> agentEnvList = new ArrayList<>();
        agentEnvList.add(CnwareMockUtil.mockScanEnvironment());
        AgentBaseDto agentBaseDto = new AgentBaseDto();
        agentBaseDto.setErrorCode("0");
        agentBaseDto.setErrorMessage("");
        agentBaseDto.isAgentBaseDtoReturnSuccess();
        PowerMockito.when(mockAgentUnifiedService.checkApplication(any(), any())).thenReturn(agentBaseDto);
        cnwareCommonServiceTest.checkConnectivity(CnwareMockUtil.mockEnvironment(), agentEnvList);
        Assert.assertNotNull(agentBaseDto);
    }


    /**
     * 用例场景：检查agent资源连通性
     * 前置条件：Agent网络异常
     * 检查点：连通性校验失败，抛出异常
     */
    @Test
    public void test_check_connectivity_should_throw_exception_when_agent_network_error() {
        List<ProtectedEnvironment> agentEnvList = new ArrayList<>();
        agentEnvList.add(CnwareMockUtil.mockScanEnvironment());
        AgentBaseDto agentBaseDto = new AgentBaseDto();
        agentBaseDto.setErrorCode("1");
        agentBaseDto.setErrorMessage("");
        agentBaseDto.isAgentBaseDtoReturnSuccess();
        PowerMockito.when(mockAgentUnifiedService.checkApplication(any(), any())).thenReturn(agentBaseDto);
        Assert.assertThrows(LegoCheckedException.class,
            () -> cnwareCommonServiceTest.checkConnectivity(CnwareMockUtil.mockEnvironment(), agentEnvList));
    }
}
