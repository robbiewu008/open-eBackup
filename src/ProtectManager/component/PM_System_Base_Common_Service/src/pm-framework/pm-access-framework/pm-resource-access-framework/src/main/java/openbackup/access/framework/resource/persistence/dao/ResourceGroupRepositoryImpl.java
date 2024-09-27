/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.access.framework.resource.persistence.dao;

import openbackup.access.framework.resource.persistence.model.ResourceGroupExtendField;
import openbackup.access.framework.resource.persistence.model.ResourceGroupMemberPo;
import openbackup.access.framework.resource.persistence.model.ResourceGroupPo;
import openbackup.access.framework.resource.service.ResourceGroupRepository;
import openbackup.data.access.framework.core.dao.ProtectedObjectMapper;
import openbackup.data.access.framework.core.entity.ProtectedObjectPo;
import openbackup.data.protection.access.provider.sdk.resourcegroup.dto.ResourceGroupDto;
import openbackup.data.protection.access.provider.sdk.resourcegroup.dto.ResourceGroupMemberDto;
import openbackup.data.protection.access.provider.sdk.resourcegroup.dto.ResourceGroupProtectedObjectDto;
import openbackup.data.protection.access.provider.sdk.resourcegroup.req.ResourceGroupQueryParams;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.query.PageQueryService;
import openbackup.system.base.query.Pagination;
import openbackup.system.base.query.SessionService;
import openbackup.system.base.query.SnakeCasePageQueryFieldNamingStrategy;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.resource.ProtectObjectRestApi;
import openbackup.system.base.sdk.resource.enums.ProtectionStatusEnum;
import openbackup.system.base.sdk.resource.model.ProtectedObjectInfo;
import openbackup.system.base.sdk.resource.model.ProtectionBatchOperationReq;
import openbackup.system.base.sdk.resource.model.ProtectionModifyDto;
import openbackup.system.base.sdk.user.enums.ResourceSetTypeEnum;
import com.huawei.oceanprotect.system.base.user.common.enums.ResourceSetScopeModuleEnum;
import com.huawei.oceanprotect.system.base.user.entity.ResourceSetResourceBo;
import com.huawei.oceanprotect.system.base.user.service.ResourceSetApi;

import com.alibaba.fastjson.JSON;
import com.baomidou.mybatisplus.core.conditions.query.LambdaQueryWrapper;
import com.baomidou.mybatisplus.core.conditions.query.QueryWrapper;
import com.baomidou.mybatisplus.core.conditions.update.LambdaUpdateWrapper;
import com.baomidou.mybatisplus.core.conditions.update.UpdateWrapper;

import feign.FeignException;
import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.BeanUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.sql.Timestamp;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Optional;
import java.util.Set;
import java.util.UUID;
import java.util.stream.Collectors;

/**
 * 资源组Repository实现
 *
 * @author c00631681
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2023-01-20
 */
@Service
@Slf4j
public class ResourceGroupRepositoryImpl implements ResourceGroupRepository {
    private static final String RESOURCE_SET_ID = "resourceSetId";

    private static final String RESOURCE_UUID = "uuid";

    @Autowired
    private ResourceGroupMapper resourceGroupMapper;

    @Autowired
    private ResourceGroupMemberMapper resourceGroupMemberMapper;

    @Autowired
    private ProtectedObjectMapper protectedObjectMapper;

    @Autowired
    private PageQueryService pageQueryService;

    @Autowired
    private SessionService sessionService;

    @Autowired
    private ProtectObjectRestApi protectObjectRestApi;

    @Autowired
    private ResourceSetApi resourceSetApi;

