/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.restore.service;

import openbackup.data.access.client.sdk.api.framework.dee.DeeCopiesManagementRestApi;
import openbackup.data.access.client.sdk.api.framework.dme.DmeUnifiedRestApi;
import openbackup.data.access.framework.copy.mng.constant.CopyPropertiesKeyConstant;
import openbackup.data.access.framework.core.copy.CopyManagerService;
import openbackup.data.access.framework.core.dao.CopiesAntiRansomwareDao;
import openbackup.data.access.framework.protection.service.lock.ResourceLockService;
import openbackup.data.access.framework.protection.service.repository.RepositoryStrategyManager;
import openbackup.data.access.framework.restore.dto.RestoreTaskContext;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.constants.RestoreTaskExtendInfoConstant;
import openbackup.data.protection.access.provider.sdk.enums.ProviderJobStatusEnum;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.restore.v2.DeeCopiesRelatedTask;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.data.protection.access.provider.sdk.util.AgentApiUtil;
import com.huawei.oceanprotect.job.constants.JobExtendInfoKeys;
import com.huawei.oceanprotect.job.constants.JobPayloadKeys;
import com.huawei.oceanprotect.job.dto.JobProgressRange;
import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyStatus;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.job.model.request.CreateJobRequest;
import openbackup.system.base.service.AvailableAgentManagementDomainService;
import openbackup.system.base.service.DeployTypeService;
import com.huawei.oceanprotect.system.base.user.service.ResourceSetApi;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.net.URI;
import java.util.List;

/**
 * 恢复任务服务类，用于辅助RestoreTaskManager处理恢复业务流程
 *
 * @author y00559272
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-12-02
 **/
@Slf4j
@Service
public class RestoreTaskService {
    private static final String ENFORCE_STOP = "enforce-stop";

    private static final String LOCK_ID = "lock_id";

    private final CopyRestApi copyRestApi;

    private final JobService jobService;

    private final DmeUnifiedRestApi dmeUnifiedRestApi;

    private final RepositoryStrategyManager repositoryStrategyManager;

    private final ResourceLockService resourceLockService;

    private AvailableAgentManagementDomainService domainService;

    private DeployTypeService deployTypeService;

    private DeeCopiesManagementRestApi deeCopiesManagementRestApi;

    private CopyManagerService copyManagerService;

    private CopiesAntiRansomwareDao copiesAntiRansomwareDao;

    private ResourceSetApi resourceSetApi;

    /**
     * 恢复任务服务构造函数
     *
     * @param copyRestApi 副本Rest服务
     * @param jobService 任务服务
     * @param dmeUnifiedRestApi dme统一框架Rest Api
     * @param repositoryStrategyManager 存储库管理类
     * @param resourceLockService 资源锁服务
     */
    public RestoreTaskService(CopyRestApi copyRestApi, JobService jobService, DmeUnifiedRestApi dmeUnifiedRestApi,
        RepositoryStrategyManager repositoryStrategyManager, ResourceLockService resourceLockService) {
        this.copyRestApi = copyRestApi;
        this.jobService = jobService;
        this.dmeUnifiedRestApi = dmeUnifiedRestApi;
        this.repositoryStrategyManager = repositoryStrategyManager;
        this.resourceLockService = resourceLockService;
    }

    @Autowired
    public void setResourceSetApi(ResourceSetApi resourceSetApi) {
        this.resourceSetApi = resourceSetApi;
    }

    @Autowired
    public void setAvailableAgentManagementDomainService(AvailableAgentManagementDomainService domainService) {
        this.domainService = domainService;
    }


    @Autowired
    public void setDeployTypeService(DeployTypeService deployTypeService) {
        this.deployTypeService = deployTypeService;
    }

    @Autowired
    public void setDeeCopiesManagementRestApi(DeeCopiesManagementRestApi deeCopiesManagementRestApi) {
        this.deeCopiesManagementRestApi = deeCopiesManagementRestApi;
    }

    @Autowired
    public void setCopyManagerService(CopyManagerService copyManagerService) {
        this.copyManagerService = copyManagerService;
    }

    @Autowired
    public void setCopiesAntiRansomwareDao(CopiesAntiRansomwareDao copiesAntiRansomwareDao) {
        this.copiesAntiRansomwareDao = copiesAntiRansomwareDao;
    }

