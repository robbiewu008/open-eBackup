/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2022. All rights reserved.
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
 * @author l00570077
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-09-24
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