package openbackup.data.access.framework.restore.handler.v1;

import openbackup.data.access.client.sdk.api.base.RestClient;
import openbackup.data.access.framework.core.common.constants.ContextConstants;
import openbackup.data.access.framework.core.common.constants.TopicConstants;
import openbackup.data.access.framework.core.common.enums.DmeJobStatusEnum;
import openbackup.data.access.framework.protection.handler.TaskCompleteHandler;
import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import openbackup.system.base.common.model.PageListResponse;
import openbackup.system.base.common.msg.NotifyManager;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyStatus;
import openbackup.system.base.sdk.copy.model.CopyStatusUpdateParam;
import openbackup.system.base.sdk.job.JobCenterRestApi;
import openbackup.system.base.sdk.job.model.JobBo;
import openbackup.system.base.sdk.job.model.JobStatusEnum;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.job.model.request.UpdateJobRequest;
import openbackup.system.base.sdk.resource.VMWareRestApi;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.security.exterattack.ExterAttack;

import lombok.extern.slf4j.Slf4j;

import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.Arrays;

/**
 * kafka consumer of task complete witch is comes from dme
 *
 * @author y30000858
 * @since 2020-09-21
 */
@Component
@Slf4j
public class RestoreTaskCompleteHandler implements TaskCompleteHandler {
    private static final String RESTORE_TYPE = "RESTORE";

    private static final String NOT_SUPPORT = "0";

    private static final String NOT_MARK = "1";

    @Autowired
    private RedissonClient redissonClient;

    @Autowired
    private CopyRestApi copyRestApi;

    @RestClient
    private JobCenterRestApi jobCenter;

    @Autowired
    private NotifyManager notifyManager;

    @Autowired
    private VMWareRestApi vmWareRestApi;

    /**
     * detect object applicable
     *
     * @param object object
     * @return detect result
     */
    @Override
    public boolean applicable(String object) {
        return Arrays.asList(RESTORE_TYPE, JobTypeEnum.INSTANT_RESTORE.name()).contains(object);
    }

    /**
     * task complete handler
     *
     * @param taskCompleteMessage task complete message
     */
    @ExterAttack
    @Override
    public void onTaskCompleteSuccess(TaskCompleteMessageBo taskCompleteMessage) {
        // find the task type
        log.info("request_id:{}, restore task complete, do restore success", taskCompleteMessage.getJobRequestId());
        cleanRestoreWorkFlow(
                taskCompleteMessage.getJobRequestId(),
                taskCompleteMessage.getJobStatus(),
                taskCompleteMessage.getJobProgress());
    }

    @ExterAttack
    @Override
    public void onTaskCompleteFailed(TaskCompleteMessageBo taskCompleteMessage) {
        // find the task type
        log.warn("request_id:{}, restore task complete, do restore failed", taskCompleteMessage.getJobRequestId());
        cleanRestoreWorkFlow(
                taskCompleteMessage.getJobRequestId(),
                taskCompleteMessage.getJobStatus(),
                taskCompleteMessage.getJobProgress());
    }

    /**
     * clean restore work flow
     *
     * @param requestId requestId
     * @param jobStatus jobStatus
     * @param jobProgress job process
     */
    private void cleanRestoreWorkFlow(String requestId, Integer jobStatus, Integer jobProgress) {
        RMap<String, String> rMap = redissonClient.getMap(requestId, StringCodec.INSTANCE);
        // 单独对vmware类型进行处理 恢复成功的时候才会走如下逻辑
        if (ResourceSubTypeEnum.VMWARE.getType().equals(rMap.get("object_type"))
                && DmeJobStatusEnum.SUCCESS.equals(DmeJobStatusEnum.fromStatus(jobStatus))) {
            vmwareRestoreSuccessProcess(rMap);
        }

        // 单独对vmware类型进行处理 恢复任务失败的时候会更新任务中的MarkStatus字段为未处理
        if (ResourceSubTypeEnum.VMWARE.getType().equals(rMap.get("object_type"))
            && DmeJobStatusEnum.FAIL.equals(DmeJobStatusEnum.fromStatus(jobStatus))) {
            updateVmwareJobMarkStatusNotMark(requestId);
        }
        if (!VerifyUtil.isEmpty(rMap.get("restore_copy_id"))) {
            CopyStatusUpdateParam param = new CopyStatusUpdateParam();
            param.setStatus(CopyStatus.NORMAL);
            log.info("request_id:{}, restore task complete, update copy status", requestId);
            copyRestApi.updateCopyStatus(rMap.get("restore_copy_id"), param);
        }
        UpdateJobRequest updateJobRequest = new UpdateJobRequest();
        // 测试需求去掉100%进度
        JobStatusEnum status = JobStatusEnum.get(DmeJobStatusEnum.fromStatus(jobStatus).name());
        updateJobRequest.setStatus(status);
        if (!VerifyUtil.isEmpty(jobProgress) || jobProgress != 0) {
            updateJobRequest.setProgress(jobProgress);
        }
        log.info("request_id:{}, restore task complete, update job status", requestId);
        jobCenter.updateJob(rMap.get("job_id"), updateJobRequest);
        // destroy work flow
        sendRestoreDoneMessage(requestId, rMap);
    }

    private void sendRestoreDoneMessage(String requestId, RMap<String, String> rMap) {
        JSONObject restoreDoneMap = new JSONObject();
        restoreDoneMap.put(ContextConstants.REQUEST_ID, requestId);
        restoreDoneMap.put("job_id", rMap.get("job_id"));
        log.info("request_id:{}, restore task complete, send restore done message", requestId);
        notifyManager.send(TopicConstants.TASK_RESTORE_DONE_TOPIC, restoreDoneMap.toString());
    }

    private void vmwareRestoreSuccessProcess(RMap<String, String> rMap) {
        boolean shouldDeleteVM = false;
        if (!VerifyUtil.isEmpty(JSONObject.fromObject(rMap.get("ext_parameters")))) {
            JSONObject extParameters = JSONObject.fromObject(rMap.get("ext_parameters"));
            if (!VerifyUtil.isEmpty(extParameters.getString("parentUuid"))) {
                // VMware恢复专属，整机恢复 原位置 新位置均会传该字段
                String parentUuid = extParameters.getString("parentUuid");
                String vmName = extParameters.getString("vm_name");
                String isDeleteOriginalVM = extParameters.getString("isDeleteOriginalVM");
                shouldDeleteVM = Boolean.parseBoolean(isDeleteOriginalVM);
                vmWareRestApi.refreshResources(parentUuid, vmName);
            }
        }
        if (!VerifyUtil.isEmpty(rMap.get("copy_id")) && shouldDeleteVM) {
            Copy copy = copyRestApi.queryCopyByID(rMap.get("copy_id"));
            vmWareRestApi.deleteVMResource(copy.getResourceId());
        }
    }

    private void updateVmwareJobMarkStatusNotMark(String requestId) {
        PageListResponse<JobBo> job = jobCenter.queryJobs(requestId, 0, 1);
        if (job.getTotalCount() != 0) {
            if (NOT_SUPPORT.equals(job.getRecords().get(0).getMarkStatus())) {
                UpdateJobRequest jobRequest = new UpdateJobRequest();
                jobRequest.setMarkStatus(NOT_MARK);
                jobCenter.updateJob(requestId, jobRequest);
            }
        }
    }
}