    @Override
    @Transactional(rollbackFor = Exception.class)
    public String save(ResourceGroupDto resourceGroupDto) {
        ResourceGroupPo resourceGroupPo = fromResourceGroupDto(resourceGroupDto);

        TokenBo.UserBo user = sessionService.getCurrentUser();
        resourceGroupPo.setUserId(user.getId());

        resourceGroupPo.setCreatedTime(new Timestamp(System.currentTimeMillis()));

        resourceGroupMapper.insert(resourceGroupPo);
        saveResourceGroupMembers(resourceGroupPo.getUuid(), resourceGroupDto.getResources());
        createResourceSetRelation(resourceGroupPo);
        return resourceGroupPo.getUuid();
    }

    private void createResourceSetRelation(ResourceGroupPo resourceGroupPo) {
        ResourceSetResourceBo resourceSetResourceBo = new ResourceSetResourceBo();
        resourceSetResourceBo.setUserId(resourceGroupPo.getUserId());
        resourceSetResourceBo.setResourceObjectId(resourceGroupPo.getUuid());
        resourceSetResourceBo.setType(ResourceSetTypeEnum.RESOURCE_GROUP);
        resourceSetResourceBo.setScopeModule(ResourceSetScopeModuleEnum.RESOURCE_GROUP.getType());
        resourceSetApi.addResourceSetRelation(resourceSetResourceBo);
    }

    private ResourceGroupPo fromResourceGroupDto(ResourceGroupDto resourceGroupDto) {
        ResourceGroupPo resourceGroupPo = new ResourceGroupPo();
        BeanUtils.copyProperties(resourceGroupDto, resourceGroupPo);
        String resourceGroupId = StringUtils.isBlank(resourceGroupDto.getUuid()) ? UUID.randomUUID().toString()
                : resourceGroupDto.getUuid();
        resourceGroupPo.setUuid(resourceGroupId);
        return resourceGroupPo;
    }

    private void saveResourceGroupMembers(String resourceGroupId,
                                          List<ResourceGroupMemberDto> resourceGroupMemberDtos) {
        for (ResourceGroupMemberDto resourceGroupMemberDto : resourceGroupMemberDtos) {
            ResourceGroupMemberPo resourceGroupMemberPo = new ResourceGroupMemberPo();
            BeanUtils.copyProperties(resourceGroupMemberDto, resourceGroupMemberPo);
            resourceGroupMemberPo.setUuid(UUID.randomUUID().toString());
            resourceGroupMemberPo.setResourceGroupId(resourceGroupId);
            resourceGroupMemberMapper.insert(resourceGroupMemberPo);
        }
    }

    private boolean isWithoutSla(ProtectedObjectInfo groupProtectObject) {
        if (VerifyUtil.isEmpty(groupProtectObject)) {
            return true;
        }
        return VerifyUtil.isEmpty(groupProtectObject.getSlaId());
    }

    @Override
    public BasePage<ResourceGroupDto> queryResourceGroups(ResourceGroupQueryParams resourceGroupQueryParams) {
        JSONObject conditions = JSONObject.fromObject(resourceGroupQueryParams.getConditions());
        fillResourceGroupConditions(conditions);
        if (conditions.containsKey("protectedObject")) {
            conditions.putAll(conditions.getJSONObject("protectedObject"));
            conditions.remove("protectedObject");
            if (conditions.containsKey("sla_compliance")) {
                conditions.put("is_sla_compliance", conditions.get("sla_compliance"));
                conditions.remove("sla_compliance");
            }
        }
        Pagination<JSONObject> resourceGroupPagination = new Pagination<>(resourceGroupQueryParams.getPageNo(),
                resourceGroupQueryParams.getPageSize(), conditions,
                Collections.singletonList(resourceGroupQueryParams.getOrders()),
                SnakeCasePageQueryFieldNamingStrategy.NAME);
        BasePage<ResourceGroupExtendField> resourceGroupPoBasePage =
                pageQueryService.pageQuery(ResourceGroupExtendField.class, resourceGroupMapper::page,
                        resourceGroupPagination, "-created_time", "uuid");
        BasePage<ResourceGroupDto> resourceGroupDtoPagination = new BasePage<>();
        BeanUtils.copyProperties(resourceGroupPoBasePage, resourceGroupDtoPagination);
        resourceGroupDtoPagination.setItems(
                resourceGroupPoBasePage.getItems().stream().map(this::convertPoToDto).collect(Collectors.toList()));
        return resourceGroupDtoPagination;
    }

