/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.controller;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.mockStatic;

import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.process.ProcessException;
import openbackup.system.base.common.process.ProcessResult;
import openbackup.system.base.common.process.ProcessUtil;
import com.huawei.oceanprotect.system.base.initialize.backstorage.InitializeFileSystems;
import com.huawei.oceanprotect.system.base.initialize.network.action.DeviceManagerHandler;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.ability.session.IStorageDeviceRepository;
import openbackup.system.base.common.exception.DeviceManagerException;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.StorageDevice;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.Arrays;

/**
 * 测试类
 *
 * @author w00504341
 * @since 2023-08-24
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {
    InitServiceController.class,
    ProcessUtil.class
})
public class InitServiceControllerTest {
    @InjectMocks
    private InitServiceController initServiceController;

    @Mock
    private IStorageDeviceRepository repository;

    @Mock
    private DeviceManagerHandler handler;

    @Mock
    private InitializeFileSystems fileSystems;

    /**
     * 用例场景：创建文件系统类型
     * 前置条件：NA
     * 检查点：检查创建文件系统类型是否成功
     */
    @Test
    public void test_create_file_system_type() {
        StorageDevice storageDevice = new StorageDevice();
        storageDevice.setUserName("a");
        storageDevice.setPassword("12");
        PowerMockito.when(repository.findLocalStorage(true)).thenReturn(storageDevice);
        PowerMockito.when(fileSystems.attainSetWorkloadTypeId(any())).thenReturn(1L);
        Assert.assertNotNull(initServiceController.creatFileSystemType());

        PowerMockito.when(repository.findLocalStorage(true)).thenThrow(DeviceManagerException.class);
        Assert.assertThrows(LegoCheckedException.class, () -> initServiceController.creatFileSystemType());
    }

    @Test
    public void test_find_float_ip_success() throws ProcessException {
        PowerMockito.mockStatic(ProcessUtil.class);
        PowerMockito.when(ProcessUtil.executeInMinutes(
            Arrays.asList("/bin/sh", "-c", "ifconfig | grep 'u-mao' -A 2 | grep 'inet' | awk -F ' ' '{print $2}'"),
            1L)).thenReturn(new ProcessResult(){{
                setExitCode(0);
        }});
        initServiceController.findFloatIp();
    }

    @Test
    public void test_find_float_ip_fail() throws ProcessException {

        PowerMockito.when(ProcessUtil.executeInMinutes(
            Arrays.asList("/bin/sh", "-c", "ifconfig | grep 'u-mao' -A 2 | grep 'inet' | awk -F ' ' '{print $2}'"),
            1L)).thenThrow(new ProcessException("",""));
        initServiceController.findFloatIp();

    }
}
