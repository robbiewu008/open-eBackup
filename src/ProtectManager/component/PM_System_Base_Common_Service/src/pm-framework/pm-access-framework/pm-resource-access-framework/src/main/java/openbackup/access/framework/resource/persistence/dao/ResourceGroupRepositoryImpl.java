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

import com.huawei.oceanprotect.report.enums.ProtectStatusEnum;
import com.huawei.oceanprotect.system.base.user.common.enums.ResourceSetScopeModuleEnum;
import com.huawei.oceanprotect.system.base.user.entity.ResourceSetResourceBo;
import com.huawei.oceanprotect.system.base.user.service.ResourceSetApi;

import com.alibaba.fastjson.JSON;
import com.baomidou.mybatisplus.core.conditions.query.LambdaQueryWrapper;
import com.baomidou.mybatisplus.core.conditions.query.QueryWrapper;
import com.baomidou.mybatisplus.core.conditions.update.LambdaUpdateWrapper;
import com.baomidou.mybatisplus.core.conditions.update.UpdateWrapper;

import lombok.extern.slf4j.Slf4j;
import openbackup.access.framework.resource.constant.SqlBuilderConstant;
import openbackup.access.framework.resource.persistence.model.ResourceGroupExtendField;
import openbackup.access.framework.resource.persistence.model.ResourceGroupMemberPo;
import openbackup.access.framework.resource.persistence.model.ResourceGroupPo;
import openbackup.access.framework.resource.service.ResourceGroupMemberRepository;
import openbackup.access.framework.resource.service.ResourceGroupRepository;
import openbackup.data.access.framework.core.dao.ProtectedObjectMapper;
import openbackup.data.access.framework.core.entity.ProtectedObjectPo;
import openbackup.data.protection.access.provider.sdk.resourcegroup.dto.ResourceGroupDto;
import openbackup.data.protection.access.provider.sdk.resourcegroup.dto.ResourceGroupMemberDto;
import openbackup.data.protection.access.provider.sdk.resourcegroup.dto.ResourceGroupProtectedObjectDto;
import openbackup.data.protection.access.provider.sdk.resourcegroup.enums.GroupTypeEnum;
import openbackup.data.protection.access.provider.sdk.resourcegroup.req.ResourceGroupQueryParams;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.ValidateUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.query.PageQueryService;
import openbackup.system.base.query.Pagination;
import openbackup.system.base.query.SessionService;
import openbackup.system.base.query.SnakeCasePageQueryFieldNamingStrategy;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.user.enums.ResourceSetTypeEnum;
import openbackup.system.base.service.ResourceGroupService;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.compress.utils.Lists;
import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.BeanUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.sql.Timestamp;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.Set;
import java.util.UUID;
import java.util.stream.Collectors;

/**
 * 资源组Repository实现
 *
 */
