/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.network.ability;

import com.huawei.oceanprotect.system.base.initialize.network.InitializeInfrModule;
import com.huawei.oceanprotect.system.base.initialize.network.common.InitConfigConstant;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerService;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.ability.rest.IntfModuleRest;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.inftmodule.InftModuleObject;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.HealthStatus;

import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Service;

import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

/**
 * 获取接口模块信息列表
 *
 * @author swx1010572
 * @version: [OceanProtect DataBackup 1.3.0]
 * @since 2022-12-27
 */
@Service
public class InitializeInfrModuleAbility implements InitializeInfrModule {
    /**
     * 批量查询接口模块信息
     *
     * @param service dm 对象
     * @param filter 过滤条件
     * @return 接口模块信息列表
     */
    @Override
    public List<InftModuleObject> getInfrModule(DeviceManagerService service, Map<String, String> filter) {
        return service.getApiRest(IntfModuleRest.class)
            .getIntfrModule(service.getDeviceId())
            .stream()
            .filter(inftModuleObject -> isExistModeType(filter, inftModuleObject))
            .filter(inftModuleObject -> HealthStatus.NORMAL.equals(inftModuleObject.getHealthStatus()))
            .collect(Collectors.toList());
    }

    private boolean isExistModeType(Map<String, String> filter, InftModuleObject inftModuleObject) {
        if (StringUtils.isEmpty(filter.get(InitConfigConstant.SERVICE_MODE))) {
            return true;
        }
        return filter.get(InitConfigConstant.SERVICE_MODE)
            .equals(inftModuleObject.getServiceMode().toString());
    }
}
