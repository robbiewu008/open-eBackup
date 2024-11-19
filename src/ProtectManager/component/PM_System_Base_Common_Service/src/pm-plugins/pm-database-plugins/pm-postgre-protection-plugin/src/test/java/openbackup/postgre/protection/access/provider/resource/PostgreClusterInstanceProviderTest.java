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
package openbackup.postgre.protection.access.provider.resource;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.service.InstanceResourceService;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * poostgre集群实例测试类
 *
 */
public class PostgreClusterInstanceProviderTest {
    private static final String ENDPOINT = "8.40.99.187";

    private final ProtectedEnvironmentService environmentService = Mockito.mock(ProtectedEnvironmentService.class);

    private final InstanceResourceService instanceResourceService = Mockito.mock(InstanceResourceService.class);

    private final AgentUnifiedService agentUnifiedService = Mockito.mock(AgentUnifiedService.class);

    private final ResourceService resourceService = Mockito.mock(ResourceService.class);

    private PostgreClusterInstanceProvider provider = new PostgreClusterInstanceProvider(environmentService,
        instanceResourceService, agentUnifiedService, resourceService);

    @Before
    public void init() {
        PowerMockito.when(environmentService.getEnvironmentById(any())).thenReturn(mockEnv());
    }

    /**
     * 用例场景：框架调 applicable接口
     * 前置条件：applicable输入资源
     * 检查点：是否返回true
     */
    @Test
    public void applicable_postgre_cluster_instance_provider_success() {
        ProtectedResource resource = new ProtectedResource();
        resource.setSubType(ResourceSubTypeEnum.POSTGRE_CLUSTER_INSTANCE.getType());
        Assert.assertTrue(provider.applicable(resource));
        resource.setSubType(ResourceSubTypeEnum.MYSQL.getType());
        Assert.assertFalse(provider.applicable(resource));
    }

    @Test
    public void test_supplyDependency() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setType(ResourceTypeEnum.DATABASE.getType());
        protectedResource.setSubType(ResourceSubTypeEnum.POSTGRE_CLUSTER_INSTANCE.getType());
        protectedResource.setUuid("test1");
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid("uuid");
        protectedEnvironment.setName("name1");
        protectedEnvironment.setEndpoint("endpoint1");
        List<ProtectedResource> list = new ArrayList<>();
        list.add(protectedEnvironment);
        PowerMockito.when(resourceService.queryDependencyResources(Mockito.anyBoolean(), Mockito.any(), Mockito.any()))
            .thenReturn(list);
        Assert.assertTrue(provider.supplyDependency(protectedResource));
    }

    /**
     * 用例场景：创建postgre集群实例前检查
     * 前置条件：参数正常
     * 检查点：无异常抛出
     */
    @Test
    public void execute_check_success() {
        PowerMockito.when(instanceResourceService.checkIsClusterInstance(any())).thenReturn(mockCheckResult());
        ProtectedResource resource = mockResource();
        provider.check(resource);
        Assert.assertEquals(ENDPOINT, resource.getPath());
    }

    /**
     * 用例场景：修改postgre集群实例前检查
     * 前置条件：参数正常
     * 检查点：无异常抛出
     */
    @Test
    public void execute_before_update_success() {
        ProtectedResource resource = mockResource();
        provider.beforeUpdate(resource);
        Assert.assertEquals(ENDPOINT, resource.getExtendInfoByKey(DatabaseConstants.VIRTUAL_IP));
    }

    private ProtectedResource mockResource() {
        ProtectedResource resource = new ProtectedResource();
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(DatabaseConstants.INSTANCE_PORT, "5432");
        resource.setExtendInfo(extendInfo);
        resource.setEnvironment(mockEnv());
        return resource;
    }

    private ProtectedEnvironment mockEnv() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setEndpoint(ENDPOINT);
        return environment;
    }

    private AgentBaseDto mockCheckResult() {
        Map<String, String> map = new HashMap<>();
        map.put(DatabaseConstants.VERSION, "9.6.0");
        AgentBaseDto checkResult = new AgentBaseDto();
        checkResult.setErrorMessage(JSONObject.fromObject(map).toString());
        return checkResult;
    }
}