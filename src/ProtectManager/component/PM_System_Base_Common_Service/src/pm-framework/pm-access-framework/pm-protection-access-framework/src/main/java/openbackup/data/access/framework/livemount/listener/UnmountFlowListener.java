package openbackup.data.access.framework.livemount.listener;

import openbackup.data.access.framework.livemount.TopicConstants;
import openbackup.data.access.framework.livemount.controller.livemount.model.LiveMountStatus;
import openbackup.data.access.framework.livemount.entity.LiveMountPolicyEntity;
import openbackup.system.base.common.constants.DateFormatConstant;
import openbackup.system.base.common.model.livemount.LiveMountEntity;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.kafka.MessageObject;
import openbackup.system.base.kafka.annotations.MessageContext;
import openbackup.system.base.kafka.annotations.MessageListener;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyStatus;
import openbackup.system.base.sdk.job.model.JobLogBo;
import openbackup.system.base.sdk.job.model.JobLogLevelEnum;
import openbackup.system.base.sdk.job.model.JobStatusEnum;
import openbackup.system.base.security.exterattack.ExterAttack;

import lombok.extern.slf4j.Slf4j;

import org.springframework.kafka.support.Acknowledgment;
import org.springframework.stereotype.Component;

import java.text.SimpleDateFormat;
import java.util.Collections;

/**
 * Unmount Flow Listener
 *
 * @author l00272247
 * @since 2020-11-05
 */
@Slf4j
@Component
public class UnmountFlowListener extends AbstractFlowListener {
    private static final String FORCE_DELETE = "force_delete";

    private static final String FORCE_UNMOUNT_LABEL = "job_log_live_mount_force_unmount_label";

    /**
     * process live mount unmounted
     *
     * @param data data
     * @param acknowledgment acknowledgment
     */
    @ExterAttack
    @MessageListener(topics = TopicConstants.LIVE_MOUNT_UNMOUNT_REQUEST, containerFactory = "retryFactory",
        lock = "resources:{payload.live_mount.mounted_copy_id,payload.live_mount.mounted_resource_id}",
        failures = TopicConstants.LIVE_MOUNT_UNMOUNT_COMPLETE,
        log = {"job_log_live_mount_unmount_execute_label", JOB_STATUS_LOG})
    public void requestLiveMountUnmount(String data, Acknowledgment acknowledgment) {
        JSONObject json = JSONObject.fromObject(data);
        LiveMountEntity liveMountEntity = getLiveMountEntity(json);
        liveMountService.checkTargetEnvironmentStatus(liveMountEntity);
        liveMountService.updateLiveMountStatus(liveMountEntity, LiveMountStatus.UNMOUNTING);
        String mountedCopyId = liveMountEntity.getMountedCopyId();
        if (copyRestApi.queryCopyByID(mountedCopyId, false) != null) {
            messageTemplate.send(TopicConstants.LIVE_MOUNT_UNMOUNT_ACTION, data);
        } else {
            updateJobStatus(json, JobStatusEnum.RUNNING);
            messageTemplate.send(TopicConstants.LIVE_MOUNT_UNMOUNT_COMPLETE,
                json.set(JOB_STATUS, JobStatusEnum.SUCCESS.name()));
        }
    }

    /**
     * do Live Mount Unmount
     *
     * @param data data
     * @param acknowledgment acknowledgment
     */
    @ExterAttack
    @MessageListener(topics = TopicConstants.LIVE_MOUNT_UNMOUNT_ACTION, messageContexts = {
        @MessageContext(topic = TASK_COMPLETE_MESSAGE,
            messages = TopicConstants.LIVE_MOUNT_UNMOUNT_COMPLETE + MessageContext.PAYLOAD,
            chain = MessageContext.STACK)
    }, containerFactory = "retryFactory", failures = TopicConstants.LIVE_MOUNT_UNMOUNT_COMPLETE)
    public void doLiveMountUnmount(String data, Acknowledgment acknowledgment) {
        JSONObject json = JSONObject.fromObject(data);
        updateJobStatus(json, JobStatusEnum.RUNNING);
        LiveMountEntity liveMountEntity = getLiveMountEntity(json);
        String mountedCopyId = liveMountEntity.getMountedCopyId();
        Copy mountedCopy = copyRestApi.queryCopyByID(mountedCopyId);
        updateCopyStatus(mountedCopy, CopyStatus.UNMOUNTING);
        JSONObject param = json.pick(REQUEST_ID, JOB_ID, LIVE_MOUNT, RESERVE_COPY, FORCE_DELETE);
        param.set(RESERVE_APP, false);
        requestLiveMountUnmount(param, mountedCopy);
    }