@Service
@Slf4j
public class ResourceGroupRepositoryImpl implements ResourceGroupRepository, ResourceGroupService {
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
    private ResourceGroupMemberRepository resourceGroupMemberRepository;

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
        if (VerifyUtil.isEmpty(resourceGroupMemberDtos)) {
            return;
        }
        List<ResourceGroupMemberPo> members = new ArrayList<>();
        for (ResourceGroupMemberDto resourceGroupMemberDto : resourceGroupMemberDtos) {
            ResourceGroupMemberPo resourceGroupMemberPo = new ResourceGroupMemberPo();
            BeanUtils.copyProperties(resourceGroupMemberDto, resourceGroupMemberPo);
            resourceGroupMemberPo.setUuid(UUID.randomUUID().toString());
            resourceGroupMemberPo.setResourceGroupId(resourceGroupId);
            members.add(resourceGroupMemberPo);
        }
        resourceGroupMemberRepository.saveBatch(members, 1000);
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
        final Map<String, Object> labelCondition = handleLabelCondition(conditions);
        Pagination<JSONObject> resourceGroupPagination = new Pagination<>(resourceGroupQueryParams.getPageNo(),
                resourceGroupQueryParams.getPageSize(), conditions,
                Collections.singletonList(resourceGroupQueryParams.getOrders()),
                SnakeCasePageQueryFieldNamingStrategy.NAME);
        BasePage<ResourceGroupExtendField> resourceGroupPoBasePage =
                pageQueryService.pageQuery(ResourceGroupExtendField.class,
                    (page, wrapper) -> {
                    addResourceLabelListCondition(wrapper, labelCondition);
                    return resourceGroupMapper.page(page, wrapper);
                    },
                        resourceGroupPagination, "-created_time", "uuid");
        BasePage<ResourceGroupDto> resourceGroupDtoPagination = new BasePage<>();
        BeanUtils.copyProperties(resourceGroupPoBasePage, resourceGroupDtoPagination);
        resourceGroupDtoPagination.setItems(
                resourceGroupPoBasePage.getItems().stream().map(this::convertPoToDto).collect(Collectors.toList()));
        return resourceGroupDtoPagination;
    }

    private Map<String, Object> handleLabelCondition(JSONObject conditions) {
        Map<String, Object> labelCondition = new HashMap<>();
        Object labelConditionMap = conditions.get(SqlBuilderConstant.KEY_LABEL_CONDITION);
        if (VerifyUtil.isEmpty(labelConditionMap) || !(labelConditionMap instanceof Map)) {
            return labelCondition;
        }
        Object labelList = ((Map<?, ?>) labelConditionMap).remove(SqlBuilderConstant.KEY_LABEL_LIST);
        if (VerifyUtil.isEmpty(labelList) || !(labelList instanceof List)) {
            return labelCondition;
        }
        labelCondition.put(SqlBuilderConstant.KEY_LABEL_LIST, labelList);
        return labelCondition;
    }

    private void addResourceLabelListCondition(QueryWrapper<ResourceGroupExtendField> wrapper,
        Map<String, Object> labelConditions) {
        if (VerifyUtil.isEmpty(labelConditions)) {
            return;
        }
        Object labelListObj = labelConditions.get(SqlBuilderConstant.KEY_LABEL_LIST);
        if (VerifyUtil.isEmpty(labelListObj) || !(labelListObj instanceof List)
            || VerifyUtil.isEmpty((List<?>) labelListObj)) {
            return;
        }
        List<String> labelList = (List<String>) labelListObj;
        StringBuilder labelListSql = new StringBuilder(SqlBuilderConstant.LABEL_LIST_CONDITION_SQL_PREFIX);
        for (int i = 0; i < labelList.size(); i++) {
            // uuid防注入
            if (!ValidateUtil.match(UUID_N0_SEPARATOR, labelList.get(i))) {
                continue;
            }
            labelListSql.append(SqlBuilderConstant.SINGLE_QUOTES).append(labelList.get(i))
                .append(SqlBuilderConstant.SINGLE_QUOTES);
            if (i < (labelList.size() - 1)) {
                labelListSql.append(SqlBuilderConstant.COMMA);
            }
        }
        labelListSql.append(SqlBuilderConstant.LABEL_LIST_CONDITION_SQL_SUFFIX)
            .append(labelList.size());
        wrapper.inSql(SqlBuilderConstant.COLUMN_Q_UUID, labelListSql.toString());
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
    public String update(ResourceGroupDto resourceGroupDto, Set<String> toDeleteResourceIdSet,
        Set<String> toAddResourceIdSet) {
        // 1. 并进行删除处理
        deleteOldGroupMemberDtos(resourceGroupDto, toDeleteResourceIdSet);

        // 2. 子资源并进行增加处理
        List<ResourceGroupMemberDto> toAddResourceGroupMemberDtos = Collections.emptyList();
        if (!GroupTypeEnum.RULE.getValue().equals(resourceGroupDto.getGroupType())) {
            toAddResourceGroupMemberDtos = resourceGroupDto.getResources().stream()
                .filter(
                    resourceGroupMemberDto -> toAddResourceIdSet.contains(resourceGroupMemberDto.getSourceId()))
                .collect(Collectors.toList());
        }
        saveResourceGroupMembers(resourceGroupDto.getUuid(), toAddResourceGroupMemberDtos);
        log.info("Finished add new members for group {}, {}.", resourceGroupDto.getName(), resourceGroupDto.getUuid());
        // 保存组信息
        return updateResourceGroup(resourceGroupDto);
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

    private void deleteOldGroupMemberDtos(ResourceGroupDto newResourceGroupDto, Set<String> toDeleteResourceIdSet) {
        // 1. 按规则过滤组以及待删除为空则不删除
        if (GroupTypeEnum.RULE.getValue().equals(newResourceGroupDto.getGroupType())
            || VerifyUtil.isEmpty(toDeleteResourceIdSet)) {
            return;
        }
        List<String> toDeleteResourceIds = new ArrayList<>(toDeleteResourceIdSet);
        // 2. 组成员列表不为空，则进行删除处理
        LambdaQueryWrapper<ResourceGroupMemberPo> wrapper = new LambdaQueryWrapper<ResourceGroupMemberPo>()
                .in(ResourceGroupMemberPo::getSourceId, toDeleteResourceIds)
                .eq(ResourceGroupMemberPo::getResourceGroupId, newResourceGroupDto.getUuid());
        int count = resourceGroupMemberMapper.delete(wrapper);
        log.info("Deleted {} resource group members for group of {}.", count,
                newResourceGroupDto.getUuid());
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
        if (GroupTypeEnum.RULE.getValue().equals(resourceGroupPo.getGroupType())) {
            resourceGroupDto.setResources(listByGroupId(resourceGroupPo.getUuid()));
        }
        return resourceGroupDto;
    }

    private List<ResourceGroupMemberDto> listByGroupId(String groupId) {
        // 过滤掉资源组本身
        return listProtectedObjectPoByGroupId(groupId).stream()
            .filter(v -> !groupId.equals(v.getResourceId()))
            .map(this::convertProtectedObjectPoToDto).collect(Collectors.toList());
    }

    private List<ProtectedObjectPo> listProtectedObjectPoByGroupId(String groupId) {
        LambdaQueryWrapper<ProtectedObjectPo> wrapper = new LambdaQueryWrapper<ProtectedObjectPo>()
            .eq(ProtectedObjectPo::getResourceGroupId, groupId);
        return protectedObjectMapper.selectList(wrapper);
    }

    private ResourceGroupMemberDto convertProtectedObjectPoToDto(ProtectedObjectPo po) {
        ResourceGroupMemberDto dto = new ResourceGroupMemberDto();
        dto.setResourceGroupId(po.getResourceGroupId());
        dto.setSourceId(po.getResourceId());
        dto.setSourceSubType(po.getSubType());
        return dto;
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
    public void deleteByScopeResourceId(String scopeResourceId) {
        List<ResourceGroupPo> resourceGroupInScope =
                Optional.ofNullable(resourceGroupMapper.selectList(new LambdaQueryWrapper<ResourceGroupPo>()
                        .eq(ResourceGroupPo::getScopeResourceId, scopeResourceId))).orElse(Lists.newArrayList());
        for (ResourceGroupPo resourceGroupPo : resourceGroupInScope) {
            delete(resourceGroupPo.getUuid());
        }
        log.info("Finished deleting all resource groups of environment {} and their members.", scopeResourceId);
    }

    @Override
    @Transactional(rollbackFor = Exception.class)
    public void delete(String resourceGroupId) {
        resourceGroupMapper.deleteById(resourceGroupId);
        resourceGroupMemberMapper.delete(
                new LambdaUpdateWrapper<ResourceGroupMemberPo>().eq(
                        ResourceGroupMemberPo::getResourceGroupId, resourceGroupId));
        resourceSetApi.deleteResourceSetRelation(resourceGroupId, ResourceSetTypeEnum.RESOURCE_GROUP);
        log.info("Finished deleting resource group with id {} and its members.", resourceGroupId);
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

    @Override
    public boolean isResourceGroupExit(String id) {
        return !VerifyUtil.isEmpty(resourceGroupMapper.selectById(id));
    }

    @Override
    public int countByGroupType(String groupType) {
        LambdaQueryWrapper<ResourceGroupPo> wrapper = new LambdaQueryWrapper<ResourceGroupPo>()
            .eq(ResourceGroupPo::getGroupType, groupType);
        return resourceGroupMapper.selectCount(wrapper).intValue();
    }

    @Override
    public int updateStatusById(String uuid, ProtectStatusEnum protectStatus) {
        ResourceGroupPo po = new ResourceGroupPo();
        po.setUuid(uuid);
        po.setProtectionStatus(protectStatus.getProtectStatusNum());
        return resourceGroupMapper.updateById(po);
    }
}