    private void fillResourceGroupConditions(JSONObject conditions) {
        if (VerifyUtil.isEmpty(conditions) || !conditions.containsKey(RESOURCE_SET_ID)) {
            return;
        }
        String resourceSetId = getValueFromCondition(RESOURCE_SET_ID, conditions);
        if (!VerifyUtil.isEmpty(resourceSetId)) {
            List<String> resourceIdList = resourceSetApi.getAllRelationsByResourceSetId(resourceSetId,
                ResourceSetTypeEnum.RESOURCE_GROUP.getType());
            // 前端同时下发resourceSetId和Uuid,将resourceSetId下的资源id列表进行二次过滤
            if (conditions.containsKey(RESOURCE_UUID)) {
                String resourceUuid = getValueFromCondition(RESOURCE_UUID, conditions);
                List<String> renameResourceId = resourceIdList.stream()
                        .filter(resourceId -> resourceId.contains(resourceUuid)).collect(Collectors.toList());
                if (VerifyUtil.isEmpty(renameResourceId)) {
                    renameResourceId.add(UUID.randomUUID().toString());
                }
                conditions.put(RESOURCE_UUID, renameResourceId);
                conditions.remove(RESOURCE_SET_ID);
                return;
            }
            if (VerifyUtil.isEmpty(resourceIdList)) {
                resourceIdList.add(UUID.randomUUID().toString());
            }
            conditions.put(RESOURCE_UUID, resourceIdList);
            conditions.remove(RESOURCE_SET_ID);
        }
    }

    private String getValueFromCondition(String key, JSONObject conditions) {
        String value;
        Object resourceSetIdObj = conditions.get(key);
        if (resourceSetIdObj instanceof String) {
            value = (String) resourceSetIdObj;
        } else {
            value = StringUtils.EMPTY;
        }
        return value;
    }

    @Override
    public Optional<ResourceGroupDto> selectByScopeResourceIdAndName(
            String scopeResourceId, String resourceGroupName) {
        List<ResourceGroupPo> resourceGroupPos = resourceGroupMapper
                .selectByScopeResourceIdAndName(
                        scopeResourceId, resourceGroupName);
        if (resourceGroupPos.isEmpty()) {
            return Optional.empty();
        }
        return Optional.of(convertPoToDto(resourceGroupPos.get(0)));
    }

    @Override
    public Optional<ResourceGroupDto> selectById(String resourceGroupId) {
        ResourceGroupPo resourceGroupPo = resourceGroupMapper.selectById(resourceGroupId);
        if (resourceGroupPo == null) {
            return Optional.empty();
        }
        return Optional.of(convertPoToDto(resourceGroupPo));
    }

    @Override
    public int getResourceGroupCount(String sourceSubType) {
        LambdaQueryWrapper<ResourceGroupPo> wrapper = new LambdaQueryWrapper<ResourceGroupPo>()
                .eq(ResourceGroupPo::getSourceSubType, sourceSubType);
        return resourceGroupMapper.selectCount(wrapper).intValue();
    }

    @Override
    public Optional<ResourceGroupMemberDto> selectMemberBySourceId(String resourceId) {
        List<ResourceGroupMemberPo> resourceGroupMemberPos = resourceGroupMemberMapper.selectByResourceId(resourceId);
        if (CollectionUtils.isEmpty(resourceGroupMemberPos)) {
            return Optional.empty();
        }
        ResourceGroupMemberDto resourceGroupMemberDto = new ResourceGroupMemberDto();
        BeanUtils.copyProperties(resourceGroupMemberPos.get(0), resourceGroupMemberDto);
        return Optional.of(resourceGroupMemberDto);
    }

