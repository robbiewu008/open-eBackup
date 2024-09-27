/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
package openbackup.data.access.framework.backup.handler.v1;

import openbackup.data.access.framework.backup.service.impl.JobBackupPostProcessService;
import openbackup.data.access.framework.core.common.constants.ContextConstants;
import openbackup.data.access.framework.core.common.enums.DmcJobStatus;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.handler.TaskCompleteHandler;
import openbackup.data.access.framework.protection.service.quota.UserQuotaManager;
import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.data.protection.access.provider.sdk.backup.ProtectedObject;
import openbackup.data.protection.access.provider.sdk.backup.v1.BackupFollowUpProvider;
import openbackup.data.protection.access.provider.sdk.copy.CopyBo;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.copy.CopyProvider;
import openbackup.data.protection.access.provider.sdk.enums.BackupTypeEnum;
import openbackup.data.protection.access.provider.sdk.job.ExtendsInfo;
import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import com.huawei.oceanprotect.job.constants.JobExtendInfoKeys;
import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import com.huawei.oceanprotect.system.base.label.service.LabelService;
import openbackup.system.base.sdk.common.model.UuidObject;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.AddCopyArchiveMapRequest;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.copy.model.CopyInfo;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.job.model.request.UpdateJobRequest;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.security.exterattack.ExterAttack;
import com.huawei.oceanprotect.system.base.user.service.ResourceSetApi;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.lang3.StringUtils;
import org.apache.logging.log4j.util.Strings;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.beans.BeanUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;

/**
 * Backup Task Complete Listener
 *
 * @author y00559272
 * @since 2020/9/19
 **/
@Slf4j
@Component
public class BackupTaskCompleteHandler implements TaskCompleteHandler {
    private static final int THOUSAND = 1000;

    private static final long QUERY_BACKUP_COPY_INFO_FAIL = 1677933569L;

    @Autowired
    private RedissonClient redissonClient;

    @Autowired
    private ProviderManager providerManager;

    @Autowired
    private CopyRestApi copyRestApi;

    @Autowired
    private JobService jobService;

    @Autowired
    private DefaultBackupFollowUpProvider defaultBackupFollowUpProvider;

    @Autowired
    private UserQuotaManager userQuotaManager;

    @Autowired
    private JobBackupPostProcessService jobBackupPostProcessService;

    @Autowired
    private ResourceSetApi resourceSetApi;

    @Autowired
    private LabelService labelService;

    /**
     * detect object applicable
     *
     * @param object object
     * @return detect result
     */
    @Override
    public boolean applicable(String object) {
        return JobTypeEnum.BACKUP.getValue().equals(object);
    }

    /**
     * task complete handler
     *
     * @param taskCompleteMessage task complete message
     */
    @ExterAttack
    @Override
    public void onTaskCompleteSuccess(TaskCompleteMessageBo taskCompleteMessage) {
        String requestId = taskCompleteMessage.getJobRequestId();
        log.info("request_id:{}, backup task complete, do backup success", requestId);
        RMap<String, String> context = redissonClient.getMap(requestId, StringCodec.INSTANCE);
        String jobId = context.get(ContextConstants.JOB_ID);
        int status = taskCompleteMessage.getJobStatus();
        DmcJobStatus jobStatus = DmcJobStatus.getByStatus(status);

        String backupType = context.get("backup_type");
        String jsonStr = context.get("protected_object");
        ProtectedObject protectedObject = JSONObject.fromObject(jsonStr).toBean(ProtectedObject.class);
        String resourceType = protectedObject.getSubType();

        BackupFollowUpProvider followUpProvider =
                providerManager.findProviderOrDefault(
                        BackupFollowUpProvider.class, resourceType, defaultBackupFollowUpProvider);

        // SQLServer事务日志备份需要产生副本（标准备份）
        if (ResourceSubTypeEnum.ORACLE.equalsSubType(resourceType)
                && BackupTypeEnum.LOG.equals(BackupTypeEnum.valueOf(backupType.toUpperCase(Locale.ENGLISH)))) {
            // Oracle日志备份不产生副本，任务成功之后直接结束
            log.info("request_id:{}, backup task complete, backup log", requestId);
            UpdateJobRequest updateJobRequest = new UpdateJobRequest();
            updateJobRequest.setExtendStr(
                    new JSONObject().set("backupType", backupType.toLowerCase(Locale.ENGLISH))
                        .set("sourceCopyType", backupType.toLowerCase(Locale.ENGLISH)).toString());
            jobService.updateJob(jobId, updateJobRequest);
            followUpProvider.handleSuccess(requestId, jobId, jobStatus.getProtectionStatus(), null);
            return;
        }
        // 构造副本信息并入库,入库成功后发送
        this.buildAndSaveCopy(requestId, jobId, resourceType, taskCompleteMessage, followUpProvider);
        jobBackupPostProcessService.onBackupJobSuccess(jobId);
    }

