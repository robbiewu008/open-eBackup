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
package openbackup.access.framework.resource.persistence.dao;

import static openbackup.system.base.common.validator.constants.RegexpConstants.UUID_N0_SEPARATOR;

import openbackup.access.framework.resource.persistence.model.ProtectedAgentExtendPo;
import openbackup.access.framework.resource.persistence.model.ProtectedEnvironmentExtendInfoPo;
import openbackup.access.framework.resource.persistence.model.ProtectedEnvironmentPo;
import openbackup.access.framework.resource.persistence.model.ProtectedResourceExtendInfoPo;
import openbackup.access.framework.resource.persistence.model.ProtectedResourcePo;
import openbackup.access.framework.resource.persistence.model.ResourceDesesitizationPo;
import openbackup.access.framework.resource.persistence.model.ResourceRepositoryQueryParams;
import openbackup.access.framework.resource.persistence.model.ResourcesGroupViewPo;
import openbackup.access.framework.resource.provider.DefaultResourceProvider;
import openbackup.access.framework.resource.service.ProtectedResourceRepository;
import openbackup.data.access.framework.core.dao.ProtectedObjectMapper;
import openbackup.data.access.framework.core.entity.ProtectedObjectPo;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResourceGroupResult;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.data.protection.access.provider.sdk.resource.ResourceDeleteParams;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.data.protection.access.provider.sdk.resource.VstoreResourceQueryParam;
import openbackup.data.protection.access.provider.sdk.resource.model.AgentTypeEnum;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.constants.ErrorCodeConstant;
import openbackup.system.base.common.constants.ResExtendConstant;
import openbackup.system.base.common.enums.ConsistentStatusEnum;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.CollectionUtils;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.StringUtil;
import openbackup.system.base.common.utils.ValidateUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.validator.constants.RegexpConstants;
import com.huawei.oceanprotect.system.base.label.dao.LabelResourceServiceDao;
import openbackup.system.base.query.PageQueryService;
import openbackup.system.base.query.Pagination;
import openbackup.system.base.query.SessionService;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.util.IdUtil;

import com.baomidou.mybatisplus.annotation.TableField;
import com.baomidou.mybatisplus.core.conditions.query.LambdaQueryWrapper;
import com.baomidou.mybatisplus.core.conditions.query.QueryWrapper;
import com.baomidou.mybatisplus.core.conditions.update.LambdaUpdateWrapper;
import com.baomidou.mybatisplus.core.conditions.update.UpdateWrapper;
import com.baomidou.mybatisplus.core.metadata.IPage;
import com.baomidou.mybatisplus.extension.plugins.pagination.Page;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.apache.logging.log4j.util.Strings;
import org.springframework.beans.BeanUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.stereotype.Repository;

import java.lang.reflect.Field;
import java.lang.reflect.Modifier;
import java.sql.Timestamp;
import java.util.AbstractMap;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.Set;
import java.util.UUID;
import java.util.stream.Collectors;

/**
 * 受保护资源存储实现
 *
 */
@Repository
@Slf4j
public class ProtectedResourceRepositoryImpl implements ProtectedResourceRepository {
    /**
     * RESOURCE_ALREADY_BIND_SLA
     */
    public static final long RESOURCE_ALREADY_BIND_SLA = 1677931447L;

    /**
     * 执行删除资源操作时，由于资源已关联智能侦测策略，操作失败。
     */
    public static final long RESOURCE_ALREADY_BIND_INTELLECT_DETECTION_POLICY = 1677747456L;

    /**
     * 一次sql查询最大的参数数量
     */
    private static final int MAX_SQL_QUERY_PARAM_NUM = 30000;

    private static final String EXTEND_INFO_FIELD_PATTERN = "^[$_a-z][$_a-zA-Z0-9]*$";

    private static final String RESOURCE_SET_ID = "resourceSetId";

    private static final String RESOURCE_UUID = "uuid";

    private static final List<String> EXCLUDE_FILED_LIST = Arrays.asList("resourceSetId");

    private static final String ASC = "asc";

    private static final String DESC = "desc";

    private static final String PLUS = "+";

    private static final int ZERO = 0;

    private static final int NOTIFY_SIZE = 5;

    private final ProtectedResourceMapper protectedResourceMapper;

    private final ProtectedResourceExtendInfoMapper protectedResourceExtendInfoMapper;

    private final PageQueryService pageQueryService;

    private final ProtectedEnvironmentExtendInfoMapper protectedEnvironmentExtendInfoMapper;

    private final ProtectedObjectMapper protectedObjectMapper;

    private ProviderManager providerManager;

    private DefaultResourceProvider defaultResourceProvider;

    @Autowired
    private DeployTypeService deployTypeService;

    @Autowired
    private ProtectedResourceAgentMapper protectedResourceAgentMapper;

    @Autowired
    private ProtectedAgentExtendMapper protectedAgentExtendMapper;


    @Autowired
    private SessionService sessionService;

    @Autowired
    private LabelResourceServiceDao labelResourceServiceDao;

    /**
     * constructor
     *
     * @param protectedResourceMapper protectedResourceMapper
     * @param protectedResourceExtendInfoMapper protectedResourceExtendInfoBoMapper
     * @param pageQueryService pageQueryService
     * @param protectedEnvironmentExtendInfoMapper protectedEnvironmentExtendInfoMapper
     * @param protectedObjectMapper protectedObjectMapper
     */
    public ProtectedResourceRepositoryImpl(
            ProtectedResourceMapper protectedResourceMapper,
            ProtectedResourceExtendInfoMapper protectedResourceExtendInfoMapper,
            PageQueryService pageQueryService,
            ProtectedEnvironmentExtendInfoMapper protectedEnvironmentExtendInfoMapper,
            ProtectedObjectMapper protectedObjectMapper) {
        this.protectedResourceMapper = protectedResourceMapper;
        this.protectedResourceExtendInfoMapper = protectedResourceExtendInfoMapper;
        this.pageQueryService = pageQueryService;
        this.protectedEnvironmentExtendInfoMapper = protectedEnvironmentExtendInfoMapper;
        this.protectedObjectMapper = protectedObjectMapper;
    }

    @Autowired
    public void setProviderManager(ProviderManager providerManager) {
        this.providerManager = providerManager;
    }

    @Autowired
    public void setDefaultResourceProvider(
        @Qualifier("defaultResourceProvider") DefaultResourceProvider defaultResourceProvider) {
        this.defaultResourceProvider = defaultResourceProvider;
    }

    private static boolean isTableField(Field field) {
        int modifier = field.getModifiers();
        return !Modifier.isStatic(modifier) && !Modifier.isFinal(modifier);
    }

    private static String getFieldName(Field field) {
        TableField tableField = field.getAnnotation(TableField.class);
        if (tableField == null) {
            return field.getName();
        }
        String fieldName = tableField.value();
        if (fieldName.isEmpty()) {
            return field.getName();
        }
        return fieldName;
    }

    /**
     * create protected resource bo
     *
     * @param resource resource
     * @return resource uuid
     */
    @Override
    public String create(ProtectedResource resource) {
        ProtectedResourcePo protectedResourcePo = ProtectedResourcePo.fromProtectedResource(resource);
        String createdTime = resource.getCreatedTime();
        long timestamp = StringUtils.isNotBlank(createdTime) ? Long.parseLong(createdTime) : System.currentTimeMillis();
        protectedResourcePo.setCreatedTime(new Timestamp(timestamp));
        boolean isEnvironment = resource instanceof ProtectedEnvironment;
        String uuid = insertProtectedResourcePo(protectedResourcePo);
        resource.setUuid(uuid);

        insertProtectedResourceExtendInfoPoList(protectedResourcePo.getExtendInfoList(), uuid);

        if (isEnvironment) {
            saveEnvironmentExtendInfo((ProtectedEnvironment) resource, uuid);
        }
        return uuid;
    }