    @Override
    public List<ResourceGroupMemberDto> selectMemberBySourceIds(List<String> resourceIds) {
        LambdaQueryWrapper<ResourceGroupMemberPo> wrapper = new LambdaQueryWrapper<ResourceGroupMemberPo>()
                .in(ResourceGroupMemberPo::getSourceId, resourceIds);
        List<ResourceGroupMemberPo> resourceGroupMemberPos = resourceGroupMemberMapper.selectList(wrapper);
        return resourceGroupMemberPos.stream().map(resourceGroupMemberPo -> {
            ResourceGroupMemberDto resourceGroupMemberDto = new ResourceGroupMemberDto();
            BeanUtils.copyProperties(resourceGroupMemberPo, resourceGroupMemberDto);
            return resourceGroupMemberDto;
        }).collect(Collectors.toList());
    }

    @Override
    @Transactional(rollbackFor = Exception.class)
    public String update(ResourceGroupDto resourceGroupDto) {
        // 1. 取出新的newResourceIdSet
        Set<String> newResourceIdset = resourceGroupDto.getResources().stream().map(
                ResourceGroupMemberDto::getSourceId).collect(Collectors.toSet());
        // 2. 找出当前的existResourceIdSet
        LambdaQueryWrapper<ResourceGroupMemberPo> wrapper = new LambdaQueryWrapper<ResourceGroupMemberPo>()
                .eq(ResourceGroupMemberPo::getResourceGroupId, resourceGroupDto.getUuid());
        List<ResourceGroupMemberPo> existResourceGroupMemberPos = resourceGroupMemberMapper.selectList(wrapper);
        Set<String> existResourceIdSet = existResourceGroupMemberPos.stream().map(
                ResourceGroupMemberPo::getSourceId).collect(Collectors.toSet());
        // 3. 计算待删除的resourceIds，并进行删除处理
        Set<String> toDeleteResourceIdSet = new HashSet<>(existResourceIdSet);
        toDeleteResourceIdSet.removeAll(newResourceIdset);
        deleteOldGroupMemberDtos(resourceGroupDto, new ArrayList<>(toDeleteResourceIdSet));
        log.info("Finished delete old members for group {}, {}.", resourceGroupDto.getName(),
                resourceGroupDto.getUuid());
        // 4. 计算待新增的resourceIds，并进行增加处理
        Set<String> toAddResourceIdSet = new HashSet<>(newResourceIdset);
        toAddResourceIdSet.removeAll(existResourceIdSet);
        List<ResourceGroupMemberDto> toAddResourceGroupMemberDtos = resourceGroupDto.getResources().stream().filter(
                        resourceGroupMemberDto -> toAddResourceIdSet.contains(resourceGroupMemberDto.getSourceId()))
                .collect(Collectors.toList());
        saveResourceGroupMembers(resourceGroupDto.getUuid(), toAddResourceGroupMemberDtos);
        createProtectIfGroupProtected(resourceGroupDto.getUuid(), toAddResourceGroupMemberDtos);
        log.info("Finished add new members for group {}, {}.", resourceGroupDto.getName(), resourceGroupDto.getUuid());
        // 保存组信息
        return updateResourceGroup(resourceGroupDto);
    }

