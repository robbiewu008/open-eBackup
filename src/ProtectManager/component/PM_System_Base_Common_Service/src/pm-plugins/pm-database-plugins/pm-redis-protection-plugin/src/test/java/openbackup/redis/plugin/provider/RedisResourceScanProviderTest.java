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
package openbackup.redis.plugin.provider;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.database.base.plugin.common.DatabaseConstants;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;

import openbackup.redis.plugin.service.RedisService;
import com.huawei.oceanprotect.system.base.kerberos.service.KerberosService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import com.google.common.collect.Lists;
import com.google.common.collect.Maps;

import org.apache.commons.collections.CollectionUtils;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatchers;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;

/**
 * redis资源扫描测试类
 *
 */
@RunWith(PowerMockRunner.class)
public class RedisResourceScanProviderTest {
    private AgentUnifiedService agentUnifiedService;

    private ProtectedEnvironmentService environmentService;

    private RedisResourceScanProvider redisResourceScanProvider;

    private KerberosService kerberosService;

    private EncryptorService encryptorService;

    private RedisService redisService;

    @Before
    public void setUp() {
        agentUnifiedService = PowerMockito.mock(AgentUnifiedService.class);
        environmentService = PowerMockito.mock(ProtectedEnvironmentService.class);
        kerberosService = PowerMockito.mock(KerberosService.class);
        encryptorService = PowerMockito.mock(EncryptorService.class);
        redisService = PowerMockito.mock(RedisService.class);
        redisResourceScanProvider = new RedisResourceScanProvider(agentUnifiedService,
            environmentService, kerberosService, encryptorService, redisService);
    }

    /**
     * 用例场景：redis类型识别
     * 前置条件：无
     * 检查点: 识别成功
     */
    @Test
    public void applicable_success() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setSubType(ResourceSubTypeEnum.REDIS.getType());
        Assert.assertTrue(redisResourceScanProvider.applicable(protectedEnvironment));
    }

    /**
     * 用例场景：如果扫描资源的subType非Redis，就返回空
     * 前置条件：无
     * 检查点: 返回空
     */
    @Test
    public void should_return_empty_if_subType_is_not_redis() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setSubType(ResourceSubTypeEnum.CLICK_HOUSE.getType());
        List<ProtectedResource> result = redisResourceScanProvider.scan(protectedEnvironment);
        Assert.assertTrue(CollectionUtils.isEmpty(result));
    }

    /**
     * 用例场景：扫描集群下的节点信息
     * 前置条件：无
     * 检查点: 扫描成功
     */
    @Test
    public void scan_success() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setSubType(ResourceSubTypeEnum.REDIS.getType());
        ProtectedResource protectedResource = new ProtectedResource();
        ProtectedResource host = new ProtectedResource();
        host.setUuid("123");
        Authentication auth = new Authentication();
        auth.setExtendInfo(Maps.newHashMap());
        protectedResource.setDependencies(Collections.singletonMap(DatabaseConstants.AGENTS, Lists.newArrayList(host)));
        protectedResource.setExtendInfo(new HashMap<>());
        ProtectedResource protectedResource1 = new ProtectedResource();
        ProtectedResource host1 = new ProtectedResource();
        host1.setUuid("456");
        protectedResource1.setDependencies(
            Collections.singletonMap(DatabaseConstants.AGENTS, Lists.newArrayList(host1)));
        protectedResource1.setExtendInfo(new HashMap<>());
        protectedResource.setAuth(auth);
        protectedResource1.setAuth(auth);
        protectedEnvironment.setDependencies(Collections.singletonMap(ResourceConstants.CHILDREN,
            Lists.newArrayList(protectedResource, protectedResource1)));
        ProtectedEnvironment agent = new ProtectedEnvironment();
        agent.setEndpoint("192.168.1.1");
        agent.setPort(123456);
        PowerMockito.when(environmentService.getEnvironmentById(ArgumentMatchers.endsWith("123"))).thenReturn(agent);
        ProtectedEnvironment agent1 = new ProtectedEnvironment();
        agent1.setEndpoint("192.168.1.2");
        agent1.setPort(123456);
        PowerMockito.when(environmentService.getEnvironmentById(ArgumentMatchers.endsWith("456"))).thenReturn(agent1);
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        response.setTotalCount(0);
        PowerMockito.when(agentUnifiedService.getDetailPageListNoRetry(ArgumentMatchers.anyString(),
                ArgumentMatchers.endsWith("192.168.1.1"), ArgumentMatchers.anyInt(), any(), ArgumentMatchers.eq(false)))
            .thenReturn(response);
        PageListResponse<ProtectedResource> response1 = new PageListResponse<>();
        response1.setTotalCount(1);
        response1.setRecords(Lists.newArrayList(protectedResource));
        PowerMockito.when(agentUnifiedService.getDetailPageListNoRetry(ArgumentMatchers.anyString(),
            ArgumentMatchers.endsWith("192.168.1.2"), ArgumentMatchers.anyInt(), any(), ArgumentMatchers.eq(false))).thenReturn(response1);

        Endpoint endpoint = new Endpoint();
        endpoint.setId(host.getUuid());
        Endpoint endpoint1 = new Endpoint();
        endpoint1.setId(host1.getUuid());
        PowerMockito.when(redisService.selectAgent(any())).thenReturn(endpoint).thenReturn(endpoint1);

        redisResourceScanProvider.scan(protectedEnvironment);
    }
}