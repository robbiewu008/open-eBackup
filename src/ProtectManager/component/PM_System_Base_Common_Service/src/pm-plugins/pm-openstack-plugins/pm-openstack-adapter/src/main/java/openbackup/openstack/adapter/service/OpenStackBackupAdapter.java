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
package openbackup.openstack.adapter.service;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.openstack.adapter.constants.OpenStackConstants;
import openbackup.openstack.adapter.dto.JobScheduleDto;
import openbackup.openstack.adapter.dto.OpenStackBackupJobDto;
import openbackup.openstack.adapter.enums.OpenStackJobType;
import openbackup.openstack.adapter.generator.OpenStackModelsGenerator;
import openbackup.openstack.adapter.generator.ProtectionGenerator;
import com.huawei.oceanprotect.sla.sdk.dto.SlaDto;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyAction;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.enums.ProtectionStatusEnum;
import openbackup.system.base.sdk.resource.model.ProtectedObjectInfo;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.ObjectUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Objects;

/**
 * OpenStack备份适配器
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.3.0]
 * @since 2022-12-22
 */
@Slf4j
@Component
public class OpenStackBackupAdapter {
    private final ProtectionManager protectionManager;
    private final SlaManager slaManager;
    private final OpenStackJobManager jobManager;
    private final OpenStackResourceManager resourceManager;

    public OpenStackBackupAdapter(
            ProtectionManager protectionManager,
            SlaManager slaManager,
            OpenStackJobManager jobManager,
            OpenStackResourceManager resourceManager) {
        this.protectionManager = protectionManager;
        this.slaManager = slaManager;
        this.jobManager = jobManager;
        this.resourceManager = resourceManager;
    }

    /**
     * 创建备份任务
     *
     * @param backupJob 创建备份任务请求体
     * @return openStackBackupJobDto
     */
    public OpenStackBackupJobDto createJob(OpenStackBackupJobDto backupJob) {
        ProtectedResource resource = getProtectedResource(backupJob.getType(), backupJob.getInstanceId());
        String resourceId = resource.getUuid();
        ProtectedObjectInfo protectedObject = protectionManager.queryProtectedObject(resourceId, false);
        JobScheduleDto jobsSchedule = backupJob.getJobsSchedule();

        if (jobsSchedule == null) {
            // jobsSchedule为空则执行手动全量备份
            executeManualBackup(backupJob, resourceId, protectedObject);
        } else {
            // 对资源创建备份时，该资源不能为已保护
            protectResource(backupJob, resourceId, protectedObject);
        }
        OpenStackBackupJobDto response = buildResponse(backupJob, resource);
        log.info("OpenStack create job of resource: {} success, instance id: {}, status: {}.", resourceId,
            backupJob.getInstanceId(), backupJob.getStatus());
        return response;
    }