    private void insertProtectedResourceExtendInfoPoList(
            List<ProtectedResourceExtendInfoPo> extendInfoPoList, String resourceId) {
        List<ProtectedResourceExtendInfoPo> extendInfoPos =
                Optional.ofNullable(extendInfoPoList).orElseGet(ArrayList::new);
        extendInfoPos.removeIf(extendInfoPo -> extendInfoPo.getResourceId() == null && resourceId == null);
        for (ProtectedResourceExtendInfoPo extendInfoPo : extendInfoPos) {
            if (resourceId != null) {
                extendInfoPo.setResourceId(resourceId);
            }
            extendInfoPo.setUuid(UUID.randomUUID().toString());
        }
        extendInfoPos.forEach(this::insertProtectedResourceExtendInfoBo);
    }

    private void saveEnvironmentExtendInfo(ProtectedEnvironment environment, String uuid) {
        ProtectedEnvironmentExtendInfoPo protectedEnvironmentExtendInfoPo =
                ProtectedEnvironmentExtendInfoPo.fromProtectedEnvironment(environment);
        protectedEnvironmentExtendInfoPo.setUuid(uuid);
        if (Objects.isNull(protectedEnvironmentExtendInfoPo.isCluster())) {
            protectedEnvironmentExtendInfoPo.setCluster(false);
        }
        int count = protectedEnvironmentExtendInfoMapper.insert(protectedEnvironmentExtendInfoPo);
        if (count != 1) {
            throw new LegoCheckedException(ErrorCodeConstant.SYSTEM_ERROR, "save environment extend info failed");
        }
    }

    private String insertProtectedResourcePo(ProtectedResourcePo resource) {
        if (resource.getUuid() == null) {
            resource.setUuid(UUID.randomUUID().toString());
        }
        int count = protectedResourceMapper.insert(resource);
        if (count != 1) {
            throw new LegoCheckedException(ErrorCodeConstant.SYSTEM_ERROR, "create resource failed");
        }
        return resource.getUuid();
    }

    private void insertProtectedResourceExtendInfoBo(ProtectedResourceExtendInfoPo protectedResourceExtendInfoPo) {
        int count = protectedResourceExtendInfoMapper.insert(protectedResourceExtendInfoPo);
        if (count != 1) {
            throw new LegoCheckedException(
                    ErrorCodeConstant.SYSTEM_ERROR,
                    "save extend info failed for key " + protectedResourceExtendInfoPo.getKey());
        }
    }

    /**
     * update resource
     *
     * @param resource resource
     * @param isOverwrite truncate
     */
    @Override
    public void update(ProtectedResource resource, boolean isOverwrite) {
        if (VerifyUtil.isEmpty(resource.getUuid())) {
            log.warn("update resource, but resource uuid is none");
            return;
        }
        // 不可更新字段
        cleanUnmodifiableFields(resource);
        ProtectedResourcePo protectedResourcePo = ProtectedResourcePo.fromProtectedResource(resource);
        // 不可更新字段Po
        cleanUnmodifiableFields(protectedResourcePo);

        // 更新资源的固有属性
        if (resource instanceof ProtectedEnvironment) {
            ProtectedEnvironment env = (ProtectedEnvironment) resource;
            log.info("update resource in respository. uuid: {},ip:{}, port:{}, linkStatus:{}", env.getUuid(),
                env.getEndpoint(), env.getPort(), env.getLinkStatus());
        }
        LambdaUpdateWrapper<ProtectedResourcePo> resourcePoLambdaUpdateWrapper =
            new UpdateWrapper<ProtectedResourcePo>().lambda();
        resourcePoLambdaUpdateWrapper.eq(ProtectedResourcePo::getUuid, protectedResourcePo.getUuid())
            .set(ProtectedResourcePo::getUuid, protectedResourcePo.getUuid());
        protectedResourceMapper.update(protectedResourcePo, resourcePoLambdaUpdateWrapper);
        if (resource instanceof ProtectedEnvironment) {
            // 更新环境的固有属性
            updateEnvironment((ProtectedEnvironment) resource);
        }
        // 更新资源已有的扩展属性
        List<ProtectedResourceExtendInfoPo> extendInfoPoListFromDatabase = getExtendInfoPoListFromDatabase(
            protectedResourcePo);
        List<ProtectedResourceExtendInfoPo> oldExtendInfoList =
                selectExistExtendInfoList(protectedResourcePo, extendInfoPoListFromDatabase);
        updateExtendInfoExist(protectedResourcePo, extendInfoPoListFromDatabase);
        // 插入资源新增的扩展属性
        List<ProtectedResourceExtendInfoPo> newExtendInfoList =
            protectedResourcePo.getExtendInfoList().stream()
                .filter(extendInfo -> !oldExtendInfoList.contains(extendInfo))
                .collect(Collectors.toList());
        insertProtectedResourceExtendInfoPoList(newExtendInfoList, protectedResourcePo.getUuid());

        if (isOverwrite) {
            // 清除未指定的扩展属性
            removeRedundantExtendInfos(protectedResourcePo);
        }
    }

    @Override
    public void updateLinkStatus(String uuid, String linkStatus) {
        log.info("Start to updateLinkStatus!uuid:{},linkStatus:{}.", uuid, linkStatus);
        LambdaUpdateWrapper<ProtectedEnvironmentExtendInfoPo> updateWrapper = new LambdaUpdateWrapper<>();
        updateWrapper.set(ProtectedEnvironmentExtendInfoPo::getLinkStatus, linkStatus);
        updateWrapper.eq(ProtectedEnvironmentExtendInfoPo::getUuid, uuid);
        protectedEnvironmentExtendInfoMapper.update(null, updateWrapper);
        ProtectedEnvironmentExtendInfoPo environmentPo = protectedEnvironmentExtendInfoMapper.selectById(uuid);
        log.info("UpdateLinkStatus end!The uuid:{},linkStatus:{}", environmentPo.getUuid(),
            environmentPo.getLinkStatus());
    }

    private void updateExtendInfoExist(ProtectedResourcePo protectedResourcePo,
        List<ProtectedResourceExtendInfoPo> extendInfoPoListFromDatabase) {
        List<ProtectedResourceExtendInfoPo> extendInfoPoListFromInput = protectedResourcePo.getExtendInfoList();
        for (ProtectedResourceExtendInfoPo inputExtendInfo : extendInfoPoListFromInput) {
            for (ProtectedResourceExtendInfoPo dbExtendInfo : extendInfoPoListFromDatabase) {
                if (!Objects.equals(inputExtendInfo.getKey(), dbExtendInfo.getKey())) {
                    continue;
                }
                // key和value都相同，不更新
                if (Objects.equals(inputExtendInfo.getValue(), dbExtendInfo.getValue())) {
                    continue;
                }
                // key相同，value不同，更新
                ProtectedResourceExtendInfoPo update = new ProtectedResourceExtendInfoPo();
                update.setUuid(dbExtendInfo.getUuid());
                update.setValue(inputExtendInfo.getValue());
                protectedResourceExtendInfoMapper.updateById(update);
            }
        }
    }

