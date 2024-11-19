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
package openbackup.data.access.framework.protection.handler.v1.archive;

import com.huawei.oceanprotect.base.cluster.sdk.service.MemberClusterService;
import com.huawei.oceanprotect.job.constants.JobExtendInfoKeys;
import com.huawei.oceanprotect.system.base.user.service.ResourceSetApi;

import jodd.util.StringUtil;
import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.copy.mng.constant.CopyPropertiesKeyConstant;
import openbackup.data.access.framework.core.common.constants.ContextConstants;
import openbackup.data.access.framework.core.common.constants.TopicConstants;
import openbackup.data.access.framework.core.common.enums.DmcJobStatus;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.handler.TaskCompleteHandler;
import openbackup.data.access.framework.protection.service.archive.ArchiveTaskService;
import openbackup.data.access.framework.protection.service.quota.UserQuotaManager;
import openbackup.data.protection.access.provider.sdk.copy.CopyBo;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.copy.CopyProvider;
import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.enums.WormValidityTypeEnum;
import openbackup.system.base.common.exception.DataMoverCheckedException;
import openbackup.system.base.common.msg.NotifyManager;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.common.model.UuidObject;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.copy.model.CopyInfo;
import openbackup.system.base.sdk.copy.model.CopyWormStatus;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.protection.model.PolicyBo;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.apache.commons.lang3.StringUtils;
import org.apache.logging.log4j.util.Strings;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.beans.BeanUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.List;

/**
 * Archive Task Complete Listener
 *
 **/
@Component
@Slf4j
public class ArchiveTaskCompleteHandler implements TaskCompleteHandler {
    private static final List<ResourceSubTypeEnum> ARCHIVE_EXPORT_RESOURCE_TYPE_LIST =
            Arrays.asList(ResourceSubTypeEnum.ORACLE, ResourceSubTypeEnum.VMWARE);

    @Autowired
    private RedissonClient redissonClient;

    @Autowired
    private NotifyManager notifyManager;

    @Autowired
    private CopyRestApi copyRestApi;

    @Autowired
    private ProviderManager providerManager;

    @Autowired
    private UserQuotaManager userQuotaManager;

    @Autowired
    private ArchiveTaskService archiveTaskService;

    @Autowired
    private MemberClusterService memberClusterService;

    @Autowired
    private ResourceSetApi resourceSetApi;

    /**
     * detect object applicable
     *
     * @param object object
     * @return detect result
     */
    @Override
    public boolean applicable(String object) {
        return JobTypeEnum.ARCHIVE.getValue().equals(object);
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
        RMap<String, String> context = redissonClient.getMap(requestId, StringCodec.INSTANCE);
        String jobType = context.get(ContextConstants.JOB_TYPE);
        if (StringUtil.isEmpty(jobType) || !JobTypeEnum.ARCHIVE.getValue().equalsIgnoreCase(jobType)) {
            log.info("archive complete, requestId is {}, jobType is not [ARCHIVE]", requestId);
            return;
        }
        String copyId = context.get(ContextConstants.COPY_ID);
        String resourceSubtype = context.get("resource_sub_type");
        String jobId = context.get(ContextConstants.JOB_ID);
        // 构造副本信息并入库,入库成功后发送
        saveCopy(jobId, copyId, 0, resourceSubtype, taskCompleteMessage);
    }

    @ExterAttack
    @Override
    public void onTaskCompleteFailed(TaskCompleteMessageBo taskCompleteMessage) {
        String requestId = taskCompleteMessage.getJobRequestId();
        RMap<String, String> context = redissonClient.getMap(requestId, StringCodec.INSTANCE);
        String jobId = context.get(ContextConstants.JOB_ID);
        String copyId = context.get(ContextConstants.COPY_ID);
        int status = taskCompleteMessage.getJobStatus();
        DmcJobStatus jobStatus = DmcJobStatus.getByStatus(status);
        String autoRetry = context.get(ContextConstants.AUTO_RETRY_TIMES);
        int autoRestryTimes = Integer.valueOf(StringUtil.isEmpty(autoRetry) ? "0" : autoRetry);
        sendArchiveDoneMessage(requestId, jobId, jobStatus.getProtectionStatus(), copyId, autoRestryTimes);
    }

