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
package openbackup.data.access.framework.restore.service;

import openbackup.data.access.framework.core.common.enums.v2.RestoreTypeEnum;
import com.huawei.oceanprotect.functionswitch.template.service.FunctionSwitchService;
import openbackup.system.base.common.license.LicenseValidateService;
import openbackup.system.base.sdk.license.enums.FunctionEnum;

import org.springframework.stereotype.Service;

/**
 * 恢复任务校验服务
 *
 **/
@Service
public class RestoreValidateService {
    private final LicenseValidateService licenseValidateService;
    private final FunctionSwitchService functionSwitchService;

    /**
     * 恢复任务校验服务构造函数
     *
     * @param licenseValidateService license校验服务
     * @param functionSwitchService 功能开关服务
     */
    public RestoreValidateService(LicenseValidateService licenseValidateService,
        FunctionSwitchService functionSwitchService) {
        this.licenseValidateService = licenseValidateService;
        this.functionSwitchService = functionSwitchService;
    }

    /**
     * 校验恢复任务license权限
     *
     * @param resourceSubType 资源子类型
     * @param type 恢复任务类型枚举
     */
    public void checkLicense(String resourceSubType, RestoreTypeEnum type) {
        switch (type) {
            case CR:
                licenseValidateService.validate(resourceSubType, FunctionEnum.RECOVERY);
                break;
            case IR:
                licenseValidateService.validate(resourceSubType, FunctionEnum.INSTANT_RECOVERY);
                break;
            case FLR:
                licenseValidateService.validate(resourceSubType, FunctionEnum.FINE_GRAINED_RECOVERY);
                break;
            default:
                throw new IllegalStateException("Unsupported restore type: " + type);
        }
    }
}
