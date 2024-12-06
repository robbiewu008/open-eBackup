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
package openbackup.data.access.framework.protection.controller;

import static openbackup.data.access.framework.core.common.constants.TopicConstants.REPLICATION_COMPLETED_TOPIC;

import com.huawei.oceanprotect.base.cluster.sdk.service.StorageUnitService;
import com.huawei.oceanprotect.sla.sdk.enums.ReplicationMode;
import com.huawei.oceanprotect.sla.sdk.enums.TargetTypeEnum;
import com.huawei.oceanprotect.system.base.user.service.ResourceSetApi;
import com.huawei.oceanprotect.system.base.user.service.UserService;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.JsonNode;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.client.sdk.api.framework.dme.DmeCopyInfo;
import openbackup.data.access.client.sdk.api.framework.dme.DmeUnifiedRestApi;
import openbackup.data.access.framework.copy.mng.constant.CopyPropertiesKeyConstant;
import openbackup.data.access.framework.copy.mng.enums.CopyTypeEnum;
import openbackup.data.access.framework.core.common.constants.CopyConstants;
import openbackup.data.access.framework.core.common.constants.CopyInfoConstants;
import openbackup.data.access.framework.core.common.enums.CopyIndexStatus;
import openbackup.data.access.framework.core.common.util.CopyInfoBuilder;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.common.enums.SpecifiedScope;
import openbackup.data.access.framework.protection.common.enums.TimeRangeWeekEnum;
import openbackup.data.access.framework.protection.dto.ArchiveRequestDto;
import openbackup.data.access.framework.protection.service.archive.ArchiveCopyProvider;
import openbackup.data.access.framework.protection.service.archive.ArchiveJobService;
import openbackup.data.access.framework.protection.service.archive.CopyDependencyQueryResponse;
import openbackup.data.access.framework.protection.service.archive.DefaultArchiveCopyProvider;
import openbackup.data.access.framework.protection.service.quota.UserQuotaManager;
import openbackup.data.access.framework.protection.service.replication.ReplicationService;
import openbackup.data.access.framework.protection.service.replication.UnifiedReplicationProvider;
import openbackup.data.access.framework.protectobject.service.ProjectObjectService;
import openbackup.data.access.framework.restore.dto.RestoreRequestDto;
import openbackup.data.protection.access.provider.sdk.archive.ArchiveRequest;
import openbackup.data.protection.access.provider.sdk.base.Filter;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.copy.CopyReplicationImport;
import openbackup.data.protection.access.provider.sdk.copy.ReplicationOriginCopyDuration;
import openbackup.data.protection.access.provider.sdk.replication.ReplicationProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.RestoreObject;
import openbackup.data.protection.access.provider.sdk.restore.RestoreProvider;
import openbackup.data.protection.access.provider.sdk.restore.RestoreRequest;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.enums.RetentionTypeEnum;
import openbackup.system.base.common.enums.TimeUnitEnum;
import openbackup.system.base.common.enums.WormValidityTypeEnum;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.anti.api.AntiRansomwareApi;
import openbackup.system.base.sdk.anti.model.AntiRansomwareScheduleRes;
import openbackup.system.base.sdk.auth.AuthRestApi;
import openbackup.system.base.sdk.auth.UserInnerResponse;
import openbackup.system.base.sdk.cluster.model.StorageUnitVo;
import openbackup.system.base.sdk.common.model.UuidObject;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.copy.model.CopyInfo;
import openbackup.system.base.sdk.copy.model.CopyStatus;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.job.model.request.CreateJobRequest;
import openbackup.system.base.sdk.protection.model.PolicyBo;
import openbackup.system.base.sdk.protection.model.RetentionBo;
import openbackup.system.base.sdk.repository.api.BackupStorageApi;
import openbackup.system.base.sdk.repository.model.NasDistributionStorageDetail;
import openbackup.system.base.sdk.resource.model.ResourceEntity;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.security.exterattack.ExterAttack;
import openbackup.system.base.security.permission.Permission;
import openbackup.system.base.util.BeanTools;
import openbackup.system.base.util.MessageTemplate;
import openbackup.system.base.util.OpServiceUtil;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.lang3.StringUtils;
import org.apache.logging.log4j.util.Strings;
import org.springframework.beans.BeanUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.validation.annotation.Validated;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.ResponseBody;
import org.springframework.web.bind.annotation.RestController;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Calendar;
import java.util.Collections;
import java.util.Date;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;

