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
package openbackup.access.framework.resource.schedule;

import openbackup.access.framework.resource.service.ProtectObjectConsistentService;
import openbackup.system.base.common.enums.DeployTypeEnum;
import openbackup.system.base.service.DeployTypeService;
import com.huawei.oceanprotect.system.sdk.dto.SystemSwitchDto;
import com.huawei.oceanprotect.system.sdk.enums.SwitchNameEnum;
import com.huawei.oceanprotect.system.sdk.enums.SwitchStatusEnum;
import com.huawei.oceanprotect.system.sdk.service.SystemSwitchInternalService;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.InitializingBean;
import org.springframework.context.annotation.PropertySource;
import org.springframework.scheduling.annotation.Scheduled;
import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.List;

/**
 * 功能描述: ProtectObjectConsistentStatusScheduler
 *
 */
@Slf4j
@Component
@PropertySource("classpath:application.yaml")
public class ProtectObjectConsistentStatusScheduler implements InitializingBean {
    private static final List<DeployTypeEnum> SUPPORTED_DEPLOY_TYPE = Arrays.asList(DeployTypeEnum.X9000,
        DeployTypeEnum.A8000, DeployTypeEnum.X8000, DeployTypeEnum.X6000, DeployTypeEnum.X3000);

    private final ProtectObjectConsistentService protectObjectService;

    private final DeployTypeService deployTypeService;

    private final SystemSwitchInternalService switchService;

    /**
     * 构造器注入
     *
     * @param protectObjectService protectObjectService
     * @param deployTypeService deployTypeService
     * @param switchService switchService
     */
    public ProtectObjectConsistentStatusScheduler(ProtectObjectConsistentService protectObjectService,
            DeployTypeService deployTypeService, SystemSwitchInternalService switchService) {
        this.protectObjectService = protectObjectService;
        this.deployTypeService = deployTypeService;
        this.switchService = switchService;
    }

    /**
     * 每隔14天执行一次标记刷新任务
     */
    @Scheduled(cron = "${system.schedule.protectobject.consistent.refresh}")
    public void refreshProtectObjectConsistentStatus() {
        if (isNeedVerify()) {
            protectObjectService.refreshProtectObjectConsistentStatus();
        }
    }

    /**
     * 每隔1天执行一次标记检测任务
     */
    @Scheduled(cron = "${system.schedule.protectobject.consistent.check}")
    public void checkProtectObjectConsistentStatus() {
        if (isNeedVerify()) {
            protectObjectService.checkProtectObjectConsistentStatus(false);
        }
    }

    @Override
    public void afterPropertiesSet() {
        if (isNeedVerify()) {
            protectObjectService.checkProtectObjectConsistentStatus(true);
        }
    }

    private boolean isNeedVerify() {
        return isEnableVerifySwitch() && isSupportedDeployType();
    }

    private boolean isEnableVerifySwitch() {
        SystemSwitchDto switchDto = switchService.queryByName(SwitchNameEnum.PROTECT_OBJECT_FILE_SYSTEM_VERIFY);
        return SwitchStatusEnum.ON.equals(switchDto.getStatus());
    }

    private boolean isSupportedDeployType() {
        DeployTypeEnum deployType = deployTypeService.getDeployType();
        return SUPPORTED_DEPLOY_TYPE.contains(deployType);
    }
}