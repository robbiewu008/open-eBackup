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
package openbackup.data.access.framework.protection.service.job;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.client.sdk.api.framework.dme.DmeUnifiedRestApi;
import openbackup.data.protection.access.provider.sdk.job.JobProvider;

import org.springframework.stereotype.Component;

/**
 * 统一备份框架Job提供者，负责任务的终止
 *
 */
@Slf4j
@Component("unifiedJobProvider")
public class UnifiedJobProvider implements JobProvider {
    private final DmeUnifiedRestApi dmeUnifiedRestApi;

    /**
     * 构造函数
     *
     * @param dmeUnifiedRestApi DME统一备份框架REST接口
     */
    public UnifiedJobProvider(DmeUnifiedRestApi dmeUnifiedRestApi) {
        this.dmeUnifiedRestApi = dmeUnifiedRestApi;
    }

    @Override
    public void stopJob(String jobId) {
        log.debug("Begin to send stop job command to datamover engine. job id is {}", jobId);
        this.dmeUnifiedRestApi.abortJob(jobId, jobId);
        log.debug("Success to send stop job command to datamover engine.");
    }

    @Override
    public boolean applicable(String object) {
        return false;
    }
}
