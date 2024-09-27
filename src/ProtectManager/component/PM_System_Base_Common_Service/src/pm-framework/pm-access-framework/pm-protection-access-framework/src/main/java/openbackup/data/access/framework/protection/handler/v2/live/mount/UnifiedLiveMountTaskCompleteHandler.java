package openbackup.data.access.framework.protection.handler.v2.live.mount;

import openbackup.data.access.framework.core.common.constants.ContextConstants;
import openbackup.data.access.framework.core.common.constants.TopicConstants;
import openbackup.data.access.framework.protection.common.constants.JobStatusLabelConstant;
import openbackup.data.access.framework.protection.handler.v2.UnifiedTaskCompleteHandler;
import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import openbackup.system.base.common.msg.NotifyManager;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.job.model.JobTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

/**
 * 统一即时挂载任务完成处理器
 *
 * @author twx1009756
 * @version [OceanProtect A8000 1.1.0]
 * @since 2022-01-18
 */
@Slf4j
@Component
public class UnifiedLiveMountTaskCompleteHandler extends UnifiedTaskCompleteHandler {
    private final NotifyManager notifyManager;

    public UnifiedLiveMountTaskCompleteHandler(NotifyManager notifyManager) {
        this.notifyManager = notifyManager;
    }

    /**
     * 处理挂载任务成功完成
     *
     * @param taskMessage taskMessage
     */
    @Override
    public void onTaskCompleteSuccess(TaskCompleteMessageBo taskMessage) {
        log.info("Live mount task complete, do live mount success, jobId: {}, request id: {}, status: {}.",
            taskMessage.getJobId(), taskMessage.getJobRequestId(), taskMessage.getJobStatus());
        sendLiveMountDoneMessage(taskMessage);
    }

    /**
     * 处理挂载任务失败完成
     *
     * @param taskMessage taskMessage
     */
    @Override
    public void onTaskCompleteFailed(TaskCompleteMessageBo taskMessage) {
        log.error("Live mount task complete failed message, requestId is {}, jobId is {}, task status is {}, abort",
            taskMessage.getJobRequestId(), taskMessage.getProperty(ContextConstants.JOB_ID),
            taskMessage.getJobStatus());
        sendLiveMountDoneMessage(taskMessage);
    }

    /**
     * 处理发送任务完成topic消息
     *
     * @param taskMessage taskMessage
     */
    public void sendLiveMountDoneMessage(TaskCompleteMessageBo taskMessage) {
        int status = taskMessage.getJobStatus();
        String requestId = taskMessage.getJobRequestId();
        String jobId = taskMessage.getProperty(ContextConstants.JOB_ID);
        JSONObject message = new JSONObject()
                .set(ContextConstants.JOB_REQUEST_ID, requestId)
                .set(ContextConstants.JOB_ID, jobId)
                .set(ContextConstants.JOB_STATUS, status)
                .set(ContextConstants.JOB_PROGRESS, taskMessage.getJobProgress())
                .set(ContextConstants.EXTEND_INFO, taskMessage.getExtendsInfo());
        log.info("Send live mount finish msg, jobId: {}, jobStatus: {}", jobId, status);
        notifyManager.send(TopicConstants.TASK_COMPLETE_TOPIC, message.toString());
    }

    /**
     * 适配器
     *
     * @param object object
     * @return true or false
     */
    @Override
    public boolean applicable(String object) {
        String jobType = JobTypeEnum.LIVE_MOUNT.getValue() + JobStatusLabelConstant.HYPHEN_STRING_DELIMITER + version();
        return jobType.equals(object);
    }
}
