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
package openbackup.access.framework.resource.service;

import openbackup.access.framework.resource.persistence.dao.ProtectedResourceRepositoryImpl;
import openbackup.access.framework.resource.persistence.dao.ResourceGroupMemberMapper;
import openbackup.access.framework.resource.persistence.dao.VirtualResourceMapper;
import openbackup.access.framework.resource.persistence.model.VirtualResourceExtendPo;
import openbackup.access.framework.resource.util.ResourceFilterUtil;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.constants.ProtectObjectErrorCode;
import openbackup.data.protection.access.provider.sdk.protection.ProtectionInterceptorProvider;
import openbackup.data.protection.access.provider.sdk.protection.model.ProtectionExecuteDto;
import openbackup.data.protection.access.provider.sdk.protection.model.ProtectionResourceDto;
import openbackup.data.protection.access.provider.sdk.protection.service.ProjectedObjectService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceQueryParams;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.resourcegroup.enums.GroupTypeEnum;
import openbackup.data.protection.access.provider.sdk.resourcegroup.constant.ResourceGroupConstant;
import openbackup.data.protection.access.provider.sdk.resourcegroup.constant.ResourceGroupErrorCode;
import openbackup.data.protection.access.provider.sdk.resourcegroup.constant.ResourceGroupLabelConstant;
import openbackup.data.protection.access.provider.sdk.resourcegroup.dto.ResourceGroupDto;
import openbackup.data.protection.access.provider.sdk.resourcegroup.dto.ResourceFilterConditionParam;
import openbackup.data.protection.access.provider.sdk.resourcegroup.dto.ResourceGroupMemberDto;
import openbackup.data.protection.access.provider.sdk.resourcegroup.dto.ResourceGroupProtectedObjectDto;
import openbackup.data.protection.access.provider.sdk.resourcegroup.dto.ResourceGroupProtectedObjectLabelDto;
import openbackup.data.protection.access.provider.sdk.resourcegroup.dto.ResourceGroupResultDto;
import openbackup.data.protection.access.provider.sdk.resourcegroup.req.CreateResourceGroupProtectedObjectRequest;
import openbackup.data.protection.access.provider.sdk.resourcegroup.req.ProtectedObjectRequest;
import openbackup.data.protection.access.provider.sdk.resourcegroup.req.ResourceGroupQueryParams;
import openbackup.data.protection.access.provider.sdk.resourcegroup.req.UpdateResourceGroupProtectedObjectRequest;
import openbackup.data.protection.access.provider.sdk.resourcegroup.resp.ResourceGroupDetailVo;
import openbackup.data.protection.access.provider.sdk.resourcegroup.resp.ResourceGroupMemberVo;
import openbackup.data.protection.access.provider.sdk.resourcegroup.resp.ResourceGroupProtectedObjectVo;
import openbackup.data.protection.access.provider.sdk.resourcegroup.resp.ResourceGroupVo;
import openbackup.data.protection.access.provider.sdk.resourcegroup.service.ResourceGroupService;
import com.huawei.oceanprotect.job.sdk.JobService;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyAction;
import com.huawei.oceanprotect.system.base.label.service.LabelService;

import com.alibaba.fastjson.JSON;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.PageListResponse;
import openbackup.system.base.common.model.job.request.QueryJobRequest;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.asserts.PowerAssert;
import openbackup.system.base.query.SessionService;
import openbackup.system.base.sdk.common.model.UuidObject;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.job.model.JobStatusEnum;
import openbackup.system.base.sdk.resource.ProtectObjectRestApi;
import openbackup.system.base.sdk.resource.ResourceGroupProtectedObjectRestApi;
import openbackup.system.base.sdk.resource.enums.ProtectionStatusEnum;
import openbackup.system.base.sdk.resource.model.ManualBackupReq;
import openbackup.system.base.sdk.resource.model.ProtectedObjectInfo;
import openbackup.system.base.sdk.resource.model.ProtectionBatchOperationReq;
import openbackup.system.base.sdk.resource.model.ProtectionModifyDto;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.security.callee.CalleeMethod;
import openbackup.system.base.security.callee.CalleeMethods;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.BeanUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.Set;
import java.util.function.Function;
import java.util.stream.Collectors;

/**
 * 资源组服务
 *
 */
@Service
@Slf4j
@CalleeMethods(
    name = "resourceGroup",
    value = {@CalleeMethod(name = "getResourceGroupLabel"), @CalleeMethod(name = "getResourceGroupNameById")})
public class ResourceGroupServiceImpl implements ResourceGroupService {
    @Autowired
    private ResourceGroupRepository resourceGroupRepository;

    @Autowired
    private ResourceService resourceService;

    @Autowired
    private ResourceGroupMemberMapper resourceGroupMemberMapper;

    @Autowired
    private ProtectObjectRestApi protectObjectRestApi;

    @Autowired
    private ResourceGroupProtectedObjectRestApi resourceGroupProtectedObjectRestApi;

    @Autowired
    private JobService jobService;

    @Autowired
    private ProviderManager providerManager;

    @Autowired
    private LabelService labelService;

    @Autowired
    private SessionService sessionService;

    @Autowired
    private VirtualResourceMapper virtualResourceMapper;

    @Autowired
    private ProjectedObjectService projectedObjectService;

