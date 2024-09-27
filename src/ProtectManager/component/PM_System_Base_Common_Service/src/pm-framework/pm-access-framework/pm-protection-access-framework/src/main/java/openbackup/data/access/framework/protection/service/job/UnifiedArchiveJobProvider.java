/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.protection.service.job;

import openbackup.data.access.client.sdk.api.framework.archive.ArchiveUnifiedRestApi;
import openbackup.data.access.framework.protection.common.util.JobExtendInfoUtil;
import openbackup.data.protection.access.provider.sdk.job.JobProvider;
import com.huawei.oceanprotect.job.constants.JobExtendInfoKeys;
import com.huawei.oceanprotect.job.dto.JobSlaDetail;
import com.huawei.oceanprotect.sla.sdk.api.SlaQueryService;
import openbackup.system.base.common.model.job.Job;
import openbackup.system.base.sdk.job.model.JobTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

/**
 * 统一归档框架任务处理Provider
 *
 * @author y00559272
 * @version [OceanProtect A8000 1.1.0]
 * @since 2022-01-20
 */
@Slf4j
@Component("unifiedArchiveJobProvider")
public class UnifiedArchiveJobProvider implements JobProvider {
    private final ArchiveUnifiedRestApi archiveUnifiedRestApi;

    @Autowired
    private SlaQueryService slaQueryService;

    /**
     * 构造函数
     *
     * @param archiveUnifiedRestApi 归档统一REST接口
     */
    public UnifiedArchiveJobProvider(ArchiveUnifiedRestApi archiveUnifiedRestApi) {
        this.archiveUnifiedRestApi = archiveUnifiedRestApi;
    }

    @Override
    public void stopJob(String jobId) {
        log.debug("Begin to send stop job command to dme-archive. job id is {}", jobId);
        this.archiveUnifiedRestApi.abortTask(jobId, jobId);
        log.debug("Success to send stop job command to dme-archive. job id is {}", jobId);
    }

    @Override
    public void fillJobInfo(Job insertJob) {
        JobExtendInfoUtil.fillJobPolicyInfo(insertJob, slaQueryService::querySlaById,
            ext -> JobExtendInfoUtil.getExtInfo(ext, JobExtendInfoKeys.SLA_ID), JobSlaDetail.class, null);
    }

    @Override
    public boolean applicable(String object) {
        return JobTypeEnum.ARCHIVE.getValue().equals(object);
    }
}
