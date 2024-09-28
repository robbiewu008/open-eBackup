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
package openbackup.clickhouse.plugin.service.impl;

import openbackup.clickhouse.plugin.common.ExceptionMatcher;
import openbackup.clickhouse.plugin.constant.ClickHouseConstant;
import openbackup.clickhouse.plugin.provider.ClickHouseAgentProvider;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.NodeInfo;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.BrowseEnvironmentResourceConditions;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceBase;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;

import openbackup.system.base.common.constants.CommonErrorCode;
import com.huawei.oceanprotect.system.base.kerberos.service.KerberosService;
import openbackup.system.base.sdk.cluster.enums.ClusterEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import com.google.common.collect.ImmutableMap;
import com.google.common.collect.Lists;
import com.google.common.collect.Maps;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatchers;
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
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * ClickHouseServiceImpl Test
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest( {ClickHouseServiceImpl.class})
public class ClickHouseServiceImplTest {
    private ClickHouseServiceImpl clickHouseService;

    @Mock
    private ResourceService resourceService;

    @Mock
    private AgentUnifiedService agentUnifiedService;

    @Mock
    private ProtectedEnvironmentService protectedEnvironmentService;

    @Mock
    private KerberosService kerberosService;

    @Mock
    private EncryptorService encryptorService;

    @Before
    public void init() {
        clickHouseService = new ClickHouseServiceImpl(resourceService, agentUnifiedService,
            protectedEnvironmentService, kerberosService, encryptorService, new ClickHouseAgentProvider(
            resourceService));
    }

    /**
     * 预期异常
     */
    @Rule
    public ExpectedException expectedException = ExpectedException.none();