    /**
     * 获取资源组后置操作label
     *
     * @param groupReq 资源组请求体
     * @return 资源组后置操作label
     */
    public ResourceGroupProtectedObjectLabelDto getResourceGroupLabel(
            CreateResourceGroupProtectedObjectRequest groupReq) {
        ResourceGroupProtectedObjectLabelDto resourceGroupLabelDto = new ResourceGroupProtectedObjectLabelDto();
        // 获取资源组主体信息
        resourceGroupRepository.selectById(groupReq.getResourceGroupId())
                .ifPresent(resourceGroupDto -> resourceGroupLabelDto.setName(
                        resourceGroupDto.getName() + ":" + resourceGroupDto.getUuid()));
        if (StringUtils.isEmpty(groupReq.getPostAction())) {
            resourceGroupLabelDto.setLabel(ResourceGroupLabelConstant.GROUP_PROTECTION_WITHOUT_BACKUP_LABEL);
        } else {
            resourceGroupLabelDto.setLabel(ResourceGroupLabelConstant.GROUP_PROTECTION_AND_MANUAL_BACKUP_LABEL);
        }
        return resourceGroupLabelDto;
    }

    /**
     * 获取资源组名称
     *
     * @param batchOperationReq 保护批处理操作请求体
     * @return 资源组名称字符串
     */
    public String getResourceGroupNameById(ProtectionBatchOperationReq batchOperationReq) {
        // 获取资源组主体信息
        List<String> resourceIds = batchOperationReq.getResourceIds();
        StringBuilder resourceGroupName = new StringBuilder();
        for (String resourceGroupId : resourceIds) {
            resourceGroupRepository.selectById(resourceGroupId).ifPresent(resourceGroupDto -> {
                resourceGroupName.append(resourceGroupDto.getName());
                if (resourceIds.indexOf(resourceGroupId) < resourceIds.size() - 1) {
                    resourceGroupName.append(",");
                }
            });
        }
        return resourceGroupName.toString();
    }

    /**
     * 创建资源组
     *
     * @param resourceGroupDto 资源组信息
     * @return resourceGroupId
     */
    @Override
    public String createResourceGroup(ResourceGroupDto resourceGroupDto) {
        // 校验名称
        checkNameForCreate(resourceGroupDto);
        // 检查某个资源类型的资源组数量不超过限制
        checkResourceGroupCount(resourceGroupDto);
        // 检查资源组成员
        checkAndUpdateGroupMembers(resourceGroupDto);
        // 检查资源组子类型
        checkSourceSubType(resourceGroupDto);
        // 设置初始保护状态
        resourceGroupDto.setProtectionStatus(ProtectionStatusEnum.UNPROTECTED.getType());
        // 资源组入库
        return resourceGroupRepository.save(resourceGroupDto);
    }

    private void checkNameForCreate(ResourceGroupDto resourceGroupDto) {
        Optional<ResourceGroupDto> optionalResourceGroupDto =
                resourceGroupRepository.selectByScopeResourceIdAndName(
                        resourceGroupDto.getScopeResourceId(),
                        resourceGroupDto.getName());
        PowerAssert.state(!optionalResourceGroupDto.isPresent(),
                () -> new LegoCheckedException(CommonErrorCode.DUPLICATE_NAME, "The name is duplicated."));
    }

    private void checkResourceGroupCount(ResourceGroupDto resourceGroupDto) {
        int resourceGroupCount = resourceGroupRepository.getResourceGroupCount(resourceGroupDto.getSourceSubType());
        PowerAssert.state(
                resourceGroupCount < ResourceGroupConstant.EACH_SUBTYPE_RESOURCE_GROUP_COUNT_MAX_LIMIT,
                () -> new LegoCheckedException(ResourceGroupErrorCode.EXCEED_RESOURCE_GROUP_MAX_NUM_LIMIT,
                        "ResourceGroup quantity exceed the limit."));

        // 按规则过滤资源组需要限制规格
        if (!GroupTypeEnum.RULE.getValue().equals(resourceGroupDto.getGroupType())) {
            return;
        }
        int ruleGroupCount = resourceGroupRepository.countByGroupType(resourceGroupDto.getGroupType());
        PowerAssert.state(
            ruleGroupCount < ResourceGroupConstant.RULE_RESOURCE_GROUP_COUNT_MAX_LIMIT,
            () -> new LegoCheckedException(ResourceGroupErrorCode.EXCEED_RESOURCE_GROUP_MAX_NUM_LIMIT,
                "ResourceGroup quantity exceed the limit."));
    }

    private void checkSourceSubType(ResourceGroupDto dto) {
        if (!GroupTypeEnum.RULE.getValue().equals(dto.getGroupType())) {
            return;
        }
        String sourceSubType = dto.getSourceSubType();
        PowerAssert.state(ResourceSubTypeEnum.VMWARE.equalsSubType(sourceSubType),
            () -> new LegoCheckedException(ResourceGroupErrorCode.SOURCE_SUB_TYPE_NOT_MATCH,
                "The subtype of resource must be vmware."));
    }