    @ExterAttack
    @Override
    public void onTaskCompleteFailed(TaskCompleteMessageBo taskCompleteMessage) {
        String requestId = taskCompleteMessage.getJobRequestId();
        RMap<String, String> context = redissonClient.getMap(requestId, StringCodec.INSTANCE);
        String jobType = context.get(ContextConstants.JOB_TYPE);
        if (VerifyUtil.isEmpty(jobType) || !Objects.equals(JobTypeEnum.get(jobType), JobTypeEnum.BACKUP)) {
            log.info("backup complete, requestId is {}, jobType is not [BACKUP]", requestId);
            return;
        }
        String jobId = context.get(ContextConstants.JOB_ID);
        int status = taskCompleteMessage.getJobStatus();
        // 发送任务终止消息到对应的workflow
        log.warn(
                "Receive task complete message, requestId is {}, jobId is {}, task status is {}, abort",
                requestId,
                jobId,
                status);

        String jsonStr = context.get("protected_object");
        ProtectedObject protectedObject = JSONObject.fromObject(jsonStr).toBean(ProtectedObject.class);
        String resourceType = protectedObject.getSubType();

        BackupFollowUpProvider followUpProvider =
                providerManager.findProviderOrDefault(
                        BackupFollowUpProvider.class, resourceType, defaultBackupFollowUpProvider);
        DmcJobStatus jobStatus = DmcJobStatus.getByStatus(status);
        followUpProvider.handleFailure(requestId, jobId, jobStatus.getProtectionStatus());
        jobBackupPostProcessService.onBackupJobFail(jobId);
    }

    /**
     * 构造副本信息并入库，保存成功时更新任务进度并发送消备份完成消息
     *
     * @param requestId 请求id
     * @param jobId 本次备份任务id
     * @param resourceType 本次任务的资源类型
     * @param taskResponse 接收到的响应消息
     * @param followUpProvider 后续处理者
     */
    private void buildAndSaveCopy(
            String requestId,
            String jobId,
            String resourceType,
            TaskCompleteMessageBo taskResponse,
            BackupFollowUpProvider followUpProvider) {
        log.info("request_id:{}, backup task complete, build copy", requestId);
        // 处理备份完成逻辑
        CopyBo tmpCopy = new CopyBo();
        tmpCopy.setResourceSubType(resourceType);
        tmpCopy.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        CopyProvider provider = providerManager.findProvider(CopyProvider.class, tmpCopy);
        ExtendsInfo extendsInfo = new ExtendsInfo();
        Map extInfo = taskResponse.getExtendsInfo();
        if (extInfo != null) {
            extendsInfo = JSONObject.toBean(JSONObject.fromObject(extInfo), ExtendsInfo.class);
        }
        List<CopyInfoBo> copyInfoList =
                provider.buildCopy(
                        requestId,
                        Optional.ofNullable(extendsInfo.getBackupId()).orElse(StringUtils.EMPTY),
                        Optional.ofNullable(extendsInfo.getInstanceId()).orElse(StringUtils.EMPTY),
                        resourceType);
        // 查不到副本信息，备份任务直接失败。不卡住，阻塞后面的任务
        if (CollectionUtils.isEmpty(copyInfoList)) {
            log.error("request_id:{} query copy info list fail copyInfoList:{}", requestId, copyInfoList);
            throw new LegoCheckedException(QUERY_BACKUP_COPY_INFO_FAIL, "query backup copy info list fail.");
        }

        // 根据时间戳，升序排序，老的副本先入库
        Collections.sort(copyInfoList, Comparator.comparingLong(copy -> Long.parseLong(copy.getTimestamp())));
        // 调用rest接口副本入库,如果已该副本id存在则跳过
        log.info("query ABBackup Copies, CopyInfoList size is :{}", copyInfoList.size());
        UuidObject resp = new UuidObject();
        String storageUnitId = jobService.queryJob(jobId).getStorageUnitId();
        for (CopyInfoBo copy : copyInfoList) {
            log.info(
                    "copy id is {}, copy time is {}, display time is {}",
                    copy.getUuid(),
                    copy.getTimestamp(),
                    copy.getDisplayTimestamp());
            copy.setProperties(addDataSizeToProperties(copy.getProperties(), extInfo));
            CopyInfo copyInfo = new CopyInfo();
            BeanUtils.copyProperties(copy, copyInfo);
            copyInfo.setStorageUnitId(storageUnitId);
            resp = copyRestApi.saveCopy(copyInfo);
            resourceSetApi.createCopyResourceSetRelation(copyInfo.getUuid(), copyInfo.getResourceId(), Strings.EMPTY);
            // 设置副本继承资源的标签
            labelService.setCopyLabel(copy.getUuid(), copy.getResourceId());
            log.info("copy save success uuid:{}, update expire time:{}", copy.getUuid(), copy.getExpirationTime());
            addCopyArchiveMap(requestId, copy);
            // 备份成功，增加用户已使用额度
            userQuotaManager.increaseUsedQuota(jobId, copyInfo);
        }
        // 发送备份完成消息
        if (resp != null && StringUtils.isNotBlank(resp.getUuid())) {
            updateJob(requestId, jobId, extInfo, copyInfoList);
            sendCompleteMessage(requestId, jobId, taskResponse, followUpProvider, resp);
        }
    }

