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
package openbackup.data.access.framework.protection.service.archive;

import com.huawei.oceanprotect.base.cluster.sdk.service.MemberClusterService;
import com.huawei.oceanprotect.job.constants.JobExtendInfoKeys;
import com.huawei.oceanprotect.job.sdk.JobService;
import com.huawei.oceanprotect.system.base.label.service.LabelService;
import com.huawei.oceanprotect.system.base.user.service.ResourceSetApi;

import com.fasterxml.jackson.databind.JsonNode;
import com.google.common.collect.Lists;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.copy.mng.constant.CopyPropertiesKeyConstant;
import openbackup.data.access.framework.core.common.constants.ContextConstants;
import openbackup.data.access.framework.core.common.constants.CopyInfoConstants;
import openbackup.data.access.framework.core.common.constants.TopicConstants;
import openbackup.data.access.framework.core.common.enums.CopyIndexStatus;
import openbackup.data.access.framework.core.common.enums.DmcJobStatus;
import openbackup.data.access.framework.core.common.util.CopyInfoBuilder;
import openbackup.data.access.framework.protection.common.constants.ArchivePolicyKeyConstant;
import openbackup.data.access.framework.protection.common.enums.ArchiveTypeEnum;
import openbackup.data.access.framework.protection.service.context.ContextManager;
import openbackup.data.access.framework.protection.service.job.InternalApiHub;
import openbackup.data.access.framework.protection.service.quota.UserQuotaManager;
import openbackup.data.protection.access.provider.sdk.base.Qos;
import openbackup.data.protection.access.provider.sdk.base.v2.BaseStorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.enums.CopyFeatureEnum;
import openbackup.data.protection.access.provider.sdk.enums.ProviderJobStatusEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryProtocolEnum;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.enums.RetentionTypeEnum;
import openbackup.system.base.common.enums.TimeUnitEnum;
import openbackup.system.base.common.enums.WormValidityTypeEnum;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.msg.NotifyManager;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.copy.model.CopyInfo;
import openbackup.system.base.sdk.copy.model.CopyStatus;
import openbackup.system.base.sdk.copy.model.CopyWormStatus;
import openbackup.system.base.sdk.job.constants.JobProgress;
import openbackup.system.base.sdk.job.model.JobStatusEnum;
import openbackup.system.base.sdk.job.model.request.UpdateJobRequest;
import openbackup.system.base.sdk.protection.model.PolicyBo;
import openbackup.system.base.sdk.protection.model.QosBo;

import org.apache.commons.lang3.StringUtils;
import org.apache.logging.log4j.util.Strings;
import org.springframework.beans.BeanUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.util.Date;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.UUID;
import java.util.concurrent.TimeUnit;

/**
 * 归档任务服务类，用例隔离对其它组件的依赖，简化manager代码
 *
 **/
@Slf4j
@Service
public class ArchiveTaskService {
    private static final String ARCHIVE_TASK_STATUS_KEY = "status";

    private static final String ENFORCE_STOP = "enforce-stop";

    private final ArchiveRepositoryService repositoryService;

    private final ContextManager contextManager;

    private final NotifyManager notifyManager;

    private final InternalApiHub internalApiHub;

    private final UserQuotaManager userQuotaManager;

    @Autowired
    private JobService jobService;

    @Autowired
    private MemberClusterService memberClusterService;

    @Autowired
    private ResourceSetApi resourceSetApi;

    @Autowired
    private LabelService labelService;

    /**
     * 归档任务服务构造函数
     *
     * @param repositoryService 归档存储库管理器
     * @param contextManager 上下文管理器
     * @param notifyManager kafka消息发送器
     * @param internalApiHub 内部api集合
     * @param userQuotaManager 额度管理器
     */
    public ArchiveTaskService(ArchiveRepositoryService repositoryService, ContextManager contextManager,
        NotifyManager notifyManager, InternalApiHub internalApiHub, UserQuotaManager userQuotaManager) {
        this.repositoryService = repositoryService;
        this.contextManager = contextManager;
        this.notifyManager = notifyManager;
        this.internalApiHub = internalApiHub;
        this.userQuotaManager = userQuotaManager;
    }