import javax.validation.constraints.NotNull;

/**
 * 功能描述
 *
 */
@Slf4j
@RestController
public class AccessPointController {
    private static final String LOCAL = "Local";

    private static final String NOT_EXIST = "NOT_EXIST";

    private static final String EXIST = "EXIST";

    private static final long SECONDS_MILLI = 1000L;

    private static final String REPLICATION = "replication";

    private static final String DEFAULT_POOL_ID = "0";

    /**
     * 反向、级联复制
     */
    private static final List<String> REVERSE_CASCADED_REPLICATION = Collections.unmodifiableList(
        Arrays.asList(CopyGeneratedByEnum.BY_REVERSE_REPLICATION.value(),
            CopyGeneratedByEnum.BY_CASCADED_REPLICATION.value()));

    @Autowired
    private CopyRestApi copyRestApi;

    @Autowired
    private DmeUnifiedRestApi dmeUnifiedRestApi;

    @Autowired
    private ProviderManager providerManager;

    @Autowired
    private AuthRestApi authRestApi;

    @Autowired
    private UnifiedReplicationProvider unifiedReplicationProvider;

    @Autowired
    private ArchiveJobService archiveJobService;

    @Autowired
    private MessageTemplate<String> messageTemplate;

    @Autowired
    private ResourceService resourceService;

    @Autowired
    private UserQuotaManager userQuotaManager;

    @Autowired
    private UserService userService;

    @Autowired
    private ReplicationService replicationService;

    @Autowired
    private StorageUnitService storageUnitService;

    @Autowired
    private ProjectObjectService projectObjectService;

    @Autowired
    private ResourceSetApi resourceSetApi;

    @Autowired
    private BackupStorageApi backupStorageApi;

    @Autowired
    private AntiRansomwareApi antiRansomwareApi;

    @Autowired
    private DefaultArchiveCopyProvider defaultArchiveCopyProvider;

    /**
     * 生成恢复任务 接口：/v1/internal/restore-task/action/create 方法：Post
     *
     * @param restoreRequestDto 恢复请求
     * @return 任务json
     * @throws JsonProcessingException json转换异常
     */
    @ExterAttack
    @PostMapping("/v1/internal/restore-task/action/create")
    @ResponseBody
    public String createRestoreTask(@RequestBody RestoreRequestDto restoreRequestDto) throws JsonProcessingException {
        Copy copy = copyRestApi.queryCopyByID(restoreRequestDto.getCopyId(), false);
        RestoreObject restoreObject = new RestoreObject();
        restoreObject.setCopyId(restoreRequestDto.getCopyId());
        restoreObject.setObjectType(restoreRequestDto.getObjectType());
        // Oracle SCN和时间点恢复在PM上没有副本
        if (copy != null) {
            restoreObject.setCopyGeneratedBy(copy.getGeneratedBy());
        } else {
            copy = new Copy();
            copy.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
            restoreObject.setCopyGeneratedBy(copy.getGeneratedBy());
        }
        RestoreRequest restoreRequest = new RestoreRequest();
        BeanUtils.copyProperties(restoreRequestDto, restoreRequest, "filters");
        if (CollectionUtils.isNotEmpty(restoreRequestDto.getFilters())) {
            // filter类型不一致，需要单独做转换
            final List<Filter> filterList = restoreRequestDto.getFilters()
                .stream()
                .map(filter -> BeanTools.copy(filter, Filter::new))
                .collect(Collectors.toList());
            restoreRequest.setFilters(filterList);
        } else {
            restoreRequest.setFilters(Collections.emptyList());
        }
        return providerManager.findProvider(RestoreProvider.class, restoreObject).createRestoreTask(restoreRequest);
    }

