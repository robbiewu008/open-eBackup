package openbackup.openstack.adapter.service;

import openbackup.data.access.framework.restore.controller.req.CreateRestoreTaskRequest;
import openbackup.data.access.framework.restore.service.RestoreTaskManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.openstack.adapter.dto.OpenStackRestoreJobDto;
import openbackup.openstack.adapter.generator.RestoreGenerator;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

/**
 * OpenStack恢复相关操作管理器
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.3.0]
 * @since 2023-01-16
 */
@Slf4j
@Component
public class OpenStackRestoreManager {
    private final RestoreTaskManager restoreTaskManager;

    public OpenStackRestoreManager(RestoreTaskManager restoreTaskManager) {
        this.restoreTaskManager = restoreTaskManager;
    }

    /**
     * 创建恢复任务
     *
     * @param restoreJob {@link OpenStackRestoreJobDto} 北向接口创建恢复任务请求体
     * @param resource 受保护资源
     * @return 恢复任务id
     */
    public String createRestoreTask(OpenStackRestoreJobDto restoreJob, ProtectedResource resource) {
        CreateRestoreTaskRequest request = RestoreGenerator.generateCreateRestoreReq(restoreJob, resource);
        String jobId = restoreTaskManager.init(request);
        log.info("Openstack create restore job: {} of copy: {} success.", jobId, restoreJob.getCopyId());
        return jobId;
    }
}