    private String addDataSizeToProperties(String properties, Map extInfo) {
        if (extInfo == null) {
            return properties;
        }
        JSONObject result = JSONObject.fromObject(properties);
        String dataBeforeReduction = String.valueOf(extInfo.getOrDefault(JobExtendInfoKeys.DATA_BEFORE_REDUCTION, 0));
        String dataAfterReduction = String.valueOf(extInfo.getOrDefault(JobExtendInfoKeys.DATA_AFTER_REDUCTION, 0));
        result.put(JobExtendInfoKeys.DATA_BEFORE_REDUCTION, dataBeforeReduction);
        result.put(JobExtendInfoKeys.DATA_AFTER_REDUCTION, dataAfterReduction);
        return result.toString();
    }

    private void updateJob(String requestId, String jobId, Map extInfo, List<CopyInfoBo> copyInfoList) {
        UpdateJobRequest updateJobRequest = new UpdateJobRequest();
        CopyInfoBo copyInfoBo = copyInfoList.get(copyInfoList.size() - 1);
        // 更新时间戳
        updateJobRequest.setCopyTime(Long.parseLong(copyInfoBo.getTimestamp()) / THOUSAND);
        // 更新扩展字段
        JSONObject extendField = JSONObject.fromObject(extInfo);
        extendField.put("backupType", BackupTypeConstants.convert2BackupType(copyInfoBo.getBackupType()));
        extendField.put("sourceCopyType", BackupTypeConstants.convert2BackupType(copyInfoBo.getSourceCopyType()));
        updateJobRequest.setExtendStr(extendField.toString());
        log.info(
                "request_id:{}, extend info={}, backup task complete, update copy time and extend info",
                requestId,
                extendField);
        jobService.updateJob(jobId, updateJobRequest);
    }

    private void sendCompleteMessage(
            String requestId,
            String jobId,
            TaskCompleteMessageBo taskResponse,
            BackupFollowUpProvider followUpProvider,
            UuidObject resp) {
        // 发送备份完成消息到DataProtectionService
        log.info("request_id:{}, backup task complete, send backup done message", requestId);
        int status = DmcJobStatus.getByStatus(taskResponse.getJobStatus()).getProtectionStatus();
        followUpProvider.handleSuccess(requestId, jobId, status, resp.getUuid());
    }

    private void addCopyArchiveMap(String requestId, CopyInfoBo copyInfoBo) {
        if (!copyInfoBo.getResourceSubType().equals(ResourceSubTypeEnum.CLOUD_BACKUP_FILE_SYSTEM.getType())) {
            return;
        }
        log.debug("cloud backup need add copy archive map");
        RMap<String, String> rMap = redissonClient.getMap(requestId, StringCodec.INSTANCE);
        String policy = rMap.get("policy");
        JSONObject extParam = JSONObject.fromObject(policy).getJSONObject("ext_parameters");
        String storageId = extParam.getString("storage_id");

        AddCopyArchiveMapRequest request =
                new AddCopyArchiveMapRequest(copyInfoBo.getUuid(), storageId, copyInfoBo.getResourceId());
        copyRestApi.addCopyArchiveMap(request);
    }
}