    /**
     * import replication copy
     *
     * @param importParam import param
     * @return result
     */
    @ExterAttack
    @PostMapping("/v1/internal/copies/action/import")
    public boolean importReplicationCopy(@RequestBody @Validated CopyReplicationImport importParam) {
        log.info("DME import copy,properties:{}, time:{}, esn: {}, backupSoftwareEsn: {}", importParam.getProperties(),
            importParam.getTimestamp(), importParam.getEsn(), importParam.getBackupSoftwareEsn());
        JSONObject properties = JSONObject.fromObject(importParam.getProperties());
        String chainId = properties.getString("chain_id");
        long timestamp = importParam.getTimestamp();
        JSONObject metadata = JSONObject.fromObject(importParam.getMetadata());
        JSONObject resource = metadata.getJSONObject("resource");
        ResourceEntity resourceEntity = resource.toBean(ResourceEntity.class);
        String resourceSubType = resourceEntity.getSubType();
        ReplicationProvider replicationProvider = providerManager.findProviderOrDefault(ReplicationProvider.class,
            resourceSubType, unifiedReplicationProvider);
        boolean isExist = replicationProvider.checkCopyWhetherExist(chainId, timestamp);
        if (isExist) {
            return false;
        }

        Date generateTime = replicationService.getGenerateTime(importParam.getGeneratedTime());
        String originCopyTimeStamp = properties.getString("originCopyTimeStamp");
        boolean isOriginCopyWorm = properties.getInt("wormCopy", 0) == 1;

        // 复制副本入库存储id
        CopyInfoBo copy = createCopyInfoBo(importParam, generateTime, isOriginCopyWorm, originCopyTimeStamp);
        replicationProvider.buildCopyProperties(copy, importParam, resourceEntity);
        log.debug("build copy info: {} success, generated by: {}", copy.getUuid(), copy.getGeneratedBy());
        saveCopyAndUpdateQuota(copy, properties, resourceEntity);

        resourceSetApi.createCopyResourceSetRelation(copy.getUuid(), resourceEntity.getUuid(), copy.getUserId());
        // 发送复制完成信息
        JSONObject copyObject = JSONObject.fromObject(copy);
        log.info("sla worm is: {}", isOriginCopyWorm);
        copyObject.put("is_origin_copy_worm", isOriginCopyWorm);
        messageTemplate.send(REPLICATION_COMPLETED_TOPIC, copyObject.toString());

        // 如果开了防勒索要检测 license
        AntiRansomwareScheduleRes targetAntiRansomwareSchedule = replicationService.getAntiRansomwareSchedule(
            copy.getResourceId());
        replicationService.checkDetectionLicense(isOriginCopyWorm, targetAntiRansomwareSchedule);
        return true;
    }

    private void buildCopyIndexStatus(CopyInfoBo copy) {
        // FC/HCS 反向复制、级联复制副本不支持索引
        log.info("replication build copy, resourceSubType: {}, generateBy: {}", copy.getResourceSubType(),
            copy.getGeneratedBy());
        if (CopyInfoConstants.UNSUPPORTED_INDEX_TYPES.contains(copy.getResourceSubType())
            && REVERSE_CASCADED_REPLICATION.contains(copy.getGeneratedBy())) {
            copy.setIndexed(CopyIndexStatus.UNSUPPORT.getIndexStaus());
        }
    }

    private boolean resetUserId(String userId, CopyInfoBo copy) {
        log.info("Start to resetUserId.UserId:{}", userId);
        if (StringUtils.isEmpty(userId)) {
            return false;
        }
        UserInnerResponse userInnerResponse;
        try {
            userInnerResponse = userService.getUserInfoByUserId(userId);
        } catch (LegoCheckedException e) {
            log.warn("Do not have user:{}", userId);
            return false;
        }
        if (VerifyUtil.isEmpty(userInnerResponse)) {
            log.info("The user is not exist!UserId:{}", userId);
            return false;
        }
        copy.setUserId(userId);
        log.info("End to resetUserId.UserId:{}", userId);
        return true;
    }

    private void resetIntraCopyUserId(JSONObject policy, CopyInfoBo copy, String userId) {
        if (!policy.containsKey("ext_parameters")) {
            return;
        }
        JSONObject ext = policy.getJSONObject("ext_parameters");
        if (!ext.containsKey("replication_target_mode")) {
            return;
        }
        boolean isIntra = ReplicationMode.INTRA.getValue() == ext.getInt("replication_target_mode");
        if (isIntra) {
            log.info("Intra replication copy reset userId, uuid: {}, original userId: {}, new userId: {}.",
                copy.getUuid(), copy.getUserId(), userId);
            copy.setUserId(userId);
        }
    }

