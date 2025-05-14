package com.huawei.oceanprotect.system.base.service.impl;

import static org.mockito.ArgumentMatchers.any;

import com.huawei.oceanprotect.system.base.initialize.network.action.DeviceManagerHandler;
import openbackup.system.base.sdk.cluster.ClusterInternalApi;
import openbackup.system.base.sdk.cluster.model.ClusterDetailInfo;
import openbackup.system.base.sdk.cluster.model.StorageSystemInfo;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.ability.session.IStorageDeviceRepository;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.StorageDevice;

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
public class DoradoServiceImplTest {

    @InjectMocks
    private DoradoServiceImpl doradoServiceImpl;

    @Mock
    private IStorageDeviceRepository repository;

    @Mock
    private ClusterInternalApi clusterInternalApi;

    @Mock
    private DeviceManagerHandler deviceManagerHandler;

    @Test
    public void test_query_dev_esn(){
        PowerMockito.when(clusterInternalApi.queryClusterDetails()).thenReturn(new ClusterDetailInfo(){{
            setStorageSystem(new StorageSystemInfo(){{
                setStorageEsn("aaa");
            }});
        }});

        Assert.assertEquals("aaa",doradoServiceImpl.queryDevEsn());
    }

    @Test
    public void test_query_device_manager() {
        PowerMockito.when(repository.findLocalStorage(true)).thenReturn(new StorageDevice());
        Assert.assertEquals(null,doradoServiceImpl.queryDeviceManager());
    }
}