    private void saveCopy(String jobId, String copyId, int autoRestryTimes, String resourceType,
        TaskCompleteMessageBo taskCompleteMessage) {
        CopyBo tmpCopy = new CopyBo();
        tmpCopy.setResourceSubType(resourceType);
        tmpCopy.setGeneratedBy(CopyGeneratedByEnum.BY_CLOUD_ARCHIVE.value());
        CopyProvider provider = providerManager.findProvider(CopyProvider.class, tmpCopy);
        String requestId = taskCompleteMessage.getJobRequestId();
        if (VerifyUtil.isEmpty(provider)) {
            log.info("request_id:{}, can't find provider", requestId);
            return;
        }
        CopyInfoBo copy = provider.buildCopy(requestId, "", "");
        // 调用rest接口副本入库
        CopyInfo copyinfo = new CopyInfo();
        BeanUtils.copyProperties(copy, copyinfo);
        // 归档设置默认未设置
        copyinfo.setWormStatus(CopyWormStatus.UNSET.getStatus());
        copyinfo.setWormExpirationTime(null);
        copyinfo.setWormValidityType(WormValidityTypeEnum.WORM_NOT_OPEN.getType());
        copyinfo.setWormDurationUnit(null);
        copyinfo.setDeviceEsn(memberClusterService.getCurrentClusterEsn());
        UuidObject resp = copyRestApi.saveCopy(copyinfo);
        resourceSetApi.createCopyResourceSetRelation(resp.getUuid(), copyId, Strings.EMPTY);
        JSONObject propertyJson = JSONObject.fromObject(copyinfo.getProperties());
        propertyJson.put(CopyPropertiesKeyConstant.SIZE,
            taskCompleteMessage.getExtendsInfo().get(JobExtendInfoKeys.DATA_BEFORE_REDUCTION));
        copyinfo.setProperties(propertyJson.toString());
        archiveTaskService.updateDataBeforeReduction(jobId, copyinfo.getProperties());
        userQuotaManager.increaseUsedQuota(requestId, copyinfo);
        // 归档完成
        if (resp != null && StringUtils.isNotBlank(resp.getUuid())) {
            if (ARCHIVE_EXPORT_RESOURCE_TYPE_LIST.contains(ResourceSubTypeEnum.get(resourceType))) {
                // 发送导出副本消息：云上保存归档副本(只支持Oracle和VMWARE)，
                // 调用v1/dme_archive/updatesnap，失败重试，全部失败则记录jobCenter
                // 导出副本完成后，发送备份完成消息到DataProtectionService
                sendArchiveExportMessage(requestId, jobId, resp.getUuid(), copyId, autoRestryTimes);
            } else {
                // 发送备份完成消息到DataProtectionService
                sendArchiveDoneMessage(requestId, jobId, IsmNumberConstant.ONE, copyId, autoRestryTimes);
            }
        } else {
            log.error("job_id: {} save copy failed : request_id: {}", jobId, requestId);
        }
    }

    private void sendArchiveExportMessage(
            String requestId, String jobId, String archiveCopyId, String backupCopyId, int autoRestryTimes) {
        // redis获取storageId
        RMap<String, String> context = redissonClient.getMap(requestId, StringCodec.INSTANCE);
        PolicyBo policyBo = JSONObject.fromObject(context.get("policy")).toBean(PolicyBo.class);
        if (policyBo.getExtParameters().get("storage_id") == null) {
            throw new DataMoverCheckedException("storage_id is null", CommonErrorCode.ERR_PARAM);
        }
        String storageId = policyBo.getExtParameters().get("storage_id").asText();

        JSONObject archiveExportMap = new JSONObject();
        archiveExportMap.put(ContextConstants.REQUEST_ID, requestId);
        archiveExportMap.put(ContextConstants.JOB_ID, jobId);
        archiveExportMap.put("storage_Id", storageId);
        archiveExportMap.put(ContextConstants.ARCHIVE_COPY_ID, archiveCopyId);
        archiveExportMap.put(ContextConstants.COPY_ID, backupCopyId);
        archiveExportMap.put(ContextConstants.AUTO_RETRY_TIMES, String.valueOf(autoRestryTimes));
        notifyManager.send(TopicConstants.ARCHIVE_COPIES_TO_S3_TOPIC, archiveExportMap.toString());
    }

    private void sendArchiveDoneMessage(
            String requestId, String jobId, Integer status, String copyId, int autoRestryTimes) {
        JSONObject archiveDoneMap = new JSONObject();
        archiveDoneMap.put(ContextConstants.REQUEST_ID, requestId);
        archiveDoneMap.put("job_id", jobId);
        archiveDoneMap.put("status", status);
        archiveDoneMap.put("copy_id", copyId);
        archiveDoneMap.put("auto_retry_times", autoRestryTimes);
        notifyManager.send(TopicConstants.TASK_ARCHIVE_DONE_TOPIC, archiveDoneMap.toString());
    }
}