    private void initCopySlaInfo(JSONObject metadata, CopyInfoBo copy) {
        JSONObject sla = metadata.getJSONObject("sla");
        if (sla == null) {
            log.info("Reverse replication copy: {} has no sla info.", copy.getUuid());
            return;
        }
        copy.setSlaName(sla.getString("name"));
        copy.setSlaProperties(sla.toString());
    }

    private CopyInfoBo createCopyInfoBo(CopyReplicationImport importParam, Date generateTime, boolean isOriginCopyWorm,
        String originCopyTimeStamp) {
        JSONObject properties = JSONObject.fromObject(importParam.getProperties());
        CopyInfoBo copy = new CopyInfoBo();
        copy.setChainId(properties.getString("chain_id"));
        copy.setGeneration(0);
        copy.setParentCopyUuid(null);
        copy.setStatus(CopyStatus.NORMAL.getValue());
        copy.setIndexed(CopyIndexStatus.UNINDEXED.getIndexStaus());
        copy.setJobType(JobTypeEnum.COPY_REPLICATION.getValue());
        copy.setGeneratedTime(Constants.SIMPLE_DATE_FORMAT.format(generateTime));
        copy.setDisplayTimestamp(Constants.SIMPLE_DATE_FORMAT.format(generateTime));
        copy.setTimestamp(String.valueOf(generateTime.getTime() * SECONDS_MILLI));
        if (StringUtils.isNotBlank(originCopyTimeStamp)) {
            copy.setOriginCopyTimeStamp(String.valueOf(Long.parseLong(originCopyTimeStamp) * SECONDS_MILLI));
        } else {
            copy.setOriginCopyTimeStamp(copy.getGeneratedTime());
        }
        JSONObject metadata = JSONObject.fromObject(importParam.getMetadata());
        JSONObject resource = metadata.getJSONObject("resource");
        copy.setResourceProperties(resource.toString());
        if (VerifyUtil.isEmpty(importParam.getBackupSoftwareEsn())) {
            copy.setDeviceEsn(importParam.getEsn());
        } else {
            copy.setDeviceEsn(importParam.getBackupSoftwareEsn());
        }
        copy.setOriginBackupId(properties.getString("origin_backup_id"));
        copy.setIsStorageSnapshot(Optional.ofNullable(resource.getJSONObject("ext_parameters"))
            .orElseGet(JSONObject::new)
            .getBoolean("storage_snapshot_flag", false));
        log.info("origin backup id from dme: {}", copy.getOriginBackupId());

        // 若集群中存在相同的用户id的用户 则重新赋予用户id
        copy.setUserId(authRestApi.queryUserInfoByName(metadata.getString("username")).getId());
        // 判断是否域内复制，是则更新副本userId
        if (metadata.containsKey("replication_policy")) {
            JSONObject replicationPolicy = metadata.getJSONObject("replication_policy");
            ResourceEntity resourceEntity = resource.toBean(ResourceEntity.class);
            resetIntraCopyUserId(replicationPolicy, copy, resourceEntity.getUserId());
        }

        boolean isReverseCopy = properties.getBoolean("reverse_copy", false);
        int replicateCount = properties.getInt("replicate_count", 1);
        copy.setGeneratedBy(getGeneratedBy(isReverseCopy, replicateCount));

        initCopyStorageId(properties, copy);
        copy.setStorageUnitId(getStorageUnitId(properties, importParam.getEsn()));

        ResourceEntity resourceEntity = resource.toBean(ResourceEntity.class);
        initCopyResourceInfo(resourceEntity, copy);

        initCopySlaInfo(metadata, copy);
        initCopyLocation(copy);
        // FC/HCS 反向复制、级联复制副本不支持索引
        buildCopyIndexStatus(copy);

        // 计算副本保留时间
        buildCopyRetentionInfo(copy, generateTime, isOriginCopyWorm, importParam, resourceEntity.getUuid());
        return copy;
    }