    private List<ProtectedResourceExtendInfoPo> getExtendInfoPoListFromDatabase(ProtectedResourcePo
            protectedResourcePo) {
        List<ProtectedResourceExtendInfoPo> extendInfoPoListFromInput = protectedResourcePo.getExtendInfoList();
        List<String> keys = getExtendInfoKeys(extendInfoPoListFromInput);
        QueryWrapper<ProtectedResourceExtendInfoPo> wrapper =
            new QueryWrapper<ProtectedResourceExtendInfoPo>()
                .eq("resource_id", protectedResourcePo.getUuid())
                .in(!keys.isEmpty(), "key", keys);
        return protectedResourceExtendInfoMapper.selectList(wrapper);
    }

    private void cleanUnmodifiableFields(ProtectedResource resource) {
        ResourceProvider resourceProvider = providerManager.findProviderOrDefault(ResourceProvider.class, resource,
            defaultResourceProvider);
        resourceProvider.cleanUnmodifiableFieldsWhenUpdate(resource);
    }

    private void cleanUnmodifiableFields(ProtectedResourcePo resource) {
        resource.setDiscriminator(null);
    }

    private void updateEnvironment(ProtectedEnvironment environment) {
        ProtectedEnvironmentExtendInfoPo protectedEnvironmentExtendInfoPo =
                ProtectedEnvironmentExtendInfoPo.fromProtectedEnvironment(environment);
        LambdaUpdateWrapper<ProtectedEnvironmentExtendInfoPo> updateWrapper =
            new UpdateWrapper<ProtectedEnvironmentExtendInfoPo>().lambda();
        updateWrapper.eq(ProtectedEnvironmentExtendInfoPo::getUuid, protectedEnvironmentExtendInfoPo.getUuid())
            .set(ProtectedEnvironmentExtendInfoPo::getUuid, protectedEnvironmentExtendInfoPo.getUuid());
        protectedEnvironmentExtendInfoMapper.update(protectedEnvironmentExtendInfoPo, updateWrapper);
    }

    private void removeRedundantExtendInfos(ProtectedResourcePo protectedResourcePo) {
        List<String> keys = getExtendInfoKeys(protectedResourcePo.getExtendInfoList());
        String resourceId = protectedResourcePo.getUuid();
        QueryWrapper<ProtectedResourceExtendInfoPo> wrapper =
                new QueryWrapper<ProtectedResourceExtendInfoPo>().eq("resource_id", resourceId).notIn("key", keys);
        int count = protectedResourceExtendInfoMapper.delete(wrapper);
        log.debug("truncate {} extend info for {}", count, resourceId);
    }

    private List<ProtectedResourceExtendInfoPo> selectExistExtendInfoList(ProtectedResourcePo protectedResourcePo,
        List<ProtectedResourceExtendInfoPo> extendInfoPoListFromDatabase) {
        List<ProtectedResourceExtendInfoPo> extendInfoPoListFromInput = protectedResourcePo.getExtendInfoList();
        return extendInfoPoListFromInput.stream()
                .filter(extendInfo -> checkProtectedResourceExtendInfoPoExist(extendInfo, extendInfoPoListFromDatabase))
                .collect(Collectors.toList());
    }

    private boolean checkProtectedResourceExtendInfoPoExist(
            ProtectedResourceExtendInfoPo item, List<ProtectedResourceExtendInfoPo> extendInfoPoList) {
        return extendInfoPoList.stream()
                .filter(extendInfoPo -> Objects.equals(item.getKey(), extendInfoPo.getKey()))
                .findFirst()
                .map(
                        extendInfoPo -> {
                            item.setUuid(extendInfoPo.getUuid());
                            return true;
                        })
                .orElse(false);
    }

    private List<String> getExtendInfoKeys(List<ProtectedResourceExtendInfoPo> extendInfoList) {
        return extendInfoList.stream().map(ProtectedResourceExtendInfoPo::getKey).collect(Collectors.toList());
    }

    @Override
    public List<String> delete(ResourceDeleteParams params) {
        boolean shouldDeleteRegister = params.isShouldDeleteRegister();
        String[] resources = params.getResources();
        List<String> resourceIdList =
                Optional.ofNullable(resources).map(Arrays::asList).orElse(Collections.emptyList()).stream()
                        .filter(Objects::nonNull)
                        .collect(Collectors.toList());
        if (resourceIdList.isEmpty()) {
            return Collections.emptyList();
        }
        List<String> relatedResourceUuidList = getRelatedResourceUuids(resourceIdList);
        if (!shouldDeleteRegister) {
            handleRegisterDelete(relatedResourceUuidList);
        }
        if (relatedResourceUuidList.isEmpty()) {
            return Collections.emptyList();
        }
        if (!params.isForce()) {
            checkSlaBond(relatedResourceUuidList);
        }
        deleteRelatedResourcesCitations(relatedResourceUuidList);
        int deletedResourceCount = protectedResourceMapper.deleteBatchIds(relatedResourceUuidList);
        log.debug("delete resource: {}", deletedResourceCount > 0);
        return relatedResourceUuidList;
    }

    @Override
    public List<String> deleteCyberEngineEnvironment(String environmentId) {
        List<String> relatedResourceUuidList = queryResourceUuidsByRootUuidCyberEngine(environmentId);
        relatedResourceUuidList.add(environmentId);
        int offset = 0;
        while (offset < relatedResourceUuidList.size()) {
            List<String> subUuidList = relatedResourceUuidList.subList(offset,
                    Math.min(offset + MAX_SQL_QUERY_PARAM_NUM, relatedResourceUuidList.size()));
            deleteRelatedResourcesCitations(subUuidList);
            int deletedResourceCount = protectedResourceMapper.deleteBatchIds(subUuidList);
            log.debug("delete resource: {}", deletedResourceCount > 0);
            log.info("delete resource count :{}", deletedResourceCount);
            offset += subUuidList.size();
        }
        return relatedResourceUuidList;
    }

    private List<String> getRelatedResourceUuids(List<String> resourceIdList) {
        List<String> toBeDeletedUuids = new ArrayList<>(resourceIdList);
        List<ProtectedResource> protectedResources = query(
            new ResourceRepositoryQueryParams(true, 0, resourceIdList.size(),
                Collections.singletonMap("uuid", resourceIdList), new String[0])).map(
            ProtectedResourcePo::toProtectedResource).getItems();
        // 结果，所有相关的uuid
        Set<String> resRelationUuids = new HashSet<>();
        for (ProtectedResource protectedResource : protectedResources) {
            ResourceProvider resourceProvider = providerManager.findProvider(ResourceProvider.class, protectedResource,
                null);
            Set<String> relationUuids = Optional.ofNullable(resourceProvider)
                .map(e -> e.queryRelationResourceToDelete(protectedResource))
                .orElse(null);
            if (!VerifyUtil.isEmpty(relationUuids)) {
                resRelationUuids.addAll(relationUuids);
                toBeDeletedUuids.remove(protectedResource.getUuid());
            }
        }
        // 这些资源的相关id插件没有返回，走框架查询逻辑
        if (!VerifyUtil.isEmpty(toBeDeletedUuids)) {
            log.info("Query id by framework, size:{}", toBeDeletedUuids.size());
            resRelationUuids.addAll(protectedResourceMapper.queryRelatedResourceUuids(toBeDeletedUuids));
        }
        // 防呆
        resRelationUuids.addAll(resourceIdList);
        return new ArrayList<>(resRelationUuids);
    }

