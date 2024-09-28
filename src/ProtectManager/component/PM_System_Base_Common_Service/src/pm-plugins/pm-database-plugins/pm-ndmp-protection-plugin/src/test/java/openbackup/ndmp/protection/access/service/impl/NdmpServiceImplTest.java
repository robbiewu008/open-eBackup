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
package openbackup.ndmp.protection.access.service.impl;

import static org.mockito.ArgumentMatchers.any;

import openbackup.access.framework.resource.service.provider.UnifiedClusterResourceIntegrityChecker;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import com.huawei.oceanprotect.nas.protection.access.service.StorageAgentService;
import openbackup.ndmp.protection.access.service.NdmpService;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;

import com.google.common.collect.Lists;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatchers;
import org.mockito.Mock;
import org.mockito.junit.MockitoJUnitRunner;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.api.support.membermodification.MemberModifier;
import org.powermock.core.classloader.annotations.PrepareForTest;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Optional;

/**
 * 服务测试类
 *
 */
@RunWith(MockitoJUnitRunner.class)
@PrepareForTest(NdmpService.class)
public class NdmpServiceImplTest {
    @Mock
    private ResourceService resourceService;

    @Mock
    private ProviderManager providerManager;

    @Mock
    private ResourceConnectionCheckProvider resourceConnectionCheckProvider;

    @Mock
    private UnifiedClusterResourceIntegrityChecker clusterIntegrityChecker;

    private NdmpServiceImpl ndmpServiceImpl;

    @Mock
    private StorageAgentService storageAgentService;

    @Mock
    private AgentUnifiedService agentService;

    @Mock
    private ProtectedEnvironmentService protectedEnvironmentService;

    @Before
    public void setUp() throws IllegalAccessException {
        ndmpServiceImpl = new NdmpServiceImpl(providerManager, resourceConnectionCheckProvider, resourceService,
            clusterIntegrityChecker, storageAgentService);
        MemberModifier.field(NdmpServiceImpl.class, "agentService").set(ndmpServiceImpl, agentService);
        MemberModifier.field(NdmpServiceImpl.class, "protectedEnvironmentService")
            .set(ndmpServiceImpl, protectedEnvironmentService);
    }

