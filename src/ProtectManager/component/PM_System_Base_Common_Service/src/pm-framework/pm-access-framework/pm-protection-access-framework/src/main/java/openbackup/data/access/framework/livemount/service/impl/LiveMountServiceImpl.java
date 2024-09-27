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
package openbackup.data.access.framework.livemount.service.impl;

import com.huawei.oceanprotect.base.cluster.sdk.enums.StorageUnitTypeEnum;
import com.huawei.oceanprotect.base.cluster.sdk.service.StorageUnitService;
import com.huawei.oceanprotect.job.constants.JobExtendInfoKeys;
import com.huawei.oceanprotect.job.constants.JobPayloadKeys;
import com.huawei.oceanprotect.job.sdk.JobService;
import com.huawei.oceanprotect.system.base.user.common.enums.ResourceSetScopeModuleEnum;
import com.huawei.oceanprotect.system.base.user.entity.ResourceSetResourceBo;
import com.huawei.oceanprotect.system.base.user.service.ResourceSetApi;

import com.baomidou.mybatisplus.core.conditions.query.LambdaQueryWrapper;
import com.baomidou.mybatisplus.core.conditions.query.QueryWrapper;
import com.baomidou.mybatisplus.core.conditions.update.LambdaUpdateWrapper;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.copy.mng.service.CopyAuthVerifyService;
import openbackup.data.access.framework.copy.mng.service.CopyService;
import openbackup.data.access.framework.core.dao.CopyMapper;
import openbackup.data.access.framework.core.entity.CopiesEntity;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.livemount.TopicConstants;
import openbackup.data.access.framework.livemount.common.LiveMountOperateType;
import openbackup.data.access.framework.livemount.common.LiveMountRestApi;
import openbackup.data.access.framework.livemount.common.constants.LiveMountConstants;
import openbackup.data.access.framework.livemount.common.enums.CopyDataSelection;
import openbackup.data.access.framework.livemount.common.enums.OperationEnums;
import openbackup.data.access.framework.livemount.common.enums.ScheduledType;
import openbackup.data.access.framework.livemount.common.model.LiveMountCreateCheckParam;
import openbackup.data.access.framework.livemount.common.model.LiveMountModel;
import openbackup.data.access.framework.livemount.common.model.LiveMountObject;
import openbackup.data.access.framework.livemount.common.model.UnmountExtendParam;
import openbackup.data.access.framework.livemount.controller.livemount.model.LiveMountEnableStatus;
import openbackup.data.access.framework.livemount.controller.livemount.model.LiveMountMigrateRequest;
import openbackup.data.access.framework.livemount.controller.livemount.model.LiveMountParam;
import openbackup.data.access.framework.livemount.controller.livemount.model.LiveMountStatus;
import openbackup.data.access.framework.livemount.dao.LiveMountEntityDao;
import openbackup.data.access.framework.livemount.dao.LiveMountModelDao;
import openbackup.data.access.framework.livemount.dao.LiveMountPolicyEntityDao;
import openbackup.data.access.framework.livemount.entity.LiveMountPolicyEntity;
import openbackup.data.access.framework.livemount.provider.DefaultLiveMountServiceProvider;
import openbackup.data.access.framework.livemount.provider.LiveMountFlowService;
import openbackup.data.access.framework.livemount.provider.LiveMountServiceProvider;
import openbackup.data.access.framework.livemount.service.LiveMountService;
import openbackup.data.access.framework.livemount.service.PolicyService;
import openbackup.data.protection.access.provider.sdk.livemount.LiveMountInterceptorProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceBase;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.constants.AuthOperationEnum;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.livemount.LiveMountEntity;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.NumberUtil;
import openbackup.system.base.common.utils.ValidateUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.validator.constants.RegexpConstants;
import openbackup.system.base.query.PageQueryParam;
import openbackup.system.base.query.PageQueryService;
import openbackup.system.base.sdk.PageQueryRestApi;
import openbackup.system.base.sdk.SystemSpecificationService;
import openbackup.system.base.sdk.cluster.model.StorageUnitVo;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.copy.model.CopyResourceSummary;
import openbackup.system.base.sdk.copy.model.CopyStatus;
import openbackup.system.base.sdk.livemount.model.Identity;
import openbackup.system.base.sdk.livemount.model.LiveMountTargetLocation;
import openbackup.system.base.sdk.livemount.model.Performance;
import openbackup.system.base.sdk.resource.EnvironmentRestApi;
import openbackup.system.base.sdk.resource.ResourceRestApi;
import openbackup.system.base.sdk.resource.model.Environment;
import openbackup.system.base.sdk.resource.model.ResourceEntity;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.schedule.ScheduleRestApi;
import openbackup.system.base.sdk.schedule.model.ScheduleResponse;
import openbackup.system.base.sdk.user.enums.ResourceSetTypeEnum;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.util.ProviderRegistry;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.collections.MapUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;
import org.springframework.web.bind.annotation.RequestBody;

import java.text.Normalizer;
import java.time.temporal.ChronoUnit;
import java.util.AbstractMap;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.Set;
import java.util.regex.Pattern;
import java.util.stream.Collectors;

import javax.validation.Valid;

/**
 * Live Mount Service
 *
 * @author l00272247
 * @since 2020-09-18
 */
@Service
@Slf4j
public class LiveMountServiceImpl implements LiveMountService {
    private static final int SIGNAL_CONTROLLER_RESTRICT_NUM = 10000000;

    // VMware、Oracle单副本挂载最大数量
    private static final int SINGLE_COPY_LIVE_MOUNT_MAX_NUM = 32;

    /**
     * 共享文件系统最大保留时间
     */
    private static final int MAX_FILE_SYSTEM_KEEP_TIME = 96;

    /**
     * 客户端名字
     */
    private static final String CLIENT_NAME = "clientName";

    /**
     * 共享名称
     */
    private static final String SHARE_NAME = "shareName";

    /**
     * 文件系统保留时间
     */
    private static final String FILE_SYSTEM_KEEP_TIME = "fileSystemKeepTime";

    // live mount 检查单副本挂载数量支持的类型
    private static final List<String> SUPPORTED_OBJECT_TYPES = Arrays.asList(ResourceSubTypeEnum.ORACLE.getType(),
            ResourceSubTypeEnum.VMWARE.getType());

    private static final Map<String, String> TIME_UNIT_MAPPING = new HashMap<>();

    private static final Map<String, ChronoUnit> CHRONO_UNIT_MAPPING = new HashMap<>();

    private static final Map<String, String> FORMAT_MAPPING = new HashMap<>();

    private static final String HOUR = "hour";

    private static final String DATE = "date";

    private static final String WEEK = "week";

    private static final String MONTH = "month";

    private static final String YEAR = "year";

    private static final String USER_ID = "userId";

    private static final String SOURCE_ID = "sourceId";

    private static final String SOURCE_NAME = "sourceName";

    private static final String SOURCE_TYPE = "sourceType";

    private static final String SOURCE_SUB_TYPE = "sourceSubType";

    private static final String SOURCE_LOCATION = "sourceLocation";

    private static final String TARGET_NAME = "targetName";

    private static final String TARGET_LOCATION = "targetLocation";

    private static final String COPY_TIME = "copyTime";

    private static final String COPY_ID = "copyId";

    private static final String LIVE_MOUNT = "live_mount";

    private static final String TYPE = "type";

    private static final String EXTEND_FIELD = "extendField";

    private static final String MIGRATE = "migrate";

    private static final String GUID = "uuid";

    private static final String PERFORMANCE = "performance";

    private static final String POLICY = "policy";

    private static final String SOURCE_COPY = "source_copy";

    private static final String MOUNTED_COPY = "mounted_copy";

    private static final String PRE_SCRIPT = "pre_script";

    private static final String POST_SCRIPT = "post_script";

    private static final String FAILED_SCRIPT = "failed_script";

    private static final String FORCE_DELETE = "force_delete";

    private static final String RESERVE_COPY = "reserve_copy";

    private static final String UNMOUNT = "unmount";

    private static final String COPY_GENERATED_BY = "generated_by";

    private static final String DATA = "data";

    private static final String EXERCISE_ID = "exerciseId";

    private static final String TARGET_RESOURCE_NAME = "targetResourceName";

    private static final String TARGET_RESOURCE_PATH = "targetResourcePath";

    private static final String TARGET_RESOURCE_IP = "targetResourceIp";

    private static final String TARGET_RESOURCE_VERSION = "targetResourceVersion";

    private static final String EXERCISE_JOB_ID = "exerciseJobId";

    private static final String CALLBACK_CANCEL = "callback.cancel";

    private static final String ACTION_CANCEL = "/v1/internal/live-mount/action/cancel";

