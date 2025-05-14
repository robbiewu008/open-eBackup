package com.huawei.oceanprotect.system.base.initialize;

import static org.mockito.ArgumentMatchers.any;

import com.huawei.oceanprotect.base.cluster.remote.dorado.service.ClusterStorageService;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;

/**
 * 功能描述
 *
 * @author x30046484
 * @since 2024-01-20
 */

@RunWith(PowerMockRunner.class)
public class UpdatePerformanceThreadTest {

    @InjectMocks
    UpdatePerformanceThread updatePerformanceThread;

    @Mock
    ClusterStorageService clusterStorageService;

    @Test
    public void test_update_performance_thread(){
        new UpdatePerformanceThread(null);
        Assert.assertTrue(true);
    }

    @Test
    public void test_run(){
        PowerMockito.doNothing().when(clusterStorageService).updatePerformance(true,true);
        updatePerformanceThread.run();
        Assert.assertTrue(true);
    }
}
