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
package openbackup.data.access.framework.backup.handler.v2;

import com.huawei.oceanprotect.base.cluster.sdk.service.MemberClusterService;
import com.huawei.oceanprotect.base.cluster.sdk.service.StorageUnitService;
import com.huawei.oceanprotect.job.constants.JobExtendInfoKeys;
import com.huawei.oceanprotect.job.sdk.JobService;
import com.huawei.oceanprotect.system.base.label.service.LabelService;
import com.huawei.oceanprotect.system.base.user.service.ResourceSetApi;

import com.google.common.collect.ImmutableSet;
import com.baomidou.mybatisplus.core.conditions.query.QueryWrapper;
import com.google.common.collect.Lists;

import lombok.extern.slf4j.Slf4j;
import openbackup.access.framework.resource.persistence.dao.ProtectedResourceMapper;
import openbackup.access.framework.resource.persistence.model.ProtectedResourcePo;
import openbackup.data.access.client.sdk.api.framework.dme.CopyVerifyStatusEnum;
import openbackup.data.access.client.sdk.api.framework.dme.DmeCopyInfo;
import openbackup.data.access.client.sdk.api.framework.dme.DmeUnifiedRestApi;
import openbackup.data.access.framework.backup.constant.BackupConstant;
import openbackup.data.access.framework.backup.service.impl.JobBackupPostProcessService;
import openbackup.data.access.framework.copy.mng.constant.CopyPropertiesKeyConstant;
import openbackup.data.access.framework.copy.mng.service.CopyService;
import openbackup.data.access.framework.core.common.constants.ContextConstants;
import openbackup.data.access.framework.core.common.constants.CopyConstants;
import openbackup.data.access.framework.core.common.constants.CopyInfoConstants;
import openbackup.data.access.framework.core.common.constants.TopicConstants;
import openbackup.data.access.framework.core.common.enums.CopyIndexStatus;
import openbackup.data.access.framework.core.common.enums.DmcJobStatus;
import openbackup.data.access.framework.core.common.enums.v2.CopyTypeEnum;
import openbackup.data.access.framework.core.common.util.CopyInfoBuilder;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.common.converters.JobDataConverter;
import openbackup.data.access.framework.protection.common.util.JobExtendInfoUtil;
import openbackup.data.access.framework.protection.common.util.StorageRepositoryUtil;
import openbackup.data.access.framework.protection.handler.v2.UnifiedTaskCompleteHandler;
import openbackup.data.access.framework.protection.service.quota.UserQuotaManager;
import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.data.protection.access.provider.sdk.backup.NextBackupChangeCauseEnum;
import openbackup.data.protection.access.provider.sdk.backup.NextBackupModifyReq;
import openbackup.data.protection.access.provider.sdk.backup.ProtectedObject;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupInterceptorProvider;
import openbackup.data.protection.access.provider.sdk.backup.v2.PostBackupTask;
import openbackup.data.protection.access.provider.sdk.base.v2.BaseStorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.copy.CapabilityProvider;
import openbackup.data.protection.access.provider.sdk.copy.CopyCommonInterceptor;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.enums.BackupTypeEnum;
import openbackup.data.protection.access.provider.sdk.enums.CopyFeatureEnum;
import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;
import openbackup.data.protection.access.provider.sdk.enums.ProviderJobStatusEnum;
import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import openbackup.data.protection.access.provider.sdk.resource.Resource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.constants.LegoNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.enums.RetentionTypeEnum;
import openbackup.system.base.common.enums.WormValidityTypeEnum;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.msg.NotifyManager;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.anti.api.AntiRansomwareApi;
import openbackup.system.base.sdk.cluster.model.StorageUnitVo;
import openbackup.system.base.sdk.common.model.UuidObject;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.CopyExtendType;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.copy.model.CopyInfo;
import openbackup.system.base.sdk.copy.model.CopyStatus;
import openbackup.system.base.sdk.copy.model.CopyStorageUnitStatus;
import openbackup.system.base.sdk.job.JobCenterRestApi;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.job.model.request.UpdateJobRequest;
import openbackup.system.base.sdk.protection.model.PolicyBo;
import openbackup.system.base.sdk.protection.model.SlaBo;
import openbackup.system.base.sdk.repository.api.BackupStorageApi;
import openbackup.system.base.sdk.repository.model.NasDistributionStorageDetail;
import openbackup.system.base.service.DeployTypeService;

