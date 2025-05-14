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

import com.huawei.oceanprotect.system.base.initialize.backstorage.ability.InitFileSystemsAbility;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerService;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.DeviceManagerResponse;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.DeviceManagerResponseError;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.filesystem.ApplicationWorkLoad;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;

import java.util.ArrayList;
import java.util.List;

/**
 * 测试文件系统
 *
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-29
 */
public class InitFileSystemsAbilityTest {
    private final DeviceManagerService service = Mockito.mock(DeviceManagerService.class);

    private final InitializeFileSystems initializeFileSystems = new InitFileSystemsAbility();

    /**
     * 用例场景：测试创建文件系统成功
     * 前置条件：满足条件
     * 检查点：成功
     */
    @Test
    public void check_attain_set_workload_type_id_success() {
        DeviceManagerResponse<List<ApplicationWorkLoad>> response = new DeviceManagerResponse<>();
        ApplicationWorkLoad application = new ApplicationWorkLoad();
        application.setId("100");
        application.setName("NAS");
        List<ApplicationWorkLoad> applicationWorkLoads = new ArrayList<>();
        applicationWorkLoads.add(application);
        response.setData(applicationWorkLoads);
        given(service.getBatchWorkLoadType()).willReturn(response);
        DeviceManagerResponse<ApplicationWorkLoad> applicationResponse = new DeviceManagerResponse<>();
        ApplicationWorkLoad applicationWorkLoad = new ApplicationWorkLoad();
        applicationWorkLoad.setId("100");
        applicationResponse.setData(applicationWorkLoad);
        response.setResult(new DeviceManagerResponseError(0, "", ""));
        response.setError(new DeviceManagerResponseError(0, "", ""));
        given(service.addWorkLoadType(any())).willReturn(applicationResponse);
        Assert.assertEquals(100L, initializeFileSystems.attainSetWorkloadTypeId(service));
    }

    /**
     * 用例场景：测试查询正确文件系统id并返回
     * 前置条件：满足条件
     * 检查点：成功
     */
    @Test
    public void check_find_workload_type_id_success() {
        DeviceManagerResponse<List<ApplicationWorkLoad>> response = new DeviceManagerResponse<>();
        ApplicationWorkLoad application = new ApplicationWorkLoad();
        application.setId("100");
        application.setName("NAS_DPA_LARGE_FS");
        List<ApplicationWorkLoad> applicationWorkLoads = new ArrayList<>();
        applicationWorkLoads.add(application);
        response.setData(applicationWorkLoads);
        given(service.getBatchWorkLoadType()).willReturn(response);
        Assert.assertEquals(100L, initializeFileSystems.attainSetWorkloadTypeId(service));
    }
}
