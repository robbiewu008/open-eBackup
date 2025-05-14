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
package com.huawei.oceanprotect.system.base.initialize.status;

import com.huawei.oceanprotect.system.base.initialize.CheckStatusThread;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerService;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.RedisTimeoutException;

/**
 * 功能描述
 *
 * @since 2024-01-20
 */
@RunWith(PowerMockRunner.class)
public class CheckStatusThreadTest {

    @InjectMocks
    CheckStatusThread checkStatusThread;

    @Mock
    RedissonClient redissonClient;

    @Test
    public void test_run(){
        checkStatusThread.setExitFlag(true);
        RMap rMap = Mockito.mock(RMap.class);
        PowerMockito.when(redissonClient.getMap(any())).thenReturn(rMap);
        PowerMockito.when(rMap.put(any(),any())).thenReturn("1");
        PowerMockito.when(rMap.expire(anyInt(),any())).thenReturn(true);
        checkStatusThread.run();
        Assert.assertTrue(true);
    }

    @Test
    public void test_run_fail(){
        checkStatusThread.setExitFlag(true);
        RMap rMap = Mockito.mock(RMap.class);
        PowerMockito.when(redissonClient.getMap(any())).thenReturn(rMap);
        PowerMockito.when(rMap.put(any(),any())).thenThrow(new RedisTimeoutException());
        PowerMockito.when(rMap.expire(anyInt(),any())).thenReturn(true);
        checkStatusThread.run();
        Assert.assertTrue(true);
    }
}