    private ProtectedResource getProtectedResource(OpenStackJobType jobType, String instanceId) {
        ProtectedResource resource;
        if (Objects.equals(jobType, OpenStackJobType.VOLUME)) {
            resource = resourceManager.queryResourceByVolumeId(instanceId)
                .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR,
                    String.format("Has no host contains this volume id: %s.", instanceId)));
        } else {
            resource = resourceManager.queryResourceById(instanceId)
                .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR,
                    String.format("Resource: %s not exist.", instanceId)));
        }
        return resource;
    }

    private OpenStackBackupJobDto buildResponse(OpenStackBackupJobDto backupJob, ProtectedResource resource) {
        JobBo job = jobManager.queryLatestJob(resource.getUuid(), JobTypeEnum.BACKUP.getValue()).orElse(null);

        return OpenStackModelsGenerator.generateBackupJobResp(backupJob, job, resource);
    }

    private void executeManualBackup(OpenStackBackupJobDto backupJob, String resourceId,
        ProtectedObjectInfo protectedObject) {
        String slaId = protectedObject == null ? OpenStackConstants.GLOBAL_SLA_ID : protectedObject.getSlaId();
        log.info("Openstack execute manual backup, sla: {}, resource: {}.", slaId, resourceId);
        // 如果资源未保护，先对资源绑定预置SLA，将保护对象置为未激活
        if (protectedObject == null) {
            String jobId = protectionManager.createProtection(slaId, resourceId, backupJob);
            if (!jobManager.isJobSuccess(jobId)) {
                jobManager.forceStopJob(jobId);
                // 防止资源一直处于保护中
                resourceManager.updateProtectionStatus(resourceId, ProtectionStatusEnum.UNPROTECTED.getType());
                throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR,
                    String.format("Protection job: %s is not success.", jobId));
            }
            // 保护成功后，防止周期性执行任务，对保护对象取消激活
            protectionManager.deactivate(resourceId);
        }
        protectionManager.manualBackup(resourceId, slaId, PolicyAction.FULL.getAction());
    }

    private void protectResource(OpenStackBackupJobDto backupJob, String resourceId,
        ProtectedObjectInfo protectedObject) {
        if (protectedObject != null) {
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR,
                String.format(Locale.ENGLISH, "Resource: %s is already under protected.", resourceId));
        }
        String slaId = slaManager.createSla(backupJob);
        String jobId = protectionManager.createProtection(slaId, resourceId, backupJob);
        log.info("Openstack create protection, resource: {}, sla id: {}, job id: {}.", resourceId, slaId, jobId);
        if (!jobManager.isJobSuccess(jobId)) {
            jobManager.forceStopJob(jobId);
            // 防止资源一直处于保护中
            resourceManager.updateProtectionStatus(resourceId, ProtectionStatusEnum.UNPROTECTED.getType());
            // 保护任务未成功，删除之前创建的SLA
            slaManager.deleteSla(slaId);
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR,
                String.format("Protection job: %s is not success.", jobId));
        }
    }

    /**
     * 更新备份任务
     *
     * @param resourceId 资源id
     * @param backupJob 更新备份任务请求体
     * @return OpenStackBackupJobDto
     */
    public OpenStackBackupJobDto updateJob(String resourceId, OpenStackBackupJobDto backupJob) {
        ProtectedObjectInfo protectedObject = protectionManager.queryProtectedObject(resourceId, true);
        if (backupJob.getJobsSchedule() == null) {
            executeManualBackup(backupJob, resourceId, protectedObject);
        } else {
            updateProtection(backupJob, resourceId, protectedObject);
        }
        return this.queryJob(resourceId);
    }

    private void updateProtection(
            OpenStackBackupJobDto backupJob, String resourceId, ProtectedObjectInfo protectedObject) {
        SlaDto oldSla = slaManager.querySla(protectedObject.getSlaId());
        String slaId = slaManager.updateSla(backupJob, oldSla);
        log.info("Openstack update protection, sla: {}, resource: {}.", slaId, resourceId);
        if (!needModifyProtection(backupJob.getDescription(), protectedObject)) {
            return;
        }
        String jobId = protectionManager.modifyProtection(slaId, backupJob, protectedObject);
        if (!jobManager.isJobSuccess(jobId)) {
            resourceManager.updateProtectionStatus(resourceId, protectedObject.getStatus());
            throw new LegoCheckedException(
                    CommonErrorCode.SYSTEM_ERROR, String.format("Modify protection job: %s was fail.", jobId));
        }
    }

    private boolean needModifyProtection(String description, ProtectedObjectInfo protectedObject) {
        Map<String, Object> extParams = protectedObject.getExtParameters();
        String oldDescription = extParams.getOrDefault(OpenStackConstants.DESCRIPTION, StringUtils.EMPTY).toString();
        log.debug(
                "Openstack modify protection, new description: {}, old description: {}.", description, oldDescription);
        return ObjectUtils.notEqual(description, oldDescription);
    }

    /**
     * 停止任务
     * 映射为系统中禁用保护功能
     *
     * @param resourceId 待禁用保护的资源id
     */
    public void stopJob(String resourceId) {
        ProtectedObjectInfo protectedObject = protectionManager.queryProtectedObject(resourceId, true);
        if (protectedObject.getStatus() == ProtectionStatusEnum.UNPROTECTED.getType()) {
            log.info("Protected object: {} is already unprotected.", resourceId);
            return;
        }
        protectionManager.deactivate(resourceId);
    }

    /**
     * 开始任务
     * 映射为系统中激活保护功能，并执行手动备份
     *
     * @param resourceId 待激活保护的资源id
     */
    public void startJob(String resourceId) {
        ProtectedObjectInfo protectedObject = protectionManager.queryProtectedObject(resourceId, true);
        if (protectedObject.getStatus() == ProtectionStatusEnum.PROTECTED.getType()) {
            log.info("Protected object: {} is already under protected.", resourceId);
            return;
        }
        protectionManager.activate(resourceId);
        protectionManager.manualBackup(
                protectedObject.getResourceId(),
                protectedObject.getSlaId(),
                PolicyAction.DIFFERENCE_INCREMENT.getAction());
    }

    /**
     * 查询单个任务
     *
     * @param resourceId 资源id
     * @return {@link OpenStackBackupJobDto} 云核OpenStack备份任务
     */
    public OpenStackBackupJobDto queryJob(String resourceId) {
        ProtectedObjectInfo protectedObject = protectionManager.queryProtectedObject(resourceId, true);
        SlaDto sla = slaManager.querySla(protectedObject.getSlaId());
        JobBo job = jobManager.queryLatestJob(resourceId, JobTypeEnum.BACKUP.getValue()).orElse(null);
        return OpenStackModelsGenerator.generateBackupJob(protectedObject, sla, job);
    }

    /**
     * 查询project下所有备份任务
     *
     * @param projectId projectId
     * @return 备份任务列表
     */
    public List<OpenStackBackupJobDto> queryJobs(String projectId) {
        List<ProtectedResource> resources = resourceManager.queryResourcesByProjectId(projectId, true);
        List<OpenStackBackupJobDto> backupJobs = new ArrayList<>();
        for (ProtectedResource resource : resources) {
            ProtectedObjectInfo objectInfo =
                    ProtectionGenerator.convert2ProtectedObjectInfo(resource.getProtectedObject());
            SlaDto sla = slaManager.querySla(objectInfo.getSlaId());
            JobBo job = jobManager.queryLatestJob(objectInfo.getResourceId(), JobTypeEnum.BACKUP.getValue())
                .orElse(null);
            backupJobs.add(OpenStackModelsGenerator.generateBackupJob(objectInfo, sla, job));
        }
        return backupJobs;
    }

    /**
     * 删除备份任务
     * 1. 查询保护是否存在
     * 2. 删除保护
     * 3. 删除SLA
     *
     * @param resourceId 资源id
     */
    public void deleteJob(String resourceId) {
        ProtectedObjectInfo protectedObject = protectionManager.queryProtectedObject(resourceId, true);
        String slaId = protectedObject.getSlaId();
        protectionManager.delete(protectedObject.getResourceId());
        slaManager.deleteSla(slaId);
    }
}
