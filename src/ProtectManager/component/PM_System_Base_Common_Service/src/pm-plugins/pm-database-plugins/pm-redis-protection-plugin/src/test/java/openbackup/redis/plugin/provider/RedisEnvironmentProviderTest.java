/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.redis.plugin.provider;

import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.redis.plugin.common.ExceptionMatcher;
import openbackup.redis.plugin.constant.RedisConstant;
import openbackup.redis.plugin.service.RedisService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.sdk.cluster.enums.ClusterEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;

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

import java.util.Collections;
import java.util.HashMap;
import java.util.Optional;

/**
 * redis集群环境检查测试类
 *
 * @author x30028756
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-14
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest( {RedisEnvironmentProvider.class})
public class RedisEnvironmentProviderTest {
    private RedisEnvironmentProvider redisEnvironmentProvider;

    @Mock
    private ResourceService resourceService;

    @Mock
    private RedisService redisService;

    @Mock
    private RedisResourceScanProvider redisResourceScanProvider;

    @Rule
    public ExpectedException expectedException = ExpectedException.none();

    @Before
    public void setUp() {
        resourceService = PowerMockito.mock(ResourceService.class);
        redisService = PowerMockito.mock(RedisService.class);
        redisResourceScanProvider = PowerMockito.mock(RedisResourceScanProvider.class);
        PowerMockito.doNothing().when(redisService).preCheck(Mockito.any(ProtectedEnvironment.class));
        redisEnvironmentProvider = new RedisEnvironmentProvider(PowerMockito.mock(ProviderManager.class),
            PowerMockito.mock(PluginConfigManager.class), resourceService, redisService, redisResourceScanProvider);
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
        Assert.assertTrue(redisEnvironmentProvider.applicable(protectedEnvironment.getSubType()));
    }

    /**
     * 用例场景：集群参数填写正确
     * 前置条件：无
     * 检查点: 校验成功
     */
    @Test
    public void check_success() {
        ProtectedEnvironment resource = new ProtectedEnvironment();
        resource.setName("T192_168");
        resource.setType(ResourceTypeEnum.DATABASE.getType());
        resource.setSubType(ResourceSubTypeEnum.REDIS.getType());
        resource.setExtendInfo(new HashMap<String, String>() {
            {
                put(RedisConstant.TYPE, DatabaseConstants.CLUSTER_TARGET);
            }
        });
        resource.setDependencies(
            Collections.singletonMap(ResourceConstants.CHILDREN, Lists.newArrayList(new ProtectedResource())));
        PowerMockito.when(resourceService.check(Mockito.any(ProtectedResource.class)))
            .thenReturn(new ActionResult[] {new ActionResult(0, "")});
        redisEnvironmentProvider.register(resource);
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
        resource.setType(ResourceTypeEnum.DATABASE.getType());
        resource.setSubType(ResourceSubTypeEnum.REDIS.getType());
        resource.setExtendInfo(new HashMap<String, String>() {
            {
                put(RedisConstant.TYPE, DatabaseConstants.CLUSTER_TARGET);
            }
        });
        ProtectedResource son1 = new ProtectedResource();
        son1.setName("xxx1");
        son1.setType(ResourceTypeEnum.DATABASE.getType());
        son1.setSubType(ResourceSubTypeEnum.REDIS.getType());
        son1.setUuid("123");
        son1.setExtendInfo(new HashMap<String, String>() {
            {
                put(RedisConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(RedisConstant.PORT, "127");
                put(RedisConstant.IP, "192.164.1.4");
                put(RedisConstant.CLIENT_PATH, "/srv/BigData/test");
                put(DatabaseConstants.STATUS, "27");
            }
        });
        ProtectedResource son2 = new ProtectedResource();
        son2.setName("xxx1");
        Authentication auth = new Authentication();
        auth.setAuthType(Authentication.NO_AUTH);
        son1.setAuth(auth);
        son2.setUuid("456");
        son2.setAuth(auth);
        son1.setDependencies(Collections.singletonMap(DatabaseConstants.AGENTS, Lists.newArrayList(son2)));
        resource.setDependencies(Collections.singletonMap(ResourceConstants.CHILDREN, Lists.newArrayList(son1)));
        PowerMockito.when(resourceService.check(Mockito.any(ProtectedResource.class)))
            .thenReturn(new ActionResult[] {new ActionResult(0, "")});
        Optional<String> statusOptional = redisEnvironmentProvider.healthCheckWithResultStatus(resource);
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
        ProtectedEnvironment clusterResource = new ProtectedEnvironment();
        clusterResource.setUuid("12221212");
        clusterResource.setName("192_168");
        clusterResource.setType(ResourceTypeEnum.DATABASE.getType());
        clusterResource.setSubType(ResourceSubTypeEnum.REDIS.getType());
        clusterResource.setExtendInfo(new HashMap<String, String>() {
            {
                put(RedisConstant.TYPE, DatabaseConstants.CLUSTER_TARGET);
            }
        });
        ProtectedResource son18161 = new ProtectedResource();
        son18161.setName("xxx1");
        son18161.setType(ResourceTypeEnum.DATABASE.getType());
        son18161.setSubType(ResourceSubTypeEnum.REDIS.getType());
        son18161.setUuid("123");
        son18161.setExtendInfo(new HashMap<String, String>() {
            {
                put(RedisConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(RedisConstant.PORT, "127");
                put(RedisConstant.IP, "192.164.1.4");
                put(RedisConstant.CLIENT_PATH, "/srv/BigData/test");
                put(DatabaseConstants.STATUS, "27");
            }
        });
        ProtectedResource son2 = new ProtectedResource();
        son2.setUuid("456");
        son2.setName("xxx1");
        Authentication auth = new Authentication();
        auth.setAuthType(Authentication.NO_AUTH);
        son2.setAuth(auth);
        son18161.setAuth(auth);
        son18161.setDependencies(Collections.singletonMap(DatabaseConstants.AGENTS, Lists.newArrayList(son2)));
        clusterResource.setDependencies(
            Collections.singletonMap(ResourceConstants.CHILDREN, Lists.newArrayList(son18161)));
        PowerMockito.when(resourceService.check(Mockito.any(ProtectedResource.class)))
            .thenReturn(new ActionResult[] {new ActionResult(1221212, "")});
        redisEnvironmentProvider.healthCheckWithResultStatus(clusterResource);
    }

    /**
     * 用例场景：扫描成功
     * 前置条件：无
     * 检查点: 无
     */
    @Test
    public void scanSuccess() {
        redisEnvironmentProvider.scan(new ProtectedEnvironment());
    }
}