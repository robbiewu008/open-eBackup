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
package openbackup.exchange.protection.access.service.impl;

import static org.assertj.core.api.Assertions.assertThatThrownBy;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertThrows;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.when;

import openbackup.data.access.client.sdk.api.framework.agent.AgentUnifiedRestApi;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.exchange.protection.access.constant.ExchangeConstant;
import com.huawei.oceanprotect.job.sdk.JobService;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;

import org.junit.Assert;
import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.extension.ExtendWith;
import org.mockito.ArgumentMatchers;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.junit.jupiter.MockitoExtension;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

@ExtendWith(MockitoExtension.class)
class ExchangeServiceImplTest {

    @Mock
    private ResourceService mockResourceService;

    @Mock
    private AgentUnifiedRestApi mockAgentUnifiedRestApi;

    @Mock
    private AgentUnifiedService mockAgentUnifiedService;

    @Mock
    private CopyRestApi copyRestApi;

    @Mock
    private JobService jobService;

    @InjectMocks
    private ExchangeServiceImpl exchangeServiceImplUnderTest;

    @Test
    public void testGetEnvironmentById() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setEndpoint("8.8.8.8");
        when(mockResourceService.getResourceById("666")).thenReturn(Optional.of(environment));

        // Run the test
        ProtectedEnvironment result = exchangeServiceImplUnderTest.getEnvironmentById("666");