    /**
     * 初始化location
     *
     * @param copy copy
     */
    private void initCopyLocation(CopyInfoBo copy) {
        copy.setLocation(LOCAL);
        // 如果是开了并行存储开关的备份存储单元组，位置信息显示备份存储单元组的名称，其他场景和之前一样
        if (!VerifyUtil.isEmpty(copy.getStorageId())) {
            NasDistributionStorageDetail storageUnitGroup = backupStorageApi.getDetail(copy.getStorageId());
            if (storageUnitGroup.isHasEnableParallelStorage()) {
                copy.setLocation(storageUnitGroup.getName());
                return;
            }
        }
        // 默认为local,存储单元不为空，默认为存储单元名称
        storageUnitService.getStorageUnitById(copy.getStorageUnitId()).ifPresent(storageUnitVo -> {
            copy.setLocation(storageUnitVo.getName());
        });
    }

    private void initCopyStorageId(JSONObject properties, CopyInfoBo copy) {
        String storageId = getStorageId(properties);
        if (!VerifyUtil.isEmpty(storageId)) {
            // 主端的复制策略有存储库id，归档时存储库id不能为空
            log.info("Storage id for replication policy configuration,storageId:{}", storageId);
            copy.setStorageId(storageId);
        }
    }

    private String getGeneratedBy(boolean isReverseCopy, int replicateCount) {
        if (isReverseCopy) {
            return CopyGeneratedByEnum.BY_REVERSE_REPLICATION.value();
        } else if (replicateCount > 1) {
            return CopyGeneratedByEnum.BY_CASCADED_REPLICATION.value();
        } else {
            return CopyGeneratedByEnum.BY_REPLICATED.value();
        }
    }

    private void saveCopyAndUpdateQuota(CopyInfoBo copy, JSONObject properties, ResourceEntity resourceEntity) {
        String backupId = properties.getString("backup_id");
        resetUserId(resourceEntity.getUserId(), copy);

        CopyInfo copyInfo = new CopyInfo();
        BeanUtils.copyProperties(copy, copyInfo);
        if (VerifyUtil.isEmpty(copyInfo.getResourceLocation())) {
            copyInfo.setResourceLocation(Strings.EMPTY);
        }
        UuidObject uuidObject = copyRestApi.saveCopy(copyInfo);

        // 增加配额
        userQuotaManager.increaseUsedQuota(backupId, copyInfo);
        copy.setUuid(uuidObject.getUuid());
        log.debug("save copy info success.");
    }

    private String getStorageUnitId(JSONObject properties, String deviceEsn) {
        List<StorageUnitVo> unitVos = storageUnitService.getStorageUnitByEsn(deviceEsn);
        if (VerifyUtil.isEmpty(unitVos)) {
            log.warn("Can't find storage unit on device(esn:{})", deviceEsn);
            return StringUtils.EMPTY;
        }
        String poolId = properties.getString("poolId", DEFAULT_POOL_ID);
        return unitVos.stream()
            .filter(storageUnitVo -> poolId.equals(storageUnitVo.getPoolId()))
            .findAny()
            .orElseGet(() -> {
                log.warn("Can't find storage unit on device(esn:{}, poolId: {})", deviceEsn, poolId);
                StorageUnitVo storageUnitVo = new StorageUnitVo();
                storageUnitVo.setId(StringUtils.EMPTY);
                return storageUnitVo;
            })
            .getId();
    }

    private String getStorageId(JSONObject properties) {
        String backupId = properties.getString(CopyConstants.BACKUP_ID);
        try {
            DmeCopyInfo dmeCopyInfo = dmeUnifiedRestApi.getCopyInfo(backupId);
            if (VerifyUtil.isEmpty(dmeCopyInfo)) {
                return Strings.EMPTY;
            }
            Map<String, Object> extendInfo = dmeCopyInfo.getExtendInfo();
            if (VerifyUtil.isEmpty(extendInfo)) {
                return Strings.EMPTY;
            }
            return Optional.ofNullable(extendInfo.get(CopyPropertiesKeyConstant.KEY_BACKUP_REPOSITORY_ID))
                .orElse("")
                .toString();
        } catch (LegoUncheckedException | LegoCheckedException e) {
            // 发送失败 ,添加事件
            log.error("get dme copy info error:", ExceptionUtil.getErrorMessage(e));
            return Strings.EMPTY;
        }
    }