    private void checkSlaBond(List<String> relatedResourceUuidList) {
        LambdaQueryWrapper<ProtectedObjectPo> wrapper =
                new LambdaQueryWrapper<ProtectedObjectPo>().in(ProtectedObjectPo::getUuid, relatedResourceUuidList);
        List<ProtectedObjectPo> protectedObjectPos = protectedObjectMapper.selectList(wrapper);
        if (VerifyUtil.isEmpty(protectedObjectPos)) {
            return ;
        }
        List<String> resourceIdList = protectedObjectPos
                .stream().map(ProtectedObjectPo::getResourceId).collect(Collectors.toList());
        // 每次最多报5个关联的资源名称所以最多只查询5个
        if (resourceIdList.size() > NOTIFY_SIZE) {
            resourceIdList = resourceIdList.subList(ZERO, NOTIFY_SIZE);
        }
        List<ProtectedResourcePo> protectedResourcePos = queryProtectedResource(resourceIdList);
        List<String> resNames = protectedResourcePos
                .stream().map(ProtectedResourcePo::getName).collect(Collectors.toList());
        // 每次最多报5个关联的资源名称，以逗号隔开
        String[] errorParams = new String[]{String.join(",", resNames)};
        if (deployTypeService.isCyberEngine()) {
            throw new LegoCheckedException(RESOURCE_ALREADY_BIND_INTELLECT_DETECTION_POLICY, errorParams,
                "Having resources are bound to intellect detection policies.");
        } else {
            throw new LegoCheckedException(RESOURCE_ALREADY_BIND_SLA, errorParams,
                "Having resources are bound to SLAs.");
        }
    }

    private void handleRegisterDelete(List<String> relatedResourceUuidList) {
        if (VerifyUtil.isEmpty(relatedResourceUuidList)) {
            return;
        }
        List<ProtectedResourcePo> protectedResourcePos = protectedResourceMapper.selectList(
            new QueryWrapper<ProtectedResourcePo>().lambda()
                .in(ProtectedResourcePo::getUuid, relatedResourceUuidList));
        Map<String, String> sourceTypeMap = protectedResourcePos.stream()
            .filter(e -> Objects.nonNull(e.getSourceType()))
            .collect(Collectors.toMap(ProtectedResourcePo::getUuid, ProtectedResourcePo::getSourceType));
        relatedResourceUuidList.removeIf(
            uuid -> Objects.equals(sourceTypeMap.get(uuid), ResourceConstants.SOURCE_TYPE_REGISTER));
    }

    private void deleteRelatedResourcesCitations(List<String> relatedResourceUuidList) {
        protectedResourceExtendInfoMapper.delete(new QueryWrapper<ProtectedResourceExtendInfoPo>().likeRight("KEY",
            ResourceConstants.CITATION + ResourceConstants.CITATION_SEPERATOR).in("VALUE", relatedResourceUuidList));
    }

    /**
     * query page data
     *
     * @param page page code
     * @param size page size
     * @param conditions conditions
     * @param orders orders
     * @return page data
     */
    @Override
    public BasePage<ProtectedResourcePo> query(int page, int size, Map<String, Object> conditions, String... orders) {
        ResourceRepositoryQueryParams params = new ResourceRepositoryQueryParams(false,
            page, size, conditions, orders);
        return query(params);
    }

    @Override
    public BasePage<ProtectedResourcePo> query(ResourceRepositoryQueryParams params) {
        Map<String, Object> validConditions =
            new HashMap<>(Optional.ofNullable(params.getConditions()).orElse(Collections.emptyMap()));
        validConditions.entrySet().removeIf(entry -> !entry.getKey().matches(EXTEND_INFO_FIELD_PATTERN));
        Pagination<Map<String, Object>> pagination =
            new Pagination<>(params.getPage(), params.getSize(), validConditions, Arrays.asList(params.getOrders()));
        List<String> extendInfoFields = getExtendInfoFields(validConditions);
        String extendCascades = extendInfoFields.stream()
                .map(this::buildJoinClause).collect(Collectors.joining(" "));

        return query(pagination, extendCascades, params);
    }

    private BasePage<ProtectedResourcePo> query(Pagination<Map<String, Object>> pagination, String extendCascades,
            ResourceRepositoryQueryParams params) {
        Map<String, Object> conditions = pagination.getConditions();
        final Map<String, Object> desesitizationConditions = new HashMap<>();
        Object desesitization = conditions.remove("resourceDesesitization");
        if (params.isDesesitization()) {
            desesitizationConditions.putAll(obtainCascadeConditions(desesitization, ResourceDesesitizationPo.class));
        }
        Map<String, Object> labelConditions = handleLabelConditions(conditions);
        Map<String, Object> protectedObjectConditions =
            obtainCascadeConditions(conditions.remove("protectedObject"), ProtectedObjectPo.class);
        Map<String, Object> rootResourceConditions =
            obtainCascadeConditions(conditions.get("environment"), ProtectedResourcePo.class);
        Map<String, Object> rootEnvironmentConditions =
            obtainCascadeConditions(conditions.remove("environment"), ProtectedEnvironmentPo.class);
        Map<String, Object> resourceConditions = obtainConditions(conditions, ProtectedResourcePo.class);
        Map<String, Object> environmentConditions = obtainConditions(conditions, ProtectedEnvironmentPo.class);
        String resourceSetId = getValueFromCondition(RESOURCE_SET_ID, conditions);
        filterExcludeFieldsMap(conditions);
        Map<String, Object> extendInfoConditions = new HashMap<>(conditions);
        conditions.clear();
        String defaultOrder = "-r.createdTime,+r.name,+r.uuid";
        if (deployTypeService.isCyberEngine()) {
            defaultOrder = "-r.createdTime,+r.name,+r.rootUuid";
        }
        return pageQueryService.pageQuery(
                ProtectedResourcePo.class,
                (page, wrapper) -> {
                    QueryWrapper<ProtectedResourcePo> queryWrapper = wrapper;
                    addResourceSetIdCondition(wrapper, resourceSetId);
                    addResourceLabelCondition(queryWrapper, labelConditions);
                    queryWrapper = addQueryCondition(queryWrapper, resourceConditions, new String[] {"r.%s"});
                    queryWrapper =
                            addQueryCondition(queryWrapper, environmentConditions, new String[] {"e.%s", "f_%s.value"});
                    queryWrapper = addQueryCondition(queryWrapper, extendInfoConditions, new String[] {"f_%s.value"});
                    queryWrapper = addQueryCondition(queryWrapper, rootResourceConditions, new String[] {"rr.%s"});
                    queryWrapper = addQueryCondition(queryWrapper, rootEnvironmentConditions, new String[] {"re.%s"});
                    if (params.isDesesitization()) {
                        queryWrapper = addQueryCondition(queryWrapper, desesitizationConditions,
                            new String[] {"rd.%s"});
                        return castAsProtectedResourcePoPage(
                                protectedResourceMapper.desesitizationPaginate(page, queryWrapper, extendCascades));
                    }
                    queryWrapper = addQueryCondition(queryWrapper, protectedObjectConditions, new String[] {"po.%s"});
                    return castAsProtectedResourcePoPage(
                            protectedResourceMapper.paginate(page, queryWrapper, extendCascades));
                },
                pagination, defaultOrder, params.isShouldIgnoreOwner() ? null : "r.uuid");
    }