    /**
     * delete live mount
     *
     * @param data data
     * @param acknowledgment acknowledgment
     * @return next topic and message
     */
    @ExterAttack
    @MessageListener(topics = TopicConstants.LIVE_MOUNT_UNMOUNT_COMPLETE, containerFactory = "retryFactory",
        log = {"job_log_live_mount_unmount_complete_label", JOB_STATUS_LOG}, unlock = true, terminatedMessage = true)
    public MessageObject deleteLiveMount(String data, Acknowledgment acknowledgment) {
        JSONObject json = JSONObject.fromObject(data);
        LiveMountEntity liveMountEntity = getLiveMountEntity(json);
        String mountedCopyId = liveMountEntity.getMountedCopyId();
        Long nowTimestamp = System.currentTimeMillis();
        boolean isDeleteCloneCopy;
        String jobStatus = json.getString(JOB_STATUS);
        String completeStatus;
        String requestId = json.getString(REQUEST_ID);
        // 判断是否加锁失败，失败后不继续执行下面流程。
        if (isLockFail(requestId)) {
            log.warn("Live mount unmount lock resource failed, requestId:{} .", requestId);
            return null;
        }
        try {
            liveMountService.updateLiveMountStatus(liveMountEntity, LiveMountStatus.INVALID);
            isDeleteCloneCopy = isNeedDeleteCloneCopy(json, liveMountEntity, mountedCopyId, nowTimestamp);
            deleteSchedule(liveMountEntity);
            String jobId = json.getString(JOB_ID);
            refreshTargetResource(jobId, liveMountEntity, 0, true);
        } catch (RuntimeException e) {
            jobStatus = JobStatusEnum.FAIL.name();
            throw e;
        } finally {
            completeStatus = checkJobStatusAndDeleteLiveMount(json, liveMountEntity.getId(), jobStatus);
        }
        return isDeleteCloneCopy ? new MessageObject(TopicConstants.LIVE_MOUNT_CLEAN_CLONE_COPY, json, jobStatus,
            completeStatus) : new MessageObject(json, jobStatus, completeStatus);
    }

    private boolean isNeedDeleteCloneCopy(JSONObject json, LiveMountEntity liveMountEntity, String mountedCopyId,
        Long nowTimestamp) {
        boolean isDeleteCloneCopy;
        if (copyRestApi.queryCopyByID(mountedCopyId, false) != null) {
            boolean isReserveCopy = json.getBoolean(RESERVE_COPY);
            Copy mountedCopy = copyRestApi.queryCopyByID(mountedCopyId);
            LiveMountPolicyEntity policy = getLiveMountPolicy(liveMountEntity);
            updateCopyStatus(mountedCopy, CopyStatus.NORMAL, true, String.valueOf(nowTimestamp),
                new SimpleDateFormat(DateFormatConstant.DATE_TIME_WITH_MILLIS).format(nowTimestamp));
            if (!isReserveCopy) {
                isDeleteCloneCopy = true;
                log.info("clone copy(uuid: {}) is need to delete", mountedCopyId);
            } else {
                String requestId = json.getString(REQUEST_ID);
                boolean isClean = cleanOldMountedCopy(liveMountEntity, mountedCopy, policy, requestId);
                isDeleteCloneCopy = !isClean;
            }
        } else {
            isDeleteCloneCopy = false;
        }
        return isDeleteCloneCopy;
    }

    /**
     * delete clone copy
     *
     * @param data data
     * @param acknowledgment acknowledgment
     */
    @ExterAttack
    @MessageListener(topics = TopicConstants.LIVE_MOUNT_CLEAN_CLONE_COPY, containerFactory = "retryFactory",
        log = {"job_log_live_mount_delete_clone_copy_label", STEP_LEVEL_STATUS_LOG}, retryable = true)
    public void deleteCloneCopy(String data, Acknowledgment acknowledgment) {
        JSONObject json = JSONObject.fromObject(data);
        String requestId = json.getString(REQUEST_ID);
        log.info("Live mount receive live-mount.clone-copy.clean msg, requestId is {}.", requestId);
        LiveMountEntity liveMountEntity = getLiveMountEntity(json);
        Copy cloneCopy = getCloneCopy(json);
        String deleteCopyId = VerifyUtil.isEmpty(cloneCopy) ? liveMountEntity.getMountedCopyId() : cloneCopy.getUuid();
        log.info("Live mount delete clone copy, requestId: {}, deleteCopyId: {}, mountedCopyId: {}.",
                requestId, deleteCopyId, liveMountEntity.getMountedCopyId());
        deleteCloneCopy(deleteCopyId, requestId);
    }

    private String checkJobStatusAndDeleteLiveMount(JSONObject json, String liveMountId, String jobStatus) {
        boolean isForceDelete = json.getBoolean(FORCE_DELETE);
        JobStatusEnum status = JobStatusEnum.valueOf(jobStatus);
        String completeStatus = null;
        if (status == JobStatusEnum.SUCCESS || isForceDelete) {
            liveMountService.deleteLiveMount(liveMountId);
        }

        // 如果卸载失败，但是执行的是强制停止，需要打印强制停止的日志, 并把任务状态置为成功
        if (isForceDelete && status != JobStatusEnum.SUCCESS) {
            updateForceUnmountJobLog(json);
            completeStatus = JobStatusEnum.SUCCESS.name();
        }
        return completeStatus == null ? jobStatus : completeStatus;
    }

    private void updateForceUnmountJobLog(JSONObject json) {
        String jobId = json.getString(JOB_ID);
        JobLogBo jobLogBo = new JobLogBo();
        jobLogBo.setStartTime(System.currentTimeMillis());
        jobLogBo.setLevel(JobLogLevelEnum.WARNING.getValue());
        jobLogBo.setLogInfo(FORCE_UNMOUNT_LABEL);
        jobLogBo.setJobId(jobId);
        updateJobLog(jobId, Collections.singletonList(jobLogBo));
    }
}