import org.apache.commons.lang3.StringUtils;
import org.apache.logging.log4j.util.Strings;
import org.springframework.beans.BeanUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Optional;
import java.util.Set;

/**
 * 统一备份任务完成处理器
 *
 */
@Slf4j
@Component
public class UnifiedBackupTaskCompleteHandler extends UnifiedTaskCompleteHandler {
    // 允许从保护对象扩展参数提取的字段列表
    private static final List<String> PROTECT_EXTEND_INFO_PARAMS = Lists.newArrayList("tenantId", "tenantName");

    private static final int THOUSAND = 1000;

    private static final String DEFAULT_POOL_ID = "0";

    private static final Set<String> INCREMENT_BACKUP_TYPE_SET = ImmutableSet.of(
            BackupTypeEnum.CUMULATIVE_INCREMENT.name().toLowerCase(Locale.ROOT),
            BackupTypeEnum.DIFFERENCE_INCREMENT.name().toLowerCase(Locale.ROOT),
            BackupTypeEnum.PERMANENT_INCREMENT.name().toLowerCase(Locale.ROOT));

    @Autowired
    private DmeUnifiedRestApi unifiedRestApi;

    @Autowired
    private CopyRestApi copyRestApi;

    @Autowired
    private BackupStorageApi backupStorageApi;

    @Autowired
    private JobService jobService;

    @Autowired
    private ProviderManager providerManager;

    @Autowired
    private NotifyManager notifyManager;

    @Autowired
    private ResourceService resourceService;

    @Autowired
    private UserQuotaManager userQuotaManager;

    @Autowired
    private DeployTypeService deployTypeService;

    @Autowired
    private MemberClusterService memberClusterService;

    @Autowired
    private JobBackupPostProcessService jobBackupPostProcessService;

    @Autowired
    private StorageUnitService storageUnitService;

    @Autowired
    private CopyService copyService;

    @Autowired
    private ResourceSetApi resourceSetApi;

    @Autowired
    private LabelService labelService;

    @Autowired
    private AntiRansomwareApi antiRansomwareApi;

    @Autowired
    private ProtectedResourceMapper protectedResourceMapper;

    @Override
    public void onTaskCompleteSuccess(TaskCompleteMessageBo taskMessage) {
        JobBo job = jobService.queryJob(taskMessage.getJobId());
        taskCompleteSuccessOrCheckPointProcess(taskMessage, false, isCheckPointOn(job));
        jobBackupPostProcessService.onBackupJobSuccess(taskMessage.getJobId());
    }