    private void addResourceLabelListCondition(QueryWrapper<ProtectedResourcePo> wrapper,
        Map<String, Object> labelConditions) {
        if (VerifyUtil.isEmpty(labelConditions)) {
            return;
        }
        List<String> labelList = (List<String>) labelConditions.get("labelList");
        if (VerifyUtil.isEmpty(labelList)) {
            return;
        }
        StringBuilder resourceIdSqlBuilder = new StringBuilder();
        resourceIdSqlBuilder.append(
            "select tlb.resource_object_id from t_label_r_resource_object tlb " + "where tlb.label_id in (");
        for (int i = 0; i < labelList.size(); i++) {
            // uuid防注入
            if (!ValidateUtil.match(UUID_N0_SEPARATOR, labelList.get(i))) {
                continue;
            }
            resourceIdSqlBuilder.append("'").append(labelList.get(i)).append("'");
            if (i < (labelList.size() - 1)) {
                resourceIdSqlBuilder.append(",");
            }
        }
        resourceIdSqlBuilder.append(
                ") " + "group by tlb.resource_object_id " + "having count(distinct tlb.label_id) = ")
            .append(labelList.size());
        wrapper.inSql("r.uuid", resourceIdSqlBuilder.toString());
    }

    private void addResourceLabelCondition(QueryWrapper<ProtectedResourcePo> wrapper,
        Map<String, Object> labelConditions) {
        if (VerifyUtil.isEmpty(labelConditions)) {
            return;
        }
        addLabelNameCondition(wrapper, labelConditions);
        addResourceLabelListCondition(wrapper, labelConditions);
        addResourceLabelEnvironmentListCondition(wrapper, labelConditions);
    }

    private void addLabelNameCondition(QueryWrapper<ProtectedResourcePo> wrapper, Map<String, Object> labelConditions) {
        Object labelName = labelConditions.get("labelName");
        boolean isLabelNameValid = false;

        StringBuilder resourceIdSqlBuilder = new StringBuilder();
        resourceIdSqlBuilder.append(
            "select distinct tlb.resource_object_id from t_label_r_resource_object tlb,t_label tl "
                + "where tl.uuid=tlb.label_id");
        if (!VerifyUtil.isEmpty(labelName)) {
            if (ValidateUtil.match(RegexpConstants.NAME_STR, labelName.toString())) {
                resourceIdSqlBuilder.append(" and tl.name like " + "'%").append(labelName).append("%'");
                isLabelNameValid = true;
            } else {
                log.warn("labelName({}) do not match regex", labelName);
            }
        }
        if (isLabelNameValid) {
            wrapper.inSql("r.uuid", resourceIdSqlBuilder.toString());
        }
    }

    private void addResourceLabelEnvironmentListCondition(QueryWrapper<ProtectedResourcePo> wrapper,
        Map<String, Object> labelConditions) {
        if (VerifyUtil.isEmpty(labelConditions)) {
            return;
        }
        List<String> labelList = (List<String>) labelConditions.get("labelEnvironmentList");
        if (VerifyUtil.isEmpty(labelList)) {
            return;
        }
        // 根据标签id查出对应的agent的uuid
        List<String> stringList = new ArrayList<>();
        List<String> idList = labelResourceServiceDao.selectResourceIdByLabelIds(labelList, stringList);
        StringBuilder resourceIdSqlBuilder = new StringBuilder();
        if (VerifyUtil.isEmpty(idList)) {
            // 如果ipList为空,设置搜不出来
            resourceIdSqlBuilder.append("SELECT NULL WHERE 1 = 0");
        } else {
            resourceIdSqlBuilder.append("SELECT res.uuid FROM resources res WHERE res.root_uuid IN (");
            for (int i = 0; i < idList.size(); i++) {
                resourceIdSqlBuilder.append("'").append(idList.get(i)).append("'");
                if (i < (idList.size() - 1)) {
                    resourceIdSqlBuilder.append(",");
                }
            }
            resourceIdSqlBuilder.append(")");
        }
        wrapper.inSql("r.uuid", resourceIdSqlBuilder.toString());
    }

    private Map<String, Object> handleLabelConditions(Map<String, Object> conditions) {
        Map<String, Object> labelConditions = new HashMap<>();
        Object labelConditionMap = conditions.remove("labelCondition");
        if (VerifyUtil.isEmpty(labelConditionMap) || !(labelConditionMap instanceof Map)) {
            return labelConditions;
        }
        Object labelName = ((Map) labelConditionMap).get("labelName");
        Object labelList = ((Map) labelConditionMap).get("labelList");
        Object labelEnvironmentList = ((Map) labelConditionMap).get("labelEnvironmentList");
        if (!VerifyUtil.isEmpty(labelName) && (labelName instanceof String)) {
            labelConditions.put("labelName", labelName);
        }
        if (!VerifyUtil.isEmpty(labelList) && (labelList instanceof List<?>)) {
            List<?> list = (List<?>) labelList;
            if (!list.isEmpty() && list.get(0) instanceof String) {
                labelConditions.put("labelList", labelList);
            }
        }
        if (!VerifyUtil.isEmpty(labelEnvironmentList) && (labelEnvironmentList instanceof List<?>)) {
            List<?> envList = (List<?>) labelEnvironmentList;
            if (!envList.isEmpty() && envList.get(0) instanceof String) {
                labelConditions.put("labelEnvironmentList", labelEnvironmentList);
            }
        }
        return labelConditions;
    }

    private void addResourceSetIdCondition(QueryWrapper<ProtectedResourcePo> wrapper, String resourceSetId) {
        if (Strings.isEmpty(resourceSetId)) {
            return;
        }
        if (!IdUtil.isUUID(resourceSetId)) {
            throw new LegoCheckedException(ErrorCodeConstant.ERR_PARAM, "resource set id incorrect");
        }
        wrapper.inSql("r.uuid", "SELECT resource_object_id FROM t_resource_set_r_resource_object "
            + "where resource_set_id = '" + resourceSetId + "'");
    }

    private String getValueFromCondition(String key, Map<String, Object> conditions) {
        if (VerifyUtil.isEmpty(conditions) || !conditions.containsKey(RESOURCE_SET_ID)) {
            return StringUtils.EMPTY;
        }
        Object value = conditions.get(key);
        if (value instanceof String) {
            return (String) value;
        }
        return StringUtils.EMPTY;
    }

    /**
     * query page data group by key corresponding value
     *
     * @param param param
     * @return page data
     */
    @Override
    public BasePage<ProtectedResourceGroupResult> groupQuery(VstoreResourceQueryParam param) {
        int page = param.getPage();
        int size = param.getSize();
        String key = param.getKey();
        String order = param.getOrder();
        String vstoreNameFilter = param.getVstoreNameFilter();
        String orderByFsNum = param.getOrderByFsNum();
        List<ResourcesGroupViewPo> groupExtendInfos;
        if ("tenantName".equals(key)) {
            groupExtendInfos =
                protectedResourceExtendInfoMapper.groupByTenantName(size, size * page,
                    vstoreNameFilter, orderByFsNum);
        } else {
            groupExtendInfos =
                protectedResourceExtendInfoMapper.groupByValue(size, size * page,
                    key, (PLUS.equals(order)) ? ASC : DESC);
        }

        QueryWrapper<ProtectedResourceExtendInfoPo> wrapper = new QueryWrapper<>();
        wrapper.select("count(*) as count, value");
        wrapper.eq("key", key);
        wrapper.groupBy("value");

        List<Map<String, Object>> list = protectedResourceExtendInfoMapper.selectMaps(wrapper);
        int total = list.size();
        int pages = total / size + (total % size == 0 ? 0 : 1);
        log.debug("group by value total:{}, pages:{}", total, pages);

        boolean isSearchProtectObject = param.isSearchProtectObject();
        List<ProtectedResourceGroupResult> resData = this.getGroupResData(key, groupExtendInfos, isSearchProtectObject);
        BasePage<ProtectedResourceGroupResult> basePage = new BasePage<>();
        basePage.setPageNo(page);
        basePage.setPageSize(size);
        basePage.setTotal(Long.parseLong(String.valueOf(total)));
        basePage.setPages(Long.parseLong(String.valueOf(pages)));
        basePage.setItems(resData);
        return basePage;
    }

