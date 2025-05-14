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

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.BDDMockito.given;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.model.repository.StoragePool;
import com.huawei.oceanprotect.system.base.initialize.backstorage.ability.InitStoragePoolAbility;
import com.huawei.oceanprotect.system.base.initialize.backstorage.beans.InitBackActionResult;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerService;
import openbackup.system.base.common.exception.DeviceManagerException;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.DeviceManagerResponse;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.DeviceManagerResponseError;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.storagepool.Task;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.junit4.SpringRunner;

/**
 * 测试设置存储池能力
 *
 * @author w00493811
 * @since 2021-01-04
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = {InitializeStoragePool.class, InitStoragePoolAbility.class})
@AutoConfigureMockMvc
public class InitializeStoragePoolTest {
    @MockBean
    private DeviceManagerService service;

    @Autowired
    private InitializeStoragePool initializeStoragePool;

    /**
     * 测试初始化存储动作
     */
    @Test
    public void test_doAction_success() {
        DeviceManagerResponse<Object> response = new DeviceManagerResponse<Object>();
        response.setResult(new DeviceManagerResponseError(0, "", ""));
        response.setError(new DeviceManagerResponseError(0, "", ""));
        given(service.setDiskPoolInfo(any())).willReturn(response);

        DeviceManagerResponse<Task> taskResponse = new DeviceManagerResponse<>();
        taskResponse.setError(new DeviceManagerResponseError(1, "", ""));
        taskResponse.setResult(new DeviceManagerResponseError(0, "", ""));
        given(service.setStoragePoolExpandInfo(any(), any())).willReturn(taskResponse);

        StoragePool storagePool = new StoragePool();
        storagePool.setParentId("111");
        InitBackActionResult result = initializeStoragePool.doAction("", "",
            storagePool, new InitBackActionResult());

        Assert.assertTrue(result.isOkay());
    }

    /**
     * 用例场景：校验不存在存储池或者存储信息的
     * 前置条件：设置存储池或者存储信息时报错
     * 检查点：无
     */
    @Test
    public void should_throw_LegoCheckedException_if_set_disk_pool_info_or_storage_pool_expand_info_when_do_action() {
        given(service.setDiskPoolInfo(any())).willThrow(
            new DeviceManagerException(CommonErrorCode.OPERATION_FAILED, "no disk pool info", null));

        given(service.setStoragePoolExpandInfo(any(), any())).willThrow(
            new DeviceManagerException(CommonErrorCode.OPERATION_FAILED, "no storage pool expand info", null));

        StoragePool storagePool = new StoragePool();
        storagePool.setParentId("111");
        InitBackActionResult result = initializeStoragePool.doAction("", "",
            storagePool, new InitBackActionResult());

        Assert.assertTrue(result.isOkay());
    }
}