    private static final String CALLBACK_CANCEL_ID = "callback.cancel.live_mount_id";

    static {
        addTimeUnitMapping("h", HOUR, ChronoUnit.HOURS, "yyyy-MM-dd HH");
        addTimeUnitMapping("d", DATE, ChronoUnit.DAYS, "yyyy-MM-dd");
        addTimeUnitMapping("w", WEEK, ChronoUnit.WEEKS, "yyyy-ww");
        addTimeUnitMapping("MO", MONTH, ChronoUnit.MONTHS, "yyyy-MM");
        addTimeUnitMapping("y", YEAR, ChronoUnit.YEARS, "yyyy");
    }

    @Value("${service.url.pm-live-mount}")
    private String liveMountUrl;

    @Autowired
    private ResourceRestApi resourceRestApi;

    @Autowired
    private LiveMountEntityDao liveMountEntityDao;

    @Autowired
    private CopyRestApi copyRestApi;

    @Autowired
    private LiveMountRestApi liveMountClientRestApi;

    @Autowired
    private ScheduleRestApi scheduleRestApi;

    @Autowired
    private PageQueryService pageQueryService;

    @Autowired
    private LiveMountModelDao liveMountModelDao;

    @Autowired
    private LiveMountPolicyEntityDao liveMountPolicyEntityDao;

    @Autowired
    private EnvironmentRestApi environmentRestApi;

    @Autowired
    private ProviderRegistry providerRegistry;

    @Autowired
    private ProviderManager providerManager;

    @Autowired
    private ResourceService resourceService;

    @Autowired
    private JobService jobService;

    @Autowired
    private DefaultLiveMountServiceProvider defaultLiveMountServiceProvider;

    @Autowired
    private DeployTypeService deployTypeService;

    @Autowired
    private SystemSpecificationService systemSpecificationService;

    @Autowired
    private PolicyService policyService;

    @Autowired
    private CopyService copyService;

    @Autowired
    private CopyAuthVerifyService copyAuthVerifyService;

    @Autowired
    private ResourceSetApi resourceSetApi;

    @Autowired
    private CopyMapper copyMapper;

    @Autowired
    private StorageUnitService storageUnitService;

    private static void addTimeUnitMapping(String unit, String type, ChronoUnit chronoUnit, String format) {
        TIME_UNIT_MAPPING.put(unit, type);
        CHRONO_UNIT_MAPPING.put(unit, chronoUnit);
        FORMAT_MAPPING.put(unit, format);
    }

    /**
     * create live mount
     *
     * @param liveMountObject live mount object
     * @param copy copy
     * @param policy policy
     * @return live mount uuid list and source copy info
     */
    @Transactional
    @Override
    public Map.Entry<Copy, List<LiveMountEntity>> createLiveMounts(
            LiveMountObject liveMountObject, Copy copy, LiveMountPolicyEntity policy) {
        String resourceId = liveMountObject.getSourceResourceId();
        CopyResourceSummary copyResourceSummary = queryCopyResourceSummary(resourceId);

        // 安全一体机适配，Nas文件系统的快照数据被标记为CloudBackup
        if (deployTypeService.isCyberEngine() && ResourceSubTypeEnum.CLOUD_BACKUP_FILE_SYSTEM.getType()
            .equals(copyResourceSummary.getResourceSubType())) {
            copyResourceSummary.setResourceSubType(ResourceSubTypeEnum.NAS_FILESYSTEM.getType());
        }
        String resourceSubType = copyResourceSummary.getResourceSubType();
        List<String> targetResourceUuidList = liveMountObject.getTargetResourceUuidList();
        // VMs can be mounted to the same environment resource.
        ResourceEntity resourceEntity =
                JSONObject.fromObject(copyResourceSummary.getResourceProperties()).toBean(ResourceEntity.class);

        // check target resource uuid
        checkTargetResource(targetResourceUuidList);
        LiveMountFlowService provider =
                providerRegistry.findProvider(LiveMountFlowService.class, resourceSubType, null);
        // get target object name
        String name =
                Optional.ofNullable(liveMountObject.getName())
                        .orElseGet(() -> getTargetObjectName(provider, resourceEntity, liveMountObject).orElse(null));
        // filter target resource uuid
        targetResourceUuidList =
                filterTargetResourceUuidList(liveMountObject, targetResourceUuidList, resourceEntity, provider);
        List<ResourceEntity> targetResources = queryResources(targetResourceUuidList, resourceSubType);
        List<String> resourceUuids = getResourceUuids(targetResources);
        List<ResourceEntity> targetResourceEntity = new ArrayList<>(targetResources);
        // 构造缺失资源
        targetResourceUuidList.stream()
                .filter(uuid -> !resourceUuids.contains(uuid))
                .map(uuid -> constructResource(uuid, liveMountObject, copyResourceSummary))
                .forEach(targetResourceEntity::add);
        targetResourceUuidList = getResourceUuids(targetResourceEntity);

        // not allow same name resource on same target machine
        checkLiveMountNameAndTargetResource(name, targetResourceUuidList, resourceSubType);
        LiveMountCreateCheckParam checkParam = getLiveMountCreateCheckParam(liveMountObject, copy, copyResourceSummary,
            targetResourceEntity);
        Identity<LiveMountCreateCheckParam> preCheckContext = new Identity<>(resourceSubType, checkParam);
        liveMountClientRestApi.createLiveMountPreCheck(preCheckContext);
        // filter performance of parameter
        liveMountObject.setParameters(filterPerformanceData(liveMountObject.getParameters()));
        List<LiveMountEntity> liveMountUuidList =
            targetResourceEntity.stream()
                        .map(targetResource -> saveLiveMountEntity(liveMountObject, targetResource))
                        .collect(Collectors.toList());
        return new AbstractMap.SimpleEntry<>(copy, liveMountUuidList);
    }

    private static LiveMountCreateCheckParam getLiveMountCreateCheckParam(LiveMountObject liveMountObject, Copy copy,
        CopyResourceSummary copyResourceSummary, List<ResourceEntity> targetResourceEntity) {
        LiveMountCreateCheckParam checkParam = new LiveMountCreateCheckParam();
        checkParam.setLiveMountObject(liveMountObject);
        checkParam.setResource(copyResourceSummary);
        checkParam.setTargetResources(targetResourceEntity);
        checkParam.setCopy(copy);
        checkParam.setOperationEnums(OperationEnums.CREATE);
        return checkParam;
    }

    private ResourceEntity constructResource(
            String uuid, LiveMountObject liveMountObject, CopyResourceSummary copyResourceSummary) {
        ResourceEntity resourceEntity = new ResourceEntity();
        resourceEntity.setUuid(uuid);
        resourceEntity.setType(copyResourceSummary.getResourceType());
        resourceEntity.setSubType(copyResourceSummary.getResourceSubType());
        resourceEntity.setEnvironmentEndPoint("");
        resourceEntity.setPath(liveMountObject.getTargetLocation().getValue());
        return resourceEntity;
    }

    private List<String> getResourceUuids(List<ResourceEntity> targetResources) {
        return Optional.ofNullable(targetResources)
            .orElse(Collections.emptyList())
            .stream()
            .filter(Objects::nonNull)
            .map(ResourceEntity::getUuid)
            .collect(Collectors.toList());
    }

    private List<String> filterTargetResourceUuidList(
            LiveMountObject liveMountObject,
            List<String> targetResourceUuidList,
            ResourceEntity resourceEntity,
            LiveMountFlowService provider) {
        if (provider != null) {
            return provider.filterTargetResourceUuidList(
                    targetResourceUuidList, liveMountObject.getTargetLocation(), resourceEntity);
        }
        if (liveMountObject.getTargetLocation() == LiveMountTargetLocation.ORIGINAL) {
            return Collections.singletonList(resourceEntity.getParentUuid());
        }
        return targetResourceUuidList;
    }

    private Optional<String> getTargetObjectName(
            LiveMountFlowService provider, ResourceEntity resourceEntity, LiveMountObject liveMountObject) {
        if (provider == null) {
            return Optional.empty();
        }
        return provider.getTargetObjectName(liveMountObject, resourceEntity.getName());
    }

    private void checkTargetResource(List<String> targetResourceUuidList) {
        targetResourceUuidList.forEach(
                targetResource -> {
                    if (targetResource.length() > IsmNumberConstant.HUNDRED_TWENTY_EIGHT) {
                        throw new LegoCheckedException(
                                CommonErrorCode.ERR_PARAM,
                                "target resource: " + targetResource + " uuid is beyond 128");
                    }
                });
    }