    private void taskCompleteSuccessOrCheckPointProcess(
            TaskCompleteMessageBo taskMessage, boolean isFailTaskCheckPointOn, boolean isResumableBackup) {
        String requestId = taskMessage.getJobRequestId();
        log.debug("Backup task complete, do backup success, request id: {}", requestId);

        // ubc备份任务处理完成后，pm生成副本结束任务阶段可以停止
        UpdateJobRequest stopEnableRequest = new UpdateJobRequest();
        stopEnableRequest.setData(new JSONObject().set(JobCenterRestApi.ENFORCE_STOP, true));
        jobService.updateJob(taskMessage.getTaskId(), stopEnableRequest);
        log.debug("Open backup job force stop enable, job ID: {}.", taskMessage.getTaskId());

        Map<String, String> context = taskMessage.getContext();

        // 副本ID和RequestID相同
        CopyInfo copyInfo = buildCopy(requestId, context, isFailTaskCheckPointOn);
        // 应用构建副本后置流程
        CopyCommonInterceptor provider =
                providerManager.findProvider(CopyCommonInterceptor.class, copyInfo.getResourceSubType(), null);
        if (!VerifyUtil.isEmpty(provider)) {
            provider.backupBuildCopyPostprocess(copyInfo);
        }
        String jobId = taskMessage.getJobId();
        JobBo job = jobService.queryJob(jobId);
        copyInfo.setStorageUnitId(job.getStorageUnitId());
        copyInfo.setStorageUnitStatus(CopyStorageUnitStatus.ONLINE.getValue());
        UuidObject resp;
        if (isResumableBackup) {
            resp = copyRestApi.saveCopy(copyInfo, true);
        } else {
            resp = copyRestApi.saveCopy(copyInfo);
        }
        copyService.deleteInvalidCopies(copyInfo.getResourceId(), LegoNumberConstant.TWO,
            Collections.singletonList(copyInfo.getUuid()));
        // 设置副本继承资源的标签
        labelService.setCopyLabel(copyInfo.getUuid(), copyInfo.getResourceId());
        resourceSetApi.createCopyResourceSetRelation(copyInfo.getUuid(), copyInfo.getResourceId(), Strings.EMPTY);
        if (resp != null && StringUtils.isNotBlank(resp.getUuid())) {
            // 备份完成，增加用户已使用配额
            userQuotaManager.increaseUsedQuota(requestId, copyInfo);
            UpdateJobRequest updateJobRequest = new UpdateJobRequest();
            // 更新时间戳
            updateJobRequest.setCopyId(resp.getUuid());
            updateJobRequest.setCopyTime(Long.parseLong(copyInfo.getTimestamp()) / THOUSAND);
            log.info("Update job timestamp success, request id: {}", requestId);
            JSONObject extendField = JSONObject.fromObject(taskMessage.getExtendsInfo());
            extendField.put("backupType", BackupTypeConstants.convert2BackupType(copyInfo.getBackupType()));
            extendField.put("sourceCopyType", BackupTypeConstants.convert2BackupType(copyInfo.getSourceCopyType()));
            updateJobRequest.setExtendStr(extendField.toString());
            jobService.updateJob(taskMessage.getJobId(), updateJobRequest);
            // 发送备份完成消息到DataProtectionService
            sendBackupDoneMessage(
                    requestId,
                    requestId,
                    DmcJobStatus.getByStatus(taskMessage.getJobStatus()).getProtectionStatus(),
                    resp.getUuid());
            // 后置处理
            this.postProcess(
                    context, JobDataConverter.convertToProviderJobStatus(taskMessage.getJobStatus()), copyInfo);
        }
    }

    private void sendBackupDoneMessage(String requestId, String jobId, Integer status, String copyId) {
        JSONObject message =
                new JSONObject().set(ContextConstants.REQUEST_ID, requestId).set("job_id", jobId).set("status", status);
        if (StringUtils.isBlank(copyId)) {
            message.put("copy_ids", "");
        } else {
            message.put("copy_ids", Lists.newArrayList(copyId));
        }
        log.debug("Backup task complete, send backup done message, request id:{}", requestId);
        notifyManager.send(TopicConstants.EXECUTE_BACKUP_DONE, message.toString());
    }

    private CopyInfo buildCopy(String requestId, Map<String, String> context, boolean isFailTaskCheckPointOn) {
        // 主存防勒索部署形态或安全一体机部署形态 副本查询不走ubc
        if (deployTypeService.isHyperDetectDeployType() || deployTypeService.isCyberEngine()) {
            return copyRestApi.queryCopyByID(requestId);
        }

        DmeCopyInfo dmeCopyInfo = unifiedRestApi.getCopyInfo(requestId);

        Map<String, Object> properties = buildProperties(context, dmeCopyInfo);

        CopyInfoBo copyInfoBo = buildCopyInfoBo(requestId, context, dmeCopyInfo, properties);
        fillWormRetentionInfo(copyInfoBo);
        CopyInfo copyInfo = new CopyInfo();
        BeanUtils.copyProperties(copyInfoBo, copyInfo);
        if (isFailTaskCheckPointOn) {
            copyInfo.setStatus(CopyStatus.INVALID.getValue());
            copyInfo.setExtendType(CopyExtendType.CHECKPOINT.getValue());
        }
        return copyInfo;
    }

    /**
     * 填充额外的worm保留时间
     *
     * @param copyInfoBo  副本信息
     */
    private void fillWormRetentionInfo(CopyInfoBo copyInfoBo) {
        boolean existWormPolicy = antiRansomwareApi.isExistWormPolicyByResourceId(copyInfoBo.getResourceId());
        if (existWormPolicy) {
            copyInfoBo.setWormDurationUnit(copyInfoBo.getDurationUnit());
            copyInfoBo.setWormRetentionDuration(copyInfoBo.getRetentionDuration());
            copyInfoBo.setWormValidityType(WormValidityTypeEnum.COPY_RETENTION_TIME_CONSISTENT.getType());
            if (RetentionTypeEnum.TEMPORARY.equals(RetentionTypeEnum.getByType(copyInfoBo.getRetentionType()))) {
                copyInfoBo.setWormExpirationTime(copyInfoBo.getExpirationTime());
            }
        }
    }

