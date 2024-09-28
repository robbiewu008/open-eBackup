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
package openbackup.data.access.framework.backup.listener.v1;

import openbackup.data.access.framework.backup.service.IBackupService;
import openbackup.data.access.framework.core.common.constants.ContextConstants;
import openbackup.data.access.framework.core.common.constants.TopicConstants;
import openbackup.data.access.framework.core.common.enums.v2.CopyTypeEnum;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.service.quota.UserQuotaManager;
import openbackup.data.protection.access.provider.sdk.backup.BackupObject;
import openbackup.data.protection.access.provider.sdk.backup.BackupProvider;
import openbackup.data.protection.access.provider.sdk.backup.ProtectedObject;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.sla.Policy;
import openbackup.data.protection.access.provider.sdk.sla.Sla;
import com.huawei.oceanprotect.functionswitch.template.service.FunctionSwitchService;
import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.kafka.annotations.MessageListener;
import openbackup.system.base.sdk.job.JobCenterRestApi;
import openbackup.system.base.sdk.job.constants.JobProgress;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.job.model.request.UpdateJobRequest;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.security.exterattack.ExterAttack;

import jodd.util.StringUtil;
import lombok.extern.slf4j.Slf4j;

import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.kafka.support.Acknowledgment;
import org.springframework.stereotype.Component;

import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * Backup msg listener, this lister will consume the backup msg and the specified provider is found by provider registry
 * to complete the backup service logic processing.
 *
 */
@Component
@Slf4j
public class ProtectionBackupListener {
    /**
     * JOB_STATUS_LOG
     */
    protected static final String JOB_STATUS_LOG = "job_status_{payload?.job_status|status}_label";

    /**
     * 服务提供者注册
     */
    private final ProviderManager providerManager;

    /**
     * Redis会话客户端
     */
    private final RedissonClient redissonClient;
    private final IBackupService backupService;
    private final ResourceService resourceService;
    private final JobCenterRestApi jobCenterRestApi;

    private JobService jobService;
    private FunctionSwitchService functionSwitchService;
    private UserQuotaManager userQuotaManager;

    // 老框架已经支持资源类型列表
    @Value("${supported.resources.backup}")
    private List<String> supportedResources;

    public ProtectionBackupListener(ProviderManager providerManager, RedissonClient redissonClient,
        IBackupService backupService, ResourceService resourceService, JobCenterRestApi jobCenterRestApi) {
        this.providerManager = providerManager;
        this.redissonClient = redissonClient;
        this.backupService = backupService;
        this.resourceService = resourceService;
        this.jobCenterRestApi = jobCenterRestApi;
    }

    @Autowired
    public void setJobService(final JobService jobService) {
        this.jobService = jobService;
    }

    @Autowired
    public void setFunctionSwitchService(final FunctionSwitchService functionSwitchService) {
        this.functionSwitchService = functionSwitchService;
    }

    @Autowired
    public void setUserQuotaManager(final UserQuotaManager userQuotaManager) {
        this.userQuotaManager = userQuotaManager;
    }

    /**
     * Consume protection backup topic message
     * 消费保护备份消息主题
     *
     * @param consumerString kafka message(Map)
     * @param acknowledgment ack
     */
    @ExterAttack
    @MessageListener(topics = TopicConstants.EXECUTE_BACKUP_PLAN, containerFactory = "retryFactory",
        log = {"job_log_protection_backup_execute_label", JOB_STATUS_LOG})
    public void backup(String consumerString, Acknowledgment acknowledgment) {
        log.info("ProtectionBackupListener receive topic:{}", TopicConstants.EXECUTE_BACKUP_PLAN);
        if (consumerString == null) {
            log.info("Protection msg is null");
            return;
        }

        BackupObject backupObject = toBackupObject(consumerString);
        if (!jobService.isJobPresent(backupObject.getRequestId())) {
            log.info("Job({}) not exist, no need to process", backupObject.getRequestId());
            return;
        }
        JobBo job = jobService.queryJob(backupObject.getRequestId());
        // 异常场景kafka可能重复消费，如果进度大于5%说明已经下发给UBC，不再重复下发
        if (job.getProgress() != null && job.getProgress() > JobProgress.DELIVER_JOB_PROGRESS) {
            log.info("Job({}) already finish this process, no need to reprocess", backupObject.getRequestId());
            return;
        }
        String resourceSubType = backupObject.getProtectedObject().getSubType();
        log.info("Backup resource subtype:{}, backupType:{}, job id: {}.", resourceSubType,
            backupObject.getBackupType(), backupObject.getRequestId());

        backupPreCheck(job);
        // 资源子类型在是当前已经支持资源子类型，则走老框架的处理流程
        if (isOldCopyBackup(resourceSubType, backupObject.getBackupType())) {
            BackupProvider protectionProvider = providerManager.findProvider(BackupProvider.class, resourceSubType);
            protectionProvider.backup(backupObject);
        } else {
            backupService.backup(backupObject);
            setJobEnforceStopToFalse(backupObject);
        }
    }