    private void checkLiveMountNameAndTargetResource(
            String name, List<String> targetResourceUuidList, String resourceSubType) {
        boolean canCreate = true;
        for (String resourceUuid : targetResourceUuidList) {
            if (!VerifyUtil.isEmpty(queryLiveMountByParam(name, resourceUuid, resourceSubType))) {
                canCreate = false;
                break;
            }
        }
        if (!canCreate) {
            throw new LegoCheckedException(
                    CommonErrorCode.ERR_MOUNT_SAMENAME_RESOURCE,
                    "can not mount same name resource on same target machine");
        }
    }

    private LiveMountEntity queryLiveMountByParam(String name, String targetResourceUuid, String resourceSubType) {
        LambdaQueryWrapper<LiveMountEntity> wrapper =
                new LambdaQueryWrapper<LiveMountEntity>()
                        .eq(LiveMountEntity::getResourceSubType, resourceSubType)
                        .eq(LiveMountEntity::getResourceName, name)
                        .eq(LiveMountEntity::getTargetResourceId, targetResourceUuid);
        return liveMountEntityDao.selectOne(wrapper);
    }

    /**
     * query resource by id
     *
     * @param resourceId resource id
     * @return resource
     */
    @Override
    public ResourceEntity queryResource(String resourceId) {
        return resourceRestApi.queryResource(resourceId);
    }

    /**
     * query copy resource summary
     *
     * @param resourceId resource id
     * @return copy resource summary
     */
    @Override
    public CopyResourceSummary queryCopyResourceSummary(String resourceId) {
        return copyService.queryCopyResourceSummary(resourceId);
    }

    /**
     * query resources
     *
     * @param resourceIds resource ids
     * @param type type
     * @return resource entities
     */
    @Override
    public List<ResourceEntity> queryResources(List<String> resourceIds, String type) {
        return Optional.ofNullable(resourceIds).orElse(Collections.emptyList()).stream()
                .map(this::queryResource)
                .filter(Objects::nonNull)
                .collect(Collectors.toList());
    }

    private LiveMountEntity saveLiveMountEntity(LiveMountObject liveMountObject, ResourceEntity targetResourceEntity) {
        CopyResourceSummary copyResourceSummary = queryCopyResourceSummary(liveMountObject.getSourceResourceId());
        ResourceEntity resourceEntity = JSONObject.fromObject(copyResourceSummary.getResourceProperties())
                .toBean(ResourceEntity.class);
        LiveMountServiceProvider provider = providerManager.findProviderOrDefault(LiveMountServiceProvider.class,
                resourceEntity.getSubType(), defaultLiveMountServiceProvider);
        LiveMountEntity entity = provider.buildLiveMountEntity(liveMountObject, resourceEntity, targetResourceEntity);
        entity.setExerciseJobId(liveMountObject.getExerciseJobId());
        entity.setExerciseId(liveMountObject.getExerciseId());
        liveMountEntityDao.insert(entity);
        createResourceSetRelation(entity);
        return entity;
    }

    /**
     * 创建及时挂载资源对象
     *
     * @param entity 及时挂载实体
     */
    private void createResourceSetRelation(LiveMountEntity entity) {
        ResourceSetResourceBo resourceSetResourceBo = new ResourceSetResourceBo();
        resourceSetResourceBo.setResourceObjectId(entity.getId());
        resourceSetResourceBo.setIsManualAdd(Boolean.TRUE);
        resourceSetResourceBo.setType(ResourceSetTypeEnum.LIVE_MOUNT);
        resourceSetResourceBo.setScopeModule(ResourceSetScopeModuleEnum.LIVE_MOUNT.getType());
        resourceSetResourceBo.setParentResourceObjectId(entity.getResourceId());
        List<String> domainList = resourceSetApi.getRelatedDomainIdList(entity.getResourceId());
        // 当前及时挂载权限跟原备份走 此处需要根据备份id来查找对应域
        if (!VerifyUtil.isEmpty(domainList)) {
            log.info("create resource set relation, resource :{} , related domain list:{}.", entity.getResourceId(),
                domainList);
            resourceSetResourceBo.setDomainIdList(domainList);
        } else {
            log.warn("create resource set relation, resource :{} have no related domain, uuid:{}.",
                entity.getResourceId(), entity.getId());
            resourceSetResourceBo.setDomainIdList(resourceSetApi.getPublicDomainIdList());
        }
        resourceSetApi.addResourceSetRelation(resourceSetResourceBo);
    }

    /**
     * get resource id of source copy
     *
     * @param liveMountEntity live mount entity
     * @return the resource id of source copy
     */
    @Override
    public String getSourceCopyResourceId(LiveMountEntity liveMountEntity) {
        return liveMountEntity.getResourceId();
    }

    /**
     * get resource of source copy
     *
     * @param liveMountEntity live mount entity
     * @return resource of source copy
     */
    @Override
    public ResourceEntity getSourceCopyResource(LiveMountEntity liveMountEntity) {
        return queryResource(liveMountEntity.getResourceId());
    }

    /**
     * update live mount
     *
     * @param liveMountEntity live mount entity
     * @param resourceId resource id
     * @param policyId policy id
     */
    @Transactional
    @Override
    public void updateLiveMountPolicy(LiveMountEntity liveMountEntity, String resourceId, String policyId) {
        LambdaUpdateWrapper<LiveMountEntity> wrapper = createPolicyUpdateWrapper(liveMountEntity, resourceId, policyId);
        liveMountEntityDao.update(null, wrapper);
    }

    /**
     * update mounted copy info
     *
     * @param liveMountEntity live mount entity
     * @param mountedCopy mounted copy
     */
    @Transactional
    @Override
    public void updateMountedCopyInfo(LiveMountEntity liveMountEntity, Copy mountedCopy) {
        LambdaUpdateWrapper<LiveMountEntity> wrapper =
                new LambdaUpdateWrapper<LiveMountEntity>()
                        .eq(LiveMountEntity::getId, liveMountEntity.getId())
                        .set(LiveMountEntity::getMountJobId, liveMountEntity.getMountJobId())
                        .set(LiveMountEntity::getMountedCopyId, mountedCopy.getUuid())
                        .set(LiveMountEntity::getMountedCopyDisplayTimestamp, mountedCopy.getDisplayTimestamp())
                        .set(LiveMountEntity::getMountedSourceCopyId, mountedCopy.getUuid())
                        .set(LiveMountEntity::getCopyId, mountedCopy.getUuid());
        liveMountEntityDao.update(null, wrapper);
    }

    /**
     * update live mount policy and properties
     *
     * @param liveMountEntity live mount entity
     * @param resourceId resource id
     * @param policyId policy id
     * @param properties properties
     */
    @Transactional
    public void updateLiveMountPolicyAndProperties(
            LiveMountEntity liveMountEntity, String resourceId, String policyId, JSONObject properties) {
        LambdaUpdateWrapper<LiveMountEntity> wrapper = createPolicyUpdateWrapper(liveMountEntity, resourceId, policyId);
        String parameters = Optional.ofNullable(properties).orElseGet(JSONObject::new).toString();
        wrapper.set(LiveMountEntity::getParameters, parameters);
        liveMountEntityDao.update(null, wrapper);
    }

    private LambdaUpdateWrapper<LiveMountEntity> createPolicyUpdateWrapper(
            LiveMountEntity liveMountEntity, String resourceId, String policyId) {
        LambdaUpdateWrapper<LiveMountEntity> wrapper = new LambdaUpdateWrapper<>();
        return wrapper.eq(LiveMountEntity::getId, liveMountEntity.getId())
                .set(LiveMountEntity::getResourceId, resourceId)
                .set(LiveMountEntity::getPolicyId, policyId);
    }

    /**
     * delete live mount
     *
     * @param liveMountId live mount id
     */
    @Transactional
    @Override
    public void deleteLiveMount(String liveMountId) {
        liveMountEntityDao.deleteById(liveMountId);
        resourceSetApi.deleteResourceSetRelation(liveMountId, ResourceSetTypeEnum.COPY);
    }

