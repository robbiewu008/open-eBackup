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
package openbackup.access.framework.resource.service.impl;

import java.util.Arrays;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatchers;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.modules.junit4.PowerMockRunnerDelegate;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.data.redis.core.RedisTemplate;
import org.springframework.data.redis.core.SetOperations;
import org.springframework.data.redis.core.ValueOperations;
import org.springframework.test.context.ContextConfiguration;
import org.springframework.test.context.junit4.SpringRunner;

import openbackup.access.framework.resource.service.ResourceRedisLockService;
import com.huawei.oceanprotect.base.cluster.sdk.dto.MemberClusterBo;
import com.huawei.oceanprotect.base.cluster.sdk.service.MemberClusterService;

import openbackup.access.framework.resource.service.impl.ResourceRedisLockServiceImpl;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.sdk.auth.api.AuthNativeApi;
import openbackup.system.base.sdk.cluster.BackupClusterJobClient;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.resource.model.ResourceLockEntity;

import feign.FeignException;

/**
 * ResourceRedisLockServiceImplTest
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(ExceptionUtil.class)
@PowerMockRunnerDelegate(SpringRunner.class)
@ContextConfiguration(classes = {ResourceRedisLockServiceImpl.class})
public class ResourceRedisLockServiceImplTest {
    @Autowired
    private ResourceRedisLockService resourceRedisLockService;

    @MockBean
    private RedisTemplate<String, String> redisTemplate;

    @MockBean
    private MemberClusterService memberClusterService;

    @MockBean
    private BackupClusterJobClient backupClusterJobClient;

    @MockBean
    private AuthNativeApi authNativeApi;

    @MockBean
    private JobService jobService;

    @MockBean
    private CopyRestApi copyRestApi;

    @MockBean
    private ResourceService resourceService;

    ValueOperations<String,String> valueOperations = Mockito.mock(ValueOperations.class);

    SetOperations<String, String> setOperations = Mockito.mock(SetOperations.class);

    @Before
    public void before(){
        MemberClusterBo memberClusterBo1=new MemberClusterBo();
        memberClusterBo1.setRemoteEsn("esn1");
        memberClusterBo1.setClusterIp("127.0.0.1");
        memberClusterBo1.setClusterPort(10);
        MemberClusterBo memberClusterBo2=new MemberClusterBo();
        memberClusterBo2.setRemoteEsn("esn2");
        memberClusterBo2.setClusterIp("127.0.0.1");
        memberClusterBo2.setClusterPort(10);
        MemberClusterBo memberClusterBo3=new MemberClusterBo();
        memberClusterBo3.setRemoteEsn("esn3");
        memberClusterBo3.setClusterIp("127.0.0.1");
        memberClusterBo3.setClusterPort(10);
        Mockito.when(memberClusterService.getAllMemberClusters()).thenReturn(Arrays.asList(memberClusterBo1,memberClusterBo2,memberClusterBo3));
        Mockito.when(redisTemplate.opsForValue()).thenReturn(valueOperations);
        Mockito.when(redisTemplate.opsForSet()).thenReturn(setOperations);
        Mockito.when(authNativeApi.generateSysadminToken()).thenReturn("token");
        Mockito.when(backupClusterJobClient.acquireLock(ArgumentMatchers.any(),ArgumentMatchers.anyString(),ArgumentMatchers.any())).thenReturn(true);
        Mockito.when(backupClusterJobClient.unlock(ArgumentMatchers.any(),ArgumentMatchers.anyString(),ArgumentMatchers.any())).thenReturn(true);
        Mockito.doNothing().when(valueOperations).set(ArgumentMatchers.anyString(),ArgumentMatchers.anyString());
        Mockito.when(memberClusterService.getCurrentClusterEsn()).thenReturn("esn1");
        Mockito.when(memberClusterService.clusterEstablished()).thenReturn(true);
        PowerMockito.mockStatic(ExceptionUtil.class);
        PowerMockito.when(ExceptionUtil.getErrorMessage(ArgumentMatchers.any())).thenReturn(new Exception("1"));
    }

    /**
     * 用例名称：redis加锁成功
     * 前置条件：redis打桩
     * 检查点：返回加锁成功
     */
    @Test
    public void test_acquireLock_success(){
        ResourceLockEntity resourceLock = new ResourceLockEntity();
        resourceLock.setResourceId("FS_DELETE_111");
        resourceLock.setLockType("r");
        Mockito.when(jobService.isJobPresent(Mockito.any())).thenReturn(false);
        Mockito.when(valueOperations.setIfAbsent(ArgumentMatchers.anyString(),ArgumentMatchers.anyString(),ArgumentMatchers.any())).thenReturn(true);
        boolean result1 = resourceRedisLockService.acquireLock("redis_resource_lock_test", Arrays.asList());
        Assert.assertTrue(result1);
    }

    /**
     * 用例名称：redis加锁失败
     * 前置条件：redis打桩
     * 检查点：返回加锁失败
     */
    @Test
    public void test_acquireLock_fail(){
        ResourceLockEntity resourceLock = new ResourceLockEntity();
        resourceLock.setResourceId("111");
        resourceLock.setLockType("r");
        Mockito.when(valueOperations.setIfAbsent(ArgumentMatchers.anyString(),ArgumentMatchers.anyString(),ArgumentMatchers.any())).thenReturn(false);
        boolean result = resourceRedisLockService.acquireLock("test", Arrays.asList(resourceLock));
        Assert.assertFalse(result);
    }

    /**
     * 用例名称：redis解锁
     * 前置条件：redis打桩
     * 检查点：解锁
     */
    @Test
    public void test_unlock() {
        Mockito.when(valueOperations.get(ArgumentMatchers.anyString())).thenReturn("[\"1\"]");
        boolean unlock1 = resourceRedisLockService.unlock("test");
        Mockito.when(valueOperations.get(ArgumentMatchers.anyString())).thenReturn("[]");
        boolean unlock2 = resourceRedisLockService.unlock("test");
        Assert.assertTrue(unlock2 && unlock1);
    }

    /**
     * 用例名称：redis加锁并同步给其他节点
     * 前置条件：redis打桩
     * 检查点：加锁成功
     */
    @Test
    public void test_lockAndMultiClusterSync(){
        ResourceLockEntity resourceLock = new ResourceLockEntity();
        resourceLock.setResourceId("111");
        resourceLock.setLockType("r");
        Mockito.when(valueOperations.setIfAbsent(ArgumentMatchers.anyString(),ArgumentMatchers.anyString(),ArgumentMatchers.any())).thenReturn(false);
        boolean test = resourceRedisLockService.lockAndMultiClusterSync("test", Arrays.asList(resourceLock));
        Assert.assertTrue(test);
        FeignException mock = Mockito.mock(FeignException.class);
        Mockito.doThrow(mock).when(backupClusterJobClient).acquireLock(ArgumentMatchers.any(),ArgumentMatchers.anyString(),ArgumentMatchers.any());
        boolean test2 = resourceRedisLockService.lockAndMultiClusterSync("test", Arrays.asList(resourceLock));
    }

    /**
     * 用例名称：redis解锁并同步给其他节点
     * 前置条件：redis打桩
     * 检查点：解锁成功
     */
    @Test
    public void test_unlockAndMultiClusterSync(){
        Mockito.when(valueOperations.setIfAbsent(ArgumentMatchers.anyString(),ArgumentMatchers.anyString(),ArgumentMatchers.any())).thenReturn(false);
        Mockito.when(valueOperations.get(ArgumentMatchers.anyString())).thenReturn("[]");
        boolean test = resourceRedisLockService.unlockAndMultiClusterSync("test");
        Assert.assertTrue(test);
        FeignException mock = Mockito.mock(FeignException.class);
        Mockito.doThrow(mock).when(backupClusterJobClient).unlock(ArgumentMatchers.any(),ArgumentMatchers.anyString(),ArgumentMatchers.any());
        boolean test2 = resourceRedisLockService.unlockAndMultiClusterSync("test");
    }
}
