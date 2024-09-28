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

import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.handler.v2.UnifiedTaskCompleteHandler;
import openbackup.data.protection.access.provider.sdk.job.JobCallbackProvider;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreInterceptorProvider;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.sdk.copy.model.CopyStatus;
import openbackup.system.base.sdk.job.model.JobTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.Arrays;

/**
 * 强制恢复任务中止回调
 *
 */
@Slf4j
@Component
public class RestoreJobCallbackProvider implements JobCallbackProvider {
    private final RestoreTaskService restoreTaskService;

    @Autowired
    private ProviderManager providerManager;

    /**
     * 统一恢复任务强制停止处理器
     *
     * @param restoreTaskService 恢复任务服务
     */
    public RestoreJobCallbackProvider(RestoreTaskService restoreTaskService) {
        this.restoreTaskService = restoreTaskService;
    }

    @Override
    public boolean applicable(String jobType) {
        return Arrays.asList(
                JobTypeEnum.RESTORE.getValue() + "-" + UnifiedTaskCompleteHandler.V2,
                JobTypeEnum.INSTANT_RESTORE.getValue() + "-" + UnifiedTaskCompleteHandler.V2,
                JobTypeEnum.RESTORE.getValue(),
                JobTypeEnum.INSTANT_RESTORE.getValue())
            .contains(jobType);
    }

    @Override
    public void doCallback(JobBo job) {
        if (job.getCopyId() == null) {
            log.info("The job has no copy id now.");
            return;
        }
        restoreTaskService.updateCopyStatus(job.getCopyId(), CopyStatus.NORMAL);

        String sourceSubType = job.getSourceSubType();
        RestoreInterceptorProvider restoreInterceptorProvider = providerManager.findProvider(
            RestoreInterceptorProvider.class, sourceSubType,
            new LegoCheckedException("Restore task can not find provider."));
        restoreInterceptorProvider.longTimeStopProcess(job);
    }
}