    private void filterExcludeFieldsMap(Map<String, Object> conditions) {
        conditions.keySet().removeIf(EXCLUDE_FILED_LIST::contains);
    }

    private List<ProtectedResourceGroupResult> getGroupResData(String key,
            List<ResourcesGroupViewPo> groupExtendInfos, boolean isSearchProtectObject) {
        List<ProtectedResourceGroupResult> resData = new ArrayList<>();
        if (groupExtendInfos.isEmpty()) {
            return Optional.ofNullable(resData).orElse(Collections.emptyList());
        }
        for (ResourcesGroupViewPo resourcesGroupViewPo : groupExtendInfos) {
            List<String> resourceIdList = Arrays.asList(resourcesGroupViewPo.getResourceIds().split(","));
            resourceIdList = resourceIdList.stream().distinct().collect(Collectors.toList());
            List<ProtectedResourcePo> resourcePos = protectedResourceMapper.selectBatchIds(resourceIdList);
            resourcePos = this.getRelativeMes(isSearchProtectObject, resourcePos);
            ProtectedResourceGroupResult data = new ProtectedResourceGroupResult();
            data.setKey(key);
            data.setValue(resourcesGroupViewPo.getValue());
            data.setResources(resourcePos.stream().map(ProtectedResourcePo::toProtectedResource).
                    collect(Collectors.toList()));
            resData.add(data);
        }
        return Optional.ofNullable(resData).orElse(Collections.emptyList());
    }

    private List<ProtectedResourcePo> getRelativeMes(boolean isSearchProtectObject,
            List<ProtectedResourcePo> resourcePos) {
        if (resourcePos.isEmpty()) {
            return resourcePos;
        }
        for (ProtectedResourcePo resourcePo : resourcePos) {
            QueryWrapper<ProtectedResourceExtendInfoPo> extendWrapper =
                    new QueryWrapper<ProtectedResourceExtendInfoPo>();
            extendWrapper.eq("resource_id", resourcePo.getUuid());
            List<ProtectedResourceExtendInfoPo> extendInfoPos =
                    protectedResourceExtendInfoMapper.selectList(extendWrapper);
            resourcePo.setExtendInfoList(extendInfoPos);
            if (isSearchProtectObject) {
                QueryWrapper<ProtectedObjectPo> wrapper = new QueryWrapper<ProtectedObjectPo>();
                wrapper.eq("resource_id", resourcePo.getUuid());
                ProtectedObjectPo objectPo = protectedObjectMapper.selectOne(wrapper);
                resourcePo.setProtectedObjectPo(objectPo);
            }
        }
        return resourcePos;
    }


    private String buildJoinClause(String field) {
        String alias = StringUtil.mapCamelCaseToUnderscore(field);
        return String.format(
                Locale.ENGLISH,
                "left join %s as f_%s on f_%s.resource_id = r.uuid and f_%s.key='%s'",
                ProtectedResourceExtendInfoPo.TABLE_NAME,
                alias,
                alias,
                alias,
                field);
    }

    private Map<String, Object> obtainCascadeConditions(Object value, Class<?> type) {
        if (!(value instanceof Map)) {
            return Collections.emptyMap();
        }
        @SuppressWarnings("unchecked")
        Map<Object, Object> originParams = (Map<Object, Object>) value;
        Map<String, Object> validParams =
                originParams.entrySet().stream()
                        .filter(entry -> entry.getKey() instanceof String)
                        .map(entry -> new AbstractMap.SimpleEntry<>((String) entry.getKey(), entry.getValue()))
                        .collect(Collectors.toMap(Map.Entry::getKey, Map.Entry::getValue));
        Map<String, Object> result = obtainConditions(validParams, type);
        List<Object> keys = new ArrayList<>(result.keySet());
        originParams.keySet().removeIf(keys::contains);
        return result;
    }

    private QueryWrapper<ProtectedResourcePo> addQueryCondition(
            QueryWrapper<ProtectedResourcePo> wrapper, Map<String, Object> environmentConditions, String[] ts) {
        QueryWrapper<ProtectedResourcePo> temp = wrapper;
        for (Map.Entry<String, Object> item : environmentConditions.entrySet()) {
            if (ts.length == 1) {
                temp = addQueryCondition(temp, item, ts[0]);
            } else if (ts.length - 1 == 1) {
                temp = temp.and(qw -> addQueryCondition(qw, item, ts[0]).or(sq -> addQueryCondition(sq, item, ts[1])));
            } else {
                log.trace("ignore");
            }
        }
        return temp;
    }

    private QueryWrapper<ProtectedResourcePo> addQueryCondition(
            QueryWrapper<ProtectedResourcePo> queryWrapper, Map.Entry<String, Object> entry, String template) {
        String key = String.format(Locale.ENGLISH, template, entry.getKey());
        Object value = entry.getValue();
        return pageQueryService.addQueryCondition(queryWrapper, key, value);
    }

    private Map<String, Object> obtainConditions(Map<String, Object> conditions, Class<?> type) {
        List<String> fields =
                Arrays.stream(type.getDeclaredFields())
                        .filter(ProtectedResourceRepositoryImpl::isTableField)
                        .map(ProtectedResourceRepositoryImpl::getFieldName)
                        .collect(Collectors.toList());
        return obtainConditions(conditions, fields);
    }

    private Map<String, Object> obtainConditions(Map<String, Object> conditions, Collection<String> fields) {
        Map<String, Object> map = new HashMap<>();
        for (String field : fields) {
            Object condition = conditions.get(field);
            if (condition != null) {
                map.put(field, condition);
            }
        }
        new ArrayList<>(fields).forEach(conditions::remove);
        return map;
    }

    private IPage<ProtectedResourcePo> castAsProtectedResourcePoPage(IPage<ProtectedEnvironmentPo> source) {
        List<ProtectedResourcePo> resources =
                source.getRecords().stream()
                        .map(
                                environment -> {
                                    if (ProtectedResourcePo.ENVIRONMENTS_DISCRIMINATOR.equals(
                                            environment.getDiscriminator())) {
                                        return environment;
                                    }
                                    ProtectedResourcePo resource = new ProtectedResourcePo();
                                    BeanUtils.copyProperties(environment, resource);
                                    return resource;
                                })
                        .collect(Collectors.toList());
        IPage<ProtectedResourcePo> target = new Page<>(source.getCurrent(), source.getSize(), source.getTotal(), true);
        target.setRecords(resources);
        return target;
    }

    private List<String> getExtendInfoFields(Map<String, Object> conditions) {
        return filterExcludeFields(getMissingFields(conditions.keySet(), ProtectedResourcePo.class));
    }

    private List<String> filterExcludeFields(List<String> fieldList) {
        return fieldList.stream().filter(filed -> !EXCLUDE_FILED_LIST.contains(filed)).collect(Collectors.toList());
    }

    private List<String> getMissingFields(Collection<String> items, Class<?>... types) {
        List<String> fields = new ArrayList<>();
        loop:
        for (String key : items) {
            for (Class<?> type : types) {
                try {
                    type.getDeclaredField(key);
                    continue loop;
                } catch (NoSuchFieldException e) {
                    log.trace("no such field", e);
                }
            }
            fields.add(key);
        }
        return fields;
    }

