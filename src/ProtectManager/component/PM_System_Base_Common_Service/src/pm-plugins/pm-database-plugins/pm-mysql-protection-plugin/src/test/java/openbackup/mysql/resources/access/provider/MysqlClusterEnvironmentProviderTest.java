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
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyMap;

import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.exception.DataProtectionAccessException;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.service.InstanceResourceService;
import openbackup.mysql.resources.access.common.MysqlConstants;
import openbackup.mysql.resources.access.enums.MysqlResourceSubTypeEnum;
import openbackup.mysql.resources.access.service.MysqlBaseService;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.host.HostRestApi;
import openbackup.system.base.sdk.host.model.Host;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;

import com.google.common.collect.ImmutableMap;
import com.google.common.collect.Lists;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.UUID;

/**
 * MySQL集群环境测试类
 *
 * @author xWX950025
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-01
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(EnvironmentLinkStatusHelper.class)
public class MysqlClusterEnvironmentProviderTest {
    private final MysqlBaseService mysqlBaseService = Mockito.mock(MysqlBaseService.class);

    private final ProviderManager providerManager = Mockito.mock(ProviderManager.class);

    private final ResourceService resourceService = Mockito.mock(ResourceService.class) ;

    private final MysqlDatabaseScanner mysqlDatabaseScanner = Mockito.mock(MysqlDatabaseScanner.class);

    private final PluginConfigManager pluginConfigManager = Mockito.mock(PluginConfigManager.class);

    private InstanceResourceService instanceResourceService = Mockito.mock(InstanceResourceService.class);

    private MysqlClusterEnvironmentProvider mySQLClusterEnvironmentProvider;

    @Mock
    private MysqlInstanceProvider mysqlInstanceProvider;

    @Mock
    private HostRestApi hostRestApi;

    @Rule
    public ExpectedException expectedException = ExpectedException.none();

    @Before
    public void init() {
        mySQLClusterEnvironmentProvider = new MysqlClusterEnvironmentProvider(
            providerManager, pluginConfigManager, resourceService, mysqlDatabaseScanner, mysqlBaseService);
        mySQLClusterEnvironmentProvider.setInstanceResourceService(instanceResourceService);
        mySQLClusterEnvironmentProvider.setHostRestApi(hostRestApi);
        mySQLClusterEnvironmentProvider.setMysqlInstanceProvider(mysqlInstanceProvider);
    }

    /**
     * 用例场景：mysql集群环境检查类provider过滤
     * 前置条件：无
     * 检查点：类过滤成功或失败
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(mySQLClusterEnvironmentProvider.applicable(MysqlResourceSubTypeEnum.MYSQL_CLUSTER.getType()));
    }

    /**
     * 用例场景：健康检查
     * 前置条件：无
     * 检查点: 检查成功
     */
    @Test
    public void health_check_success() {
        ProtectedEnvironment environment = getClusterEnvironment();
        mockClusterAgentEnv(environment, LinkStatusEnum.ONLINE);
        PowerMockito.mockStatic(EnvironmentLinkStatusHelper.class);
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any()))
            .thenReturn(LinkStatusEnum.ONLINE.getStatus().toString());
        PowerMockito.when(resourceService.query(anyInt(),anyInt(),anyMap())).thenReturn(mockInstance());
        PowerMockito.when(hostRestApi.queryHostByID(any())).thenReturn(getHostInfo());
        PowerMockito.doNothing().when(mysqlInstanceProvider).healthCheck(any());
        mySQLClusterEnvironmentProvider.validate(environment);
        Mockito.verify(hostRestApi, Mockito.times(1)).queryHostByID(any());
    }

    /**
     * 用例场景：健康检查
     * 前置条件：联通性校验失败
     * 检查点: 抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_when_health_check() {
        expectedException.expect(DataProtectionAccessException.class);
        ProtectedEnvironment environment = getClusterEnvironment();
        PowerMockito.when(resourceService.query(anyInt(),anyInt(),anyMap())).thenReturn(mockInstance());
        PowerMockito.when(hostRestApi.queryHostByID(any())).thenReturn(getHostInfo());
        PowerMockito.doNothing().when(mysqlInstanceProvider).healthCheck(any());
        mockClusterAgentEnv(environment, LinkStatusEnum.OFFLINE);
        PowerMockito.mockStatic(EnvironmentLinkStatusHelper.class);
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any()))
            .thenReturn(LinkStatusEnum.OFFLINE.getStatus().toString());
        mySQLClusterEnvironmentProvider.validate(environment);
    }

    /**
     * 用例场景：健康检查
     * 前置条件：联通性校验失败
     * 检查点: 抛出异常
     */
    @Test
    public void health_check_for_eapp_success() {
        expectedException.expect(DataProtectionAccessException.class);
        ProtectedEnvironment environment = getClusterEnvironment();
        environment.setExtendInfo(ImmutableMap.of(DatabaseConstants.CLUSTER_TYPE, MysqlConstants.EAPP));
        PowerMockito.when(resourceService.query(anyInt(),anyInt(),anyMap())).thenReturn(mockInstance());
        PowerMockito.when(hostRestApi.queryHostByID(any())).thenReturn(getHostInfo());
        PowerMockito.doNothing().when(mysqlInstanceProvider).healthCheck(any());
        mockClusterAgentEnv(environment, LinkStatusEnum.OFFLINE);
        PowerMockito.mockStatic(EnvironmentLinkStatusHelper.class);
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any()))
            .thenReturn(LinkStatusEnum.OFFLINE.getStatus().toString());
        mySQLClusterEnvironmentProvider.validate(environment);
    }

    /**
     * 用例场景 主机在线状态校验
     * 前置条件：主机正常注册，存在mysql资源，但是不重复
     * 检查点: 检查正常
     */
    @Test
    public void check_success_when_have_existed_mysql_res() {
        ProtectedEnvironment clusterEnvironment = getClusterEnvironment();
        mockClusterAgentEnv(clusterEnvironment, LinkStatusEnum.ONLINE);
        PowerMockito.mockStatic(EnvironmentLinkStatusHelper.class);
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any()))
            .thenReturn(LinkStatusEnum.ONLINE.getStatus().toString());
        PageListResponse<ProtectedResource> queryRes = new PageListResponse<>();
        ProtectedEnvironment environment = getClusterEnvironment();
        environment.getDependencies().get(DatabaseConstants.AGENTS).get(0).setUuid(UUID.randomUUID().toString());

        ProtectedEnvironment environmentTwo = getClusterEnvironment();
        environmentTwo.setUuid(UUID.randomUUID().toString());
        queryRes.setRecords(Arrays.asList(environment, environmentTwo));

        Map<String, List<ProtectedResource>> environmentTwoDep = new HashMap<>();
        ProtectedResource protectedResourceOne = new ProtectedResource();
        protectedResourceOne.setUuid(UUID.randomUUID().toString());
        List<ProtectedResource> environmentTwoDepList = Arrays.asList(protectedResourceOne);
        environmentTwoDep.put(DatabaseConstants.AGENTS, environmentTwoDepList);
        environmentTwo.setDependencies(environmentTwoDep);
        PowerMockito.when(mysqlBaseService.getEnvironmentById(environmentTwo.getUuid())).thenReturn(environmentTwo);

        ProtectedEnvironment existAgentEnv = getClusterEnvironment();
        List<ProtectedResource> existAgentResList = existAgentEnv.getDependencies().get(DatabaseConstants.AGENTS);
        for (ProtectedResource existAgentRes: existAgentResList) {
            existAgentRes.setUuid(UUID.randomUUID().toString());
        }
        PowerMockito.when(mysqlBaseService.getEnvironmentById(queryRes.getRecords().get(0).getUuid())).thenReturn(existAgentEnv);

        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(queryRes);
        mySQLClusterEnvironmentProvider.register(clusterEnvironment);
        Assert.assertEquals(LinkStatusEnum.ONLINE.getStatus().toString(), clusterEnvironment.getLinkStatus());
    }

    /**
     * 用例场景 主机在线状态校验
     * 前置条件：主机正常注册
     * 检查点: 检查正常，不存在mysql资源
     */
    @Test
    public void check_success_when_have_existed_mysql_res_and_no_repeat() {
        ProtectedEnvironment clusterEnvironment = getClusterEnvironment();
        mockClusterAgentEnv(clusterEnvironment, LinkStatusEnum.ONLINE);
        PowerMockito.mockStatic(EnvironmentLinkStatusHelper.class);
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any()))
            .thenReturn(LinkStatusEnum.ONLINE.getStatus().toString());
        PageListResponse<ProtectedResource> queryRes = new PageListResponse<>();
        ProtectedEnvironment environment = getClusterEnvironment();
        environment.getDependencies().get(DatabaseConstants.AGENTS).get(0).setUuid(UUID.randomUUID().toString());
        queryRes.setRecords(Collections.singletonList(environment));

        ProtectedEnvironment existAgentEnv = getClusterEnvironment();
        List<ProtectedResource> existAgentResList = existAgentEnv.getDependencies().get(DatabaseConstants.AGENTS);
        for (ProtectedResource existAgentRes: existAgentResList) {
            existAgentRes.setUuid(UUID.randomUUID().toString());
        }
        PowerMockito.when(mysqlBaseService.getEnvironmentById(queryRes.getRecords().get(0).getUuid())).thenReturn(existAgentEnv);

        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(queryRes);
        mySQLClusterEnvironmentProvider.register(clusterEnvironment);
        Assert.assertEquals(LinkStatusEnum.ONLINE.getStatus().toString(), clusterEnvironment.getLinkStatus());
    }

    /**
     * 用例场景 主机在线状态校验
     * 前置条件：主机正常注册，不存在mysql资源
     * 检查点: 检查正常
     */
    @Test
    public void check_success_when_do_not_have_existed_mysql_res() {
        ProtectedEnvironment clusterEnvironment = getClusterEnvironment();
        mockClusterAgentEnv(clusterEnvironment, LinkStatusEnum.ONLINE);
        PowerMockito.mockStatic(EnvironmentLinkStatusHelper.class);
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any()))
            .thenReturn(LinkStatusEnum.ONLINE.getStatus().toString());
        PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<ProtectedResource>();
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(pageListResponse);
        mySQLClusterEnvironmentProvider.register(clusterEnvironment);
        Assert.assertEquals(LinkStatusEnum.ONLINE.getStatus().toString(), clusterEnvironment.getLinkStatus());
    }
    /**
     * 用例场景 主机在线状态校验
     * 前置条件：主机正常注册
     * 检查点: 检查正常
     */
    @Test
    public void check_success_when_register_cluster() {
        ProtectedEnvironment clusterEnvironment = getClusterEnvironmentWhenRegister();
        mockClusterAgentEnv(clusterEnvironment, LinkStatusEnum.ONLINE);
        PowerMockito.mockStatic(EnvironmentLinkStatusHelper.class);
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any()))
            .thenReturn(LinkStatusEnum.ONLINE.getStatus().toString());
        PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<ProtectedResource>();
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(pageListResponse);
        mySQLClusterEnvironmentProvider.register(clusterEnvironment);
        Assert.assertEquals(LinkStatusEnum.ONLINE.getStatus().toString(), clusterEnvironment.getLinkStatus());
        Assert.assertNotNull(clusterEnvironment.getUuid());
    }
    /**
     * 用例场景 主机在线状态校验
     * 前置条件：存在重复的mysql资源
     * 检查点: 报错
     */
    @Test
    public void should_throw_LegoCheckedException_when_check_success_if_have_existed_mysql_res_and_repeat() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("Host is already in mysql cluster!");
        ProtectedEnvironment clusterEnvironment = getClusterEnvironment();
        mockClusterAgentEnv(clusterEnvironment, LinkStatusEnum.ONLINE);
        PowerMockito.mockStatic(EnvironmentLinkStatusHelper.class);
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any()))
            .thenReturn(LinkStatusEnum.ONLINE.getStatus().toString());
        PageListResponse<ProtectedResource> queryRes = new PageListResponse<>();
        ProtectedEnvironment environment = getClusterEnvironment();
        environment.getDependencies().get(DatabaseConstants.AGENTS).get(0).setUuid(UUID.randomUUID().toString());

        ProtectedEnvironment environmentTwo = getClusterEnvironment();
        environmentTwo.setUuid(UUID.randomUUID().toString());
        queryRes.setRecords(Arrays.asList(environment, environmentTwo));

        Map<String, List<ProtectedResource>> environmentTwoDep = new HashMap<>();
        ProtectedResource protectedResourceOne = new ProtectedResource();
        protectedResourceOne.setUuid("11111111111");
        List<ProtectedResource> environmentTwoDepList = Arrays.asList(protectedResourceOne);
        environmentTwoDep.put(DatabaseConstants.AGENTS, environmentTwoDepList);
        environmentTwo.setDependencies(environmentTwoDep);
        PowerMockito.when(mysqlBaseService.getEnvironmentById(environmentTwo.getUuid())).thenReturn(environmentTwo);

        ProtectedEnvironment existAgentEnv = getClusterEnvironment();
        List<ProtectedResource> existAgentResList = existAgentEnv.getDependencies().get(DatabaseConstants.AGENTS);
        for (ProtectedResource existAgentRes: existAgentResList) {
            existAgentRes.setUuid(UUID.randomUUID().toString());
        }
        PowerMockito.when(mysqlBaseService.getEnvironmentById(queryRes.getRecords().get(0).getUuid())).thenReturn(existAgentEnv);

        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(queryRes);
        mySQLClusterEnvironmentProvider.register(clusterEnvironment);
        Assert.assertEquals(LinkStatusEnum.ONLINE.getStatus().toString(), clusterEnvironment.getLinkStatus());
    }

    /**
     * 用例场景 主机在线状态校验
     * 前置条件：主机正常注册
     * 检查点: 检查正常，集群节点小于等于1个
     */
    @Test
    public void should_throw_LegoCheckedException_when_check_if_node_is_one() {
        expectedException.expect(LegoCheckedException.class);
        ProtectedEnvironment clusterEnvironment = getClusterEnvironment();
        clusterEnvironment.getDependencies().put(DatabaseConstants.AGENTS, new ArrayList<>());
        mySQLClusterEnvironmentProvider.register(clusterEnvironment);
    }

    /**
     * 用例场景 eapp节点数量超限
     * 前置条件：主机正常注册
     * 检查点: 检查正常，集群节点大于4个
     */
    @Test
    public void should_throw_LegoCheckedException_when_check_if_node_over_limit() {
        expectedException.expect(LegoCheckedException.class);
        ProtectedEnvironment clusterEnvironment = getClusterEnvironment();
        clusterEnvironment.setExtendInfo(ImmutableMap.of(DatabaseConstants.CLUSTER_TYPE, MysqlConstants.EAPP));
        ProtectedResource node = new ProtectedResource();
        clusterEnvironment.getDependencies()
            .put(DatabaseConstants.AGENTS, Lists.newArrayList(node, node, node, node, node));
        mySQLClusterEnvironmentProvider.register(clusterEnvironment);
    }

    /**
     * 用例场景 主机在线状态校验
     * 前置条件：主机正常注册
     * 检查点: 检查正常
     */
    @Test
    public void should_throw_LegoCheckedException_when_check() {
        expectedException.expect(DataProtectionAccessException.class);
        ProtectedEnvironment clusterEnvironment = getClusterEnvironment();
        mockClusterAgentEnv(clusterEnvironment, LinkStatusEnum.OFFLINE);
        PowerMockito.mockStatic(EnvironmentLinkStatusHelper.class);
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any()))
            .thenReturn(LinkStatusEnum.OFFLINE.getStatus().toString());
        mySQLClusterEnvironmentProvider.register(clusterEnvironment);
    }

    /**
     * 用例场景 扫描集群环境失败
     * 前置条件：主机正常注册，集群环境注册正常
     * 检查点: 扫描失败
     */
    @Test
    public void scan_success() {
        ProtectedEnvironment clusterEnvironment = getClusterEnvironment();
        ProtectedResource protectedResource = new ProtectedResource();
        HashMap<String, String> hashMap = new HashMap<>();
        protectedResource.setExtendInfo(hashMap);
        PageListResponse<ProtectedResource> result = new PageListResponse<>();
        result.setRecords(Collections.singletonList(protectedResource));
        PowerMockito.when(mysqlDatabaseScanner.scan(any())).thenReturn(Collections.singletonList(protectedResource));
        PowerMockito.when(mysqlBaseService.getEnvironmentById(any())).thenReturn(clusterEnvironment);
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(result);
        List<ProtectedResource> scan = mySQLClusterEnvironmentProvider.scan(clusterEnvironment);
        Assert.assertEquals(3, scan.size());
    }

    private void mockClusterAgentEnv(ProtectedEnvironment environment, LinkStatusEnum linkStatusEnum) {
        List<ProtectedResource> agentRes = environment.getDependencies().get(DatabaseConstants.AGENTS);
        for (ProtectedResource protectedResource: agentRes) {
            ProtectedEnvironment agentEnv = new ProtectedEnvironment();
            agentEnv.setLinkStatus(linkStatusEnum.getStatus().toString());
            PowerMockito.when(mysqlBaseService.getEnvironmentById(protectedResource.getUuid())).thenReturn(agentEnv);
        }
    }

    /**
     * 构建环境资源
     * 设置protectedEnvironment的uuid为2222
     *
     * @return 环境资源
     */
    private ProtectedEnvironment getClusterEnvironment() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        Map<String, List<ProtectedResource>> dependency = new HashMap<>();
        ProtectedResource protectedResourceOne = new ProtectedResource();
        protectedResourceOne.setUuid("11111111111");

        ProtectedResource protectedResourceTwo = new ProtectedResource();
        protectedResourceTwo.setUuid("22222222222");
        dependency.put(DatabaseConstants.AGENTS, Arrays.asList(protectedResourceOne, protectedResourceTwo));
        protectedEnvironment.setDependencies(dependency);
        protectedEnvironment.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        protectedEnvironment.setUuid("2222");
        return protectedEnvironment;
    }

    /**
     * 构建注册时的环境资源 设置protectedEnvironment的uuid为空
     *
     * @return 环境资源
     */
    private ProtectedEnvironment getClusterEnvironmentWhenRegister() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        Map<String, List<ProtectedResource>> dependency = new HashMap<>();
        ProtectedResource protectedResourceOne = new ProtectedResource();
        protectedResourceOne.setUuid("11111111111");

        ProtectedResource protectedResourceTwo = new ProtectedResource();
        protectedResourceTwo.setUuid("22222222222");
        dependency.put(DatabaseConstants.AGENTS, Arrays.asList(protectedResourceOne, protectedResourceTwo));
        protectedEnvironment.setDependencies(dependency);
        protectedEnvironment.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        return protectedEnvironment;
    }

    private PageListResponse<ProtectedResource> mockInstance() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid(UUID.randomUUID().toString());
        protectedResource.setAuth(new Authentication());
        protectedResource.setEnvironment(new ProtectedEnvironment());
        List<ProtectedResource> dataList = new ArrayList<>();
        dataList.add(protectedResource);
        PageListResponse<ProtectedResource> instances = new PageListResponse<>();
        instances.setRecords(dataList);
        return instances;
    }

    private Host getHostInfo() {
        Host host = new Host();
        host.setIp("8.40.0.1");
        host.setPort("3306");
        return host;
    }
}
