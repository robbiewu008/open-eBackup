package openbackup.data.access.framework.protection.listener.v1.archive;

import openbackup.data.access.client.sdk.api.base.RestClient;
import openbackup.data.access.framework.core.common.constants.TopicConstants;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.archive.ArchiveImportObject;
import openbackup.data.protection.access.provider.sdk.archive.ArchiveImportProvider;
import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.job.JobCenterRestApi;
import openbackup.system.base.sdk.job.model.JobLogBo;
import openbackup.system.base.sdk.job.model.JobLogLevelEnum;
import openbackup.system.base.sdk.job.model.request.UpdateJobRequest;
import openbackup.system.base.security.exterattack.ExterAttack;

import com.alibaba.fastjson.JSONObject;

import lombok.extern.slf4j.Slf4j;

import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.kafka.annotation.KafkaListener;
import org.springframework.kafka.core.KafkaTemplate;
import org.springframework.kafka.support.Acknowledgment;
import org.springframework.retry.annotation.Backoff;
import org.springframework.retry.annotation.Recover;
import org.springframework.retry.annotation.Retryable;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.List;
import java.util.Optional;

/**
 * 归档副本导入消息监听器
 *
 * @author z30009433
 * @since 2021-01-08
 */
@Component
@Slf4j
public class ArchiveImportListener {
    private static final int TASK_FAIL_STATUS = 6;

    private static final int TASK_FAIL_PROGRESS = 100;

    @Autowired
    private KafkaTemplate kafkaTemplate;

    @Autowired
    private RedissonClient redissonClient;

    @RestClient
    private JobCenterRestApi jobCenter;

    @Autowired
    private ProviderManager providerManager;

    /**
     * Consume restore topic message
     *
     * @param msg archive import msg
     * @param acknowledgment Acknowledgment
     */
    @ExterAttack
    @Retryable(
            exclude = {LegoCheckedException.class},
            maxAttempts = 5,
            backoff = @Backoff(delay = 2 * 60 * 1000))
    @KafkaListener(groupId = TopicConstants.KAFKA_CONSUMER_GROUP, topics = TopicConstants.IMPORT_ARCHIVE_COPIES_TOPIC,
        autoStartup = "false")
    public void archiveImport(String msg, Acknowledgment acknowledgment) {
        if (VerifyUtil.isEmpty(msg)) {
            log.info("topic = import.archive.copies, msg is empty");
            return;
        }
        JSONObject msgJson = JSONObject.parseObject(msg);

        String requestId = msgJson.get("request_id").toString();
        RMap<String, String> context = redissonClient.getMap(requestId, StringCodec.INSTANCE);
        String repositoryType = context.get("repositoryType");
        ArchiveImportProvider archiveProvider =
                providerManager.findProvider(ArchiveImportProvider.class, repositoryType);
        archiveProvider.scanArchive(createArchiveObject(requestId, context));
        acknowledgment.acknowledge();
    }

    /**
     * recover scan response topic message
     *
     * @param consumerString san response delete msg
     * @param acknowledgment Acknowledgment
     * @param exception Exception
     */
    @ExterAttack
    @Recover
    public void recover(Exception exception, String consumerString, Acknowledgment acknowledgment) {
        acknowledgment.acknowledge();
        log.error("import.archive.copies kafka Exception:", exception);
        TaskCompleteMessageBo taskComplete = new TaskCompleteMessageBo();
        openbackup.system.base.common.utils.JSONObject properties =
                openbackup.system.base.common.utils.JSONObject.fromObject(consumerString);
        String jobRequestId =
                Optional.ofNullable(properties.getString("job_request_id")).orElse(properties.getString("request_id"));
        RMap<String, String> rMap = redissonClient.getMap(jobRequestId, StringCodec.INSTANCE);
        taskComplete.setJobRequestId(jobRequestId);
        taskComplete.setJobStatus(TASK_FAIL_STATUS);
        taskComplete.setJobProgress(TASK_FAIL_PROGRESS);
        taskComplete.setJobId("");
        String jobId = rMap.get("job_id");
        JobLogBo jobLogBo = new JobLogBo();
        jobLogBo.setStartTime(System.currentTimeMillis());
        jobLogBo.setJobId(jobId);
        jobLogBo.setLevel(JobLogLevelEnum.ERROR.getValue());
        jobLogBo.setLogInfo("task_running_failed_label");
        jobLogBo.setLogDetail(String.valueOf(CommonErrorCode.SYSTEM_ERROR));
        List<JobLogBo> jobLogBoList = new ArrayList<>();
        jobLogBoList.add(jobLogBo);
        UpdateJobRequest updateJobRequest = new UpdateJobRequest();
        updateJobRequest.setJobLogs(jobLogBoList);
        jobCenter.updateJob(jobId, updateJobRequest);
        String mes = openbackup.system.base.common.utils.JSONObject.fromObject(taskComplete).toString();
        kafkaTemplate.send(TopicConstants.JOB_COMPLETE_TOPIC, mes);
    }

    private ArchiveImportObject createArchiveObject(String requireId, RMap<String, String> context) {
        // 从redis获取storageId，jobId
        String storageId = context.get("storageId");
        String jobId = context.get("job_id");
        String repositoryType = context.get("repositoryType");
        return ArchiveImportObject.builder().requestId(requireId).jobId(jobId).storageId(storageId)
                .type(Integer.parseInt(repositoryType)).build();
    }
}