    @Override
    public void updateLiveMount(LiveMountEntity liveMountEntity, LiveMountPolicyEntity policy, String copyId,
            boolean isManualUpdate) {
        checkHasActive(liveMountEntity.getEnableStatus(), true);
        Copy sourceCopy;
        LiveMountServiceProvider liveMountServiceProvider = providerManager.findProviderOrDefault(
            LiveMountServiceProvider.class, liveMountEntity.getResourceSubType(), defaultLiveMountServiceProvider);
        if (copyId != null) {
            sourceCopy = queryValidCopy(liveMountEntity.getResourceId(), copyId);
        } else if (!CopyDataSelection.LATEST.getName().equals(policy.getCopyDataSelectionPolicy())) {
            Map<String, String> ranges = new HashMap<>();
            ranges.put(CopyDataSelection.LAST_HOUR.getName(), "hour");
            ranges.put(CopyDataSelection.LAST_DAY.getName(), "date");
            ranges.put(CopyDataSelection.LAST_WEEK.getName(), "week");
            ranges.put(CopyDataSelection.LAST_MONTH.getName(), "month");
            String range = ranges.get(policy.getCopyDataSelectionPolicy());
            sourceCopy = queryLatestCopy(liveMountEntity, range, true);
        } else {
            // 根据应用类型筛选副本，
            boolean isSupportLogCopy = liveMountServiceProvider.isSupportLogCopy();
            sourceCopy = queryLatestCopy(liveMountEntity, null, isSupportLogCopy);
        }
        boolean isValid = liveMountServiceProvider.isSourceCopyCanBeMounted(sourceCopy, isManualUpdate);
        if (!isValid) {
            return;
        }
        executeLiveMountOnCopyChanged(liveMountEntity, policy, sourceCopy, isManualUpdate);
    }

    private Copy queryLatestCopy(LiveMountEntity liveMountEntity, String range, boolean isSupportLogCopy) {
        Map<String, Object> condition = Collections.singletonMap(COPY_GENERATED_BY,
            CopyGeneratedByEnum.BY_BACKUP.value());
        if (isSupportLogCopy) {
            return copyRestApi.queryLatestBackupCopy(liveMountEntity.getResourceId(), range, condition);
        } else {
            return copyRestApi.queryLatestBackupCopyWithOutLog(liveMountEntity.getResourceId(), range, condition)
                .orElse(null);
        }
    }

    private Copy queryLatestCopy(LiveMountEntity liveMountEntity, String range) {
        Map<String, Object> condition =
                Collections.singletonMap(COPY_GENERATED_BY, CopyGeneratedByEnum.BY_BACKUP.value());
        return copyRestApi.queryLatestBackupCopy(liveMountEntity.getResourceId(), range, condition);
    }

    /**
     * execute live mount on copy changed
     *
     * @param liveMountEntity live mount entity
     * @param policy policy
     * @param sourceCopy source copy
     * @param isStrict strict
     */
    @Override
    public void executeLiveMountOnCopyChanged(
            LiveMountEntity liveMountEntity, LiveMountPolicyEntity policy, Copy sourceCopy, boolean isStrict) {
        Copy mountedCopy = copyRestApi.queryCopyByID(liveMountEntity.getMountedCopyId(), false);
        if (!isStrict && mountedCopy != null && Objects.equals(sourceCopy.getUuid(), mountedCopy.getParentCopyUuid())) {
            log.info(
                    "mounted parent copy uuid:{} with source copy uuid:{} is same, Automatic scheduling cancelled.",
                    mountedCopy.getParentCopyUuid(),
                    sourceCopy.getUuid());
            return;
        }
        executeLiveMount(liveMountEntity, policy, sourceCopy, mountedCopy);
    }

    /**
     * clean mounted copy info
     *
     * @param liveMountEntity live mount entity
     */
    @Transactional
    @Override
    public void cleanMountedCopyInfo(LiveMountEntity liveMountEntity) {
        LambdaUpdateWrapper<LiveMountEntity> wrapper =
                new LambdaUpdateWrapper<LiveMountEntity>()
                        .eq(LiveMountEntity::getId, liveMountEntity.getId())
                        .set(LiveMountEntity::getMountedCopyDisplayTimestamp, null)
                        .set(LiveMountEntity::getMountedCopyId, null)
                        .set(LiveMountEntity::getMountedSourceCopyId, null);
        liveMountEntityDao.update(null, wrapper);
    }

    /**
     * execute live mount
     *
     * @param liveMountEntity live mount entity
     * @param policy policy
     * @param sourceCopy source copy
     * @param mountedCopy mounted copy
     * @param isDebuts debuts
     */
    @Override
    public void executeLiveMount(
            LiveMountEntity liveMountEntity,
            LiveMountPolicyEntity policy,
            Copy sourceCopy,
            Copy mountedCopy,
            boolean isDebuts) {
        if (sourceCopy != null) {
            // 如果是创建liveMount，不校验状态。
            if (!isDebuts) {
                checkLiveMountStatus(liveMountEntity.getStatus(), LiveMountOperateType.UPDATE);
            }
            TokenBo token = TokenBo.get(null);
            String userId;
            if (token != null) {
                userId = token.getUser().getId();
            } else {
                userId = sourceCopy.getUserId();
            }
            JSONObject jobData = getCallbackData(liveMountEntity);
            LiveMountFlowService provider = providerRegistry.findProvider(LiveMountFlowService.class,
                liveMountEntity.getResourceSubType(), null);
            JSONObject task = new JSONObject().set(TYPE, LIVE_MOUNT)
                .set(SOURCE_ID, sourceCopy.getResourceId())
                .set(SOURCE_NAME, sourceCopy.getResourceName())
                .set(SOURCE_SUB_TYPE, sourceCopy.getResourceSubType())
                .set(SOURCE_LOCATION, sourceCopy.getResourceLocation())
                .set(COPY_TIME, NumberUtil.convertToLong(sourceCopy.getTimestamp()))
                .set(COPY_ID, sourceCopy.getUuid())
                .set(TARGET_NAME, liveMountEntity.getTargetResourceName())
                .set(USER_ID, userId)
                .set(TARGET_LOCATION, getJobTargetLocation(provider, liveMountEntity))
                .set(DATA, jobData)
                .set(EXERCISE_ID, liveMountEntity.getExerciseId())
                .set(EXERCISE_JOB_ID, liveMountEntity.getExerciseJobId());
            JSONObject jobExtFiled = fillJobExtFiled(liveMountEntity, policy);
            task.set(EXTEND_FIELD, jobExtFiled);

            JSONObject params =
                    new JSONObject()
                            .set(LIVE_MOUNT, liveMountEntity)
                            .set(POLICY, policy)
                            .set(SOURCE_COPY, sourceCopy)
                            .set(MOUNTED_COPY, mountedCopy)
                            .set(LIVE_MOUNT_DEBUTS, isDebuts);

            scheduleRestApi.createImmediateSchedule(
                    TopicConstants.LIVE_MOUNT_EXECUTE_REQUEST, addJobQueueScope(params, sourceCopy), task);
        }
    }

    private JSONObject fillJobExtFiled(LiveMountEntity liveMountEntity, LiveMountPolicyEntity policy) {
        JSONObject jobExtFiled = new JSONObject();
        JSONObject jobConfigFiled = JSONObject.fromObject(liveMountEntity.getParameters());
        jobConfigFiled.set(TARGET_RESOURCE_IP, liveMountEntity.getTargetResourceIp());
        jobConfigFiled.set(TARGET_RESOURCE_NAME, liveMountEntity.getTargetResourceName());
        jobConfigFiled.set(TARGET_RESOURCE_PATH, liveMountEntity.getTargetResourcePath());
        ResourceEntity resourceEntity = queryResource(liveMountEntity.getTargetResourceId());
        if (resourceEntity != null) {
            jobConfigFiled.set(TARGET_RESOURCE_VERSION, resourceEntity.getVersion());
        }
        jobExtFiled.set(JobExtendInfoKeys.JOB_CONFIG, jobConfigFiled);
        if (policy != null) {
            jobExtFiled.set(JobExtendInfoKeys.LIVE_MOUNT_POLICY_ID, policy.getPolicyId());
        }
        return jobExtFiled;
    }

    private JSONObject addJobQueueScope(JSONObject params, Copy sourceCopy) {
        String queueScope = jobService.extractJobQueueScope(sourceCopy.getResourceSubType(), LIVE_MOUNT);
        if (StringUtils.isNotBlank(queueScope)) {
            String resourceId = sourceCopy.getResourceId();
            String rootUuid = resourceService.getResourceById(resourceId).map(ResourceBase::getRootUuid).orElse(null);
            params.put(queueScope, rootUuid);
            params.put(JobPayloadKeys.KEY_QUEUE_JOB_TYPE, LIVE_MOUNT);
        }
        return params;
    }

    private String getJobTargetLocation(LiveMountFlowService provider, LiveMountEntity liveMountEntity) {
        return provider != null
            ? provider.getJobTargetLocation(liveMountEntity)
            : liveMountEntity.getTargetResourcePath();
    }

    private JSONObject getCallbackData(LiveMountEntity liveMountEntity) {
        JSONObject jobData = new JSONObject();
        jobData.set(CALLBACK_CANCEL, liveMountUrl + ACTION_CANCEL);
        jobData.set(CALLBACK_CANCEL_ID, liveMountEntity.getId());
        return jobData;
    }

