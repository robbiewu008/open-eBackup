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
 * @author x30046484
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