    private void checkAndUpdateGroupMembers(ResourceGroupDto resourceGroupDto) {
        List<ResourceGroupMemberDto> groupMemberDtos = resourceGroupDto.getResources();
        if (GroupTypeEnum.RULE.getValue().equals(resourceGroupDto.getGroupType())
            || VerifyUtil.isEmpty(groupMemberDtos)) {
            log.info("No resources in the resource group.");
            return;
        }

        // 资源组成员数量不超过上限
        PowerAssert.state(
                groupMemberDtos.size() <= ResourceGroupConstant.RESOURCE_GROUP_MEMBER_COUNT_MAX_LIMIT,
                () -> new LegoCheckedException(ResourceGroupErrorCode.EXCEED_RESOURCE_GROUP_MEMBER_MAX_NUM_LIMIT,
                    new String[] {String.valueOf(ResourceGroupConstant.RESOURCE_GROUP_MEMBER_COUNT_MAX_LIMIT)},
                        "ResourceGroup members' quantity exceed the limit."));

        Set<String> resourceGroupMemberUuids =
                groupMemberDtos.stream().map(ResourceGroupMemberDto::getSourceId).collect(Collectors.toSet());

        // 检查重复id
        PowerAssert.state(resourceGroupMemberUuids.size() == groupMemberDtos.size(),
                () -> new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM,
                        "Ids of resource group members has duplicate values."));

        // 获取资源组成员对应的资源信息
        Map<String, ProtectedResource> existsResourceMap = getStringProtectedResourceMap(resourceGroupMemberUuids);

