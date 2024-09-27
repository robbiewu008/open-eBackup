package openbackup.data.access.framework.copy.verify.service;

import openbackup.data.access.client.sdk.api.framework.dme.CopyVerifyStatusEnum;
import openbackup.data.access.framework.copy.mng.constant.CopyPropertiesKeyConstant;
import openbackup.data.access.framework.copy.verify.constant.CopyVerifyTaskErrorCode;
import openbackup.data.access.framework.core.common.constants.TopicConstants;
import openbackup.data.protection.access.provider.sdk.verify.CopyVerifyTask;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.asserts.PowerAssert;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.copy.model.CopyStatus;
import openbackup.system.base.sdk.copy.model.UpdateCopyPropertiesRequest;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.job.model.request.CreateJobRequest;
import openbackup.system.base.sdk.job.model.request.JobMessage;
import openbackup.system.base.sdk.lock.LockRequest;
import openbackup.system.base.sdk.lock.LockResource;
import openbackup.system.base.sdk.lock.LockTypeEnum;

import com.google.common.collect.Lists;

import lombok.extern.slf4j.Slf4j;

import org.springframework.util.Assert;

import java.util.Locale;
import java.util.function.Supplier;

/**
 * 副本校验辅助工具类，提供静态方法.
 *
 * @author y00559272
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/7/29
 **/
@Slf4j
public abstract class CopyVerifyHelper {
    private static final int DEFAULT_COPY_VERIFY_PRIORITY = 10;

    private CopyVerifyHelper() {
    }

    /**
     * 根据副本校验上下文构建创建任务请求
     *
     * @param context 副本校验任务上下文
     * @return 任务创建请求对象
     */
    public static CreateJobRequest buildCreateJobReq(CopyVerifyManagerContext context) {
        CopyVerifyTask task = context.getTask();
        CreateJobRequest jobRequest = new CreateJobRequest();
        jobRequest.setJobId(task.getRequestId());
        jobRequest.setRequestId(task.getRequestId());
        jobRequest.setCopyId(task.getCopyId());
        jobRequest.setIsSystem(Boolean.FALSE);
        jobRequest.setRequestId(context.getRequestId());
        jobRequest.setType(JobTypeEnum.COPY_VERIFY.getValue());
        final Copy copy = context.getCopy();
        jobRequest.setSourceId(copy.getResourceId());
        jobRequest.setSourceName(copy.getResourceName());
        jobRequest.setSourceLocation(copy.getLocation());
        jobRequest.setSourceType(copy.getResourceType());
        jobRequest.setSourceSubType(copy.getResourceSubType());
        jobRequest.setSourceLocation(copy.getResourceLocation());
        jobRequest.setCopyTime(Long.valueOf(copy.getTimestamp()));
        jobRequest.setTargetLocation(copy.getResourceLocation());
        jobRequest.setEnableStop(Boolean.TRUE);
        jobRequest.setMessage(buildCopyVerifyStartMessage(task));
        jobRequest.setUserId(copy.getUserId());

        return jobRequest;
    }

    private static JobMessage buildCopyVerifyStartMessage(CopyVerifyTask task) {
        JobMessage message = new JobMessage();
        message.setInContext(false);
        message.setPayload(JSONObject.fromObject(task));
        message.setTopic(TopicConstants.COPY_VERIFY_EXECUTE);
        return message;
    }

    static CopyVerifyTask parseFromJobMessage(String message) {
        Assert.hasLength(message, "Copy verify job message is empty");
        final JobMessage jobMessage = JSONObject.toBean(message, JobMessage.class);
        final JSONObject payload = jobMessage.getPayload();
        Assert.notEmpty(payload, "Copy verify job payload is empty");
        return payload.toBean(CopyVerifyTask.class);
    }

    static UpdateCopyPropertiesRequest buildUpdateRequest(String key, String value) {
        UpdateCopyPropertiesRequest req = new UpdateCopyPropertiesRequest();
        req.setKey(key);
        req.setValue(value);
        return req;
    }

