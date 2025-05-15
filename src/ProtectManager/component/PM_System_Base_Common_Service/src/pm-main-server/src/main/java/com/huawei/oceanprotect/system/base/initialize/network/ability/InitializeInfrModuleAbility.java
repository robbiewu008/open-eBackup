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
