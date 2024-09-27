/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.sdk.systembackup;

import openbackup.system.base.common.rest.CommonFeignConfiguration;
import openbackup.system.base.sdk.systembackup.model.SystemBackupResponse;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.ResponseBody;

/**
 * system backup service
 *
 * @author dWX1009286
 * @since 2021-09-27
 */
@FeignClient(name = "SystemBackupRestApi", url = "${service.url.pm-system-base}/v1/internal/sysbackup",
    configuration = CommonFeignConfiguration.class)
public interface SystemBackupRestApi {
    /**
     * 获取默认备份策略id
     *
     * @return 备份策略
     */
    @ExterAttack
    @GetMapping("/policy")
    @ResponseBody
    long getPolicy();

    /**
     * 根据备份策略执行备份动作
     *
     * @param desc 备份描述
     * @return 备份结果
     */
    @ExterAttack
    @PostMapping("/policy/action/run")
    @ResponseBody
    SystemBackupResponse backup(String desc);

    /**
     * dme-a服务开启后，将原管理数据标记为无效
     */
    @PostMapping("/mark/invalid")
    void markSystemBackupInvalid();

    /**
     * 获取管理数据是否有备份或恢复任务正在执行
     *
     * @return 是否有任务正在执行
     */
    @ExterAttack
    @GetMapping("/action/status")
    boolean isRunningWork();
}
