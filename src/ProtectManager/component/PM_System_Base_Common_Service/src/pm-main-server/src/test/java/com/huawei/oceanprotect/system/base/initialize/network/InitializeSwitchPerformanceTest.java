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
package com.huawei.oceanprotect.system.base.initialize.network;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.BDDMockito.given;

import openbackup.system.base.common.exception.LegoCheckedException;
import com.huawei.oceanprotect.system.base.initialize.network.ability.InitializeSwitchPerformanceAbility;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerService;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.ability.rest.PerformanceConfigRest;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.performance.PerformanceSwitchRequest;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.PerformanceSwitchEnum;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;

/**
 * 集群存储性能 测试类
 *
 */
public class InitializeSwitchPerformanceTest {
    private final InitializeSwitchPerformance initializeSwitchPerformance = new InitializeSwitchPerformanceAbility();

    private final DeviceManagerService service = Mockito.mock(DeviceManagerService.class);

    private final PerformanceConfigRest performanceConfigRest = Mockito.mock(PerformanceConfigRest.class);

    /**
     * 用例场景：测试初始化打开性能监控开关失败
     * 前置条件：NA
     * 检查点：正常执行
     */
    @Test
    public void update_performance_config_failed() {
        given(service.getApiRest(PerformanceConfigRest.class)).willReturn(performanceConfigRest);
        given(service.getDeviceId()).willReturn("xxxxxxx");
        PerformanceSwitchRequest performanceSwitchRequest = new PerformanceSwitchRequest();
        performanceSwitchRequest.setPerformanceSwitchEnum(PerformanceSwitchEnum.ENABLE);
        given(performanceConfigRest.updatePerformanceConfig("xxxxxxx", performanceSwitchRequest)).willThrow(
            new LegoCheckedException("some error"));
        initializeSwitchPerformance.updatePerformanceConfig(service, PerformanceSwitchEnum.ENABLE.name());
        Assert.assertThrows("some error", LegoCheckedException.class,
            () -> performanceConfigRest.updatePerformanceConfig("xxxxxxx", performanceSwitchRequest));
    }

    /**
     * 用例场景：测试初始化打开性能监控开关成功
     * 前置条件：NA
     * 检查点：正常执行
     */
    @Test
    public void update_performance_config_success() {
        given(service.getApiRest(PerformanceConfigRest.class)).willReturn(performanceConfigRest);
        given(service.getDeviceId()).willReturn("xxxxxxx");
        PerformanceSwitchRequest performanceSwitchRequest = new PerformanceSwitchRequest();
        performanceSwitchRequest.setPerformanceSwitchEnum(PerformanceSwitchEnum.ENABLE);
        given(performanceConfigRest.updatePerformanceConfig("xxxxxxx", performanceSwitchRequest)).willReturn("'data': {}");
        initializeSwitchPerformance.updatePerformanceConfig(service, PerformanceSwitchEnum.ENABLE.name());
        Assert.assertEquals("'data': {}",performanceConfigRest.updatePerformanceConfig("xxxxxxx", performanceSwitchRequest));
    }
}
