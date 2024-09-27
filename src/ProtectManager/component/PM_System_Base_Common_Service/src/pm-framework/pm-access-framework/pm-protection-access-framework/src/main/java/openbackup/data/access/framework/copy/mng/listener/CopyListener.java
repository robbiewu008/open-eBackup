/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.copy.mng.listener;

import openbackup.data.access.framework.core.common.constants.ContextConstants;
import openbackup.data.access.framework.core.common.constants.TopicConstants;
import openbackup.data.access.framework.core.common.enums.DmeJobStatusEnum;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.dto.TaskCompleteMessageDto;
import openbackup.data.protection.access.provider.sdk.copy.CopyBo;
import openbackup.data.protection.access.provider.sdk.copy.CopyProvider;
import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.kafka.annotations.MessageListener;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.job.JobCenterRestApi;
import openbackup.system.base.sdk.job.model.request.JobStopParam;
import openbackup.system.base.sdk.job.model.request.UpdateJobRequest;
import openbackup.system.base.security.exterattack.ExterAttack;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.util.MessageTemplate;

import lombok.extern.slf4j.Slf4j;

import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.beans.BeanUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.kafka.support.Acknowledgment;
import org.springframework.stereotype.Component;

/**
 * Copy msg listener, this lister will consume the copy msg and
 * the specified provider is found by provider registry
 * to complete the restore service logic processing.
 *
 * @author h30003246
 * @version [OceanStor 100P 8.1.0]
 * @since 2020-08-25
 */
@Component
@Slf4j
public class CopyListener {
    /**
     * JOB_STATUS_LOG
     */
    private static final String JOB_STATUS_LOG = "job_status_{payload?.job_status|status}_label";

    @Autowired
    private ProviderManager providerManager;

    @Autowired
    private RedissonClient redissonClient;

    @Autowired
    private CopyRestApi copyRestApi;

    @Autowired
    private JobCenterRestApi jobCenterRestApi;

    @Autowired
    @Qualifier("unifiedCopyProvider")
    private CopyProvider unfiedCopyProvider;

    @Autowired
    private MessageTemplate<?> messageTemplate;

    @Autowired
    private DeployTypeService deployTypeService;

    @Autowired
    private JobService jobService;

    /**
     * Consume restore topic message
     *
     * @param consumerString delete msg
     * @param acknowledgment Acknowledgment
     */
    @ExterAttack
    @MessageListener(topics = TopicConstants.COPY_DELETE_TOPIC, containerFactory = "retryFactory",
        log = {"job_log_copy_delete_exec_label", JOB_STATUS_LOG})
    public void delete(String consumerString, Acknowledgment acknowledgment) {
        JSONObject jsonObject = JSONObject.fromObject(consumerString);
        if (!(jsonObject.get(ContextConstants.REQUEST_ID) instanceof String)) {
            return;
        }

        String requestId = (String) jsonObject.get(ContextConstants.REQUEST_ID);
        if (!jobService.isJobPresent(requestId)) {
            return;
        }
        RMap<String, String> jobMap = redissonClient.getMap(requestId, StringCodec.INSTANCE);
        String copyId = jobMap.get(ContextConstants.COPY_ID);
        // 查询副本的接口
        Copy copy = copyRestApi.queryCopyByID(copyId, false);
        if (copy == null) {
            completeTaskWhenCopyDeleted(requestId, copyId);
            return;
        }
        CopyBo copyBo = new CopyBo();
        BeanUtils.copyProperties(copy, copyBo);
        CopyProvider protectionProvider = providerManager.findProvider(CopyProvider.class, copyBo, null);
        if (protectionProvider == null) {
            protectionProvider = unfiedCopyProvider;
        }
        protectionProvider.deleteCopy(requestId, copyBo);
        // 下发给数据面/爱数的副本删除任务不支持停止
        updateTaskCannotStop(requestId);
    }

    private void completeTaskWhenCopyDeleted(String requestId, String copyId) {
        log.error("copy(copyId: {}) already not exist. (requestId: {})", copyId, requestId);
        TaskCompleteMessageDto completeMessage = new TaskCompleteMessageDto();
        completeMessage.setJobId(requestId);
        completeMessage.setJobRequestId(requestId);
        completeMessage.setJobStatus(DmeJobStatusEnum.SUCCESS.getTypeName());
        completeMessage.setJobProgress(IsmNumberConstant.HUNDRED);
        JSONObject message = JSONObject.fromObject(completeMessage);
        message.put(MessageTemplate.REQUEST_ID, requestId);
        messageTemplate.send(TopicConstants.TASK_COMPLETE_TOPIC, message);
    }

    private void updateTaskCannotStop(String requestId) {
        JobStopParam jobStopParam = new JobStopParam();
        jobStopParam.setEnforceStop(false);
        jobStopParam.setBackupEngineCancelable(false);
        UpdateJobRequest updateJobRequest = new UpdateJobRequest();
        updateJobRequest.setEnableStop(false);
        jobCenterRestApi.updateJob(requestId, updateJobRequest, jobStopParam);
    }
}