    private Map<String, Object> buildProperties(Map<String, String> context, DmeCopyInfo dmeCopyInfo) {
        // 设置副本属性信息
        Map<String, Object> properties = new HashMap<>();

        // 副本的快照信息
        properties.put(CopyPropertiesKeyConstant.SNAPSHOTS, dmeCopyInfo.getSnapshots());

        // 设置备份对象的子资源信息
        properties.put(CopyPropertiesKeyConstant.KEY_PROTECT_SUB_OBJECT, dmeCopyInfo.getProtectSubObject());

        // 设置副本的存储库信息
        List<BaseStorageRepository> repositoryList = getBaseStorageRepositories(dmeCopyInfo);
        properties.put(CopyPropertiesKeyConstant.KEY_REPOSITORIES, repositoryList);

        // 设置副本的逻辑大小/缩减前数据量
        properties.put(CopyPropertiesKeyConstant.SIZE, dmeCopyInfo.getSize());

        if (!VerifyUtil.isEmpty(dmeCopyInfo.getExtendInfo())) {
            properties.putAll(dmeCopyInfo.getExtendInfo());
        }

        // 增加保护对象额外参数
        selectFieldsAndSetProperties(properties, dmeCopyInfo.getProtectObject());
        log.debug("Retrieve copy type from dme: {}", dmeCopyInfo.getType());
        // 添加副本校验状态
        CopyVerifyStatusEnum copyVerifyStatusEnum = dmeCopyInfo.getCopyVerifyStatus();
        properties.put(CopyPropertiesKeyConstant.KEY_VERIFY_STATUS, copyVerifyStatusEnum.getVerifyStatus());
        // 添加副本格式
        properties.put(CopyPropertiesKeyConstant.KEY_FORMAT, dmeCopyInfo.getFormat());
        return properties;
    }

