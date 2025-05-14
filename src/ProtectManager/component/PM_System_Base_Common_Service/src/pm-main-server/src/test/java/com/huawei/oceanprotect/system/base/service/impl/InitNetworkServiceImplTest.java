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
package com.huawei.oceanprotect.system.base.service.impl;

import java.util.ArrayList;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.BDDMockito.given;
import static org.mockito.Mockito.doThrow;

import openbackup.data.access.framework.core.dao.InitNetworkConfigMapper;
import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.enums.DeployTypeEnum;
import openbackup.system.base.common.exception.LegoCheckedException;
import com.huawei.oceanprotect.system.base.initialize.network.action.DeviceManagerHandler;
import com.huawei.oceanprotect.system.base.initialize.network.common.ArchiveNetworkConfig;
import com.huawei.oceanprotect.system.base.initialize.network.common.ConfigStatus;
import com.huawei.oceanprotect.system.base.initialize.network.common.InitConfigConstant;
import com.huawei.oceanprotect.system.base.initialize.network.common.InitNetworkBody;
import com.huawei.oceanprotect.system.base.initialize.network.enums.InitServiceType;
import com.huawei.oceanprotect.system.base.initialize.status.InitStatusService;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerService;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.ability.rest.UserRest;
import openbackup.system.base.service.DeployTypeService;
import com.huawei.oceanprotect.system.base.service.InitNetworkService;
import com.huawei.oceanprotect.system.base.service.InitService;
import openbackup.system.base.util.ProviderRegistry;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.api.support.membermodification.MemberModifier;
import org.redisson.api.RLock;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.RedisException;

/**
 * 测试初始化线程类
 *
 */
public class InitNetworkServiceImplTest {
    private final RedissonClient redissonClient = Mockito.mock(RedissonClient.class);

    private final RLock lock = PowerMockito.mock(RLock.class);

    private final InitStatusService initStatusService = PowerMockito.mock(InitStatusService.class);

    private final InitNetworkConfigMapper initNetworkConfigMapper = PowerMockito.mock(InitNetworkConfigMapper.class);

    private final InitService<InitNetworkBody> initNetworkService = new InitNetworkServiceImpl();

    private final DeviceManagerHandler deviceManagerHandler = PowerMockito.mock(DeviceManagerHandler.class);

    private final DeployTypeService deployTypeService = PowerMockito.mock(DeployTypeService.class);

    private final ProviderRegistry registry = PowerMockito.mock(ProviderRegistry.class);

    private final JobService jobService = PowerMockito.mock(JobService.class);

    /**
     * 用例场景：测试初始化是锁失败
     * 前置条件：满足条件
     * 检查点：失败
     */
    @Test
    public void should_throw_LegoCheckedException_if_try_lock_when_init()
        throws IllegalAccessException, InterruptedException {
        mockParam();
        given(lock.tryLock(InitConfigConstant.INIT_SYSTEM_LOCK_WAIT_TIMES,
            InitConfigConstant.INIT_SYSTEM_LOCK_RELEASE_TIMES,
            InitConfigConstant.INIT_SYSTEM_LOCK_TIME_UNIT)).willReturn(false);
        Assert.assertThrows(LegoCheckedException.class, () -> initNetworkService.init(new InitNetworkBody()));
    }

    /**
     * 用例场景：测试初始化时已经初始化失败
     * 前置条件：满足条件
     * 检查点：失败
     */
    @Test
    public void should_throw_LegoCheckedException_if_get_init_config_status_when_init()
        throws IllegalAccessException, InterruptedException {
        mockParam();
        ConfigStatus configStatus = new ConfigStatus();
        configStatus.setStatus(InitConfigConstant.ERROR_CODE_RUNNING);
        given(initStatusService.queryInitStatus()).willReturn(configStatus);
        Assert.assertThrows(LegoCheckedException.class, () -> initNetworkService.init(new InitNetworkBody()));
    }

    /**
     * 用例场景：测试初始化时已经初始化成功
     * 前置条件：满足条件
     * 检查点：成功
     */
    @Test
    public void init_already_success() throws IllegalAccessException, InterruptedException {
        mockParam();
        ConfigStatus configStatus = new ConfigStatus();
        configStatus.setStatus(Constants.ERROR_CODE_OK);
        given(initStatusService.queryInitStatus()).willReturn(configStatus);
        initNetworkService.init(new InitNetworkBody());
        Mockito.verify(jobService).countJob(any());
    }

    /**
     * 用例场景：测试初始化成功
     * 前置条件：满足条件
     * 检查点：成功
     */
    @Test
    public void init_success() throws IllegalAccessException, InterruptedException {
        mockParam();
        InitNetworkBody initNetworkBody = new InitNetworkBody();
        initNetworkBody.setArchiveNetworkConfig(new ArchiveNetworkConfig());
        Assert.assertEquals(InitConfigConstant.INIT_SUCCESS, initNetworkService.init(initNetworkBody));
    }

    /**
     * 用例场景：测试初始化类是否正确
     * 前置条件：满足条件
     * 检查点：成功
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(initNetworkService.applicable(InitServiceType.INIT_NETWORK.getType()));
    }

    private void mockParam() throws IllegalAccessException, InterruptedException {
        MemberModifier.field(InitNetworkServiceImpl.class, "redissonClient").set(initNetworkService, redissonClient);
        MemberModifier.field(InitNetworkServiceImpl.class, "initStatusService")
            .set(initNetworkService, initStatusService);
        MemberModifier.field(InitNetworkServiceImpl.class, "initNetworkConfigMapper")
            .set(initNetworkService, initNetworkConfigMapper);
        MemberModifier.field(InitNetworkServiceImpl.class, "deviceManagerHandler")
                .set(initNetworkService, deviceManagerHandler);
        MemberModifier.field(InitNetworkServiceImpl.class, "deployTypeService")
                .set(initNetworkService, deployTypeService);
        MemberModifier.field(InitNetworkServiceImpl.class, "registry")
                .set(initNetworkService, registry);
        MemberModifier.field(InitNetworkServiceImpl.class, "jobService")
                .set(initNetworkService, jobService);
        DeviceManagerService deviceManager = Mockito.mock(DeviceManagerService.class);
        UserRest userRest = Mockito.mock(UserRest.class);
        given(userRest.getUser(anyString())).willReturn(new ArrayList<>());
        given(jobService.countJob(any())).willReturn(0L);
        given(deviceManager.getApiRest(UserRest.class)).willReturn(userRest);
        given(deviceManagerHandler.achiveDeviceManagerService(any())).willReturn(deviceManager);
        given(deployTypeService.getDeployType()).willReturn(DeployTypeEnum.X8000);
        given(redissonClient.getLock(InitConfigConstant.INIT_SYSTEM_LOCK_NAME)).willReturn(lock);
        given(registry.findProviderOrDefault(any(),anyString(),any())).willReturn(Mockito.mock(InitNetworkService.class));
        given(redissonClient.getMap(anyString())).willReturn(Mockito.mock(RMap.class));
        ConfigStatus configStatus = new ConfigStatus();
        configStatus.setStatus(InitConfigConstant.ERROR_CODE_YES);
        given(initStatusService.queryInitStatus()).willReturn(configStatus);
        given(lock.tryLock(InitConfigConstant.INIT_SYSTEM_LOCK_WAIT_TIMES,
            InitConfigConstant.INIT_SYSTEM_LOCK_RELEASE_TIMES,
            InitConfigConstant.INIT_SYSTEM_LOCK_TIME_UNIT)).willReturn(true);
        doThrow(new RedisException()).when(lock).unlock();
    }

}
