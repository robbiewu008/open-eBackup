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
import static org.mockito.ArgumentMatchers.anyList;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentDetailDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppResource;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.BrowseEnvironmentResourceConditions;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.common.DatabaseErrorCode;
import openbackup.database.base.plugin.service.ClusterEnvironmentService;
import openbackup.database.base.plugin.service.InstanceResourceService;
import openbackup.db2.protection.access.constant.Db2Constants;
import openbackup.db2.protection.access.enums.Db2ClusterTypeEnum;
import openbackup.db2.protection.access.service.Db2InstanceService;
import openbackup.db2.protection.access.service.Db2TablespaceService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.rest.FeignBuilder;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.config.AgentProxyProperties;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Ignore;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.UUID;

/**
 * {@link Db2ClusterProvider} 测试类
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {FeignBuilder.class, EnvironmentLinkStatusHelper.class})
public class Db2ClusterProviderTest {
    private final ClusterEnvironmentService clusterEnvironmentService = PowerMockito.mock(
        ClusterEnvironmentService.class);

    private final ProtectedEnvironmentService environmentService = PowerMockito.mock(ProtectedEnvironmentService.class);

    private final ResourceService resourceService = PowerMockito.mock(ResourceService.class);

    private final Db2TablespaceService db2TablespaceService = PowerMockito.mock(Db2TablespaceService.class);

    private final Db2InstanceService db2instanceService = PowerMockito.mock(Db2InstanceService.class);

    private final InstanceResourceService instanceResourceService = PowerMockito.mock(InstanceResourceService.class);

    private Db2ClusterProvider db2ClusterProvider;

    @Before
    public void init() {
        AgentProxyProperties agentProxyProperties = new AgentProxyProperties();
        agentProxyProperties.setPort(50000);
        agentProxyProperties.setHost("127.0.0.1");
        PowerMockito.mockStatic(FeignBuilder.class);
        this.db2ClusterProvider = new Db2ClusterProvider(environmentService,
            clusterEnvironmentService, resourceService);
        db2ClusterProvider.setDb2TablespaceService(db2TablespaceService);
        db2ClusterProvider.setDb2InstanceService(db2instanceService);
        db2ClusterProvider.setInstanceResourceService(instanceResourceService);
    }

    /**
     * 用例场景：框架调 applicable接口
     * 前置条件：applicable输入资源类型
     * 检查点：是否返回true
     */
    @Test
    public void applicable_db2_cluster_provider_success() {
        Assert.assertTrue(db2ClusterProvider.applicable(ResourceSubTypeEnum.DB2_CLUSTER.getType()));
        Assert.assertFalse(db2ClusterProvider.applicable(ResourceSubTypeEnum.MYSQL_CLUSTER.getType()));
    }

    /**
     * 用例场景：db2集群健康检查
     * 前置条件：节点在线
     * 检查点: 检查成功
     */
    @Test
    public void db2_cluster_health_check_success() {
        ProtectedEnvironment protectedEnvironment = buildEnvironment(LinkStatusEnum.ONLINE.getStatus().toString());
        PowerMockito.mockStatic(EnvironmentLinkStatusHelper.class);
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any()))
            .thenReturn(LinkStatusEnum.ONLINE.getStatus().toString());
        db2ClusterProvider.validate(protectedEnvironment);
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：db2集群健康检查
     * 前置条件：节点离线
     * 检查点: 检查失败
     */
    @Test
    public void should_throw_LegoCheckedException_if_host_offline_when_db2_cluster_health_check() {
        ProtectedEnvironment protectedEnvironment = buildEnvironment(LinkStatusEnum.OFFLINE.getStatus().toString());
        PowerMockito.mockStatic(EnvironmentLinkStatusHelper.class);
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any()))
            .thenReturn(LinkStatusEnum.OFFLINE.getStatus().toString());
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> db2ClusterProvider.validate(protectedEnvironment));
        Assert.assertEquals("Cluster host is offLine.", legoCheckedException.getMessage());
        Assert.assertEquals(CommonErrorCode.HOST_OFFLINE, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：修改db2集群时检查集群节点信息
     * 前置条件：集群信息
     * 检查点: 无异常抛出
     */
    @Test
    public void execute_update_db2_cluster_check_success() {
        PowerMockito.when(environmentService.getEnvironmentById(any()))
            .thenReturn(buildEnvironment(LinkStatusEnum.ONLINE.getStatus().toString()));
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(mockClusterResponse(10));
        ProtectedEnvironment protectedEnvironment = buildEnvironment(LinkStatusEnum.ONLINE.getStatus().toString());
        protectedEnvironment.setUuid(UUID.randomUUID().toString());
        protectedEnvironment.setExtendInfoByKey(DatabaseConstants.CLUSTER_TYPE, Db2ClusterTypeEnum.POWER_HA.getType());
        db2ClusterProvider.register(protectedEnvironment);
        Assert.assertEquals(LinkStatusEnum.ONLINE.getStatus().toString(), protectedEnvironment.getLinkStatus());
    }

    /**
     * 用例场景：创建db2集群时检查集群节点信息
     * 前置条件：集群信息
     * 检查点: 无异常抛出
     */
    @Test
    public void execute_register_db2_cluster_check_success() {
        PowerMockito.when(environmentService.getEnvironmentById(any()))
            .thenReturn(buildEnvironment(LinkStatusEnum.ONLINE.getStatus().toString()));
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(mockClusterResponse(10));
        ProtectedEnvironment protectedEnvironment = buildEnvironment(LinkStatusEnum.ONLINE.getStatus().toString());
        db2ClusterProvider.register(protectedEnvironment);
        Assert.assertEquals(LinkStatusEnum.ONLINE.getStatus().toString(), protectedEnvironment.getLinkStatus());
    }

    /**
     * 用例场景：扫描数据库
     * 前置条件：环境信息正常
     * 检查点: 扫描成功
     */
    @Test
    @Ignore
    public void execute_db2_scan_success() {
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any(), any())).thenReturn(queryClusterInstance());
        List<ProtectedResource> databases = db2ClusterProvider.scan(
            buildEnvironment(LinkStatusEnum.ONLINE.getStatus().toString()));
        Assert.assertEquals(1, databases.size());
    }

    /**
     * 用例场景：浏览db2表空间
     * 前置条件：环境信息正常
     * 检查点: 浏览成功
     */
    @Test
    public void execute_db2_tablespace_browse_success() {
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(mockResource());
        PowerMockito.when(db2TablespaceService.queryClusterTablespace(any(), any()))
            .thenReturn(mockTablespace());
        PageListResponse<ProtectedResource> response = db2ClusterProvider.browse(mockEnv(), mockConditions());
        Assert.assertEquals(1, response.getTotalCount());
    }

    /**
     * 用例场景：db2集群规格校验
     * 前置条件：超过规格数
     * 检查点: 抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_cluster_num_exceed_max_when_db2_cluster_check() {
        PowerMockito.doNothing().when(clusterEnvironmentService).checkClusterNodeNum(anyList());
        // Db2ClusterTypeEnum.getByType(clusterType);
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(mockClusterResponse(101));
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> db2ClusterProvider.register(mockEnv()));
        Assert.assertEquals(DatabaseErrorCode.RESOURCE_REACHED_THE_UPPER_LIMIT, legoCheckedException.getErrorCode());
    }

    private PageListResponse<ProtectedResource> mockClusterResponse(int count) {
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        response.setTotalCount(count);
        return response;
    }

    private ProtectedEnvironment buildEnvironment(String status) {
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
        environment.setExtendInfoByKey(DatabaseConstants.CLUSTER_TYPE, Db2ClusterTypeEnum.DPF.getType());
        return environment;
    }

    private PageListResponse<ProtectedResource> queryClusterInstance() {
        PageListResponse<ProtectedResource> result = new PageListResponse<>();
        result.setTotalCount(1);
        ProtectedResource clusterInstance = new ProtectedResource();
        clusterInstance.setUuid(UUID.randomUUID().toString());
        ProtectedEnvironment agent = new ProtectedEnvironment();
        agent.setEndpoint("127.0.0.1");
        agent.setPort(50000);
        agent.setExtendInfoByKey(ResourceConstants.AGENT_IP_LIST, "127.0.0.1");
        List<ProtectedResource> agents = new ArrayList<>();
        agents.add(agent);
        Map<String, List<ProtectedResource>> agentMap = new HashMap<>();
        agentMap.put(DatabaseConstants.AGENTS, agents);
        ProtectedResource instance = new ProtectedResource();
        instance.setDependencies(agentMap);
        List<ProtectedResource> children = new ArrayList<>();
        children.add(instance);
        Map<String, List<ProtectedResource>> childrenMap = new HashMap<>();
        childrenMap.put(DatabaseConstants.CHILDREN, children);
        clusterInstance.setDependencies(childrenMap);
        clusterInstance.setExtendInfoByKey(Db2Constants.CATALOG_IP_KEY, "127.0.0.1");
        result.setRecords(Collections.singletonList(clusterInstance));
        return result;
    }

    private AgentBaseDto mockAgentBaseDto(String errorCode) {
        AgentBaseDto agentBaseDto = new AgentBaseDto();
        agentBaseDto.setErrorCode(errorCode);
        return agentBaseDto;
    }

    private AgentDetailDto mockDatabases() {
        HashMap<String, String> extendInfo = new HashMap<>();
        extendInfo.put(DatabaseConstants.VERSION, "V10.5.5.5");
        extendInfo.put(DatabaseConstants.DEPLOY_OS_KEY, "redhat");
        AppResource appResource = new AppResource();
        appResource.setExtendInfo(extendInfo);
        AgentDetailDto result = new AgentDetailDto();
        result.setResourceList(Collections.singletonList(appResource));
        return result;
    }

    private Optional<ProtectedResource> mockResource() {
        return Optional.ofNullable(queryClusterInstance().getRecords().get(0));
    }

    private PageListResponse<ProtectedResource> mockTablespace() {
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        response.setTotalCount(1);
        ProtectedResource tablespace = new ProtectedResource();
        tablespace.setName("tablespace");
        List<ProtectedResource> tablespaceList = new ArrayList<>();
        tablespaceList.add(tablespace);
        response.setRecords(tablespaceList);
        return response;
    }

    private ProtectedEnvironment mockEnv() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setSubType(ResourceSubTypeEnum.DB2_CLUSTER.getType());
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        List<ProtectedResource> resources = new ArrayList<>();
        ProtectedResource resource = new ProtectedResource();
        resources.add(resource);
        ProtectedResource resource1 = new ProtectedResource();
        resources.add(resource1);
        dependencies.put("agents", resources);
        environment.setDependencies(dependencies);
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(DatabaseConstants.CLUSTER_TYPE, "dpf");
        environment.setExtendInfo(extendInfo);
        return environment;
    }

    private BrowseEnvironmentResourceConditions mockConditions() {
        BrowseEnvironmentResourceConditions conditions = new BrowseEnvironmentResourceConditions();
        conditions.setPageNo(0);
        conditions.setPageSize(100);
        conditions.setResourceType(ResourceSubTypeEnum.DB2_TABLESPACE.getType());
        return conditions;
    }
}