    private CopyInfoBo buildCopyInfoBo(
            String requestId, Map<String, String> context, DmeCopyInfo dmeCopyInfo, Map<String, Object> properties) {
        String resJson = context.get(CopyPropertiesKeyConstant.KEY_BACKUP_RESOURCE);
        Resource resource = JSONObject.toBean(resJson, Resource.class);
        String backupType = CopyTypeEnum.getCopyType(dmeCopyInfo.getType()).getCopyType();
        String sourceCopyType = CopyTypeEnum.getCopyType(dmeCopyInfo.getSourceCopyType()).getCopyType();
        TaskEnvironment taskEnvironment = dmeCopyInfo.getProtectEnv();
        String slaJson = context.get(CopyPropertiesKeyConstant.KEY_BACKUP_SLA);
        SlaBo slaBo = JSONObject.toBean(slaJson, SlaBo.class);
        PolicyBo policyBo = JSONObject.toBean(context.get(CopyPropertiesKeyConstant.KEY_BACKUP_POLICY), PolicyBo.class);
        String name = getLocationName(context, dmeCopyInfo);
        int abBackType = BackupTypeConstants.convert2AbBackType(backupType);
        return CopyInfoBuilder.builder()
                .setBaseCopyInfo()
                .setLocation(StringUtils.isEmpty(name) ? CopyInfoConstants.COPY_INIT_LOCATION : name)
                .setStorageUnitStatus(CopyStorageUnitStatus.ONLINE.getValue())
                .setCopyId(requestId)
                .setOriginBackupId(requestId)
                .setEBackupTimestamp(dmeCopyInfo.getTimestamp())
                .setCopyFeature(buildFeature(dmeCopyInfo))
                .setResourceInfo(resource)
                .setRetentionInfo(context, abBackType, slaBo, policyBo,
                    dmeCopyInfo.getTimestamp() * IsmNumberConstant.THOUSAND)
                .setWormRetentionInfo(slaBo, policyBo, dmeCopyInfo.getTimestamp() * IsmNumberConstant.THOUSAND)
                .setProperties(properties)
                .setOtherInfo(context.get(CopyPropertiesKeyConstant.KEY_BACKUP_CHAIN_ID), resJson, slaJson)
                .setEnvironmentInfo(taskEnvironment.getName(), taskEnvironment.getEndpoint())
                .setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value())
                .setIndexed(CopyIndexStatus.UNINDEXED.getIndexStaus())
                .setDeletable(Boolean.TRUE)
                .setBackupType(abBackType)
                .setUserId(queryUserId(resource.getUuid()))
                .setName(context.get(CopyConstants.COPY_NAME))
                .setStorageId(getStorageId(dmeCopyInfo))
                .setSourceCopyType(BackupTypeConstants.convert2AbBackType(sourceCopyType))
                .setDeviceEsn(memberClusterService.getCurrentClusterEsn())
                .setPoolId(getCopyPoolId(properties))
                .build();
    }

    private String queryUserId(String uuid) {
        QueryWrapper<ProtectedResourcePo> wrapper = new QueryWrapper<>();
        wrapper.eq("uuid", uuid);
        if (!protectedResourceMapper.exists(wrapper)) {
            return Strings.EMPTY;
        }
        return protectedResourceMapper.selectById(uuid).getUserId();
    }

    private String getLocationName(Map<String, String> context, DmeCopyInfo dmeCopyInfo) {
        // 如果是开了并行存储开关的备份存储单元组，位置信息显示备份存储单元组的名称，其他场景和之前一样
        if (!VerifyUtil.isEmpty(getStorageId(dmeCopyInfo))) {
            NasDistributionStorageDetail storageUnitGroup = backupStorageApi.getDetail(getStorageId(dmeCopyInfo));
            if (storageUnitGroup.isHasEnableParallelStorage()) {
                return storageUnitGroup.getName();
            }
        }
        String storageUnitId = jobService.queryJob(context.get(ContextConstants.JOB_ID)).getStorageUnitId();
        return VerifyUtil.isEmpty(storageUnitId)
                ? CopyInfoConstants.COPY_INIT_LOCATION
                : storageUnitService.getStorageUnitById(storageUnitId).orElse(new StorageUnitVo()).getName();
    }

    private String getCopyPoolId(Map<String, Object> properties) {
        List<StorageRepository> repositories =
                JSONObject.fromObject(properties).getJSONArray("repositories").toBean(StorageRepository.class);
        if (VerifyUtil.isEmpty(repositories)) {
            return DEFAULT_POOL_ID;
        }
        JSONObject storageInfo =
                JSONObject.fromObject(repositories.get(0).getExtendInfo()).getJSONObject("storage_info");
        if (VerifyUtil.isEmpty(storageInfo)) {
            return DEFAULT_POOL_ID;
        }
        return storageInfo.getString("storage_pool", DEFAULT_POOL_ID);
    }

    private List<BaseStorageRepository> getBaseStorageRepositories(DmeCopyInfo dmeCopyInfo) {
        List<BaseStorageRepository> repositoryList = new ArrayList<>();
        // 本地文件系统快照备份没有repositories数据
        if (dmeCopyInfo.getRepositories() != null) {
            dmeCopyInfo
                    .getRepositories()
                    .forEach(
                            repository -> {
                                String id = repository.getId();
                                Integer type = repository.getType();
                                Integer protocol = StorageRepositoryUtil.getRepositoryProtocol(repository);
                                Integer role = repository.getRole();
                                Map<String, Object> extendInfo = repository.getExtendInfo();
                                BaseStorageRepository baseStorageRepository =
                                        new BaseStorageRepository(id, type, protocol, extendInfo);
                                baseStorageRepository.setRole(role);
                                baseStorageRepository.setRemotePath(repository.getRemotePath());
                                repositoryList.add(baseStorageRepository);
                            });
        }
        return repositoryList;
    }

    private String getStorageId(DmeCopyInfo dmeCopyInfo) {
        Map<String, Object> extendInfo = dmeCopyInfo.getExtendInfo();
        if (VerifyUtil.isEmpty(extendInfo)) {
            return "";
        }
        return Optional.ofNullable(extendInfo.get("storage_id")).orElse("").toString();
    }

    private String getStorageType(DmeCopyInfo dmeCopyInfo) {
        Map<String, Object> extendInfo = dmeCopyInfo.getExtendInfo();
        if (VerifyUtil.isEmpty(extendInfo)) {
            return "";
        }
        return Optional.ofNullable(extendInfo.get("storage_type")).orElse("").toString();
    }

    private List<CopyFeatureEnum> buildFeature(DmeCopyInfo dmeCopyInfo) {
        String subType = dmeCopyInfo.getProtectObject().getSubType();
        CapabilityProvider provider = providerManager.findProvider(CapabilityProvider.class, subType, null);
        if (provider == null) {
            return Lists.newArrayList(CopyFeatureEnum.RESTORE);
        }
        return provider.supportFeatures();
    }

    private boolean isCheckPointOn(JobBo job) {
        // 不是 CheckPoint 的任务直接返回 false，直接进入失败流程
        // checkPoint 任务需要检查副本数量阈值 checkInvalidCopiesNumAndDeleteCopies
        // 达到，删除所有副本并返回 false 跳过保留无效副本流程直接进入失败流程
        // 未达到，返回 true 进入保留无效副本流程
        JSONObject protectedObjExtParam = JobExtendInfoUtil.getProtectedObjExtParam(job);
        return protectedObjExtParam != null && protectedObjExtParam.containsKey(JobExtendInfoKeys.CHECK_POINT)
            && protectedObjExtParam.getBoolean(JobExtendInfoKeys.CHECK_POINT);
    }

    private boolean deleteCopyWhenTaskRetryTouchLimit(JobBo job) {
        // 校验 retryNum 是否存在，如果不在给默认值3
        JSONObject protectedObjExtParam = JobExtendInfoUtil.getProtectedObjExtParam(job);
        int retryNum = protectedObjExtParam.containsKey(JobExtendInfoKeys.RETRY_NUM) ? protectedObjExtParam.getInt(
            JobExtendInfoKeys.RETRY_NUM) : BackupConstant.DEFAULT_CHECK_POINT_RETRY_NUM;
        // 检验 retryNum 是否在合法区间，如果不在给默认值 3
        retryNum = (retryNum >= BackupConstant.MIN_CHECK_POINT_RETRY_NUM
            && retryNum <= BackupConstant.MAX_CHECK_POINT_RETRY_NUM)
            ? retryNum
            : BackupConstant.DEFAULT_CHECK_POINT_RETRY_NUM;
        int jobOverrideTimes = jobService.getJobOverrideTimes(job.getExtendStr());
        if (jobOverrideTimes >= retryNum) {
            copyService.deleteInvalidCopies(job.getSourceId(), retryNum, Collections.emptyList());
            return true;
        }
        return false;
    }

    @Override
    public void onTaskCompleteFailed(TaskCompleteMessageBo taskMessage) {
        JobBo job = jobService.queryJob(taskMessage.getJobId());
        if (isCheckPointOn(job) && !deleteCopyWhenTaskRetryTouchLimit(job) && checkDmeCopyExist(
            taskMessage.getJobId())) {
            taskCompleteSuccessOrCheckPointProcess(taskMessage, true, true);
            jobBackupPostProcessService.onBackupJobFail(taskMessage.getJobId());
            return;
        }
        String requestId = taskMessage.getJobRequestId();
        String jobId = taskMessage.getProperty(ContextConstants.JOB_ID);
        if (VerifyUtil.isEmpty(jobId)) {
            jobId = requestId;
        }
        int status = taskMessage.getJobStatus();
        DmcJobStatus jobStatus = DmcJobStatus.getByStatus(status);
        // 发送任务终止消息到对应的workflow
        log.warn(
                "Received task complete message, requestId is {}, jobId is {}, task status is {}, abort",
                requestId,
                jobId,
                status);
        sendBackupDoneMessage(requestId, jobId, jobStatus.getProtectionStatus(), null);

        // 后置失败
        Map<String, String> context = taskMessage.getContext();
        this.postProcess(context, JobDataConverter.convertToProviderJobStatus(status), null);
        jobBackupPostProcessService.onBackupJobFail(jobId);
    }

    private boolean checkDmeCopyExist(String jobId) {
        boolean copyAdded = false;
        try {
            copyAdded = unifiedRestApi.queryCopyAdded(jobId);
        } catch (LegoCheckedException e) {
            log.error("query copy added from dme failed,job id:{}", jobId);
        }
        log.debug("task generate copy result:{}", copyAdded);
        return copyAdded;
    }

    @Override
    public boolean applicable(String object) {
        String jobType = JobTypeEnum.BACKUP.getValue() + "-" + version();
        return jobType.equals(object);
    }

    private void selectFieldsAndSetProperties(Map<String, Object> properties, TaskResource protectObject) {
        if (protectObject == null || protectObject.getExtendInfo() == null) {
            log.warn("Dme copy not exists protect object or extend info.");
            return;
        }

        for (String extendInfoParam : PROTECT_EXTEND_INFO_PARAMS) {
            log.debug("Add protect object extend info:{}", extendInfoParam);
            properties.put(extendInfoParam, protectObject.getExtendInfo().get(extendInfoParam));
        }
    }

    private void postProcess(Map<String, String> backupContext, ProviderJobStatusEnum jobStatus, CopyInfo copyInfo) {
        String protectedObjectStr = backupContext.get("protected_object");
        ProtectedObject protectedObject = JSONObject.fromObject(protectedObjectStr).toBean(ProtectedObject.class);
        int copyFormat = Integer.parseInt(backupContext.get(ContextConstants.COPY_FORMAT));
        String backupType = backupContext.get(ContextConstants.BACKUP_TYPE);
        PolicyBo policyBo =
                JSONObject.toBean(backupContext.get(CopyPropertiesKeyConstant.KEY_BACKUP_POLICY), PolicyBo.class);
        String userId = backupContext.get(Constants.CURRENT_OPERATE_USER_ID);
        if (isNeedBackupToFull(jobStatus, backupType, copyFormat)) {
            log.info("Full Backup or E6000 increment backup failed this time, convert next backup type to Full!");
            setNextBackupToFull(protectedObject.getResourceId());
        }

        String resourceType = protectedObject.getSubType();
        BackupInterceptorProvider interceptor =
                providerManager.findProvider(BackupInterceptorProvider.class, resourceType);
        if (interceptor != null) {
            log.info("Backup interceptor for {} is provided, ready to postProcess!", resourceType);
            PostBackupTask postBackupTask = new PostBackupTask();
            postBackupTask.setBackupType(BackupTypeEnum.valueOf(backupType.toUpperCase(Locale.ENGLISH)));
            postBackupTask.setProtectedObject(protectedObject);
            postBackupTask.setJobStatus(jobStatus);
            postBackupTask.setCopyInfo(copyInfo);
            postBackupTask.setPolicyBo(policyBo);
            postBackupTask.setUserId(userId);
            interceptor.finalize(postBackupTask);
        }
    }

    /**
     * 下次备份需要转换为全量备份的情况
     * 必要条件：任务状态失败且是快照
     *         所有形态下的全量备份失败
     *         E6000形态下的增量备份失败
     *
     * @param jobStatus 任务状态
     * @param backupType 备份类型
     * @param copyFormat 副本格式
     * @return 下次备份是否需要转全量
     */
    private Boolean isNeedBackupToFull(ProviderJobStatusEnum jobStatus, String backupType, int copyFormat) {
        // 任务状态失败，并且是快照格式的
        if (jobStatus.checkSuccess() || copyFormat != CopyFormatEnum.INNER_SNAPSHOT.getCopyFormat()) {
            return false;
        }
        // 所有形态下的全量备份失败，要转全量
        if (BackupTypeEnum.FULL.name().toLowerCase(Locale.ROOT).equals(backupType)) {
            return true;
        }
        // E6000 形态下，增量备份失败也要转全量
        return deployTypeService.isPacific() && INCREMENT_BACKUP_TYPE_SET.contains(backupType);
    }

    private void setNextBackupToFull(String resourceId) {
        NextBackupChangeCauseEnum toFullReason = deployTypeService.isPacific()
            ? NextBackupChangeCauseEnum.E6000_BACKUP_FAILED
            : NextBackupChangeCauseEnum.BIGDATA_PLUGIN_TO_FULL_LABEL;
        NextBackupModifyReq nextBackupModifyReq = NextBackupModifyReq.build(resourceId, toFullReason);
        resourceService.modifyNextBackup(nextBackupModifyReq);
    }
}
