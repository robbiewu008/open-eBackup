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

import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.service.ClusterEnvironmentService;
import openbackup.database.base.plugin.service.InstanceResourceService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.UUID;

/**
 * {@link PostgreClusterProvider} 测试类
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(EnvironmentLinkStatusHelper.class)
public class PostgreClusterProviderTest {
    private final ClusterEnvironmentService clusterEnvironmentService = PowerMockito.mock(
        ClusterEnvironmentService.class);

    private final ProtectedEnvironmentService environmentService = PowerMockito.mock(ProtectedEnvironmentService.class);

    private final InstanceResourceService instanceResourceService = PowerMockito.mock(InstanceResourceService.class);

    private final AgentUnifiedService agentUnifiedService = PowerMockito.mock(AgentUnifiedService.class);

    private final PostgreClusterProvider postgreClusterProvider = new PostgreClusterProvider(clusterEnvironmentService,
        environmentService, instanceResourceService, agentUnifiedService);

    /**
     * 用例场景：框架调 applicable接口
     * 前置条件：applicable输入资源类型
     * 检查点：是否返回true
     */
    @Test
    public void applicable_postgre_cluster_provider_success() {
        Assert.assertTrue(postgreClusterProvider.applicable(ResourceSubTypeEnum.POSTGRE_CLUSTER.getType()));
        Assert.assertFalse(postgreClusterProvider.applicable(ResourceSubTypeEnum.GAUSSDBT.getType()));
    }

    /**
     * 用例场景：postgre集群健康检查
     * 前置条件：节点在线
     * 检查点: 检查成功
     */
    @Test
    public void health_check_success() {
        ProtectedEnvironment protectedEnvironment = buildEnv(LinkStatusEnum.ONLINE.getStatus().toString());
        PowerMockito.mockStatic(EnvironmentLinkStatusHelper.class);
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any()))
            .thenReturn(LinkStatusEnum.ONLINE.getStatus().toString());
        postgreClusterProvider.validate(protectedEnvironment);
    }

    /**
     * 用例场景：postgre集群健康检查
     * 前置条件：节点离线
     * 检查点: 检查失败
     */
    @Test
    public void should_throw_LegoCheckedException_if_host_offline_when_health_check() {
        ProtectedEnvironment protectedEnvironment = buildEnv(LinkStatusEnum.OFFLINE.getStatus().toString());
        PowerMockito.mockStatic(EnvironmentLinkStatusHelper.class);
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any()))
            .thenReturn(LinkStatusEnum.OFFLINE.getStatus().toString());
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> postgreClusterProvider.validate(protectedEnvironment));
        Assert.assertEquals("Select host is offLine.", legoCheckedException.getMessage());
        Assert.assertEquals(CommonErrorCode.HOST_OFFLINE, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：修改集群时检查集群节点信息
     * 前置条件：集群信息
     * 检查点: 无异常抛出
     */
    @Test
    public void execute_update_check_success() {
        PowerMockito.when(environmentService.getEnvironmentById(any()))
            .thenReturn(buildEnv(LinkStatusEnum.ONLINE.getStatus().toString()));
        ProtectedEnvironment protectedEnvironment = buildEnv(LinkStatusEnum.ONLINE.getStatus().toString());
        protectedEnvironment.setUuid(UUID.randomUUID().toString());
        postgreClusterProvider.register(protectedEnvironment);
        Assert.assertEquals(LinkStatusEnum.ONLINE.getStatus().toString(), protectedEnvironment.getLinkStatus());
    }

    /**
     * 用例场景：创建集群时检查集群节点信息
     * 前置条件：集群信息
     * 检查点: 无异常抛出
     */
    @Test
    public void execute_register_check_success() {
        PowerMockito.when(environmentService.getEnvironmentById(any()))
            .thenReturn(buildEnv(LinkStatusEnum.ONLINE.getStatus().toString()));
        ProtectedEnvironment protectedEnvironment = buildEnv(LinkStatusEnum.ONLINE.getStatus().toString());
        postgreClusterProvider.register(protectedEnvironment);
        Assert.assertEquals(LinkStatusEnum.ONLINE.getStatus().toString(), protectedEnvironment.getLinkStatus());
    }

    private ProtectedEnvironment buildEnv(String status) {
        ProtectedEnvironment host = new ProtectedEnvironment();
        host.setUuid(UUIDGenerator.getUUID());
        host.setLinkStatus(status);
        ProtectedEnvironment hostTwo = BeanTools.copy(host, ProtectedEnvironment::new);
        hostTwo.setUuid(UUIDGenerator.getUUID());
        List<ProtectedResource> resources = new ArrayList<>();
        resources.add(host);
        resources.add(hostTwo);
        Map<String, List<ProtectedResource>> agents = new HashMap<>();
        agents.put(DatabaseConstants.AGENTS, resources);
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setDependencies(agents);
        return environment;
    }
}