    /**
     * 根据归档policy的扩展参数获取归档存储库信息
     *
     * @param extParameters 归档策略的扩展参数
     * @param isNeedAuth true:需要密码；false：不需要密码
     * @return 存储库信息 {@code StorageRepository}
     */
    StorageRepository getRepositoryFromPolicyExtParameters(JSONObject extParameters, boolean isNeedAuth) {
        // 从SLA高级参数中获取归档存储的id
        String archiveStorageId = extParameters.getString(ArchivePolicyKeyConstant.STORAGE_ID_KEY);
        if (StringUtils.isEmpty(archiveStorageId)) {
            archiveStorageId = extParameters.getString(ArchivePolicyKeyConstant.STORAGE_ID_KEY);
        }

        final int protocol = extParameters.getInt(ArchivePolicyKeyConstant.PROTOCOL_KEY);
        log.info("query archive repository, repository id={}, protocol={}", archiveStorageId, protocol);
        StorageRepository storageRepository = repositoryService.queryRepository(archiveStorageId,
            RepositoryProtocolEnum.getByProtocol(protocol));
        if (!isNeedAuth && Objects.nonNull(storageRepository)) {
            storageRepository.cleanAuth();
        }
        return storageRepository;
    }

    /**
     * 获得本地存储信息
     *
     * @return 本地存储
     */
    StorageRepository buildLocalStorageRepository() {
        return repositoryService.buildLocalStorageRepository();
    }

    StorageRepository buildStorageRepositoryByCopyEsn(String copyEsn) {
        return repositoryService.buildStorageRepositoryByCopyEsn(copyEsn);
    }

    /**
     * 查询qos策略限速信息
     *
     * @param qosId qos策略id
     * @return qos信息对象 {@code Qos}
     */
    Optional<Qos> queryQos(String qosId) {
        if (StringUtils.isBlank(qosId)) {
            return Optional.empty();
        }
        final QosBo qosBo = internalApiHub.getQosCommonRestApi().queryQos(qosId);
        if (qosBo == null) {
            log.info("query archive qos is empty, qos id={}", qosId);
            return Optional.empty();
        }
        final Qos qos = new Qos();
        qos.setBandwidth(qosBo.getSpeedLimit());
        return Optional.of(qos);
    }

    /**
     * 保存归档副本信息
     *
     * @param requestId 归档任务请求id
     * @param extendsInfo 任务扩展信息
     * @return 副本信息 {@code CopyInfo}
     */
    CopyInfo buildAndSaveArchiveCopy(String requestId, Map extendsInfo) {
        final CopyInfo copyInfo = buildArchiveCopy(requestId, extendsInfo);
        internalApiHub.getCopyRestApi().saveCopy(copyInfo);
        resourceSetApi.createCopyResourceSetRelation(copyInfo.getUuid(), copyInfo.getParentCopyUuid(), Strings.EMPTY);
        // 设置副本继承资源的标签
        labelService.setCopyLabel(copyInfo.getUuid(), copyInfo.getResourceId());
        final ArchiveContext archiveContext = contextManager.getArchiveContext(requestId);
        updateDataBeforeReduction(archiveContext.getJobId(), copyInfo.getProperties());
        userQuotaManager.increaseUsedQuota(requestId, copyInfo);
        return copyInfo;
    }

