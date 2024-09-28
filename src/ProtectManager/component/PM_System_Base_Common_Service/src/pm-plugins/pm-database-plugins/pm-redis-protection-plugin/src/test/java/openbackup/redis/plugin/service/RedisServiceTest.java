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
package openbackup.redis.plugin.service;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.NodeInfo;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import openbackup.redis.plugin.common.ExceptionMatcher;
import openbackup.redis.plugin.constant.RedisConstant;
import openbackup.redis.plugin.provider.RedisAgentProvider;
import openbackup.redis.plugin.service.impl.RedisServiceImpl;
import openbackup.system.base.common.constants.CommonErrorCode;
import com.huawei.oceanprotect.system.base.kerberos.service.KerberosService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;

import com.google.common.collect.ImmutableMap;
import com.google.common.collect.Lists;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatchers;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Optional;

/**
 * Redis Service Test
 *
 */
@RunWith(PowerMockRunner.class)
public class RedisServiceTest {
    private RedisService redisService;

    private ResourceService resourceService;

    private AgentUnifiedService agentUnifiedService;

    private KerberosService kerberosService;

    private EncryptorService encryptorService;

    private ProtectedEnvironmentService environmentService;

    /**
     * 预期异常
     */
    @Rule
    public ExpectedException expectedException = ExpectedException.none();

    @Before
    public void setUp() {
        resourceService = PowerMockito.mock(ResourceService.class);
        agentUnifiedService = PowerMockito.mock(AgentUnifiedService.class);
        kerberosService = PowerMockito.mock(KerberosService.class);
        encryptorService = PowerMockito.mock(EncryptorService.class);
        environmentService = PowerMockito.mock(ProtectedEnvironmentService.class);
        RedisAgentProvider redisAgentProvider = new RedisAgentProvider();
        redisService = new RedisServiceImpl(resourceService, agentUnifiedService, kerberosService, encryptorService,
            environmentService, redisAgentProvider);
    }