    private void buildCopyRetentionInfo(CopyInfoBo copy, Date replicationDate, boolean isOriginCopyWorm,
        CopyReplicationImport importParam, String resourceId) {
        log.debug("[replication task] originCopyWorm is: {}, copy is: {}", isOriginCopyWorm, copy.getUuid());
        JSONObject metadata = JSONObject.fromObject(importParam.getMetadata());
        PolicyBo replicationPolicy = metadata.getBean("replication_policy", PolicyBo.class);
        long originTimestamp = importParam.getTimestamp() * SECONDS_MILLI;
        JsonNode replicationTargetType = replicationPolicy.getExtParameters().get("replication_target_type");
        log.debug("Replication policy type: {}, id: {}", replicationPolicy.getType(), replicationPolicy.getUuid());
        boolean existWormPolicy = antiRansomwareApi.isExistWormPolicyByResourceId(resourceId);
        log.info("Resource:{} is exist worm policy:{}", resourceId, existWormPolicy);
        RetentionBo repRetention = replicationPolicy.getRetention();
        if (replicationTargetType != null && replicationTargetType.asInt() == TargetTypeEnum.SPECIFIED_COPY.getType()) {
            log.debug("Replication copy of the specified date, timestamp: {}", originTimestamp);
            JSONObject jsonObject = JSONObject.fromObject(importParam.getProperties());
            boolean isMatchMonthPolicy = jsonObject.getBoolean("isMatchMonthPolicy", false);
            handleCopy(copy, replicationPolicy, isMatchMonthPolicy, originTimestamp);
            // 原副本为worm格式,从端副本过期时间取备份副本和复制策略中保留时间长的
            if (isOriginCopyWorm) {
                fillOriginCopyWorm(copy, importParam, replicationDate.getTime(), repRetention);
            }
            if (existWormPolicy) {
                fillWormPolicyCopyWorm(copy, replicationDate.getTime());
            }
            return;
        }

        if (RetentionTypeEnum.TEMPORARY.getType().equals(repRetention.getRetentionType())) {
            CopyInfoBuilder.buildRetentionInfo(copy, replicationPolicy, replicationDate.getTime());
            // 原副本为worm格式,从端副本过期时间取备份副本和复制策略中保留时间长的
            if (isOriginCopyWorm) {
                fillOriginCopyWorm(copy, importParam, replicationDate.getTime(), repRetention);
            }
            if (existWormPolicy) {
                fillWormPolicyCopyWorm(copy, replicationDate.getTime());
            }
        } else {
            // 复制策略为永久保留，无须与原副本保留时间比较
            copy.setRetentionType(RetentionTypeEnum.PERMANENT.getType());
            copy.setRetentionDuration(0);
            copy.setExpirationTime(null);
        }
    }

    private void fillWormPolicyCopyWorm(CopyInfoBo copy, long replicationTimestamp) {
        copy.setWormValidityType(WormValidityTypeEnum.COPY_RETENTION_TIME_CONSISTENT.getType());
        copy.setWormRetentionDuration(copy.getRetentionDuration());
        copy.setWormDurationUnit(copy.getDurationUnit());
        if (RetentionTypeEnum.TEMPORARY.getType().equals(copy.getRetentionType())) {
            Date backupWormExpirationTime = CopyInfoBuilder.computeExpirationTime(replicationTimestamp,
                TimeUnitEnum.getByUnit(copy.getWormDurationUnit()), copy.getWormRetentionDuration());
            copy.setWormExpirationTime(backupWormExpirationTime);
        }
    }

