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
package openbackup.database.base.plugin.service.impl;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;

import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.common.DatabaseErrorCode;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.util.BeanTools;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.UUID;

/**
 * {@link ClusterEnvironmentServiceImpl} 测试类
 *
 * @author lWX776769
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-08-29
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest( {EnvironmentLinkStatusHelper.class})
public class ClusterEnvironmentServiceImplTest {
    private final ResourceService resourceService = PowerMockito.mock(ResourceService.class);

    private final ClusterEnvironmentServiceImpl clusterEnvironmentService = new ClusterEnvironmentServiceImpl(
        resourceService);

    /**
     * 用例场景：集群节点数检查
     * 前置条件：节点数小于二
     * 检查点: 抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_host_num_less_than_two_when_check_cluster_node() {
        ProtectedEnvironment protectedEnvironment = buildEnv(LinkStatusEnum.ONLINE.getStatus().toString());
        List<ProtectedResource> agents = protectedEnvironment.getDependencies().get(DatabaseConstants.AGENTS);
        agents.remove(IsmNumberConstant.ONE);
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> clusterEnvironmentService.checkClusterNodeNum(agents));
        Assert.assertEquals("Select cluster node num is error.", legoCheckedException.getMessage());
        Assert.assertEquals(CommonErrorCode.ERR_PARAM, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：集群节点数检查
     * 前置条件：节点数不小于二
     * 检查点: 无异常抛出
     */
    @Test
    public void check_cluster_node_num_success() {
        ProtectedEnvironment protectedEnvironment = buildEnv(LinkStatusEnum.ONLINE.getStatus().toString());
        List<ProtectedResource> agents = protectedEnvironment.getDependencies().get(DatabaseConstants.AGENTS);
        clusterEnvironmentService.checkClusterNodeNum(agents);
        agents.remove(0);
        Assert.assertThrows(LegoCheckedException.class, () -> clusterEnvironmentService.checkClusterNodeNum(agents));
    }

    /**
     * 用例场景：集群节点状态检查
     * 前置条件：节点状态离线
     * 检查点: 抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_host_offline_when_check() {
        PowerMockito.mockStatic(EnvironmentLinkStatusHelper.class);
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any()))
            .thenReturn(LinkStatusEnum.OFFLINE.getStatus().toString());
        List<ProtectedEnvironment> environments = buildEnvironments(LinkStatusEnum.OFFLINE.getStatus().toString());
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> clusterEnvironmentService.checkClusterNodeStatus(environments));
        Assert.assertEquals("Select host is offLine.", legoCheckedException.getMessage());
        Assert.assertEquals(CommonErrorCode.HOST_OFFLINE, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：集群节点状态检查
     * 前置条件：节点状态都在线
     * 检查点: 无异常抛出
     */
    @Test
    public void check_cluster_node_status_is_online() {
        PowerMockito.mockStatic(EnvironmentLinkStatusHelper.class);
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any()))
            .thenReturn(LinkStatusEnum.ONLINE.getStatus().toString());
        List<ProtectedEnvironment> environments = buildEnvironments(LinkStatusEnum.ONLINE.getStatus().toString());

        try {
            clusterEnvironmentService.checkClusterNodeStatus(environments);
        } catch (Exception e) {
            Assert.fail();
        }
    }

    /**
     * 用例场景：注册集群时检查集群节点是否被注册
     * 前置条件：集群节点被注册
     * 检查点: 抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_host_is_registered_when_check() {
        ProtectedResource resource = buildEnv(LinkStatusEnum.ONLINE.getStatus().toString());
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(buildKingBaseCluster());
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(Optional.of(resource));
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> clusterEnvironmentService.checkRegisterNodeIsRegistered(resource));
        Assert.assertEquals("The host has been registered.", legoCheckedException.getMessage());
        Assert.assertEquals(CommonErrorCode.CLUSTER_NODE_IS_REGISTERED, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：检查集群重复节点
     * 前置条件：存在重复节点
     * 检查点: 抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_host_duplicate_when_check() {
        List<ProtectedResource> environments = new ArrayList<>();
        ProtectedResource node = new ProtectedResource();
        node.setUuid(UUIDGenerator.getUUID());
        environments.add(node);
        environments.add(node);
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> clusterEnvironmentService.checkClusterNodeNum(environments));
        Assert.assertEquals("There are duplicate hosts.", legoCheckedException.getMessage());
        Assert.assertEquals(CommonErrorCode.ERR_PARAM, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：检查集群os类型
     * 前置条件：os类型不一致
     * 检查点: 抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_os_type_inconsistencies_when_check() {
        List<ProtectedEnvironment> environments = buildEnvironments(LinkStatusEnum.ONLINE.getStatus().toString());
        environments.get(IsmNumberConstant.ZERO).setOsType("Linux");
        environments.get(IsmNumberConstant.ONE).setOsType("Windows");
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> clusterEnvironmentService.checkClusterNodeOsType(environments));
        Assert.assertEquals("Select host os type inconsistencies.", legoCheckedException.getMessage());
        Assert.assertEquals(DatabaseErrorCode.DATABASE_OS_INCONSISTENT, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：检查集群os类型
     * 前置条件：os类型一致
     * 检查点: 无异常抛出
     */
    @Test
    public void check_cluster_node_os_type_suceesss() {
        List<ProtectedEnvironment> environments = buildEnvironments(LinkStatusEnum.ONLINE.getStatus().toString());
        environments.get(IsmNumberConstant.ZERO).setOsType("Linux");
        environments.get(IsmNumberConstant.ONE).setOsType("Linux");
        clusterEnvironmentService.checkClusterNodeOsType(environments);
        ProtectedEnvironment node = new ProtectedEnvironment();
        node.setUuid(UUIDGenerator.getUUID());
        node.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        environments.add(node);
        environments.get(IsmNumberConstant.TWO).setOsType("Windows");
        Assert.assertThrows(LegoCheckedException.class,
            () -> clusterEnvironmentService.checkClusterNodeOsType(environments));
    }

    /**
     * 用例场景：检查集群是否已经注册实例
     * 前置条件：已经注册
     * 检查点: 抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_cluster_has_registered_instance_when_check() {
        PageListResponse<ProtectedResource> cluster = buildKingBaseCluster();
        cluster.setTotalCount(IsmNumberConstant.ONE);
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(cluster);
        ProtectedEnvironment protectedEnvironment = buildEnv(LinkStatusEnum.ONLINE.getStatus().toString());
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> clusterEnvironmentService.checkClusterIsRegisteredInstance(protectedEnvironment));
        Assert.assertEquals("The cluster has registered instances and cannot be modified.",
            legoCheckedException.getMessage());
        Assert.assertEquals(CommonErrorCode.DB_CLUSTER_HAS_INSTANCE, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：检查集群是否已经注册实例
     * 前置条件：没有注册
     * 检查点: 无异常抛出
     */
    @Test
    public void check_cluster_is_registered_instance_success() {
        PageListResponse<ProtectedResource> cluster = buildKingBaseCluster();
        cluster.setTotalCount(IsmNumberConstant.ZERO);
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(cluster);
        ProtectedEnvironment protectedEnvironment = buildEnv(LinkStatusEnum.ONLINE.getStatus().toString());
        clusterEnvironmentService.checkClusterIsRegisteredInstance(protectedEnvironment);
        Mockito.verify(resourceService, Mockito.times(1)).query(anyInt(), anyInt(), any());
    }

    /**
     * 用例场景：检查集群更新节点是否已经被注册
     * 前置条件：未被注册
     * 检查点: 无异常抛出
     */
    @Test
    public void should_throw_LegoCheckedException_if_update_node_is_registered_when_update_check() {
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(buildKingBaseCluster());
        PowerMockito.when(resourceService.getResourceById(any()))
            .thenReturn(Optional.of(buildEnv(LinkStatusEnum.ONLINE.getStatus().toString())));
        ProtectedResource protectedResource = buildEnv(LinkStatusEnum.ONLINE.getStatus().toString());
        clusterEnvironmentService.checkUpdateNodeIsRegistered(protectedResource);
        Mockito.verify(resourceService, Mockito.times(2)).getResourceById(any());
    }

    /**
     * 用例场景：检查集群节点数最大规格
     * 前置条件：超过最大规格
     * 检查点: 抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_cluster_node_is_exceed_max_when_check_num() {
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> clusterEnvironmentService.checkClusterNodeCountLimit(3, 2));
        Assert.assertEquals(DatabaseErrorCode.CLUSTER_NODE_NUMBER_ERROR, legoCheckedException.getErrorCode());
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
        environment.setUuid(UUIDGenerator.getUUID());
        return environment;
    }

    private List<ProtectedEnvironment> buildEnvironments(String status) {
        ProtectedEnvironment node = new ProtectedEnvironment();
        node.setUuid(UUIDGenerator.getUUID());
        node.setLinkStatus(status);
        ProtectedEnvironment nodeTwo = BeanTools.copy(node, ProtectedEnvironment::new);
        nodeTwo.setUuid(UUIDGenerator.getUUID());
        List<ProtectedEnvironment> environments = new ArrayList<>();
        environments.add(node);
        environments.add(nodeTwo);
        return environments;
    }

    private PageListResponse<ProtectedResource> buildKingBaseCluster() {
        ProtectedResource resource = new ProtectedResource();
        resource.setUuid(UUID.randomUUID().toString());
        List<ProtectedResource> resources = new ArrayList<>();
        resources.add(resource);
        PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<>();
        pageListResponse.setRecords(resources);
        pageListResponse.setTotalCount(1);
        return pageListResponse;
    }
}