        // Verify the results
        assertEquals(result.getEndpoint(), "8.8.8.8");
    }

    @Test
    void testGetEnvironmentById_ResourceServiceReturnsAbsent() {
        // Setup
        when(mockResourceService.getResourceById("agentId")).thenReturn(Optional.empty());

        // Run the test
        assertThatThrownBy(() -> exchangeServiceImplUnderTest.getEnvironmentById("agentId"))
            .isInstanceOf(LegoCheckedException.class);
    }

    @Test
    void testGetResourceById() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setEndpoint("8.8.8.8");
        when(mockResourceService.getResourceById("089a77e6-2029-4be7-b606-17f2515bf882"))
            .thenReturn(Optional.of(protectedResource));

        // Run the test
        final ProtectedResource result =
            exchangeServiceImplUnderTest.getResourceById("089a77e6-2029-4be7-b606-17f2515bf882");

        // Verify the results
        assertEquals("8.8.8.8", result.getEndpoint());
    }

    @Test
    void testGetResourceById_ResourceServiceReturnsAbsent() {
        // Setup
        when(mockResourceService.getResourceById("122765b3-bd8e-4296-a696-135eba908881")).thenReturn(Optional.empty());

        // Run the test
        assertThatThrownBy(() -> exchangeServiceImplUnderTest.getResourceById("122765b3-bd8e-4296-a696-135eba908881"))
            .isInstanceOf(LegoCheckedException.class);
    }

    @Test
    void testCheckConnection() throws Exception {
        // Setup
        final ProtectedEnvironment environment = getEnvironment();

        // Configure ResourceService.getResourceById(...).
        ProtectedEnvironment agentEnvironment = new ProtectedEnvironment();
        agentEnvironment.setEndpoint("8.8.8.8");
        agentEnvironment.setPort(666);

        when(mockResourceService.getResourceById(any())).thenReturn(Optional.of(agentEnvironment));

        // Configure AgentUnifiedRestApi.check(...).
        final AgentBaseDto agentBaseDto = new AgentBaseDto();
        agentBaseDto.setErrorCode("0");
        agentBaseDto.setErrorMessage("errorMessage");
        when(mockAgentUnifiedService.check(anyString(), any(), any())).thenReturn(agentBaseDto);

        // Run the test
        exchangeServiceImplUnderTest.checkConnection(environment);
    }

    @Test
    void testQueryClusterInfo() {
        // Setup
        final ProtectedEnvironment environment = getEnvironment();
        ProtectedEnvironment agentEnvironment = new ProtectedEnvironment();
        agentEnvironment.setEndpoint("8.8.8.8");
        agentEnvironment.setPort(666);
        when(mockResourceService.getResourceById(any())).thenReturn(Optional.of(agentEnvironment));

        // Configure AgentUnifiedService.getClusterInfo(...).
        final AppEnvResponse appEnvResponse = new AppEnvResponse();
        appEnvResponse.setUuid("666");
        HashMap<String, String> map = new HashMap<>();
        map.put("dag_uuid", "");
        map.put("member_server_sum", "");
        appEnvResponse.setExtendInfo(map);
        when(mockAgentUnifiedService.getClusterInfo(any(), any())).thenReturn(appEnvResponse);

        // Run the test
        final AppEnvResponse result = exchangeServiceImplUnderTest.queryClusterInfo(environment, null);
        Assert.assertEquals(result.getUuid(), "666");
    }

    /**
     * 用例场景：测试扫描邮箱
     * 前置条件：无
     * 检查点：扫描成功
     */
    @Test
    void testScanMailbox() {
        final ProtectedEnvironment environment = getEnvironment();
        ProtectedResource agentEnvironment = new ProtectedResource();
        agentEnvironment.setEndpoint("8.8.8.8");
        agentEnvironment.setPort(666);
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        ProtectedResource mailbox = new ProtectedResource();
        Map<String, String> extendinfo = new HashMap<>();
        extendinfo.put("ExchangeGuid", "mail");
        mailbox.setExtendInfo(extendinfo);
        response.setRecords(Collections.singletonList(mailbox));
        when(mockAgentUnifiedService.getDetailPageList(any(), any(), any(), any())).thenReturn(response);
        List<ProtectedResource> mailboxes = exchangeServiceImplUnderTest.scanMailboxes(environment,
            agentEnvironment, new ProtectedResource());
        Assertions.assertFalse(mailboxes.isEmpty());
        Assertions.assertEquals("mail", mailboxes.get(0).getUuid());
    }

    @Test
    void testQueryClusterInfoDag() {
        // Setup
        final ProtectedEnvironment environment = getDagEnvironment();
        ProtectedEnvironment agentEnvironment = new ProtectedEnvironment();
        agentEnvironment.setEndpoint("8.8.8.8");
        agentEnvironment.setPort(666);
        when(mockResourceService.getResourceById(any())).thenReturn(Optional.of(agentEnvironment));

        // Configure AgentUnifiedService.getClusterInfo(...).
        final AppEnvResponse appEnvResponse = new AppEnvResponse();
        appEnvResponse.setUuid("666");
        HashMap<String, String> map = new HashMap<>();
        map.put("dag_uuid", "1234");
        map.put("member_server_sum", "2");
        appEnvResponse.setExtendInfo(map);
        when(mockAgentUnifiedService.getClusterInfo(any(), any())).thenReturn(appEnvResponse);

        // Run the test
        final AppEnvResponse result = exchangeServiceImplUnderTest.queryClusterInfo(environment, null);
        Assert.assertEquals(result.getUuid(), "666");

        ProtectedEnvironment dagEnvironmentWithOneAgent = getDagEnvironmentWithOneAgent();
        try {
            exchangeServiceImplUnderTest.queryClusterInfo(dagEnvironmentWithOneAgent, null);
        } catch (LegoCheckedException exception) {
            Assert.assertEquals(CommonErrorCode.SYSTEM_ERROR, exception.getErrorCode());
        }
    }

    @Test
    void testSingleConnectCheck_ResourceServiceReturnsAbsent() {
        // Setup
        final ProtectedEnvironment environment = getEnvironment();
        when(mockResourceService.getResourceById(any())).thenReturn(Optional.empty());

        // Run the test
        assertThatThrownBy(() -> exchangeServiceImplUnderTest.singleConnectCheck("agentId", environment))
            .isInstanceOf(LegoCheckedException.class);
    }

    @Test
    void testIsExistCopy() {
        // Setup
        String resourceId = "2615222a-1993-4f84-8be5-7c74dc335a6d";
        Map<String, Object> filter = new HashMap<>();
        filter.put(DatabaseConstants.COPY_GENERATED_BY, CopyGeneratedByEnum.BY_BACKUP.value());
        when(copyRestApi.queryLatestBackupCopy("2615222a-1993-4f84-8be5-7c74dc335a6d", null, filter)).thenReturn(null);

        // Run the test
        Assertions.assertFalse(exchangeServiceImplUnderTest.isExistCopy(resourceId));
    }

    /**
     * 用例场景：测试查询最新的副本
     * 前置条件：无
     * 检查点：查询成功
     */
    @Test
    void testIGetLatestCopy() {
        // Setup
        String resourceId = "2615222a-1993-4f84-8be5-7c74dc335a6d";
        Map<String, Object> filter = new HashMap<>();
        filter.put(DatabaseConstants.COPY_GENERATED_BY, CopyGeneratedByEnum.BY_BACKUP.value());
        when(copyRestApi.queryLatestBackupCopy("2615222a-1993-4f84-8be5-7c74dc335a6d", null, filter))
            .thenReturn(new Copy());

        // Run the test
        Assertions.assertTrue(exchangeServiceImplUnderTest.getLatestCopy(resourceId).isPresent());
    }

    /**
     * 用例场景：测试校验域名和用户名
     * 前置条件：无
     * 检查点：结果符合预期
     */
    @Test
    void testCheckDomainUser() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        Authentication auth = new Authentication();
        auth.setAuthKey("dddd");
        environment.setAuth(auth);
        try {
            exchangeServiceImplUnderTest.checkDomainUser(environment);
            Assertions.fail("domain user is invalid");
        } catch (LegoCheckedException ex) {
            Assertions.assertEquals(ex.getErrorCode(), CommonErrorCode.EXCHANGE_DOMAIN_USERNAME_INVALID);
        }
        auth.setAuthKey("DAG\\Administrator");
        exchangeServiceImplUnderTest.checkDomainUser(environment);
        auth.setAuthKey("Administrator@dag.com");
        exchangeServiceImplUnderTest.checkDomainUser(environment);
    }

    /**
     * 用例场景：测试校验任务最大并发数
     * 前置条件：无
     * 检查点：结果符合预期
     */
    @Test
    public void testCheckMaxConcurrentJobNumber() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(ExchangeConstant.MAX_CONCURRENT_JOB_NUMBER, "1");
        environment.setExtendInfo(extendInfo);
        exchangeServiceImplUnderTest.checkMaxConcurrentJobNumber(environment);
        extendInfo.put(ExchangeConstant.MAX_CONCURRENT_JOB_NUMBER, "30");
        assertThrows(LegoCheckedException.class,
            () -> exchangeServiceImplUnderTest.checkMaxConcurrentJobNumber(environment));
    }

    /**
     * 用例场景：测试删除资源时有任务运行，不允许删除
     * 前置条件：无
     * 检查点：抛出异常
     */
    @Test
    public void testCheckCanDeleteResource() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setUuid("uuid");
        when(jobService.getJobCount(ArgumentMatchers.anyList(), ArgumentMatchers.anyList(), ArgumentMatchers.anyList()))
            .thenReturn(1);
        assertThrows(LegoCheckedException.class,
            () -> exchangeServiceImplUnderTest.checkCanDeleteResource(environment));
    }

    private ProtectedEnvironment getEnvironment() {
        String json =
            "{\"name\":\"Server01\",\"type\":\"Database\",\"subType\":\"Exchange-group\",\"extendInfo\":{\"linkStatus\":\"0\",\"isGroup\":0},\"auth\":{\"authType\":2,\"authKey\":\"extest\\\\Administrator\",\"authPwd\":\"Huawei@123\"},\"dependencies\":{\"agents\":[{\"uuid\":\"2615222a-1993-4f84-8be5-7c74dc335a6d\"}]}}";
        ProtectedEnvironment read = JsonUtil.read(json, ProtectedEnvironment.class);
        return read;
    }

    private ProtectedEnvironment getDagEnvironment() {
        String json =
            "{\"name\":\"Server01\",\"type\":\"Database\",\"subType\":\"Exchange-group\",\"extendInfo\":{\"linkStatus\":\"0\",\"isGroup\":1},\"auth\":{\"authType\":2,\"authKey\":\"extest\\\\Administrator\",\"authPwd\":\"Huawei@123\"},\"dependencies\":{\"agents\":[{\"uuid\":\"2615222a-1993-4f84-8be5-7c74dc335a6d\"},{\"uuid\":\"2615222a-1993-4f84-8be5-7c74dc335a6d\"}]}}";
        ProtectedEnvironment read = JsonUtil.read(json, ProtectedEnvironment.class);
        return read;
    }

    private ProtectedEnvironment getDagEnvironmentWithOneAgent() {
        String json =
            "{\"name\":\"Server01\",\"type\":\"Database\",\"subType\":\"Exchange-group\",\"extendInfo\":{\"linkStatus\":\"0\",\"isGroup\":1},\"auth\":{\"authType\":2,\"authKey\":\"extest\\\\Administrator\",\"authPwd\":\"Huawei@123\"},\"dependencies\":{\"agents\":[{\"uuid\":\"2615222a-1993-4f84-8be5-7c74dc335a6d\"},{\"uuid\":\"2615222a-1993-4f84-8be5-7c74dc335a6d\"}]}}";
        ProtectedEnvironment read = JsonUtil.read(json, ProtectedEnvironment.class);
        return read;
    }
}
