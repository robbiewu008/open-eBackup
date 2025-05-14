/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
 * @author swx1010572
 * @since 2022-08-18
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