    // 新老应用下发备份前校验是否能够备份及额度
    private void backupPreCheck(JobBo job) {
        String userId = job.getUserId();

        log.debug("User: {} has ability to backup.", userId);
        userQuotaManager.checkBackupQuota(userId, null);
    }

    private boolean isOldCopyBackup(String resourceSubType, String backupType) {
        // 如果是本地文件系统且是快照备份，则走新框架
        if (ResourceSubTypeEnum.CLOUD_BACKUP_FILE_SYSTEM.getType().equals(resourceSubType)
            && CopyTypeEnum.NATIVE_SNAPSHOT.getCopyType().equals(backupType)) {
            return false;
        }
        return supportedResources.contains(resourceSubType);
    }

    private void setJobEnforceStopToFalse(BackupObject backupObject) {
        UpdateJobRequest updateJobRequest = new UpdateJobRequest();
        jobCenterRestApi.updateJob(backupObject.getTaskId(), updateJobRequest, false);
    }

    @ExterAttack
    private BackupObject toBackupObject(String consumerString) {
        // 从消息体中取出request_id
        Map map = JsonUtil.read(consumerString, Map.class);
        String requestId = String.valueOf(map.get("request_id"));

        // 根据requestId取出缓存的保护对象等信息
        RMap<String, String> redis = redissonClient.getMap(requestId, StringCodec.INSTANCE);
        redis.put(ContextConstants.JOB_TYPE, JobTypeEnum.BACKUP.name());

        // 从redis中读取转换为protectedObject
        String jsonStr = redis.get("protected_object");
        if (StringUtil.isEmpty(jsonStr)) {
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "Task already be stopped or failed!");
        }
        ProtectedObject protectedObject = JSONObject.toBean(jsonStr, ProtectedObject.class);

        Optional<ProtectedResource> resOptional = resourceService.getResourceById(protectedObject.getResourceId());
        resOptional.ifPresent(protectedResource -> {
            // 将资源的扩展信息更新到资源中，并更新Redis缓存
            log.debug("Update Resource in the redis cache with V2 Resource.");
            JSONObject jsonObject = JSONObject.fromObject(redis.get(ContextConstants.RESOURCE))
                .set("extendInfo", protectedResource.getExtendInfo());
            redis.put(ContextConstants.RESOURCE, jsonObject.toString());
        });

        // 从redis中读取转换为sla
        String slaStr = redis.get("sla");
        Sla slaBo = JSONObject.toBean(slaStr, Sla.class);
        // 整理出备份对象
        BackupObject backupObject = new BackupObject();
        backupObject.setRequestId(requestId);
        backupObject.setProtectedObject(protectedObject);
        backupObject.setBackupType(redis.get("backup_type"));
        backupObject.setTaskId(redis.get("job_id"));
        backupObject.setChainId(redis.get("chain_id"));
        backupObject.setSla(slaBo);

        Policy policy = JSONObject.fromObject(redis.get("policy")).toBean(Policy.class);
        backupObject.setPolicy(policy);
        return backupObject;
    }
}
