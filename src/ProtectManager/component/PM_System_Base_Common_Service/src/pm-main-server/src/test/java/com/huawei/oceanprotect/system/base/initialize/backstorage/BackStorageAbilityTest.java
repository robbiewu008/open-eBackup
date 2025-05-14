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
package com.huawei.oceanprotect.system.base.initialize.backstorage;

import static org.mockito.BDDMockito.given;

import openbackup.system.base.common.exception.LegoCheckedException;
import com.huawei.oceanprotect.system.base.initialize.backstorage.ability.InitBackStorageAbility;
import com.huawei.oceanprotect.system.base.initialize.status.InitStatusService;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerService;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.DeviceManagerResponse;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.storagepool.StoragePool;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.system.SystemInfo;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.ArrayList;
import java.util.List;

/**
 * 测试创建存储能力
 *
 * @author l00347293
 * @since 2021-07-03
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = {InitializeBackStorage.class, InitBackStorageAbility.class})
public class BackStorageAbilityTest {
    @MockBean
    private DeviceManagerService service;

    @MockBean
    private InitStatusService initStatusService;

    @Autowired
    private InitializeBackStorage initializeBackStorage;

    /**
     * 用例场景：测试初始化校验检查10T
     * 前置条件：不满足10T
     * 检查点：失败
     */
    @Test
    public void test_check_storage_capacity() {
        List<StoragePool> storagePools = new ArrayList<>();
        StoragePool storagePool = new StoragePool();
        storagePool.setUserTotalCapacity("9");
        storagePools.add(storagePool);
        DeviceManagerResponse<List<StoragePool>> response = new DeviceManagerResponse<>();
        response.setData(storagePools);
        given(service.getStoragePools()).willReturn(response);

        DeviceManagerResponse<SystemInfo> systemResponse = new DeviceManagerResponse<>();
        SystemInfo systemInfo = new SystemInfo();
        systemInfo.setSectorSize("1099511627776");  // 单位是TB
        systemResponse.setData(systemInfo);
        given(service.getSystem()).willReturn(systemResponse);

        Assert.assertThrows(LegoCheckedException.class, () -> initializeBackStorage.check(service));
    }

    /**
     * 用例场景：试初始化校验查询不到存储库
     * 前置条件：不存在存储库
     * 检查点：失败
     */
    @Test
    public void should_throw_LegoCheckedException_if_not_exist_storage_pools_when_check() {
        DeviceManagerResponse<List<StoragePool>> response = new DeviceManagerResponse<>();
        List<StoragePool> storagePools = new ArrayList<>();
        response.setData(storagePools);
        given(service.getStoragePools()).willReturn(response);
        Assert.assertThrows(LegoCheckedException.class, () -> initializeBackStorage.check(service));
    }

    /**
     * 用例场景：测试初始化校验检查10T
     * 前置条件：满足条件
     * 检查点：成功,不报错
     */
    @Test
    public void check_storage_capacity_success() {
        List<StoragePool> storagePools = new ArrayList<>();
        StoragePool storagePool = new StoragePool();
        storagePool.setUserTotalCapacity("20");
        storagePools.add(storagePool);
        DeviceManagerResponse<List<StoragePool>> response = new DeviceManagerResponse<>();
        response.setData(storagePools);
        given(service.getStoragePools()).willReturn(response);

        DeviceManagerResponse<SystemInfo> systemResponse = new DeviceManagerResponse<>();
        SystemInfo systemInfo = new SystemInfo();
        systemInfo.setSectorSize("1099511627776");  // 单位是TB
        systemResponse.setData(systemInfo);
        given(service.getSystem()).willReturn(systemResponse);
        initializeBackStorage.check(service);
    }
}