    /**
     * 创建恢复任务
     *
     * @param context 恢复任务上下文
     * @return 任务id
     */
    public String createJob(RestoreTaskContext context) {
        CreateJobRequest jobRequest = RestoreTaskHelper.buildRestoreJobRequest(context);
        jobRequest.setDomainIdList(resourceSetApi.getRelatedDomainIdList(jobRequest.getCopyId()));
        addJobQueueScope(context, jobRequest);
        return jobService.createJob(jobRequest);
    }

    private void addJobQueueScope(RestoreTaskContext context, CreateJobRequest jobRequest) {
        JSONObject payload = jobRequest.getMessage().getPayload();
        RestoreTask restoreTask = context.getRestoreTask();
        TaskResource resource = restoreTask.getTargetObject();
        String jobQueueScope = jobService.extractJobQueueScope(resource.getSubType(), JobTypeEnum.RESTORE.getValue());
        if (StringUtils.isNotBlank(jobQueueScope)) {
            payload.put(JobPayloadKeys.KEY_QUEUE_JOB_TYPE, JobTypeEnum.RESTORE.getValue());
            payload.put(jobQueueScope, resource.getRootUuid());
        }
    }

    /**
     * 修改job为指定状态
     *
     * @param jobId 任务id
     * @param status 需要修改成的任务状态
     */
    void updateJobStatus(String jobId, ProviderJobStatusEnum status) {
        log.info("Restore task update job status, jobId: {}, status: {}", jobId, status.name());
        jobService.updateJob(jobId, RestoreTaskHelper.buildJobRequestWithStatus(status));
    }

    /**
     * 指定job的enforce_stop为false，使job下发给dme后无法在pm侧直接被删除
     *
     * @param jobId 任务id
     */
    void updateJobEnforceStopToFalse(String jobId) {
        jobService.updateJob(jobId,
            RestoreTaskHelper.buildJobRequestWithData(new JSONObject().set(ENFORCE_STOP, false)));
    }

    /**
     * 将资源锁id更新到任务data字段中
     *
     * @param jobId 任务id/资源锁id
     */
    void updateJobLockId(String jobId) {
        jobService.updateJob(jobId, RestoreTaskHelper.buildJobRequestWithData(new JSONObject().set(LOCK_ID, jobId)));
    }

    void modifyJobProgressRange(String jobId) {
        jobService.modifyProgressRange(jobId, new JobProgressRange(RestoreTaskExtendInfoConstant.RESTORE_RANGE_START,
            RestoreTaskExtendInfoConstant.RESTORE_RANGE_END));
    }

    /**
     * 修改副本为指定状态
     *
     * @param copyId 副本id
     * @param status 需要修改成的状态
     */
    void updateCopyStatus(String copyId, CopyStatus status) {
        log.info("Restore task update copy status, copyId: {}, status: {}", copyId, status.name());
        copyManagerService.updateCopyStatus(copyId, status.getValue());
    }

    /**
     * 修改副本为初始状态
     *
     * @param jobId 任务id
     * @param copyId 副本id
     */
    void updateCopyToOriginalStatus(String jobId, String copyId) {
        final JobBo jobBo = jobService.queryJob(jobId);
        JSONObject jobExtendInfo = JSONObject.fromObject(jobBo.getExtendStr());
        String copyStatus = jobExtendInfo.getString(JobExtendInfoKeys.RESTORE_COPY_ORIGINAL_STATUS,
            CopyStatus.NORMAL.getValue());
        this.updateCopyStatus(copyId, CopyStatus.get(copyStatus));
    }

    /**
     * 根据副本信息查询副本信息
     *
     * @param copyId 副本id
     * @return 副本对象 {@code Copy}
     */
    Copy queryCopyDetail(String copyId) {
        return copyRestApi.queryCopyByID(copyId);
    }

