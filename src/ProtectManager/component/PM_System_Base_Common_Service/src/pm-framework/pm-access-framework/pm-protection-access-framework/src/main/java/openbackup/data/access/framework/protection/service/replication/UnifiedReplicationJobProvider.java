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
package openbackup.data.access.framework.protection.service.replication;

import com.huawei.oceanprotect.functionswitch.template.service.FunctionSwitchService;
import com.huawei.oceanprotect.job.constants.JobExtendInfoKeys;
import com.huawei.oceanprotect.job.dto.JobSlaDetail;
import com.huawei.oceanprotect.sla.sdk.api.SlaQueryService;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.client.sdk.api.framework.dme.replicate.AdvanceReplicationJob;
import openbackup.data.access.client.sdk.api.framework.dme.replicate.DmeReplicateService;
import openbackup.data.access.framework.protection.common.util.JobExtendInfoUtil;
import openbackup.data.protection.access.provider.sdk.job.JobProvider;
import openbackup.data.protection.access.provider.sdk.job.ProviderJobMessage;
import openbackup.system.base.common.model.job.Job;
import openbackup.system.base.sdk.job.model.JobTypeEnum;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

/**
 * Unified Replication job provider
 *
 */
@Slf4j
@Component
public class UnifiedReplicationJobProvider implements JobProvider {
    @Autowired
    private DmeReplicateService dmeReplicateService;

    @Autowired
    private FunctionSwitchService functionSwitchService;

    @Autowired
    private SlaQueryService slaQueryService;

    @Override
    public ProviderJobMessage queryJobMessage(String associativeId, long startTime) {
        // 高级备份复制特性不会调用该方法
        return new ProviderJobMessage();
    }

    @Override
    public void stopJob(String planId) {
        AdvanceReplicationJob advanceReplicationJob = new AdvanceReplicationJob();
        advanceReplicationJob.setTaskId(planId);
        dmeReplicateService.abortReplicationTask(advanceReplicationJob);
        log.info("send abort replication task success. request id: {}", planId);
    }

    @Override
    public void fillJobInfo(Job insertJob) {
        JobExtendInfoUtil.fillJobPolicyInfo(insertJob, slaQueryService::querySlaById,
            ext -> JobExtendInfoUtil.getExtInfo(ext, JobExtendInfoKeys.SLA_ID), JobSlaDetail.class, null);
    }

    @Override
    public boolean applicable(String object) {
        if (object == null) {
            return false;
        }
        if (JobTypeEnum.COPY_REPLICATION.getValue().equals(object)) {
            return true;
        }
        return object.endsWith("_" + JobTypeEnum.COPY_REPLICATION.getValue());
    }
}
