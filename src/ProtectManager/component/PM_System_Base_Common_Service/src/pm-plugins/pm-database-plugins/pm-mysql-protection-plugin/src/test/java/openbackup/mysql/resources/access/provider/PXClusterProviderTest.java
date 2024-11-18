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
package openbackup.mysql.resources.access.provider;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.exception.DataProtectionAccessException;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import openbackup.mysql.resources.access.common.MysqlConstants;
import openbackup.mysql.resources.access.enums.MysqlResourceSubTypeEnum;
import openbackup.system.base.common.rest.FeignBuilder;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * pxc集群校验测试类
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {FeignBuilder.class})
public class PXClusterProviderTest {
    private ResourceService resourceService;

    private PXClusterProvider pXClusterProvider;

    private EncryptorService encryptorService;

    private final AgentUnifiedService agentUnifiedService = PowerMockito.mock(AgentUnifiedService.class);

    @Rule
    public ExpectedException expectedException = ExpectedException.none();

    @Before
    public void init() {
        this.resourceService = Mockito.mock(ResourceService.class);
        this.encryptorService = Mockito.mock(EncryptorService.class);

        this.pXClusterProvider = new PXClusterProvider(this.resourceService, this.encryptorService,
            agentUnifiedService);
    }

    /**
     * 用例场景：mysql集群类型检查类provider过滤
     * 前置条件：无
     * 检查点：集群类型类过滤成功
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(pXClusterProvider.applicable(MysqlConstants.MYSQL_PXC));
    }

    /**
     * 用例场景：mysql集群条件检查
     * 前置条件：无
     * 检查点：集群条件检查成功
     */
    @Test
    public void checkIsCluster_success() throws IllegalAccessException {
        ProtectedResource protectedResource = getProtectedResource();
        AgentBaseDto agentDto = new AgentBaseDto();
        agentDto.setErrorCode("0");
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid("111111");
        protectedEnvironment.setEndpoint("127.0.0.1");
        protectedEnvironment.setPort(3388);
        PowerMockito.when(resourceService.getResourceById(any()))
            .thenReturn(java.util.Optional.of(protectedEnvironment));
        Assert.assertTrue(pXClusterProvider.checkIsCluster(protectedResource));
    }

    private ProtectedResource getProtectedResource() {
        ProtectedResource protectedResource = new ProtectedResource();
        Map<String, List<ProtectedResource>> dependency = new HashMap<>();
        ProtectedResource resourceItem = new ProtectedResource();
        resourceItem.setParentUuid("11111");
        resourceItem.setUuid("11111");
        resourceItem.setExtendInfo(new HashMap<>());
        dependency.put(DatabaseConstants.CHILDREN, Collections.singletonList(resourceItem));
        protectedResource.setDependencies(dependency);
        protectedResource.setSubType(MysqlResourceSubTypeEnum.MYSQL_CLUSTER_INSTANCE.getType());
        protectedResource.setExtendInfo(new HashMap<>());
        Map<String, String> ext = new HashMap<>();
        ext.put(DatabaseConstants.INSTANCE_PORT, "3306");
        Authentication auth = new Authentication();
        auth.setAuthKey("root");
        auth.setAuthPwd("123456");
        auth.setExtendInfo(ext);
        resourceItem.setAuth(auth);
        Map<String, List<ProtectedResource>> itemDependency = new HashMap<>();
        ProtectedResource agent = new ProtectedResource();
        agent.setUuid("11111");
        itemDependency.put(DatabaseConstants.AGENTS, Collections.singletonList(agent));
        resourceItem.setDependencies(itemDependency);
        return protectedResource;
    }

    /**
     * 用例场景：mysql集群条件检查失败
     * 前置条件：无
     * 检查点：集群条件检查失败
     */
    @Test
    public void should_throw_LegoCheckedException_when_checkIsCluster() throws IllegalAccessException {
        expectedException.expect(DataProtectionAccessException.class);
        ProtectedResource protectedResource = getProtectedResource();
        AgentBaseDto agentDto = new AgentBaseDto();
        agentDto.setErrorCode("1");
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid("111111");
        protectedEnvironment.setEndpoint("127.0.0.1");
        protectedEnvironment.setPort(3388);
        PowerMockito.when(resourceService.getResourceById(any()))
            .thenReturn(java.util.Optional.of(protectedEnvironment));
        pXClusterProvider.checkIsCluster(protectedResource);
    }
}