    /**
     * 调用DME\DEE接口下发恢复任务
     *
     * @param restoreTask 恢复任务请求对象
     */
    void startTask(RestoreTask restoreTask) {
        if (deployTypeService.isHyperDetectDeployType() || deployTypeService.isCyberEngine()) {
            log.info("begin to send restore task({}) to dee.", restoreTask.getRequestId());
            deeCopiesManagementRestApi
                .restoreFsSnapshot(buildDeeRestoreTask(restoreTask.getTaskId(), restoreTask.getCopyId()));
            log.info("send restore task({}) to dee successfully.", restoreTask.getRequestId());
        } else {
            URI uri = domainService.getUrlByAgents(AgentApiUtil.getAgentIds(restoreTask.getAgents()));
            log.info("restore uri:{}, task id:{}", JSONObject.stringify(uri), restoreTask.getRequestId());
            dmeUnifiedRestApi.createRestoreTask(uri, restoreTask);
            log.info("Restore task start, send to DME success, requestId={}.", restoreTask.getRequestId());
        }
    }

    /**
     * 锁定恢复任务资源
     * <p>
     * 资源锁定之后一定要解锁，解锁方法请见 {@link #unlockResources}
     * </p>
     *
     * @param requestId 请求id
     * @param copyId 副本id
     * @param resources 需要锁定的资源列表
     * @return result 是否解锁成功
     */
    boolean lockResources(String requestId, String copyId, List<LockResourceBo> resources) {
        log.info("Restore task lock resource, requestId={}", requestId);
        return resourceLockService.lock(RestoreTaskHelper.buildLockRequest(requestId, copyId, resources));
    }

    /**
     * 解锁恢复任务资源
     *
     * @param requestId 请求id
     */
    void unlockResources(String requestId) {
        log.info("Restore task unlock resource, requestId={}", requestId);
        resourceLockService.unlock(requestId, requestId, true);
    }

    /**
     * 从任务信息中获取恢复任务的消息对象
     *
     * @param jobId 任务id
     * @return 恢复任务信息
     */
    public RestoreTask getRestoreTaskFromJob(String jobId) {
        final JobBo jobBo = jobService.queryJob(jobId);
        return RestoreTaskHelper.parseFromJobMessage(jobBo.getMessage());
    }

    /**
     * 构造DeeRestoreTask
     *
     * @param taskId taskId
     * @param copyId copyId
     * @return DeeRestoreTask
     */
    private DeeCopiesRelatedTask buildDeeRestoreTask(String taskId, String copyId) {
        Copy copy = queryCopyDetail(copyId);
        String copyPropertiesStr = copy.getProperties();
        JSONObject copyProperties = JSONObject.fromObject(copyPropertiesStr);
        DeeCopiesRelatedTask deeCopiesRelatedTask = new DeeCopiesRelatedTask();
        deeCopiesRelatedTask.setTaskId(taskId);
        deeCopiesRelatedTask.setRequestId(taskId);
        deeCopiesRelatedTask.setSubType(copy.getResourceSubType());
        deeCopiesRelatedTask.setSnapshotId(copyProperties.getString(
            CopyPropertiesKeyConstant.KEY_PROPERTIES_SNAPSHOT_ID));
        deeCopiesRelatedTask.setSnapshotName(copyProperties.getString(
            CopyPropertiesKeyConstant.KEY_PROPERTIES_SNAPSHOT_NAME));
        deeCopiesRelatedTask.setFilesystemId(copyProperties.getString(
            CopyPropertiesKeyConstant.KEY_PROPERTIES_FILESYSTEM_ID));
        deeCopiesRelatedTask.setFilesystemName(copyProperties.getString(
            CopyPropertiesKeyConstant.KEY_PROPERTIES_FILESYSTEM_NAME));
        deeCopiesRelatedTask.setVstoreId(copyProperties.getString(
            CopyPropertiesKeyConstant.KEY_PROPERTIES_TENANT_ID));
        deeCopiesRelatedTask.setVstoreName(copyProperties.getString(
            CopyPropertiesKeyConstant.KEY_PROPERTIES_TENANT_NAME));
        String resourcePropertiesStr = copy.getResourceProperties();
        JSONObject resourceProperties = JSONObject.fromObject(resourcePropertiesStr);
        deeCopiesRelatedTask.setDeviceId(resourceProperties.getString(
            CopyPropertiesKeyConstant.KEY_RESOURCE_PROPERTIES_ROOT_UUID));
        deeCopiesRelatedTask.setDeviceName(resourceProperties.getString(
            CopyPropertiesKeyConstant.KEY_RESOURCE_PROPERTIES_ENVIRONMENT_NAME));
        log.info("restore task is {}", deeCopiesRelatedTask);
        return deeCopiesRelatedTask;
    }
}
