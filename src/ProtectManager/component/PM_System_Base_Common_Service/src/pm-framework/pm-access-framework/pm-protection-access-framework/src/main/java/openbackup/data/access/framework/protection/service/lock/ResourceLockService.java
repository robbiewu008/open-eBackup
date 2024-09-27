package openbackup.data.access.framework.protection.service.lock;

import openbackup.data.access.framework.protection.common.constants.ResourceLockLabelConstant;
import openbackup.data.access.framework.protection.service.job.JobLogRecorder;
import openbackup.system.base.common.errors.ResourceLockErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.job.model.JobLogLevelEnum;
import openbackup.system.base.sdk.lock.LockRequest;
import openbackup.system.base.sdk.lock.LockResponse;
import openbackup.system.base.sdk.lock.ResourceLockRestApi;
import openbackup.system.base.service.DeployTypeService;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Service;

import java.util.Collections;

/**
 * 资源锁服务
 *
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2022/1/26
 **/
@Slf4j
@Service
public class ResourceLockService {
    private final ResourceLockRestApi resourceLockRestApi;
    private final JobLogRecorder jobLogRecorder;

    private final DeployTypeService deployTypeService;

    /**
     * 资源锁服务构造函数
     *
     * @param resourceLockRestApi 资源锁服务rest api
     * @param jobLogRecorder 任务日志记录器
     * @param deployTypeService 部署类型服务
     */
    public ResourceLockService(ResourceLockRestApi resourceLockRestApi, JobLogRecorder jobLogRecorder,
        DeployTypeService deployTypeService) {
        this.resourceLockRestApi = resourceLockRestApi;
        this.jobLogRecorder = jobLogRecorder;
        this.deployTypeService = deployTypeService;
    }

    /**
     * 锁定指定资源
     *
     * @param lockRequest 锁定资源请求
     * @return result 是否解锁成功
     */
    public boolean lock(LockRequest lockRequest) {
        // 记录加锁步骤开始
        jobLogRecorder.recordJobStep(
                lockRequest.getRequestId(), JobLogLevelEnum.INFO, ResourceLockLabelConstant.LOCK_START_KEY, null);
        try {
            LockResponse response = resourceLockRestApi.lock(lockRequest);
            boolean isLockSuccess = response.isSuccess();
            log.info("lock resource finished, result={}, resources={}", isLockSuccess, response.getFailedResource());
            if (isLockSuccess) {
                jobLogRecorder.recordJobStep(lockRequest.getRequestId(), JobLogLevelEnum.INFO,
                    ResourceLockLabelConstant.LOCK_SUCCESS_KEY, null);
                return Boolean.TRUE;
            }
            jobLogRecorder.recordJobStepWithError(lockRequest.getRequestId(), ResourceLockLabelConstant.LOCK_FAILED_KEY,
                deployTypeService.isCyberEngine()
                    ? ResourceLockErrorCode.OCEAN_CYBER_RESOURCE_ALREADY_LOCKED
                    : ResourceLockErrorCode.RESOURCE_ALREADY_LOCKED,
                Collections.singletonList(response.getFailedResource()));
            return Boolean.FALSE;
            // 记录加锁成功步骤
        } catch (Exception ex) {
            log.error("lock resource error", ex);
            // 记录加锁失败步骤
            jobLogRecorder.recordJobStepWithError(lockRequest.getRequestId(), ResourceLockLabelConstant.LOCK_FAILED_KEY,
                deployTypeService.isCyberEngine()
                    ? ResourceLockErrorCode.OCEAN_CYBER_RESOURCE_ALREADY_LOCKED
                    : ResourceLockErrorCode.RESOURCE_ALREADY_LOCKED, null);
            return Boolean.FALSE;
        }
    }

    /**
     * 解锁资源
     *
     * @param requestId 请求id
     * @param lockId 资源锁id
     * @param isNeedLog 是否需要记录解锁步骤日志
     */
    public void unlock(String requestId, String lockId, boolean isNeedLog) {
        if (isNeedLog) {
            // 记录解锁步骤开始
            jobLogRecorder.recordJobStep(requestId, JobLogLevelEnum.INFO, ResourceLockLabelConstant.UNLOCK_START_KEY,
                null);
        }
        try {
            // 执行解锁
            resourceLockRestApi.unlock(lockId, requestId);
            if (isNeedLog) {
                // 记录解锁步骤成功
                jobLogRecorder.recordJobStep(requestId, JobLogLevelEnum.INFO,
                    ResourceLockLabelConstant.UNLOCK_SUCCESS_KEY, null);
            }
        } catch (Exception ex) {
            // 记录解锁步骤失败
            jobLogRecorder.recordJobStep(
                    requestId, JobLogLevelEnum.ERROR, ResourceLockLabelConstant.UNLOCK_FAILED_KEY, null);
            throw new LegoCheckedException("unlock resource failed", ex);
        }
    }
}