    /**
     * query related resource uuids
     *
     * @param parentUuids parent uuids
     * @param excludeResourceUuids exclude resource uuids
     * @return related resource uuids
     */
    @Override
    public Set<String> queryRelatedResourceUuids(List<String> parentUuids, String... excludeResourceUuids) {
        List<String> uuids =
                protectedResourceMapper.queryRelatedResourceUuids(
                        Optional.ofNullable(parentUuids).orElse(Collections.emptyList()).stream()
                                .filter(Objects::nonNull)
                                .collect(Collectors.toList()),
                        CollectionUtils.nonNullList(excludeResourceUuids));
        return new HashSet<>(uuids);
    }

    /**
     * query resource uuids
     *
     * @param pagination pagination
     * @return result
     */
    @Override
    public BasePage<String> queryResourceUuids(Pagination<JSONObject> pagination) {
        return pageQueryService.pageQuery(
                ProtectedResourcePo.class, protectedResourceMapper::queryResourceUuids, pagination, null);
    }

    @Override
    public List<ProtectedResourcePo> queryDependencyResources(String key, List<String> uuids) {
        List<ProtectedEnvironmentPo> protectedEnvironmentPos = protectedResourceMapper.queryDependencyResources(key,
            uuids);
        return new ArrayList<>(protectedEnvironmentPos);
    }

    @Override
    public List<ProtectedResourcePo> queryResourcesByUserId(String userId) {
        List<ProtectedEnvironmentPo> protectedEnvironmentPos = protectedResourceMapper.queryResourcesByUserId(userId);
        return new ArrayList<>(protectedEnvironmentPos);
    }

    @Override
    public List<ProtectedResourcePo> queryAgentResourceList(Map<String, Object> map) {
        return new ArrayList<>(protectedResourceAgentMapper.queryAgentResourceList(map));
    }

    @Override
    public int queryAgentResourceCount(Map<String, Object> map) {
        return protectedResourceAgentMapper.queryAgentResourceCount(map);
    }

    @Override
    public List<String> queryExistResourceUuidBySourceType(List<String> uuidList, String sourceType) {
        if (org.springframework.util.CollectionUtils.isEmpty(uuidList)) {
            return Collections.emptyList();
        }
        return protectedResourceMapper.queryExistResourceUuidBySourceType(uuidList, sourceType);
    }

    @Override
    public List<String> queryResourceUuidsByRootUuidCyberEngine(String parentUuid) {
        if (StringUtils.isEmpty(parentUuid)) {
            return Collections.emptyList();
        }
        return protectedResourceMapper.queryResourceUuidsByRootUuidCyberEngine(parentUuid);
    }

    /**
     * 根据uuid集合查询受保护的资源
     *
     * @param uuids uuid集合
     * @return 受保护资源集合
     */
    @Override
    public List<ProtectedObjectPo> queryProtectedObject(List<String> uuids) {
        if (VerifyUtil.isEmpty(uuids)) {
            return Collections.emptyList();
        }
        LambdaQueryWrapper<ProtectedObjectPo> wrapper =
                new LambdaQueryWrapper<ProtectedObjectPo>().in(ProtectedObjectPo::getUuid, uuids);
        return protectedObjectMapper.selectList(wrapper);
    }

    @Override
    public Integer queryProtectObjectCountByConsistentStatus(ConsistentStatusEnum status) {
        LambdaQueryWrapper<ProtectedObjectPo> wrapper = new LambdaQueryWrapper<ProtectedObjectPo>()
                .eq(ProtectedObjectPo::getConsistentStatus, status.getStatus());
        return protectedObjectMapper.selectCount(wrapper).intValue();
    }

    @Override
    public List<String> queryProtectObjectIdListByConsistentStatus(ConsistentStatusEnum status) {
        LambdaQueryWrapper<ProtectedObjectPo> wrapper = new LambdaQueryWrapper<ProtectedObjectPo>()
                .eq(ProtectedObjectPo::getConsistentStatus, status.getStatus());
        return protectedObjectMapper.selectList(wrapper).stream()
                .map(ProtectedObjectPo::getUuid)
                .collect(Collectors.toList());
    }

    @Override
    public ProtectedObjectPo queryProtectObjectById(String uuid) {
        LambdaQueryWrapper<ProtectedObjectPo> wrapper = new LambdaQueryWrapper<ProtectedObjectPo>()
                .eq(ProtectedObjectPo::getUuid, uuid);
        return protectedObjectMapper.selectOne(wrapper);
    }

    @Override
    public List<ProtectedResourcePo> queryProtectedResource(List<String> uuids) {
        if (VerifyUtil.isEmpty(uuids)) {
            return Collections.emptyList();
        }
        LambdaQueryWrapper<ProtectedResourcePo> resourceWrapper =
                new LambdaQueryWrapper<ProtectedResourcePo>().in(ProtectedResourcePo::getUuid, uuids);
        return protectedResourceMapper.selectList(resourceWrapper);
    }

    @Override
    public void updateAllProtectObjectConsistentStatus(ConsistentStatusEnum status) {
        LambdaUpdateWrapper<ProtectedObjectPo> wrapper = new LambdaUpdateWrapper<ProtectedObjectPo>()
                .set(ProtectedObjectPo::getConsistentStatus, status.getStatus())
                .set(ProtectedObjectPo::getConsistentResults, StringUtils.EMPTY);
        protectedObjectMapper.update(null, wrapper);
    }

    @Override
    public void updateProtectObjectConsistentById(String uuid, ConsistentStatusEnum status, String consistentResults) {
        LambdaUpdateWrapper<ProtectedObjectPo> wrapper = new LambdaUpdateWrapper<ProtectedObjectPo>()
                .eq(ProtectedObjectPo::getUuid, uuid)
                .set(ProtectedObjectPo::getConsistentStatus, status.getStatus())
                .set(ProtectedObjectPo::getConsistentResults, consistentResults);
        protectedObjectMapper.update(null, wrapper);
    }

    @Override
    public List<ProtectedResource> queryAllResourceIdsByPathAndRootUuid(String path, String rootUuid) {
        LambdaQueryWrapper<ProtectedResourcePo> wrapper = new LambdaQueryWrapper<ProtectedResourcePo>()
                .eq(ProtectedResourcePo::getRootUuid, rootUuid)
                .likeRight(ProtectedResourcePo::getPath, path);

        return protectedResourceMapper.selectList(wrapper).stream()
                .map(protectedResourcePo -> {
                    ProtectedResource resource = new ProtectedResource();
                    BeanUtils.copyProperties(protectedResourcePo, resource);
                    return resource;
                }).collect(Collectors.toList());
    }

    @Override
    public void updateAgentShared(String uuid, Boolean isShared) {
        // 不传共享参数，默认为false
        saveOrUpdateHostAgentShared(uuid, isShared != null && isShared);
    }

    private void saveOrUpdateHostAgentShared(String uuid, Boolean isShared) {
        LambdaQueryWrapper<ProtectedAgentExtendPo> queryWrapper = new LambdaQueryWrapper<>();
        queryWrapper.eq(ProtectedAgentExtendPo::getUuid, uuid);
        ProtectedAgentExtendPo oldProtectedAgentExtendPo = protectedAgentExtendMapper.selectOne(queryWrapper);
        if (oldProtectedAgentExtendPo == null) {
            oldProtectedAgentExtendPo = new ProtectedAgentExtendPo();
            oldProtectedAgentExtendPo.setUuid(uuid);
            oldProtectedAgentExtendPo.setIsShared(isShared);
            oldProtectedAgentExtendPo.setLastUpdateTime(System.currentTimeMillis());
            protectedAgentExtendMapper.insert(oldProtectedAgentExtendPo);
        } else {
            oldProtectedAgentExtendPo.setIsShared(isShared);
            oldProtectedAgentExtendPo.setLastUpdateTime(System.currentTimeMillis());
            protectedAgentExtendMapper.updateById(oldProtectedAgentExtendPo);
        }
    }