    /**
     * 用例场景：查询资源
     * 前置条件：已有资源
     * 检  查  点：查询成功
     */
    @Test
    public void getexistingNdmpresources() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid("fc963582-3750-4dce-acf6-ce828a7355ab");
        protectedResource.setExtendInfoByKey("status", "OFFLINE");
        protectedResource.setSubType(ResourceSubTypeEnum.NDMP.getType());
        protectedResource.setType(ResourceTypeEnum.DATABASE.getType());
        PowerMockito.when(
            resourceService.query(ArgumentMatchers.anyInt(), ArgumentMatchers.anyInt(), ArgumentMatchers.anyMap()))
            .thenReturn(new PageListResponse<>(1, Collections.singletonList(protectedResource)));
        List<ProtectedResource> existingGaussDbResources = ndmpServiceImpl.getexistingNdmpresources(
            ResourceSubTypeEnum.NDMP.getType(), new HashMap<>());
        Assert.assertEquals("OFFLINE", existingGaussDbResources.get(0).getExtendInfoByKey("status"));
    }

    /**
     * 用例场景：查询注册资源
     * 前置条件：已有资源
     * 检  查  点：查询成功
     */
    @Test
    public void testGetEnvironmentById() {
        ProtectedEnvironment protectedResource = new ProtectedEnvironment();
        protectedResource.setUuid("UUID");
        protectedResource.setSubType(ResourceSubTypeEnum.NDMP_BACKUPSET.getType());
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(Optional.of(protectedResource));
        ProtectedEnvironment environmentById = ndmpServiceImpl.getEnvironmentById("123");
        Assert.assertEquals("UUID", environmentById.getUuid());
    }

    /**
     * 用例场景：查询注册资源
     * 前置条件：已有资源
     * 检  查  点：查询成功
     */
    @Test
    public void testGetAppEnvResponse() {
        ProtectedEnvironment protectedResource = new ProtectedEnvironment();
        protectedResource.setUuid("fc963582-3750-4dce-acf6-ce828a7355ab");
        protectedResource.setSubType(ResourceSubTypeEnum.NDMP_BACKUPSET.getType());
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(Optional.of(protectedResource));
        AppEnvResponse appEnvResponse = new AppEnvResponse();
        appEnvResponse.setSubType(ResourceSubTypeEnum.NDMP_BACKUPSET.getType());
        CheckResult<AppEnvResponse> checkResult = new CheckResult<>();
        checkResult.setData(appEnvResponse);
        PowerMockito.when(clusterIntegrityChecker.generateCheckResult(any())).thenReturn(checkResult);
        AppEnvResponse response = ndmpServiceImpl.getAppEnvResponse(Lists.newArrayList(protectedResource),
            protectedResource);
        Assert.assertNotNull(response);
    }

    /**
     * 用例场景：参数设置
     * 前置条件：无
     * 检  查  点：节点通过
     */
    @Test
    public void testSupplyNodes() {
        Endpoint endpoint = new Endpoint();
        endpoint.setId("111111");
        List<Endpoint> endpoints = new ArrayList<>();
        endpoints.add(endpoint);
        PowerMockito.when(storageAgentService.queryInternalAgents()).thenReturn(endpoints);
        ProtectedEnvironment protectedResource = new ProtectedEnvironment();
        protectedResource.setUuid("UUID");
        protectedResource.setSubType(ResourceSubTypeEnum.NDMP_BACKUPSET.getType());

        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(Optional.of(protectedResource));
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedResource.setName("test");
        protectedResource.setUuid("123");
        protectedResource.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        PowerMockito.when(resourceService.getResourceById(ArgumentMatchers.any()))
            .thenReturn(Optional.of(protectedEnvironment));
        List<TaskEnvironment> taskEnvironments = ndmpServiceImpl.supplyNodes();
        Assert.assertEquals(0, taskEnvironments.size());
    }

    /**
     * 用例场景：查询endpoint
     * 前置条件：无
     * 检  查  点：返回正确
     */
    @Test
    public void testGetInterAgents() {
        Endpoint endpoint = new Endpoint();
        endpoint.setId("111111");
        List<Endpoint> endpoints = new ArrayList<>();
        endpoints.add(endpoint);
        PowerMockito.when(storageAgentService.queryInternalAgents()).thenReturn(endpoints);
        ProtectedEnvironment protectedResource = new ProtectedEnvironment();
        protectedResource.setUuid("UUID");
        protectedResource.setSubType(ResourceSubTypeEnum.NDMP_BACKUPSET.getType());

        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(Optional.of(protectedResource));
        List<ProtectedResource> interAgents = ndmpServiceImpl.getInterAgents();
        Assert.assertEquals(1, interAgents.size());
    }

    /**
     * 用例场景：查询可使用agent
     * 前置条件：无
     * 检  查  点：返回正确
     */
    @Test
    public void testHealthCheck() {
        ProtectedEnvironment protectedResource = new ProtectedEnvironment();
        protectedResource.setUuid("UUID");
        protectedResource.setSubType(ResourceSubTypeEnum.NDMP_BACKUPSET.getType());
        protectedResource.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        ProtectedResource agent = new ProtectedResource();
        agent.setUuid("uuid");
        List<ProtectedResource> agents = Lists.newArrayList(agent);
        protectedResource.setDependencies(Collections.singletonMap("agents", agents));
        AgentBaseDto agentBaseDto = new AgentBaseDto();
        agentBaseDto.setErrorCode("0");
        PowerMockito.when(agentService.checkApplication(any(), any())).thenReturn(new AgentBaseDto());

        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        PowerMockito.when(protectedEnvironmentService.getEnvironmentById(any())).thenReturn(protectedEnvironment);

        List<ProtectedResource> interAgents = ndmpServiceImpl.getOneAgentHealthCheck(protectedResource);
        Assert.assertEquals(0, interAgents.size());
    }
}