        // 检查存在性
        PowerAssert.state(existsResourceMap.size() == resourceGroupMemberUuids.size(),
                () -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST,
                        "ResourceGroup members' resource_id is not valid."));

        // 获取已经存在的resourceGroupMembers
        List<ResourceGroupMemberDto> existResourceGroupMemberDtos =
                resourceGroupRepository.selectMemberBySourceIds(new ArrayList<>(resourceGroupMemberUuids));
        Map<String, ResourceGroupMemberDto> existResourceGroupMemberDtoMap =
                existResourceGroupMemberDtos.stream().collect(
                        Collectors.toMap(ResourceGroupMemberDto::getSourceId, Function.identity()));

        for (ResourceGroupMemberDto groupMemberDto : groupMemberDtos) {
            ProtectedResource existResource = existsResourceMap.get(groupMemberDto.getSourceId());
            // 检查类型和保护状态
            ResourceGroupMemberDto existResourceGroupMemberDto =
                    existResourceGroupMemberDtoMap.get(existResource.getUuid());
            checkGroupMember(existResource, existResourceGroupMemberDto, resourceGroupDto);
            // 将查到的资源类型赋值给 member
            groupMemberDto.setSourceSubType(existResource.getSubType());
        }
    }

    private Map<String, ProtectedResource> getStringProtectedResourceMap(Set<String> resourceGroupMemberUuids) {
        ResourceQueryParams params = new ResourceQueryParams();
        params.setShouldDecrypt(false);
        params.setPage(0);
        params.setSize(500);
        params.setConditions(Collections.singletonMap("uuid", resourceGroupMemberUuids));
        params.setShouldIgnoreOwner(true);
        openbackup.data.protection.access.provider.sdk.base.PageListResponse<ProtectedResource> response =
                resourceService.query(params);
        List<ProtectedResource> existResources = response.getRecords();
        return existResources.stream().collect(Collectors.toMap(ProtectedResource::getUuid, Function.identity()));
    }

    private void checkGroupMember(ProtectedResource existResource, ResourceGroupMemberDto existResourceGroupMemberDto,
                                  ResourceGroupDto resourceGroupDto) {
        // 检查类型
        PowerAssert.state(Objects.equals(existResource.getSubType(), resourceGroupDto.getSourceSubType()),
                () -> new LegoCheckedException(ResourceGroupErrorCode.SOURCE_SUB_TYPE_NOT_MATCH,
                        "The subtype of resource group member does not match the group's."));

        // 检查受保护状态
        if (VerifyUtil.isEmpty(resourceGroupDto.getUuid())) {
            // 如果资源组uuid 为空，创建流程，那么资源不应该有保护对象
            PowerAssert.state(VerifyUtil.isEmpty(
                    existResource.getProtectedObject()),
                    () -> new LegoCheckedException(
                            ResourceGroupErrorCode.RESOURCE_GROUP_MEMBER_PROTECT_STATUS_NOT_VALID,
                            "The resource with protected object can not be added into the resource group."));
        } else {
            // 如果资源组uuid 不为空，为修改流程，如果有保护对象，则其组id 与资源组uuid应该相同
            if (!VerifyUtil.isEmpty(existResource.getProtectedObject())) {
                PowerAssert.state(
                        Objects.equals(existResource.getProtectedObject().getResourceGroupId(),
                                resourceGroupDto.getUuid()),
                        () -> new LegoCheckedException(
                                ResourceGroupErrorCode.RESOURCE_GROUP_MEMBER_PROTECT_STATUS_NOT_VALID,
                                "The resource with protected object can not be added into the resource group."));
            }
        }

        // 不允许一个资源加入到多个组
        if (resourceGroupDto.getUuid() != null && existResourceGroupMemberDto != null) {
            PowerAssert.state(
                    Objects.equals(existResourceGroupMemberDto.getResourceGroupId(), resourceGroupDto.getUuid()),
                    () -> new LegoCheckedException(ResourceGroupErrorCode.RESOURCE_ID_EXIST_IN_OTHER_GROUP,
                            "Resource group members cannot be added to more than one group."));
        }
        if (resourceGroupDto.getUuid() == null) {
            PowerAssert.state(existResourceGroupMemberDto == null,
                    () -> new LegoCheckedException(ResourceGroupErrorCode.RESOURCE_ID_EXIST_IN_OTHER_GROUP,
                            "Resource group members cannot be added to more than one group."));
        }
    }

    /**
     * 修改资源组
     *
     * @param newResourceGroupDto 用于修改用的资源组信息
     * @return resourceGroupId
     */
    @Override
    public String updateResourceGroup(ResourceGroupDto newResourceGroupDto) {
        // 检查存在性
        ResourceGroupDto existGroupDto = checkIfExist(newResourceGroupDto);
        // 检查名称
        checkNameForUpdate(newResourceGroupDto);
        // 存在运行中的任务不允许修改
        if (checkHasUnfinishedJob(existGroupDto.getUuid())) {
            throw new LegoCheckedException(CommonErrorCode.HAVE_RUNNING_JOB, "There are some unfinished jobs with "
                    + "resource_group_id " + existGroupDto.getUuid() + ", so the resource group can not be "
                    + "modified.");
        }
        // 将数据库中资源类型赋值给修改用的dto，该值不会允许修改，但是后面组成员校验需要用
        newResourceGroupDto.setSourceSubType(existGroupDto.getSourceSubType());
        newResourceGroupDto.setGroupType(existGroupDto.getGroupType());

        // 检查并更新组成员
        checkAndUpdateGroupMembers(newResourceGroupDto);

        // 找出当前的existResourceIdSet，manual组会查询member表， rule会查protected_object表
        Set<String> existResourceIdSet = existGroupDto.getResources().stream().map(
            ResourceGroupMemberDto::getSourceId).collect(Collectors.toSet());

        // 取出新的newResourceIdSet
        Set<String> newResourceIdset;
        List<ProtectionResourceDto> protectionResources = Collections.emptyList();
        if (GroupTypeEnum.RULE.getValue().equals(existGroupDto.getGroupType())) {
            protectionResources = filterGroupMembersByRule(newResourceGroupDto,
                existGroupDto);
            newResourceIdset = protectionResources.stream()
                .map(ProtectionResourceDto::getUuid).collect(Collectors.toSet());
        } else {
            newResourceIdset = newResourceGroupDto.getResources().stream().map(
                ResourceGroupMemberDto::getSourceId).collect(Collectors.toSet());
        }

        // 计算待删除和待添加的ids
        Set<String> toDeleteResourceIdSet = removeElements(existResourceIdSet, newResourceIdset);
        Set<String> toAddResourceIdSet = removeElements(newResourceIdset, existResourceIdSet);
        updateProtectObjectIfProtected(newResourceGroupDto, existGroupDto,
            toDeleteResourceIdSet, toAddResourceIdSet, protectionResources);
        return resourceGroupRepository.update(newResourceGroupDto, toDeleteResourceIdSet, toAddResourceIdSet);
    }

    private void updateProtectObjectIfProtected(ResourceGroupDto newGroupDto, ResourceGroupDto existGroupDto,
        Set<String> toDeleteResourceIdSet, Set<String> toAddResourceIdSet,
        List<ProtectionResourceDto> protectionResources) {
        ResourceGroupProtectedObjectDto protectedObject = existGroupDto.getProtectedObjectDto();
        if (VerifyUtil.isEmpty(protectedObject) || VerifyUtil.isEmpty(protectedObject.getSlaId())) {
            return;
        }
        // 创建保护时旧的dto中的保护对象
        newGroupDto.setProtectedObjectDto(existGroupDto.getProtectedObjectDto());
        ProtectionExecuteDto dto = new ProtectionExecuteDto();
        dto.setSlaId(protectedObject.getSlaId());
        dto.setResource(newGroupDto);
        dto.setExtParameters(protectedObject.getExtParameters());
        if (GroupTypeEnum.RULE.getValue().equals(newGroupDto.getGroupType())) {
            dto.setSubResources(protectionResources.stream()
                .filter(v -> toAddResourceIdSet.contains(v.getUuid())).collect(Collectors.toList()));
        } else {
            dto.setSubResources(resourceService
                .getResourceListByUuidList(new ArrayList<>(toAddResourceIdSet))
                .stream()
                .map(this::convertToProtectionResource)
                .collect(Collectors.toList()));
        }
        projectedObjectService.changeGroupProjectedObject(dto, new ArrayList<>(toDeleteResourceIdSet));
    }

    private Set<String> removeElements(Set<String> sources, Set<String> removes) {
        Set<String> target = new HashSet<>(sources);
        target.removeAll(removes);
        return target;
    }

    private List<ProtectionResourceDto> filterGroupMembersByRule(ResourceGroupDto dto, ResourceGroupDto existGroupDto) {
        if (!GroupTypeEnum.RULE.getValue().equals(dto.getGroupType())) {
            return Collections.emptyList();
        }
        if (VerifyUtil.isEmpty(dto.getExtendStr())
        || VerifyUtil.isEmpty(existGroupDto.getProtectedObjectDto())) {
            return Collections.emptyList();
        }
        ResourceFilterConditionParam extendParam = JSON.parseObject(dto.getExtendStr(),
            ResourceFilterConditionParam.class);
        List<VirtualResourceExtendPo> resourceExtendPos = listAllVmware(extendParam.getResourcePathFilter(),
            existGroupDto.getScopeResourceId());
        List<VirtualResourceExtendPo> resources = resourceExtendPos.stream()
            .filter(v -> v.getProtectedObject() == null
                || dto.getUuid().equals(v.getProtectedObject().getResourceGroupId()))
            .collect(Collectors.toList());
        resources = ResourceFilterUtil.filterResources(resources, extendParam);
        return resources
            .stream()
            .map(this::convertToProtectionResource)
            .collect(Collectors.toList());
    }

    private List<VirtualResourceExtendPo> listAllVmware(String path, String scopeId) {
        return virtualResourceMapper.listAll(VerifyUtil.isEmpty(path) ? path : path + "%", scopeId);
    }

    private ResourceGroupDto checkIfExist(ResourceGroupDto newResourceGroupDto) {
        Optional<ResourceGroupDto> existGroupDtoOptional =
                resourceGroupRepository.selectById(newResourceGroupDto.getUuid());
        PowerAssert.state(existGroupDtoOptional.isPresent(),
                () -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST,
                        "The resource group to update does not exist."));
        return existGroupDtoOptional.get();
    }
    private void checkNameForUpdate(ResourceGroupDto newResourceGroupDto) {
        if (newResourceGroupDto.getName() == null) {
            return;
        }
        // 如果数据库中已有同名group，则必须和待修改的为同一个
        Optional<ResourceGroupDto> optionalResourceGroupDto = resourceGroupRepository
                .selectByScopeResourceIdAndName(
                        newResourceGroupDto.getScopeResourceId(),
                        newResourceGroupDto.getName());
        optionalResourceGroupDto.ifPresent(
                groupDto -> PowerAssert.state(Objects.equals(groupDto.getUuid(), newResourceGroupDto.getUuid()),
                () -> new LegoCheckedException(CommonErrorCode.DUPLICATE_NAME,
                        "The name of is duplicated.")));
    }

    /**
     * queryResourceGroups
     *
     * @param queryParams queryParams
     * @return PageListResponse<ResourceGroupVo>
     */
    @Override
    public PageListResponse<ResourceGroupVo> queryResourceGroups(ResourceGroupQueryParams queryParams) {
        BasePage<ResourceGroupDto> resourceGroupPoBasePage = resourceGroupRepository.queryResourceGroups(queryParams);
        PageListResponse<ResourceGroupVo> voPageListResponse = new PageListResponse<>();
        voPageListResponse.setPageSize((int) resourceGroupPoBasePage.getPages());
        if (CollectionUtils.isEmpty(resourceGroupPoBasePage.getItems())) {
            return new PageListResponse<>(0, new ArrayList<>());
        }
        convertBasePageToPageListResponse(resourceGroupPoBasePage, voPageListResponse);
        return voPageListResponse;
    }

    @Override
    public String deleteResourceGroup(String uuid) {
        Optional<ResourceGroupDto> optionalResourceGroupDto = resourceGroupRepository.selectById(uuid);
        if (!optionalResourceGroupDto.isPresent()) {
            log.warn("ResourceGroup with id of {} has already been deleted!", uuid);
            throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST,
                "resource group has been deleted.");
        }
        ResourceGroupDto resourceGroupDto = optionalResourceGroupDto.get();
        String[] errorParams = new String[] {resourceGroupDto.getName()};
        if (!VerifyUtil.isEmpty(resourceGroupDto.getProtectedObjectDto())) {
            throw new LegoCheckedException(ProtectedResourceRepositoryImpl.RESOURCE_ALREADY_BIND_SLA, errorParams,
                "The resource group with protected object can not be deleted.");
        }
        resourceGroupRepository.delete(uuid);
        return uuid;
    }

    private void convertBasePageToPageListResponse(BasePage<ResourceGroupDto> poBasePage,
                                                   PageListResponse<ResourceGroupVo> dtoPageListResponse) {
        dtoPageListResponse.setTotalCount((int) poBasePage.getTotal());
        List<ResourceGroupDto> resourceGroupDtos = poBasePage.getItems();
        List<ResourceGroupVo> resourceGroupVos = new ArrayList<>();

        // Po 转化为 Dto，再转化为 Vo
        for (ResourceGroupDto resourceGroupDto : resourceGroupDtos) {
            BeanUtils.copyProperties(resourceGroupDto, resourceGroupDto);
            ResourceGroupVo resourceGroupVo = new ResourceGroupVo();
            BeanUtils.copyProperties(resourceGroupDto, resourceGroupVo);
            resourceGroupVo.setCreatedTime(resourceGroupDto.getCreatedTime().toString());
            // 处理资源数量
            resourceGroupVo.setResourceCount(
                    resourceGroupDto.getResources() != null ? resourceGroupDto.getResources().size() : 0);
            resourceGroupVo.setResourceGroupMembers(getResourceGroupMembers(resourceGroupDto));
            resourceGroupVo.setLabelList(labelService.findAllDomainLabelByResourceId(resourceGroupDto.getUuid(),
                sessionService.getCurrentUser().getDomainId()));
            // 转换保护对象
            if (resourceGroupDto.getProtectedObjectDto() != null) {
                ResourceGroupProtectedObjectVo protectedObjectVo = new ResourceGroupProtectedObjectVo();
                BeanUtils.copyProperties(resourceGroupDto.getProtectedObjectDto(), protectedObjectVo);
                resourceGroupVo.setProtectedObject(protectedObjectVo);
            }
            resourceGroupVos.add(resourceGroupVo);
        }
        dtoPageListResponse.setRecords(resourceGroupVos);
    }

    @Override
    public ResourceGroupDetailVo queryResourceGroupDetail(String resourceGroupId) {
        // 获取资源组主体信息
        Optional<ResourceGroupDto> optionalResourceGroupDto = resourceGroupRepository.selectById(resourceGroupId);
        PowerAssert.state(optionalResourceGroupDto.isPresent(),
                () -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST,
                        "The resource group to query does not exist."));
        ResourceGroupDetailVo resourceGroupDetailVo = new ResourceGroupDetailVo();
        ResourceGroupDto resourceGroupDto = optionalResourceGroupDto.get();
        BeanUtils.copyProperties(resourceGroupDto, resourceGroupDetailVo);
        resourceGroupDetailVo.setCreatedTime(resourceGroupDto.getCreatedTime().toString());

        // 获取资源组成员信息
        resourceGroupDetailVo.setResourceGroupMembers(getResourceGroupMembers(resourceGroupDto));
        // 获取保护对象信息
        setProtectedObject(resourceGroupDto, resourceGroupDetailVo);
        return resourceGroupDetailVo;
    }

    @Override
    public Optional<ResourceGroupDto> queryResourceGroupDto(String resourceGroupId) {
        return resourceGroupRepository.selectById(resourceGroupId);
    }

    private List<ResourceGroupMemberVo> getResourceGroupMembers(ResourceGroupDto resourceGroupDto) {
        List<ResourceGroupMemberDto> resourceGroupMemberDtos = resourceGroupDto.getResources();
        if (VerifyUtil.isEmpty(resourceGroupMemberDtos)) {
            return new ArrayList<>();
        }
        Set<String> resourceGroupMemberUuids =
                resourceGroupMemberDtos.stream().map(ResourceGroupMemberDto::getSourceId).collect(Collectors.toSet());
        Map<String, ProtectedResource> protectedResourceMap = getStringProtectedResourceMap(resourceGroupMemberUuids);
        List<ResourceGroupMemberVo> resourceGroupMemberVos = new ArrayList<>();
        for (ResourceGroupMemberDto groupMemberDto : resourceGroupDto.getResources()) {
            ResourceGroupMemberVo resourceGroupMemberVo = new ResourceGroupMemberVo();
            BeanUtils.copyProperties(groupMemberDto, resourceGroupMemberVo);
            ProtectedResource protectedResource = protectedResourceMap.get(groupMemberDto.getSourceId());
            if (protectedResource == null) {
                resourceGroupMemberMapper.deleteById(groupMemberDto.getUuid());
                continue;
            }
            resourceGroupMemberVo.setSourceName(protectedResource.getName());
            resourceGroupMemberVo.setPath(protectedResource.getPath());
            resourceGroupMemberVo.setStatus(protectedResource.getExtendInfoByKey("status"));
            resourceGroupMemberVos.add(resourceGroupMemberVo);
        }
        return resourceGroupMemberVos;
    }

    private void setProtectedObject(ResourceGroupDto resourceGroupDto, ResourceGroupDetailVo resourceGroupDetailVo) {
        if (resourceGroupDto.getProtectedObjectDto() != null) {
            ResourceGroupProtectedObjectVo protectedObjectVo = new ResourceGroupProtectedObjectVo();
            BeanUtils.copyProperties(resourceGroupDto.getProtectedObjectDto(), protectedObjectVo);
            resourceGroupDetailVo.setProtectedObject(protectedObjectVo);
            resourceGroupDetailVo.setProtectionStatus(ProtectionStatusEnum.PROTECTED.getType());
        } else {
            resourceGroupDetailVo.setProtectionStatus(ProtectionStatusEnum.UNPROTECTED.getType());
        }
    }

    @Override
    public List<ResourceGroupResultDto> createProtectedObject(CreateResourceGroupProtectedObjectRequest request) {
        ResourceGroupDto resourceGroupDto =
                resourceGroupRepository.selectById(request.getResourceGroupId())
                        .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST,
                                "Not found resource group. ID:" + request.getResourceGroupId()));
        // 保护前检查
        preCheckBeforeProtect(request, resourceGroupDto);
        // 创建保护
        ProtectionExecuteDto dto = buildProtectionExecuteDto(request, resourceGroupDto);
        projectedObjectService.createGroupProjectedObject(dto, (userId) -> handlePostAction(request, userId));
        List<ResourceGroupResultDto> resourceGroupResultDtoList = new ArrayList<>();
        ResourceGroupResultDto resourceGroupResultDto = new ResourceGroupResultDto();
        resourceGroupResultDto.setResourceName(resourceGroupDto.getName());
        resourceGroupResultDto.setSuccess(true);
        resourceGroupResultDtoList.add(resourceGroupResultDto);
        return resourceGroupResultDtoList;
    }

    private ProtectionExecuteDto buildProtectionExecuteDto(CreateResourceGroupProtectedObjectRequest request,
        ResourceGroupDto resourceGroupDto) {
        ProtectionExecuteDto dto = new ProtectionExecuteDto();
        dto.setSlaId(request.getSlaId());
        dto.setResource(resourceGroupDto);
        dto.setExtParameters(request.getExtParams());
        dto.setPostAction(request.getPostAction());
        if (GroupTypeEnum.RULE.getValue().equals(resourceGroupDto.getGroupType())) {
            ResourceFilterConditionParam extendParam = JSON.parseObject(resourceGroupDto.getExtendStr(),
                ResourceFilterConditionParam.class);
            List<VirtualResourceExtendPo> resourceExtendPos = listAllVmware(extendParam.getResourcePathFilter(),
                resourceGroupDto.getScopeResourceId());
            List<ProtectionResourceDto> protectionResources = ResourceFilterUtil
                .filterResources(resourceExtendPos, extendParam)
                .stream()
                .map(this::convertToProtectionResource)
                .collect(Collectors.toList());
            dto.setSubResources(protectionResources);
        } else {
            List<String> ids = dto.getResource()
                .getResources()
                .stream()
                .map(ResourceGroupMemberDto::getSourceId)
                .collect(Collectors.toList());
            List<ProtectionResourceDto> protectionResources = resourceService.getResourceListByUuidList(ids)
                .stream()
                .map(this::convertToProtectionResource)
                .collect(Collectors.toList());
            dto.setSubResources(protectionResources);
        }
        return dto;
    }

    private ProtectionResourceDto convertToProtectionResource(VirtualResourceExtendPo po) {
        ProtectionResourceDto dto = new ProtectionResourceDto();
        BeanUtils.copyProperties(po, dto);
        dto.setProtectedObject(po.getProtectedObject());
        return dto;
    }

    private ProtectionResourceDto convertToProtectionResource(ProtectedResource member) {
        ProtectionResourceDto dto = new ProtectionResourceDto();
        BeanUtils.copyProperties(member, dto);
        return dto;
    }

    private void handlePostAction(CreateResourceGroupProtectedObjectRequest createReq, String userId) {
        log.info("Start to handle postAction {}", createReq.getPostAction());
        if (VerifyUtil.isEmpty(createReq.getPostAction())) {
            log.info("the post action is empty, nothing need to do.");
            return;
        }
        if (!Objects.equals(createReq.getPostAction(), "BACKUP")) {
            log.warn("Invalid post action {}, no need to execute.", createReq.getPostAction());
            return;
        }
        ManualBackupReq backupReq = new ManualBackupReq();
        backupReq.setAction(PolicyAction.FULL.getAction());
        backupReq.setSlaId(createReq.getSlaId());
        backupReq.setIsResourceGroup(true);
        List<String> jobIds = resourceGroupProtectedObjectRestApi.manualBackup(
                createReq.getResourceGroupId(), userId, backupReq);
        log.info("Create manual backup job success, job id{} for resource group {} for post action {}.", jobIds,
                createReq.getResourceGroupId(), createReq.getPostAction());
    }

    private void preCheckBeforeProtect(CreateResourceGroupProtectedObjectRequest createProtectedObjectReq,
                                       ResourceGroupDto resourceGroupDto) {
        ProtectedObjectInfo protectObject =
            protectObjectRestApi.getProtectObject(createProtectedObjectReq.getResourceGroupId());
        if (!VerifyUtil.isEmpty(protectObject)) {
            throw new LegoCheckedException(ProtectObjectErrorCode.RESOURCE_ALREADY_PROTECTED,
                "The resource has been already protected.");
        }
        //  资源组为空，不允许创建保护
        if (!GroupTypeEnum.RULE.getValue().equals(resourceGroupDto.getGroupType())
            && VerifyUtil.isEmpty(resourceGroupDto.getResources())) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "The resource group with no member can not "
                    + "be protected.");
        }
        // 如果资源组类型为虚拟机组，不能绑定通用sla
        ProtectionInterceptorProvider provider = providerManager.findProvider(ProtectionInterceptorProvider.class,
                resourceGroupDto.getSourceSubType(), null);
        if (provider != null) {
            log.debug("Protection interceptor for {} is provided, and the interceptor will be used!",
                    resourceGroupDto.getSourceSubType());
            provider.preCheck(buildCreateProtectedObjectRequest(createProtectedObjectReq));
        }
    }

    private ProtectedObjectRequest buildCreateProtectedObjectRequest(
            CreateResourceGroupProtectedObjectRequest createReq) {
        ProtectedObjectRequest request = new ProtectedObjectRequest();
        BeanUtils.copyProperties(createReq, request);
        return request;
    }

    private ProtectionModifyDto createProtectionModifyDto(
        UpdateResourceGroupProtectedObjectRequest updateProtectedObjectReq, String resourceId, boolean isResourceGroup,
        boolean isGroupSubResource, String resourceGroupId) {
        ProtectionModifyDto protectionModifyDto = new ProtectionModifyDto();
        protectionModifyDto.setSlaId(updateProtectedObjectReq.getSlaId());
        protectionModifyDto.setResourceId(resourceId);
        protectionModifyDto.setIsResourceGroup(isResourceGroup);
        protectionModifyDto.setIsGroupSubResource(isGroupSubResource);
        protectionModifyDto.setResourceGroupId(resourceGroupId);
        protectionModifyDto.setExtParameters(!VerifyUtil.isEmpty(updateProtectedObjectReq.getExtParams())
            ? updateProtectedObjectReq.getExtParams().toMap(Object.class) : new HashMap<>());
        return protectionModifyDto;
    }

    private UuidObject
        createResourceGroupProtectObject(CreateResourceGroupProtectedObjectRequest createProtectedObjectReq) {
        UpdateResourceGroupProtectedObjectRequest request = new UpdateResourceGroupProtectedObjectRequest();
        BeanUtils.copyProperties(createProtectedObjectReq, request);
        ProtectionModifyDto modifyDto = createProtectionModifyDto(request, request.getResourceGroupId(),
                true, false, createProtectedObjectReq.getResourceGroupId());
        return protectObjectRestApi.createProtectedObjectInternal(modifyDto);
    }

    @Override
    public List<ResourceGroupResultDto> updateProtectedObject(UpdateResourceGroupProtectedObjectRequest updateReq) {
        ResourceGroupDto resourceGroupDto = resourceGroupRepository.selectById(updateReq.getResourceGroupId())
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST,
                "Not found resource group. ID:" + updateReq.getResourceGroupId()));
        if (VerifyUtil.isEmpty(resourceGroupDto.getResources())) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "The protected object of resource group "
                    + "with no member can not be modified.");
        }
        // 校验资源组绑定sla是否发生更改，如果发生更改，类型为虚拟机组的资源不能修改绑定为通用sla
        checkSlaIsChanged(updateReq, resourceGroupDto);
        // 存在运行中任务的资源组不允许修改保护
        if (checkHasUnfinishedJob(resourceGroupDto.getUuid())) {
            throw new LegoCheckedException(CommonErrorCode.HAVE_RUNNING_JOB, "There are some unfinished jobs with "
                    + "resource_group_id " + resourceGroupDto.getUuid() + ", so the protected object can not be "
                    + "modified.");
        }
        // 修改资源组的保护对象
        ProtectionModifyDto modifyGroupDto =
            createProtectionModifyDto(updateReq, updateReq.getResourceGroupId(),
                    true, false, updateReq.getResourceGroupId());
        String userId = TokenBo.get().getUser().getId();
        String groupJobId = protectObjectRestApi.modifyProtectedObject(userId, modifyGroupDto);
        log.info("Success create modify protect object job(for group resource:{}). Job id :{}",
            updateReq.getResourceGroupId(), groupJobId);
        List<ResourceGroupResultDto> resourceGroupResultDtoList = new ArrayList<>();
        ResourceGroupResultDto resourceGroupResultDto = new ResourceGroupResultDto();
        resourceGroupResultDto.setResourceName(resourceGroupDto.getName());
        resourceGroupResultDto.setSuccess(true);
        resourceGroupResultDtoList.add(resourceGroupResultDto);
        return resourceGroupResultDtoList;
    }

    private ProtectedObjectRequest buildUpdateProtectedObjectRequest(
            UpdateResourceGroupProtectedObjectRequest updateReq) {
        ProtectedObjectRequest request = new ProtectedObjectRequest();
        BeanUtils.copyProperties(updateReq, request);
        return request;
    }

    private boolean checkHasUnfinishedJob(String resourceGroupId) {
        List<String> statusList = new ArrayList<>();
        JobStatusEnum.getUnfinishedStatusList().forEach(jobStatusEnum -> statusList.add(jobStatusEnum.name()));
        QueryJobRequest queryJobRequest = new QueryJobRequest();
        queryJobRequest.setStatusList(statusList);
        queryJobRequest.setResourceGroupId(resourceGroupId);
        Integer jobCount = jobService.getJobCount(queryJobRequest);
        return jobCount != 0;
    }

    private void checkSlaIsChanged(UpdateResourceGroupProtectedObjectRequest updateReq,
                                   ResourceGroupDto resourceGroupDto) {
        if (!updateReq.getSlaId().equals(resourceGroupDto.getProtectedObjectDto().getSlaId())) {
            ProtectionInterceptorProvider provider = providerManager.findProvider(ProtectionInterceptorProvider.class,
                    resourceGroupDto.getSourceSubType(), null);
            if (provider != null) {
                log.debug("Protection interceptor for {} is provided, and the interceptor will be used!",
                        resourceGroupDto.getSourceSubType());
                provider.preCheck(buildUpdateProtectedObjectRequest(updateReq));
            }
        }
    }

    @Override
    public String deleteProtectedObject(String resourceGroupId) {
        ResourceGroupDto resourceGroupDto = resourceGroupRepository.selectById(resourceGroupId)
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST,
                "Not found resource group. ID:" + resourceGroupId));
        // 存在运行中任务的资源组不允许删除保护
        if (checkHasUnfinishedJob(resourceGroupDto.getUuid())) {
            throw new LegoCheckedException(CommonErrorCode.HAVE_RUNNING_JOB, "There are some unfinished jobs with "
                    + "resource_group_id " + resourceGroupDto.getUuid() + ", so the protected object can not be "
                    + "deleted.");
        }
        ProtectionBatchOperationReq deleteGroupReq = new ProtectionBatchOperationReq();
        deleteGroupReq.setIsResourceGroup(true);
        deleteGroupReq.setResourceIds(Collections.singletonList(resourceGroupId));
        protectObjectRestApi.deleteProtectedObjects(deleteGroupReq);
        log.info("Remove group protect success.Group id:{}", resourceGroupId);
        List<String> subResourceIds = resourceGroupDto.getResources()
            .stream()
            .map(ResourceGroupMemberDto::getSourceId)
            .collect(Collectors.toList());
        ProtectionBatchOperationReq deleteGroupSubSourceReq = new ProtectionBatchOperationReq();
        deleteGroupSubSourceReq.setIsResourceGroup(false);
        deleteGroupSubSourceReq.setResourceIds(subResourceIds);
        protectObjectRestApi.deleteProtectedObjects(deleteGroupSubSourceReq);
        log.info("Remove protect success.Resource ids:{}", subResourceIds);
        return resourceGroupId;
    }

    @Override
    public void activateResourceGroup(ProtectionBatchOperationReq batchOperationReq) {
        protectObjectRestApi.activateResourceGroup(batchOperationReq);
    }

    @Override
    public void deactivateResourceGroup(ProtectionBatchOperationReq batchOperationReq) {
        protectObjectRestApi.deactivateResourceGroup(batchOperationReq);
    }

    @Override
    public List<ResourceGroupDto> getAllResourceGroupList(List<String> subTypeList) {
        if (VerifyUtil.isEmpty(subTypeList)) {
            return Collections.emptyList();
        }
        return resourceGroupRepository.getAllResourceGroupList(subTypeList);
    }
}