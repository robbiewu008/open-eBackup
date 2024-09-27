package openbackup.data.access.framework.protection.handler.v2.archive;

import openbackup.data.access.framework.core.common.enums.DmcJobStatus;
import openbackup.data.access.framework.protection.handler.v2.UnifiedTaskCompleteHandler;
import openbackup.data.access.framework.protection.service.archive.ArchiveTaskManager;
import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import openbackup.system.base.sdk.job.model.JobTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

/**
 * 统一归档任务完成处理器
 *
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2022/1/12
 **/
@Slf4j
@Component
public class UnifiedArchiveTaskCompleteHandler extends UnifiedTaskCompleteHandler {
    private final ArchiveTaskManager archiveTaskManager;

    public UnifiedArchiveTaskCompleteHandler(ArchiveTaskManager archiveTaskManager) {
        this.archiveTaskManager = archiveTaskManager;
    }

    @Override
    public void onTaskCompleteSuccess(TaskCompleteMessageBo taskCompleteMessage) {
        log.debug("Archive task success.");
        archiveTaskManager.archiveSuccess(taskCompleteMessage.getJobRequestId(),
            DmcJobStatus.getByStatus(taskCompleteMessage.getJobStatus()), taskCompleteMessage.getExtendsInfo());
    }

    @Override
    public void onTaskCompleteFailed(TaskCompleteMessageBo taskCompleteMessage) {
        log.debug("Archive task failed.");
        archiveTaskManager.archiveFailed(taskCompleteMessage.getJobRequestId(),
                DmcJobStatus.getByStatus(taskCompleteMessage.getJobStatus()));
    }

    @Override
    public boolean applicable(String object) {
        String jobType = JobTypeEnum.ARCHIVE.getValue() + "-" + version();
        return jobType.equals(object);
    }
}
