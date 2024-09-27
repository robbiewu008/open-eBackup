package openbackup.data.access.framework.protection.handler.v1.replication;

import openbackup.data.access.framework.core.common.constants.TopicConstants;
import openbackup.data.access.framework.core.common.enums.DmeJobStatusEnum;
import openbackup.data.access.framework.protection.handler.TaskCompleteHandler;
import openbackup.data.access.framework.protection.handler.v2.UnifiedTaskCompleteHandler;
import openbackup.data.access.framework.protection.service.replication.UnifiedReplicationCopyProcessor;
import openbackup.data.protection.access.provider.sdk.backup.ProtectedObject;
import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import openbackup.system.base.common.constants.RedisConstants;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.JobSpeedConverter;
import openbackup.system.base.sdk.cluster.model.TargetClusterVo;
import openbackup.system.base.sdk.job.JobCenterRestApi;
import openbackup.system.base.sdk.job.model.JobLogBo;
import openbackup.system.base.sdk.job.model.JobStatusEnum;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.job.model.request.UpdateJobRequest;
import openbackup.system.base.security.exterattack.ExterAttack;
import openbackup.system.base.util.MessageTemplate;
import openbackup.system.base.util.ProviderRegistry;

import lombok.extern.slf4j.Slf4j;

import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.concurrent.atomic.AtomicStampedReference;

/**
 * Replication Task Complete Handler
 *
 * @author l00272247
 * @since 2020-12-18
 */
@Component
@Slf4j
public class ReplicationTaskCompleteHandler implements TaskCompleteHandler {
    @Autowired
    private RedissonClient redissonClient;

    @Autowired
    private ProviderRegistry registry;

    @Autowired
    private MessageTemplate<?> messageTemplate;

    @Autowired
    private JobCenterRestApi jobCenterRestApi;

    @Autowired
    private UnifiedReplicationCopyProcessor unifiedReplicationCopyProcessor;

    /**
     * task success handler
     *
     * @param taskCompleteMessage task complete message
     */
    @ExterAttack
    @Override
    public void onTaskCompleteSuccess(TaskCompleteMessageBo taskCompleteMessage) {
        String requestId = taskCompleteMessage.getJobRequestId();
        RMap<String, String> context = redissonClient.getMap(requestId, StringCodec.INSTANCE);
        cleanTargetClusterRelatedTaskInfo(context);
        String jsonStr = context.get("protected_object");
        ProtectedObject protectedObject = JSONObject.fromObject(jsonStr).toBean(ProtectedObject.class);
        ReplicationCopyProcessor processor =
                registry.findProviderOrDefault(
                        ReplicationCopyProcessor.class, protectedObject.getSubType(), unifiedReplicationCopyProcessor);
        boolean isComplete = true;
        if (processor != null) {
            try {
                AtomicStampedReference<Boolean> stampedReference = processor.process(taskCompleteMessage);
                int count = stampedReference.getStamp();
                isComplete = stampedReference.getReference();
                recordeReplicatedCopyNumber(taskCompleteMessage, count);
            } catch (LegoCheckedException e) {
                log.error("process replication copy failed. request id: {}", requestId, e);
                taskCompleteMessage.setJobStatus(DmeJobStatusEnum.FAIL.getTypeName());
            }
        }
        if (isComplete) {
            JSONObject message = JSONObject.fromObject(taskCompleteMessage).set("request_id", requestId);
            message.remove("job_status");
            messageTemplate.send(TopicConstants.REPLICATION_COMPLETE, message);
        }
    }

    /**
     * task fail handler
     *
     * @param taskCompleteMessage task complete message
     */
    @Override
    public void onTaskCompleteFailed(TaskCompleteMessageBo taskCompleteMessage) {
        // 失败的逻辑已做处理，本次仅做接口适配
        onTaskCompleteSuccess(taskCompleteMessage);
    }

    private void recordeReplicatedCopyNumber(TaskCompleteMessageBo taskCompleteMessage, int count) {
        if (count < 0) {
            return;
        }
        String requestId = taskCompleteMessage.getJobRequestId();
        RMap<String, String> map = redissonClient.getMap(requestId, StringCodec.INSTANCE);
        String jobId = map.get("job_id");
        JobLogBo jobLogBo = new JobLogBo();
        jobLogBo.setJobId(jobId);
        jobLogBo.setStartTime(System.currentTimeMillis());
        jobLogBo.setLogInfo("job_log_copy_replication_replicated_label");
        jobLogBo.setLogInfoParam(Collections.singletonList(String.valueOf(count)));
        UpdateJobRequest request = new UpdateJobRequest();
        String jobStatus = map.get("job_status");
        log.info("update replication job status: {}", jobStatus);
        JobStatusEnum status = JobStatusEnum.get(jobStatus);
        request.setStatus(status);
        request.setJobLogs(Collections.singletonList(jobLogBo));
        if (taskCompleteMessage.getExtendsInfo() != null) {
            request.setExtendStr(JSONObject.writeValueAsString(taskCompleteMessage.getExtendsInfo()));
        }
        Long speed = taskCompleteMessage.getSpeed();
        if (speed != null) {
            String jobSpeed = JobSpeedConverter.convertJobSpeed(String.valueOf(speed));
            request.setSpeed(jobSpeed);
        }
        log.info("Get replication speed, speed: {}, job_id: {}", request.getSpeed(), jobId);
        jobCenterRestApi.updateJob(jobId, request);
    }

    /**
     * clean Target Cluster Related Task Info
     *
     * @param context context
     */
    public void cleanTargetClusterRelatedTaskInfo(RMap<String, String> context) {
        TargetClusterVo targetCluster =
                JSONObject.fromObject(context.get("target_cluster")).toBean(TargetClusterVo.class);
        RMap<String, String> map =
                redissonClient.getMap(
                        RedisConstants.TARGET_CLUSTER_RELATED_TASKS + targetCluster.getClusterId(),
                        StringCodec.INSTANCE);
        String jobId = context.get("job_id");
        map.remove(jobId);
    }

    /**
     * detect object applicable
     *
     * @param object object
     * @return detect result
     */
    @Override
    public boolean applicable(String object) {
        return JobTypeEnum.COPY_REPLICATION.getValue().equals(object)
            || (JobTypeEnum.COPY_REPLICATION.getValue() + "-" + UnifiedTaskCompleteHandler.V2).equals(object);
    }
}
