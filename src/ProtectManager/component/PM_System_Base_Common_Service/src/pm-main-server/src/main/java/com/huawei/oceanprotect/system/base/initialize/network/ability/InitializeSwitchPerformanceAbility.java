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
package com.huawei.oceanprotect.system.base.initialize.network.ability;

import com.huawei.oceanprotect.system.base.initialize.network.InitializeSwitchPerformance;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerService;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.ability.rest.PerformanceConfigRest;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.performance.PerformanceSwitchRequest;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.PerformanceSwitchEnum;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.exception.DeviceManagerException;
import openbackup.system.base.common.exception.LegoCheckedException;

import org.springframework.stereotype.Component;

/**
 * 集群存储性能
 *
 */
@Slf4j
@Component
public class InitializeSwitchPerformanceAbility implements InitializeSwitchPerformance {
    @Override
    public void updatePerformanceConfig(DeviceManagerService service, String performanceSwitch) {
        PerformanceSwitchRequest performanceSwitchRequest = new PerformanceSwitchRequest();
        performanceSwitchRequest.setPerformanceSwitchEnum(PerformanceSwitchEnum.valueOf(performanceSwitch));
        try {
            PerformanceConfigRest performanceConfigRest = service.getApiRest(PerformanceConfigRest.class);
            performanceConfigRest.updatePerformanceConfig(service.getDeviceId(), performanceSwitchRequest);
        } catch (DeviceManagerException | LegoCheckedException e) {
            log.error("updatePerformance fail message: {}", e.getMessage());
        }
    }
}