    static LockRequest buildLockRequest(String requestId, String taskId, String copyId) {
        final LockRequest lockRequest = new LockRequest();
        lockRequest.setLockId(taskId);
        lockRequest.setRequestId(requestId);
        lockRequest.setPriority(DEFAULT_COPY_VERIFY_PRIORITY);
        lockRequest.setResources(Lists.newArrayList(new LockResource(copyId, LockTypeEnum.READ)));
        return lockRequest;
    }

    /**
     * 校验副本是否有副本校验文件
     *
     * @param copy 副本信息
     */
    static void copyHasVerificationFile(Copy copy) {
        String properties = copy.getProperties();
        JSONObject copyProperties = JSONObject.fromObject(properties);
        Supplier<RuntimeException> copyFileNotExistedException = () -> new LegoCheckedException(
            CopyVerifyTaskErrorCode.COPY_VERIFY_FILE_NOT_EXISTED, "No verification file is generated for the copy.");
        PowerAssert.state(copyProperties.containsKey(CopyPropertiesKeyConstant.KEY_VERIFY_STATUS),
            copyFileNotExistedException);
        String value = copyProperties.getString(CopyPropertiesKeyConstant.KEY_VERIFY_STATUS);
        log.info("Copy Verify check copy verify status {}", value);
        CopyVerifyStatusEnum verifyStatus = CopyVerifyStatusEnum.getByStatus(value);
        PowerAssert.state(verifyStatus != CopyVerifyStatusEnum.VERIFY_FILE_NOT_EXIST, copyFileNotExistedException);
    }

    /**
     * 校验副本状态
     *
     * @param copy 副本状态
     */
    static void checkCopyStatus(Copy copy) {
        final String errorMessage = String.format(Locale.ENGLISH, "Copy[id=%s] status is [%s], can not verify",
            copy.getUuid(), copy.getStatus());
        PowerAssert.state(CopyStatus.NORMAL.equals(CopyStatus.get(copy.getStatus())),
            () -> new LegoCheckedException(CopyVerifyTaskErrorCode.COPY_STATUS_IS_NOT_INVALID, errorMessage));
    }

    /**
     * 校验副本生成方式不是归档
     *
     * @param copy 生成方式
     */
    static void copyIsNotGeneratedByArchive(Copy copy) {
        final String errorMessage = String.format(Locale.ENGLISH, "Copy[id=%s] is generate by [%s], can not verify",
            copy.getUuid(), copy.getGeneratedBy());
        CopyGeneratedByEnum generateType = CopyGeneratedByEnum.getByGenerateType(copy.getGeneratedBy());
        boolean isGenerateByArchive = generateType != CopyGeneratedByEnum.BY_CLOUD_ARCHIVE
            && generateType != CopyGeneratedByEnum.BY_TAPE_ARCHIVE;
        PowerAssert.state(isGenerateByArchive,
            () -> new LegoCheckedException(CopyVerifyTaskErrorCode.COPY_IS_GENERATE_BY_ARCHIVE,
                    errorMessage));
    }

    /**
     * 校验副本生成方式不是复制
     *
     * @param copy 生成方式
     */
    public static void copyIsNotGeneratedByReplication(Copy copy) {
        final String errorMessage = String.format(Locale.ENGLISH, "Copy[id=%s] is generate by [%s], can not verify",
                copy.getUuid(), copy.getGeneratedBy());
        CopyGeneratedByEnum generateType = CopyGeneratedByEnum.getByGenerateType(copy.getGeneratedBy());
        boolean isGenerateByReplication = generateType != CopyGeneratedByEnum.BY_REPLICATED
                && generateType != CopyGeneratedByEnum.BY_CASCADED_REPLICATION
                && generateType != CopyGeneratedByEnum.BY_REVERSE_REPLICATION;
        PowerAssert.state(isGenerateByReplication,
                () -> new LegoCheckedException(CopyVerifyTaskErrorCode.COPY_IS_GENERATE_BY_REPLICATION,
                        errorMessage));
    }
}