    /**
     * 用例场景：当资源有uuid时，调checkNodeExists直接返回，不做是否已添加过的校验
     * 前置条件：无
     * 检查点: 执行成功，无返回
     */
    @Test
    public void check_node_exists_success_if_has_uuid() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setExtendInfo(
            ImmutableMap.of(RedisConstant.IP, RedisConstant.IP, RedisConstant.PORT, RedisConstant.PORT));
        protectedResource.setUuid("123");
        ProtectedResource databaseProtectedResource = new ProtectedResource();
        databaseProtectedResource.setUuid("123");
        databaseProtectedResource.setExtendInfo(
            ImmutableMap.of(RedisConstant.IP, RedisConstant.IP, RedisConstant.PORT, RedisConstant.PORT));
        PowerMockito.when(
                resourceService.query(ArgumentMatchers.anyInt(), ArgumentMatchers.anyInt(), ArgumentMatchers.anyMap()))
            .thenReturn(new PageListResponse<>(0, Arrays.asList(databaseProtectedResource)));
        redisService.checkNodeExists(protectedResource);
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
            ImmutableMap.of(RedisConstant.IP, RedisConstant.IP, RedisConstant.PORT, RedisConstant.PORT));
        PowerMockito.when(
                resourceService.query(ArgumentMatchers.anyInt(), ArgumentMatchers.anyInt(), ArgumentMatchers.anyMap()))
            .thenReturn(new PageListResponse<>(0, Arrays.asList()));
        redisService.checkNodeExists(protectedResource);
    }

    /**
     * 用例场景：当资源没有uuid时，校验是否已添加过，资源已添加过
     * 前置条件：无
     * 检查点: 抛出异常，异常码：RESOURCE_REPEAT
     */
    @Test
    public void should_throw_LegoCheckedException_if_resource_exists_when_register_cluster() {
        expectedException.expect(new ExceptionMatcher(CommonErrorCode.NODE_REPEAT, "The node already exists."));
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setExtendInfo(
            ImmutableMap.of(RedisConstant.IP, RedisConstant.IP, RedisConstant.PORT, RedisConstant.PORT));
        PowerMockito.when(
                resourceService.query(ArgumentMatchers.anyInt(), ArgumentMatchers.anyInt(), ArgumentMatchers.anyMap()))
            .thenReturn(new PageListResponse<>(1, Arrays.asList(new ProtectedResource())));
        redisService.checkNodeExists(protectedResource);
    }

    /**
     * 用例场景：注册Redis集群时，集群节点连接失败
     * 前置条件：无
     * 检查点: 抛出异常，异常码：CONNECT_FAILED
     */
    @Test
    public void should_throw_LegoCheckedException_if_node_connect_failed_when_register_cluster() {
        expectedException.expect(
            new ExceptionMatcher(CommonErrorCode.REDIS_CONNECT_FAILED, "Redis node connect failed."));
        ProtectedEnvironment protectedResource = new ProtectedEnvironment();
        protectedResource.setName("192_168");
        protectedResource.setType(ResourceTypeEnum.DATABASE.getType());
        protectedResource.setSubType(ResourceSubTypeEnum.REDIS.getType());
        Authentication auth = new Authentication();
        auth.setExtendInfo(new HashMap<>());
        protectedResource.setAuth(auth);
        protectedResource.setExtendInfo(new HashMap<String, String>() {
            {
                put(RedisConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(RedisConstant.PORT, "127");
                put(RedisConstant.IP, "192.164.1.4");
                put(RedisConstant.CLIENT_PATH, "/srv/BigData/test");
            }
        });
        ProtectedResource son17311 = new ProtectedResource();
        son17311.setName("xxx1");
        son17311.setType(ResourceTypeEnum.DATABASE.getType());
        son17311.setSubType(ResourceSubTypeEnum.REDIS.getType());
        son17311.setExtendInfo(new HashMap<String, String>() {
            {
                put(RedisConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(RedisConstant.PORT, "127");
                put(RedisConstant.IP, "192.164.1.4");
                put(RedisConstant.CLIENT_PATH, "/srv/BigData/test");
            }
        });
        ProtectedResource son2 = new ProtectedResource();
        son2.setName("xxx2");
        son2.setExtendInfo(new HashMap<String, String>() {
            {
                put(RedisConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(RedisConstant.PORT, "127");
                put(RedisConstant.IP, "192.164.1.4");
                put(RedisConstant.CLIENT_PATH, "/srv/BigData/test");
            }
        });
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setPort(11);
        protectedEnvironment.setEndpoint("192.167.1.1");
        son17311.setEnvironment(protectedEnvironment);
        son2.setEnvironment(protectedEnvironment);
        son2.setType(ResourceTypeEnum.DATABASE.getType());
        son2.setSubType(ResourceSubTypeEnum.REDIS.getType());
        son17311.setAuth(auth);
        ProtectedResource agent = new ProtectedResource();
        agent.setUuid("123");
        agent.setEndpoint("10.10.10.10");
        agent.setPort(123);
        son17311.setDependencies(new HashMap<String, List<ProtectedResource>>() {
            {
                put(DatabaseConstants.AGENTS, Lists.newArrayList(agent));
            }
        });
        son2.setAuth(auth);
        PowerMockito.when(
                resourceService.query(ArgumentMatchers.eq(0), ArgumentMatchers.anyInt(), ArgumentMatchers.anyMap()))
            .thenReturn(new PageListResponse<>(0, Arrays.asList()));
        protectedResource.setDependencies(ImmutableMap.of(ResourceConstants.CHILDREN, Arrays.asList(son17311, son2)));
        PageListResponse<ProtectedResource> response001 = new PageListResponse<>();
        response001.setTotalCount(0);
        response001.setRecords(Lists.newArrayList(son17311));
        PowerMockito.when(
            agentUnifiedService.getDetailPageListNoRetry(ArgumentMatchers.anyString(), ArgumentMatchers.anyString(),
                ArgumentMatchers.anyInt(), ArgumentMatchers.any(), ArgumentMatchers.eq(false))).thenReturn(response001);
        ProtectedEnvironment protectedenvironment = new ProtectedEnvironment();
        protectedenvironment.setEndpoint("172.1.4.4");
        protectedenvironment.setPort(1123);
        protectedEnvironment.setUserId("userId");
        PowerMockito.when(environmentService.getEnvironmentById(ArgumentMatchers.anyString()))
            .thenReturn(protectedenvironment);
        redisService.preCheck(protectedResource);
    }

    /**
     * 用例场景：集群修改场景
     * 前置条件：无
     * 检查点: 流程正常走完
     */
    @Test
    public void update_success() {
        ProtectedEnvironment cluster = new ProtectedEnvironment();
        cluster.setLinkStatus("1");
        cluster.setUuid("123");
        cluster.setName("192_168");
        cluster.setType(ResourceTypeEnum.DATABASE.getType());
        cluster.setSubType(ResourceSubTypeEnum.REDIS.getType());
        Authentication auth = new Authentication();
        auth.setExtendInfo(new HashMap<>());
        cluster.setAuth(auth);
        cluster.setExtendInfo(new HashMap<String, String>() {
            {
                put(RedisConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(RedisConstant.PORT, "127");
                put(RedisConstant.IP, "192.164.1.4");
                put(RedisConstant.CLIENT_PATH, "/srv/BigData/test");
            }
        });
        ProtectedResource son17321 = new ProtectedResource();
        son17321.setUuid("12");
        son17321.setName("xxx1");
        son17321.setType(ResourceTypeEnum.DATABASE.getType());
        son17321.setSubType(ResourceSubTypeEnum.REDIS.getType());
        son17321.setExtendInfo(new HashMap<String, String>() {
            {
                put(RedisConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(RedisConstant.PORT, "127");
                put(RedisConstant.IP, "192.164.1.4");
                put(RedisConstant.CLIENT_PATH, "/srv/BigData/test");
                put("sslEnable", "true");
            }
        });
        son17321.setUuid("13");
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setPort(11);
        protectedEnvironment.setEndpoint("192.167.1.1");
        son17321.setEnvironment(protectedEnvironment);
        son17321.setAuth(auth);
        ProtectedResource agent = new ProtectedResource();
        agent.setUuid("123");
        agent.setEndpoint("10.10.10.10");
        agent.setPort(123);
        son17321.setDependencies(new HashMap<String, List<ProtectedResource>>() {
            {
                put(DatabaseConstants.AGENTS, Lists.newArrayList(agent));
            }
        });
        cluster.setDependencies(ImmutableMap.of(ResourceConstants.CHILDREN, Arrays.asList(son17321)));
        PowerMockito.when(
                resourceService.query(ArgumentMatchers.eq(0), ArgumentMatchers.anyInt(), ArgumentMatchers.anyMap()))
            .thenReturn(new PageListResponse<>(0, Arrays.asList(son17321)));
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        response.setTotalCount(0);
        response.setRecords(Lists.newArrayList(son17321));
        PowerMockito.when(
            agentUnifiedService.getDetailPageListNoRetry(ArgumentMatchers.anyString(), ArgumentMatchers.anyString(),
                ArgumentMatchers.anyInt(), ArgumentMatchers.any(), ArgumentMatchers.eq(false))).thenReturn(response);
        ProtectedEnvironment protectiveness01 = new ProtectedEnvironment();
        protectiveness01.setEndpoint("172.1.4.4");
        protectiveness01.setPort(1123);
        protectedEnvironment.setUserId("userId");
        PowerMockito.when(environmentService.getEnvironmentById(ArgumentMatchers.anyString()))
            .thenReturn(protectiveness01);
        PowerMockito.when(resourceService.getResourceById(ArgumentMatchers.eq(false), ArgumentMatchers.anyString()))
            .thenReturn(Optional.of(cluster));
        redisService.preCheck(cluster);
    }

    /**
     * 用例场景：注册Redis集群时，版本号错误
     * 前置条件：无
     * 检查点: 抛出异常，异常码：VERSION_ERROR
     */
    @Test
    public void should_throw_LegoCheckedException_if_version_is_error_when_register_cluster() {
        expectedException.expect(new ExceptionMatcher(CommonErrorCode.VERSION_ERROR, "Version id is error."));
        ProtectedEnvironment protectedResource17301 = new ProtectedEnvironment();
        protectedResource17301.setName("192_168");
        protectedResource17301.setType(ResourceTypeEnum.DATABASE.getType());
        protectedResource17301.setSubType(ResourceSubTypeEnum.REDIS.getType());
        Authentication auth = new Authentication();
        auth.setExtendInfo(new HashMap<>());
        protectedResource17301.setAuth(auth);
        protectedResource17301.setExtendInfo(new HashMap<String, String>() {
            {
                put(RedisConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(RedisConstant.PORT, "127");
                put(RedisConstant.IP, "192.164.1.4");
                put(RedisConstant.CLIENT_PATH, "/srv/BigData/test");
            }
        });
        ProtectedResource son17312 = new ProtectedResource();
        son17312.setName("xxx1");
        son17312.setType(ResourceTypeEnum.DATABASE.getType());
        son17312.setSubType(ResourceSubTypeEnum.REDIS.getType());
        son17312.setExtendInfo(new HashMap<String, String>() {
            {
                put(RedisConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(RedisConstant.PORT, "127");
                put(RedisConstant.IP, "192.164.1.4");
                put(RedisConstant.EXTEND_INFO_KEY_VERSION, "1.0.2");
                put(RedisConstant.CLIENT_PATH, "/srv/BigData/test");
            }
        });
        ProtectedResource son217331 = new ProtectedResource();
        son217331.setName("xxx2");
        son217331.setExtendInfo(new HashMap<String, String>() {
            {
                put(RedisConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(RedisConstant.PORT, "127");
                put(RedisConstant.IP, "192.164.1.4");
                put(RedisConstant.CLIENT_PATH, "/srv/BigData/test");
            }
        });
        ProtectedEnvironment protectedEnvironment2 = new ProtectedEnvironment();
        protectedEnvironment2.setPort(11);
        protectedEnvironment2.setEndpoint("192.167.1.1");
        son17312.setEnvironment(protectedEnvironment2);
        son217331.setEnvironment(protectedEnvironment2);
        son217331.setType(ResourceTypeEnum.DATABASE.getType());
        son217331.setSubType(ResourceSubTypeEnum.REDIS.getType());
        son17312.setAuth(auth);
        ProtectedResource agent = new ProtectedResource();
        agent.setUuid("123");
        agent.setEndpoint("10.10.10.10");
        agent.setPort(123);
        son217331.setAuth(auth);
        son17312.setDependencies(new HashMap<String, List<ProtectedResource>>() {
            {
                put(DatabaseConstants.AGENTS, Lists.newArrayList(agent));
            }
        });
        protectedResource17301.setDependencies(
            ImmutableMap.of(ResourceConstants.CHILDREN, Arrays.asList(son17312, son217331)));
        PowerMockito.when(
                resourceService.query(ArgumentMatchers.eq(0), ArgumentMatchers.anyInt(), ArgumentMatchers.anyMap()))
            .thenReturn(new PageListResponse<>(0, Arrays.asList()));
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        response.setTotalCount(1);
        response.setRecords(Lists.newArrayList(son17312));
        PowerMockito.when(
            agentUnifiedService.getDetailPageListNoRetry(ArgumentMatchers.anyString(), ArgumentMatchers.anyString(),
                ArgumentMatchers.anyInt(), ArgumentMatchers.any(), ArgumentMatchers.eq(false))).thenReturn(response);
        ProtectedEnvironment protectedenvironment = new ProtectedEnvironment();
        protectedenvironment.setPort(1123);
        protectedEnvironment2.setUserId("userId");
        protectedenvironment.setEndpoint("172.1.4.4");
        PowerMockito.when(environmentService.getEnvironmentById(ArgumentMatchers.anyString()))
            .thenReturn(protectedenvironment);
        redisService.preCheck(protectedResource17301);
    }

    /**
     * 用例场景：注册Redis集群时，集群节点连接失败
     * 前置条件：无
     * 检查点: 抛出异常，异常码：CONNECT_FAILED
     */
    @Test
    public void should_throw_LegoCheckedException_if_node_connect_failed_exists_when_register_cluster() {
        expectedException.expect(
            new ExceptionMatcher(CommonErrorCode.REDIS_CONNECT_FAILED, "Redis node connect failed."));
        ProtectedEnvironment clusterProtectedResource = new ProtectedEnvironment();
        clusterProtectedResource.setName("192_168");
        clusterProtectedResource.setType(ResourceTypeEnum.DATABASE.getType());
        clusterProtectedResource.setSubType(ResourceSubTypeEnum.REDIS.getType());
        Authentication auth = new Authentication();
        auth.setExtendInfo(new HashMap<>());
        clusterProtectedResource.setAuth(auth);
        clusterProtectedResource.setExtendInfo(new HashMap<String, String>() {
            {
                put(RedisConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(RedisConstant.PORT, "127");
                put(RedisConstant.IP, "192.164.1.4");
                put(RedisConstant.CLIENT_PATH, "/srv/BigData/test");
            }
        });
        ProtectedResource son1 = new ProtectedResource();
        son1.setName("xxx1");
        son1.setType(ResourceTypeEnum.DATABASE.getType());
        son1.setSubType(ResourceSubTypeEnum.REDIS.getType());
        son1.setExtendInfo(new HashMap<String, String>() {
            {
                put(RedisConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(RedisConstant.PORT, "127");
                put(RedisConstant.IP, "192.164.1.4");
                put(RedisConstant.EXTEND_INFO_KEY_VERSION, "5.0.2");
                put(RedisConstant.CLIENT_PATH, "/srv/BigData/test");
            }
        });
        ProtectedResource daughter = new ProtectedResource();
        daughter.setName("xxx2");
        daughter.setExtendInfo(new HashMap<String, String>() {
            {
                put(RedisConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(RedisConstant.PORT, "127");
                put(RedisConstant.IP, "192.164.1.4");
                put(RedisConstant.CLIENT_PATH, "/srv/BigData/test");
            }
        });
        ProtectedEnvironment protectedEnvironment3 = new ProtectedEnvironment();
        protectedEnvironment3.setPort(11);
        protectedEnvironment3.setEndpoint("192.167.1.1");
        son1.setEnvironment(protectedEnvironment3);
        daughter.setEnvironment(protectedEnvironment3);
        daughter.setType(ResourceTypeEnum.DATABASE.getType());
        daughter.setSubType(ResourceSubTypeEnum.REDIS.getType());
        son1.setAuth(auth);
        ProtectedResource agent = new ProtectedResource();
        agent.setUuid("123");
        agent.setEndpoint("10.10.10.10");
        agent.setPort(123);
        son1.setDependencies(new HashMap<String, List<ProtectedResource>>() {
            {
                put(DatabaseConstants.AGENTS, Lists.newArrayList(agent));
            }
        });
        daughter.setAuth(auth);
        clusterProtectedResource.setDependencies(ImmutableMap.of(ResourceConstants.CHILDREN, Arrays.asList(son1)));
        PowerMockito.when(
                resourceService.query(ArgumentMatchers.anyInt(), ArgumentMatchers.anyInt(), ArgumentMatchers.anyMap()))
            .thenReturn(new PageListResponse<>(0, Arrays.asList()));
        PageListResponse<ProtectedResource> response2 = new PageListResponse<>();
        response2.setTotalCount(1);
        response2.setRecords(Lists.newArrayList(son1));
        PowerMockito.when(
            agentUnifiedService.getDetailPageListNoRetry(ArgumentMatchers.anyString(), ArgumentMatchers.anyString(),
                ArgumentMatchers.anyInt(), ArgumentMatchers.any(), ArgumentMatchers.eq(false))).thenReturn(response2);
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setEndpoint("172.1.4.4");
        environment.setPort(1123);
        protectedEnvironment3.setUserId("userId");
        PowerMockito.when(environmentService.getEnvironmentById(ArgumentMatchers.anyString())).thenReturn(environment);
        redisService.preCheck(clusterProtectedResource);
    }

    /**
     * 用例场景：注册Redis集群时，集群节点与实际不一致
     * 前置条件：无
     * 检查点: 抛出异常，异常码：ILLEGAL_PARAM
     */
    @Test
    public void should_throw_LegoCheckedException_if_nodes_are_inconsistent_when_register_cluster() {
        expectedException.expect(
            new ExceptionMatcher(CommonErrorCode.CLUSTER_NODES_INCONSISTENT, "Cluster nodes are inconsistent."));
        ProtectedEnvironment protectedResource17302 = new ProtectedEnvironment();
        protectedResource17302.setName("192_168");
        protectedResource17302.setType(ResourceTypeEnum.DATABASE.getType());
        protectedResource17302.setSubType(ResourceSubTypeEnum.REDIS.getType());
        Authentication auth = new Authentication();
        auth.setExtendInfo(new HashMap<>());
        protectedResource17302.setAuth(auth);
        protectedResource17302.setExtendInfo(new HashMap<String, String>() {
            {
                put(RedisConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(RedisConstant.PORT, "127");
                put(RedisConstant.IP, "192.164.1.4");
                put(RedisConstant.CLIENT_PATH, "/srv/BigData/test");
            }
        });
        ProtectedResource son18142 = new ProtectedResource();
        son18142.setName("xxx1");
        son18142.setType(ResourceTypeEnum.DATABASE.getType());
        son18142.setSubType(ResourceSubTypeEnum.REDIS.getType());
        son18142.setExtendInfo(new HashMap<String, String>() {
            {
                put(RedisConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(RedisConstant.EXTEND_INFO_KEY_VERSION, "5.0.2");
                put(RedisConstant.CLIENT_PATH, "/srv/BigData/test");
                put(RedisConstant.PORT, "127");
                put(RedisConstant.IP, "192.164.1.4");
            }
        });
        ProtectedResource song = new ProtectedResource();
        song.setName("xxx2");
        song.setExtendInfo(new HashMap<String, String>() {
            {
                put(RedisConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(RedisConstant.PORT, "127");
                put(RedisConstant.IP, "192.164.1.4");
                put(RedisConstant.CLIENT_PATH, "/srv/BigData/test");
            }
        });
        ProtectedEnvironment protectedEnvironment4 = new ProtectedEnvironment();
        protectedEnvironment4.setPort(11);
        protectedEnvironment4.setEndpoint("192.167.1.1");
        son18142.setEnvironment(protectedEnvironment4);
        song.setEnvironment(protectedEnvironment4);
        song.setType(ResourceTypeEnum.DATABASE.getType());
        song.setSubType(ResourceSubTypeEnum.REDIS.getType());
        son18142.setAuth(auth);
        ProtectedResource host = new ProtectedResource();
        host.setUuid("123");
        host.setEndpoint("10.10.10.10");
        host.setPort(123);
        son18142.setDependencies(new HashMap<String, List<ProtectedResource>>() {
            {
                put(DatabaseConstants.AGENTS, Lists.newArrayList(host));
            }
        });
        song.setAuth(auth);
        PowerMockito.when(
                resourceService.query(ArgumentMatchers.anyInt(), ArgumentMatchers.anyInt(), ArgumentMatchers.anyMap()))
            .thenReturn(new PageListResponse<>(0, Arrays.asList()));
        protectedResource17302.setDependencies(ImmutableMap.of(ResourceConstants.CHILDREN, Arrays.asList(son18142)));
        PageListResponse<ProtectedResource> response3 = new PageListResponse<>();
        response3.setTotalCount(1);
        response3.setRecords(Lists.newArrayList(son18142));
        PowerMockito.when(
            agentUnifiedService.getDetailPageListNoRetry(ArgumentMatchers.anyString(), ArgumentMatchers.anyString(),
                ArgumentMatchers.anyInt(), ArgumentMatchers.any(), ArgumentMatchers.eq(false))).thenReturn(response3);
        ProtectedEnvironment protectedenvironment = new ProtectedEnvironment();
        protectedenvironment.setEndpoint("172.1.4.4");
        protectedenvironment.setPort(1123);
        protectedEnvironment4.setUserId("userId");
        PowerMockito.when(environmentService.getEnvironmentById(ArgumentMatchers.anyString()))
            .thenReturn(protectedenvironment);
        AppEnvResponse appEnvResponse = new AppEnvResponse();
        List<NodeInfo> nodes = new ArrayList<>();
        NodeInfo nodeInfo0 = new NodeInfo();
        nodeInfo0.setExtendInfo(ImmutableMap.of(RedisConstant.IP, "192.168.1.1", RedisConstant.PORT, "8086"));
        NodeInfo nodeInfo1 = new NodeInfo();
        nodeInfo1.setExtendInfo(ImmutableMap.of(RedisConstant.IP, "192.168.1.1", RedisConstant.PORT, "8088"));
        nodes.add(nodeInfo1);
        nodes.add(nodeInfo0);
        appEnvResponse.setNodes(nodes);
        PageListResponse<ProtectedResource> result1 = new PageListResponse<>();
        result1.setTotalCount(1);
        ProtectedResource resultProtectedResource1 = new ProtectedResource();
        resultProtectedResource1.setExtendInfo(
            ImmutableMap.of(RedisConstant.EXTEND_INFO_KEY_VERSION, "5.0.3", RedisConstant.ROLE, "1", RedisConstant.SLOT,
                "5461-10921"));
        result1.setRecords(Lists.newArrayList(resultProtectedResource1));
        PowerMockito.when(agentUnifiedService.getClusterInfo(Mockito.any(ProtectedResource.class),
            Mockito.any(ProtectedEnvironment.class))).thenReturn(appEnvResponse);
        redisService.preCheck(protectedResource17302);
    }

    /**
     * 用例场景：注册Redis集群时，集群参数正确
     * 前置条件：无
     * 检查点: 检查成功，无异常
     */
    @Test
    public void create_success() {
        ProtectedEnvironment protectedResource17303 = new ProtectedEnvironment();
        protectedResource17303.setName("192_168");
        protectedResource17303.setType(ResourceTypeEnum.DATABASE.getType());
        protectedResource17303.setSubType(ResourceSubTypeEnum.REDIS.getType());
        Authentication auth = new Authentication();
        auth.setExtendInfo(new HashMap<>());
        protectedResource17303.setAuth(auth);
        protectedResource17303.setExtendInfo(new HashMap<String, String>() {
            {
                put(RedisConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(RedisConstant.PORT, "127");
                put(RedisConstant.IP, "192.164.1.4");
                put(RedisConstant.CLIENT_PATH, "/srv/BigData/test");
            }
        });
        ProtectedResource son18141 = new ProtectedResource();
        son18141.setName("xxx1");
        son18141.setType(ResourceTypeEnum.DATABASE.getType());
        son18141.setSubType(ResourceSubTypeEnum.REDIS.getType());
        son18141.setExtendInfo(new HashMap<String, String>() {
            {
                put(RedisConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(RedisConstant.IP, "192.164.1.4");
                put(RedisConstant.PORT, "127");
                put(RedisConstant.EXTEND_INFO_KEY_VERSION, "5.0.2");
                put(RedisConstant.CLIENT_PATH, "/srv/BigData/test");
            }
        });
        ProtectedResource son0002 = new ProtectedResource();
        son0002.setName("xxx2");
        son0002.setExtendInfo(new HashMap<String, String>() {
            {
                put(RedisConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(RedisConstant.PORT, "127");
                put(RedisConstant.IP, "192.164.1.4");
                put(RedisConstant.CLIENT_PATH, "/srv/BigData/test");
            }
        });
        ProtectedEnvironment protectedEnvironment17341 = new ProtectedEnvironment();
        protectedEnvironment17341.setPort(11);
        protectedEnvironment17341.setEndpoint("192.167.1.1");
        son18141.setEnvironment(protectedEnvironment17341);
        son0002.setEnvironment(protectedEnvironment17341);
        son0002.setType(ResourceTypeEnum.DATABASE.getType());
        son0002.setSubType(ResourceSubTypeEnum.REDIS.getType());
        ProtectedResource agent = new ProtectedResource();
        agent.setUuid("123");
        agent.setEndpoint("10.10.10.10");
        agent.setPort(123);
        son18141.setAuth(auth);
        son18141.setDependencies(new HashMap<String, List<ProtectedResource>>() {
            {
                put(DatabaseConstants.AGENTS, Lists.newArrayList(agent));
            }
        });
        son0002.setAuth(auth);
        protectedResource17303.setDependencies(ImmutableMap.of(ResourceConstants.CHILDREN, Arrays.asList(son18141)));
        PowerMockito.when(
                resourceService.query(ArgumentMatchers.anyInt(), ArgumentMatchers.anyInt(), ArgumentMatchers.anyMap()))
            .thenReturn(new PageListResponse<>(0, Arrays.asList()));
        PageListResponse<ProtectedResource> response4 = new PageListResponse<>();
        response4.setTotalCount(1);
        response4.setRecords(Lists.newArrayList(son18141));
        PowerMockito.when(
            agentUnifiedService.getDetailPageListNoRetry(ArgumentMatchers.anyString(), ArgumentMatchers.anyString(),
                ArgumentMatchers.anyInt(), ArgumentMatchers.any(), ArgumentMatchers.eq(false))).thenReturn(response4);
        ProtectedEnvironment protectedenvironment = new ProtectedEnvironment();
        protectedenvironment.setPort(1123);
        protectedenvironment.setEndpoint("172.1.4.4");
        protectedenvironment.setAuthorizedUser("user000");
        protectedEnvironment17341.setUserId("userId");
        PowerMockito.when(environmentService.getEnvironmentById(ArgumentMatchers.anyString()))
            .thenReturn(protectedenvironment);
        AppEnvResponse appEnvResponse = new AppEnvResponse();
        List<NodeInfo> nodes = new ArrayList<>();
        NodeInfo nodeInfo0 = new NodeInfo();
        nodeInfo0.setExtendInfo(ImmutableMap.of(RedisConstant.IP, "192.164.1.4", RedisConstant.PORT, "127"));
        nodes.add(nodeInfo0);
        appEnvResponse.setNodes(nodes);
        PageListResponse<ProtectedResource> result1 = new PageListResponse<>();
        ProtectedResource resultProtectedResource1 = new ProtectedResource();
        resultProtectedResource1.setExtendInfo(
            ImmutableMap.of(RedisConstant.EXTEND_INFO_KEY_VERSION, "5.0.3", RedisConstant.ROLE, "1", RedisConstant.SLOT,
                "5461-10921"));
        result1.setRecords(Lists.newArrayList(resultProtectedResource1));
        result1.setTotalCount(1);
        PowerMockito.when(agentUnifiedService.getClusterInfo(Mockito.any(ProtectedResource.class),
            Mockito.any(ProtectedEnvironment.class))).thenReturn(appEnvResponse);
        redisService.preCheck(protectedResource17303);
        Assert.assertEquals(protectedenvironment.getAuthorizedUser(), protectedResource17303.getAuthorizedUser());
        List<ProtectedResource> children = protectedResource17303.getDependencies().get(ResourceConstants.CHILDREN);
        for (ProtectedResource child : children) {
            Assert.assertEquals(protectedenvironment.getAuthorizedUser(), child.getAuthorizedUser());
        }
    }
}