    private void fillOriginCopyWorm(CopyInfoBo copy, CopyReplicationImport importParam, long replicationTimestamp,
        RetentionBo repRetention) {
        ReplicationOriginCopyDuration originCopyDuration = importParam.getOriginCopyDuration();
        int originCopyRetentionType = originCopyDuration.getRetentionType();
        log.info("check origin copy worm, origin copy retention is: {}, unit is: {}, retention type is: {}",
            originCopyDuration.getRetentionDuration(), originCopyDuration.getDurationUnit(),
            originCopyRetentionType);
        Date backupWormExpirationTime = null;
        int wormRetentionDuration = originCopyDuration.getWormRetentionDuration();
        String wormDurationUnit = originCopyDuration.getWormDurationUnit();
        int wormValidityType = originCopyDuration.getWormValidityType();
        if (StringUtils.isEmpty(wormDurationUnit)) {
            wormRetentionDuration = originCopyDuration.getRetentionDuration();
            wormDurationUnit = originCopyDuration.getDurationUnit();
        }
        // 如果备份副本是永久保留并且开启了已副本保持一致场景下，设置worm过期时间为复制副本过期时间
        if (originCopyDuration.getRetentionType() == RetentionTypeEnum.PERMANENT.getType()
            && wormValidityType == WormValidityTypeEnum.COPY_RETENTION_TIME_CONSISTENT.getType()) {
            wormRetentionDuration = repRetention.getRetentionDuration();
            wormDurationUnit = repRetention.getDurationUnit();
        }
        copy.setWormValidityType(wormValidityType);
        copy.setWormRetentionDuration(wormRetentionDuration);
        copy.setWormDurationUnit(wormDurationUnit);
        if (StringUtils.isNotEmpty(wormDurationUnit)) {
            backupWormExpirationTime = CopyInfoBuilder.computeExpirationTime(replicationTimestamp,
                TimeUnitEnum.getByUnit(wormDurationUnit), wormRetentionDuration);
            copy.setWormExpirationTime(backupWormExpirationTime);
        }
        // 过期时间应该取原副本过期时间的情况：原副本为worm格式,原副本为永久保留或原副本保留时间大于复制策略保留时间
        if (!RetentionTypeEnum.TEMPORARY.getType().equals(originCopyRetentionType) || copy.getExpirationTime()
            .before(backupWormExpirationTime)) {
            copy.setWormExpirationTime(backupWormExpirationTime);
        }
    }