    @Override
    public void updateResExtendDomain(String uuid, String dmeDomains) {
        saveOrUpdate(uuid, dmeDomains, ResExtendConstant.AGENT_DOMAIN_AVAILABLE_IP);
    }

    @Override
    public void updateResExtendCpuAndMemRate(ProtectedAgentExtendPo curAgentExtendInfo) {
        saveOrUpdateHostAgentInfo(curAgentExtendInfo);
    }

    /**
     * 查询agent拓展信息
     *
     * @param uuid uuid
     * @return ProtectedAgentExtendPo
     */
    @Override
    public ProtectedAgentExtendPo queryProtectedAgentExtendByUuid(String uuid) {
        return protectedAgentExtendMapper.selectById(uuid);
    }

    @Override
    public List<ProtectedResourceExtendInfoPo> queryExtendInfoListByResourceIdAndKey(String resUuid, String key) {
        QueryWrapper<ProtectedResourceExtendInfoPo> resQueryWrapper = new QueryWrapper<>();
        resQueryWrapper.eq("resource_id", resUuid).eq("key", key);
        return protectedResourceExtendInfoMapper.selectList(resQueryWrapper);
    }

    @Override
    public void saveOrUpdate(String uuid, String value, String keyName) {
        LambdaQueryWrapper<ProtectedResourceExtendInfoPo> queryWrapper = new LambdaQueryWrapper<>();
        queryWrapper.eq(ProtectedResourceExtendInfoPo::getResourceId, uuid)
            .eq(ProtectedResourceExtendInfoPo::getKey, keyName);
        ProtectedResourceExtendInfoPo oldIpsExtendInfo = protectedResourceExtendInfoMapper.selectOne(queryWrapper);
        if (oldIpsExtendInfo == null) {
            oldIpsExtendInfo = new ProtectedResourceExtendInfoPo();
            oldIpsExtendInfo.setUuid(UUID.randomUUID().toString());
            oldIpsExtendInfo.setResourceId(uuid);
            oldIpsExtendInfo.setKey(keyName);
            oldIpsExtendInfo.setValue(value);
            protectedResourceExtendInfoMapper.insert(oldIpsExtendInfo);
        } else {
            oldIpsExtendInfo.setValue(value);
            protectedResourceExtendInfoMapper.updateById(oldIpsExtendInfo);
        }
    }

    private void saveOrUpdateHostAgentInfo(ProtectedAgentExtendPo curAgentExtendInfo) {
        LambdaQueryWrapper<ProtectedAgentExtendPo> queryWrapper = new LambdaQueryWrapper<>();
        queryWrapper.eq(ProtectedAgentExtendPo::getUuid, curAgentExtendInfo.getUuid());
        ProtectedAgentExtendPo oldProtectedAgentExtendPo = protectedAgentExtendMapper.selectOne(queryWrapper);
        if (oldProtectedAgentExtendPo == null) {
            oldProtectedAgentExtendPo = new ProtectedAgentExtendPo();
            oldProtectedAgentExtendPo.setUuid(curAgentExtendInfo.getUuid());
            buildOldAgentExtendPo(curAgentExtendInfo, oldProtectedAgentExtendPo);
            protectedAgentExtendMapper.insert(oldProtectedAgentExtendPo);
        } else {
            buildOldAgentExtendPo(curAgentExtendInfo, oldProtectedAgentExtendPo);
            protectedAgentExtendMapper.updateById(oldProtectedAgentExtendPo);
        }
    }

    private void buildOldAgentExtendPo(ProtectedAgentExtendPo curAgentExtendInfo,
        ProtectedAgentExtendPo oldProtectedAgentExtendPo) {
        oldProtectedAgentExtendPo.setCpuRate(curAgentExtendInfo.getCpuRate());
        oldProtectedAgentExtendPo.setMemRate(curAgentExtendInfo.getMemRate());
        oldProtectedAgentExtendPo.setCpuRateAlarmThresholdCount(curAgentExtendInfo.getCpuRateAlarmThresholdCount());
        oldProtectedAgentExtendPo.setCpuRateClearAlarmThresholdCount(
            curAgentExtendInfo.getCpuRateClearAlarmThresholdCount());
        oldProtectedAgentExtendPo.setMemRateAlarmThresholdCount(curAgentExtendInfo.getMemRateAlarmThresholdCount());
        oldProtectedAgentExtendPo.setMemRateClearAlarmThresholdCount(
            curAgentExtendInfo.getMemRateClearAlarmThresholdCount());
        oldProtectedAgentExtendPo.setSendMemRateAlarmThreshold(curAgentExtendInfo.getSendMemRateAlarmThreshold());
        oldProtectedAgentExtendPo.setSendCpuRateAlarmThreshold(curAgentExtendInfo.getSendCpuRateAlarmThreshold());
        oldProtectedAgentExtendPo.setLastUpdateTime(System.currentTimeMillis());
    }

    @Override
    public void updateUserId(String uuid, String userId, String authorizedUserName) {
        log.info("Start to updateUserId!uuid:{}.", uuid);
        LambdaUpdateWrapper<ProtectedResourcePo> updateWrapper = new LambdaUpdateWrapper<>();
        updateWrapper.set(ProtectedResourcePo::getUserId, userId);
        updateWrapper.set(ProtectedResourcePo::getAuthorizedUser, authorizedUserName);
        updateWrapper.eq(ProtectedResourcePo::getUuid, uuid);
        protectedResourceMapper.update(null, updateWrapper);
    }

    /**
     * 根据key和资源id删除资源拓展信息
     *
     * @param resourceId resourceId
     * @param key key
     */
    @Override
    public void deleteProtectResourceExtendInfoByResourceId(String resourceId, String key) {
        if (VerifyUtil.isEmpty(resourceId) || VerifyUtil.isEmpty(key)) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "param can not null!");
        }
        LambdaQueryWrapper<ProtectedResourceExtendInfoPo> queryWrapper = new LambdaQueryWrapper<>();
        queryWrapper.eq(ProtectedResourceExtendInfoPo::getResourceId, resourceId);
        queryWrapper.eq(ProtectedResourceExtendInfoPo::getKey, key);
        protectedResourceExtendInfoMapper.delete(queryWrapper);
    }

    @Override
    public void legoHostSighWithOldPrivateKey() {
        Map<String, Object> param = new HashMap<>();
        param.put("type", "Host");
        param.put("scenario", AgentTypeEnum.EXTERNAL_AGENT.getValue());
        param.put("isCluster", false);
        param.put("pageNo", 0);
        param.put("pageSize", 20000);
        // 查询在线主机
        List<ProtectedResourcePo> protectedResourcePos = this.queryAgentResourceList(param);
        log.info("legoHostSighWithOld protectedResourcePos size: {}", protectedResourcePos.size());
        protectedResourcePos.forEach(protectedResourcePo -> {
            log.info("legoHostSighWithOld uuid: {}", protectedResourcePo.getUuid());
            ProtectedResourceExtendInfoPo resourceExtendInfoPo = new ProtectedResourceExtendInfoPo();
            resourceExtendInfoPo.setUuid(UUID.randomUUID().toString());
            resourceExtendInfoPo.setResourceId(protectedResourcePo.getUuid());
            resourceExtendInfoPo.setKey(Constants.USE_OLD_PRIVATE);
            resourceExtendInfoPo.setValue("1");
            this.insertProtectedResourceExtendInfoBo(resourceExtendInfoPo);
        });
    }
}