    /**
     * query valid copy
     *
     * @param sourceResourceId source resource id
     * @param copyId copy id
     * @return valid copy
     */
    @Override
    public Copy queryValidCopy(String sourceResourceId, String copyId) {
        Copy copy = copyRestApi.queryCopyByID(copyId);
        if (!Objects.equals(copy.getResourceId(), sourceResourceId)) {
            throw new LegoCheckedException(
                CommonErrorCode.ERR_PARAM, "resource of copy is inconsistent with the source resource");
        }

        // 安全一体机适配，Nas文件系统的快照数据被标记为CloudBackup
        if (deployTypeService.isCyberEngine() && ResourceSubTypeEnum.CLOUD_BACKUP_FILE_SYSTEM.getType()
            .equals(copy.getResourceSubType())) {
            copy.setResourceSubType(ResourceSubTypeEnum.NAS_FILESYSTEM.getType());
        }
        return copy;
    }

    /**
     * query live mount entities
     *
     * @param page page
     * @param size size
     * @param conditions conditions
     * @param orders orders
     * @return live mount entity page
     */
    @Override
    public BasePage<LiveMountModel> queryLiveMountEntities(int page, int size, String conditions, List<String> orders) {
        return pageQueryService.pageQuery(
                LiveMountModel.class,
                liveMountModelDao::page,
                new PageQueryParam(page, size, conditions, orders),
                "-created_time",
                "id");
    }

