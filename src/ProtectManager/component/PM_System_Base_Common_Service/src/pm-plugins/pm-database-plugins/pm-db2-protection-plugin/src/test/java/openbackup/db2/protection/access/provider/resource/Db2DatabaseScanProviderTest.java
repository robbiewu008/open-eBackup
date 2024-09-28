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
package openbackup.db2.protection.access.provider.resource;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentDetailDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppResource;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.rest.FeignBuilder;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.UUID;

/**
 * {@link Db2DatabaseScanProvider} 测试类
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {FeignBuilder.class})
public class Db2DatabaseScanProviderTest {
    private final ResourceService resourceService = PowerMockito.mock(ResourceService.class);

    private final AgentUnifiedService agentUnifiedService = PowerMockito.mock(AgentUnifiedService.class);

    private Db2DatabaseScanProvider provider;

    @Before
    public void init() {
        this.provider = new Db2DatabaseScanProvider(resourceService, agentUnifiedService);
    }

    /**
     * 用例场景：框架调 applicable接口
     * 前置条件：applicable输入资源
     * 检查点：是否返回true
     */
    @Test
    public void applicable_db2_cluster_instance_provider_success() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setSubType(ResourceSubTypeEnum.U_BACKUP_AGENT.getType());
        Assert.assertTrue(provider.applicable(environment));
        environment.setSubType(ResourceSubTypeEnum.GAUSSDBT.getType());
        Assert.assertFalse(provider.applicable(environment));
    }

    /**
     * 用例场景: 扫描数据库成功
     * 前置条件：信息服务正常
     * 检查点: 扫描成功
     */
    @Test
    public void execute_scan_success() {
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any(), any())).thenReturn(queryInstance());
        PowerMockito.when(agentUnifiedService.getDetail(any(), any(), any(), any())).thenReturn(mockDatabases());
        List<ProtectedResource> databaseList = provider.scan(mockEnv());
        Assert.assertEquals(1, databaseList.size());
    }

    /**
     * 用例场景 扫描数据库失败
     * 前置条件：check失败
     * 检查点: 扫描失败
     */
    @Test
    public void should_throw_LegoCheckedException_if_check_failed_when_scan_database() {
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any(), any())).thenReturn(queryInstance());
        PowerMockito.when(agentUnifiedService.check(any(), any(), any(), any())).thenReturn(mockAgentBaseDto("500"));
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> provider.scan(mockEnv()));
        Assert.assertEquals(CommonErrorCode.AGENT_NETWORK_ERROR, legoCheckedException.getErrorCode());
    }

    private PageListResponse<ProtectedResource> queryInstance() {
        PageListResponse<ProtectedResource> result = new PageListResponse<>();
        result.setTotalCount(1);
        HashMap<String, String> extendInfo = new HashMap<>();
        extendInfo.put(DatabaseConstants.IS_TOP_INSTANCE, DatabaseConstants.TOP_INSTANCE);
        ProtectedResource instance = new ProtectedResource();
        instance.setExtendInfo(extendInfo);
        Authentication auth = new Authentication();
        auth.setAuthKey("db2inst1");
        auth.setAuthPwd("db2inst1");
        instance.setAuth(auth);
        result.setRecords(Collections.singletonList(instance));
        return result;
    }

    private ProtectedEnvironment mockEnv() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setUuid(UUID.randomUUID().toString());
        environment.setEndpoint("127.0.0.1");
        environment.setPort(50000);
        return environment;
    }

    private AgentBaseDto mockAgentBaseDto(String errorCode) {
        AgentBaseDto agentBaseDto = new AgentBaseDto();
        agentBaseDto.setErrorCode(errorCode);
        return agentBaseDto;
    }

    private AgentDetailDto mockDatabases() {
        HashMap<String, String> extendInfo = new HashMap<>();
        extendInfo.put(DatabaseConstants.VERSION, "7.2.3");
        extendInfo.put(DatabaseConstants.DEPLOY_OS_KEY, "redhat");
        AppResource appResource = new AppResource();
        appResource.setExtendInfo(extendInfo);
        AgentDetailDto result = new AgentDetailDto();
        result.setResourceList(Collections.singletonList(appResource));
        return result;
    }
}
