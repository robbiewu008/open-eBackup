/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.openstack.adapter.service;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.openstack.adapter.constants.OpenStackErrorCodes;
import openbackup.openstack.adapter.dto.OpenStackRestoreJobDto;
import openbackup.openstack.adapter.enums.OpenStackJobType;
import openbackup.openstack.adapter.exception.OpenStackException;
import openbackup.openstack.adapter.generator.OpenStackModelsGenerator;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.sdk.job.model.JobTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

/**
 * OpenStack恢复适配器
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.3.0]
 * @since 2023-01-16
 */
@Slf4j
@Component
public class OpenStackRestoreAdapter {
    private final OpenStackResourceManager resourceManager;
    private final OpenStackRestoreManager restoreManager;
    private final OpenStackJobManager jobManager;

    public OpenStackRestoreAdapter(OpenStackResourceManager resourceManager, OpenStackRestoreManager restoreManager,
        OpenStackJobManager jobManager) {
        this.resourceManager = resourceManager;
        this.restoreManager = restoreManager;
        this.jobManager = jobManager;
    }

    /**
     * 创建恢复任务
     *
     * @param restoreJob 北向接口创建恢复任务请求体
     * @return 创建恢复任务响应体
     */
    public OpenStackRestoreJobDto createJob(OpenStackRestoreJobDto restoreJob) {
        String jobId = restoreManager.createRestoreTask(restoreJob, getProtectedResource(restoreJob));
        restoreJob.setId(jobId);
        return restoreJob;
    }

    private ProtectedResource getProtectedResource(OpenStackRestoreJobDto restoreJob) {
        if (isServerRestore(restoreJob)) {
            return resourceManager
                    .queryResourceById(restoreJob.getInstanceId())
                    .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "Resource not exist."));
        }
        return resourceManager.queryResourceByVolumeId(restoreJob.getInstanceId())
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR,
                String.format("Has no host contains this volume id: %s.", restoreJob.getInstanceId())));
    }

    private boolean isServerRestore(OpenStackRestoreJobDto restoreJob) {
        return Objects.equals(restoreJob.getType(), OpenStackJobType.SERVER);
    }

    /**
     * 查询恢复任务
     *
     * @param jobId 任务id
     * @return {@link OpenStackRestoreJobDto} OpenStack恢复任务详情
     */
    public OpenStackRestoreJobDto queryJob(String jobId) {
        return OpenStackModelsGenerator.generateRestoreJob(getJob(jobId));
    }

    private JobBo getJob(String jobId) {
        try {
            return jobManager.queryJob(jobId);
        } catch (LegoCheckedException exception) {
            throw new OpenStackException(OpenStackErrorCodes.NOT_FOUND,
                String.format("Job: %s is not exists.", jobId));
        }
    }

    /**
     * 查询所有恢复任务
     *
     * @param projectId OpenStack项目id
     * @return 恢复任务列表
     */
    public List<OpenStackRestoreJobDto> queryJobs(String projectId) {
        List<ProtectedResource> resources = resourceManager.queryResourcesByProjectId(projectId, false);
        List<OpenStackRestoreJobDto> restoreJobs = new ArrayList<>();
        for (ProtectedResource resource : resources) {
            List<JobBo> jobs = jobManager.queryAllJobs(resource.getUuid(), JobTypeEnum.RESTORE.getValue());
            for (JobBo job : jobs) {
                restoreJobs.add(OpenStackModelsGenerator.generateRestoreJob(job));
            }
        }
        log.info("Openstack find: {} restore jobs.", restoreJobs.size());
        return restoreJobs;
    }
}