    private void createProtectIfGroupProtected(String groupId,
        List<ResourceGroupMemberDto> toAddResourceGroupMemberDtos) {
        // 当前组已受到保护，添加组成员时，要同时添加组成员的保护
        ProtectedObjectInfo groupProtectObject = protectObjectRestApi.getProtectObject(groupId);
        if (isWithoutSla(groupProtectObject)) {
            return;
        }
        boolean isProtected = ProtectionStatusEnum.PROTECTED.getType().equals(groupProtectObject.getStatus());
        toAddResourceGroupMemberDtos.forEach(memberDto -> {
            ProtectionModifyDto protectionModifyDto = new ProtectionModifyDto();
            protectionModifyDto.setSlaId(groupProtectObject.getSlaId());
            protectionModifyDto.setResourceId(memberDto.getSourceId());
            protectionModifyDto.setIsResourceGroup(false);
            protectionModifyDto.setIsGroupSubResource(true);
            protectionModifyDto.setResourceGroupId(groupId);
            protectionModifyDto.setExtParameters(groupProtectObject.getExtParameters());
            protectObjectRestApi.createProtectedObjectInternal(protectionModifyDto);
            if (!isProtected) {
                ProtectionBatchOperationReq operationReq = new ProtectionBatchOperationReq();
                operationReq.setResourceIds(Collections.singletonList(memberDto.getSourceId()));
                operationReq.setIsResourceGroup(false);
                try {
                    protectObjectRestApi.deactivate(operationReq);
                } catch (LegoUncheckedException | FeignException ex) {
                    log.error("Call deactivate api failed, resource id: {}", memberDto.getSourceId(), ex);
                    throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "call deactivate error.");
                }
            }
            log.info("Create group sub resource protect object success. Resource id :{}", memberDto.getSourceId());
        });
    }

    @Override
    public String updateResourceGroup(ResourceGroupDto resourceGroupDto) {
        ResourceGroupPo resourceGroupPo = fromResourceGroupDto(resourceGroupDto);
        LambdaUpdateWrapper<ResourceGroupPo> resourceGroupPoLambdaUpdateWrapper =
                new UpdateWrapper<ResourceGroupPo>().lambda();
        resourceGroupPoLambdaUpdateWrapper.eq(ResourceGroupPo::getUuid, resourceGroupPo.getUuid());
        cleanUnmodifiableFields(resourceGroupPo);
        resourceGroupMapper.update(resourceGroupPo, resourceGroupPoLambdaUpdateWrapper);
        log.info("Finished update for group {}, {}.", resourceGroupDto.getName(), resourceGroupDto.getUuid());
        return resourceGroupDto.getUuid();
    }

    @Override
    public List<ResourceGroupDto> getAllResourceGroupList(List<String> subTypeList) {
        QueryWrapper<ResourceGroupPo> queryWrapper = new QueryWrapper<>();
        queryWrapper.in("source_sub_type", subTypeList);
        return resourceGroupMapper.selectList(queryWrapper).stream().map(resourceGroupPo -> {
            ResourceGroupDto resourceGroupDto = new ResourceGroupDto();
            BeanUtils.copyProperties(resourceGroupPo, resourceGroupDto);
            return resourceGroupDto;
        }).collect(Collectors.toList());
    }

    private void cleanUnmodifiableFields(ResourceGroupPo resourceGroupPo) {
        resourceGroupPo.setSourceType(null);
        resourceGroupPo.setSourceSubType(null);
        resourceGroupPo.setPath(null);
        resourceGroupPo.setCreatedTime(null);
        resourceGroupPo.setUserId(null);
        resourceGroupPo.setProtectionStatus(null);
    }

    private void deleteOldGroupMemberDtos(ResourceGroupDto newResourceGroupDto, List<String> toDeleteResourceIds) {
        // 1. 待删除为空则不删除
        if (VerifyUtil.isEmpty(toDeleteResourceIds)) {
            return;
        }
        // 2. 组成员列表不为空，则进行删除处理
        LambdaQueryWrapper<ResourceGroupMemberPo> wrapper = new LambdaQueryWrapper<ResourceGroupMemberPo>()
                .in(ResourceGroupMemberPo::getSourceId, toDeleteResourceIds)
                .eq(ResourceGroupMemberPo::getResourceGroupId, newResourceGroupDto.getUuid());
        int count = resourceGroupMemberMapper.delete(wrapper);
        log.info("Deleted {} resource group members for group of {}.", count,
                newResourceGroupDto.getUuid());
        // 3. 当前组已受到保护，移除组成员时，要同时移除组成员的保护
        ProtectedObjectInfo groupProtectObject = protectObjectRestApi.getProtectObject(newResourceGroupDto.getUuid());
        if (isWithoutSla(groupProtectObject)) {
            return;
        }
        ProtectionBatchOperationReq batchOperationReq = new ProtectionBatchOperationReq();
        batchOperationReq.setResourceIds(toDeleteResourceIds);
        batchOperationReq.setIsResourceGroup(false);
        protectObjectRestApi.deleteProtectedObjects(batchOperationReq);
        log.info("Remove protect for group sub resources success. Deleted resources ids:{}", toDeleteResourceIds);
    }

    /**
     * convertPoToDto
     *
     * @param resourceGroupPo resourceGroupPo
     * @return ResourceGroupDto
     */
    public ResourceGroupDto convertPoToDto(ResourceGroupPo resourceGroupPo) {
        ResourceGroupDto resourceGroupDto = new ResourceGroupDto();
        BeanUtils.copyProperties(resourceGroupPo, resourceGroupDto);
        // 添加组成员信息
        List<ResourceGroupMemberPo> resourceGroupMemberPos =
                resourceGroupMemberMapper.selectByResourceGroupId(resourceGroupPo.getUuid());
        resourceGroupDto.setResources(
                resourceGroupMemberPos.stream().map(this::convertPoToDto).collect(Collectors.toList()));
        // 添加保护对象信息
        ProtectedObjectPo protectedObjectPo = protectedObjectMapper.selectById(resourceGroupPo.getUuid());
        if (protectedObjectPo != null) {
            resourceGroupDto.setProtectedObjectDto(convertPoToDto(protectedObjectPo));
        }
        return resourceGroupDto;
    }

    private ResourceGroupMemberDto convertPoToDto(ResourceGroupMemberPo resourceGroupMemberPo) {
        ResourceGroupMemberDto resourceGroupMemberDto = new ResourceGroupMemberDto();
        BeanUtils.copyProperties(resourceGroupMemberPo, resourceGroupMemberDto);
        return resourceGroupMemberDto;
    }

    private ResourceGroupProtectedObjectDto convertPoToDto(ProtectedObjectPo protectedObjectPo) {
        ResourceGroupProtectedObjectDto protectedObjectDto = new ResourceGroupProtectedObjectDto();
        BeanUtils.copyProperties(protectedObjectPo, protectedObjectDto);
        protectedObjectDto
            .setExtParameters(JSONObject.fromObject(JSON.parse(protectedObjectPo.getExtParameters()).toString()));
        return protectedObjectDto;
    }

    @Override
    @Transactional(rollbackFor = Exception.class)
    public void delete(String resourceGroupId) {
        resourceGroupMapper.deleteById(resourceGroupId);
        resourceGroupMemberMapper.delete(
                new LambdaUpdateWrapper<ResourceGroupMemberPo>().eq(
                        ResourceGroupMemberPo::getResourceGroupId, resourceGroupId));
        resourceSetApi.deleteResourceSetRelation(resourceGroupId, ResourceSetTypeEnum.RESOURCE_GROUP);
        log.info("Finish delete resource group with id {} and it's members.", resourceGroupId);
    }


    @Override
    public Optional<ProtectedObjectPo> selectProtectedObjectById(String resourceGroupId) {
        LambdaQueryWrapper<ProtectedObjectPo> wrapper =
                new LambdaQueryWrapper<ProtectedObjectPo>().eq(ProtectedObjectPo::getResourceGroupId, resourceGroupId);
        List<ProtectedObjectPo> protectedObjectPos = protectedObjectMapper.selectList(wrapper);
        if (VerifyUtil.isEmpty(protectedObjectPos)) {
            return Optional.empty();
        }
        return Optional.of(protectedObjectPos.get(0));
    }
}
