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
package openbackup.obs.plugin.agent;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyMap;

import openbackup.access.framework.resource.persistence.dao.ProtectedResourceAgentMapper;
import openbackup.access.framework.resource.service.AgentBusinessService;
import com.huawei.oceanprotect.base.cluster.sdk.dto.MemberClusterBo;
import com.huawei.oceanprotect.base.cluster.sdk.service.MemberClusterService;
import openbackup.data.access.framework.agent.DefaultProtectAgentSelector;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.access.framework.protection.common.constants.AgentKeyConstant;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedObject;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;

import openbackup.system.base.common.enums.DeployTypeEnum;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.sdk.auth.UserInnerResponse;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.UUID;

/**
 * {@link ObjectStorageAgentSelector} 测试类
 *
 * @author l00370588
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-1-14
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(EnvironmentLinkStatusHelper.class)
public class ObjectStorageAgentSelectorTest {
    private final ResourceService resourceService = PowerMockito.mock(ResourceService.class);

    private final DeployTypeService deployTypeService = PowerMockito.mock(DeployTypeService.class);

    private final ProtectedResourceAgentMapper protectedResourceAgentMapper = PowerMockito.mock(
        ProtectedResourceAgentMapper.class);

    private final DefaultProtectAgentSelector defaultSelector = PowerMockito.mock(DefaultProtectAgentSelector.class);

    private final MemberClusterService memberClusterService = PowerMockito.mock(MemberClusterService.class);

    private final AgentBusinessService agentBusinessService = PowerMockito.mock(AgentBusinessService.class);

    private final ObjectStorageAgentSelector objectStorageAgentSelector = new ObjectStorageAgentSelector(
        agentBusinessService, resourceService, protectedResourceAgentMapper);

    /**
     * 用例场景：ObjectStorage的agent适配器
     * 前置条件：输入资源类型
     * 检查点：匹配返回true，否则返回false
     */
    @Test
    public void applicable_object_agent_selector_success() {
        Assert.assertTrue(objectStorageAgentSelector.applicable(ResourceTypeEnum.OBJECT_STORAGE.getType()));
    }

    /**
     * 用例场景：查询ObjectStorage的agent
     * 前置条件：输入资源信息
     * 检查点：是否获取到agent
     */
    @Test
    public void execute_select_success() {
        PowerMockito.mockStatic(EnvironmentLinkStatusHelper.class);
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any())).thenReturn("1");
        Map<String, String> parameters = new HashMap<>();
        ProtectedResource protectedResource = getProtectedResource();
        parameters.put(AgentKeyConstant.USER_INFO, JSONObject.writeValueAsString(new UserInnerResponse()));
        List<Endpoint> endpointList = mockEndPointList();
        PowerMockito.when(agentBusinessService.queryInternalAgents()).thenReturn(mockEndPointList());
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(mockPageListResponse());
        PowerMockito.when(resourceService.getResourceById(anyBoolean(), any())).thenReturn(Optional.of(mockEnv()));
        PowerMockito.when(defaultSelector.selectByAgentParameter(any(), any())).thenReturn(endpointList);
        MemberClusterBo memberClusterBo = new MemberClusterBo();
        memberClusterBo.setRemoteEsn("1d161131361");
        List<Endpoint> endpoints = objectStorageAgentSelector.select(protectedResource, parameters);
        Assert.assertEquals(1, endpoints.size());
    }

    private List<Endpoint> mockEndPointList() {
        List<Endpoint> endpointList = new ArrayList<>();
        Endpoint endpoint = new Endpoint();
        endpoint.setIp("127.0.0.1");
        endpoint.setPort(8088);
        endpointList.add(endpoint);
        return endpointList;
    }

    private ProtectedResource getProtectedResource() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(ResourceSubTypeEnum.NAS_FILESYSTEM.getType());
        ProtectedObject protectedObject = new ProtectedObject();
        Map<String, Object> objectExtendParam = new HashMap<>();
        objectExtendParam.put(AgentKeyConstant.AGENTS_KEY, "9d135bb7-23fb-4c93-bbd6-8f26a1f430bd");
        protectedObject.setExtParameters(objectExtendParam);
        protectedResource.setProtectedObject(protectedObject);
        return protectedResource;
    }

    private ProtectedResource mockProtectedResource() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(ResourceSubTypeEnum.NAS_FILESYSTEM.getType());
        protectedResource.setEnvironment(mockEnv());
        return protectedResource;
    }

    private ProtectedEnvironment mockEnv() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setUuid(UUID.randomUUID().toString());
        environment.setEndpoint("XX.XX.XX.XX");
        environment.setPort(50529);
        environment.setLinkStatus("1");
        return environment;
    }

    private PageListResponse<ProtectedResource> mockPageListResponse() {
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        response.setRecords(Collections.singletonList(mockProtectedResource()));
        response.setTotalCount(1);
        return response;
    }

    /**
     * 用例场景：x8000部署方式下查询ObjectStorage的agent
     * 前置条件：输入资源信息
     * 检查点：是否获取到agent
     */
    @Test
    public void execute_x8000_select_success() {
        PowerMockito.mockStatic(EnvironmentLinkStatusHelper.class);
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any())).thenReturn("1");
        MemberClusterBo memberClusterBo = new MemberClusterBo();
        memberClusterBo.setRemoteEsn("1d161131361");
        PowerMockito.when(memberClusterService.getCurrentNodeInfo()).thenReturn(memberClusterBo);
        PowerMockito.when(deployTypeService.getDeployType()).thenReturn(DeployTypeEnum.X8000);
        List<Endpoint> endpointList = Arrays.asList(
            new Endpoint("b88e84f0-a7d0-472d-a336-7898436ffe43", "192.169.1.13", 9099),
            new Endpoint("b88e84f0-a7d0-472d-a336-7898436ffe45", "192.169.1.12", 9099));

        PowerMockito.when(agentBusinessService.queryInternalAgents()).thenReturn(endpointList);

        Map<String, String> parameters = new HashMap<>();
        parameters.put(AgentKeyConstant.USER_INFO, JSONObject.writeValueAsString(new UserInnerResponse()));
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(mockPageListResponse());
        PowerMockito.when(resourceService.getResourceById(anyBoolean(), any())).thenReturn(Optional.of(mockEnv()));
        PowerMockito.when(defaultSelector.selectByAgentParameter(any(), any())).thenReturn(endpointList);
        List<Endpoint> endpoints = objectStorageAgentSelector.select(new ProtectedResource(), parameters);
        Assert.assertEquals(parameters.get(AgentKeyConstant.AGENTS_KEY),
            "b88e84f0-a7d0-472d-a336-7898436ffe43;b88e84f0-a7d0-472d-a336-7898436ffe45");
        Assert.assertEquals(endpoints.size(), 2);
        select_success();
    }

    private void select_success() {
        ProtectedResource resource = new ProtectedResource();
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(AgentKeyConstant.AGENTS_KEY,
            "b88e84f0-a7d0-472d-a336-7898436ffe43;b88e84f0-a7d0-472d-a336-7898436ffe45");
        resource.setExtendInfo(extendInfo);

        Map<String, String> parameters = new HashMap<>();
        parameters.put(AgentKeyConstant.USER_INFO, JSONObject.writeValueAsString(new UserInnerResponse()));
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(mockPageListResponse());
        PowerMockito.when(resourceService.getResourceById(anyBoolean(), any())).thenReturn(Optional.of(mockEnv()));
        List<Endpoint> endpoints = objectStorageAgentSelector.select(resource, parameters);
        Assert.assertEquals(parameters.get(AgentKeyConstant.AGENTS_KEY),
            "b88e84f0-a7d0-472d-a336-7898436ffe43;b88e84f0-a7d0-472d-a336-7898436ffe45");
        Assert.assertEquals(endpoints.size(), 2);
    }

}