    private CopyInfo buildArchiveCopy(String requestId, Map extendsInfo) {
        final ArchiveContext archiveContext = contextManager.getArchiveContext(requestId);
        CopyInfo copyInfo = new CopyInfo();
        final String originalCopyId = archiveContext.getOriginalCopyId();
        final Copy originalCopy = internalApiHub.getCopyRestApi().queryCopyByID(originalCopyId, true);
        BeanUtils.copyProperties(originalCopy, copyInfo);
        // 归档设置默认未设置
        copyInfo.setWormStatus(CopyWormStatus.UNSET.getStatus());
        copyInfo.setWormExpirationTime(null);
        copyInfo.setWormValidityType(WormValidityTypeEnum.WORM_NOT_OPEN.getType());
        copyInfo.setWormDurationUnit(null);
        copyInfo.setParentCopyUuid(originalCopyId);
        final String archiveCopyId = archiveContext.getArchiveCopyId()
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST,
                "archive copy id can not find in context"));
        copyInfo.setUuid(archiveCopyId);
        copyInfo.setStatus(CopyStatus.NORMAL.getValue());
        copyInfo.setIsReplicated(originalCopy.getGeneratedBy().equals(CopyGeneratedByEnum.BY_REPLICATED.value()));
        copyInfo.setIsArchived(true);
        final String nowDatetimeStr = LocalDateTime.now().format(DateTimeFormatter.ISO_LOCAL_DATE_TIME);
        copyInfo.setTimestamp(String.valueOf(TimeUnit.MILLISECONDS.toMicros(System.currentTimeMillis())));
        copyInfo.setDisplayTimestamp(nowDatetimeStr);
        copyInfo.setGeneratedTime(nowDatetimeStr);
        copyInfo.setIndexed(CopyInfoConstants.COPY_INIT_INDEXED);
        final JSONObject extParameters = JSONObject.fromObject(archiveContext.getPolicyExtParams());
        final StorageRepository archiveRepository = getRepositoryFromPolicyExtParameters(extParameters, false);
        setCopyLocation(copyInfo, archiveRepository);
        // 根据归档存储协议，选择不同的副本生成类型
        final CopyGeneratedByEnum copyGenerateType = selectCopyGenerateType(
            extParameters.getInt(ArchivePolicyKeyConstant.PROTOCOL_KEY));
        copyInfo.setGeneratedBy(copyGenerateType.value());
        copyInfo.setFeatures(CopyFeatureEnum.setAndGetFeatures(Lists.newArrayList(CopyFeatureEnum.RESTORE)));
        if (!archiveContext.getManualArchiveTag()) {
            copyInfo.setSlaName(archiveContext.getSlaName());
            copyInfo.setSlaProperties(archiveContext.getSlaJson());
        }
        setCopyProperty(extendsInfo, archiveContext, copyInfo, archiveRepository);
        // 临时设置为永久保留，后续单独处理
        if (archiveContext.getManualArchiveTag()) {
            setRetentionForManual(copyInfo, archiveContext.getPolicy());
        } else {
            setRetention(copyInfo, archiveContext.getPolicy());
        }
        // FC/HCS 归档副本不支持索引
        if (CopyInfoConstants.UNSUPPORTED_INDEX_TYPES.contains(copyInfo.getResourceSubType())) {
            copyInfo.setIndexed(CopyIndexStatus.UNSUPPORT.getIndexStaus());
        }
        copyInfo.setDeviceEsn(memberClusterService.getCurrentClusterEsn());
        String originCopyTimeStamp =
            StringUtils.isNotEmpty(originalCopy.getOriginCopyTimeStamp()) ? originalCopy.getOriginCopyTimeStamp()
                : String.valueOf(TimeUnit.MICROSECONDS.toMillis(Long.parseLong(originalCopy.getTimestamp())));
        copyInfo.setOriginCopyTimeStamp(originCopyTimeStamp);
        return copyInfo;
    }

    private void setRetentionForManual(CopyInfo copyInfo, PolicyBo policy) {
        log.info("[ARCHIVE_TASK] Get archive tape policy copy={}.", copyInfo.getUuid());
        if (RetentionTypeEnum.PERMANENT.equals(RetentionTypeEnum.getByType(policy.getRetention().getRetentionType()))) {
            log.info("[ARCHIVE_TASK] Archive permanent copies.");
            // 执行的备份类型在SLA中未开启或者保留策略为永久保留
            copyInfo.setRetentionType(RetentionTypeEnum.PERMANENT.getType());
            copyInfo.setExpirationTime(null);
        } else {
            copyInfo.setRetentionType(RetentionTypeEnum.TEMPORARY.getType());
            copyInfo.setDurationUnit(policy.getExtParameters().get(ArchivePolicyKeyConstant.DURATION_UNIT).asText());
            copyInfo.setRetentionDuration(
                    policy.getExtParameters().get(ArchivePolicyKeyConstant.RETENTION_DURATION).asInt());
            log.info("[ARCHIVE_TASK] Set retention duration success.");
            copyInfo.setExpirationTime(CopyInfoBuilder.computeExpirationTime(new Date().getTime(),
                    TimeUnitEnum.getByUnit(copyInfo.getDurationUnit()), copyInfo.getRetentionDuration()));
        }
    }

    private void setCopyProperty(Map extendsInfo, ArchiveContext archiveContext, CopyInfo copyInfo,
        StorageRepository archiveRepository) {
        String properties = copyInfo.getProperties();
        JSONObject propertyJson = JSONObject.fromObject(properties);
        propertyJson.put(CopyPropertiesKeyConstant.KEY_ARCHIVE_COPY_ID, archiveContext.getArchiveCopyId());
        propertyJson.put(CopyPropertiesKeyConstant.KEY_ARCHIVE_REPOSITORY_ID, archiveRepository.getId());
        final JSONArray repositories = propertyJson.getJSONArray(CopyPropertiesKeyConstant.KEY_REPOSITORIES);
        final BaseStorageRepository baseStorageRepository = new BaseStorageRepository();
        BeanUtils.copyProperties(archiveRepository, baseStorageRepository);
        repositories.add(baseStorageRepository);
        propertyJson.put(CopyPropertiesKeyConstant.KEY_REPOSITORIES, repositories);
        propertyJson.put(CopyPropertiesKeyConstant.SIZE, extendsInfo.get(JobExtendInfoKeys.DATA_BEFORE_REDUCTION));
        // 将手动归档填的参数放到副本的properties里面
        propertyJson.put(CopyPropertiesKeyConstant.IS_MANUAL_ARCHIVE, archiveContext.getManualArchiveTag());
        propertyJson.put(CopyPropertiesKeyConstant.MANUAL_ARCHIVE_POLICY,
                JSONObject.writeValueAsString(archiveContext.getPolicy()));
        copyInfo.setProperties(propertyJson.toString());
    }

    /**
     * 设置副本位置
     * 对象归档：副本位置为：桶(endpoint)
     *
     * @param copyInfo 副本信息
     * @param archiveRepository 归档存储库信息
     */
    private void setCopyLocation(CopyInfo copyInfo, StorageRepository archiveRepository) {
        String targetLocation = Objects.equals(archiveRepository.getProtocol(), RepositoryProtocolEnum.S3.getProtocol())
            ? archiveRepository.getPath().concat("(").concat(archiveRepository.getEndpoint().getIp()).concat(")")
            : archiveRepository.getEndpoint().getIp();
        copyInfo.setLocation(targetLocation);
    }

    private void setRetention(CopyInfo copyInfo, PolicyBo policy) {
        if (RetentionTypeEnum.PERMANENT.equals(RetentionTypeEnum.getByType(policy.getRetention().getRetentionType()))) {
            log.info("[ARCHIVE_TASK] Archive permanent copies.");
            // 执行的备份类型在SLA中未开启或者保留策略为永久保留
            copyInfo.setRetentionType(RetentionTypeEnum.PERMANENT.getType());
            copyInfo.setExpirationTime(null);
        } else {
            Optional.of(policy).map(PolicyBo::getExtParameters)
                    .map(jsonNode -> jsonNode.get(ArchivePolicyKeyConstant.ARCHIVE_TARGET_TYPE)).map(JsonNode::asInt)
                    .map(ArchiveTypeEnum::getArchiveTypeEnum)
                    .ifPresent(archiveTypeEnum -> archiveTypeEnum.handleCopy(copyInfo, policy));
        }
    }

    /**
     * 发送归档完成消息
     *
     * @param requestId 归档任务请求id
     * @param jobStatus 归档任务状态
     */
    void sendArchiveDoneMessage(String requestId, DmcJobStatus jobStatus) {
        final ArchiveContext archiveContext = contextManager.getArchiveContext(requestId);
        JSONObject archiveRequest = new JSONObject();
        archiveRequest.put(ContextConstants.REQUEST_ID, requestId);
        archiveRequest.put(ArchiveContext.JOB_ID, archiveContext.getJobId());
        archiveRequest.put(ARCHIVE_TASK_STATUS_KEY, jobStatus.getProtectionStatus());
        archiveRequest.put(ArchiveContext.ORIGINAL_COPY_ID_KEY, archiveContext.getOriginalCopyId());
        archiveRequest.put(ArchiveContext.AUTO_RETRY_TIMES, archiveContext.getRetryTimes());
        notifyManager.send(TopicConstants.TASK_ARCHIVE_DONE_TOPIC, archiveRequest.toString());
    }

    /**
     * 修改job为指定状态
     *
     * @param jobId 任务id
     */
    void updateJobStatusRunning(String jobId) {
        log.info("Restore task update job status, jobId={}, status={}", jobId, ProviderJobStatusEnum.RUNNING.name());
        UpdateJobRequest updateRequest = new UpdateJobRequest();
        updateRequest.setStatus(JobStatusEnum.get(ProviderJobStatusEnum.RUNNING.name()));
        updateRequest.setData(new JSONObject().set(ENFORCE_STOP, false));
        updateRequest.setProgress(JobProgress.DELIVER_JOB_PROGRESS);
        internalApiHub.getJobService().updateJob(jobId, updateRequest);
    }

    /**
     * 更新归档任务的缩减前数据量，与备份缩减前数据量保持一致
     *
     * @param jobId jobId
     * @param copyProperties copyProperties
     */
    public void updateDataBeforeReduction(String jobId, String copyProperties) {
        JSONObject properties = JSONObject.fromObject(copyProperties);
        String dataBeforeReduction = properties.getString(CopyPropertiesKeyConstant.SIZE);
        if (!VerifyUtil.isEmpty(dataBeforeReduction)) {
            log.info("Send updateJobRequest!dataBeforeReduction:{}", dataBeforeReduction);
            UpdateJobRequest updateJobRequest = new UpdateJobRequest();
            updateJobRequest.setExtendStr(
                new JSONObject().set(JobExtendInfoKeys.DATA_BEFORE_REDUCTION, dataBeforeReduction).toString());
            jobService.updateJob(jobId, updateJobRequest);
        }
    }

    /**
     * 获取副本信息
     *
     * @param copyId 副本id
     * @return 副本信息{@code Copy}
     */
    Copy getArchiveCopy(String copyId) {
        return internalApiHub.getCopyRestApi().queryCopyByID(copyId, true);
    }

    /**
     * 生成归档副本id并设置到到上下文中 如果已经存在，返回已存在的；如果不存在，随机生成并返回
     *
     * @param requestId 归档任务请求id
     * @return 上下文中归档副本id
     */
    String genArchiveCopyIdAndSetToContext(String requestId) {
        final ArchiveContext archiveContext = contextManager.getArchiveContext(requestId);
        final String archiveCopyId = archiveContext.getArchiveCopyId().orElse(UUID.randomUUID().toString());
        archiveContext.setArchiveCopyId(archiveCopyId);
        return archiveCopyId;
    }

    private CopyGeneratedByEnum selectCopyGenerateType(int protocol) {
        switch (RepositoryProtocolEnum.getByProtocol(protocol)) {
            case S3 :
                return CopyGeneratedByEnum.BY_CLOUD_ARCHIVE;
            case TAPE :
                return CopyGeneratedByEnum.BY_TAPE_ARCHIVE;
            default :
                throw new IllegalArgumentException("function not support this protocol");
        }
    }

    /**
     *  查询存储信息列表
     *
     * @param storageId 存储库id
     * @return 存储库信息列表
     */
    List<StorageRepository> buildSubRepositoryList(String storageId) {
        return repositoryService.buildSubRepositoryList(storageId);
    }
}