    private void handleCopy(CopyInfoBo copy, PolicyBo policy, boolean isMatchMonthPolicy, long originTimestamp) {
        log.info("[REPLICATION_TASK] Get replication policy uuid={}, timestamp:{}.", policy.getUuid(), originTimestamp);
        copy.setRetentionType(RetentionTypeEnum.TEMPORARY.getType());

        Date generateTime = new Date(originTimestamp);
        Calendar generateDate = Calendar.getInstance();
        generateDate.setTime(generateTime);
        int month = generateDate.get(Calendar.MONTH) + 1;
        int dayOfWeek = generateDate.get(Calendar.DAY_OF_WEEK);
        log.info("[REPLICATION_TASK] Get copy generate month is {}, day 0f week is {}", month, dayOfWeek);
        TimeRangeWeekEnum timeRangeWeekEnum = TimeRangeWeekEnum.getByDayOfWeek(dayOfWeek);
        List<SpecifiedScope> specifiedScopes = new ArrayList<>();
        for (JsonNode jsonNode : policy.getExtParameters().get("specified_scope")) {
            String copyType = jsonNode.get("copy_type").asText();
            CopyTypeEnum typeEnum = CopyTypeEnum.getByType(copyType);
            JsonNode generateTimeRange = jsonNode.get("generate_time_range");
            int retentionDuration = jsonNode.get("retention_duration").asInt();
            if (CopyTypeEnum.YEAR.equals(typeEnum) && month == generateTimeRange.asInt()) {
                log.debug("match year policy");
                specifiedScopes.add(new SpecifiedScope(typeEnum, retentionDuration));
                continue;
            }
            if (CopyTypeEnum.WEEK.equals(typeEnum) && timeRangeWeekEnum.getValue().equals(generateTimeRange.asText())) {
                log.debug("match week policy");
                specifiedScopes.add(new SpecifiedScope(typeEnum, retentionDuration));
                continue;
            }
            if (CopyTypeEnum.MONTH.equals(typeEnum) && isMatchMonthPolicy) {
                log.debug("match month policy");
                specifiedScopes.add(new SpecifiedScope(typeEnum, retentionDuration));
            }
        }

        if (specifiedScopes.isEmpty()) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "Can not find retention policy");
        }
        log.info("[REPLICATION_TASK] Get specified scopes(nums: {}) success.", specifiedScopes.size());
        specifiedScopes.sort((o1, o2) -> o2.getDays() - o1.getDays());
        SpecifiedScope specifiedScope = specifiedScopes.get(0);
        copy.setDurationUnit(specifiedScope.getTimeUnit().getUnit());
        copy.setRetentionDuration(specifiedScope.getRetentionDuration());
        log.info("[REPLICATION_TASK] Set retention duration success.");
        copy.setExpirationTime(CopyInfoBuilder.computeExpirationTime(System.currentTimeMillis(),
            TimeUnitEnum.getByUnit(copy.getDurationUnit()), copy.getRetentionDuration()));
    }

    private void initCopyResourceInfo(ResourceEntity resourceEntity, CopyInfoBo copy) {
        copy.setResourceId(resourceEntity.getUuid());
        copy.setResourceName(resourceEntity.getName());
        copy.setResourceEnvironmentIp(resourceEntity.getEnvironmentEndPoint());
        copy.setResourceEnvironmentName(resourceEntity.getEnvironmentName());
        if (ResourceSubTypeEnum.IMPORT_COPY.getType().equals(resourceEntity.getSubType())) {
            copy.setResourceLocation(LOCAL);
        } else {
            copy.setResourceLocation(resourceEntity.getPath());
        }
        if (resourceService.getResourceById(resourceEntity.getUuid()).isPresent()) {
            copy.setResourceStatus(EXIST);
        } else {
            copy.setResourceStatus(NOT_EXIST);
        }
        copy.setResourceType(resourceEntity.getType());
        copy.setResourceSubType(resourceEntity.getSubType());
    }

    /**
     * 生成归档任务 接口：/v1/internal/archive-task/action/create 方法：Post
     *
     * @param archiveRequest 恢复请求
     * @return 任务json
     */
    @ExterAttack
    @PostMapping("/v1/internal/archive-task/action/create")
    @ResponseBody
    public String createArchiveTask(@RequestBody ArchiveRequestDto archiveRequest) {
        ArchiveRequest request = new ArchiveRequest();
        BeanUtils.copyProperties(archiveRequest, request);
        final CreateJobRequest createJobRequest = archiveJobService.buildJobRequest(request);
        return JSONObject.fromObject(createJobRequest).toString();
    }

    /**
     * 手动复制时，校验存储单元是否满足限制
     *
     * @param storageType storageType
     * @param storageId storageId
     * @param resourceId resourceId
     */
    @ExterAttack
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_RD_ADMIN},
        enableCheckAuth = false, checkRolePermission = true)
    @GetMapping("/v1/manual-replication/check-units")
    public void checkBeforeManualReplication(@NotNull @RequestParam("storageType") String storageType,
            @NotNull @RequestParam("storageId") String storageId,
            @NotNull @RequestParam("resourceId") String resourceId) {
        projectObjectService.checkBeforeManualReplication(storageType, storageId, resourceId);
    }

    /**
     * 手动复制时，校验存储单元是否满足限制(内部接口，给)
     *
     * @param clusterId clusterId
     * @param storageType storageType
     * @param storageId storageId
     * @param resourceId resourceId
     */
    @ExterAttack
    @GetMapping("/v1/internal/replication/check-manual-replication")
    public void checkBeforeManualReplicationInternal(@NotNull @RequestParam("clusterId") Integer clusterId,
            @NotNull @RequestParam("storageType") String storageType,
            @NotNull @RequestParam("storageId") String storageId,
            @NotNull @RequestParam("resourceId") String resourceId) {
        replicationService.checkManualRep(clusterId, storageType, storageId, resourceId);
    }

    /**
     * 判断是否是op服务化部署场景
     *
     * @return 是否是op服务化部署场景
     */
    @ExterAttack
    @GetMapping("/v1/internal/replication/is-op-service")
    public boolean isOpService() {
        log.info("Start to get hcs service!");
        return OpServiceUtil.isHcsService();
    }

    /**
     * 获取副本依赖链，不包含全量副本
     *
     * @param copyUuidList copyUuidList
     * @return 副本依赖链
     */
    @ExterAttack
    @GetMapping("/v1/internal/archive")
    public List<CopyDependencyQueryResponse> queryDependencyCopies(@NotNull @RequestParam List<String> copyUuidList) {
        log.info("Start to query dependency copies of copies:{}.", copyUuidList.size());
        List<Copy> copyList = copyUuidList.stream()
            .map(uuid -> copyRestApi.queryCopyByID(uuid))
            .collect(Collectors.toList());
        ArchiveCopyProvider archiveCopyProvider = providerManager.findProviderOrDefault(ArchiveCopyProvider.class,
            copyList.get(0).getResourceSubType(), defaultArchiveCopyProvider);
        return archiveCopyProvider.queryCopyDependenceChain(copyList);
    }
}