    /**
     * 用例场景：当资源有uuid时，调checkNodeExists直接返回，不做是否已添加过的校验
     * 前置条件：无
     * 检查点: 执行成功，无返回
     */
    @Test
    public void check_node_exists_success_if_has_uuid() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setExtendInfo(
            ImmutableMap.of(ClickHouseConstant.IP, ClickHouseConstant.IP, ClickHouseConstant.PORT,
                ClickHouseConstant.PORT));
        protectedResource.setUuid("123");
        ProtectedResource databaseProtectedResource = new ProtectedResource();
        databaseProtectedResource.setUuid("123");
        databaseProtectedResource.setExtendInfo(
            ImmutableMap.of(ClickHouseConstant.IP, ClickHouseConstant.IP, ClickHouseConstant.PORT,
                ClickHouseConstant.PORT));
        PowerMockito.when(resourceService.query(ArgumentMatchers.anyInt(), ArgumentMatchers.anyInt(), ArgumentMatchers.anyMap()))
            .thenReturn(new PageListResponse<>(0, Arrays.asList(databaseProtectedResource)));
        clickHouseService.checkNodeExists(protectedResource);
    }

    /**
     * 用例场景：当资源没有uuid时，校验是否已添加过，资源未添加过
     * 前置条件：无
     * 检查点: 执行成功
     */
    @Test
    public void check_node_exists_success_if_resource_does_not_exists() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setExtendInfo(
            ImmutableMap.of(ClickHouseConstant.IP, ClickHouseConstant.IP, ClickHouseConstant.PORT,
                ClickHouseConstant.PORT));
        PowerMockito.when(resourceService.query(ArgumentMatchers.anyInt(), ArgumentMatchers.anyInt(), ArgumentMatchers.anyMap()))
            .thenReturn(new PageListResponse<>(0, Arrays.asList()));
        clickHouseService.checkNodeExists(protectedResource);
    }

    /**
     * 用例场景：当资源没有uuid时，校验是否已添加过，资源已添加过
     * 前置条件：无
     * 检查点: 抛出异常，异常码：RESOURCE_REPEAT
     */
    @Test
    public void check_node_exists_fail_if_resource_exists() {
        expectedException.expect(new ExceptionMatcher(CommonErrorCode.RESOURCE_REPEAT, "The node already exists."));
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setExtendInfo(
            ImmutableMap.of(ClickHouseConstant.IP, ClickHouseConstant.IP, ClickHouseConstant.PORT,
                ClickHouseConstant.PORT));
        PowerMockito.when(resourceService.query(ArgumentMatchers.anyInt(), ArgumentMatchers.anyInt(), ArgumentMatchers.anyMap()))
            .thenReturn(new PageListResponse<>(1, Arrays.asList(new ProtectedResource())));
        clickHouseService.checkNodeExists(protectedResource);
    }

    /**
     * 用例场景：注册ClickHouse集群时，集群节点连接失败
     * 前置条件：无
     * 检查点: 抛出异常，异常码：CLICK_HOUSE_CONNECT_FAILED
     */
    @Test
    public void should_throw_LegoCheckedException_if_node_connect_failed_when_register_cluster() {
        expectedException.expect(
            new ExceptionMatcher(ClickHouseConstant.CLICK_HOUSE_CONNECT_FAILED, "ClickHouse node connect failed."));
        ProtectedEnvironment protectedResource = new ProtectedEnvironment();
        protectedResource.setName("192_168");
        protectedResource.setType(ClickHouseConstant.CLUSTER_TYPE);
        protectedResource.setSubType(ResourceSubTypeEnum.CLICK_HOUSE.getType());
        Authentication auth = new Authentication();
        auth.setExtendInfo(new HashMap<>());
        protectedResource.setAuth(auth);
        protectedResource.setExtendInfo(new HashMap<String, String>() {
            private static final long serialVersionUID = 2600531992340843610L;

            {
                put(ClickHouseConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(ClickHouseConstant.PORT, "127");
                put(ClickHouseConstant.IP, "192.164.1.4");
                put(ClickHouseConstant.CLIENT_PATH, "/srv/BigData/test");
            }
        });
        ProtectedResource son1 = new ProtectedResource();
        son1.setName("xxx1");
        son1.setType(ClickHouseConstant.NODE_TYPE);
        son1.setSubType(ResourceSubTypeEnum.CLICK_HOUSE.getType());
        son1.setExtendInfo(new HashMap<String, String>() {
            private static final long serialVersionUID = 6854815339967675723L;

            {
                put(ClickHouseConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(ClickHouseConstant.PORT, "127");
                put(ClickHouseConstant.IP, "192.164.1.4");
                put(ClickHouseConstant.CLIENT_PATH, "/srv/BigData/test");
                put(ClickHouseConstant.VERSION, "25.1");
            }
        });
        ProtectedResource son2 = new ProtectedResource();
        son2.setName("xxx2");
        son2.setExtendInfo(new HashMap<String, String>() {
            private static final long serialVersionUID = 2005811516188356546L;

            {
                put(ClickHouseConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(ClickHouseConstant.PORT, "127");
                put(ClickHouseConstant.IP, "192.164.1.4");
                put(ClickHouseConstant.CLIENT_PATH, "/srv/BigData/test");
            }
        });
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setPort(11);
        protectedEnvironment.setEndpoint("192.167.1.1");
        son1.setEnvironment(protectedEnvironment);
        son2.setEnvironment(protectedEnvironment);
        son2.setType(ClickHouseConstant.NODE_TYPE);
        son2.setSubType(ResourceSubTypeEnum.CLICK_HOUSE.getType());
        son1.setAuth(auth);
        ProtectedResource agent = new ProtectedResource();
        agent.setUuid("123");
        son1.setDependencies(new HashMap<String, List<ProtectedResource>>() {
            private static final long serialVersionUID = 1561291724760613983L;

            {
                put(DatabaseConstants.AGENTS, Lists.newArrayList(agent));
            }
        });
        son2.setDependencies(new HashMap<String, List<ProtectedResource>>() {
            private static final long serialVersionUID = -5998772378038662775L;

            {
                put(DatabaseConstants.AGENTS, Lists.newArrayList(agent));
            }
        });
        son2.setAuth(auth);
        protectedResource.setDependencies(ImmutableMap.of(ResourceConstants.CHILDREN, Arrays.asList(son1, son2)));
        PowerMockito.when(resourceService.query(ArgumentMatchers.eq(0), ArgumentMatchers.anyInt(), ArgumentMatchers.anyMap()))
            .thenReturn(new PageListResponse<>(0, Arrays.asList()));
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        response.setTotalCount(0);
        response.setRecords(Lists.newArrayList(son1));
        PowerMockito.when(agentUnifiedService.getDetailPageListNoRetry(ArgumentMatchers.anyString(), ArgumentMatchers.anyString(),
                ArgumentMatchers.anyInt(), ArgumentMatchers.any(), ArgumentMatchers.eq(false))).thenReturn(response);
        ProtectedEnvironment protectedenvironment = new ProtectedEnvironment();
        protectedenvironment.setEndpoint("172.1.4.4");
        protectedenvironment.setPort(1123);
        protectedEnvironment.setUserId("userId");
        PowerMockito.when(protectedEnvironmentService.getEnvironmentById(ArgumentMatchers.anyString()))
            .thenReturn(protectedenvironment);
        clickHouseService.preCheck(protectedResource);
    }

    /**
     * 用例场景：注册ClickHouse集群时，版本号错误
     * 前置条件：无
     * 检查点: 抛出异常，异常码：VERSION_ERROR
     */
    @Test
    public void should_throw_LegoCheckedException_if_version_is_error_when_register_cluster() {
        expectedException.expect(new ExceptionMatcher(CommonErrorCode.VERSION_ERROR, "Version id is error."));
        ProtectedEnvironment protectedResource01 = new ProtectedEnvironment();
        protectedResource01.setType(ClickHouseConstant.CLUSTER_TYPE);
        protectedResource01.setSubType(ResourceSubTypeEnum.CLICK_HOUSE.getType());
        Authentication auth = new Authentication();
        auth.setExtendInfo(new HashMap<>());
        protectedResource01.setName("192_168");
        protectedResource01.setAuth(auth);
        protectedResource01.setExtendInfo(new HashMap<String, String>() {
            private static final long serialVersionUID = 3467845971265910848L;

            {
                put(ClickHouseConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(ClickHouseConstant.PORT, "127");
                put(ClickHouseConstant.IP, "192.164.1.4");
                put(ClickHouseConstant.CLIENT_PATH, "/srv/BigData/test");
            }
        });
        ProtectedResource protectedResourceSon01 = new ProtectedResource();
        protectedResourceSon01.setName("xxx1");
        protectedResourceSon01.setType(ClickHouseConstant.NODE_TYPE);
        protectedResourceSon01.setSubType(ResourceSubTypeEnum.CLICK_HOUSE.getType());
        protectedResourceSon01.setExtendInfo(new HashMap<String, String>() {
            private static final long serialVersionUID = -3188415893500409112L;

            {
                put(ClickHouseConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(ClickHouseConstant.PORT, "127");
                put(ClickHouseConstant.IP, "192.164.1.4");
                put(ClickHouseConstant.VERSION, "1.0.2");
                put(ClickHouseConstant.CLIENT_PATH, "/srv/BigData/test");
            }
        });
        ProtectedResource son2 = new ProtectedResource();
        son2.setName("xxx2");
        son2.setExtendInfo(new HashMap<String, String>() {
            private static final long serialVersionUID = 5696469537826847135L;

            {
                put(ClickHouseConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(ClickHouseConstant.IP, "192.164.1.4");
                put(ClickHouseConstant.PORT, "127");
                put(ClickHouseConstant.CLIENT_PATH, "/srv/BigData/test");
            }
        });
        ProtectedEnvironment protectedEnvironment1 = new ProtectedEnvironment();
        protectedEnvironment1.setPort(11);
        protectedEnvironment1.setEndpoint("192.167.1.1");
        protectedResourceSon01.setEnvironment(protectedEnvironment1);
        son2.setEnvironment(protectedEnvironment1);
        son2.setType(ClickHouseConstant.NODE_TYPE);
        son2.setSubType(ResourceSubTypeEnum.CLICK_HOUSE.getType());
        ProtectedResource agent = new ProtectedResource();
        protectedResourceSon01.setAuth(auth);
        agent.setUuid("123");
        protectedResourceSon01.setDependencies(new HashMap<String, List<ProtectedResource>>() {
            private static final long serialVersionUID = 7092338675744819242L;

            {
                put(DatabaseConstants.AGENTS, Lists.newArrayList(agent));
            }
        });
        son2.setAuth(auth);
        PowerMockito.when(resourceService.query(ArgumentMatchers.eq(0), ArgumentMatchers.anyInt(), ArgumentMatchers.anyMap()))
            .thenReturn(new PageListResponse<>(0, Arrays.asList()));
        protectedResource01.setDependencies(
            ImmutableMap.of(ResourceConstants.CHILDREN, Arrays.asList(protectedResourceSon01, son2)));
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        response.setTotalCount(1);
        response.setRecords(Lists.newArrayList(protectedResourceSon01));
        PowerMockito.when(agentUnifiedService.getDetailPageListNoRetry(ArgumentMatchers.anyString(), ArgumentMatchers.anyString(),
                ArgumentMatchers.anyInt(), ArgumentMatchers.any(), ArgumentMatchers.eq(false))).thenReturn(response);
        ProtectedEnvironment protectedenvironment = new ProtectedEnvironment();
        protectedenvironment.setEndpoint("172.1.4.4");
        protectedenvironment.setPort(1123);
        protectedEnvironment1.setUserId("userId");
        PowerMockito.when(protectedEnvironmentService.getEnvironmentById(ArgumentMatchers.anyString()))
            .thenReturn(protectedenvironment);
        clickHouseService.preCheck(protectedResource01);
    }

    /**
     * 用例场景：注册ClickHouse集群时，集群节点连接失败
     * 前置条件：无
     * 检查点: 抛出异常，异常码：CLICK_HOUSE_CONNECT_FAILED
     */
    @Test
    public void should_throw_LegoCheckedException_if_node_connect_failed_exists_when_register_cluster() {
        expectedException.expect(
            new ExceptionMatcher(ClickHouseConstant.CLICK_HOUSE_CONNECT_FAILED, "ClickHouse node connect failed."));
        ProtectedEnvironment resource = new ProtectedEnvironment();
        resource.setName("192_168");
        resource.setSubType(ResourceSubTypeEnum.CLICK_HOUSE.getType());
        Authentication auth = new Authentication();
        auth.setExtendInfo(new HashMap<>());
        resource.setAuth(auth);
        resource.setType(ClickHouseConstant.CLUSTER_TYPE);
        resource.setExtendInfo(new HashMap<String, String>() {
            private static final long serialVersionUID = -6956858255207485103L;

            {
                put(ClickHouseConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(ClickHouseConstant.PORT, "127");
                put(ClickHouseConstant.IP, "192.164.1.4");
                put(ClickHouseConstant.CLIENT_PATH, "/srv/BigData/test");
            }
        });
        ProtectedResource son1 = new ProtectedResource();
        son1.setType(ClickHouseConstant.NODE_TYPE);
        son1.setName("xxx1");
        son1.setSubType(ResourceSubTypeEnum.CLICK_HOUSE.getType());
        son1.setExtendInfo(new HashMap<String, String>() {
            private static final long serialVersionUID = 7127163217111157362L;

            {
                put(ClickHouseConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(ClickHouseConstant.PORT, "127");
                put(ClickHouseConstant.IP, "192.164.1.4");
                put(ClickHouseConstant.QUERY_TYPE_VALUE_VERSION, "21.3.4.25");
                put(ClickHouseConstant.CLIENT_PATH, "/srv/BigData/test");
            }
        });
        ProtectedResource son2 = new ProtectedResource();
        son2.setName("xxx2");
        son2.setExtendInfo(new HashMap<String, String>() {
            private static final long serialVersionUID = -8068655196055823593L;

            {
                put(ClickHouseConstant.PORT, "127");
                put(ClickHouseConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(ClickHouseConstant.PORT, "127");
                put(ClickHouseConstant.CLIENT_PATH, "/srv/BigData/test");
            }
        });
        ProtectedEnvironment protectedEnvironment2 = new ProtectedEnvironment();
        protectedEnvironment2.setPort(11);
        protectedEnvironment2.setEndpoint("192.167.1.1");
        son1.setEnvironment(protectedEnvironment2);
        son2.setEnvironment(protectedEnvironment2);
        son2.setType(ClickHouseConstant.NODE_TYPE);
        son1.setAuth(auth);
        ProtectedResource agent = new ProtectedResource();
        agent.setUuid("123");
        son2.setSubType(ResourceSubTypeEnum.CLICK_HOUSE.getType());
        son1.setDependencies(new HashMap<String, List<ProtectedResource>>() {
            private static final long serialVersionUID = 8292979386922644755L;

            {
                put(DatabaseConstants.AGENTS, Lists.newArrayList(agent));
            }
        });
        son2.setAuth(auth);
        resource.setDependencies(ImmutableMap.of(ResourceConstants.CHILDREN, Arrays.asList(son1)));
        PowerMockito.when(resourceService.query(ArgumentMatchers.anyInt(), ArgumentMatchers.anyInt(), ArgumentMatchers.anyMap()))
            .thenReturn(new PageListResponse<>(0, Arrays.asList()));
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        response.setTotalCount(1);
        response.setRecords(Lists.newArrayList(son1));
        PowerMockito.when(agentUnifiedService.getDetailPageListNoRetry(ArgumentMatchers.anyString(), ArgumentMatchers.anyString(),
                ArgumentMatchers.anyInt(), ArgumentMatchers.any(), ArgumentMatchers.eq(false))).thenReturn(response);
        ProtectedEnvironment protectedenvironment = new ProtectedEnvironment();
        protectedenvironment.setEndpoint("172.1.4.4");
        protectedenvironment.setPort(1123);
        protectedEnvironment2.setUserId("userId");
        PowerMockito.when(protectedEnvironmentService.getEnvironmentById(ArgumentMatchers.anyString()))
            .thenReturn(protectedenvironment);
        clickHouseService.preCheck(resource);
    }

    /**
     * 用例场景：注册ClickHouse集群时，集群节点与实际不一致
     * 前置条件：无
     * 检查点: 抛出异常，异常码：ILLEGAL_PARAM
     */
    @Test
    public void should_throw_LegoCheckedException_if_nodes_are_inconsistent_when_register_cluster() {
        expectedException.expect(
            new ExceptionMatcher(CommonErrorCode.CLUSTER_NODES_INCONSISTENT, "Cluster nodes are inconsistent."));
        ProtectedEnvironment protectedResource = new ProtectedEnvironment();
        protectedResource.setName("192_168");
        protectedResource.setType(ClickHouseConstant.CLUSTER_TYPE);
        Authentication auth = new Authentication();
        auth.setExtendInfo(new HashMap<>());
        protectedResource.setAuth(auth);
        protectedResource.setSubType(ResourceSubTypeEnum.CLICK_HOUSE.getType());
        protectedResource.setExtendInfo(new HashMap<String, String>() {
            private static final long serialVersionUID = -7547468913772655704L;

            {
                put(ClickHouseConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(ClickHouseConstant.PORT, "127");
                put(ClickHouseConstant.IP, "192.164.1.4");
                put(ClickHouseConstant.CLIENT_PATH, "/srv/BigData/test");
            }
        });
        ProtectedResource son1 = new ProtectedResource();
        son1.setSubType(ResourceSubTypeEnum.CLICK_HOUSE.getType());
        son1.setName("xxx1");
        son1.setType(ClickHouseConstant.NODE_TYPE);
        son1.setExtendInfo(new HashMap<String, String>() {
            private static final long serialVersionUID = -6850584907935241490L;

            {
                put(ClickHouseConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(ClickHouseConstant.PORT, "127");
                put(ClickHouseConstant.IP, "192.164.1.4");
                put(ClickHouseConstant.QUERY_TYPE_VALUE_VERSION, "21.3.4.25");
                put(ClickHouseConstant.CLIENT_PATH, "/srv/BigData/test");
            }
        });
        ProtectedEnvironment protectedEnvironment3 = new ProtectedEnvironment();
        protectedEnvironment3.setPort(11);
        protectedEnvironment3.setEndpoint("192.167.1.1");
        son1.setEnvironment(protectedEnvironment3);
        son1.setAuth(auth);
        ProtectedResource agent = new ProtectedResource();
        agent.setUuid("123");
        son1.setDependencies(new HashMap<String, List<ProtectedResource>>() {
            private static final long serialVersionUID = 1186057745992558826L;

            {
                put(DatabaseConstants.AGENTS, Lists.newArrayList(agent));
            }
        });
        protectedResource.setDependencies(ImmutableMap.of(ResourceConstants.CHILDREN, Arrays.asList(son1)));
        PowerMockito.when(resourceService.query(ArgumentMatchers.anyInt(), ArgumentMatchers.anyInt(), ArgumentMatchers.anyMap()))
            .thenReturn(new PageListResponse<>(0, Arrays.asList()));
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        response.setTotalCount(1);
        response.setRecords(Lists.newArrayList(son1));
        PowerMockito.when(agentUnifiedService.getDetailPageListNoRetry(ArgumentMatchers.anyString(), ArgumentMatchers.anyString(),
                ArgumentMatchers.anyInt(), ArgumentMatchers.any(), ArgumentMatchers.eq(false))).thenReturn(response);
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setEndpoint("172.1.4.4");
        environment.setPort(1123);
        protectedEnvironment3.setUserId("userId");
        PowerMockito.when(protectedEnvironmentService.getEnvironmentById(ArgumentMatchers.anyString()))
            .thenReturn(environment);
        AppEnvResponse appEnvResponse = new AppEnvResponse();
        List<NodeInfo> nodes = new ArrayList<>();
        NodeInfo nodeInfo0 = new NodeInfo();
        nodeInfo0.setExtendInfo(
            ImmutableMap.of(ClickHouseConstant.IP, "192.168.1.1", ClickHouseConstant.HOST_ADDRESS, "192.168.1.1",
                ClickHouseConstant.PORT, "8086"));
        NodeInfo nodeInfo1 = new NodeInfo();
        nodeInfo1.setExtendInfo(
            ImmutableMap.of(ClickHouseConstant.IP, "192.168.1.1", ClickHouseConstant.HOST_ADDRESS, "192.168.1.1",
                ClickHouseConstant.PORT, "8088"));
        nodes.add(nodeInfo1);
        nodes.add(nodeInfo0);
        appEnvResponse.setNodes(nodes);
        PageListResponse<ProtectedResource> result1 = new PageListResponse<>();
        result1.setTotalCount(1);
        ProtectedResource resultProtectedResource1 = new ProtectedResource();
        resultProtectedResource1.setExtendInfo(
            ImmutableMap.of(ClickHouseConstant.QUERY_TYPE_VALUE_VERSION, "21.3.4.25"));
        result1.setRecords(Lists.newArrayList(resultProtectedResource1));
        PowerMockito.when(agentUnifiedService.getClusterInfoNoRetry(Mockito.any(ProtectedResource.class),
            Mockito.any(ProtectedEnvironment.class))).thenReturn(appEnvResponse);
        clickHouseService.preCheck(protectedResource);
    }

    /**
     * 用例场景：查询节点详情信息时，节点服务挂了
     * 前置条件：无
     * 检查点: 抛出异常，异常码：CLICK_HOUSE_CONNECT_FAILED
     */
    @Test
    public void should_throw_LegoCheckedException_if_node_connect_failed() {
        expectedException.expect(
            new ExceptionMatcher(ClickHouseConstant.CLICK_HOUSE_CONNECT_FAILED, "ClickHouse node connect failed."));
        Authentication auth = new Authentication();
        auth.setAuthType(Authentication.NO_AUTH);
        ProtectedResource resource = new ProtectedResource();
        resource.setAuth(auth);
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(DatabaseConstants.EXTEND_INFO_KEY_KERBEROS_ID, "x");
        resource.setExtendInfo(extendInfo);
        clickHouseService.queryClusterDetail(new ProtectedEnvironment(), resource, "databases", "wqj", null);
    }

    /**
     * 用例场景：注册ClickHouse集群时，集群参数正确
     * 前置条件：无
     * 检查点: 检查成功，无异常
     */
    @Test
    public void preCheck_success() {
        ProtectedEnvironment protectedResource = new ProtectedEnvironment();
        protectedResource.setName("192_168");
        protectedResource.setSubType(ResourceSubTypeEnum.CLICK_HOUSE.getType());
        protectedResource.setType(ClickHouseConstant.CLUSTER_TYPE);
        Authentication auth = new Authentication();
        auth.setExtendInfo(new HashMap<>());
        protectedResource.setAuth(auth);
        protectedResource.setExtendInfo(new HashMap<String, String>() {
            private static final long serialVersionUID = -7622378606902422971L;

            {
                put(ClickHouseConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(ClickHouseConstant.IP, "192.164.1.4");
                put(ClickHouseConstant.PORT, "127");
                put(ClickHouseConstant.CLIENT_PATH, "/srv/BigData/test");
            }
        });
        ProtectedResource son1 = new ProtectedResource();
        son1.setName("xxx1");
        son1.setSubType(ResourceSubTypeEnum.CLICK_HOUSE.getType());
        son1.setType(ClickHouseConstant.NODE_TYPE);
        son1.setExtendInfo(new HashMap<String, String>() {
            private static final long serialVersionUID = -7178604864389077393L;

            {
                put(ClickHouseConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(ClickHouseConstant.IP, "192.164.1.4");
                put(ClickHouseConstant.QUERY_TYPE_VALUE_VERSION, "21.3.4.25");
                put(ClickHouseConstant.PORT, "127");
                put(ClickHouseConstant.CLIENT_PATH, "/srv/BigData/test");
            }
        });
        ProtectedEnvironment protectedEnvironment4 = new ProtectedEnvironment();
        protectedEnvironment4.setPort(11);
        protectedEnvironment4.setEndpoint("192.167.1.1");
        son1.setEnvironment(protectedEnvironment4);
        son1.setAuth(auth);
        ProtectedResource remoteAgent = new ProtectedResource();
        remoteAgent.setUuid("123");
        son1.setDependencies(new HashMap<String, List<ProtectedResource>>() {
            private static final long serialVersionUID = 4142610250593060721L;

            {
                put(DatabaseConstants.AGENTS, Lists.newArrayList(remoteAgent));
            }
        });
        protectedResource.setDependencies(ImmutableMap.of(ResourceConstants.CHILDREN, Arrays.asList(son1)));
        PowerMockito.when(resourceService.query(ArgumentMatchers.anyInt(), ArgumentMatchers.anyInt(), ArgumentMatchers.anyMap()))
            .thenReturn(new PageListResponse<>(0, Arrays.asList()));
        PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<>();
        pageListResponse.setTotalCount(1);
        pageListResponse.setRecords(Lists.newArrayList(son1));
        PowerMockito.when(agentUnifiedService.getDetailPageListNoRetry(ArgumentMatchers.anyString(), ArgumentMatchers.anyString(),
                    ArgumentMatchers.anyInt(), ArgumentMatchers.any(), ArgumentMatchers.eq(false)))
            .thenReturn(pageListResponse);
        ProtectedEnvironment cluster = new ProtectedEnvironment();
        cluster.setEndpoint("172.1.4.4");
        cluster.setPort(1123);
        protectedEnvironment4.setUserId("userId");
        PowerMockito.when(protectedEnvironmentService.getEnvironmentById(ArgumentMatchers.anyString()))
            .thenReturn(cluster);
        AppEnvResponse appEnvResponse = new AppEnvResponse();
        NodeInfo nodeInfo0 = new NodeInfo();
        nodeInfo0.setExtendInfo(
            ImmutableMap.of(ClickHouseConstant.HOST_ADDRESS, "192.164.1.4", ClickHouseConstant.PORT, "127"));

        PageListResponse<ProtectedResource> result1 = new PageListResponse<>();
        result1.setTotalCount(1);
        ProtectedResource resultProtectedResource1 = new ProtectedResource();
        resultProtectedResource1.setExtendInfo(
            ImmutableMap.of(ClickHouseConstant.QUERY_TYPE_VALUE_VERSION, "21.3.4.25"));
        result1.setRecords(Lists.newArrayList(resultProtectedResource1));
        appEnvResponse.setNodes(Arrays.asList(nodeInfo0));
        PowerMockito.when(agentUnifiedService.getClusterInfoNoRetry(Mockito.any(ProtectedResource.class),
            Mockito.any(ProtectedEnvironment.class))).thenReturn(appEnvResponse);
        clickHouseService.preCheck(protectedResource);
    }

    /**
     * 用例场景：健康检查成功
     * 前置条件：无
     * 检查点: 校验成功
     */
    @Test
    public void healthCheckWithResultStatus() {
        ProtectedEnvironment resource = new ProtectedEnvironment();
        resource.setUuid("12221212");
        resource.setName("192_168");
        resource.setType(ClickHouseConstant.CLUSTER_TYPE);
        resource.setSubType(ResourceSubTypeEnum.CLICK_HOUSE.getType());
        resource.setExtendInfo(new HashMap<String, String>() {
            private static final long serialVersionUID = -5469401308768521661L;

            {
                put(ClickHouseConstant.TYPE, DatabaseConstants.CLUSTER_TARGET);
            }
        });
        ProtectedResource son1 = new ProtectedResource();
        son1.setName("xxx1");
        son1.setType(ClickHouseConstant.NODE_TYPE);
        son1.setSubType(ResourceSubTypeEnum.CLICK_HOUSE.getType());
        son1.setUuid("123");
        son1.setExtendInfo(new HashMap<String, String>() {
            private static final long serialVersionUID = -7348427913702406070L;

            {
                put(ClickHouseConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(ClickHouseConstant.PORT, "127");
                put(ClickHouseConstant.IP, "192.164.1.4");
                put(ClickHouseConstant.CLIENT_PATH, "/srv/BigData/test");
                put(DatabaseConstants.STATUS, "27");
            }
        });
        ProtectedResource son2 = new ProtectedResource();
        son2.setName("xxx1");
        son2.setUuid("456");
        Authentication auth = new Authentication();
        auth.setAuthType(Authentication.NO_AUTH);
        auth.setExtendInfo(Maps.newHashMap());
        son1.setAuth(auth);
        son2.setAuth(auth);
        son1.setDependencies(Collections.singletonMap(DatabaseConstants.AGENTS, Lists.newArrayList(son2)));
        resource.setDependencies(Collections.singletonMap(ResourceConstants.CHILDREN, Lists.newArrayList(son1)));
        PowerMockito.when(resourceService.check(Mockito.any(ProtectedResource.class)))
            .thenReturn(new ActionResult[] {new ActionResult(0, "")});
        AppEnvResponse appEnvResponse = new AppEnvResponse();
        NodeInfo nodeInfo0 = new NodeInfo();
        nodeInfo0.setExtendInfo(ImmutableMap.of(ClickHouseConstant.IP, "192.168.1.1", ClickHouseConstant.PORT, "8086"));
        NodeInfo nodeInfo1 = new NodeInfo();
        nodeInfo1.setExtendInfo(ImmutableMap.of(ClickHouseConstant.IP, "192.168.1.1", ClickHouseConstant.PORT, "8088"));
        PageListResponse<ProtectedResource> result1 = new PageListResponse<>();
        result1.setTotalCount(1);
        ProtectedResource resultProtectedResource1 = new ProtectedResource();
        resultProtectedResource1.setExtendInfo(
            ImmutableMap.of(ClickHouseConstant.QUERY_TYPE_VALUE_VERSION, "21.3.4.25"));
        result1.setRecords(Lists.newArrayList(resultProtectedResource1));
        ProtectedEnvironment protectiveness1 = new ProtectedEnvironment();
        protectiveness1.setEndpoint("172.1.4.4");
        protectiveness1.setPort(1123);
        PowerMockito.when(protectedEnvironmentService.getEnvironmentById(ArgumentMatchers.anyString()))
            .thenReturn(protectiveness1);
        appEnvResponse.setNodes(Arrays.asList(nodeInfo1, nodeInfo0));
        PowerMockito.when(agentUnifiedService.getClusterInfoNoRetry(Mockito.any(ProtectedResource.class),
            Mockito.any(ProtectedEnvironment.class))).thenReturn(appEnvResponse);
        Optional<String> statusOptional = clickHouseService.healthCheckWithResultStatus(resource, true);
        Assert.assertEquals(statusOptional.get(), String.valueOf(ClusterEnum.StatusEnum.NORMAL.getStatus()));
    }

    /**
     * 用例场景：集群一个节点，并且节点离线，集群也是离线状态
     * 前置条件：创建前检查
     * 检查点: 报错，错误码：HOST_OFFLINE
     */
    @Test
    public void should_throw_LegoCheckedException_if_cluster_is_offline() {
        expectedException.expect(
            new ExceptionMatcher(CommonErrorCode.HOST_OFFLINE, "Protected environment is offLine!"));
        ProtectedEnvironment cluster = new ProtectedEnvironment();
        cluster.setUuid("12221212");
        cluster.setName("192_168");
        cluster.setType(ClickHouseConstant.CLUSTER_TYPE);
        cluster.setSubType(ResourceSubTypeEnum.CLICK_HOUSE.getType());
        cluster.setExtendInfo(new HashMap<String, String>() {
            private static final long serialVersionUID = -8958874757404362726L;

            {
                put(ClickHouseConstant.TYPE, DatabaseConstants.CLUSTER_TARGET);
            }
        });
        ProtectedResource child = new ProtectedResource();
        child.setName("xxx1");
        child.setType(ClickHouseConstant.NODE_TYPE);
        child.setSubType(ResourceSubTypeEnum.CLICK_HOUSE.getType());
        child.setUuid("123");
        child.setExtendInfo(new HashMap<String, String>() {
            private static final long serialVersionUID = 4390178156661606001L;

            {
                put(ClickHouseConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(ClickHouseConstant.PORT, "127");
                put(ClickHouseConstant.IP, "192.164.1.4");
                put(ClickHouseConstant.CLIENT_PATH, "/srv/BigData/test");
                put(DatabaseConstants.STATUS, "27");
            }
        });
        ProtectedResource agent = new ProtectedResource();
        agent.setName("xxx1");
        agent.setUuid("456");
        Authentication auth = new Authentication();
        auth.setAuthType(Authentication.NO_AUTH);
        child.setAuth(auth);
        agent.setAuth(auth);
        child.setDependencies(Collections.singletonMap(DatabaseConstants.AGENTS, Lists.newArrayList(agent)));
        cluster.setDependencies(Collections.singletonMap(ResourceConstants.CHILDREN, Lists.newArrayList(child)));
        PowerMockito.when(resourceService.check(Mockito.any(ProtectedResource.class)))
            .thenReturn(new ActionResult[] {new ActionResult(1221212, "")});
        ProtectedEnvironment protectiveness1 = new ProtectedEnvironment();
        protectiveness1.setEndpoint("172.1.4.4");
        protectiveness1.setPort(1123);
        PowerMockito.when(protectedEnvironmentService.getEnvironmentById(ArgumentMatchers.anyString()))
            .thenReturn(protectiveness1);

        NodeInfo nodeInfo0 = new NodeInfo();
        nodeInfo0.setExtendInfo(ImmutableMap.of(ClickHouseConstant.IP, "192.168.1.1", ClickHouseConstant.PORT, "8086"));
        NodeInfo nodeInfo1 = new NodeInfo();
        nodeInfo1.setExtendInfo(ImmutableMap.of(ClickHouseConstant.IP, "192.168.1.1", ClickHouseConstant.PORT, "8088"));

        AppEnvResponse appEnvResponse = new AppEnvResponse();

        appEnvResponse.setNodes(Arrays.asList(nodeInfo1, nodeInfo0));

        PowerMockito.when(agentUnifiedService.getClusterInfoNoRetry(Mockito.any(ProtectedResource.class),
            Mockito.any(ProtectedEnvironment.class))).thenReturn(appEnvResponse);

        clickHouseService.healthCheckWithResultStatus(cluster, true);
    }

    /**
     * 扫描集群下的数据库
     */
    @Test
    public void scanDatabases() {
        ProtectedEnvironment cluster = new ProtectedEnvironment();
        cluster.setName("192_168");
        cluster.setType(ClickHouseConstant.CLUSTER_TYPE);
        cluster.setSubType(ResourceSubTypeEnum.CLICK_HOUSE.getType());
        Authentication auth = new Authentication();
        cluster.setAuth(auth);
        cluster.setExtendInfo(new HashMap<String, String>() {
            private static final long serialVersionUID = -5611344849253893546L;

            {
                put(ClickHouseConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(ClickHouseConstant.PORT, "127");
                put(ClickHouseConstant.IP, "192.164.1.4");
                put(ClickHouseConstant.CLIENT_PATH, "/srv/BigData/test");
            }
        });
        ProtectedResource node1 = new ProtectedResource();
        node1.setName("xxx1");
        node1.setType(ClickHouseConstant.NODE_TYPE);
        node1.setSubType(ResourceSubTypeEnum.CLICK_HOUSE.getType());
        node1.setUuid("123");
        node1.setExtendInfo(new HashMap<String, String>() {
            private static final long serialVersionUID = 8178590797768694516L;

            {
                put(ClickHouseConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(ClickHouseConstant.PORT, "127");
                put(ClickHouseConstant.IP, "192.164.1.4");
                put(ClickHouseConstant.QUERY_TYPE_VALUE_VERSION, "21.3.4.25");
                put(ClickHouseConstant.CLIENT_PATH, "/srv/BigData/test");
            }
        });

        ProtectedResource agent1 = new ProtectedResource();
        agent1.setName("xxx2");
        agent1.setUuid("456");
        agent1.setExtendInfo(new HashMap<String, String>() {
            private static final long serialVersionUID = -2880213776526176242L;

            {
                put(ClickHouseConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(ClickHouseConstant.PORT, "127");
                put(ClickHouseConstant.IP, "192.164.1.4");
                put(ClickHouseConstant.CLIENT_PATH, "/srv/BigData/test");
            }
        });
        ProtectedEnvironment agentEnv1 = new ProtectedEnvironment();
        agentEnv1.setPort(11);
        agentEnv1.setEndpoint("192.167.1.1");
        node1.setEnvironment(agentEnv1);
        agent1.setEnvironment(agentEnv1);
        agent1.setType(ClickHouseConstant.NODE_TYPE);
        agent1.setSubType(ResourceSubTypeEnum.CLICK_HOUSE.getType());
        node1.setAuth(auth);
        node1.setDependencies(new HashMap<String, List<ProtectedResource>>() {
            private static final long serialVersionUID = -5982701759021390493L;

            {
                put(DatabaseConstants.AGENTS, Lists.newArrayList(agent1));
            }
        });
        agent1.setAuth(auth);

        cluster.setDependencies(ImmutableMap.of(ResourceConstants.CHILDREN, Arrays.asList(node1)));
        PowerMockito.when(resourceService.query(ArgumentMatchers.anyInt(), ArgumentMatchers.anyInt(), ArgumentMatchers.anyMap()))
            .thenReturn(new PageListResponse<>(0, Arrays.asList()));

        ProtectedResource detail1 = new ProtectedResource();
        ProtectedResource detail2 = new ProtectedResource();
        ProtectedResource detail3 = new ProtectedResource();
        detail1.setExtendInfo(ImmutableMap.of(ClickHouseConstant.DB_NAME, "db001"));
        detail2.setExtendInfo(ImmutableMap.of(ClickHouseConstant.DB_NAME, "db002"));
        detail3.setExtendInfo(ImmutableMap.of(ClickHouseConstant.DB_NAME, "db003"));

        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        response.setTotalCount(1);
        response.setRecords(Lists.newArrayList(detail1, detail2, detail3));
        PowerMockito.when(agentUnifiedService.getDetailPageListNoRetry(ArgumentMatchers.anyString(), ArgumentMatchers.anyString(),
                ArgumentMatchers.anyInt(), ArgumentMatchers.any(), ArgumentMatchers.eq(false))).thenReturn(response);
        ProtectedEnvironment protectedenvironment = new ProtectedEnvironment();
        protectedenvironment.setEndpoint("172.1.4.4");
        protectedenvironment.setPort(1123);
        PowerMockito.when(protectedEnvironmentService.getEnvironmentById(ArgumentMatchers.anyString()))
            .thenReturn(protectedenvironment);

        String databaseNames = clickHouseService.scanDataBases(cluster)
            .stream()
            .map(ResourceBase::getName)
            .sorted()
            .collect(Collectors.joining(","));
        Assert.assertEquals("db001,db002,db003", databaseNames);
    }

    /**
     * 查询表
     */
    @Test
    public void browseTables() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setName("192_168");
        environment.setType(ClickHouseConstant.CLUSTER_TYPE);
        environment.setSubType(ResourceSubTypeEnum.CLICK_HOUSE.getType());
        Authentication auth = new Authentication();
        environment.setAuth(auth);
        environment.setExtendInfo(new HashMap<String, String>() {
            private static final long serialVersionUID = 6169156333957087058L;

            {
                put(ClickHouseConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(ClickHouseConstant.PORT, "127");
                put(ClickHouseConstant.IP, "192.164.1.4");
                put(ClickHouseConstant.CLIENT_PATH, "/srv/BigData/test");
            }
        });
        ProtectedResource child1 = new ProtectedResource();
        child1.setName("xxx1");
        child1.setType(ClickHouseConstant.NODE_TYPE);
        child1.setSubType(ResourceSubTypeEnum.CLICK_HOUSE.getType());
        child1.setUuid("123");
        child1.setExtendInfo(new HashMap<String, String>() {
            private static final long serialVersionUID = 2784470566565582636L;

            {
                put(ClickHouseConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(ClickHouseConstant.PORT, "127");
                put(ClickHouseConstant.IP, "192.164.1.4");
                put(ClickHouseConstant.QUERY_TYPE_VALUE_VERSION, "21.3.4.25");
                put(ClickHouseConstant.CLIENT_PATH, "/srv/BigData/test");
            }
        });

        ProtectedResource agent01 = new ProtectedResource();
        agent01.setName("xxx2");
        agent01.setUuid("456");
        agent01.setExtendInfo(new HashMap<String, String>() {
            private static final long serialVersionUID = 5788004929473025617L;

            {
                put(ClickHouseConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(ClickHouseConstant.PORT, "127");
                put(ClickHouseConstant.IP, "192.164.1.4");
                put(ClickHouseConstant.CLIENT_PATH, "/srv/BigData/test");
            }
        });
        ProtectedEnvironment agentEnvironment = new ProtectedEnvironment();
        agentEnvironment.setPort(11);
        agentEnvironment.setEndpoint("192.167.1.1");
        child1.setEnvironment(agentEnvironment);
        agent01.setEnvironment(agentEnvironment);
        agent01.setType(ClickHouseConstant.NODE_TYPE);
        agent01.setSubType(ResourceSubTypeEnum.CLICK_HOUSE.getType());
        child1.setAuth(auth);
        child1.setDependencies(new HashMap<String, List<ProtectedResource>>() {
            private static final long serialVersionUID = 8713247775953505263L;

            {
                put(DatabaseConstants.AGENTS, Lists.newArrayList(agent01));
            }
        });
        agent01.setAuth(auth);

        environment.setDependencies(ImmutableMap.of(ResourceConstants.CHILDREN, Arrays.asList(child1)));
        PowerMockito.when(resourceService.query(ArgumentMatchers.anyInt(), ArgumentMatchers.anyInt(), ArgumentMatchers.anyMap()))
            .thenReturn(new PageListResponse<>(0, Arrays.asList()));

        ProtectedResource detail2 = new ProtectedResource();
        ProtectedResource detail3 = new ProtectedResource();
        ProtectedResource detail1 = new ProtectedResource();
        detail1.setName("table001");
        detail2.setName("table002");
        detail3.setName("table003");

        PageListResponse<ProtectedResource> ProtectedResourceResponse = new PageListResponse<>();
        ProtectedResourceResponse.setTotalCount(1);
        ProtectedResourceResponse.setRecords(Lists.newArrayList(detail1, detail2, detail3));
        PowerMockito.when(agentUnifiedService.getDetailPageListNoRetry(ArgumentMatchers.anyString(), ArgumentMatchers.anyString(),
                    ArgumentMatchers.anyInt(), ArgumentMatchers.any(), ArgumentMatchers.eq(false)))
            .thenReturn(ProtectedResourceResponse);
        ProtectedEnvironment protectedenvironment = new ProtectedEnvironment();
        protectedenvironment.setEndpoint("172.1.4.4");
        protectedenvironment.setPort(1123);
        PowerMockito.when(protectedEnvironmentService.getEnvironmentById(ArgumentMatchers.anyString()))
            .thenReturn(protectedenvironment);

        BrowseEnvironmentResourceConditions environmentResourceConditions = new BrowseEnvironmentResourceConditions();
        environmentResourceConditions.setParentId("parentId");
        ProtectedResource databaseResource = new ProtectedResource();
        databaseResource.setName("db001");
        PowerMockito.when(resourceService.getResourceById(ArgumentMatchers.anyString()))
            .thenReturn(Optional.of(databaseResource));

        String databaseNames = clickHouseService.browseTables(environment, environmentResourceConditions)
            .getRecords()
            .stream()
            .map(ResourceBase::getName)
            .sorted()
            .collect(Collectors.joining(","));
        Assert.assertEquals("table001,table002,table003", databaseNames);
    }
}
