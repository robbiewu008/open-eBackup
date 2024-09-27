/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.livemount.listener;

import openbackup.data.access.framework.livemount.TopicConstants;
import openbackup.data.access.framework.livemount.controller.livemount.model.LiveMountStatus;
import openbackup.data.access.framework.livemount.provider.LiveMountFlowService;
import openbackup.system.base.common.constants.DateFormatConstant;
import openbackup.system.base.common.model.livemount.LiveMountEntity;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.kafka.MessageObject;
import openbackup.system.base.kafka.annotations.MessageContext;
import openbackup.system.base.kafka.annotations.MessageListener;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyStatus;
import openbackup.system.base.sdk.job.model.JobStatusEnum;
import openbackup.system.base.security.exterattack.ExterAttack;
import openbackup.system.base.util.ProviderRegistry;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.kafka.support.Acknowledgment;
import org.springframework.stereotype.Component;

import java.text.SimpleDateFormat;

/**
 * Migrate Flow Listener
 *
 * @author h30003246
 * @since 2021-01-05
 */
@Slf4j
@Component
public class MigrateFlowListener extends AbstractFlowListener {
    private static final String JOB_ID = "job_id";

    private static final String REQUEST_ID = "request_id";

    private static final String LIVE_MOUNT = "live_mount";

    private static final String MIGRATE = "migrate";

    /**
     * 任务执行失败，根据下发迁移任务状态设置已挂载副本状态。
     */
    private static final String HAS_DELIVER_MIGRATE = "has_deliver_migrate";

    private static final String TRUE = "true";

    private static final String JOB_STATUS = "job_status";

    @Autowired
    private ProviderRegistry providerRegistry;

    /**
     * process live mount unmounted
     *
     * @param data data
     * @param acknowledgment acknowledgment
     */
    @ExterAttack
    @MessageListener(topics = TopicConstants.LIVE_MOUNT_MIGRATE_REQUEST,
        lock = "resources:{payload.live_mount.mounted_copy_id,payload.live_mount.mounted_resource_id}",
        messageContexts = {
            @MessageContext(topic = TASK_COMPLETE_MESSAGE,
                messages = TopicConstants.LIVE_MOUNT_MIGRATE_COMPLETE + MessageContext.PAYLOAD,
                chain = MessageContext.STACK)
        },
        containerFactory = "retryFactory",
        failures = TopicConstants.LIVE_MOUNT_MIGRATE_COMPLETE,
        log = {"job_log_live_mount_migrate_execute_label", JOB_STATUS_LOG})
    public void requestLiveMountMigrate(String data, Acknowledgment acknowledgment) {
        JSONObject json = JSONObject.fromObject(data);
        LiveMountEntity liveMountEntity = getLiveMountEntity(json);
        liveMountService.checkTargetEnvironmentStatus(liveMountEntity);
        liveMountService.updateLiveMountStatus(liveMountEntity, LiveMountStatus.MIGRATING);
        updateJobStatus(json, JobStatusEnum.RUNNING);
        String mountedCopyId = liveMountEntity.getMountedCopyId();
        Copy mountedCopy = copyRestApi.queryCopyByID(mountedCopyId);
        updateCopyStatus(mountedCopy, CopyStatus.UNMOUNTING);
        JSONObject param = json.pick(REQUEST_ID, JOB_ID, LIVE_MOUNT, MIGRATE);

        LiveMountFlowService provider = providerRegistry.findProvider(LiveMountFlowService.class,
            liveMountEntity.getResourceSubType(), null);
        if (provider != null) {
            provider.migrateLiveMount(param);
        }

        // 迁移任务成功下发到dme
        setRequestCache(param.getString(REQUEST_ID), HAS_DELIVER_MIGRATE, String.valueOf(true));
    }

    /**
     * delete live mount
     *
     * @param data data
     * @param acknowledgment acknowledgment
     * @return next topic and message
     */
    @ExterAttack
    @MessageListener(topics = TopicConstants.LIVE_MOUNT_MIGRATE_COMPLETE, containerFactory = "retryFactory",
        log = {"job_log_live_mount_migrate_complete_label", JOB_STATUS_LOG}, unlock = true, terminatedMessage = true)
    public MessageObject migrateLiveMount(String data, Acknowledgment acknowledgment) {
        JSONObject json = JSONObject.fromObject(data);
        LiveMountEntity liveMountEntity = getLiveMountEntity(json);
        String jobStatus = json.getString(JOB_STATUS);
        JobStatusEnum status = JobStatusEnum.valueOf(jobStatus);
        liveMountService.updateLiveMountStatus(liveMountEntity, LiveMountStatus.AVAILABLE);
        updateMountedCopyStatus(json, status, liveMountEntity);
        checkJobStatusAndDeleteLiveMount(status, liveMountEntity);
        return getMessageObject(json, jobStatus, status);
    }

    private MessageObject getMessageObject(JSONObject json, String jobStatus, JobStatusEnum status) {
        // 如果迁移成功需要删除副本
        return status.checkSuccess()
            ? new MessageObject(TopicConstants.LIVE_MOUNT_CLEAN_CLONE_COPY, json)
            : new MessageObject(json, jobStatus, null);
    }

    private void updateMountedCopyStatus(JSONObject json, JobStatusEnum statusEnum, LiveMountEntity liveMountEntity) {
        String mountedCopyId = liveMountEntity.getMountedCopyId();
        Long nowTimestamp = System.currentTimeMillis();
        Copy mountedCopy = copyRestApi.queryCopyByID(mountedCopyId);
        String requestId = json.getString(REQUEST_ID);
        if (TRUE.equals(getRequestCache(requestId, HAS_DELIVER_MIGRATE)) && statusEnum.checkSuccess()) {
            updateCopyStatus(mountedCopy, CopyStatus.NORMAL, true, String.valueOf(nowTimestamp),
                new SimpleDateFormat(DateFormatConstant.DATE_TIME_WITH_MILLIS).format(nowTimestamp));
        } else {
            updateCopyStatus(mountedCopy, CopyStatus.MOUNTED);
        }
    }

    private void checkJobStatusAndDeleteLiveMount(JobStatusEnum statusEnum, LiveMountEntity liveMountEntity) {
        if (statusEnum.checkSuccess()) {
            liveMountService.deleteLiveMount(liveMountEntity.getId());
            deleteSchedule(liveMountEntity);
        }
    }
}