    /**
     * query live mount entities by policy id
     *
     * @param policyId live mount policy id
     * @return policy entity
     */
    @Override
    public List<LiveMountEntity> queryLiveMountEntitiesByPolicyId(String policyId) {
        if (VerifyUtil.isEmpty(policyId)) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "the policy id is null");
        }
        return liveMountEntityDao.selectList(new QueryWrapper<LiveMountEntity>().eq("policy_id", policyId));
    }

    /**
     * get live mount entity by id
     *
     * @param liveMountId live mount id
     * @return live mount entity
     */
    @Override
    public LiveMountEntity selectLiveMountEntityById(String liveMountId) {
        LiveMountEntity liveMountEntity = liveMountEntityDao.selectById(liveMountId);
        if (liveMountEntity == null) {
            throw new LegoCheckedException(
                    CommonErrorCode.OBJ_NOT_EXIST, "live mount is not exist. live mount id is " + liveMountId);
        }
        return liveMountEntity;
    }

    /**
     * modify live mount
     *
     * @param liveMountId live mount id
     * @param liveMountParam live mount param
     */
    @Override
    public void modifyLiveMount(String liveMountId, LiveMountParam liveMountParam) {
        LiveMountEntity liveMountEntity = selectLiveMountEntityById(liveMountId);
        checkHasActive(liveMountEntity.getEnableStatus(), true);
        checkLiveMountStatus(liveMountEntity.getStatus(), LiveMountOperateType.MODIFY);
        String policyId = liveMountParam.getPolicyId();
        JSONObject param = JSONObject.fromObject(liveMountEntity.getParameters());
        Map<String, Object> parameters = liveMountParam.getParameters();
        if (parameters != null) {
            param = modifyParameters(param, parameters, liveMountEntity.getResourceSubType());
        }
        if (!VerifyUtil.isEmpty(policyId)) {
            LiveMountPolicyEntity policy = liveMountPolicyEntityDao.selectPolicy(policyId);
            if (VerifyUtil.isEmpty(policy)) {
                throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "policy " + policyId + " is not exist");
            }
        }
        LiveMountObject liveMountObject = new LiveMountObject();
        String resourceId = liveMountEntity.getResourceId();
        liveMountObject.setSourceResourceId(resourceId);
        liveMountObject.setPolicyId(policyId);
        liveMountObject.setTargetLocation(LiveMountTargetLocation.get(liveMountEntity.getTargetLocation()));
        liveMountObject.setTargetResourceUuidList(Collections.singletonList(liveMountEntity.getTargetResourceId()));
        liveMountObject.setParameters(param.toMap(Object.class));

        CopyResourceSummary copyResourceSummary = queryCopyResourceSummary(resourceId);
        String resourceSubType = copyResourceSummary.getResourceSubType();

        LiveMountCreateCheckParam checkParam = new LiveMountCreateCheckParam();
        checkParam.setLiveMountObject(liveMountObject);
        checkParam.setResource(copyResourceSummary);
        checkParam.setOperationEnums(OperationEnums.MODIFY);
        Identity<LiveMountCreateCheckParam> context = new Identity<>(resourceSubType, checkParam);
        liveMountClientRestApi.createLiveMountPreCheck(context);
        String mountedCopyId = liveMountEntity.getMountedCopyId();

        Boolean canModify = verifyParameters(JSONObject.fromObject(liveMountEntity.getParameters()), param);
        liveMountEntity.setParameters(param.toString());
        if (!VerifyUtil.isEmpty(mountedCopyId) && canModify) {
            liveMountClientRestApi.updatePerformanceSetting(new Identity<>(resourceSubType, liveMountEntity));
        }
        updateLiveMountPolicyAndProperties(liveMountEntity, resourceId, policyId, param);
        String policy1 = Optional.ofNullable(policyId).orElse("");
        String policy2 = Optional.ofNullable(liveMountEntity.getPolicyId()).orElse("");
        if (!Objects.equals(policy1, policy2)) {
            initialAndUpdateLiveMountSchedule(liveMountEntity, policyId);
        }
    }

    /**
     * destroy live mount
     *
     * @param liveMountId live mount id
     * @param isReserveCopy reserve copy
     * @param isForceDelete force delete
     * @param extendParam extendParam
     */
    @Override
    public void unmountLiveMount(String liveMountId, boolean isReserveCopy, boolean isForceDelete,
        UnmountExtendParam extendParam) {
        LiveMountEntity liveMountEntity = liveMountEntityDao.selectById(liveMountId);
        if (liveMountEntity == null) {
            return;
        }
        checkLiveMountStatus(liveMountEntity.getStatus(), LiveMountOperateType.DESTROY);
        LiveMountFlowService provider =
                providerRegistry.findProvider(LiveMountFlowService.class, liveMountEntity.getResourceSubType(), null);

        JSONObject jobData = getCallbackData(liveMountEntity);
        JSONObject task = new JSONObject().set(TYPE, UNMOUNT).set(SOURCE_ID, liveMountEntity.getResourceId())
                .set(SOURCE_NAME, liveMountEntity.getResourceName()).set(SOURCE_TYPE, liveMountEntity.getResourceType())
                .set(SOURCE_SUB_TYPE, liveMountEntity.getResourceSubType())
                .set(SOURCE_LOCATION, liveMountEntity.getResourcePath())
                .set(TARGET_NAME, liveMountEntity.getTargetResourceName())
                .set(EXERCISE_ID, extendParam.getExerciseId())
                .set(EXERCISE_JOB_ID, extendParam.getExerciseJobId())
                .set(USER_ID,
                        VerifyUtil.isEmpty(extendParam.getUserId())
                                ? TokenBo.get().getUser().getId()
                                : extendParam.getUserId())
                .set(TARGET_LOCATION, getJobTargetLocation(provider, liveMountEntity)).set(DATA, jobData);
        String mountedCopyId = liveMountEntity.getMountedCopyId();
        if (!VerifyUtil.isEmpty(mountedCopyId)) {
            Copy mountedCopy = copyRestApi.queryCopyByID(mountedCopyId, false);
            if (mountedCopy != null) {
                task.set(COPY_TIME, NumberUtil.convertToLong(mountedCopy.getTimestamp()))
                        .set(COPY_ID, mountedCopy.getUuid());
            }
        } else {
            if (!VerifyUtil.isEmpty(liveMountEntity.getCopyId())) {
                task.set(COPY_ID, liveMountEntity.getCopyId());
            }
        }
        JSONObject params =
                new JSONObject()
                        .set(LIVE_MOUNT, liveMountEntity)
                        .set(RESERVE_COPY, isReserveCopy)
                        .set(FORCE_DELETE, isForceDelete);
        scheduleRestApi.createImmediateSchedule(TopicConstants.LIVE_MOUNT_UNMOUNT_REQUEST, params, task);
    }

    /**
     * check live mount status
     *
     * @param status live mount status
     * @param operate live mount operate
     */
    @Override
    public void checkLiveMountStatus(String status, String operate) {
        if (!LiveMountStatus.STATUS_MAP.get(operate).apply(status)) {
            throw new LegoCheckedException(
                    CommonErrorCode.OPERATION_FAILED, "the live mount status is:" + status + ", can't " + operate);
        }
    }

    @Override
    public void revokeLiveMountUserId(String userId) {
        LambdaUpdateWrapper<LiveMountEntity> wrapper = new LambdaUpdateWrapper<>();
        wrapper.eq(LiveMountEntity::getUserId, userId).set(LiveMountEntity::getUserId, null);

        liveMountEntityDao.update(null, wrapper);
    }

    @Override
    public void cancelLiveMount(String liveMountId) {
        // 更新挂载状态为取消
        LiveMountEntity liveMountEntity = selectLiveMountEntityById(liveMountId);
        updateLiveMountStatus(liveMountEntity, LiveMountStatus.MOUNT_FAILED);
    }

    /**
     * initial live mount schedule
     *
     * @param liveMountEntity live mount entity, copy data selection type and parameters is new data
     * @param policyId policy id
     * @return schedule id
     */
    @Override
    public Optional<String> initialLiveMountSchedule(LiveMountEntity liveMountEntity, String policyId) {
        boolean isAllEmptyPolicyId = VerifyUtil.isEmpty(liveMountEntity.getPolicyId()) && VerifyUtil.isEmpty(policyId);
        if (isAllEmptyPolicyId) {
            log.info("policy of live mount {} is not changed. policy id: {}", liveMountEntity.getId(), policyId);
            return Optional.ofNullable(liveMountEntity.getScheduleId());
        }
        if (!VerifyUtil.isEmpty(liveMountEntity.getPolicyId())) {
            log.info("live mount {} is not automatic mode.", liveMountEntity.getId());
            deleteSchedule(liveMountEntity);
        }
        String activePolicyId;
        if (VerifyUtil.isEmpty(policyId)) {
            // 首次初始化调度器场景
            activePolicyId = liveMountEntity.getPolicyId();
        } else {
            // 修改policy初始化调度器场景
            activePolicyId = policyId;
        }
        LiveMountPolicyEntity policy = liveMountPolicyEntityDao.selectPolicy(activePolicyId);
        // scheduled action
        if (ScheduledType.PERIOD_SCHEDULE.getName().equals(policy.getSchedulePolicy())) {
            String interval = policy.getScheduleInterval() + policy.getScheduleIntervalUnit();
            JSONObject params = new JSONObject().set("live_mount_id", liveMountEntity.getId());
            ScheduleResponse scheduleResponse =
                    scheduleRestApi.createIntervalSchedule(
                            TopicConstants.LIVE_MOUNT_SCHEDULE, interval, policy.getScheduleStartTime(), params);
            return Optional.ofNullable(scheduleResponse.getScheduleId());
        }
        return Optional.empty();
    }

    /**
     * initial Live Mount Schedule
     *
     * @param liveMountEntity live mount entity
     * @param policyId policy id
     */
    @Override
    public void initialAndUpdateLiveMountSchedule(LiveMountEntity liveMountEntity, String policyId) {
        String scheduleId = initialLiveMountSchedule(liveMountEntity, policyId).orElse(null);
        if (!Objects.equals(scheduleId, liveMountEntity.getScheduleId())) {
            updateLiveMountSchedule(liveMountEntity, scheduleId);
        }
    }

    /**
     * update live mount schedule
     *
     * @param liveMountEntity live mount entity
     * @param scheduleId schedule id
     */
    @Transactional
    @Override
    public void updateLiveMountSchedule(LiveMountEntity liveMountEntity, String scheduleId) {
        LambdaUpdateWrapper<LiveMountEntity> wrapper =
                new LambdaUpdateWrapper<LiveMountEntity>()
                        .eq(LiveMountEntity::getId, liveMountEntity.getId())
                        .set(LiveMountEntity::getScheduleId, scheduleId);
        liveMountEntityDao.update(null, wrapper);
    }

    /**
     * update live mount parameters
     *
     * @param liveMountEntity liveMountEntity
     * @param parameters parameters
     */
    @Override
    public void updateLiveMountParameters(LiveMountEntity liveMountEntity, String parameters) {
        LambdaUpdateWrapper<LiveMountEntity> wrapper = new LambdaUpdateWrapper<>();
        wrapper.eq(LiveMountEntity::getId, liveMountEntity.getId());
        wrapper.set(LiveMountEntity::getParameters, parameters);
        liveMountEntityDao.update(null, wrapper);
    }

    private void deleteSchedule(LiveMountEntity liveMountEntity) {
        String scheduleId = liveMountEntity.getScheduleId();
        if (!VerifyUtil.isEmpty(scheduleId)) {
            scheduleRestApi.deleteSchedule(scheduleId);
        }
    }

    /**
     * update live mount status
     *
     * @param liveMountEntity live mount entity
     * @param status status
     */
    @Transactional
    @Override
    public void updateLiveMountStatus(LiveMountEntity liveMountEntity, LiveMountStatus status) {
        LambdaUpdateWrapper<LiveMountEntity> wrapper =
                new LambdaUpdateWrapper<LiveMountEntity>()
                        .eq(LiveMountEntity::getId, liveMountEntity.getId())
                        .set(LiveMountEntity::getStatus, status.getName());
        liveMountEntityDao.update(null, wrapper);
    }

    /**
     * update mounted resource
     *
     * @param liveMountEntity live mount entity
     * @param mountedResource mounted resource
     */
    @Transactional
    @Override
    public void updateLiveMountMountedResource(LiveMountEntity liveMountEntity, String mountedResource) {
        LambdaUpdateWrapper<LiveMountEntity> wrapper =
                new LambdaUpdateWrapper<LiveMountEntity>()
                        .eq(LiveMountEntity::getId, liveMountEntity.getId())
                        .set(LiveMountEntity::getMountedResourceId, mountedResource);
        liveMountEntityDao.update(null, wrapper);
    }

    /**
     * query target environments
     *
     * @param copyId copy id
     * @return target environments
     */
    @Override
    public BasePage<Environment> queryTargetEnvironments(String copyId) {
        Copy copy = copyRestApi.queryCopyByID(copyId);
        LambdaQueryWrapper<LiveMountEntity> wrapper =
                new LambdaQueryWrapper<LiveMountEntity>().eq(LiveMountEntity::getResourceId, copy.getResourceId());
        List<LiveMountEntity> liveMountEntities = liveMountEntityDao.selectList(wrapper);

        LiveMountFlowService provider =
                providerRegistry.findProvider(LiveMountFlowService.class, copy.getResourceSubType(), null);
        List<Environment> details = provider == null
            ? Collections.emptyList()
            : provider.getEnvironments(liveMountEntities, copy);
        return BasePage.create(details);
    }

    @Override
    public void activateLiveMount(String liveMountId) {
        LiveMountEntity liveMountEntity = liveMountEntityDao.selectById(liveMountId);
        checkActiveLiveMount(liveMountEntity);
        if (!VerifyUtil.isEmpty(liveMountEntity.getEnableStatus())
                && LiveMountEnableStatus.ACTIVATED.equals(
                        LiveMountEnableStatus.get(liveMountEntity.getEnableStatus()))) {
            throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED, "The live mount is activated.");
        }
        LambdaUpdateWrapper<LiveMountEntity> updateWrapper =
                new LambdaUpdateWrapper<LiveMountEntity>()
                        .eq(LiveMountEntity::getId, liveMountEntity.getId())
                        .set(LiveMountEntity::getEnableStatus, LiveMountEnableStatus.ACTIVATED.getName());
        liveMountEntityDao.update(null, updateWrapper);
    }

    @Override
    public void deactivateLiveMount(String liveMountId) {
        LiveMountEntity liveMountEntity = liveMountEntityDao.selectById(liveMountId);
        checkActiveLiveMount(liveMountEntity);
        if (!VerifyUtil.isEmpty(liveMountEntity.getEnableStatus())
                && LiveMountEnableStatus.DISABLED.equals(
                        LiveMountEnableStatus.get(liveMountEntity.getEnableStatus()))) {
            throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED, "The live mount is disabled.");
        }
        LambdaUpdateWrapper<LiveMountEntity> updateWrapper =
                new LambdaUpdateWrapper<LiveMountEntity>()
                        .eq(LiveMountEntity::getId, liveMountEntity.getId())
                        .set(LiveMountEntity::getEnableStatus, LiveMountEnableStatus.DISABLED.getName());
        liveMountEntityDao.update(null, updateWrapper);
    }

    private void checkActiveLiveMount(LiveMountEntity liveMountEntity) {
        if (VerifyUtil.isEmpty(liveMountEntity)) {
            throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "live mount is not exist.");
        }
        if (!LiveMountStatus.canActivate(liveMountEntity.getStatus())) {
            throw new LegoCheckedException(
                    CommonErrorCode.OPERATION_FAILED,
                    "Not allow change live mount status when it was " + liveMountEntity.getStatus());
        }
    }

    /**
     * if live mount enable status is not equal activated, raise error.
     *
     * @param enableStatus activated, disabled
     * @param isStrict true, false
     * @return enable_status
     */
    @Override
    public boolean checkHasActive(String enableStatus, boolean isStrict) {
        boolean hasActive = LiveMountEnableStatus.ACTIVATED.getName().equals(enableStatus);
        if (isStrict) {
            if (!hasActive) {
                throw new LegoCheckedException(
                        CommonErrorCode.OPERATION_FAILED,
                        "When livemount is disabled, it cannot be modified or updated.");
            }
        }
        return hasActive;
    }

    private JSONObject modifyParameters(JSONObject param, Map<String, Object> parameters, String resourceSubType) {
        JSONObject performance = param.getJSONObject(PERFORMANCE);
        loadPerformance(performance, parameters);
        param.put(PERFORMANCE, performance);
        if (ResourceSubTypeEnum.ORACLE.getType().equals(resourceSubType)) {
            setScriptParameters(param, parameters, PRE_SCRIPT);
            setScriptParameters(param, parameters, POST_SCRIPT);
            setScriptParameters(param, parameters, FAILED_SCRIPT);
        }
        return param;
    }

    private Boolean verifyParameters(JSONObject param, Map<String, Object> parameters) {
        Map<String, Object> paramMap = param.getJSONObject(PERFORMANCE).toMap(Object.class);
        Map<String, Object> performanceMap = JSONObject.fromObject(parameters.get(PERFORMANCE)).toMap(Object.class);

        return !paramMap.equals(performanceMap);
    }

    private void setScriptParameters(JSONObject param, Map<String, Object> parameters, String preScript) {
        Object script = parameters.get(preScript);
        if (VerifyUtil.isEmpty(script)) {
            param.remove(preScript);
        } else {
            param.put(preScript, script);
        }
    }

    private JSONObject loadPerformance(JSONObject param, Map<String, Object> parameters) {
        Map<String, Object> performance = JSONObject.fromObject(parameters.get(PERFORMANCE)).toMap(Object.class);
        param.putAll(performance);
        performance.forEach(
                (key, value) -> {
                    if (value == null) {
                        param.remove(key);
                    }
                });
        return param;
    }

    // 过滤输入为0，将小数变为整数
    private Map<String, Object> filterPerformanceData(Map<String, Object> parameters) {
        Map<String, Object> performanceMap = JSONObject.fromObject(parameters.get(PERFORMANCE)).toMap(Object.class);
        PerformanceValidator performanceValidator = new PerformanceValidator();
        Performance performance = performanceValidator.loadPerformance(performanceMap);
        Map<String, Object> performanceMap1 = JSONObject.fromObject(performance).toMap(Object.class);
        return JSONObject.fromObject(parameters).set(PERFORMANCE, performanceMap1).toMap(Object.class);
    }

    /**
     * check target environment status
     *
     * @param liveMount live mount
     */
    @Override
    public void checkTargetEnvironmentStatus(LiveMountEntity liveMount) {
        LiveMountInterceptorProvider liveMountInterceptorProvider = providerManager.findProvider(
            LiveMountInterceptorProvider.class, liveMount.getResourceSubType(), null);
        // 安全一体机适配
        if (deployTypeService.isCyberEngine()) {
            return;
        }
        if (liveMountInterceptorProvider != null && !liveMountInterceptorProvider.isRefreshTargetEnvironment()) {
            return;
        }
        ResourceEntity resourceEntity = resourceRestApi.queryResource(liveMount.getTargetResourceId());
        if (VerifyUtil.isEmpty(resourceEntity)) {
            throw new LegoCheckedException(
                    CommonErrorCode.RESOURCE_LINK_STATUS_OFFLINE, "target environment is offline.");
        }

        String environmentId = resourceEntity.getEnvironmentUuid();
        Environment targetEnvironment =
                PageQueryRestApi.get(environmentRestApi::queryEnvironment)
                        .queryOne(new JSONObject().set(GUID, environmentId));

        if (VerifyUtil.isEmpty(targetEnvironment)
                || Environment.ENVIRONMENT_OFFLINE.equals(targetEnvironment.getLinkStatus())) {
            throw new LegoCheckedException(
                    CommonErrorCode.RESOURCE_LINK_STATUS_OFFLINE, "target environment is offline.");
        }
    }

    /**
     * migrate Live mount
     *
     * @param liveMountId livemount id
     * @param mountMigrateRequest migrate live mount request
     */
    @Override
    public void migrateLiveMount(String liveMountId, LiveMountMigrateRequest mountMigrateRequest) {
        LiveMountEntity liveMountEntity = selectLiveMountEntityById(liveMountId);
        checkLiveMountStatus(liveMountEntity.getStatus(), LiveMountOperateType.MIGRATE);
        checkHasActive(liveMountEntity.getEnableStatus(), true);

        if (VerifyUtil.isEmpty(liveMountEntity.getMountedCopyId())) {
            throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED, "The livemount cannot be migrated.");
        }
        LiveMountFlowService provider = providerRegistry.findProvider(LiveMountFlowService.class,
            liveMountEntity.getResourceSubType(), null);
        if (provider != null) {
            // check request params
            provider.migrateLiveMountPreCheck(liveMountEntity, mountMigrateRequest);
        }

        CopyResourceSummary copyResourceSummary = queryCopyResourceSummary(liveMountEntity.getResourceId());
        ResourceEntity resourceEntity =
                JSONObject.fromObject(copyResourceSummary.getResourceProperties()).toBean(ResourceEntity.class);

        TokenBo token = TokenBo.get();
        String userId = token.getUser().getId();

        JSONObject jobData = getCallbackData(liveMountEntity);
        JSONObject task =
                new JSONObject()
                        .set(TYPE, MIGRATE)
                        .set(SOURCE_ID, resourceEntity.getUuid())
                        .set(SOURCE_NAME, resourceEntity.getName())
                        .set(SOURCE_TYPE, resourceEntity.getType())
                        .set(SOURCE_SUB_TYPE, resourceEntity.getSubType())
                        .set(SOURCE_LOCATION, resourceEntity.getPath())
                        .set(TARGET_NAME, liveMountEntity.getTargetResourceName())
                        .set(USER_ID, userId)
                        .set(TARGET_LOCATION, getJobTargetLocation(provider, liveMountEntity))
                        .set(DATA, jobData);
        String mountedCopyId = liveMountEntity.getMountedCopyId();
        Copy mountedCopy = copyRestApi.queryCopyByID(mountedCopyId);
        task.set(COPY_TIME, NumberUtil.convertToLong(mountedCopy.getTimestamp())).set(COPY_ID, mountedCopy.getUuid());

        JSONObject params = new JSONObject().set(LIVE_MOUNT, liveMountEntity).set(MIGRATE, mountMigrateRequest);
        scheduleRestApi.createImmediateSchedule(TopicConstants.LIVE_MOUNT_MIGRATE_REQUEST, params, task);
    }

    @Override
    public int countLiveMountByCopyId(String copyId) {
        return copyRestApi.countCopyByParentId(copyId);
    }

    @Override
    public List<String> createLiveMountCommon(@RequestBody @Valid LiveMountObject liveMountObject) {
        int targetResourceSize = liveMountObject.getTargetResourceUuidList().size();
        if (targetResourceSize > IsmNumberConstant.EIGHT) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "number of targets is exceeded the limit(8).");
        }
        // 校验本地盘是否支持此应用的副本(本地盘暂时都不支持即时挂载)
        checkIsLocalDiskSupportCopy(liveMountObject);
        // 检查即时挂载最大数量: 控制器数*25
        if (!deployTypeService.isCyberEngine()) {
            checkCreateLiveMountNum(targetResourceSize);
        } else {
            // 校验共享文件系统名称
            liveMountObject.getFileSystemShareInfoList().forEach(item -> {
                checkFileSystemNameRegex(item.getFileSystemName());
                // 校验高级参数
                checkAdvanceParams(item.getAdvanceParams());
            });
            // 校验共享文件系统保留时间
            checkFileSystemKeepTime(liveMountObject);
        }

        String policyId = liveMountObject.getPolicyId();

        // 检查挂载更新策略
        LiveMountPolicyEntity policy = getLiveMountPolicyEntity(policyId);
        String copyId = liveMountObject.getCopyId();
        Copy copy = queryValidCopy(liveMountObject.getSourceResourceId(), copyId);

        // 安全一体机下worm类型文件系统禁用恢复
        checkResourceSubType(copy.getResourceId());
        // 恢复演练功能在恢复演练层已校验恢复演练权限，此处不再校验副本即时挂载权限
        if (StringUtils.isEmpty(liveMountObject.getExerciseJobId())) {
            copyAuthVerifyService.checkCopyOperationAuth(copy,
                Collections.singletonList(AuthOperationEnum.LIVE_MOUNT.getAuthOperation()));
        }
        // 校验单副本挂载规格
        if (!deployTypeService.isCyberEngine()) {
            checkSingleCopyMaxNum(copy, targetResourceSize);
        }

        // 检查副本
        checkSourceCopy(copy);

        Map.Entry<Copy, List<LiveMountEntity>> entry = createLiveMounts(liveMountObject, copy, policy);
        executeLiveMountsOnCreated(entry, policy);
        return entry.getValue().stream().map(LiveMountEntity::getId).collect(Collectors.toList());
    }

    private void checkIsLocalDiskSupportCopy(LiveMountObject liveMountObject) {
        CopiesEntity copiesEntity = copyMapper.selectById(liveMountObject.getCopyId());
        if (VerifyUtil.isEmpty(copiesEntity.getStorageId())) {
            return;
        }
        StorageUnitVo storageUnitVo = storageUnitService.getStorageUnitById(copiesEntity.getStorageId())
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Storage unit not exist"));
        log.info("Copy subtype is : {} StorageUnit type is: {}", copiesEntity.getResourceSubType(),
            storageUnitVo.getDeviceType());
        if (StorageUnitTypeEnum.BASIC_DISK.getType().equals(storageUnitVo.getDeviceType())) {
            throw new LegoCheckedException(CommonErrorCode.LIVE_MOUNT_NOT_SUPPORT_BASIC_DISK,
                "Local disks do not support instant mounting.");
        }
    }

    private void checkAdvanceParams(Map<String, Object> advanceParams) {
        if (MapUtils.isEmpty(advanceParams)) {
            return;
        }
        Set<String> keySet = advanceParams.keySet();
        if (CollectionUtils.isEmpty(keySet)) {
            return;
        }
        // 校验客户端名字
        if (keySet.contains(CLIENT_NAME)) {
            String clientName = MapUtils.getString(advanceParams, CLIENT_NAME);
            // 长度1-255，不能包含空格，且不能以-开头
            if (StringUtils.isEmpty(clientName) || clientName.toCharArray().length > 255 || StringUtils.contains(
                    clientName, " ") || StringUtils.startsWith(clientName, "-")) {
                throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "The client name is error.");
            }
        }
        // 校验共享名称
        if (keySet.contains(SHARE_NAME)) {
            checkFileSystemNameRegex(MapUtils.getString(advanceParams, SHARE_NAME));
        }
    }

    private void checkFileSystemKeepTime(LiveMountObject liveMountObject) {
        JSONObject parameters = JSONObject.fromObject(liveMountObject.getParameters());
        JSONObject performance = parameters.getJSONObject(PERFORMANCE);
        Map<String, Object> params = performance.toMap(Object.class);
        String fileSystemKeepTime = MapUtils.getString(params, FILE_SYSTEM_KEEP_TIME);
        if (StringUtils.isEmpty(fileSystemKeepTime) || !Pattern.compile(RegexpConstants.INTEGER_NEGATIVE)
                .matcher(Normalizer.normalize(fileSystemKeepTime, Normalizer.Form.NFKC))
                .matches() || Integer.valueOf(fileSystemKeepTime) > MAX_FILE_SYSTEM_KEEP_TIME) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM,
                    "The fileSystemKeepTime must range from 1 to 96.");
        }
    }

    private void checkFileSystemNameRegex(String fileSystemName) {
        // 支持文件系统的名称由数字，字母，"-"、"."、"_"组成，字符为1到255
        if (!StringUtils.isEmpty(fileSystemName) && fileSystemName.toCharArray().length <= 255 && ValidateUtil.match(
                RegexpConstants.FILE_SYSTEM_NAME_REGEX, fileSystemName)) {
            return;
        }
        throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Nas filesystem name is not right!");
    }

    private void checkResourceSubType(String resourceId) {
        if (deployTypeService.isCyberEngine() && StringUtils.isNotEmpty(resourceId)) {
            Optional<ProtectedResource> resourceOptional = resourceService.getResourceById(resourceId);
            if (resourceOptional.isPresent() && resourceOptional.get().getExtendInfo() != null && StringUtils.equals(
                    resourceOptional.get().getExtendInfo().get(
                        LiveMountConstants.FILE_SUB_TYPE), LiveMountConstants.SUB_TYPE_WORM)) {
                throw new LegoCheckedException("Invalid resource sub type to start restore task: " + resourceId);
            }
        }
    }

    private LiveMountPolicyEntity getLiveMountPolicyEntity(String policyId) {
        LiveMountPolicyEntity policy = null;
        if (!VerifyUtil.isEmpty(policyId)) {
            if (policyId.length() > IsmNumberConstant.HUNDRED_TWENTY_EIGHT) {
                throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "policy id is greater than 128.");
            }
            policy = policyService.selectPolicyById(policyId);
            if (policy == null) {
                throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "policy is not exist. id=" + policyId);
            }
        }
        return policy;
    }

    private void checkCreateLiveMountNum(int targetResourceSize) {
        int count = liveMountEntityDao.selectCount(null).intValue() + targetResourceSize;

        int totalLimit = systemSpecificationService.getClusterNodeCount() * SIGNAL_CONTROLLER_RESTRICT_NUM;
        log.info("current total limit is {}", totalLimit);
        if (count > totalLimit) {
            if (deployTypeService.isCyberEngine()) {
                throw new LegoCheckedException(
                        CommonErrorCode.LIVE_MOUNT_COUNT_OVER_LIMIT_CYBER_ENGINE,
                        new String[] {String.valueOf(totalLimit)},
                        "Exceeded the limit(" + totalLimit + ").");
            }
            throw new LegoCheckedException(
                    CommonErrorCode.LIVE_MOUNT_COUNT_OVER_LIMIT,
                    new String[] {String.valueOf(totalLimit)},
                    "Exceeded the limit(" + totalLimit + ").");
        }
    }

    private void checkSingleCopyMaxNum(Copy copy, int targetResourceSize) {
        // 校验单副本挂载数量是否超过系统规格
        int copyLiveMountCount = countLiveMountByCopyId(copy.getUuid()) + targetResourceSize;
        if (SUPPORTED_OBJECT_TYPES.contains(copy.getResourceSubType())
                && copyLiveMountCount > SINGLE_COPY_LIVE_MOUNT_MAX_NUM) {
            if (deployTypeService.isCyberEngine()) {
                throw new LegoCheckedException(
                        CommonErrorCode.LIVE_MOUNT_COUNT_OVER_LIMIT_CYBER_ENGINE,
                        new String[] {String.valueOf(SINGLE_COPY_LIVE_MOUNT_MAX_NUM)},
                        "Exceeded the limit(" + SINGLE_COPY_LIVE_MOUNT_MAX_NUM + ").");
            }
            throw new LegoCheckedException(
                    CommonErrorCode.LIVE_MOUNT_COUNT_OVER_LIMIT,
                    new String[] {String.valueOf(SINGLE_COPY_LIVE_MOUNT_MAX_NUM)},
                    "Exceeded the limit(" + SINGLE_COPY_LIVE_MOUNT_MAX_NUM + ").");
        }
        log.info("Vmware or Oracle live mount nums({}) not exceeding the quantity limit.", copyLiveMountCount);
    }

    @Override
    public void checkSourceCopy(Copy copy) {
        CopyStatus status = CopyStatus.get(copy.getStatus());
        if (status != CopyStatus.NORMAL) {
            if (deployTypeService.isCyberEngine()) {
                throw new LegoCheckedException(
                    CommonErrorCode.SNAPSHOT_RESTORE_STATUS_ERROR, "Snapshot status is not normal");
            } else {
                throw new LegoCheckedException(
                    CommonErrorCode.COPY_LIVE_MOUNT_STATUS_ERROR, "Copy status is not normal");
            }
        }
        if (copy.getGeneration() > IsmNumberConstant.TWO) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "Copy(generation > 2) is not allowed");
        }
        if (copy.getGeneration() == IsmNumberConstant.TWO
                && !CopyGeneratedByEnum.BY_LIVE_MOUNTE.value().equals(copy.getGeneratedBy())) {
            throw new LegoCheckedException(
                    CommonErrorCode.ERR_PARAM, "non-clone copy with generation 2 is not allowed");
        }
    }

    private void executeLiveMountsOnCreated(
            Map.Entry<Copy, List<LiveMountEntity>> entry, LiveMountPolicyEntity policy) {
        Copy sourceCopy = entry.getKey();
        List<LiveMountEntity> liveMountEntities = entry.getValue();
        for (LiveMountEntity liveMountEntity : liveMountEntities) {
            executeLiveMount(liveMountEntity, policy, sourceCopy, null, true);
        }
    }
}
